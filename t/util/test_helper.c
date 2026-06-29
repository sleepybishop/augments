#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/interval_tree.h"
#include "src/max_subarray.h"

static int itree_print_cb(itree_node *node, void *arg)
{
    printf("%zu|%zu|%zu\n", node->id, node->low, node->high);
    return 0;
}

int main(int argc, char *argv[])
{
    itree it;
    sub_tree subt;

    interval_tree_init(&it);
    sub_tree_init(&subt);

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

        /* 6. Max Subarray Sum Tree Commands */
        else if (strncmp(buf, "MAXSUB ADD ", 11) == 0) {
            int key;
            double val;
            if (sscanf(buf + 11, "%d|%lf", &key, &val) == 2) {
                sub_tree_add(&subt, key, val);
            }
        } else if (strncmp(buf, "MAXSUB REMOVE ", 14) == 0) {
            int key;
            if (sscanf(buf + 14, "%d", &key) == 1) {
                sub_tree_remove(&subt, key);
            }
        } else if (strncmp(buf, "MAXSUB UPDATE ", 14) == 0) {
            int key;
            double val;
            if (sscanf(buf + 14, "%d|%lf", &key, &val) == 2) {
                sub_tree_update(&subt, key, val);
            }
        } else if (strncmp(buf, "MAXSUB QUERY ", 13) == 0) {
            int low, high;
            if (sscanf(buf + 13, "%d|%d", &low, &high) == 2) {
                printf("%.2f\n", sub_tree_query(&subt, low, high));
            }
        }

        else if (strcmp(buf, "GRAPH") == 0 || strcmp(buf, "ITREE GRAPH") == 0) {
            interval_tree_graph(&it, NULL);
        } else if (strcmp(buf, "MAXSUB GRAPH") == 0) {
            sub_tree_graph(&subt, NULL);
        } else if (strncmp(buf, "ECHO ", 5) == 0) {
            printf("%s\n", buf + 5);
        }
    }

    interval_tree_destroy(&it);
    sub_tree_destroy(&subt);

    return 0;
}
