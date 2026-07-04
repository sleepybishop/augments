#include "hash_tree.h"
#include <stdlib.h>
#include <stdio.h>

static void hash_tree_augment(hash_tree_node *node);
#define RB_AUGMENT(x) hash_tree_augment(x)
#include "tree.h"
#include <stdlib.h>
#include <stdio.h>

#define B_CONST 313ULL
#define M_CONST 1000000007ULL

static inline int hash_tree_cmp(hash_tree_node *a, hash_tree_node *b)
{
    if (a->key < b->key)
        return -1;
    if (a->key > b->key)
        return 1;
    return 0;
}

static void hash_tree_augment(hash_tree_node *node)
{
    while (node) {
        size_t s = 1;
        uint64_t l_hash = 0, r_hash = 0;
        uint64_t l_b = 1, r_b = 1;

        if (RB_LEFT(node, link)) {
            s += RB_LEFT(node, link)->size;
            l_hash = RB_LEFT(node, link)->hash;
            l_b = RB_LEFT(node, link)->b_pow;
        }

        if (RB_RIGHT(node, link)) {
            s += RB_RIGHT(node, link)->size;
            r_hash = RB_RIGHT(node, link)->hash;
            r_b = RB_RIGHT(node, link)->b_pow;
        }

        uint64_t val_hash = ((uint64_t)node->val * r_b) % M_CONST;
        uint64_t left_shifted = (l_hash * ((r_b * B_CONST) % M_CONST)) % M_CONST;

        uint64_t h = (left_shifted + val_hash + r_hash) % M_CONST;
        uint64_t bp = (((l_b * B_CONST) % M_CONST) * r_b) % M_CONST;

        if (node->size == s && node->hash == h && node->b_pow == bp)
            break;

        node->size = s;
        node->hash = h;
        node->b_pow = bp;

        node = RB_PARENT(node, link);
    }
}

RB_GENERATE(_HASHTREE, hash_tree_node, link, hash_tree_cmp)

void hash_tree_init(hash_tree_tree *tree)
{
    if (!tree)
        return;
        RB_INIT(&tree->rbt);
}

int hash_tree_insert(hash_tree_tree *tree, hash_tree_node *node, size_t key, char val)
{
            node->key = key;
    node->val = val;
    node->size = 1;
    node->hash = (uint64_t)val % M_CONST;
    node->b_pow = B_CONST;

    hash_tree_node *res = RB_INSERT(_HASHTREE, &tree->rbt, node);
    if (res != NULL) {
                res->val = val;
        hash_tree_augment(res);
        return 0;
    }
    return 0;
}

int hash_tree_remove(hash_tree_tree *tree, size_t key)
{
    hash_tree_node search = {.key = key};
    hash_tree_node *node = RB_FIND(_HASHTREE, &tree->rbt, &search);
    if (node) {
        RB_REMOVE(_HASHTREE, &tree->rbt, node);
                return 1;
    }
    return 0;
}

hash_tree_node *hash_tree_find(hash_tree_tree *tree, size_t key)
{
    hash_tree_node search = {.key = key};
    return RB_FIND(_HASHTREE, &tree->rbt, &search);
}

uint64_t hash_tree_hash(hash_tree_tree *tree)
{
    hash_tree_node *root = RB_ROOT(&tree->rbt);
    return root ? root->hash : 0;
}

static void destroy_node(hash_tree_tree *tree, hash_tree_node *node)
{
    while (node) {
        if (RB_LEFT(node, link)) {
            hash_tree_node *l = RB_LEFT(node, link);
            RB_LEFT(node, link) = RB_RIGHT(l, link);
            RB_RIGHT(l, link) = node;
            node = l;
        } else {
            hash_tree_node *r = RB_RIGHT(node, link);
                        node = r;
        }
    }
}

void hash_tree_destroy(hash_tree_tree *tree)
{
    if (tree) {
        destroy_node(tree, RB_ROOT(&tree->rbt));
        RB_INIT(&tree->rbt);
    }
}

static void graph_node(hash_tree_node *node, FILE *f)
{
        fprintf(f, "  \"node%p\" [fillcolor=\"%s\", label=\"{{ key: %zu | val: '%c' } | { size: %zu | hash: %llu }}\"];\n",
            (void *)node, RB_COLOR(node, link) == RB_BLACK ? "#000000" : "#aa0000", node->key, node->val, node->size,
            (unsigned long long)node->hash);

    if (RB_LEFT(node, link)) {
        fprintf(f, "  \"node%p\" -> \"node%p\" [label=\"L\"];\n", (void *)node, (void *)RB_LEFT(node, link));
        graph_node(RB_LEFT(node, link), f);
    }
    if (RB_RIGHT(node, link)) {
        fprintf(f, "  \"node%p\" -> \"node%p\" [label=\"R\"];\n", (void *)node, (void *)RB_RIGHT(node, link));
        graph_node(RB_RIGHT(node, link), f);
    }
}

void hash_tree_graph(hash_tree_tree *tree, FILE *stream)
{
    if (!stream)
        stream = stdout;
    if (tree) {
        fprintf(stream, "digraph {\n");
        fprintf(stream,
                "graph [ranksep=\"0.25\", rankdir=\"TB\", fontname=\"Monofur\", bgcolor=\"transparent\", nodesep=\"0.125\"];\n");
        fprintf(stream, "node [fontname=\"Monofur\", color=\"#cccccc\", style=\"filled\", height=\"0\", width=\"1\", "
                        "shape=\"Mrecord\", fontcolor=\"#ffffff\"];\n");
        fprintf(stream,
                "edge [fontname=\"Monofur\", color=\"#cccccc\", arrowsize=\"0.80\", penwidth=\"2.0\", fontcolor=\"#cccccc\"];\n");

        if (RB_ROOT(&tree->rbt)) {
            graph_node(RB_ROOT(&tree->rbt), stream);
        }
        fprintf(stream, "}\n");
    }
}
