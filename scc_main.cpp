#include <cstdio>
using namespace std;

#include "graph.h"
#include "scc_verify.h"
#include "static.h"

#ifndef STATIC_MODE
#include "dynamic.h"
#endif

#include <assert.h>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <omp.h>
#include <sys/time.h>

int main(int argc, char** argv)
{
    if (argc < 4) {
        printf("graph_file, vertices, edges [optional thread_no] \n");
        exit(0);
    }
    int threads = 4;
    if (argc == 5) {
        threads = atoi(argv[4]);
    }
    omp_set_num_threads(threads);

    int n = atoi(argv[2]);
    unsigned m = unsigned(atoi(argv[3]));
    int* scc_maps;
    int max_deg_vert;
    double avg_degree;

    int vert_cutoff = 10000;

    graph g = generate(argv[1], n, m, max_deg_vert, avg_degree);
    auto t1 = std::chrono::high_resolution_clock::now();
    run_scc(g, scc_maps, max_deg_vert, avg_degree, vert_cutoff);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto diff1 = std::chrono::duration<double, std::milli>(t2 - t1).count();
    std::cout << "static : " << diff1 << "ms\n";
    g.scc_map = scc_maps;
    update_g_with_scc(g);
    // scc_verify(g, g.scc_map);
    // printf("%d static\n", g.scc_count);
    // printf("%d no of vertices\n", g.n);
    // printf("%d no of edges\n", g.m);

    //////////////////////////////////////////
    //////////////////////////////////////////

#ifndef STATIC_MODE
    char update_file[50];
    while (scanf("%s", update_file) == 1) {
        dynamic(g, g.n, update_file);
        update_g_with_scc(g);

        printf("\n");
        // scc_verify(g, g.scc_map);
        // printf("%d dynamic\n", g.scc_count);
        // printf("%d no of vertices\n", g.n);
        // printf("%d no of edges\n", g.m);
    }
#endif
    clear_graph(g);

    return 0;
}
