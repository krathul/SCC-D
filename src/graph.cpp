#include "graph.h"

#include <algorithm>
#include <stdexcept>
#include <stdio.h>
#include <unordered_map>

static void common_read_edge(char* filename, unsigned& m, int*& srcs,
    int*& dsts)
{
    FILE* file = fopen(filename, "r");
    int src, dest;
    unsigned counter = 0;

    srcs = new int[m];
    dsts = new int[m];
    while (fscanf(file, "%d%*[ \t]%d", &src, &dest) == 2) {

        srcs[counter] = src;
        dsts[counter] = dest;
        ++counter;
    }

    fclose(file);
}

static void create_csr(int n, unsigned m, int* srcs, int* dsts, int*& out_array,
    int*& in_array, unsigned*& out_degree_list,
    unsigned*& in_degree_list, int& max_deg_vert,
    double& avg_degree)
{
    out_array = new int[m];
    in_array = new int[m];
    out_degree_list = new unsigned[n + 1];
    in_degree_list = new unsigned[n + 1];

    for (unsigned i = 0; i < m; ++i)
        out_array[i] = 0;
    for (unsigned i = 0; i < m; ++i)
        in_array[i] = 0;
    for (int i = 0; i < n + 1; ++i)
        out_degree_list[i] = 0;
    for (int i = 0; i < n + 1; ++i)
        in_degree_list[i] = 0;

    unsigned* temp_counts = new unsigned[n];
    for (int i = 0; i < n; ++i)
        temp_counts[i] = 0;
    for (unsigned i = 0; i < m; ++i)
        ++temp_counts[srcs[i]];
    for (int i = 0; i < n; ++i)
        out_degree_list[i + 1] = out_degree_list[i] + temp_counts[i];
    std::copy(out_degree_list, out_degree_list + n, temp_counts);
    for (unsigned i = 0; i < m; ++i)
        out_array[temp_counts[srcs[i]]++] = dsts[i];

    for (int i = 0; i < n; ++i)
        temp_counts[i] = 0;
    for (unsigned i = 0; i < m; ++i)
        ++temp_counts[dsts[i]];
    for (int i = 0; i < n; ++i)
        in_degree_list[i + 1] = in_degree_list[i] + temp_counts[i];
    std::copy(in_degree_list, in_degree_list + n, temp_counts);
    for (unsigned i = 0; i < m; ++i)
        in_array[temp_counts[dsts[i]]++] = srcs[i];
    delete[] temp_counts;

    avg_degree = 0.0;
    double max_degree = 0.0;
    for (int i = 0; i < n; ++i) {
        unsigned out_degree = out_degree_list[i + 1] - out_degree_list[i];
        unsigned in_degree = in_degree_list[i + 1] - in_degree_list[i];
        double degree = (double)out_degree * (double)in_degree;
        avg_degree += (double)out_degree;
        if (degree > max_degree) {
            max_deg_vert = i;
            max_degree = degree;
        }
    }
    avg_degree /= (double)n;
}

graph generate(int n_vertices, unsigned n_edges, int* srcs, int* dsts)
{
    int* out_array;
    int* in_array;

    int n = n_vertices;
    unsigned m = n_edges;

    unsigned* out_degree_list;
    unsigned* in_degree_list;

    create_csr(n, m, srcs, dsts, out_array, in_array, out_degree_list,
        in_degree_list);

    graph g = {
        n, m, out_array, in_array, out_degree_list, in_degree_list,
        0, NULL, NULL, NULL
    };

    return g;
}

graph generate(char* file_name, int n_vertices, unsigned n_edges, int& max_deg_vert, double& avg_degree)
{
    int* srcs;
    int* dsts;
    int* out_array;
    int* in_array;

    int n = n_vertices;
    unsigned m = n_edges;

    unsigned* out_degree_list;
    unsigned* in_degree_list;

    common_read_edge(file_name, m, srcs, dsts);

    create_csr(n, m, srcs, dsts, out_array, in_array, out_degree_list,
        in_degree_list, max_deg_vert, avg_degree);

    graph g = {
        n, m, out_array, in_array, out_degree_list, in_degree_list,
        0, NULL, NULL, NULL
    }; // keeping NULL at start to create the obejct

    delete[] srcs;
    delete[] dsts;

    return g;
}

void create_csr(int n, unsigned m, int* srcs, int* dsts, int*& out_array,
    int*& in_array, unsigned*& out_degree_list,
    unsigned*& in_degree_list)
{
    out_array = new int[m];
    in_array = new int[m];
    out_degree_list = new unsigned[n + 1];
    in_degree_list = new unsigned[n + 1];

    for (unsigned i = 0; i < m; ++i)
        out_array[i] = 0;
    for (unsigned i = 0; i < m; ++i)
        in_array[i] = 0;
    for (int i = 0; i < n + 1; ++i)
        out_degree_list[i] = 0;
    for (int i = 0; i < n + 1; ++i)
        in_degree_list[i] = 0;

    unsigned* temp_counts = new unsigned[n];
    for (int i = 0; i < n; ++i)
        temp_counts[i] = 0;
    for (unsigned i = 0; i < m; ++i)
        ++temp_counts[srcs[i]];
    for (int i = 0; i < n; ++i)
        out_degree_list[i + 1] = out_degree_list[i] + temp_counts[i];
    std::copy(out_degree_list, out_degree_list + n, temp_counts);
    for (unsigned i = 0; i < m; ++i)
        out_array[temp_counts[srcs[i]]++] = dsts[i];

    for (int i = 0; i < n; ++i)
        temp_counts[i] = 0;
    for (unsigned i = 0; i < m; ++i)
        ++temp_counts[dsts[i]];
    for (int i = 0; i < n; ++i)
        in_degree_list[i + 1] = in_degree_list[i] + temp_counts[i];
    std::copy(in_degree_list, in_degree_list + n, temp_counts);
    for (unsigned i = 0; i < m; ++i)
        in_array[temp_counts[dsts[i]]++] = srcs[i];
    delete[] temp_counts;
}

void update_g_with_scc(graph& g)
{
    int scc_count = 0;
    int* scc_map = g.scc_map;
    int* rep;
    int* counts;
    int curr_index = 0;

    std::unordered_map<int, int> temp_indexes;
    for (int i = 0; i < g.n; i++) {
        if (temp_indexes.find(scc_map[i]) == temp_indexes.end()) {
            temp_indexes[scc_map[i]] = curr_index++;
            scc_count++;
        }
    }

    rep = new int[g.n];
    counts = new int[g.n];

    for (int i = 0; i < g.n; i++) {
        counts[i] = 0;
    }
    for (int i = 0; i < g.n; i++) {
        rep[temp_indexes[scc_map[i]]] = scc_map[i];
        counts[temp_indexes[scc_map[i]]]++;
    }
    g.scc_count = scc_count;
    g.rep_nodes = rep;
    g.count_in_sccs = counts;
}

int find_index(int to_find, int* arr, int arr_siz)
{
    int index = -1;
    for (int i = 0; i < arr_siz; i++) {
        if (arr[i] == to_find) {
            index = i;
            break;
        }
    }
    if (-1 == index) {
        printf("couldnt not find scc_node %d in the list\n", to_find);
        printf("Printing the scc list\n");
        for (int i = 0; i < arr_siz; i++) {
            printf("%d\n", arr[i]);
        }
        printf("\n\n");
        throw std::runtime_error("Did not match any index, throwing error");
    }
    return index;
}

void clear_graph(graph& g)
{
    delete[] g.out_array;
    delete[] g.in_array;
    delete[] g.out_degree_list;
    delete[] g.in_degree_list;
    delete[] g.scc_map;
    delete[] g.rep_nodes;
    delete[] g.count_in_sccs;
}
