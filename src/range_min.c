#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define EPSILON 1e-6
#include "range_min.h"

#ifndef INFINITY
#define INFINITY (1.0 / 0.0)
#endif

static int node_cmp(struct min_node *a, struct min_node *b)
{
    if (a->key < b->key)
        return -1;
    if (a->key > b->key)
        return 1;
    return 0;
}

static void augment_node(min_node *node)
{
    while (node) {
        min_node *l = RB_LEFT(node, link);
        min_node *r = RB_RIGHT(node, link);

        double old_min = node->min_val;
        node->min_val = node->val;
        if (l && l->min_val < node->min_val)
            node->min_val = l->min_val;
        if (r && r->min_val < node->min_val)
            node->min_val = r->min_val;

        /* Stop propagating early if the min didn't change */
        if (fabs(node->min_val - old_min) < EPSILON)
            break;

        node = RB_PARENT(node, link);
    }
}

#undef RB_AUGMENT
#define RB_AUGMENT augment_node
RB_GENERATE(_MIN_TREE, min_node, link, node_cmp);

void min_tree_init(min_tree *tree)
{
    if (!tree)
        return;
        if (tree) {
        RB_INIT(&tree->rbt);
    }
}

static void create_node(min_node *n,  int key, double val)
{
        memset(n, 0, sizeof(*n));
    if (n) {
        n->key = key;
        n->val = val;
        n->min_val = val;
    }
    }

int min_tree_add(min_tree *tree, min_node *n, int key, double val)
{
    if (!tree)
        return -1;
    create_node(n, key, val);
    if (!n)
        return -1;
    min_node *f = RB_INSERT(_MIN_TREE, &tree->rbt, n);
    if (f) {
                return 1; /* Duplicate */
    }
    return 0;
}

min_node *min_tree_find(min_tree *tree, int key)
{
        min_node temp = {.key = key};
    return RB_FIND(_MIN_TREE, &tree->rbt, &temp);
}

int min_tree_remove(min_tree *tree, int key)
{
    if (!tree)
        return 0;
    min_node *n = min_tree_find(tree, key);
    if (!n)
        return 0;
    RB_REMOVE(_MIN_TREE, &tree->rbt, n);
        return 1;
}

void min_tree_update(min_tree *tree, int key, double new_val)
{
    if (!tree)
        return;
    min_node *n = min_tree_find(tree, key);
    if (n) {
        n->val = new_val;
        augment_node(n);
    }
}

static double min_query_helper(min_node *node, int L, int R, double sub_L, double sub_R)
{
    if (!node)
        return INFINITY;

    /* If the subtree range [sub_L, sub_R] is completely outside [L, R] */
    if (sub_R < L || sub_L > R)
        return INFINITY;

    /* If the subtree range [sub_L, sub_R] is completely inside [L, R] */
    if (sub_L >= L && sub_R <= R)
        return node->min_val;

    /* Otherwise, it partially overlaps. Check the node itself if it is in [L, R] */
    double res = INFINITY;
    if (node->key >= L && node->key <= R) {
        res = node->val;
    }

    double left_res = min_query_helper(RB_LEFT(node, link), L, R, sub_L, (double)node->key);
    if (left_res < res)
        res = left_res;

    double right_res = min_query_helper(RB_RIGHT(node, link), L, R, (double)node->key, sub_R);
    if (right_res < res)
        res = right_res;

    return res;
}

double min_tree_query(min_tree *tree, int low_key, int high_key)
{
    if (!tree)
        return INFINITY;
    return min_query_helper(RB_ROOT(&tree->rbt), low_key, high_key, -INFINITY, INFINITY);
}

static void destroy_node(min_tree *tree, min_node *node)
{
    while (node) {
        if (RB_LEFT(node, link)) {
            min_node *l = RB_LEFT(node, link);
            RB_LEFT(node, link) = RB_RIGHT(l, link);
            RB_RIGHT(l, link) = node;
            node = l;
        } else {
            min_node *r = RB_RIGHT(node, link);
                        node = r;
        }
    }
}

void min_tree_destroy(min_tree *tree)
{
    if (tree) {
        destroy_node(tree, RB_ROOT(&tree->rbt));
        RB_INIT(&tree->rbt);
    }
}

static void graph_node(min_node *node, FILE *stream)
{
    fprintf(stream, "  \"node%p\" [fillcolor=\"%s\", label=\"{{ key: %d } | { val: %.2f | min_val: %.2f }}\"];\n", (void *)node,
            RB_COLOR(node, link) == RB_BLACK ? "#000000" : "#aa0000", node->key, node->val, node->min_val);
    if (RB_LEFT(node, link)) {
        fprintf(stream, "  \"node%p\" -> \"node%p\" [label=\"L\"];\n", (void *)node, (void *)RB_LEFT(node, link));
    }
    if (RB_RIGHT(node, link)) {
        fprintf(stream, "  \"node%p\" -> \"node%p\" [label=\"R\"];\n", (void *)node, (void *)RB_RIGHT(node, link));
    }
}

void min_tree_graph(min_tree *tree, FILE *stream)
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

        min_node *node;
        RB_FOREACH(node, _MIN_TREE, &tree->rbt)
        {
            graph_node(node, stream);
        }
        fprintf(stream, "}\n");
    }
}
