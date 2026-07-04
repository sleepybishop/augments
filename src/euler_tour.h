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

typedef struct edge_map_node {
    TREAP_ENTRY(edge_map_node) link;
    uint64_t key;
    ett_node *ptr;
} edge_map_node;

TREAP_HEAD(edge_map_treap, edge_map_node);

typedef struct euler_tour {
    TREAP_HEAD(ett_treap, ett_node) trt;
    size_t max_vertices;
    ett_node **vertex_nodes;
    struct edge_map_treap edge_map;
    uint64_t prng_state[2];
} euler_tour;

void euler_tour_init(euler_tour *et, size_t max_vertices, ett_node **vertex_nodes_buf);

/* Graph operations */
void euler_tour_link(euler_tour *et, int u, int v, ett_node *uv_node, ett_node *vu_node, edge_map_node *uv_map, edge_map_node *vu_map);
void euler_tour_cut(euler_tour *et, int u, int v);
int euler_tour_connected(euler_tour *et, int u, int v);

void euler_tour_graph(euler_tour *et, FILE *stream);

#endif
