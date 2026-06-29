#ifndef TREAP_H
#define TREAP_H

#include <stdint.h>
#include <stdlib.h>

#define TREAP_HEAD(name, type)                                                                                                     \
    struct name {                                                                                                                  \
        struct type *trh_root;                                                                                                     \
    }

#define TREAP_INITIALIZER(root) {NULL}
#define TREAP_INIT(root)                                                                                                           \
    do {                                                                                                                           \
        (root)->trh_root = NULL;                                                                                                   \
    } while (0)

#define TREAP_ENTRY(type)                                                                                                          \
    struct {                                                                                                                       \
        struct type *tre_left;                                                                                                     \
        struct type *tre_right;                                                                                                    \
        struct type *tre_parent;                                                                                                   \
        uint32_t tre_priority;                                                                                                     \
    }

#define TREAP_LEFT(elm, field) (elm)->field.tre_left
#define TREAP_RIGHT(elm, field) (elm)->field.tre_right
#define TREAP_PARENT(elm, field) (elm)->field.tre_parent
#define TREAP_PRIO(elm, field) (elm)->field.tre_priority
#define TREAP_ROOT(head) (head)->trh_root
#define TREAP_EMPTY(head) (TREAP_ROOT(head) == NULL)

#ifndef TREAP_AUGMENT
#define TREAP_AUGMENT(x)                                                                                                           \
    do {                                                                                                                           \
    } while (0)
#endif

#define TREAP_ROTATE_LEFT(head, elm, tmp, field)                                                                                   \
    do {                                                                                                                           \
        (tmp) = TREAP_RIGHT(elm, field);                                                                                           \
        if ((TREAP_RIGHT(elm, field) = TREAP_LEFT(tmp, field)) != NULL) {                                                          \
            TREAP_PARENT(TREAP_LEFT(tmp, field), field) = (elm);                                                                   \
        }                                                                                                                          \
        if ((TREAP_PARENT(tmp, field) = TREAP_PARENT(elm, field)) != NULL) {                                                       \
            if ((elm) == TREAP_LEFT(TREAP_PARENT(elm, field), field))                                                              \
                TREAP_LEFT(TREAP_PARENT(elm, field), field) = (tmp);                                                               \
            else                                                                                                                   \
                TREAP_RIGHT(TREAP_PARENT(elm, field), field) = (tmp);                                                              \
        } else {                                                                                                                   \
            (head)->trh_root = (tmp);                                                                                              \
        }                                                                                                                          \
        TREAP_LEFT(tmp, field) = (elm);                                                                                            \
        TREAP_PARENT(elm, field) = (tmp);                                                                                          \
        TREAP_AUGMENT(elm);                                                                                                        \
        TREAP_AUGMENT(tmp);                                                                                                        \
    } while (0)

#define TREAP_ROTATE_RIGHT(head, elm, tmp, field)                                                                                  \
    do {                                                                                                                           \
        (tmp) = TREAP_LEFT(elm, field);                                                                                            \
        if ((TREAP_LEFT(elm, field) = TREAP_RIGHT(tmp, field)) != NULL) {                                                          \
            TREAP_PARENT(TREAP_RIGHT(tmp, field), field) = (elm);                                                                  \
        }                                                                                                                          \
        if ((TREAP_PARENT(tmp, field) = TREAP_PARENT(elm, field)) != NULL) {                                                       \
            if ((elm) == TREAP_LEFT(TREAP_PARENT(elm, field), field))                                                              \
                TREAP_LEFT(TREAP_PARENT(elm, field), field) = (tmp);                                                               \
            else                                                                                                                   \
                TREAP_RIGHT(TREAP_PARENT(elm, field), field) = (tmp);                                                              \
        } else {                                                                                                                   \
            (head)->trh_root = (tmp);                                                                                              \
        }                                                                                                                          \
        TREAP_RIGHT(tmp, field) = (elm);                                                                                           \
        TREAP_PARENT(elm, field) = (tmp);                                                                                          \
        TREAP_AUGMENT(elm);                                                                                                        \
        TREAP_AUGMENT(tmp);                                                                                                        \
    } while (0)

#define TREAP_GENERATE(name, type, field, cmp)                                                                                     \
    struct type *name##_TREAP_INSERT(struct name *head, struct type *elm)                                                          \
    {                                                                                                                              \
        struct type *parent = NULL;                                                                                                \
        struct type *node = TREAP_ROOT(head);                                                                                      \
        int comp = 0;                                                                                                              \
        while (node != NULL) {                                                                                                     \
            parent = node;                                                                                                         \
            comp = (cmp)(elm, node);                                                                                               \
            if (comp < 0)                                                                                                          \
                node = TREAP_LEFT(node, field);                                                                                    \
            else if (comp > 0)                                                                                                     \
                node = TREAP_RIGHT(node, field);                                                                                   \
            else                                                                                                                   \
                return (node);                                                                                                     \
        }                                                                                                                          \
        TREAP_PARENT(elm, field) = parent;                                                                                         \
        TREAP_LEFT(elm, field) = TREAP_RIGHT(elm, field) = NULL;                                                                   \
        if (parent != NULL) {                                                                                                      \
            if (comp < 0)                                                                                                          \
                TREAP_LEFT(parent, field) = elm;                                                                                   \
            else                                                                                                                   \
                TREAP_RIGHT(parent, field) = elm;                                                                                  \
        } else {                                                                                                                   \
            TREAP_ROOT(head) = elm;                                                                                                \
        }                                                                                                                          \
        TREAP_AUGMENT(elm);                                                                                                        \
        node = parent;                                                                                                             \
        while (node != NULL) {                                                                                                     \
            TREAP_AUGMENT(node);                                                                                                   \
            node = TREAP_PARENT(node, field);                                                                                      \
        }                                                                                                                          \
        node = elm;                                                                                                                \
        while (TREAP_PARENT(node, field) != NULL && TREAP_PRIO(node, field) > TREAP_PRIO(TREAP_PARENT(node, field), field)) {      \
            struct type *p = TREAP_PARENT(node, field);                                                                            \
            struct type *tmp;                                                                                                      \
            if (node == TREAP_LEFT(p, field)) {                                                                                    \
                TREAP_ROTATE_RIGHT(head, p, tmp, field);                                                                           \
            } else {                                                                                                               \
                TREAP_ROTATE_LEFT(head, p, tmp, field);                                                                            \
            }                                                                                                                      \
        }                                                                                                                          \
        return (NULL);                                                                                                             \
    }                                                                                                                              \
    struct type *name##_TREAP_REMOVE(struct name *head, struct type *elm)                                                          \
    {                                                                                                                              \
        struct type *tmp;                                                                                                          \
        while (TREAP_LEFT(elm, field) != NULL || TREAP_RIGHT(elm, field) != NULL) {                                                \
            if (TREAP_LEFT(elm, field) == NULL) {                                                                                  \
                TREAP_ROTATE_LEFT(head, elm, tmp, field);                                                                          \
            } else if (TREAP_RIGHT(elm, field) == NULL) {                                                                          \
                TREAP_ROTATE_RIGHT(head, elm, tmp, field);                                                                         \
            } else if (TREAP_PRIO(TREAP_LEFT(elm, field), field) > TREAP_PRIO(TREAP_RIGHT(elm, field), field)) {                   \
                TREAP_ROTATE_RIGHT(head, elm, tmp, field);                                                                         \
            } else {                                                                                                               \
                TREAP_ROTATE_LEFT(head, elm, tmp, field);                                                                          \
            }                                                                                                                      \
        }                                                                                                                          \
        struct type *parent = TREAP_PARENT(elm, field);                                                                            \
        if (parent != NULL) {                                                                                                      \
            if (TREAP_LEFT(parent, field) == elm)                                                                                  \
                TREAP_LEFT(parent, field) = NULL;                                                                                  \
            else                                                                                                                   \
                TREAP_RIGHT(parent, field) = NULL;                                                                                 \
        } else {                                                                                                                   \
            TREAP_ROOT(head) = NULL;                                                                                               \
        }                                                                                                                          \
        struct type *node = parent;                                                                                                \
        while (node != NULL) {                                                                                                     \
            TREAP_AUGMENT(node);                                                                                                   \
            node = TREAP_PARENT(node, field);                                                                                      \
        }                                                                                                                          \
        return (elm);                                                                                                              \
    }                                                                                                                              \
    struct type *name##_TREAP_FIND(struct name *head, struct type *elm)                                                            \
    {                                                                                                                              \
        struct type *node = TREAP_ROOT(head);                                                                                      \
        int comp;                                                                                                                  \
        while (node != NULL) {                                                                                                     \
            comp = (cmp)(elm, node);                                                                                               \
            if (comp < 0)                                                                                                          \
                node = TREAP_LEFT(node, field);                                                                                    \
            else if (comp > 0)                                                                                                     \
                node = TREAP_RIGHT(node, field);                                                                                   \
            else                                                                                                                   \
                return (node);                                                                                                     \
        }                                                                                                                          \
        return (NULL);                                                                                                             \
    }

#define TREAP_INSERT(name, x, y) name##_TREAP_INSERT(x, y)
#define TREAP_REMOVE(name, x, y) name##_TREAP_REMOVE(x, y)
#define TREAP_FIND(name, x, y) name##_TREAP_FIND(x, y)

#endif
