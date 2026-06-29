# Augmented Search Trees

[#CI](https://github.com/sleepybishop/interval_trees/actions/workflows/ci.yml)

A modular C library implementing **0 advanced tree augmentations** built on top of robust, production-grade base tree templates:
1. **Red-Black Trees ([src/tree.h](file:///home/joe/src/sleepybishop/augments/src/tree.h))**: Upgraded OpenBSD Red-Black tree macro library.

Every augmentation maintains $O(\log n)$ insertion, deletion, and rotation complexity by computing subtree-local properties on the fly.

---

## Repository Structure

*   [src/](file:///home/joe/src/sleepybishop/augments/src/): Source code for all base templates and augmentations.

---

## Tree Derivation Map

Below is a map of the base tree templates in the library and the modules derived by augmenting them:

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

## License

This library is licensed under the 2-clause BSD license. See [LICENSE](file:///home/joe/src/sleepybishop/augments/LICENSE) for details.
