#include "rope.h"
#include <stdlib.h>
#include <stdio.h>

static void rope_augment(rope_node *node)
{
    if (!node)
        return;
    node->size = 1;
    if (TREAP_LEFT(node, link))
        node->size += TREAP_LEFT(node, link)->size;
    if (TREAP_RIGHT(node, link))
        node->size += TREAP_RIGHT(node, link)->size;
}

#undef TREAP_AUGMENT
#define TREAP_AUGMENT(x) rope_augment(x)

static rope_node *rope_merge(rope_node *l, rope_node *r)
{
    if (!l)
        return r;
    if (!r)
        return l;

    if (TREAP_PRIO(l, link) > TREAP_PRIO(r, link)) {
        rope_node *merged_right = rope_merge(TREAP_RIGHT(l, link), r);
        TREAP_RIGHT(l, link) = merged_right;
        if (merged_right)
            TREAP_PARENT(merged_right, link) = l;
        rope_augment(l);
        return l;
    } else {
        rope_node *merged_left = rope_merge(l, TREAP_LEFT(r, link));
        TREAP_LEFT(r, link) = merged_left;
        if (merged_left)
            TREAP_PARENT(merged_left, link) = r;
        rope_augment(r);
        return r;
    }
}

static void rope_split(rope_node *node, size_t k, rope_node **l, rope_node **r)
{
    if (!node) {
        *l = *r = NULL;
        return;
    }

    size_t left_sz = TREAP_LEFT(node, link) ? TREAP_LEFT(node, link)->size : 0;

    if (left_sz >= k) {
        rope_split(TREAP_LEFT(node, link), k, l, &TREAP_LEFT(node, link));
        if (TREAP_LEFT(node, link))
            TREAP_PARENT(TREAP_LEFT(node, link), link) = node;
        rope_augment(node);
        *r = node;
        if (*r)
            TREAP_PARENT(*r, link) = NULL;
    } else {
        rope_split(TREAP_RIGHT(node, link), k - left_sz - 1, &TREAP_RIGHT(node, link), r);
        if (TREAP_RIGHT(node, link))
            TREAP_PARENT(TREAP_RIGHT(node, link), link) = node;
        rope_augment(node);
        *l = node;
        if (*l)
            TREAP_PARENT(*l, link) = NULL;
    }
}

void rope_init(rope *rope)
{
    TREAP_INIT(&rope->trt);
}

static uint32_t fast_rand(void)
{
    static uint32_t x = 123456789;
    static uint32_t y = 362436069;
    static uint32_t z = 521288629;
    static uint32_t w = 88675123;
    uint32_t t = x ^ (x << 11);
    x = y;
    y = z;
    z = w;
    return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
}

int rope_insert(rope *rope, size_t index, char val)
{
    rope_node *node = malloc(sizeof(*node));
    if (!node)
        return -1;
    node->val = val;
    node->size = 1;
    TREAP_LEFT(node, link) = TREAP_RIGHT(node, link) = TREAP_PARENT(node, link) = NULL;
    TREAP_PRIO(node, link) = fast_rand();

    rope_node *l = NULL, *r = NULL;
    rope_split(TREAP_ROOT(&rope->trt), index, &l, &r);

    rope_node *merged = rope_merge(l, node);
    merged = rope_merge(merged, r);

    TREAP_ROOT(&rope->trt) = merged;
    if (merged)
        TREAP_PARENT(merged, link) = NULL;
    return 0;
}

int rope_remove(rope *rope, size_t index)
{
    rope_node *root = TREAP_ROOT(&rope->trt);
    if (!root || index >= root->size)
        return -1;

    rope_node *l = NULL, *mid_r = NULL;
    rope_split(root, index, &l, &mid_r);

    rope_node *mid = NULL, *r = NULL;
    rope_split(mid_r, 1, &mid, &r);

    rope_node *merged = rope_merge(l, r);
    TREAP_ROOT(&rope->trt) = merged;
    if (merged)
        TREAP_PARENT(merged, link) = NULL;

    free(mid);
    return 0;
}

char rope_query(rope *rope, size_t index)
{
    rope_node *node = TREAP_ROOT(&rope->trt);
    if (!node || index >= node->size)
        return '\0';

    while (node) {
        size_t left_sz = TREAP_LEFT(node, link) ? TREAP_LEFT(node, link)->size : 0;
        if (index == left_sz)
            return node->val;
        if (index < left_sz) {
            node = TREAP_LEFT(node, link);
        } else {
            index -= left_sz + 1;
            node = TREAP_RIGHT(node, link);
        }
    }
    return '\0';
}

static void destroy_node(rope_node *node)
{
    if (node) {
        destroy_node(TREAP_LEFT(node, link));
        destroy_node(TREAP_RIGHT(node, link));
        free(node);
    }
}

void rope_destroy(rope *rope)
{
    if (rope) {
        destroy_node(TREAP_ROOT(&rope->trt));
        TREAP_INIT(&rope->trt);
    }
}

static void graph_node(rope_node *node, FILE *f)
{
    if (!node)
        return;
    fprintf(f, "  \"node%p\" [fillcolor=\"#1b4f72\", label=\"{{ val: '%c' | prio: %u } | { size: %zu }}\"];\n", (void *)node,
            node->val, TREAP_PRIO(node, link), node->size);

    if (TREAP_LEFT(node, link)) {
        fprintf(f, "  \"node%p\" -> \"node%p\" [label=\"L\"];\n", (void *)node, (void *)TREAP_LEFT(node, link));
        graph_node(TREAP_LEFT(node, link), f);
    }
    if (TREAP_RIGHT(node, link)) {
        fprintf(f, "  \"node%p\" -> \"node%p\" [label=\"R\"];\n", (void *)node, (void *)TREAP_RIGHT(node, link));
        graph_node(TREAP_RIGHT(node, link), f);
    }
}

void rope_graph(rope *rope, FILE *stream)
{
    if (!stream)
        stream = stdout;
    if (rope) {
        fprintf(stream, "digraph {\n");
        fprintf(stream,
                "graph [ranksep=\"0.25\", rankdir=\"TB\", fontname=\"Monofur\", bgcolor=\"transparent\", nodesep=\"0.125\"];\n");
        fprintf(stream, "node [fontname=\"Monofur\", color=\"#cccccc\", style=\"filled\", height=\"0\", width=\"1\", "
                        "shape=\"Mrecord\", fontcolor=\"#ffffff\"];\n");
        fprintf(stream,
                "edge [fontname=\"Monofur\", color=\"#cccccc\", arrowsize=\"0.80\", penwidth=\"2.0\", fontcolor=\"#cccccc\"];\n");

        if (TREAP_ROOT(&rope->trt)) {
            graph_node(TREAP_ROOT(&rope->trt), stream);
        }
        fprintf(stream, "}\n");
    }
}
