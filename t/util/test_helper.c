#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/interval_tree.h"
#include "src/max_subarray.h"
#include "src/order_statistic.h"
#include "src/priority_search.h"
#include "src/range_min.h"
#include "src/range_sum.h"
#include "src/hash_tree.h"
#include "src/lcp_tree.h"
#include "src/rope.h"

static int itree_print_cb(itree_node *node, void *arg)
{
    printf("%zu|%zu|%zu\n", node->id, node->low, node->high);
    return 0;
}

static void ps_print_cb(ps_node *node, void *arg)
{
    printf("%.2f|%.2f\n", node->x, node->y);
}

int main(int argc, char *argv[])
{
    itree it;
    sub_tree subt;
    os_tree ost;
    ps_tree pst;
    min_tree mt;
    sum_tree st;
    hash_tree_tree ht;
    lcp_tree lcpt;
    rope ropt;

    interval_tree_init(&it);
    sub_tree_init(&subt);
    os_tree_init(&ost);
    ps_tree_init(&pst);
    min_tree_init(&mt);
    sum_tree_init(&st);
    hash_tree_init(&ht);
    lcp_tree_init(&lcpt);
    rope_init(&ropt);

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

        /* 2. Order-Statistic Tree Commands */
        else if (strncmp(buf, "OSTREE ADD ", 11) == 0) {
            int key;
            if (sscanf(buf + 11, "%d", &key) == 1) {
                os_tree_add(&ost, key);
            }
        } else if (strncmp(buf, "OSTREE REMOVE ", 14) == 0) {
            int key;
            if (sscanf(buf + 14, "%d", &key) == 1) {
                os_tree_remove(&ost, key);
            }
        } else if (strncmp(buf, "OSTREE SELECT ", 14) == 0) {
            size_t rank;
            if (sscanf(buf + 14, "%zu", &rank) == 1) {
                os_node *n = os_tree_select(&ost, rank);
                if (n) {
                    printf("%d\n", n->key);
                } else {
                    printf("NULL\n");
                }
            }
        } else if (strncmp(buf, "OSTREE RANK ", 12) == 0) {
            int key;
            if (sscanf(buf + 12, "%d", &key) == 1) {
                size_t r = os_tree_rank(&ost, key);
                if (r == (size_t)-1) {
                    printf("NOT_FOUND\n");
                } else {
                    printf("%zu\n", r);
                }
            }
        }

        /* 3. Range Sum Tree Commands */
        else if (strncmp(buf, "SUMTREE ADD ", 12) == 0) {
            int key;
            double val;
            if (sscanf(buf + 12, "%d|%lf", &key, &val) == 2) {
                sum_tree_add(&st, key, val);
            }
        } else if (strncmp(buf, "SUMTREE REMOVE ", 15) == 0) {
            int key;
            if (sscanf(buf + 15, "%d", &key) == 1) {
                sum_tree_remove(&st, key);
            }
        } else if (strncmp(buf, "SUMTREE UPDATE ", 15) == 0) {
            int key;
            double val;
            if (sscanf(buf + 15, "%d|%lf", &key, &val) == 2) {
                sum_tree_update(&st, key, val);
            }
        } else if (strncmp(buf, "SUMTREE QUERY ", 14) == 0) {
            int low, high;
            if (sscanf(buf + 14, "%d|%d", &low, &high) == 2) {
                printf("%.2f\n", sum_tree_query(&st, low, high));
            }
        }

        /* 4. Range Minimum Tree Commands */
        else if (strncmp(buf, "MINTREE ADD ", 12) == 0) {
            int key;
            double val;
            if (sscanf(buf + 12, "%d|%lf", &key, &val) == 2) {
                min_tree_add(&mt, key, val);
            }
        } else if (strncmp(buf, "MINTREE REMOVE ", 15) == 0) {
            int key;
            if (sscanf(buf + 15, "%d", &key) == 1) {
                min_tree_remove(&mt, key);
            }
        } else if (strncmp(buf, "MINTREE UPDATE ", 15) == 0) {
            int key;
            double val;
            if (sscanf(buf + 15, "%d|%lf", &key, &val) == 2) {
                min_tree_update(&mt, key, val);
            }
        } else if (strncmp(buf, "MINTREE QUERY ", 14) == 0) {
            int low, high;
            if (sscanf(buf + 14, "%d|%d", &low, &high) == 2) {
                double res = min_tree_query(&mt, low, high);
                if (res == (1.0 / 0.0)) {
                    printf("INFINITY\n");
                } else {
                    printf("%.2f\n", res);
                }
            }
        }

        /* 5. Priority Search Tree Commands */
        else if (strncmp(buf, "PSTREE ADD ", 11) == 0) {
            double x, y;
            if (sscanf(buf + 11, "%lf|%lf", &x, &y) == 2) {
                ps_tree_add(&pst, x, y);
            }
        } else if (strncmp(buf, "PSTREE REMOVE ", 14) == 0) {
            double x, y;
            if (sscanf(buf + 14, "%lf|%lf", &x, &y) == 2) {
                ps_tree_remove(&pst, x, y);
            }
        } else if (strncmp(buf, "PSTREE QUERY ", 13) == 0) {
            double x_min, x_max, y_min;
            if (sscanf(buf + 13, "%lf|%lf|%lf", &x_min, &x_max, &y_min) == 3) {
                ps_tree_query(&pst, x_min, x_max, y_min, ps_print_cb, NULL);
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

        /* 7. hash_tree Commands */
        else if (strncmp(buf, "HASHTREE ADD ", 13) == 0) {
            size_t key;
            char val;
            if (sscanf(buf + 13, "%zu|%c", &key, &val) == 2) {
                hash_tree_insert(&ht, key, val);
            }
        } else if (strncmp(buf, "HASHTREE REMOVE ", 16) == 0) {
            size_t key;
            if (sscanf(buf + 16, "%zu", &key) == 1) {
                hash_tree_remove(&ht, key);
            }
        } else if (strcmp(buf, "HASHTREE HASH") == 0) {
            printf("%llu\n", (unsigned long long)hash_tree_hash(&ht));
        }

        /* 8. LCP Commands */
        else if (strncmp(buf, "LCP ADD ", 8) == 0) {
            lcp_tree_add(&lcpt, buf + 8);
        } else if (strncmp(buf, "LCP REMOVE ", 11) == 0) {
            lcp_tree_remove(&lcpt, buf + 11);
        } else if (strcmp(buf, "LCP QUERY") == 0) {
            printf("%zu\n", lcp_tree_query(&lcpt));
        }

        /* 9. Rope Commands */
        else if (strncmp(buf, "ROPE ADD ", 9) == 0) {
            size_t idx;
            char val;
            if (sscanf(buf + 9, "%zu|%c", &idx, &val) == 2) {
                rope_insert(&ropt, idx, val);
            }
        } else if (strncmp(buf, "ROPE REMOVE ", 12) == 0) {
            size_t idx;
            if (sscanf(buf + 12, "%zu", &idx) == 1) {
                rope_remove(&ropt, idx);
            }
        } else if (strncmp(buf, "ROPE QUERY ", 11) == 0) {
            size_t idx;
            if (sscanf(buf + 11, "%zu", &idx) == 1) {
                char res = rope_query(&ropt, idx);
                if (res) {
                    printf("%c\n", res);
                } else {
                    printf("\n");
                }
            }
        }

        else if (strcmp(buf, "GRAPH") == 0 || strcmp(buf, "ITREE GRAPH") == 0) {
            interval_tree_graph(&it, NULL);
        } else if (strcmp(buf, "INCHULL GRAPH") == 0) {
            inc_hull_graph(&iht, NULL);
        } else if (strcmp(buf, "DYNHULL GRAPH") == 0) {
            dyn_hull_graph(&dht, NULL);
        } else if (strcmp(buf, "EULER GRAPH") == 0) {
            euler_tour_graph(&ett, NULL);
        } else if (strcmp(buf, "OSTREE GRAPH") == 0) {
            os_tree_graph(&ost, NULL);
        } else if (strcmp(buf, "SUMTREE GRAPH") == 0) {
            sum_tree_graph(&st, NULL);
        } else if (strcmp(buf, "MINTREE GRAPH") == 0) {
            min_tree_graph(&mt, NULL);
        } else if (strcmp(buf, "PSTREE GRAPH") == 0) {
            ps_tree_graph(&pst, NULL);
        } else if (strcmp(buf, "MAXSUB GRAPH") == 0) {
            sub_tree_graph(&subt, NULL);
        } else if (strcmp(buf, "HASHTREE GRAPH") == 0) {
            hash_tree_graph(&ht, NULL);
        } else if (strcmp(buf, "LCP GRAPH") == 0) {
            lcp_tree_graph(&lcpt, NULL);
        } else if (strcmp(buf, "ROPE GRAPH") == 0) {
            rope_graph(&ropt, NULL);
        } else if (strncmp(buf, "ECHO ", 5) == 0) {
            printf("%s\n", buf + 5);
        }
    }

    interval_tree_destroy(&it);
    sub_tree_destroy(&subt);
    os_tree_destroy(&ost);
    ps_tree_destroy(&pst);
    min_tree_destroy(&mt);
    sum_tree_destroy(&st);
    hash_tree_destroy(&ht);
    lcp_tree_destroy(&lcpt);
    rope_destroy(&ropt);

    return 0;
}
