#ifndef RANGE_SUM_H
#define RANGE_SUM_H

#include <stddef.h>
#include <stdio.h>
#include "tree.h"

/*
 * Range Sum tree implementation.
 * Augments a red-black tree with a 'sum' field in each node,
 * representing the sum of 'val' in the subtree.
 */

typedef struct sum_node {
    int key;
    double val;
    double sum;
    RB_ENTRY(sum_node) link;
} sum_node;

typedef struct {
    RB_HEAD(_SUM_TREE, sum_node) rbt;
} sum_tree;

void sum_tree_init(sum_tree *tree);
int sum_tree_add(sum_tree *tree, int key, double val);
int sum_tree_remove(sum_tree *tree, int key);
void sum_tree_update(sum_tree *tree, int key, double new_val);
double sum_tree_query(sum_tree *tree, int low_key, int high_key);
sum_node *sum_tree_find(sum_tree *tree, int key);
void sum_tree_destroy(sum_tree *tree);
void sum_tree_graph(sum_tree *tree, FILE *stream);

#endif /* RANGE_SUM_H */
