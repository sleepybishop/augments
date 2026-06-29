#ifndef TRUE_ROPE_H
#define TRUE_ROPE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "treap.h"

typedef struct rope_node {
    TREAP_ENTRY(rope_node) link;
    char val;
    size_t size;
} rope_node;

typedef struct rope {
    TREAP_HEAD(rope_treap, rope_node) trt;
} rope;

void rope_init(rope *rope);
int rope_insert(rope *rope, size_t index, char val);
int rope_remove(rope *rope, size_t index);
char rope_query(rope *rope, size_t index);
void rope_destroy(rope *rope);
void rope_graph(rope *rope, FILE *stream);

#endif
