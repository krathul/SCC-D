#include "graph.h"
#include "scc_serial.h"

#include <omp.h>

const int TRIM_LEVEL = 1;
const int THREAD_QUEUE_SIZE = 2048;
const float ALPHA = 15.0;
const int BETA = 25;


#include "scc_trim.h"
#include "scc_fwbw.h"
#include "scc_color.h"
// #include "scc_verify.h"

void run_scc(graph &g, int *&scc_maps, int max_deg_vert, double avg_degree,
             int vert_cutoff) {

    int num_trim = 0;
    int num_fwbw = 0;
    int num_color = 0;
    int num_serial = 0;

    scc_maps = new int[g.n];
    bool *valid = new bool[g.n];
    int *valid_verts = new int[g.n];
    int num_valid = g.n;

    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < g.n; ++i)
            valid[i] = true;
        #pragma omp for nowait
        for (int i = 0; i < g.n; ++i)
            scc_maps[i] = -1;
        #pragma omp for nowait
        for (int i = 0; i < g.n; ++i)
            valid_verts[i] = i;
    }

    if (TRIM_LEVEL == 0) {
        /*  num_trim = scc_trim_none(g, valid,
         *       valid_verts, num_valid,
         *       scc_maps);*/
    } else if (TRIM_LEVEL == 1) {
        num_trim = scc_trim(g, valid, valid_verts, num_valid, scc_maps);
    } else if (TRIM_LEVEL == 2) {
        num_trim = scc_trim_complete(g, valid, valid_verts, num_valid, scc_maps);
        num_valid = 0;
        for (int i = 0; i < g.n; ++i)
            if (valid[i])
                valid_verts[num_valid++] = i;
    }

    // scc_verify(g, scc_maps);

    num_fwbw = scc_fwbw(g, valid, valid_verts, num_valid, max_deg_vert,
                        avg_degree, scc_maps);

    // scc_verify(g, scc_maps);

    num_color =
    scc_color(g, valid, valid_verts, num_valid, vert_cutoff, scc_maps);

    // scc_verify(g, scc_maps);

    tarjan t(g, valid, valid_verts, num_valid, scc_maps);
    num_serial = t.run_valid();

    delete[] valid;
    delete[] valid_verts;
}
