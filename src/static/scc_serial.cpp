#include "scc_serial.h"
#include "graph.h"

#include <omp.h>

tarjan::tarjan(graph& g)
{
    int num_verts = g.n;

    indices = new int[num_verts];
    lowlinks = new int[num_verts];
    in_stack = new bool[num_verts];
    stack = new int[num_verts];
    stack_pos = 0;
    for (int i = 0; i < num_verts; ++i)
        indices[i] = NULL_VAL;
    for (int i = 0; i < num_verts; ++i)
        lowlinks[i] = NULL_VAL;
    for (int i = 0; i < num_verts; ++i)
        in_stack[i] = false;

    index = 0;
    local_scc_count = 0;
}

tarjan::tarjan(graph& g_in, bool* valid_in,
               int* valid_verts_in, int num_valid_in,
               int* scc_maps_in)
{
    g = g_in;
    valid = valid_in;
    valid_verts = valid_verts_in;
    num_valid = num_valid_in;
    scc_maps = scc_maps_in;

    int num_verts = g.n;
    indices = new int[num_verts];
    lowlinks = new int[num_verts];
    in_stack = new bool[num_verts];
    #pragma omp parallel for
    for (int i = 0; i < num_valid; ++i)
        indices[valid_verts[i]] = NULL_VAL;
    #pragma omp parallel for
    for (int i = 0; i < num_valid; ++i)
        lowlinks[valid_verts[i]] = NULL_VAL;
    #pragma omp parallel for
    for (int i = 0; i < num_valid; ++i)
        in_stack[valid_verts[i]] = false;

    stack = new int[num_verts];
    stack_pos = 0;

    index = 0;
    local_scc_count = 0;
}

tarjan::~tarjan()
{
    delete [] indices;
    delete [] lowlinks;
    delete [] in_stack;
    delete [] stack;
}

int tarjan::run()
{
    int num_verts = g.n;
    for (int v = 0; v < num_verts; ++v)
    {
        if (indices[v] == NULL_VAL)
        {
            scc_serial(v);
        }
    }

    return local_scc_count;
}

int tarjan::run_valid()
{
    for (int i = 0; i < num_valid; ++i)
    {
        int vert = valid_verts[i];
        if (indices[vert] == NULL_VAL)
        {
            scc_serial_valid(vert);
        }
    }

    return local_scc_count;
}


void tarjan::tarjan::scc_serial(int vert)
{
    indices[vert] = index;
    lowlinks[vert] = index;
    ++index;

    stack[stack_pos++] = vert;
    in_stack[vert] = true;

    int out_degree = out_degree(g, vert);
    int* outs = out_vertices(g, vert);
    for(int i = 0; i < out_degree; ++i)
    {
        int out = outs[i];

        if(indices[out] == NULL_VAL)
        {
            scc_serial(out);
            lowlinks[vert] = lowlinks[vert] < lowlinks[out] ? lowlinks[vert] : lowlinks[out];
        }
        else if(in_stack[out])
        {
            lowlinks[vert] = lowlinks[vert] < indices[out] ? lowlinks[vert] : indices[out];
        }
    }

    if(lowlinks[vert] == indices[vert])
    {
        scc_maps[vert] = vert;
        int neighbor = NULL_VAL;
        while(vert != neighbor)
        {
            neighbor = stack[--stack_pos];
            scc_maps[neighbor] = vert;
            in_stack[neighbor] = false;
        }

        ++local_scc_count;
    }
}

void tarjan::scc_serial_valid(int vert)
{
    indices[vert] = index;
    lowlinks[vert] = index;
    ++index;

    stack[stack_pos++] = vert;
    in_stack[vert] = true;

    int out_degree = out_degree(g, vert);
    int* outs = out_vertices(g, vert);
    for(int i = 0; i < out_degree; ++i)
    {
        int out = outs[i];

        if (valid[out])
        {
            if(indices[out] == NULL_VAL)
            {
                scc_serial_valid(out);
                lowlinks[vert] = lowlinks[vert] < lowlinks[out] ? lowlinks[vert] : lowlinks[out];
            }
            else if(in_stack[out])
            {
                lowlinks[vert] = lowlinks[vert] < indices[out] ? lowlinks[vert] : indices[out];
            }
        }
    }

    if(lowlinks[vert] == indices[vert])
    {
        scc_maps[vert] = vert;
        int neighbor = NULL_VAL;
        while(vert != neighbor)
        {
            neighbor = stack[--stack_pos];
            scc_maps[neighbor] = vert;
            in_stack[neighbor] = false;
        }

        ++local_scc_count;
    }
}
