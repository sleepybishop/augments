# Augmented Search Trees

[#CI](https://github.com/sleepybishop/interval_trees/actions/workflows/ci.yml)

A modular C library implementing **2 advanced tree augmentations** built on top of robust, production-grade base tree templates:
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

### 2. Max Subarray Sum Tree (`MAXSUB`)
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
