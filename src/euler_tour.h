#ifndef EULER_TOUR_H
#define EULER_TOUR_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "treap.h"

/* We represent each directed edge and each vertex visit as a node in the Treap. */
typedef struct ett_node {
    TREAP_ENTRY(ett_node) link;
    int u, v; /* The directed edge (u, v). If u == v, it's a vertex visit. */
    size_t size;
} ett_node;

typedef struct euler_tour {
    TREAP_HEAD(ett_treap, ett_node) trt;
    size_t max_vertices;
    ett_node **vertex_nodes; /* Pointer to the first visit node of each vertex */
} euler_tour;

void euler_tour_init(euler_tour *et, size_t max_vertices);
void euler_tour_destroy(euler_tour *et);

/* Graph operations */
void euler_tour_link(euler_tour *et, int u, int v);
void euler_tour_cut(euler_tour *et, int u, int v);
int euler_tour_connected(euler_tour *et, int u, int v);

void euler_tour_graph(euler_tour *et, FILE *stream);

#endif
