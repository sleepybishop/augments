#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include "interval_tree.h"

#define MIN_VAL 0

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
    itree_node *l = RB_LEFT(node, link);
    itree_node *r = RB_RIGHT(node, link);
    itree_node *p = RB_PARENT(node, link);

    node->left_max = MIN_VAL;
    node->right_max = MIN_VAL;

    if (l)
        node->left_max = MAX(l->high, MAX(l->left_max, l->right_max));
    if (r)
        node->right_max = MAX(r->high, MAX(r->left_max, r->right_max));
    if (p)
        augment_node(p);
}

#undef RB_AUGMENT
#define RB_AUGMENT augment_node
RB_GENERATE(_INTERVAL_TREE, itree_node, link, node_cmp);

typedef int (*itree_node_cb)(itree_node *node, void *arg);

static int on_match(itree_node *node, itree_match_params *p)
{
    return 0;
}

static int delete_node(itree_node *node, void *arg)
{
    itree *ctx = (itree *)arg;
    RB_REMOVE(_INTERVAL_TREE, &ctx->rbt, node);
    free(node);
    return 0;
}

static int match_node(itree_node *node, void *arg)
{
    itree_match_params *p = (itree_match_params *)arg;
    if (p->low >= node->low && p->high <= node->high) {
        return on_match(node, p);
    } else if (p->overlaps > 0) {
        if ((p->low >= node->low && p->low <= node->high) || (p->high >= node->low && p->high <= node->high) ||
            (p->low < node->low && p->high > node->high)) {
            return on_match(node, p);
        }
    }
    return 0;
}

static itree_node *create_node(size_t low, size_t high, size_t id)
{
    itree_node *n = (itree_node *)calloc(1, sizeof(itree_node));
    if (n) {
        n->id = id;
        n->low = low;
        n->high = high;
        n->left_max = MIN_VAL;
        n->right_max = MIN_VAL;
    }
    return n;
}

/* iterative inorder, no branch pruning */
static int traverse(itree *ctx, itree_node_cb cb, void *arg)
{
    itree_node *n, *nxt;
    int rc = 0;
    if (arg == NULL)
        arg = ctx;
    for (n = RB_MIN(_INTERVAL_TREE, &ctx->rbt); n != NULL; n = nxt) {
        nxt = RB_NEXT(_INTERVAL_TREE, &ctx->rbt, n);
        if ((rc = (*cb)(n, arg)) != 0)
            return rc;
    }
    return 0;
}

/* recursive inorder with branch pruning */
static int traverse2(itree *ctx, itree_node *node, itree_node_cb cb, void *arg)
{
    int rc = 0;
    itree_match_params *p = (itree_match_params *)arg;

    /* skip entire subtree? */
    if (p->low > MAX(node->high, MAX(node->left_max, node->right_max)))
        return 0;

    /* descend left */
    if (RB_LEFT(node, link))
        traverse2(ctx, RB_LEFT(node, link), cb, arg);

    if ((rc = (*cb)(node, arg)) != 0)
        return (rc);

    /* skip right subtree? */
    if (p->high < node->low)
        return 0;

    /* descend right */
    if (RB_RIGHT(node, link))
        traverse2(ctx, RB_RIGHT(node, link), cb, arg);

    return 0;
}

/* iterative inorder with branch pruning */
static int traverse3(itree *ctx, itree_node_cb cb, void *arg)
{
    itree_node *n, *nxt;
    itree_match_params *p = (itree_match_params *)arg;
    int rc = 0, descend_right = 1;

    for (n = RB_MIN(_INTERVAL_TREE, &ctx->rbt); n != NULL; n = nxt) {
        itree_node *elm = n;

        if (p->low > MAX(n->high, MAX(n->left_max, n->right_max))) {
            while (RB_PARENT(elm, link) && (elm == RB_RIGHT(RB_PARENT(elm, link), link)))
                elm = RB_PARENT(elm, link);
            elm = RB_PARENT(elm, link);
            nxt = elm;
            continue;
        }

        if (RB_RIGHT(elm, link) && descend_right) {
            elm = RB_RIGHT(elm, link);
            while (RB_LEFT(elm, link))
                elm = RB_LEFT(elm, link);
        } else {
            if (RB_PARENT(elm, link) && (elm == RB_LEFT(RB_PARENT(elm, link), link)))
                elm = RB_PARENT(elm, link);
            else {
                while (RB_PARENT(elm, link) && (elm == RB_RIGHT(RB_PARENT(elm, link), link)))
                    elm = RB_PARENT(elm, link);
                elm = RB_PARENT(elm, link);
            }
        }
        nxt = elm;

        descend_right = (p->high < n->low) ? 0 : 1;

        if ((rc = (*cb)(n, arg)) != 0)
            return rc;
    }

    return 0;
}

int intervaltree_add(itree *ctx, size_t low, size_t high, size_t id)
{
    int ret = 1;
    itree_node *f, *n;
    n = create_node(low, high, id);

    f = RB_INSERT(_INTERVAL_TREE, &ctx->rbt, n);
    if (f) {
        free(n); /* already exists */
        ret = 0;
    }
    return ret;
}

int itree_remove(itree *ctx, size_t low, size_t high, size_t overlaps)
{
    if (ctx) {
        itree_match_params p = {.low = low, .high = high, .overlaps = overlaps};
        traverse3(ctx, delete_node, (void *)&p);
    }
    return 0;
}

void intervaltree_find(itree *ctx, size_t low, size_t high, size_t overlaps)
{
    if (ctx) {
        itree_match_params p = {.low = low, .high = high, .overlaps = overlaps};
        traverse3(ctx, match_node, (void *)&p);
    }
}

void intervaltree_destroy(itree *ctx)
{
    if (ctx) {
        traverse(ctx, delete_node, NULL);
    }
}

static int graph_node(itree_node *node, void *arg)
{
    FILE *stream = (FILE *)arg;

    fprintf(stream, "  \"node%zu\" [fillcolor=\"%s\", label=\"{{ node: %zu} | {%zu | %zu | {%zu | %zu} }}\"];\n", node->id,
            RB_COLOR(node, link) == RB_BLACK ? "#000000" : "#aa0000", node->id, node->low, node->high, node->left_max,
            node->right_max);
    if (RB_LEFT(node, link)) {
        fprintf(stream, "  \"node%zu\" -> \"node%zu\" [label=\"L\"];\n", node->id, RB_LEFT(node, link)->id);
    }
    if (RB_RIGHT(node, link)) {
        fprintf(stream, "  \"node%zu\" -> \"node%zu\" [label=\"R\"];\n", node->id, RB_RIGHT(node, link)->id);
    }

    return 0;
}

void intervaltree_graph(itree *ctx, FILE *stream)
{
    if (!stream)
        stream = stdout;
    if (ctx) {
        fprintf(stream, "digraph {\n");
        fprintf(stream, "graph [ranksep=\"0.25\", rankdir=\"LR\" fontname=\"Monofur\", bgcolor=\"#222222\", nodesep=\"0.125\"];\n");
        fprintf(stream, "node [fontname=\"Monofur\", color=\"#cccccc\", style=\"filled\", height=\"0\", width=\"1\", "
                        "shape=\"Mrecord\", fontcolor=\"#ffffff\"];\n");
        fprintf(stream,
                "edge [fontname=\"Monofur\", color=\"#cccccc\", arrowsize=\"0.80\", penwidth=\"2.0\", fontcolor=\"#cccccc\"];\n");
        traverse(ctx, graph_node, stream);
        fprintf(stream, "}\n");
    }
}
