#include "incremental_hull.h"
#include <stdlib.h>
#include <math.h>

#define EPSILON 1e-6

int inc_hull_cmp(inc_hull_node *a, inc_hull_node *b)
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

#undef RB_AUGMENT
#define RB_AUGMENT(x)                                                                                                              \
    do {                                                                                                                           \
    } while (0)

RB_GENERATE(inc_hull_rb, inc_hull_node, link, inc_hull_cmp)

void inc_hull_init(inc_hull_tree *tree)
{
        RB_INIT(&tree->rbt);
}

/* Returns > 0 if left turn (CCW), < 0 if right turn (CW), 0 if collinear */
static double ccw(inc_hull_node *a, inc_hull_node *b, inc_hull_node *c)
{
    double ab_x = b->x - a->x;
    double ab_y = b->y - a->y;
    double ac_x = c->x - a->x;
    double ac_y = c->y - a->y;
    return ab_x * ac_y - ab_y * ac_x;
}

void inc_hull_insert(inc_hull_tree *tree, inc_hull_node *p, double x, double y)
{
        p->x = x;
    p->y = y;

    inc_hull_node *conflict = inc_hull_rb_RB_INSERT(&tree->rbt, p);
    if (conflict) {
        if (p->y > conflict->y) {
            inc_hull_rb_RB_REMOVE(&tree->rbt, conflict);
                        inc_hull_rb_RB_INSERT(&tree->rbt, p);
        } else {
                        return;
        }
    }

    inc_hull_node *L = RB_PREV(inc_hull_rb, &tree->rbt, p);
    inc_hull_node *R = RB_NEXT(inc_hull_rb, &tree->rbt, p);

    /* If p is swallowed by the line L->R (meaning ccw > -EPSILON), remove p */
    if (L && R && ccw(L, p, R) >= -EPSILON) {
        inc_hull_rb_RB_REMOVE(&tree->rbt, p);
                return;
    }

    /* Eliminate obsolete left neighbors */
    while ((L = RB_PREV(inc_hull_rb, &tree->rbt, p)) != NULL) {
        inc_hull_node *L2 = RB_PREV(inc_hull_rb, &tree->rbt, L);
        if (L2 && ccw(L2, L, p) >= -EPSILON) {
            inc_hull_rb_RB_REMOVE(&tree->rbt, L);
                    } else {
            break;
        }
    }

    /* Eliminate obsolete right neighbors */
    while ((R = RB_NEXT(inc_hull_rb, &tree->rbt, p)) != NULL) {
        inc_hull_node *R2 = RB_NEXT(inc_hull_rb, &tree->rbt, R);
        if (R2 && ccw(p, R, R2) >= -EPSILON) {
            inc_hull_rb_RB_REMOVE(&tree->rbt, R);
                    } else {
            break;
        }
    }
}

static void destroy_node(inc_hull_tree *tree, inc_hull_node *node)
{
    while (node) {
        if (RB_LEFT(node, link)) {
            inc_hull_node *l = RB_LEFT(node, link);
            RB_LEFT(node, link) = RB_RIGHT(l, link);
            RB_RIGHT(l, link) = node;
            node = l;
        } else {
            inc_hull_node *r = RB_RIGHT(node, link);
                        node = r;
        }
    }
}

void inc_hull_destroy(inc_hull_tree *tree)
{
    destroy_node(tree, RB_ROOT(&tree->rbt));
    RB_INIT(&tree->rbt);
}

static void graph_node(inc_hull_node *node, FILE *f)
{
        fprintf(f, "  \"node%p\" [fillcolor=\"%s\", label=\"(%.1f, %.1f)\"];\n", (void *)node,
            RB_COLOR(node, link) == RB_BLACK ? "#000000" : "#aa0000", node->x, node->y);
    if (RB_LEFT(node, link)) {
        fprintf(f, "  \"node%p\" -> \"node%p\" [label=\"L\"];\n", (void *)node, (void *)RB_LEFT(node, link));
        graph_node(RB_LEFT(node, link), f);
    }
    if (RB_RIGHT(node, link)) {
        fprintf(f, "  \"node%p\" -> \"node%p\" [label=\"R\"];\n", (void *)node, (void *)RB_RIGHT(node, link));
        graph_node(RB_RIGHT(node, link), f);
    }
}

void inc_hull_graph(inc_hull_tree *tree, FILE *stream)
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
