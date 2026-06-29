# Augmented Search Trees

[#CI](https://github.com/sleepybishop/interval_trees/actions/workflows/ci.yml)

Augmenting a small dependable data structure with logic to address a more complex use case is a pattern I've used to great effect in my career.
It's a relic from a different time now that you can pull libraries with every corner of the internet via a package manager.

The code here implementes a modular C library implementing **12 advanced tree augmentations** built on top of robust, production-grade base tree templates:
1. **Red-Black Trees ([src/tree.h](file:///home/joe/src/sleepybishop/augments/src/tree.h))**: Upgraded OpenBSD Red-Black tree macro library.
2. **Treaps ([src/treap.h](file:///home/joe/src/sleepybishop/augments/src/treap.h))**: BSD-style randomized binary search tree template supporting split and merge.

Every augmentation maintains $O(\log n)$ insertion, deletion, and rotation complexity by computing subtree-local properties on the fly.

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
*   **[Dynamic Convex Hull](file:///home/joe/src/sleepybishop/augments/src/dynamic_hull.h)** (`DYNHULL`): Augments a Treap to maintain the upper/lower hulls of a set of points, supporting both insertion and deletion in logarithmic time.

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

## License

This library is licensed under the 2-clause BSD license. See [LICENSE](file:///home/joe/src/sleepybishop/augments/LICENSE) for details.
