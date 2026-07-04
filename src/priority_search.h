#ifndef PRIORITY_SEARCH_H
#define PRIORITY_SEARCH_H

#include <stddef.h>
#include <stdio.h>
#include "tree.h"

/*
 * Priority Search tree implementation.
 * Stores 2D points (x, y) ordered by x (as BST key).
 * Augments each node with the maximum y-coordinate in its subtree
 * and a pointer to the node holding that maximum.
 * Enables 3-sided range queries: [x_min, x_max] and y >= y_min.
 */

typedef struct ps_node {
    double x;
    double y;
    double max_y;
    struct ps_node *max_y_node;
    RB_ENTRY(ps_node) link;
} ps_node;

typedef struct {
    RB_HEAD(_PS_TREE, ps_node) rbt;
} ps_tree;

void ps_tree_init(ps_tree *tree);
int ps_tree_add(ps_tree *tree, ps_node *node, double x, double y);
int ps_tree_remove(ps_tree *tree, double x, double y);
typedef void (*ps_cb)(ps_node *node, void *arg);
void ps_tree_query(ps_tree *tree, double x_min, double x_max, double y_min, ps_cb cb, void *arg);
ps_node *ps_tree_find(ps_tree *tree, double x, double y);
void ps_tree_destroy(ps_tree *tree);
void ps_tree_graph(ps_tree *tree, FILE *stream);

#endif /* PRIORITY_SEARCH_H */
