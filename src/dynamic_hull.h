#ifndef DYNAMIC_HULL_H
#define DYNAMIC_HULL_H

#include "treap.h"
#include <stdio.h>

typedef struct dyn_hull_node {
    TREAP_ENTRY(dyn_hull_node) link;
    double x, y;
    struct dyn_hull_node *max_x_node;
    struct dyn_hull_node *min_x_node;

    int is_swallowed;
    struct dyn_hull_node *L_tangent;
    struct dyn_hull_node *R_tangent;
} dyn_hull_node;

typedef struct dyn_hull_tree {
    TREAP_HEAD(dyn_hull_treap, dyn_hull_node) trt;
} dyn_hull_tree;

void dyn_hull_init(dyn_hull_tree *tree);
void dyn_hull_insert(dyn_hull_tree *tree, double x, double y);
void dyn_hull_remove(dyn_hull_tree *tree, double x, double y);
void dyn_hull_destroy(dyn_hull_tree *tree);
void dyn_hull_query(dyn_hull_tree *tree, FILE *stream);

void dyn_hull_graph(dyn_hull_tree *tree, FILE *stream);

#endif
