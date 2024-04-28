
#ifndef GENERIC_LINKED_LISTS_H
#define GENERIC_LINKED_LISTS_H

#include <stdbool.h>
#include <stdlib.h>

struct link {
  struct link *next;
};
struct list {
  struct link *head;
};

#define LIST_UPCAST(LIST) &((LIST)->head)

// Generic iteration
struct list_iter {
  struct link **link;
};

#define NEW_LIST()                                                             \
  {                                                                            \
    .head = NULL                                                               \
  }

static inline struct list_iter
generic_list_begin(struct link **list)
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

#define list_head(LIST) ((void *)(*(LIST).link))

// Rest of the interface
void
generic_free_linked_list(struct link **list);

void
generic_push_new_link(struct link **list, size_t link_size);

#define free_linked_list(LIST) generic_free_linked_list(LIST_UPCAST(LIST))
#define push_new_link(LIST, LINK_SIZE)                                         \
  generic_push_new_link(LIST_UPCAST(LIST), LINK_SIZE)

void
delete_link(struct list_iter itr);

#define GEN_STRUCTS(LIST_NAME, KEY_TYPE)                                       \
  struct LIST_NAME##_link {                                                    \
    struct link link;                                                          \
    KEY_TYPE key;                                                              \
  };                                                                           \
  struct LIST_NAME##_list {                                                    \
    struct link *head;                                                         \
  };

#define GEN_ADD_KEY(LIST_NAME, KEY_TYPE)                                       \
  void LIST_NAME##_add_key(struct LIST_NAME##_list *list, KEY_TYPE key)        \
  {                                                                            \
    push_new_link(list, sizeof(struct LIST_NAME##_link));                      \
    struct LIST_NAME##_link *link = list_head(list_begin(list));               \
    link->key = key;                                                           \
  }

#define GEN_FREE_LIST(LIST_NAME, KEY_TYPE, FREE_KEY)                           \
  void LIST_NAME##_free_list(struct LIST_NAME##_list *list)                    \
  {                                                                            \
    for (struct list_iter itr = list_begin(list); !list_end(itr);              \
         itr = list_next(itr)) {                                               \
      struct LIST_NAME##_link *link = list_head(itr);                          \
      FREE_KEY(link->key);                                                     \
    }                                                                          \
    free_linked_list(list);                                                    \
    list->head = NULL;                                                         \
  }

#define GEN_DELETE_KEY(LIST_NAME, KEY_TYPE, IS_EQ, FREE_KEY)                   \
  void LIST_NAME##_delete_key(struct LIST_NAME##_list *list,                   \
                              const KEY_TYPE key)                              \
  {                                                                            \
    for (struct list_iter itr = list_begin(list); !list_end(itr);              \
         itr = list_next(itr)) {                                               \
      struct LIST_NAME##_link *link = list_head(itr);                          \
      if (IS_EQ(link->key, key)) {                                             \
        FREE_KEY(link->key);                                                   \
        delete_link(itr);                                                      \
        return;                                                                \
      }                                                                        \
    }                                                                          \
  }

#define GEN_CONTAINS_KEY(LIST_NAME, KEY_TYPE, IS_EQ)                           \
  bool LIST_NAME##_contains_key(struct LIST_NAME##_list *list,                 \
                                const KEY_TYPE key)                            \
  {                                                                            \
    for (struct list_iter itr = list_begin(list); !list_end(itr);              \
         itr = list_next(itr)) {                                               \
      struct LIST_NAME##_link *link = list_head(itr);                          \
      if (IS_EQ(link->key, key)) {                                             \
        return true;                                                           \
      }                                                                        \
    }                                                                          \
    return false;                                                              \
  }

#define GEN_LIST(LIST_NAME, KEY_TYPE, IS_EQ, FREE_KEY)                         \
  GEN_STRUCTS(LIST_NAME, KEY_TYPE);                                            \
  GEN_ADD_KEY(LIST_NAME, KEY_TYPE);                                            \
  GEN_DELETE_KEY(LIST_NAME, KEY_TYPE, IS_EQ, FREE_KEY);                        \
  GEN_CONTAINS_KEY(LIST_NAME, KEY_TYPE, IS_EQ);                                \
  GEN_FREE_LIST(LIST_NAME, KEY_TYPE, FREE_KEY);

#endif
