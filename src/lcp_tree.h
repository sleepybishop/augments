#ifndef LCP_TREE_H
#define LCP_TREE_H

#include <stddef.h>
#include <stdio.h>

typedef struct lcp_node {
    struct _LCP_TREE {
        struct lcp_node *rbe_left;
        struct lcp_node *rbe_right;
        struct lcp_node *rbe_parent;
        int rbe_color;
    } link;
    char *str;

    /* Augmentation */
    char *min_str;
    char *max_str;
    size_t lcp;
} lcp_node;

typedef struct lcp_tree {
    struct _LCP {
        struct lcp_node *rbh_root;
    } rbt;
} lcp_tree;

void lcp_tree_init(lcp_tree *tree);
int lcp_tree_add(lcp_tree *tree, lcp_node *node, const char *str);
int lcp_tree_remove(lcp_tree *tree, const char *str);
size_t lcp_tree_query(lcp_tree *tree);
void lcp_tree_destroy(lcp_tree *tree);
void lcp_tree_graph(lcp_tree *tree, FILE *stream);

#endif
