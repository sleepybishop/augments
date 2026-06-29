# Augmented Search Trees

[#CI](https://github.com/sleepybishop/interval_trees/actions/workflows/ci.yml)

A modular C library implementing **1 advanced tree augmentations** built on top of robust, production-grade base tree templates:
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

## License

This library is licensed under the 2-clause BSD license. See [LICENSE](file:///home/joe/src/sleepybishop/augments/LICENSE) for details.
