#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "max_subarray.h"

#define EPSILON 1e-6

#ifndef INFINITY
#define INFINITY (1.0 / 0.0)
#endif

static int node_cmp(struct sub_node *a, struct sub_node *b)
{
    if (a->key < b->key)
        return -1;
    if (a->key > b->key)
        return 1;
    return 0;
}

static void augment_node(sub_node *node)
{
    while (node) {
        sub_node *l = RB_LEFT(node, link);
        sub_node *r = RB_RIGHT(node, link);

        double old_sum = node->sum;
        double old_pref = node->max_prefix;
        double old_suff = node->max_suffix;
        double old_sub = node->max_sub;

        node->sum = node->val + (l ? l->sum : 0.0) + (r ? r->sum : 0.0);

        /* Max prefix */
        double l_pref = l ? l->max_prefix : -INFINITY;
        double l_sum_mid = (l ? l->sum : 0.0) + node->val;
        double l_sum_mid_r = (l ? l->sum : 0.0) + node->val + (r ? r->max_prefix : 0.0);

        node->max_prefix = l_pref;
        if (l_sum_mid > node->max_prefix)
            node->max_prefix = l_sum_mid;
        if (r && l_sum_mid_r > node->max_prefix)
            node->max_prefix = l_sum_mid_r;

        /* Max suffix */
        double r_suff = r ? r->max_suffix : -INFINITY;
        double r_sum_mid = node->val + (r ? r->sum : 0.0);
        double r_sum_mid_l = (l ? l->max_suffix : 0.0) + node->val + (r ? r->sum : 0.0);

        node->max_suffix = r_suff;
        if (r_sum_mid > node->max_suffix)
            node->max_suffix = r_sum_mid;
        if (l && r_sum_mid_l > node->max_suffix)
            node->max_suffix = r_sum_mid_l;

        /* Max subarray sum */
        double max_s = node->val;
        if (l && l->max_sub > max_s)
            max_s = l->max_sub;
        if (r && r->max_sub > max_s)
            max_s = r->max_sub;

        double l_suff_part = (l && l->max_suffix > 0.0) ? l->max_suffix : 0.0;
        double r_pref_part = (r && r->max_prefix > 0.0) ? r->max_prefix : 0.0;
        double cross = l_suff_part + node->val + r_pref_part;
        if (cross > max_s)
            max_s = cross;

        node->max_sub = max_s;

        /* Stop propagating early if all augmented values are unchanged */
        if (fabs(node->sum - old_sum) < EPSILON && fabs(node->max_prefix - old_pref) < EPSILON &&
            fabs(node->max_suffix - old_suff) < EPSILON && fabs(node->max_sub - old_sub) < EPSILON) {
            break;
        }

        node = RB_PARENT(node, link);
    }
}

#undef RB_AUGMENT
#define RB_AUGMENT augment_node
RB_GENERATE(_SUB_TREE, sub_node, link, node_cmp);

void sub_tree_init(sub_tree *tree)
{
    if (!tree)
        return;
        if (tree) {
        RB_INIT(&tree->rbt);
    }
}

static void create_node(sub_node *n,  int key, double val)
{
        memset(n, 0, sizeof(*n));
    if (n) {
        n->key = key;
        n->val = val;
        n->sum = val;
        n->max_prefix = val;
        n->max_suffix = val;
        n->max_sub = val;
    }
    }

int sub_tree_add(sub_tree *tree, sub_node *n, int key, double val)
{
    if (!tree)
        return -1;
    create_node(n, key, val);
    if (!n)
        return -1;
    sub_node *f = RB_INSERT(_SUB_TREE, &tree->rbt, n);
    if (f) {
                return 1; /* Duplicate */
    }
    return 0;
}

sub_node *sub_tree_find(sub_tree *tree, int key)
{
        sub_node temp = {.key = key};
    return RB_FIND(_SUB_TREE, &tree->rbt, &temp);
}

int sub_tree_remove(sub_tree *tree, int key)
{
    if (!tree)
        return 0;
    sub_node *n = sub_tree_find(tree, key);
    if (!n)
        return 0;
    RB_REMOVE(_SUB_TREE, &tree->rbt, n);
        return 1;
}

void sub_tree_update(sub_tree *tree, int key, double new_val)
{
    if (!tree)
        return;
    sub_node *n = sub_tree_find(tree, key);
    if (n) {
        n->val = new_val;
        augment_node(n);
    }
}

typedef struct sub_metrics {
    double sum;
    double max_prefix;
    double max_suffix;
    double max_sub;
    int is_empty;
} sub_metrics;

static sub_metrics merge_metrics(sub_metrics L, double mid_val, int mid_in_range, sub_metrics R)
{
    sub_metrics res;
    res.is_empty = 0;

    if (L.is_empty && R.is_empty && !mid_in_range) {
        res.is_empty = 1;
        res.sum = 0.0;
        res.max_prefix = -INFINITY;
        res.max_suffix = -INFINITY;
        res.max_sub = -INFINITY;
        return res;
    }

    res.sum = (L.is_empty ? 0.0 : L.sum) + (mid_in_range ? mid_val : 0.0) + (R.is_empty ? 0.0 : R.sum);

    /* Max prefix */
    double max_pref = -INFINITY;
    if (!L.is_empty) {
        if (L.max_prefix > max_pref)
            max_pref = L.max_prefix;
    }
    if (mid_in_range) {
        double val = (L.is_empty ? 0.0 : L.sum) + mid_val;
        if (val > max_pref)
            max_pref = val;
        if (!R.is_empty) {
            double val2 = (L.is_empty ? 0.0 : L.sum) + mid_val + R.max_prefix;
            if (val2 > max_pref)
                max_pref = val2;
        }
    } else {
        if (L.is_empty && !R.is_empty) {
            if (R.max_prefix > max_pref)
                max_pref = R.max_prefix;
        }
    }
    res.max_prefix = max_pref;

    /* Max suffix */
    double max_suff = -INFINITY;
    if (!R.is_empty) {
        if (R.max_suffix > max_suff)
            max_suff = R.max_suffix;
    }
    if (mid_in_range) {
        double val = mid_val + (R.is_empty ? 0.0 : R.sum);
        if (val > max_suff)
            max_suff = val;
        if (!L.is_empty) {
            double val2 = L.max_suffix + mid_val + (R.is_empty ? 0.0 : R.sum);
            if (val2 > max_suff)
                max_suff = val2;
        }
    } else {
        if (R.is_empty && !L.is_empty) {
            if (L.max_suffix > max_suff)
                max_suff = L.max_suffix;
        }
    }
    res.max_suffix = max_suff;

    /* Max subarray sum */
    double max_s = -INFINITY;
    if (!L.is_empty && L.max_sub > max_s)
        max_s = L.max_sub;
    if (!R.is_empty && R.max_sub > max_s)
        max_s = R.max_sub;
    if (mid_in_range) {
        double left_part = 0.0;
        if (!L.is_empty && L.max_suffix > 0.0)
            left_part = L.max_suffix;
        double right_part = 0.0;
        if (!R.is_empty && R.max_prefix > 0.0)
            right_part = R.max_prefix;
        double cross = left_part + mid_val + right_part;
        if (cross > max_s)
            max_s = cross;
    }
    res.max_sub = max_s;

    return res;
}

static sub_metrics sub_query_helper(sub_node *node, int L, int R, double sub_L, double sub_R)
{
    sub_metrics empty = {.is_empty = 1, .sum = 0.0, .max_prefix = -INFINITY, .max_suffix = -INFINITY, .max_sub = -INFINITY};
    if (!node)
        return empty;

    /* Completely outside */
    if (sub_R < L || sub_L > R)
        return empty;

    /* Completely inside */
    if (sub_L >= L && sub_R <= R) {
        sub_metrics res = {.sum = node->sum,
                           .max_prefix = node->max_prefix,
                           .max_suffix = node->max_suffix,
                           .max_sub = node->max_sub,
                           .is_empty = 0};
        return res;
    }

    /* Partially overlapping */
    int mid_in_range = (node->key >= L && node->key <= R);

    sub_metrics left_res = sub_query_helper(RB_LEFT(node, link), L, R, sub_L, (double)node->key);
    sub_metrics right_res = sub_query_helper(RB_RIGHT(node, link), L, R, (double)node->key, sub_R);

    return merge_metrics(left_res, node->val, mid_in_range, right_res);
}

double sub_tree_query(sub_tree *tree, int low_key, int high_key)
{
    if (!tree)
        return -INFINITY;
    sub_metrics res = sub_query_helper(RB_ROOT(&tree->rbt), low_key, high_key, -INFINITY, INFINITY);
    return res.max_sub;
}

static void destroy_node(sub_tree *tree, sub_node *node)
{
    while (node) {
        if (RB_LEFT(node, link)) {
            sub_node *l = RB_LEFT(node, link);
            RB_LEFT(node, link) = RB_RIGHT(l, link);
            RB_RIGHT(l, link) = node;
            node = l;
        } else {
            sub_node *r = RB_RIGHT(node, link);
                        node = r;
        }
    }
}

void sub_tree_destroy(sub_tree *tree)
{
    if (tree) {
        destroy_node(tree, RB_ROOT(&tree->rbt));
        RB_INIT(&tree->rbt);
    }
}

static void graph_node(sub_node *node, FILE *stream)
{
    fprintf(stream,
            "  \"node%p\" [fillcolor=\"%s\", label=\"{{ key: %d } | { val: %.2f | sum: %.2f } | { pref: %.2f | suff: %.2f | "
            "max_sub: %.2f }}\"];\n",
            (void *)node, RB_COLOR(node, link) == RB_BLACK ? "#000000" : "#aa0000", node->key, node->val, node->sum,
            node->max_prefix, node->max_suffix, node->max_sub);
    if (RB_LEFT(node, link)) {
        fprintf(stream, "  \"node%p\" -> \"node%p\" [label=\"L\"];\n", (void *)node, (void *)RB_LEFT(node, link));
    }
    if (RB_RIGHT(node, link)) {
        fprintf(stream, "  \"node%p\" -> \"node%p\" [label=\"R\"];\n", (void *)node, (void *)RB_RIGHT(node, link));
    }
}

void sub_tree_graph(sub_tree *tree, FILE *stream)
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

        sub_node *node;
        RB_FOREACH(node, _SUB_TREE, &tree->rbt)
        {
            graph_node(node, stream);
        }
        fprintf(stream, "}\n");
    }
}
