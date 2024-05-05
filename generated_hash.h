
#ifndef CHAINED_HASH_H
#define CHAINED_HASH_H

#include <stdbool.h>
#include <stdio.h>

#include "generated_list.h"

#define EQ_CMP(A, B) ((A) == (B))
#define NOP_DESTRUCTOR(KEY)

GEN_LIST(int, unsigned int, EQ_CMP, NOP_DESTRUCTOR);

struct hash_table {
  struct int_list *bins;
  unsigned int size;
  unsigned int used;
};

#define MIN_SIZE 8

struct int_list *
get_key_bin(struct hash_table *table, unsigned int key)
{
  unsigned int mask = table->size - 1;
  unsigned int index = key & mask;
  return &table->bins[index];
}

static void
init_bins(struct hash_table *table)
{
  for (struct int_list *bin = table->bins; bin < table->bins + table->size;
       bin++) {
    *bin = (struct int_list)NEW_LIST();
  }
}

struct hash_table *
new_table()
{
  struct hash_table *table = malloc(sizeof *table);
  struct int_list *bins = malloc(MIN_SIZE * sizeof *bins);
  *table = (struct hash_table){.bins = bins, .size = MIN_SIZE, .used = 0};
  init_bins(table);
  return table;
}

void
free_table(struct hash_table *table)
{
  for (struct int_list *bin = table->bins; bin < table->bins + table->size;
       bin++) {
    int_free_list(bin);
  }
  free(table->bins);
  free(table);
}

#define MOVE_LINK(FROM, TO)                                                    \
  do {                                                                         \
    typeof(**FROM) *link = *FROM;                                              \
    *FROM = link->next;                                                        \
    link->next = *TO;                                                          \
    *TO = link;                                                                \
  } while (0)

void
insert_key(struct hash_table *table, unsigned int key);

static void
copy_links(struct hash_table *table, struct int_list *beg, struct int_list *end)
{
  for (struct int_list *bin = beg; bin < end; bin++) {
    for (ITR(bin) itr = ITR_BEG(bin); !ITR_END(itr);) {
      unsigned int key = ITR_DEREF(itr)->key;
      MOVE_LINK(itr, ITR_BEG(get_key_bin(table, key)));
    }
  }
}

static void
resize(struct hash_table *table, unsigned int new_size)
{
  // remember these so we can copy and free the old bins
  struct int_list *old_bins = table->bins, *old_from = old_bins,
                  *old_to = old_from + table->size;

  // set up the new table
  table->bins = malloc(new_size * sizeof *table->bins);
  table->size = new_size;
  init_bins(table);

  // copy keys
  copy_links(table, old_from, old_to);

  // free the old bins memory
  free(old_bins);
}

void
insert_key(struct hash_table *table, unsigned int key)
{
  struct int_list *bin = get_key_bin(table, key);
  if (!int_contains_key(bin, key)) {
    int_add_key(bin, key);
    table->used++;
    if (table->size == table->used) {
      resize(table, 2 * table->size);
    }
  }
}

bool
contains_key(struct hash_table *table, unsigned int key)
{
  return int_contains_key(get_key_bin(table, key), key);
}

void
delete_key(struct hash_table *table, unsigned int key)
{
  struct int_list *bin = get_key_bin(table, key);
  if (int_contains_key(bin, key)) {
    int_delete_key(bin, key);
    table->used--;
    if (table->size > MIN_SIZE && table->used < table->size / 4) {
      resize(table, table->size / 2);
    }
  }
}

#endif
