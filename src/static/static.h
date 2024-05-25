#ifndef STATIC_H
#define STATIC_H

#include "graph.h"

void run_scc(graph &g, int *&scc_maps, int max_deg_vert, double avg_degree,
             int vert_cutoff);

#endif
