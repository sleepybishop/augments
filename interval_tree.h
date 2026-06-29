#include <stdint.h>
#include "tree.h"

#ifndef _INTERVALTREE_H_
#define _INTERVALTREE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * http://en.wikipedia.org/wiki/Interval_tree#Augmented_tree
 * Cormen, Thomas H.; Leiserson, Charles E.; Rivest, Ronald L.; Stein, Clifford (2009)
 * Introduction to Algorithms (3rd ed.), MIT Press and McGraw-Hill, ISBN 978-0-262-03384-8
 */

typedef struct itree_node {
    size_t low;
    size_t high;
    size_t left_max;
    size_t right_max;
    size_t id;
    RB_ENTRY(itree_node) link;
} itree_node;

typedef struct {
    RB_HEAD(_INTERVAL_TREE, itree_node) rbt;
} itree;

typedef struct {
    size_t low;
    size_t high;
    size_t overlaps;
} itree_match_params;

int intervaltree_add(itree *ctx, size_t low, size_t high, size_t id);
int intervaltree_extend(itree *ctx, size_t low, size_t high, size_t id);
int intervaltree_remove(itree *ctx, size_t low, size_t high, size_t overlaps);
void intervaltree_find(itree *ctx, size_t low, size_t high, size_t overlaps);
void intervaltree_destroy(itree *ctx);
void intervaltree_graph(itree *ctx, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif /* _INTERVALTREE_H_ */
