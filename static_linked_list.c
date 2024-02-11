
#include "static_linked_list.h"

#include <stdlib.h>

struct generic_link *
new_link(size_t link_size, struct generic_link *next)
{
  struct generic_link *link = malloc(link_size);
  link->next = next;
  return link;
}

void
generic_push_new_link(struct generic_link **list, size_t link_size)
{
  *list = new_link(sizeof(struct generic_link), *list);
}

void
delete_link(struct list_iter itr)
{
  struct generic_link *next = (*itr.link)->next;
  free(*itr.link);
  *itr.link = next;
}

void
generic_free_linked_list(struct generic_link **list)
{
  while (*list) {
    struct generic_link *next = (*list)->next;
    free(*list);
    *list = next;
  }
}
