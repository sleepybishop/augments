#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "interval_tree.h"

int main(int argc, char *argv[])
{
    itree it = {0};
    char buf[4096] = {0};
    size_t i = 0, j = 0;

    while (fgets(buf, sizeof(buf), stdin)) {
        size_t id;
        size_t low = 0, high = 0;

        int nf = sscanf(buf, "%zu|%zu|%zu\n", &id, &low, &high);

        //	fprintf(stderr, "NF[%d]: %d|%f|%f\n", nf, id, low, high);
        if (nf == 3) {
            intervaltree_add(&it, (double)low, (double)high, id);
        }
    }

    intervaltree_graph(&it, NULL);

    size_t start = 0, end = 0;
    // intervaltree_find(&it, start, end);
    intervaltree_destroy(&it);
}
