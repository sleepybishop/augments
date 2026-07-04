#ifndef MAX_SUBARRAY_H
#define MAX_SUBARRAY_H

#include <stddef.h>
#include <stdio.h>
#include "tree.h"

/*
 * Max Subarray Sum tree implementation.
 * Each node has a key and a val.
 * Augments the red-black tree with:
 * - sum: total sum of values in the subtree.
 * - max_prefix: maximum prefix sum of the subtree.
 * - max_suffix: maximum suffix sum of the subtree.
 * - max_sub: maximum subarray sum of the subtree.
 * Enables O(log n) queries for the maximum subarray sum in any range [low_key, high_key].
 */

typedef struct sub_node {
    int key;
    double val;
    double sum;
    double max_prefix;
    double max_suffix;
    double max_sub;
    RB_ENTRY(sub_node) link;
} sub_node;

typedef struct {
    RB_HEAD(_SUB_TREE, sub_node) rbt;
} sub_tree;

void sub_tree_init(sub_tree *tree);
int sub_tree_add(sub_tree *tree, sub_node *node, int key, double val);
int sub_tree_remove(sub_tree *tree, int key);
void sub_tree_update(sub_tree *tree, int key, double new_val);
double sub_tree_query(sub_tree *tree, int low_key, int high_key);
sub_node *sub_tree_find(sub_tree *tree, int key);
void sub_tree_destroy(sub_tree *tree);
void sub_tree_graph(sub_tree *tree, FILE *stream);

#endif /* MAX_SUBARRAY_H */
