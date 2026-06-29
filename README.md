# Augmented Search Trees

[#CI](https://github.com/sleepybishop/interval_trees/actions/workflows/ci.yml)

A modular C library implementing **6 advanced tree augmentations** built on top of robust, production-grade base tree templates:
1. **Red-Black Trees ([src/tree.h](file:///home/joe/src/sleepybishop/augments/src/tree.h))**: Upgraded OpenBSD Red-Black tree macro library.

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

## License

This library is licensed under the 2-clause BSD license. See [LICENSE](file:///home/joe/src/sleepybishop/augments/LICENSE) for details.
