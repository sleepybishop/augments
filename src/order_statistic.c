#include <stdio.h>
#include <stdlib.h>
#include "order_statistic.h"

static int node_cmp(struct os_node *a, struct os_node *b)
{
    if (a->key < b->key)
        return -1;
    if (a->key > b->key)
        return 1;
    return 0;
}

static void augment_node(os_node *node)
{
    while (node) {
        os_node *l = RB_LEFT(node, link);
        os_node *r = RB_RIGHT(node, link);

        size_t old_size = node->size;
        node->size = 1 + (l ? l->size : 0) + (r ? r->size : 0);

        /* Stop propagating early if the size didn't change */
        if (node->size == old_size)
            break;

        node = RB_PARENT(node, link);
    }
}

#undef RB_AUGMENT
#define RB_AUGMENT augment_node
RB_GENERATE(_OS_TREE, os_node, link, node_cmp);

void os_tree_init(os_tree *tree)
{
    if (tree) {
        RB_INIT(&tree->rbt);
    }
}

static os_node *create_node(int key)
{
    os_node *n = (os_node *)calloc(1, sizeof(os_node));
    if (n) {
        n->key = key;
        n->size = 1;
    }
    return n;
}

int os_tree_add(os_tree *tree, int key)
{
    if (!tree)
        return 0;
    os_node *n = create_node(key);
    if (!n)
        return 0;
    os_node *f = RB_INSERT(_OS_TREE, &tree->rbt, n);
    if (f) {
        free(n);
        return 0; /* Duplicate */
    }
    return 1;
}

os_node *os_tree_find(os_tree *tree, int key)
{
    if (!tree)
        return NULL;
    os_node temp = {.key = key};
    return RB_FIND(_OS_TREE, &tree->rbt, &temp);
}

int os_tree_remove(os_tree *tree, int key)
{
    if (!tree)
        return 0;
    os_node *n = os_tree_find(tree, key);
    if (!n)
        return 0;
    RB_REMOVE(_OS_TREE, &tree->rbt, n);
    free(n);
    return 1;
}

os_node *os_tree_select(os_tree *tree, size_t rank)
{
    if (!tree)
        return NULL;
    os_node *curr = RB_ROOT(&tree->rbt);
    while (curr) {
        size_t left_size = RB_LEFT(curr, link) ? RB_LEFT(curr, link)->size : 0;
        if (rank == left_size)
            return curr;
        if (rank < left_size) {
            curr = RB_LEFT(curr, link);
        } else {
            rank -= (left_size + 1);
            curr = RB_RIGHT(curr, link);
        }
    }
    return NULL;
}

size_t os_tree_rank(os_tree *tree, int key)
{
    if (!tree)
        return (size_t)-1;
    os_node *node = os_tree_find(tree, key);
    if (!node)
        return (size_t)-1;

    size_t r = RB_LEFT(node, link) ? RB_LEFT(node, link)->size : 0;
    os_node *curr = node;
    while (curr) {
        os_node *p = RB_PARENT(curr, link);
        if (p && curr == RB_RIGHT(p, link)) {
            r += (RB_LEFT(p, link) ? RB_LEFT(p, link)->size : 0) + 1;
        }
        curr = p;
    }
    return r;
}

static void destroy_node(os_node *node)
{
    if (node) {
        destroy_node(RB_LEFT(node, link));
        destroy_node(RB_RIGHT(node, link));
        free(node);
    }
}

void os_tree_destroy(os_tree *tree)
{
    if (tree) {
        destroy_node(RB_ROOT(&tree->rbt));
        RB_INIT(&tree->rbt);
    }
}

static void graph_node(os_node *node, FILE *stream)
{
    fprintf(stream, "  \"node%p\" [fillcolor=\"%s\", label=\"{{ key: %d } | { size: %zu }}\"];\n", (void *)node,
            RB_COLOR(node, link) == RB_BLACK ? "#000000" : "#aa0000", node->key, node->size);
    if (RB_LEFT(node, link)) {
        fprintf(stream, "  \"node%p\" -> \"node%p\" [label=\"L\"];\n", (void *)node, (void *)RB_LEFT(node, link));
    }
    if (RB_RIGHT(node, link)) {
        fprintf(stream, "  \"node%p\" -> \"node%p\" [label=\"R\"];\n", (void *)node, (void *)RB_RIGHT(node, link));
    }
}

void os_tree_graph(os_tree *tree, FILE *stream)
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

        os_node *node;
        RB_FOREACH(node, _OS_TREE, &tree->rbt)
        {
            graph_node(node, stream);
        }
        fprintf(stream, "}\n");
    }
}
