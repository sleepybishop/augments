#include <stdio.h>
#include <stdlib.h>
#include "priority_search.h"

static int node_cmp(struct ps_node *a, struct ps_node *b)
{
    if (a->x < b->x)
        return -1;
    if (a->x > b->x)
        return 1;
    if (a->y < b->y)
        return -1;
    if (a->y > b->y)
        return 1;
    return 0;
}

static void augment_node(ps_node *node)
{
    while (node) {
        ps_node *l = RB_LEFT(node, link);
        ps_node *r = RB_RIGHT(node, link);

        double old_max_y = node->max_y;
        ps_node *old_max_node = node->max_y_node;

        node->max_y = node->y;
        node->max_y_node = node;

        if (l && l->max_y > node->max_y) {
            node->max_y = l->max_y;
            node->max_y_node = l->max_y_node;
        }
        if (r && r->max_y > node->max_y) {
            node->max_y = r->max_y;
            node->max_y_node = r->max_y_node;
        }

        /* Stop propagating early if the max didn't change */
        if (node->max_y == old_max_y && node->max_y_node == old_max_node)
            break;

        node = RB_PARENT(node, link);
    }
}

#undef RB_AUGMENT
#define RB_AUGMENT augment_node
RB_GENERATE(_PS_TREE, ps_node, link, node_cmp);

void ps_tree_init(ps_tree *tree)
{
    if (tree) {
        RB_INIT(&tree->rbt);
    }
}

static ps_node *create_node(double x, double y)
{
    ps_node *n = (ps_node *)calloc(1, sizeof(ps_node));
    if (n) {
        n->x = x;
        n->y = y;
        n->max_y = y;
        n->max_y_node = n;
    }
    return n;
}

int ps_tree_add(ps_tree *tree, double x, double y)
{
    if (!tree)
        return 0;
    ps_node *n = create_node(x, y);
    if (!n)
        return 0;
    ps_node *f = RB_INSERT(_PS_TREE, &tree->rbt, n);
    if (f) {
        free(n);
        return 0; /* Duplicate */
    }
    return 1;
}

ps_node *ps_tree_find(ps_tree *tree, double x, double y)
{
    if (!tree)
        return NULL;
    ps_node temp = {.x = x, .y = y};
    return RB_FIND(_PS_TREE, &tree->rbt, &temp);
}

int ps_tree_remove(ps_tree *tree, double x, double y)
{
    if (!tree)
        return 0;
    ps_node *n = ps_tree_find(tree, x, y);
    if (!n)
        return 0;
    RB_REMOVE(_PS_TREE, &tree->rbt, n);
    free(n);
    return 1;
}

static void ps_query_helper(ps_node *node, double x_min, double x_max, double y_min, ps_cb cb, void *arg)
{
    if (!node)
        return;

    /* If the maximum y in this subtree is less than y_min, prune */
    if (node->max_y < y_min)
        return;

    /* If the current node itself matches the 3-sided query, report it */
    if (node->x >= x_min && node->x <= x_max && node->y >= y_min) {
        cb(node, arg);
    }

    /* Prune left subtree if all its keys are strictly less than x_min */
    if (node->x >= x_min) {
        ps_query_helper(RB_LEFT(node, link), x_min, x_max, y_min, cb, arg);
    }
    /* Prune right subtree if all its keys are strictly greater than x_max */
    if (node->x <= x_max) {
        ps_query_helper(RB_RIGHT(node, link), x_min, x_max, y_min, cb, arg);
    }
}

void ps_tree_query(ps_tree *tree, double x_min, double x_max, double y_min, ps_cb cb, void *arg)
{
    if (tree && cb) {
        ps_query_helper(RB_ROOT(&tree->rbt), x_min, x_max, y_min, cb, arg);
    }
}

static void destroy_node(ps_node *node)
{
    if (node) {
        destroy_node(RB_LEFT(node, link));
        destroy_node(RB_RIGHT(node, link));
        free(node);
    }
}

void ps_tree_destroy(ps_tree *tree)
{
    if (tree) {
        destroy_node(RB_ROOT(&tree->rbt));
        RB_INIT(&tree->rbt);
    }
}

static void graph_node(ps_node *node, FILE *stream)
{
    fprintf(stream, "  \"node%p\" [fillcolor=\"%s\", label=\"{{ x: %.2f | y: %.2f } | { max_y: %.2f }}\"];\n", (void *)node,
            RB_COLOR(node, link) == RB_BLACK ? "#000000" : "#aa0000", node->x, node->y, node->max_y);
    if (RB_LEFT(node, link)) {
        fprintf(stream, "  \"node%p\" -> \"node%p\" [label=\"L\"];\n", (void *)node, (void *)RB_LEFT(node, link));
    }
    if (RB_RIGHT(node, link)) {
        fprintf(stream, "  \"node%p\" -> \"node%p\" [label=\"R\"];\n", (void *)node, (void *)RB_RIGHT(node, link));
    }
}

void ps_tree_graph(ps_tree *tree, FILE *stream)
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

        ps_node *node;
        RB_FOREACH(node, _PS_TREE, &tree->rbt)
        {
            graph_node(node, stream);
        }
        fprintf(stream, "}\n");
    }
}
