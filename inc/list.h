#ifndef _LIST_H_

#include <base.h>

typedef struct _list_item
{
    struct _list_item *previous;
    struct _list_item *next;
} list_item;

static inline void
list_create(list_item *list)
{
    list->previous = list;
    list->next = list;
}

static inline void
list_insert(list_item *previous, list_item *next, list_item *item)
{
    next->previous = item;
    item->next = next;
    item->previous = previous;
    previous->next = item;
}

static inline void
list_add(list_item *list, list_item *item)
{
    list_insert(list, list->next, item);
}

static inline void
list_remove(list_item *item)
{
    item->previous->next = item->next;
    item->next->previous = item->previous;
    list_create(item);
}

static inline bool
list_is_empty(list_item *item)
{
    return item == item->next;
}

#define list_entry(item, type, member) container_of(item, type, member)

#define list_for_each(item, list)                                              \
    for (item = (list)->next; item != (list); item = item->next)

#define list_for_each_entry(item, list, member)                                \
    for (item = list_entry((list)->next, typeof(*item), member);               \
         &item->member != (list);                                              \
         item = list_entry(item->member.next, typeof(*item), member))

#endif // _LIST_H_
