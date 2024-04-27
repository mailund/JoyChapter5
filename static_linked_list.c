
#include "static_linked_list.h"

#include <stdlib.h>

struct link *
new_link(size_t link_size, struct link *next)
{
  struct link *link = malloc(link_size);
  link->next = next;
  return link;
}

void
generic_push_new_link(struct link **list, size_t link_size)
{
  *list = new_link(sizeof(struct link), *list);
}

void
delete_link(struct list_iter itr)
{
  struct link *next = (*itr.link)->next;
  free(*itr.link);
  *itr.link = next;
}

void
generic_free_linked_list(struct link **list)
{
  while (*list) {
    struct link *next = (*list)->next;
    free(*list);
    *list = next;
  }
}
