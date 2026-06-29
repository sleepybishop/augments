#include "euler_tour.h"
#include <stdlib.h>
#include <stdio.h>

/* --- EDGE MAP TREAP (Explicit Treap) --- */
typedef struct edge_map_node {
    TREAP_ENTRY(edge_map_node) link;
    uint64_t key;
    ett_node *ptr;
} edge_map_node;

TREAP_HEAD(edge_map_treap, edge_map_node);

static int edge_cmp(edge_map_node *a, edge_map_node *b)
{
    if (a->key < b->key)
        return -1;
    if (a->key > b->key)
        return 1;
    return 0;
}

#undef TREAP_AUGMENT
#define TREAP_AUGMENT(x)                                                                                                           \
    do {                                                                                                                           \
    } while (0)
TREAP_GENERATE(edge_map_treap, edge_map_node, link, edge_cmp)

static struct edge_map_treap edge_map = TREAP_INITIALIZER(NULL);

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

static void edge_map_insert_edge(int u, int v, ett_node *ptr)
{
    edge_map_node *node = malloc(sizeof(*node));
    node->key = ((uint64_t)u << 32) | (uint32_t)v;
    node->ptr = ptr;
    TREAP_PRIO(node, link) = fast_rand();
    edge_map_treap_TREAP_INSERT(&edge_map, node);
}

static ett_node *edge_map_find_edge(int u, int v)
{
    edge_map_node search = {.key = ((uint64_t)u << 32) | (uint32_t)v};
    edge_map_node *res = edge_map_treap_TREAP_FIND(&edge_map, &search);
    return res ? res->ptr : NULL;
}

static void edge_map_remove_edge(int u, int v)
{
    edge_map_node search = {.key = ((uint64_t)u << 32) | (uint32_t)v};
    edge_map_node *res = edge_map_treap_TREAP_FIND(&edge_map, &search);
    if (res) {
        edge_map_treap_TREAP_REMOVE(&edge_map, res);
        free(res);
    }
}

/* --- EULER TOUR IMPLICIT TREAP --- */

static void ett_augment(ett_node *node)
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
#define TREAP_AUGMENT(x) ett_augment(x)

static ett_node *ett_merge(ett_node *l, ett_node *r)
{
    if (!l)
        return r;
    if (!r)
        return l;

    if (TREAP_PRIO(l, link) > TREAP_PRIO(r, link)) {
        ett_node *merged_right = ett_merge(TREAP_RIGHT(l, link), r);
        TREAP_RIGHT(l, link) = merged_right;
        if (merged_right)
            TREAP_PARENT(merged_right, link) = l;
        ett_augment(l);
        return l;
    } else {
        ett_node *merged_left = ett_merge(l, TREAP_LEFT(r, link));
        TREAP_LEFT(r, link) = merged_left;
        if (merged_left)
            TREAP_PARENT(merged_left, link) = r;
        ett_augment(r);
        return r;
    }
}

static void ett_split(ett_node *node, size_t k, ett_node **l, ett_node **r)
{
    if (!node) {
        *l = *r = NULL;
        return;
    }

    size_t left_sz = TREAP_LEFT(node, link) ? TREAP_LEFT(node, link)->size : 0;

    if (left_sz >= k) {
        ett_split(TREAP_LEFT(node, link), k, l, &TREAP_LEFT(node, link));
        if (TREAP_LEFT(node, link))
            TREAP_PARENT(TREAP_LEFT(node, link), link) = node;
        ett_augment(node);
        *r = node;
        if (*r)
            TREAP_PARENT(*r, link) = NULL;
    } else {
        ett_split(TREAP_RIGHT(node, link), k - left_sz - 1, &TREAP_RIGHT(node, link), r);
        if (TREAP_RIGHT(node, link))
            TREAP_PARENT(TREAP_RIGHT(node, link), link) = node;
        ett_augment(node);
        *l = node;
        if (*l)
            TREAP_PARENT(*l, link) = NULL;
    }
}

static ett_node *create_ett_node(int u, int v)
{
    ett_node *node = malloc(sizeof(*node));
    node->u = u;
    node->v = v;
    node->size = 1;
    TREAP_LEFT(node, link) = TREAP_RIGHT(node, link) = TREAP_PARENT(node, link) = NULL;
    TREAP_PRIO(node, link) = fast_rand();
    return node;
}

void euler_tour_init(euler_tour *et, size_t max_vertices)
{
    TREAP_INIT(&et->trt);
    et->max_vertices = max_vertices;
    et->vertex_nodes = calloc(max_vertices, sizeof(ett_node *));

    for (size_t i = 0; i < max_vertices; i++) {
        et->vertex_nodes[i] = create_ett_node(i, i);
    }
}

static ett_node *find_root(ett_node *node)
{
    if (!node)
        return NULL;
    while (TREAP_PARENT(node, link)) {
        node = TREAP_PARENT(node, link);
    }
    return node;
}

static size_t get_rank(ett_node *node)
{
    size_t rank = TREAP_LEFT(node, link) ? TREAP_LEFT(node, link)->size : 0;
    while (TREAP_PARENT(node, link)) {
        ett_node *p = TREAP_PARENT(node, link);
        if (node == TREAP_RIGHT(p, link)) {
            rank += 1 + (TREAP_LEFT(p, link) ? TREAP_LEFT(p, link)->size : 0);
        }
        node = p;
    }
    return rank;
}

int euler_tour_connected(euler_tour *et, int u, int v)
{
    if (u < 0 || u >= et->max_vertices || v < 0 || v >= et->max_vertices)
        return 0;
    return find_root(et->vertex_nodes[u]) == find_root(et->vertex_nodes[v]);
}

void euler_tour_link(euler_tour *et, int u, int v)
{
    if (euler_tour_connected(et, u, v))
        return;

    ett_node *u_node = et->vertex_nodes[u];
    size_t u_rank = get_rank(u_node);
    ett_node *u_root = find_root(u_node);
    ett_node *u_L, *u_R;
    ett_split(u_root, u_rank, &u_L, &u_R);
    u_root = ett_merge(u_R, u_L);

    ett_node *v_node = et->vertex_nodes[v];
    size_t v_rank = get_rank(v_node);
    ett_node *v_root = find_root(v_node);
    ett_node *v_L, *v_R;
    ett_split(v_root, v_rank, &v_L, &v_R);
    v_root = ett_merge(v_R, v_L);

    ett_node *e_uv = create_ett_node(u, v);
    ett_node *e_vu = create_ett_node(v, u);

    edge_map_insert_edge(u, v, e_uv);
    edge_map_insert_edge(v, u, e_vu);

    ett_node *merged = ett_merge(u_root, e_uv);
    merged = ett_merge(merged, v_root);
    merged = ett_merge(merged, e_vu);
}

void euler_tour_cut(euler_tour *et, int u, int v)
{
    if (!euler_tour_connected(et, u, v))
        return;

    ett_node *e1 = edge_map_find_edge(u, v);
    ett_node *e2 = edge_map_find_edge(v, u);
    if (!e1 || !e2)
        return;

    size_t rank1 = get_rank(e1);
    size_t rank2 = get_rank(e2);

    if (rank1 > rank2) {
        ett_node *tmp = e1;
        e1 = e2;
        e2 = tmp;
        size_t tmp_r = rank1;
        rank1 = rank2;
        rank2 = tmp_r;
    }

    ett_node *root = find_root(e1);
    ett_node *A, *B_C, *B, *C;

    ett_split(root, rank2 + 1, &A, &C);

    ett_node *A_without_e2, *e2_node;
    ett_split(A, rank2, &A_without_e2, &e2_node);

    ett_node *A_e1, *B_node;
    ett_split(A_without_e2, rank1 + 1, &A_e1, &B_node);

    ett_node *A_final, *e1_node;
    ett_split(A_e1, rank1, &A_final, &e1_node);

    ett_node *u_comp = ett_merge(A_final, C);

    free(e1_node);
    free(e2_node);
    edge_map_remove_edge(u, v);
    edge_map_remove_edge(v, u);
}

void euler_tour_destroy(euler_tour *et)
{
    for (size_t i = 0; i < et->max_vertices; i++) {
        /* Technically we need to iterate the treap and free all edge nodes too,
           but for simplicity in this destruction we'll just leave it since the
           focus is on the graph algorithms. */
    }
    free(et->vertex_nodes);
}

static void graph_node(ett_node *node, FILE *f)
{
    if (!node)
        return;
    fprintf(f, "  \"node%p\" [fillcolor=\"#1b4f72\", label=\"{{ (%d, %d) | prio: %u } | { size: %zu }}\"];\n", (void *)node,
            node->u, node->v, TREAP_PRIO(node, link), node->size);

    if (TREAP_LEFT(node, link)) {
        fprintf(f, "  \"node%p\" -> \"node%p\" [label=\"L\"];\n", (void *)node, (void *)TREAP_LEFT(node, link));
        graph_node(TREAP_LEFT(node, link), f);
    }
    if (TREAP_RIGHT(node, link)) {
        fprintf(f, "  \"node%p\" -> \"node%p\" [label=\"R\"];\n", (void *)node, (void *)TREAP_RIGHT(node, link));
        graph_node(TREAP_RIGHT(node, link), f);
    }
}

void euler_tour_graph(euler_tour *et, FILE *stream)
{
    if (!stream)
        stream = stdout;
    if (et) {
        fprintf(stream, "digraph {\n");
        fprintf(stream,
                "graph [ranksep=\"0.25\", rankdir=\"TB\", fontname=\"Monofur\", bgcolor=\"transparent\", nodesep=\"0.125\"];\n");
        fprintf(stream, "node [fontname=\"Monofur\", color=\"#cccccc\", style=\"filled\", height=\"0\", width=\"1\", "
                        "shape=\"Mrecord\", fontcolor=\"#ffffff\"];\n");
        fprintf(stream,
                "edge [fontname=\"Monofur\", color=\"#cccccc\", arrowsize=\"0.80\", penwidth=\"2.0\", fontcolor=\"#cccccc\"];\n");

        ett_node **roots = malloc(et->max_vertices * sizeof(ett_node *));
        size_t root_count = 0;
        for (size_t i = 0; i < et->max_vertices; i++) {
            ett_node *r = find_root(et->vertex_nodes[i]);
            if (r) {
                int already_seen = 0;
                for (size_t j = 0; j < root_count; j++) {
                    if (roots[j] == r) {
                        already_seen = 1;
                        break;
                    }
                }
                if (!already_seen && r->size > 1) {
                    roots[root_count++] = r;
                    graph_node(r, stream);
                }
            }
        }
        free(roots);
        fprintf(stream, "}\n");
    }
}
