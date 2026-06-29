#ifndef HASHTREE_H
#define HASHTREE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct hash_tree_node {
    struct _HASHTREE_TREE {
        struct hash_tree_node *rbe_left;
        struct hash_tree_node *rbe_right;
        struct hash_tree_node *rbe_parent;
        int rbe_color;
    } link;
    size_t key;
    char val;

    /* Augmentation fields */
    size_t size;
    uint64_t hash;
    uint64_t b_pow;
} hash_tree_node;

typedef struct hash_tree_tree {
    struct _HASHTREE {
        struct hash_tree_node *rbh_root;
    } rbt;
} hash_tree_tree;

void hash_tree_init(hash_tree_tree *tree);
int hash_tree_insert(hash_tree_tree *tree, size_t key, char val);
int hash_tree_remove(hash_tree_tree *tree, size_t key);
hash_tree_node *hash_tree_find(hash_tree_tree *tree, size_t key);
uint64_t hash_tree_hash(hash_tree_tree *tree);
void hash_tree_destroy(hash_tree_tree *tree);
void hash_tree_graph(hash_tree_tree *tree, FILE *stream);

#endif
