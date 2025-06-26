#ifndef __KSH_LIST_H__
#define __KSH_LIST_H__

#include <stddef.h>

#ifndef alignof
#define alignof(x) __alignof__(x)
#endif
#ifndef typeof
#define typeof(expr) __typeof__(expr)
#endif
#ifndef container_of
#define container_of(ptr, type, member) ({ \
                const typeof( ((type *)0)->member ) *__mptr = (ptr); \
                (type *)( (char *)__mptr - offsetof(type,member) );})
#endif


struct list_node {
    struct list_node *next;
    struct list_node *prev;
};

struct list {
    struct list_node node;
};

static inline void
list_init(struct list *list) {
    list->node.next = &list->node;
    list->node.prev = &list->node;
}

static inline void
list_insert(struct list *list,
            struct list_node *node)
{
    node->next = list->node.next;
    node->prev = &list->node;

    list->node.next = node;
    node->next->prev = node;
}

static inline int
list_empty(struct list *list)
{
    return list->node.next == &list->node;
}

static inline void
list_remove(struct list *list,
            struct list_node *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

#endif
