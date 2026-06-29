#include "dynamic_hull.h"
#include <stdlib.h>

static int dyn_hull_cmp(dyn_hull_node *a, dyn_hull_node *b)
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

static double ccw(dyn_hull_node *a, dyn_hull_node *b, dyn_hull_node *c)
{
    return (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
}

static dyn_hull_node *tangent_from_right(dyn_hull_node *curr, dyn_hull_node *q)
{
    dyn_hull_node *best = curr;
    while (curr) {
        if (!TREAP_LEFT(curr, link) && !TREAP_RIGHT(curr, link))
            return curr;

        if (curr->is_swallowed) {
            if (ccw(curr->L_tangent, curr->R_tangent, q) > 0) {
                best = curr->L_tangent;
                curr = TREAP_LEFT(curr, link);
            } else {
                best = curr->R_tangent;
                curr = TREAP_RIGHT(curr, link);
            }
        } else {
            if (TREAP_LEFT(curr, link) && ccw(curr->L_tangent, curr, q) > 0) {
                best = curr->L_tangent;
                curr = TREAP_LEFT(curr, link);
            } else if (TREAP_RIGHT(curr, link) && ccw(curr, curr->R_tangent, q) <= 0) {
                best = curr->R_tangent;
                curr = TREAP_RIGHT(curr, link);
            } else {
                return curr;
            }
        }
    }
    return best;
}

static dyn_hull_node *tangent_from_left(dyn_hull_node *curr, dyn_hull_node *q)
{
    dyn_hull_node *best = curr;
    while (curr) {
        if (!TREAP_LEFT(curr, link) && !TREAP_RIGHT(curr, link))
            return curr;

        if (curr->is_swallowed) {
            if (ccw(curr->L_tangent, curr->R_tangent, q) > 0) {
                best = curr->R_tangent;
                curr = TREAP_RIGHT(curr, link);
            } else {
                best = curr->L_tangent;
                curr = TREAP_LEFT(curr, link);
            }
        } else {
            if (TREAP_RIGHT(curr, link) && ccw(curr, curr->R_tangent, q) > 0) {
                best = curr->R_tangent;
                curr = TREAP_RIGHT(curr, link);
            } else if (TREAP_LEFT(curr, link) && ccw(curr->L_tangent, curr, q) <= 0) {
                best = curr->L_tangent;
                curr = TREAP_LEFT(curr, link);
            } else {
                return curr;
            }
        }
    }
    return best;
}

static void get_common_tangent(dyn_hull_node *L, dyn_hull_node *R, dyn_hull_node **out_l, dyn_hull_node **out_r)
{
    dyn_hull_node *ta = L->max_x_node;
    dyn_hull_node *tb = R->min_x_node;

    for (int i = 0; i < 40; i++) {
        dyn_hull_node *next_ta = tangent_from_right(L, tb);
        dyn_hull_node *next_tb = tangent_from_left(R, next_ta);
        if (next_ta == ta && next_tb == tb)
            break;
        ta = next_ta;
        tb = next_tb;
    }
    *out_l = ta;
    *out_r = tb;
}

static void dyn_hull_augment(dyn_hull_node *node)
{
    if (!node)
        return;

    node->min_x_node = node;
    if (TREAP_LEFT(node, link))
        node->min_x_node = TREAP_LEFT(node, link)->min_x_node;

    node->max_x_node = node;
    if (TREAP_RIGHT(node, link))
        node->max_x_node = TREAP_RIGHT(node, link)->max_x_node;

    if (!TREAP_LEFT(node, link) && !TREAP_RIGHT(node, link)) {
        node->is_swallowed = 0;
        node->L_tangent = node->R_tangent = NULL;
        return;
    }

    if (!TREAP_LEFT(node, link)) {
        node->is_swallowed = 0;
        node->L_tangent = NULL;
        node->R_tangent = tangent_from_left(TREAP_RIGHT(node, link), node);
        return;
    }

    if (!TREAP_RIGHT(node, link)) {
        node->is_swallowed = 0;
        node->L_tangent = tangent_from_right(TREAP_LEFT(node, link), node);
        node->R_tangent = NULL;
        return;
    }

    dyn_hull_node *ta, *tb;
    get_common_tangent(TREAP_LEFT(node, link), TREAP_RIGHT(node, link), &ta, &tb);

    if (ccw(ta, tb, node) <= 0) {
        node->is_swallowed = 1;
        node->L_tangent = ta;
        node->R_tangent = tb;
    } else {
        node->is_swallowed = 0;
        node->L_tangent = tangent_from_right(TREAP_LEFT(node, link), node);
        node->R_tangent = tangent_from_left(TREAP_RIGHT(node, link), node);
    }
}

#undef TREAP_AUGMENT
#define TREAP_AUGMENT(x) dyn_hull_augment(x)

TREAP_GENERATE(dyn_hull_treap, dyn_hull_node, link, dyn_hull_cmp)

void dyn_hull_init(dyn_hull_tree *tree)
{
    TREAP_INIT(&tree->trt);
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

void dyn_hull_insert(dyn_hull_tree *tree, double x, double y)
{
    dyn_hull_node search = {.x = x, .y = y};
    if (dyn_hull_treap_TREAP_FIND(&tree->trt, &search))
        return;

    dyn_hull_node *p = malloc(sizeof(*p));
    p->x = x;
    p->y = y;
    TREAP_PRIO(p, link) = fast_rand();
    dyn_hull_treap_TREAP_INSERT(&tree->trt, p);
}

void dyn_hull_remove(dyn_hull_tree *tree, double x, double y)
{
    dyn_hull_node search = {.x = x, .y = y};
    dyn_hull_node *res = dyn_hull_treap_TREAP_FIND(&tree->trt, &search);
    if (res) {
        dyn_hull_treap_TREAP_REMOVE(&tree->trt, res);
        free(res);
    }
}

static void print_hull(dyn_hull_node *node, FILE *stream)
{
    if (!node)
        return;

    if (node->is_swallowed) {
        /* The node itself is skipped. We only traverse the bridge endpoints.
           But wait, the hull of the subtree is:
           Hull of left up to L_tangent, then the edge L_tangent->R_tangent, then Hull of right starting from R_tangent.
           Actually, to just print the overall tree's hull, we only call print_hull from the ROOT.
           To properly trace the hull, we need a different recursive logic. */
    }
}

static void collect_hull(dyn_hull_node *node, dyn_hull_node *min_node, dyn_hull_node *max_node, FILE *stream)
{
    if (!node)
        return;

    double min_x = min_node ? min_node->x : -1e18;
    double max_x = max_node ? max_node->x : 1e18;

    if (node->is_swallowed) {
        if (node->L_tangent) {
            dyn_hull_node *actual_max = (max_node && max_node->x < node->L_tangent->x) ? max_node : node->L_tangent;
            if (min_x <= actual_max->x) {
                collect_hull(TREAP_LEFT(node, link), min_node, actual_max, stream);
            }
        }

        if (node->R_tangent) {
            dyn_hull_node *actual_min = (min_node && min_node->x > node->R_tangent->x) ? min_node : node->R_tangent;
            if (actual_min->x <= max_x) {
                collect_hull(TREAP_RIGHT(node, link), actual_min, max_node, stream);
            }
        }
    } else {
        if (node->L_tangent) {
            dyn_hull_node *actual_max = (max_node && max_node->x < node->L_tangent->x) ? max_node : node->L_tangent;
            if (min_x <= actual_max->x) {
                collect_hull(TREAP_LEFT(node, link), min_node, actual_max, stream);
            }
        }

        if (node->x >= min_x && node->x <= max_x) {
            int skip = 0;
            if (min_node && node->x == min_x && node != min_node)
                skip = 1;
            if (max_node && node->x == max_x && node != max_node)
                skip = 1;
            if (!skip) {
                fprintf(stream, "%.1f %.1f\n", node->x, node->y);
            }
        }

        if (node->R_tangent) {
            dyn_hull_node *actual_min = (min_node && min_node->x > node->R_tangent->x) ? min_node : node->R_tangent;
            if (actual_min->x <= max_x) {
                collect_hull(TREAP_RIGHT(node, link), actual_min, max_node, stream);
            }
        }
    }
}

void dyn_hull_query(dyn_hull_tree *tree, FILE *stream)
{
    dyn_hull_node *root = TREAP_ROOT(&tree->trt);
    if (root) {
        collect_hull(root, NULL, NULL, stream);
    }
}

static void destroy_node(dyn_hull_node *node)
{
    if (node) {
        destroy_node(TREAP_LEFT(node, link));
        destroy_node(TREAP_RIGHT(node, link));
        free(node);
    }
}

void dyn_hull_destroy(dyn_hull_tree *tree)
{
    destroy_node(TREAP_ROOT(&tree->trt));
    TREAP_INIT(&tree->trt);
}

static void graph_node(dyn_hull_node *node, FILE *f)
{
    if (!node)
        return;
    fprintf(f, "  \"node%p\" [fillcolor=\"#1b4f72\", label=\"{{ (%.1f, %.1f) | prio: %u } | { swallowed: %d }}\"];\n", (void *)node,
            node->x, node->y, TREAP_PRIO(node, link), node->is_swallowed);

    if (TREAP_LEFT(node, link)) {
        fprintf(f, "  \"node%p\" -> \"node%p\" [label=\"L\"];\n", (void *)node, (void *)TREAP_LEFT(node, link));
        graph_node(TREAP_LEFT(node, link), f);
    }
    if (TREAP_RIGHT(node, link)) {
        fprintf(f, "  \"node%p\" -> \"node%p\" [label=\"R\"];\n", (void *)node, (void *)TREAP_RIGHT(node, link));
        graph_node(TREAP_RIGHT(node, link), f);
    }
}

void dyn_hull_graph(dyn_hull_tree *tree, FILE *stream)
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

        if (TREAP_ROOT(&tree->trt)) {
            graph_node(TREAP_ROOT(&tree->trt), stream);
        }
        fprintf(stream, "}\n");
    }
}
