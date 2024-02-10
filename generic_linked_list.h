
#ifndef GENERIC_LINKED_LISTS_H
#define GENERIC_LINKED_LISTS_H

#include <stdbool.h>
#include <stdlib.h>

#define OWNER(T) T * // Use this when you need a var that holds a pointer to T
#define REF(T) T **  // Use this when you move pointer to pointers to T around

struct generic_link {
  struct generic_link *next;
};

#define LINK_HEADER struct generic_link generic
#define LIST_HEADER OWNER(struct generic_link) generic

#define LINK_UPCAST(LINK) &((LINK)->generic)
#define LIST_UPCAST(LIST) &((LIST)->generic)

// Generic iteration
struct list_iter {
  struct generic_link **link;
};

#define NEW_LIST()                                                             \
  {                                                                            \
    .generic = NULL                                                            \
  }

static inline struct list_iter generic_list_begin(REF(struct generic_link) list)
{
  return (struct list_iter){.link = list};
}
#define list_begin(LIST) generic_list_begin(LIST_UPCAST(LIST))

static inline bool
list_end(struct list_iter itr)
{
  return !*itr.link;
}
static inline struct list_iter
list_next(struct list_iter itr)
{
  return (struct list_iter){.link = &(*itr.link)->next};
}
#define cast_itr(T, ITR) ((T *)(*(ITR).link))
#define void_cast(ITR) cast_itr(void, ITR)

// Rest of the interface
void generic_free_linked_list(REF(struct generic_link) list);

void
generic_push_new_link(REF(struct generic_link) list, size_t link_size);

#define free_linked_list(LIST) generic_free_linked_list(LIST_UPCAST(LIST))
#define push_new_link(LIST, LINK_SIZE)                                         \
  generic_push_new_link(LIST_UPCAST(LIST), LINK_SIZE)

void
delete_link(struct list_iter itr);

#endif
