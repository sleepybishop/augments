# Augmented Search Trees

[#CI](https://github.com/sleepybishop/interval_trees/actions/workflows/ci.yml)

A modular C library implementing **11 advanced tree augmentations** built on top of robust, production-grade base tree templates:
1. **Red-Black Trees ([src/tree.h](file:///home/joe/src/sleepybishop/augments/src/tree.h))**: Upgraded OpenBSD Red-Black tree macro library.
2. **Treaps ([src/treap.h](file:///home/joe/src/sleepybishop/augments/src/treap.h))**: BSD-style randomized binary search tree template supporting split and merge.

Every augmentation maintains $O(\log n)$ insertion, deletion, and rotation complexity by computing subtree-local properties on the fly.

---

## Repository Structure

*   [src/](file:///home/joe/src/sleepybishop/augments/src/): Source code for all base templates and augmentations.
*   [t/](file:///home/joe/src/sleepybishop/augments/t/): Comprehensive test suite running under Valgrind.
*   [t/util/test_helper.c](file:///home/joe/src/sleepybishop/augments/t/util/test_helper.c): Unified interactive CLI harness for testing and visualization.

---

## Tree Derivation Map

Below is a map of the base tree templates in the library and the modules derived by augmenting them:

### 1. Base Red-Black Tree Template (`tree.h`)

*   **[Interval Tree](file:///home/joe/src/sleepybishop/augments/src/interval_tree.h)** (`ITREE`): Augments the tree with a `max` field representing the maximum high endpoint in the subtree, enabling optimal range overlap and containment searches.
*   **[Order-Statistic Tree](file:///home/joe/src/sleepybishop/augments/src/order_statistic.h)** (`OSTREE`): Augments the tree with a `size` field representing the number of nodes in the subtree, enabling rank and selection queries.
*   **[Range Sum Tree](file:///home/joe/src/sleepybishop/augments/src/range_sum.h)** (`SUMTREE`): Augments the tree with a `sum` field representing the sum of node values in the subtree, enabling dynamic prefix and range sum queries.
*   **[Range Minimum Tree](file:///home/joe/src/sleepybishop/augments/src/range_min.h)** (`MINTREE`): Augments the tree with a `min_val` field representing the minimum value in the subtree, enabling dynamic range minimum queries (RMQ).
*   **[Priority Search Tree](file:///home/joe/src/sleepybishop/augments/src/priority_search.h)** (`PSTREE`): Stores 2D points (x, y) ordered by x (as BST key) and augments each node with the maximum y in its subtree, enabling 3-sided range queries.
*   **[Max Subarray Sum Tree](file:///home/joe/src/sleepybishop/augments/src/max_subarray.h)** (`MAXSUB`): Stores a sequence of values and augments each node with subtree-level sum, max_prefix, max_suffix, and max_sub metrics, enabling range maximum subarray sum queries.
*   **[Hash Tree](file:///home/joe/src/sleepybishop/augments/src/hash_tree.h)** (`HASHTREE`): Augments the tree with a rolling hash of the sequence to support fast substring equality checks.
*   **[LCP Tree](file:///home/joe/src/sleepybishop/augments/src/lcp_tree.h)** (`LCPTREE`): Augments the tree with a longest common prefix of string keys, enabling optimal prefix searches.
*   **[Incremental Convex Hull](file:///home/joe/src/sleepybishop/augments/src/incremental_hull.h)** (`INCHULL`): Maintains the upper boundary of a 2D convex hull dynamically by adding points in left-to-right order.

### 2. Base Treap Template (`treap.h`)

*   **[Rope](file:///home/joe/src/sleepybishop/augments/src/rope.h)** (`ROPE`): Augments a Treap with subtree sizes to support character sequences and fast split/merge indexing.
*   **[Euler Tour Tree](file:///home/joe/src/sleepybishop/augments/src/euler_tour.h)** (`EULER`): Augments a Treap to represent forest structures via Euler tours, supporting dynamic link, cut, and connectivity queries.

---

## Build & Test

To build the test helper and run the entire test suite under Valgrind:
```bash
make check
```

To format the codebase:
```bash
make indent
```

To generate transparent PNG visualizations under `images/` for all active modules:
```bash
make images
```

---

## Module Reference

### 1. Interval Tree (`ITREE`)
Augments the tree with a `max` field representing the maximum high endpoint in the subtree, enabling optimal range overlap and containment searches.

*   **Header**: [src/interval_tree.h](file:///home/joe/src/sleepybishop/augments/src/interval_tree.h)
*   **Implementation**: [src/interval_tree.c](file:///home/joe/src/sleepybishop/augments/src/interval_tree.c)
*   **Key API**:
    *   `void interval_tree_init(itree *tree, tree_allocator *alloc)`
    *   `int interval_tree_add(itree *tree, size_t low, size_t high, size_t id)`
    *   `int interval_tree_remove(itree *tree, size_t low, size_t high, size_t id)`
    *   `void interval_tree_find(itree *tree, size_t low, size_t high, int overlaps, itree_cb cb, void *arg)`
    *   `void interval_tree_graph(itree *tree, FILE *stream)`

![Interval Tree](images/interval_tree.png)

---

### 2. Order-Statistic Tree (`OSTREE`)
Augments the tree with a `size` field representing the number of nodes in the subtree, enabling rank and selection queries.

*   **Header**: [src/order_statistic.h](file:///home/joe/src/sleepybishop/augments/src/order_statistic.h)
*   **Implementation**: [src/order_statistic.c](file:///home/joe/src/sleepybishop/augments/src/order_statistic.c)
*   **Key API**:
    *   `void os_tree_init(os_tree *tree, tree_allocator *alloc)`
    *   `int os_tree_add(os_tree *tree, int key)`
    *   `int os_tree_remove(os_tree *tree, int key)`
    *   `os_node *os_tree_select(os_tree *tree, size_t rank)`
    *   `size_t os_tree_rank(os_tree *tree, int key)`
    *   `void os_tree_graph(os_tree *tree, FILE *stream)`

![Order-Statistic Tree](images/order_statistic.png)

---

### 3. Range Sum Tree (`SUMTREE`)
Augments the tree with a `sum` field representing the sum of node values in the subtree, enabling dynamic prefix and range sum queries.

*   **Header**: [src/range_sum.h](file:///home/joe/src/sleepybishop/augments/src/range_sum.h)
*   **Implementation**: [src/range_sum.c](file:///home/joe/src/sleepybishop/augments/src/range_sum.c)
*   **Key API**:
    *   `void sum_tree_init(sum_tree *tree, tree_allocator *alloc)`
    *   `int sum_tree_add(sum_tree *tree, int key, double val)`
    *   `int sum_tree_remove(sum_tree *tree, int key)`
    *   `void sum_tree_update(sum_tree *tree, int key, double new_val)`
    *   `double sum_tree_query(sum_tree *tree, int low_key, int high_key)`
    *   `void sum_tree_graph(sum_tree *tree, FILE *stream)`

![Range Sum Tree](images/range_sum.png)

---

### 4. Range Minimum Tree (`MINTREE`)
Augments the tree with a `min_val` field representing the minimum value in the subtree, enabling dynamic range minimum queries (RMQ).

*   **Header**: [src/range_min.h](file:///home/joe/src/sleepybishop/augments/src/range_min.h)
*   **Implementation**: [src/range_min.c](file:///home/joe/src/sleepybishop/augments/src/range_min.c)
*   **Key API**:
    *   `void min_tree_init(min_tree *tree, tree_allocator *alloc)`
    *   `int min_tree_add(min_tree *tree, int key, double val)`
    *   `int min_tree_remove(min_tree *tree, int key)`
    *   `void min_tree_update(min_tree *tree, int key, double new_val)`
    *   `double min_tree_query(min_tree *tree, int low_key, int high_key)`
    *   `void min_tree_graph(min_tree *tree, FILE *stream)`

![Range Minimum Tree](images/range_min.png)

---

### 5. Priority Search Tree (`PSTREE`)
Stores 2D points (x, y) ordered by x (as BST key) and augments each node with the maximum y in its subtree, enabling 3-sided range queries.

*   **Header**: [src/priority_search.h](file:///home/joe/src/sleepybishop/augments/src/priority_search.h)
*   **Implementation**: [src/priority_search.c](file:///home/joe/src/sleepybishop/augments/src/priority_search.c)
*   **Key API**:
    *   `void ps_tree_init(ps_tree *tree, tree_allocator *alloc)`
    *   `int ps_tree_add(ps_tree *tree, double x, double y)`
    *   `int ps_tree_remove(ps_tree *tree, double x, double y)`
    *   `void ps_tree_query(ps_tree *tree, double x_min, double x_max, double y_min, ps_cb cb, void *arg)`
    *   `void ps_tree_graph(ps_tree *tree, FILE *stream)`

![Priority Search Tree](images/priority_search.png)

---

### 6. Max Subarray Sum Tree (`MAXSUB`)
Stores a sequence of values and augments each node with subtree-level sum, max_prefix, max_suffix, and max_sub metrics, enabling range maximum subarray sum queries.

*   **Header**: [src/max_subarray.h](file:///home/joe/src/sleepybishop/augments/src/max_subarray.h)
*   **Implementation**: [src/max_subarray.c](file:///home/joe/src/sleepybishop/augments/src/max_subarray.c)
*   **Key API**:
    *   `void sub_tree_init(sub_tree *tree, tree_allocator *alloc)`
    *   `int sub_tree_add(sub_tree *tree, int key, double val)`
    *   `int sub_tree_remove(sub_tree *tree, int key)`
    *   `void sub_tree_update(sub_tree *tree, int key, double new_val)`
    *   `double sub_tree_query(sub_tree *tree, int low_key, int high_key)`
    *   `void sub_tree_graph(sub_tree *tree, FILE *stream)`

![Max Subarray Sum Tree](images/max_subarray.png)

---

### 7. Hash Tree (`HASHTREE`)
Augments the tree with a rolling hash of the sequence to support fast substring equality checks.

*   **Header**: [src/hash_tree.h](file:///home/joe/src/sleepybishop/augments/src/hash_tree.h)
*   **Implementation**: [src/hash_tree.c](file:///home/joe/src/sleepybishop/augments/src/hash_tree.c)
*   **Key API**:
    *   `void hash_tree_init(hash_tree_tree *tree, tree_allocator *alloc)`
    *   `int hash_tree_add(hash_tree_tree *tree, size_t key, char val)`
    *   `int hash_tree_remove(hash_tree_tree *tree, size_t key)`
    *   `void hash_tree_graph(hash_tree_tree *tree, FILE *stream)`

![Hash Tree](images/hash_tree.png)

---

### 8. LCP Tree (`LCPTREE`)
Augments the tree with a longest common prefix of string keys, enabling optimal prefix searches.

*   **Header**: [src/lcp_tree.h](file:///home/joe/src/sleepybishop/augments/src/lcp_tree.h)
*   **Implementation**: [src/lcp_tree.c](file:///home/joe/src/sleepybishop/augments/src/lcp_tree.c)
*   **Key API**:
    *   `void lcp_tree_init(lcp_tree *tree, tree_allocator *alloc)`
    *   `int lcp_tree_add(lcp_tree *tree, const char *str)`
    *   `int lcp_tree_remove(lcp_tree *tree, const char *str)`
    *   `void lcp_tree_graph(lcp_tree *tree, FILE *stream)`

![LCP Tree](images/lcp_tree.png)

---

### 9. Incremental Convex Hull (`INCHULL`)
Maintains the upper boundary of a 2D convex hull dynamically by adding points in left-to-right order.

*   **Header**: [src/incremental_hull.h](file:///home/joe/src/sleepybishop/augments/src/incremental_hull.h)
*   **Implementation**: [src/incremental_hull.c](file:///home/joe/src/sleepybishop/augments/src/incremental_hull.c)
*   **Key API**:
    *   `void inc_hull_init(inc_hull_tree *tree, tree_allocator *alloc)`
    *   `void inc_hull_insert(inc_hull_tree *tree, double x, double y)`
    *   `void inc_hull_destroy(inc_hull_tree *tree)`
    *   `void inc_hull_graph(inc_hull_tree *tree, FILE *stream)`

![Incremental Convex Hull](images/incremental_hull.png)

---

### 10. Rope (`ROPE`)
Augments a Treap with subtree sizes to support character sequences and fast split/merge indexing.

*   **Header**: [src/rope.h](file:///home/joe/src/sleepybishop/augments/src/rope.h)
*   **Implementation**: [src/rope.c](file:///home/joe/src/sleepybishop/augments/src/rope.c)
*   **Key API**:
    *   `void rope_init(rope *r, tree_allocator *alloc)`
    *   `void rope_add(rope *r, size_t idx, char val)`
    *   `void rope_remove(rope *r, size_t idx)`
    *   `char rope_query(rope *r, size_t idx)`
    *   `void rope_graph(rope *r, FILE *stream)`

![Rope](images/rope.png)

---

### 11. Euler Tour Tree (`EULER`)
Augments a Treap to represent forest structures via Euler tours, supporting dynamic link, cut, and connectivity queries.

*   **Header**: [src/euler_tour.h](file:///home/joe/src/sleepybishop/augments/src/euler_tour.h)
*   **Implementation**: [src/euler_tour.c](file:///home/joe/src/sleepybishop/augments/src/euler_tour.c)
*   **Key API**:
    *   `void euler_tour_init(euler_tour *et, size_t max_vertices, tree_allocator *alloc)`
    *   `void euler_tour_destroy(euler_tour *et)`
    *   `void euler_tour_link(euler_tour *et, int u, int v)`
    *   `void euler_tour_cut(euler_tour *et, int u, int v)`
    *   `int euler_tour_connected(euler_tour *et, int u, int v)`
    *   `void euler_tour_graph(euler_tour *et, FILE *stream)`

![Euler Tour Tree](images/euler_tour.png)

---

## License

This library is licensed under the 2-clause BSD license. See [LICENSE](file:///home/joe/src/sleepybishop/augments/LICENSE) for details.
