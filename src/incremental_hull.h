#ifndef INCREMENTAL_HULL_H
#define INCREMENTAL_HULL_H

#include <stdio.h>
#include "tree.h"

typedef struct inc_hull_node {
    RB_ENTRY(inc_hull_node) link;
    double x;
    double y;
} inc_hull_node;

typedef struct inc_hull_tree {
    RB_HEAD(inc_hull_rb, inc_hull_node) rbt;
} inc_hull_tree;

int inc_hull_cmp(inc_hull_node *a, inc_hull_node *b);
RB_PROTOTYPE(inc_hull_rb, inc_hull_node, link, inc_hull_cmp)

void inc_hull_init(inc_hull_tree *tree);
void inc_hull_insert(inc_hull_tree *tree, inc_hull_node *node, double x, double y);
void inc_hull_destroy(inc_hull_tree *tree);
void inc_hull_graph(inc_hull_tree *tree, FILE *stream);

#endif
