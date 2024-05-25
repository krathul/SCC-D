#ifndef GRAPH_H
#define GRAPH_H

#include <unordered_set>
#include <utility>

#define out_degree(g, n) (g.out_degree_list[n + 1] - g.out_degree_list[n])
#define in_degree(g, n) (g.in_degree_list[n + 1] - g.in_degree_list[n])
#define out_vertices(g, n) &g.out_array[g.out_degree_list[n]]
#define in_vertices(g, n) &g.in_array[g.in_degree_list[n]]

struct pair_hash {
    inline std::size_t operator()(const std::pair<int, int>& v) const
    {
        return v.first * 31 + v.second;
    }
};

typedef std::unordered_set<std::pair<int, int>, pair_hash> del_set;

struct graph {
public:
    int n;
    unsigned m;
    int* out_array;
    int* in_array;
    unsigned* out_degree_list;
    unsigned* in_degree_list;
    int scc_count; // no.of scc's
    int* scc_map;
    int* rep_nodes; // array keeping count in
    int* count_in_sccs; //
};

graph generate(char* file_name, int n_vertices, unsigned n_edges, int& max_deg_vert, double& avg_degree);

graph generate(int n_vertices, unsigned n_edges, int* srcs, int* dsts);

void update_g_with_scc(graph& g);

int find_index(int to_find, int* arr, int arr_siz);

void clear_graph(graph& g);

void create_csr(int n, unsigned m, int* srcs, int* dsts, int*& out_array,
    int*& in_array, unsigned*& out_degree_list,
    unsigned*& in_degree_list);

#endif
