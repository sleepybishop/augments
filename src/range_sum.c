#include <stdio.h>
#include <stdlib.h>
#include "range_sum.h"

static int node_cmp(struct sum_node *a, struct sum_node *b)
{
    if (a->key < b->key)
        return -1;
    if (a->key > b->key)
        return 1;
    return 0;
}

static void augment_node(sum_node *node)
{
    while (node) {
        sum_node *l = RB_LEFT(node, link);
        sum_node *r = RB_RIGHT(node, link);

        double old_sum = node->sum;
        node->sum = node->val + (l ? l->sum : 0.0) + (r ? r->sum : 0.0);

        /* Stop propagating early if the sum didn't change */
        if (node->sum == old_sum)
            break;

        node = RB_PARENT(node, link);
    }
}

#undef RB_AUGMENT
#define RB_AUGMENT augment_node
RB_GENERATE(_SUM_TREE, sum_node, link, node_cmp);

void sum_tree_init(sum_tree *tree)
{
    if (tree) {
        RB_INIT(&tree->rbt);
    }
}

static sum_node *create_node(int key, double val)
{
    sum_node *n = (sum_node *)calloc(1, sizeof(sum_node));
    if (n) {
        n->key = key;
        n->val = val;
        n->sum = val;
    }
    return n;
}

int sum_tree_add(sum_tree *tree, int key, double val)
{
    if (!tree)
        return 0;
    sum_node *n = create_node(key, val);
    if (!n)
        return 0;
    sum_node *f = RB_INSERT(_SUM_TREE, &tree->rbt, n);
    if (f) {
        free(n);
        return 0; /* Duplicate */
    }
    return 1;
}

sum_node *sum_tree_find(sum_tree *tree, int key)
{
    if (!tree)
        return NULL;
    sum_node temp = {.key = key};
    return RB_FIND(_SUM_TREE, &tree->rbt, &temp);
}

int sum_tree_remove(sum_tree *tree, int key)
{
    if (!tree)
        return 0;
    sum_node *n = sum_tree_find(tree, key);
    if (!n)
        return 0;
    RB_REMOVE(_SUM_TREE, &tree->rbt, n);
    free(n);
    return 1;
}

void sum_tree_update(sum_tree *tree, int key, double new_val)
{
    if (!tree)
        return;
    sum_node *n = sum_tree_find(tree, key);
    if (n) {
        n->val = new_val;
        augment_node(n);
    }
}

static double sum_less_than_or_equal(sum_node *node, int X)
{
    double total = 0.0;
    while (node) {
        if (node->key <= X) {
            total += node->val;
            if (RB_LEFT(node, link)) {
                total += RB_LEFT(node, link)->sum;
            }
            node = RB_RIGHT(node, link);
        } else {
            node = RB_LEFT(node, link);
        }
    }
    return total;
}

double sum_tree_query(sum_tree *tree, int low_key, int high_key)
{
    if (!tree)
        return 0.0;
    return sum_less_than_or_equal(RB_ROOT(&tree->rbt), high_key) - sum_less_than_or_equal(RB_ROOT(&tree->rbt), low_key - 1);
}

static void destroy_node(sum_node *node)
{
    if (node) {
        destroy_node(RB_LEFT(node, link));
        destroy_node(RB_RIGHT(node, link));
        free(node);
    }
}

void sum_tree_destroy(sum_tree *tree)
{
    if (tree) {
        destroy_node(RB_ROOT(&tree->rbt));
        RB_INIT(&tree->rbt);
    }
}

static void graph_node(sum_node *node, FILE *stream)
{
    fprintf(stream, "  \"node%p\" [fillcolor=\"%s\", label=\"{{ key: %d } | { val: %.2f | sum: %.2f }}\"];\n", (void *)node,
            RB_COLOR(node, link) == RB_BLACK ? "#000000" : "#aa0000", node->key, node->val, node->sum);
    if (RB_LEFT(node, link)) {
        fprintf(stream, "  \"node%p\" -> \"node%p\" [label=\"L\"];\n", (void *)node, (void *)RB_LEFT(node, link));
    }
    if (RB_RIGHT(node, link)) {
        fprintf(stream, "  \"node%p\" -> \"node%p\" [label=\"R\"];\n", (void *)node, (void *)RB_RIGHT(node, link));
    }
}

void sum_tree_graph(sum_tree *tree, FILE *stream)
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

        sum_node *node;
        RB_FOREACH(node, _SUM_TREE, &tree->rbt)
        {
            graph_node(node, stream);
        }
        fprintf(stream, "}\n");
    }
}
