#ifndef ORDER_STATISTIC_H
#define ORDER_STATISTIC_H

#include <stddef.h>
#include <stdio.h>
#include "tree.h"

/*
 * Order-Statistic tree implementation.
 * Augments a red-black tree with a 'size' field in each node,
 * representing the number of nodes in the subtree.
 */

typedef struct os_node {
    int key;
    size_t size;
    RB_ENTRY(os_node) link;
} os_node;

typedef struct {
    RB_HEAD(_OS_TREE, os_node) rbt;
} os_tree;

void os_tree_init(os_tree *tree);
int os_tree_add(os_tree *tree, int key);
int os_tree_remove(os_tree *tree, int key);
os_node *os_tree_select(os_tree *tree, size_t rank);
size_t os_tree_rank(os_tree *tree, int key);
os_node *os_tree_find(os_tree *tree, int key);
void os_tree_destroy(os_tree *tree);
void os_tree_graph(os_tree *tree, FILE *stream);

#endif /* ORDER_STATISTIC_H */
