#include "dynamic.h"
#include "graph.h"

#include <chrono>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <vector>

const int THREAD_QUEUE_SIZE = 2048;

#include "scc_delete.h"
#include "scc_insert.h"

static void edge_insertion(graph& g, FILE* file, int* insertsrc_avail,
    int* insertdst_avail, int& inserts, int num_verts,
    int* scc_maps, del_set& deleted_edges)
{
    int root_count, condensed_edges, extra_condense_count = 0;
    int *scc_outarr, *scc_inarr, *scc_index, *inverse_scc;
    unsigned int *scc_outdegree_list, *scc_indegree_list;

    auto t1 = std::chrono::high_resolution_clock::now();
    insert_condense(g, scc_maps, num_verts, g.m, scc_outarr, scc_inarr,
        scc_outdegree_list, scc_indegree_list, root_count,
        condensed_edges, scc_index, inverse_scc, deleted_edges);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto diff1 = std::chrono::duration<double, std::milli>(t2 - t1).count();
    std::cout << "generate condense graph : " << diff1 << "ms\n";
    // updating diff csr
    char op;
    int* scc_diff_src = new int[inserts];
    int* scc_diff_dst = new int[inserts]; // for condensed graph
    int src, dst, extra_in = 0, extra_out = 0;
    int* diff_in_src = new int[inserts]; // inserts is the upper bound
    int* diff_in_dst = new int[inserts];
    int* diff_out_src = new int[inserts];
    int* diff_out_dst = new int[inserts];

    t1 = std::chrono::high_resolution_clock::now();
    while (fscanf(file, "%c%*[ \t]%d%*[ \t]%d%*[\n]", &op, &src, &dst) == 3) {
        if (op == 'a') {
            // printf("Line has %d -> %d\n",src,dst);
            ////////////////////////adding src//////////////////////
            if (insertsrc_avail[src] > 0) {
                int out_deg = out_degree(g, src);
                int* out_verts = out_vertices(g, src);
                for (int i = 0; i < out_deg; i++) {
                    if (out_verts[i] == -1) {
                        out_verts[i] = dst;
                        insertsrc_avail[src]--;
                        break;
                    }
                }
            } else {
                diff_out_src[extra_out] = src;
                diff_out_dst[extra_out] = dst;
                extra_out++;
            }

            if (scc_maps[src] != scc_maps[dst]) {
                scc_diff_src[extra_condense_count] = inverse_scc[scc_maps[src]];
            }
            ////////////////////////////////////////////////////////
            ////////////////////////adding dst/////////////////////

            if (insertdst_avail[dst] > 0) {
                int in_deg = in_degree(g, dst);
                int* in_verts = in_vertices(g, dst);
                for (int i = 0; i < in_deg; i++) {
                    if (in_verts[i] == -1) {
                        in_verts[i] = src;
                        insertdst_avail[dst]--;
                        break;
                    }
                }
            } else {
                diff_in_src[extra_in] = src;
                diff_in_dst[extra_in] = dst;
                extra_in++;
            }
            if (scc_maps[src] != scc_maps[dst]) {
                scc_diff_dst[extra_condense_count] = inverse_scc[scc_maps[dst]];
                extra_condense_count++;
            }
        }
    }
    t2 = std::chrono::high_resolution_clock::now();
    auto diff_ = std::chrono::duration<double, std::milli>(t2 - t1).count();
    std::cout << "edge insertion (processing) : " << diff_ << "ms\n";

    t1 = std::chrono::high_resolution_clock::now();
    graph condensed_g = { root_count, (unsigned int)condensed_edges,
        scc_outarr, scc_inarr,
        scc_outdegree_list, scc_indegree_list, 0, 0, 0, 0 };

    // int* placeholder1;
    // unsigned int* placeholder2;
    // int* diff_out_vert;
    // unsigned int* diff_outdeg_list;
    // int* diff_in_verts;
    // unsigned int* diff_indeg_list;
    unsigned int* scc_diff_outdeg_list;
    unsigned int* scc_diff_indeg_list;
    int* scc_diff_out;
    int* scc_diff_in;

    // create_csr(num_verts, extra_out, diff_out_src, diff_out_dst,
    //     diff_out_vert, placeholder1, diff_outdeg_list, placeholder2);
    //
    // delete[] placeholder1;
    // delete[] placeholder2;
    //
    // create_csr(num_verts, extra_in, diff_in_src, diff_in_dst, placeholder1,
    //     diff_in_verts, placeholder2, diff_indeg_list);
    //
    // delete[] placeholder1;
    // delete[] placeholder2;

    create_csr(root_count, extra_condense_count, scc_diff_src, scc_diff_dst,
        scc_diff_out, scc_diff_in, scc_diff_outdeg_list,
        scc_diff_indeg_list);

    // scc algorithm on condensed graph
    condensed_g.m = condensed_g.m + extra_condense_count;
    condense_scc_update(
        condensed_g, scc_maps, scc_index, inverse_scc, scc_diff_out, scc_diff_in,
        scc_diff_outdeg_list,
        scc_diff_indeg_list); // assuming this updates and gives us the result
    //////////////////////////////////

    for (int i = 0; i < num_verts; i++) {
        scc_maps[i] = scc_maps[scc_maps[i]];
    }
    t2 = std::chrono::high_resolution_clock::now();
    auto diff2 = std::chrono::duration<double, std::milli>(t2 - t1).count();
    std::cout << "edge insertion : " << diff2 << "ms\n";

    t1 = std::chrono::high_resolution_clock::now();
    // generating updated graph
    del_set updated_edges;
    int temp_i = 0;
    int temp_src = 0;
    int temp_dst = 0;

    while (temp_i < g.m) {
        temp_dst = g.out_array[temp_i];
        if ((temp_dst != -1) && (g.out_degree_list[temp_src + 1] > temp_i)) {
            updated_edges.insert(std::make_pair(temp_src, temp_dst));
            temp_i += 1;
        } else {
            temp_src += 1;
        }
    }

    temp_i = 0;
    temp_src = 0;
    temp_dst = 0;
    while (temp_i < g.m) {
        temp_src = g.in_array[temp_i];
        if ((temp_src != -1) && (g.in_degree_list[temp_dst + 1] > temp_i)) {
            updated_edges.insert(std::make_pair(temp_src, temp_dst));
            temp_i += 1;
        } else {
            temp_dst += 1;
        }
    }

    for (int i = 0; i < extra_out; i++) {
        updated_edges.insert(std::make_pair(diff_out_src[i], diff_out_dst[i]));
    }

    for (int i = 0; i < extra_in; i++) {
        updated_edges.insert(std::make_pair(diff_in_src[i], diff_in_dst[i]));
    }

    int* temp_srcs = new int[updated_edges.size()];
    int* temp_dsts = new int[updated_edges.size()];
    temp_i = 0;
    for (auto it : updated_edges) {
        temp_srcs[temp_i] = it.first;
        temp_dsts[temp_i] = it.second;
        temp_i += 1;
    }

    graph updated_g = generate(g.n, updated_edges.size(), temp_srcs, temp_dsts);
    updated_g.scc_map = new int[updated_g.n];
    std::copy(scc_maps, scc_maps + num_verts, updated_g.scc_map);

    clear_graph(g);
    g = updated_g;
    t2 = std::chrono::high_resolution_clock::now();
    auto diff3 = std::chrono::duration<double, std::milli>(t2 - t1).count();
    std::cout << "graph updation : " << diff3 << "ms\n";

    clear_graph(condensed_g);

    delete[] temp_dsts;
    delete[] temp_srcs;

    delete[] diff_out_src;
    delete[] diff_out_dst;
    delete[] diff_in_src;
    delete[] diff_in_dst;

    delete[] scc_diff_src;
    delete[] scc_diff_dst;

    delete[] scc_diff_outdeg_list;
    delete[] scc_diff_indeg_list;
    delete[] scc_diff_out;
    delete[] scc_diff_in;
    // delete[] diff_out_vert;
    // delete[] diff_in_verts;
    // delete[] diff_outdeg_list;
    // delete[] diff_indeg_list;
    delete[] inverse_scc;
    delete[] scc_index;
}

static void edge_deletion(graph& g, FILE* file, int* insertsrc_avail,
    int* insertdst_avail, int& inserts, del_set& deleted_edges)
{
    // get data structure needed for deletion, those trees
    // updating csr part
    int* out_queue; // to store the vertices whose out edges needs to be searched
    int* in_queue; // to store the vertices whose in edges needs to be searched
    std::vector<int> temp_out_q;
    std::vector<int> temp_in_q;
    char op;
    int src, dst, found;
    // deleting for diff csr
    auto t1 = std::chrono::high_resolution_clock::now();
    while (fscanf(file, "%c%*[ \t]%d%*[ \t]%d%*[\n]", &op, &src, &dst) == 3) {
        if (op == 'd') {
            deleted_edges.insert(std::make_pair(src, dst)); // takes care of deleted edges
            if (g.scc_map[dst] == g.scc_map[src]) { // if part of same scc, can lead to break down
                temp_out_q.push_back(dst);
                temp_in_q.push_back(src);
            }

            int out_deg = out_degree(g, src);
            int* out_verts = out_vertices(g, src);
            found = 0;
            for (int i = 0; i < out_deg; i++) {
                if (found) {
                    break; // might be useful later
                }
                if (out_verts[i] == dst) {
                    // out_verts[i] = -1; //will do it later as it causes issues
                    found = 1;
                    insertsrc_avail[src]++;
                }
            }

            // changing in "in" side
            int in_deg = in_degree(g, dst);
            int* in_verts = in_vertices(g, dst);
            found = 0;
            for (int i = 0; i < in_deg; i++) {
                if (found) {
                    break;
                }
                if (in_verts[i] == src) {
                    // in_verts[i] = -1; //will add later
                    found = 1;
                    insertdst_avail[dst]++;
                }
            }
        } else {
            inserts++;
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto diff1 = std::chrono::duration<double, std::milli>(t2 - t1).count();
    std::cout << "edge deletion(processing) : " << diff1 << "ms\n";

    t1 = std::chrono::high_resolution_clock::now();
    int tmp_q_size = temp_out_q.size();
    out_queue = new int[tmp_q_size];
    in_queue = new int[tmp_q_size];

    for (int i = 0; i < temp_out_q.size(); i++) {
        out_queue[i] = temp_out_q[i];
    }
    for (int i = 0; i < temp_in_q.size(); i++) {
        in_queue[i] = temp_in_q[i];
    }

    naive_delete(deleted_edges, g, out_queue, in_queue, temp_in_q.size());
    t2 = std::chrono::high_resolution_clock::now();
    diff1 = std::chrono::duration<double, std::milli>(t2 - t1).count();
    std::cout << "edge deletion : " << diff1 << "ms\n";
    delete[] out_queue;
    delete[] in_queue;
}

void dynamic(graph& g, int verts, char* u_file)
{
    int inserts = 0;
    del_set deleted_edges; // TO:DO define has functions to take care of deleted
    del_set insert_edges;

    FILE* file = fopen(u_file, "r");
    if (NULL == file) {
        printf("File unable to open\n");
        exit(0);
    }

    int* insertsrc_avail = new int[verts]; // array to count insertions available, might be useful
    // in insertion of edges
    int* insertdst_avail = new int[verts]; // same but for dsts
    for (int i = 0; i < verts; i++) {
        insertsrc_avail[i] = 0;
        insertdst_avail[i] = 0;
    }

    edge_deletion(g, file, insertsrc_avail, insertdst_avail, inserts,
        deleted_edges);

    fseek(file, 0, SEEK_SET);

    // printf("insertions : %d, deletions: %d\n", insertions, deletions);
    edge_insertion(g, file, insertsrc_avail, insertdst_avail, inserts, verts,
        g.scc_map, deleted_edges);

    delete[] insertsrc_avail;
    delete[] insertdst_avail;
}
