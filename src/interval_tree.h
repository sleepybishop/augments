#ifndef INTERVAL_TREE_H
#define INTERVAL_TREE_H

#include <stddef.h>
#include <stdio.h>
#include "tree.h"

/*
 * Interval tree implementation.
 * Augments a red-black tree with a 'max' field in each node,
 * representing the maximum high endpoint in the subtree.
 */

typedef struct itree_node {
    size_t low;
    size_t high;
    size_t max;
    size_t id;
    RB_ENTRY(itree_node) link;
} itree_node;

typedef struct {
    RB_HEAD(_INTERVAL_TREE, itree_node) rbt;
} itree;

void interval_tree_init(itree *tree);
int interval_tree_add(itree *tree, itree_node *node, size_t low, size_t high, size_t id);
int interval_tree_remove(itree *tree, size_t low, size_t high, size_t id);
/* Callback returns 0 to continue search, non-zero to stop */
typedef int (*itree_cb)(itree_node *node, void *arg);
void interval_tree_find(itree *tree, size_t low, size_t high, int overlaps, itree_cb cb, void *arg);
void interval_tree_destroy(itree *tree);
void interval_tree_graph(itree *tree, FILE *stream);

#endif /* INTERVAL_TREE_H */
