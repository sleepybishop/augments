#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interval_tree.h"

static int node_cmp(struct itree_node *a, struct itree_node *b)
{
    if (a->low < b->low)
        return -1;
    else if (a->low > b->low)
        return 1;
    else {
        if (a->high > b->high)
            return -1;
        else if (a->high < b->high)
            return 1;
    }
    return 0;
}

static void augment_node(itree_node *node)
{
    while (node) {
        itree_node *l = RB_LEFT(node, link);
        itree_node *r = RB_RIGHT(node, link);

        size_t old_max = node->max;
        node->max = node->high;
        if (l && l->max > node->max)
            node->max = l->max;
        if (r && r->max > node->max)
            node->max = r->max;

        /* Stop propagating early if the max didn't change */
        if (node->max == old_max)
            break;

        node = RB_PARENT(node, link);
    }
}

#undef RB_AUGMENT
#define RB_AUGMENT augment_node
RB_GENERATE(_INTERVAL_TREE, itree_node, link, node_cmp);

void interval_tree_init(itree *tree)
{
    if (tree) {
        RB_INIT(&tree->rbt);
    }
}

static itree_node *create_node(size_t low, size_t high, size_t id)
{
    itree_node *n = (itree_node *)calloc(1, sizeof(itree_node));
    if (n) {
        n->id = id;
        n->low = low;
        n->high = high;
        n->max = high;
    }
    return n;
}

int interval_tree_add(itree *tree, size_t low, size_t high, size_t id)
{
    int ret = 1;
    itree_node *f, *n;
    n = create_node(low, high, id);
    if (!n)
        return 0;

    f = RB_INSERT(_INTERVAL_TREE, &tree->rbt, n);
    if (f) {
        free(n); /* already exists */
        ret = 0;
    }
    return ret;
}

struct find_ctx {
    size_t low;
    size_t high;
    int overlaps;
    itree_cb cb;
    void *arg;
    int stop;
};

static void find_nodes_helper(itree_node *node, struct find_ctx *ctx)
{
    if (!node || ctx->stop)
        return;

    /* Skip entire subtree if query low is greater than the max high of this subtree */
    if (ctx->low > node->max)
        return;

    /* Descend left only if it is possible to find matches (ctx->low <= left->max) */
    if (RB_LEFT(node, link) && ctx->low <= RB_LEFT(node, link)->max) {
        find_nodes_helper(RB_LEFT(node, link), ctx);
    }

    if (ctx->stop)
        return;

    /* Check if current node matches */
    int is_match = 0;
    if (ctx->low >= node->low && ctx->high <= node->high) {
        is_match = 1;
    } else if (ctx->overlaps > 0) {
        if ((ctx->low >= node->low && ctx->low <= node->high) || (ctx->high >= node->low && ctx->high <= node->high) ||
            (ctx->low < node->low && ctx->high > node->high)) {
            is_match = 1;
        }
    }

    if (is_match) {
        if (ctx->cb(node, ctx->arg) != 0) {
            ctx->stop = 1;
            return;
        }
    }

    /* Descend right only if query high is >= node's low */
    if (RB_RIGHT(node, link) && ctx->high >= node->low) {
        find_nodes_helper(RB_RIGHT(node, link), ctx);
    }
}

void interval_tree_find(itree *tree, size_t low, size_t high, int overlaps, itree_cb cb, void *arg)
{
    if (tree && cb) {
        struct find_ctx ctx = {.low = low, .high = high, .overlaps = overlaps, .cb = cb, .arg = arg, .stop = 0};
        find_nodes_helper(RB_ROOT(&tree->rbt), &ctx);
    }
}

struct collect_ctx {
    itree_node **collected;
    int count;
    int capacity;
};

static int collect_cb(itree_node *node, void *arg)
{
    struct collect_ctx *ctx = (struct collect_ctx *)arg;
    if (ctx->count >= ctx->capacity) {
        ctx->capacity *= 2;
        ctx->collected = realloc(ctx->collected, ctx->capacity * sizeof(itree_node *));
    }
    ctx->collected[ctx->count++] = node;
    return 0;
}

int interval_tree_remove(itree *tree, size_t low, size_t high, size_t id)
{
    /*
     * In this implementation, we remove the specific node with (low, high, id).
     * If id is (size_t)-1, we remove all matching nodes.
     */
    if (!tree)
        return 0;

    struct collect_ctx c_ctx = {.collected = malloc(16 * sizeof(itree_node *)), .count = 0, .capacity = 16};
    if (!c_ctx.collected)
        return 0;

    interval_tree_find(tree, low, high, 1, collect_cb, &c_ctx);

    int removed = 0;
    for (int i = 0; i < c_ctx.count; i++) {
        itree_node *n = c_ctx.collected[i];
        if (n->low == low && n->high == high && (id == (size_t)-1 || n->id == id)) {
            RB_REMOVE(_INTERVAL_TREE, &tree->rbt, n);
            free(n);
            removed++;
        }
    }
    free(c_ctx.collected);
    return removed;
}

static void destroy_node(itree_node *node)
{
    if (node) {
        destroy_node(RB_LEFT(node, link));
        destroy_node(RB_RIGHT(node, link));
        free(node);
    }
}

void interval_tree_destroy(itree *tree)
{
    if (tree) {
        destroy_node(RB_ROOT(&tree->rbt));
        RB_INIT(&tree->rbt);
    }
}

static int graph_node(itree_node *node, FILE *stream)
{
    fprintf(stream, "  \"node%zu\" [fillcolor=\"%s\", label=\"{{ node: %zu} | {%zu | %zu | {%zu} }}\"];\n", node->id,
            RB_COLOR(node, link) == RB_BLACK ? "#000000" : "#aa0000", node->id, node->low, node->high, node->max);
    if (RB_LEFT(node, link)) {
        fprintf(stream, "  \"node%zu\" -> \"node%zu\" [label=\"L\"];\n", node->id, RB_LEFT(node, link)->id);
    }
    if (RB_RIGHT(node, link)) {
        fprintf(stream, "  \"node%zu\" -> \"node%zu\" [label=\"R\"];\n", node->id, RB_RIGHT(node, link)->id);
    }
    return 0;
}

void interval_tree_graph(itree *tree, FILE *stream)
{
    if (!stream)
        stream = stdout;
    if (tree) {
        fprintf(stream, "digraph {\n");
        fprintf(stream,
                "graph [ranksep=\"0.25\", rankdir=\"LR\" fontname=\"Monofur\", bgcolor=\"transparent\", nodesep=\"0.125\"];\n");
        fprintf(stream, "node [fontname=\"Monofur\", color=\"#cccccc\", style=\"filled\", height=\"0\", width=\"1\", "
                        "shape=\"Mrecord\", fontcolor=\"#ffffff\"];\n");
        fprintf(stream,
                "edge [fontname=\"Monofur\", color=\"#cccccc\", arrowsize=\"0.80\", penwidth=\"2.0\", fontcolor=\"#cccccc\"];\n");

        itree_node *node;
        RB_FOREACH(node, _INTERVAL_TREE, &tree->rbt)
        {
            graph_node(node, stream);
        }
        fprintf(stream, "}\n");
    }
}
