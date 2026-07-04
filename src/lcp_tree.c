#include "lcp_tree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void lcp_augment(lcp_node *node);
#define RB_AUGMENT(x) lcp_augment(x)
#include "tree.h"

static inline int lcp_cmp(lcp_node *a, lcp_node *b)
{
    return strcmp(a->str, b->str);
}

static size_t calc_lcp(const char *a, const char *b)
{
    if (!a || !b)
        return 0;
    size_t i = 0;
    while (a[i] && b[i] && a[i] == b[i])
        i++;
    return i;
}

static void lcp_augment(lcp_node *node)
{
    while (node) {
        char *min_s = node->str;
        char *max_s = node->str;

        if (RB_LEFT(node, link)) {
            min_s = RB_LEFT(node, link)->min_str;
        }
        if (RB_RIGHT(node, link)) {
            max_s = RB_RIGHT(node, link)->max_str;
        }

        size_t l = calc_lcp(min_s, max_s);

        if (node->min_str == min_s && node->max_str == max_s && node->lcp == l)
            break;

        node->min_str = min_s;
        node->max_str = max_s;
        node->lcp = l;
        node = RB_PARENT(node, link);
    }
}

RB_GENERATE(_LCP, lcp_node, link, lcp_cmp)

void lcp_tree_init(lcp_tree *tree)
{
    if (!tree)
        return;
        RB_INIT(&tree->rbt);
}

int lcp_tree_add(lcp_tree *tree, lcp_node *node, const char *str)
{
    if (!tree || !str)
        return -1;
            node->str = (char *)str;
            node->min_str = node->str;
    node->max_str = node->str;
    node->lcp = strlen(str);

    lcp_node *f = RB_INSERT(_LCP, &tree->rbt, node);
    if (f) {
                        return 1; /* Duplicate */
    }
    return 0;
}

int lcp_tree_remove(lcp_tree *tree, const char *str)
{
    lcp_node search;
    search.str = (char *)str;
    lcp_node *node = RB_FIND(_LCP, &tree->rbt, &search);
    if (node) {
        RB_REMOVE(_LCP, &tree->rbt, node);
        free(node->str);
                return 1;
    }
    return 0;
}

size_t lcp_tree_query(lcp_tree *tree)
{
    lcp_node *root = RB_ROOT(&tree->rbt);
    return root ? root->lcp : 0;
}

static void destroy_node(lcp_tree *tree, lcp_node *node)
{
    while (node) {
        if (RB_LEFT(node, link)) {
            lcp_node *l = RB_LEFT(node, link);
            RB_LEFT(node, link) = RB_RIGHT(l, link);
            RB_RIGHT(l, link) = node;
            node = l;
        } else {
            lcp_node *r = RB_RIGHT(node, link);
                                    node = r;
        }
    }
}

void lcp_tree_destroy(lcp_tree *tree)
{
    if (tree) {
        destroy_node(tree, RB_ROOT(&tree->rbt));
        RB_INIT(&tree->rbt);
    }
}

static void graph_node(lcp_node *node, FILE *f)
{
        fprintf(f, "  \"node%p\" [fillcolor=\"%s\", label=\"{{ str: '%s' | lcp: %zu } | { min: '%s' | max: '%s' }}\"];\n", (void *)node,
            RB_COLOR(node, link) == RB_BLACK ? "#000000" : "#aa0000", node->str, node->lcp, node->min_str, node->max_str);

    if (RB_LEFT(node, link)) {
        fprintf(f, "  \"node%p\" -> \"node%p\" [label=\"L\"];\n", (void *)node, (void *)RB_LEFT(node, link));
        graph_node(RB_LEFT(node, link), f);
    }
    if (RB_RIGHT(node, link)) {
        fprintf(f, "  \"node%p\" -> \"node%p\" [label=\"R\"];\n", (void *)node, (void *)RB_RIGHT(node, link));
        graph_node(RB_RIGHT(node, link), f);
    }
}

void lcp_tree_graph(lcp_tree *tree, FILE *stream)
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
