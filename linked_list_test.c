
#include "generic_linked_list.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct int_list_link {
  LINK_HEADER;
  unsigned int key;
};
struct int_list {
  LIST_HEADER;
};

#define NEW_INTLIST()                                                          \
  {                                                                            \
    .generic = NEW_LIST()                                                      \
  }

#define GEN_ADD_ELEMENT(LIST_NAME, KEY_TYPE)                                   \
  void LIST_NAME##_add_element(struct LIST_NAME *list, KEY_TYPE key)           \
  {                                                                            \
    push_new_link(list, sizeof(struct LIST_NAME##_link));                      \
    struct LIST_NAME##_link *link = void_cast(list_begin(list));               \
    link->key = key;                                                           \
  }

GEN_ADD_ELEMENT(int_list, unsigned int);

void
free_int_list(struct int_list *list)
{
  free_linked_list(list);
  list->generic = NULL;
}

void
int_delete_element(struct int_list *list, unsigned int key)
{
  for (struct list_iter itr = list_begin(list); !list_end(itr);
       itr = list_next(itr)) {
    if (cast_itr(struct int_list_link, itr)->key == key) {
      delete_link(itr);
      return;
    }
  }
}

bool
int_contains_element(struct int_list *list, unsigned int key)
{
  for (struct list_iter itr = list_begin(list); !list_end(itr);
       itr = list_next(itr)) {
    if (cast_itr(struct int_list_link, itr)->key == key) {
      return true;
    }
  }
  return false;
}

int
main()
{
  unsigned int some_keys[] = {
      1, 2, 3, 4, 5,
  };
  size_t n = sizeof(some_keys) / sizeof(*some_keys);
  struct int_list owner = NEW_INTLIST();

  // init_linked_list(list);

  for (unsigned int i = 0; i < n; i++) {
    printf("inserting key %u\n", some_keys[i]);
    int_list_add_element(&owner, some_keys[i]);
  }
  printf("\n");

  for (unsigned int i = 0; i < n; i++) {
    printf("is key %u in list? %d\n", some_keys[i],
           int_contains_element(&owner, some_keys[i]));

    assert(int_contains_element(&owner, some_keys[i]));
  }
  printf("\n");

  printf("Removing keys 3 and 4\n");
  int_delete_element(&owner, 3);
  int_delete_element(&owner, 4);
  printf("\n");

  for (unsigned int i = 0; i < n; i++) {
    printf("is key %u in &owner? %d\n", some_keys[i],
           int_contains_element(&owner, some_keys[i]));
    if (some_keys[i] == 3 || some_keys[i] == 4)
      assert(!int_contains_element(&owner, some_keys[i]));
    else
      assert(int_contains_element(&owner, some_keys[i]));
  }
  printf("\n");

  free_int_list(&owner);

  return EXIT_SUCCESS;
}
