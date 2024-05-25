#ifndef SCC_DELETE_H
#define SCC_DELETE_H

#include "graph.h"
#include <utility>


bool not_deleted(int &src, int &dst, del_set &deleted_edges) {
  std::pair<int, int> temp = std::make_pair(src, dst);
  if (deleted_edges.find(temp) != deleted_edges.end()) {
    return false;
  } else {
    return true;
  }
}

void fw_new(int &node, graph &g, int *&fw_reach, int *&reachable,
            del_set &deleted_edges, int *&queue,
            int *&next_queue) { // TO:DO can be combined with fw_propagate in
                                // the other file, need to implement that

  int q_size = 0, nxt_q_size = 0;
  queue[q_size++] = node;
  fw_reach[node] = node;
  while (q_size) {
    //	for(int i=0; i<q_size; i++){ //doing separately to ensure we dont repeat
    // vertices later 		int vert = queue[i]; 		fw_reach[vert] =
    // node;
    //	}
    for (int i = 0; i < q_size; i++) {
      int vert = queue[i];
      int Odeg = out_degree(g, vert);
      int *Overts = out_vertices(g, vert);
      for (int j = 0; j < Odeg; j++) {
        int nVert = Overts[j];
        if (!reachable[nVert] and not_deleted(vert, nVert, deleted_edges) and
            g.scc_map[node] == g.scc_map[nVert] and fw_reach[nVert] != node) {
          next_queue[nxt_q_size++] = nVert;
          fw_reach[nVert] = node;
        }
      }
    }
    std::swap(queue, next_queue);
    q_size = nxt_q_size;
    nxt_q_size = 0;

  }
  return;
}

void find_scc(
    int &node, int *&to_change, int &num_changed, graph &g, int *&reachable,
    del_set &deleted_edges, int *&fw_reach, int *&queue,
    int *&next_queue) { // will return the list of verts that needs scc changed

  int q_size = 0, nxt_q_size = 0;
  queue[q_size++] = node;
  to_change[num_changed++] = node;
  reachable[node] = 1;
  while (q_size) {
    for (int i = 0; i < q_size; i++) {
      int vert = queue[i];
      int Ideg = in_degree(g, vert);
      int *Iverts = in_vertices(g, vert);
      for (int j = 0; j < Ideg; j++) {
        int nVert = Iverts[j];
        if (not_deleted(nVert, vert, deleted_edges) and
            node == fw_reach[nVert] and !reachable[nVert] and
            g.scc_map[nVert] ==
                g.scc_map[node]) { // last confition might be redundant
          next_queue[nxt_q_size++] = nVert;

          reachable[nVert] = 1;
          to_change[num_changed++] =
              nVert; // adding this vertex beacuse it needs scc change
        }
      }
    }
    std::swap(queue, next_queue);
    q_size = nxt_q_size;
    nxt_q_size = 0;
  }
}

void update_sccs(int &vert, int *&to_change, int &size, graph &g) {

  int index = find_index(g.scc_map[vert], g.rep_nodes, g.scc_count);

  g.count_in_sccs[index] -= size; // reducing the number of scc nodes

  for (int i = 0; i < size; i++) {

    g.scc_map[to_change[i]] = vert;
  }

  g.rep_nodes[g.scc_count] = vert;
  g.count_in_sccs[g.scc_count] = size;
  g.scc_count++;
}

void make_scc(int &vert, graph &g, int *&fw_reach, int *&reachable,
              del_set &deleted_edges, int *&queue, int *&next_queue,
              int *&to_change) {
  // printf("Starting to make scc of vertex %d\n", vert);
  fw_new(vert, g, fw_reach, reachable, deleted_edges, queue, next_queue);
  int num_changed = 0;
  // int index = find_index(g.scc_map[vert], g.rep_nodes, g.scc_count);
  find_scc(vert, to_change, num_changed, g, reachable, deleted_edges, fw_reach,
           queue, next_queue);
  update_sccs(vert, to_change, num_changed, g);
}

void create_new_sccs(
    int *&unreachable_verts, int *&reachable, int &num_vrts, graph &g,
    int *&fw_reach,
    del_set &deleted_edges) { // currently we go vertex by vertex and form a new
                              // scc for that particular vertex

  //
  int *queue = new int[g.n];
  int *next_queue = new int[g.n];
  int *to_change = new int[g.n];
  //

  for (int i = 0; i < num_vrts;
       i++) { // this part not yet to parallelise due to potential conflict in
              // forward update
    int vert = unreachable_verts[i];
    if (!reachable[vert]) {
      make_scc(vert, g, fw_reach, reachable, deleted_edges, queue, next_queue,
               to_change);
    }
  }

  //
  delete[] queue;
  delete[] next_queue;
  delete[] to_change;
//
}

void fw_propagate(int rep_node, graph &g, int *reachable, int *fw_reach,
                  del_set &deleted_edges, int *&queue, int *&next_queue) {

  int q_size = 0, nxt_q_size = 0;
  queue[q_size++] = rep_node;
  fw_reach[rep_node] = rep_node;
  while (q_size) {
    for (int i = 0; i < q_size; i++) {
      int vert = queue[i];
      // reachable[vert] = 1;
      int Odeg = out_degree(g, vert);
      int *Overts = out_vertices(g, vert);
      for (int j = 0; j < Odeg; j++) {
        int nVert = Overts[j];
        if ((rep_node != fw_reach[nVert]) and
            (g.scc_map[nVert] == g.scc_map[rep_node]) and
            not_deleted(vert, nVert,
                        deleted_edges)) { // checking for -1 as only rep_node is
                                          // propagating hence other nodes cant
                                          // come here
          next_queue[nxt_q_size++] = nVert;
          fw_reach[nVert] = rep_node;
        }
      }
    }
    q_size = nxt_q_size;
    std::swap(queue, next_queue);
    nxt_q_size = 0;
  }
}

void bw_propagate(int rep_node, graph &g, int *reachable, int *fw_arr,
                  int &unaffected, del_set &deleted_edges, int *&queue,
                  int *&next_queue) {
  int *scc_counts = g.count_in_sccs;
  int *scc_map = g.scc_map;

  int q_size = 0, nxt_q_size = 0;
  reachable[rep_node] = 1;
  queue[0] = rep_node;
  q_size++;

  while (q_size) {
    for (int i = 0; i < q_size; i++) {
      unaffected++;
      int vert = queue[i];
      int Ideg = in_degree(g, vert);
      int *Iverts = in_vertices(g, vert);
      for (int j = 0; j < Ideg; j++) {
        int nVert = Iverts[j];
        if (scc_map[rep_node] == scc_map[nVert] and
            rep_node == fw_arr[nVert] and
            not_deleted(nVert, vert, deleted_edges) and !reachable[nVert]) {
          next_queue[nxt_q_size++] = nVert;
          reachable[nVert] = 1;
        }
      }
    }
    std::swap(queue, next_queue);
    q_size = nxt_q_size;
    nxt_q_size = 0;
  }
}

void search(int rep_node, graph &g, int *reachable, int *fw_arr,
            int &unaffected, del_set &deleted_edges, int *&queue,
            int *&next_queue) {

  fw_propagate(rep_node, g, reachable, fw_arr, deleted_edges, queue,
               next_queue);
  bw_propagate(rep_node, g, reachable, fw_arr, unaffected, deleted_edges, queue,
               next_queue);
}

void naive_delete(del_set &deleted_edges, graph &g, int *&out_q, int *&in_q,
                  int q_size) {

  int *reachable = new int[g.n]; // to check if they are part of an scc (could
                                 // be formed later or retained in an scc)
  int *fw_reach = new int[g.n]; // useful to take into account all vertices that
                                // can be reached from rep_node

  int unaffected = 0; // takes count of unaffected nodes
  for (int i = 0; i < g.n; i++) {
    fw_reach[i] = -1;
  }
  for (int i = 0; i < g.n; i++) {
    reachable[i] = 0;
  }

  // defining for maximum capacity
  int *queue = new int[g.n];
  int *next_queue = new int[g.n];
  //
  for (int i = 0; i < g.scc_count; i++) {
    int vert = g.rep_nodes[i];
    if (!reachable[vert]) {
      search(vert, g, reachable, fw_reach, unaffected, deleted_edges, queue,
             next_queue);
    }
  }
  // deleted the pre allocated space
  delete[] queue;
  delete[] next_queue;
  //

  int *unreachable = new int[g.n - unaffected]; // array to keep list of verts
                                                // that needs to be changed
  int affected = 0;
  for (int i = 0; i < g.n; i++) {
    if (!reachable[i]) {
      unreachable[affected++] = i;
    }
  }

  create_new_sccs(unreachable, reachable, affected, g, fw_reach,
                  deleted_edges); // unreachable is array filled with changed
                                  // verts (bad var names)

  delete[] fw_reach;
  delete[] unreachable;
  delete[] reachable;
}

#endif
