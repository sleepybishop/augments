#ifndef RANGE_MIN_H
#define RANGE_MIN_H

#include <stddef.h>
#include <stdio.h>
#include "tree.h"

/*
 * Range Minimum tree implementation.
 * Augments a red-black tree with a 'min_val' field in each node,
 * representing the minimum of 'val' in the subtree.
 */

typedef struct min_node {
    int key;
    double val;
    double min_val;
    RB_ENTRY(min_node) link;
} min_node;

typedef struct {
    RB_HEAD(_MIN_TREE, min_node) rbt;
} min_tree;

void min_tree_init(min_tree *tree);
int min_tree_add(min_tree *tree, int key, double val);
int min_tree_remove(min_tree *tree, int key);
void min_tree_update(min_tree *tree, int key, double new_val);
double min_tree_query(min_tree *tree, int low_key, int high_key);
min_node *min_tree_find(min_tree *tree, int key);
void min_tree_destroy(min_tree *tree);
void min_tree_graph(min_tree *tree, FILE *stream);

#endif /* RANGE_MIN_H */
