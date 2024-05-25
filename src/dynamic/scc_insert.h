
#include "graph.h"

#include <algorithm>
#include <utility>
#include <omp.h>

int *con_scc_color_propagate(graph &g, int *valid, int *valid_verts,
                             int num_valid, unsigned int *scc_diff_outdeg_list,
                             int *scc_diff_out) {
  int num_verts = g.n;
  bool *in_queue = new bool[num_verts];
  bool *in_queue_next = new bool[num_verts];
  int *queue = new int[num_verts];
  int *queue_next = new int[num_verts];

  std::copy(valid_verts, valid_verts + num_valid, queue);

  int *colors = new int[num_verts];
#pragma omp parallel for schedule(static)
  for (int i = 0; i < num_valid; ++i) {
    int vert = valid_verts[i];
    colors[vert] = vert;
    in_queue[vert] = true;
    in_queue_next[vert] = false;
  }

  int next_size = 0;
  int queue_size = num_valid;
#pragma omp parallel
  {
    int thread_queue[THREAD_QUEUE_SIZE];
    int thread_queue_size = 0;
    int thread_start;

    while (queue_size) {
#pragma omp for schedule(guided) nowait
      for (int i = 0; i < queue_size; ++i) {
        int vert = queue[i];
        in_queue[vert] = false;
        int color = colors[vert];
        bool changed = false;

        int out_degree = out_degree(g, vert);
        int *outs = out_vertices(g, vert);
        for (int j = 0; j < out_degree; ++j) {
          int out = outs[j];
          int out_color = colors[out];

          if (valid[out] && color > out_color) {
            colors[out] = color;
            changed = true;

            if (!in_queue_next[out]) {
              in_queue_next[out] = true;
              thread_queue[thread_queue_size++] = out;

              if (thread_queue_size == THREAD_QUEUE_SIZE) {
#pragma omp atomic capture
                thread_start = next_size += thread_queue_size;

                thread_start -= thread_queue_size;
                for (int l = 0; l < thread_queue_size; ++l)
                  queue_next[thread_start + l] = thread_queue[l];
                thread_queue_size = 0;
              }
            }
          }
        }

        out_degree =
            scc_diff_outdeg_list[vert + 1] - scc_diff_outdeg_list[vert];
        outs = &scc_diff_out[scc_diff_outdeg_list[vert]];
        for (int j = 0; j < out_degree; ++j) {
          int out = outs[j];
          int out_color = colors[out];

          if (valid[out] && color > out_color) {
            colors[out] = color;
            changed = true;

            if (!in_queue_next[out]) {
              in_queue_next[out] = true;
              thread_queue[thread_queue_size++] = out;

              if (thread_queue_size == THREAD_QUEUE_SIZE) {
#pragma omp atomic capture
                thread_start = next_size += thread_queue_size;

                thread_start -= thread_queue_size;
                for (int l = 0; l < thread_queue_size; ++l)
                  queue_next[thread_start + l] = thread_queue[l];
                thread_queue_size = 0;
              }
            }
          }
        }

        if (changed && !in_queue_next[vert]) {
          in_queue_next[vert] = true;
          thread_queue[thread_queue_size++] = vert;

          if (thread_queue_size == THREAD_QUEUE_SIZE) {
#pragma omp atomic capture
            thread_start = next_size += thread_queue_size;

            thread_start -= thread_queue_size;
            for (int l = 0; l < thread_queue_size; ++l)
              queue_next[thread_start + l] = thread_queue[l];
            thread_queue_size = 0;
          }
        }
      }

#pragma omp atomic capture
      thread_start = next_size += thread_queue_size;

      thread_start -= thread_queue_size;
      for (int l = 0; l < thread_queue_size; ++l)
        queue_next[thread_start + l] = thread_queue[l];
      thread_queue_size = 0;
#pragma omp barrier

#pragma omp single
      {
        queue_size = next_size;
        next_size = 0;
        int *temp = queue;
        queue = queue_next;
        queue_next = temp;
        bool *temp2 = in_queue;
        in_queue = in_queue_next;
        in_queue_next = temp2;
      }
    } // end while
  } // end parallel

  delete[] in_queue;
  delete[] in_queue_next;
  delete[] queue;
  delete[] queue_next;

  return colors;
}

int *con_scc_color_get_roots(graph &g, int *valid, int *colors, int &num_roots,
                             int *valid_verts, int num_valid, int *scc_maps,
                             int *scc_index) {
  int *roots = new int[num_valid];
  num_roots = 0;

#pragma omp parallel
  {
    int thread_queue[THREAD_QUEUE_SIZE];
    int thread_queue_size = 0;
    int thread_start;

#pragma omp for schedule(guided) nowait
    for (int i = 0; i < num_valid; ++i) {
      int vert = valid_verts[i];

      if (colors[vert] == vert) {
        valid[vert] = false;
        scc_maps[scc_index[vert]] = scc_index[vert];
        thread_queue[thread_queue_size++] = vert;

        if (thread_queue_size == THREAD_QUEUE_SIZE) {
#pragma omp atomic capture
          thread_start = num_roots += thread_queue_size;

          thread_start -= thread_queue_size;
          for (int l = 0; l < thread_queue_size; ++l)
            roots[thread_start + l] = thread_queue[l];
          thread_queue_size = 0;
        }
      }
    }

#pragma omp atomic capture
    thread_start = num_roots += thread_queue_size;

    thread_start -= thread_queue_size;
    for (int l = 0; l < thread_queue_size; ++l)
      roots[thread_start + l] = thread_queue[l];
  }

  return roots;
}

void con_scc_color_find_sccs(graph &g, int *&valid, int *colors, int *roots,
                             int num_roots, int *&valid_verts, int num_valid,
                             int *scc_maps, int *scc_index, int *scc_diff_in,
                             unsigned int *scc_diff_indeg_list) {
  // int num_verts = g.n;
  int *queue = new int[num_valid];
  int *queue_next = new int[num_valid];
  std::copy(roots, roots + num_roots, queue);
  int queue_size = num_roots;
  int next_size = 0;

#pragma omp parallel
  {
    int thread_queue[THREAD_QUEUE_SIZE];
    int thread_queue_size = 0;
    int thread_start;

    while (queue_size) {

#pragma omp for schedule(guided)
      for (int i = 0; i < queue_size; ++i) {
        int vert = queue[i];
        int color_vert = colors[vert];

        int in_degree = in_degree(g, vert);
        int *ins = in_vertices(g, vert);
        for (int j = 0; j < in_degree; ++j) {
          int in = ins[j];
          // printf("checking vertice %d\n",in);
          int color_in = colors[in];

          if (valid[in] && color_in == color_vert) {
            valid[in] = false;
            scc_maps[scc_index[in]] = scc_index[color_in];
            thread_queue[thread_queue_size++] = in;

            if (thread_queue_size == THREAD_QUEUE_SIZE) {
#pragma omp atomic capture
              thread_start = next_size += thread_queue_size;

              thread_start -= thread_queue_size;
              for (int l = 0; l < thread_queue_size; ++l)
                queue_next[thread_start + l] = thread_queue[l];
              thread_queue_size = 0;
            }
          }
        }
        in_degree = scc_diff_indeg_list[vert + 1] - scc_diff_indeg_list[vert];
        ins = &scc_diff_in[scc_diff_indeg_list[vert]];
        for (int j = 0; j < in_degree; ++j) {
          int in = ins[j];
          int color_in = colors[in];

          if (valid[in] && color_in == color_vert) {
            valid[in] = false;
            scc_maps[scc_index[in]] = scc_index[color_in];
            thread_queue[thread_queue_size++] = in;

            if (thread_queue_size == THREAD_QUEUE_SIZE) {
#pragma omp atomic capture
              thread_start = next_size += thread_queue_size;

              thread_start -= thread_queue_size;
              for (int l = 0; l < thread_queue_size; ++l)
                queue_next[thread_start + l] = thread_queue[l];
              thread_queue_size = 0;
            }
          }
        }
      }

#pragma omp atomic capture
      thread_start = next_size += thread_queue_size;

      thread_start -= thread_queue_size;
      for (int l = 0; l < thread_queue_size; ++l)
        queue_next[thread_start + l] = thread_queue[l];
      thread_queue_size = 0;
#pragma omp barrier

#pragma omp single
      {
        queue_size = next_size;
        next_size = 0;
        int *temp = queue;
        queue = queue_next;
        queue_next = temp;
      }
    }
  }
  delete[] queue;
  delete[] queue_next;
}

void con_scc_update_valid(graph &g, int *&valid, int *&valid_verts,
                          int &num_valid) {
  int new_num_valid = 0;
  int *new_valid_verts = new int[num_valid];

#pragma omp parallel
  {
    int thread_queue[THREAD_QUEUE_SIZE];
    int thread_queue_size = 0;
    int thread_start;

#pragma omp for nowait
    for (int i = 0; i < num_valid; ++i) {
      int vert = valid_verts[i];
      if (valid[vert]) {
        thread_queue[thread_queue_size++] = vert;

        if (thread_queue_size == THREAD_QUEUE_SIZE) {
#pragma omp atomic capture
          thread_start = new_num_valid += thread_queue_size;

          thread_start -= thread_queue_size;
          for (int l = 0; l < thread_queue_size; ++l)
            new_valid_verts[thread_start + l] = thread_queue[l];
          thread_queue_size = 0;
        }
      }
    }

#pragma omp atomic capture
    thread_start = new_num_valid += thread_queue_size;

    thread_start -= thread_queue_size;
    for (int l = 0; l < thread_queue_size; ++l)
      new_valid_verts[thread_start + l] = thread_queue[l];
  }

  num_valid = new_num_valid;

  delete[] valid_verts;
  valid_verts = new_valid_verts;
}

int con_scc_color(graph &g, int *&valid, int *&valid_verts, int *scc_maps,
                  int *scc_index, int *inverse_scc, int *scc_diff_out,
                  int *scc_diff_in, unsigned int *scc_diff_outdeg_list,
                  unsigned int *scc_diff_indeg_list) {
  int num_scc = 0;
  int num_roots = 0;
  int num_valid = g.n;

  while (num_valid > 0) {
    int *colors = con_scc_color_propagate(g, valid, valid_verts, num_valid,
                                          scc_diff_outdeg_list, scc_diff_out);

    int *roots =
        con_scc_color_get_roots(g, valid, colors, num_roots, valid_verts,
                                num_valid, scc_maps, scc_index);

    con_scc_color_find_sccs(g, valid, colors, roots, num_roots, valid_verts,
                            num_valid, scc_maps, scc_index, scc_diff_in,
                            scc_diff_indeg_list);

    con_scc_update_valid(g, valid, valid_verts, num_valid);

    num_scc += num_roots;

    delete[] colors;
    delete[] roots;
  }

  return num_scc;
}

void insert_condense(graph &g, int *&scc_maps, int num_verts, int edges,
                     int *&scc_outarr, int *&scc_inarr,
                     unsigned int *&scc_outdegree_list,
                     unsigned int *&scc_indegree_list, int &roots, int &C_edges,
                     int *&scc_index, int *&inverse_scc,
                     del_set &deleted_edges) {

  /// making the csr for condensed graph
  scc_index = new int[num_verts];   // scc_index[index] = representative scc
  inverse_scc = new int[num_verts]; // inverse_scc[scc_root_node] = vertex in
                                    // condensed graphs
  int root_count = 0, i;
  int condensed_edges = 0;
  int *temp = new int[num_verts];
  std::copy(scc_maps, scc_maps + num_verts, temp);
  for (i = 0; i < num_verts; i++) {
    if (temp[scc_maps[i]] == -1) {
      continue;
    } else {
      scc_index[root_count] = scc_maps[i];
      inverse_scc[scc_maps[i]] = root_count++;
      temp[scc_maps[i]] = -1;
    }
  }
  delete[] temp;
  int *tscc_src = new int[edges];
  int *tscc_dst = new int[edges];
  for (i = 0; i < num_verts; i++) {
    int *out_verts = out_vertices(g, i);
    int out_deg = out_degree(g, i);
    for (int j = 0; j < out_deg; j++) {
      std::pair<int, int> temp = std::make_pair(i, out_verts[j]);
      if ((deleted_edges.find(temp) == deleted_edges.end())) {
        if (scc_maps[i] != scc_maps[out_verts[j]]) {
          tscc_src[condensed_edges] = inverse_scc[scc_maps[i]];
          tscc_dst[condensed_edges] = inverse_scc[scc_maps[out_verts[j]]];
          condensed_edges++;
        }
      }
    }
  }
  int *scc_src = new int[condensed_edges];
  int *scc_dst = new int[condensed_edges];
  std::copy(tscc_src, tscc_src + condensed_edges, scc_src);
  std::copy(tscc_dst, tscc_dst + condensed_edges, scc_dst);
  delete[] tscc_src;
  delete[] tscc_dst;

  create_csr(root_count, condensed_edges, scc_src, scc_dst, scc_outarr,
                 scc_inarr, scc_outdegree_list, scc_indegree_list);

  /////////////////////////
  roots = root_count;
  C_edges = condensed_edges;
  delete[] scc_src;
  delete[] scc_dst;
}

void condense_scc_update(graph &g, int *scc_maps, int *scc_index,
                         int *inverse_scc, int *scc_diff_out, int *scc_diff_in,
                         unsigned int *scc_diff_outdeg_list,
                         unsigned int *scc_diff_indeg_list) {
  int *valid = new int[g.n];
  int *valid_verts = new int[g.n];
  for (int i = 0; i < g.n; i++) {
    valid[i] = 1;
    valid_verts[i] = i;
  }
  con_scc_color(g, valid, valid_verts, scc_maps, scc_index, inverse_scc,
                scc_diff_out, scc_diff_in, scc_diff_outdeg_list,
                scc_diff_indeg_list);
  delete[] valid;
  delete[] valid_verts;
}
