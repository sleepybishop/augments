#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/interval_tree.h"

static int itree_print_cb(itree_node *node, void *arg)
{
    printf("%zu|%zu|%zu\n", node->id, node->low, node->high);
    return 0;
}

int main(int argc, char *argv[])
{
    itree it;

    interval_tree_init(&it);

    char buf[4096];

    while (fgets(buf, sizeof(buf), stdin)) {
        /* Strip trailing newline/carriage returns */
        buf[strcspn(buf, "\r\n")] = '\0';
        if (strlen(buf) == 0)
            continue;

        /* 1. Interval Tree Commands */
        if (strncmp(buf, "ITREE ADD ", 10) == 0) {
            size_t id, low, high;
            if (sscanf(buf + 10, "%zu|%zu|%zu", &id, &low, &high) == 3) {
                interval_tree_add(&it, low, high, id);
            }
        } else if (strncmp(buf, "ITREE REMOVE ", 13) == 0) {
            size_t low, high, id;
            if (sscanf(buf + 13, "%zu|%zu|%zu", &low, &high, &id) == 3) {
                interval_tree_remove(&it, low, high, id);
            }
        } else if (strncmp(buf, "ITREE FIND ", 11) == 0) {
            size_t low, high;
            int overlaps;
            if (sscanf(buf + 11, "%zu|%zu|%d", &low, &high, &overlaps) == 3) {
                interval_tree_find(&it, low, high, overlaps, itree_print_cb, NULL);
            }
        }

        else if (strcmp(buf, "GRAPH") == 0 || strcmp(buf, "ITREE GRAPH") == 0) {
            interval_tree_graph(&it, NULL);
        } else if (strncmp(buf, "ECHO ", 5) == 0) {
            printf("%s\n", buf + 5);
        }
    }

    interval_tree_destroy(&it);

    return 0;
}
