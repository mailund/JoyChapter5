
#include "open_addressing_map.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_SIZE 8

unsigned int static p(unsigned int k, unsigned int i, unsigned int m)
{
  return (k + i) & (m - 1);
}

static void
init_table(struct hash_table *table, unsigned int size, struct bin *begin,
           struct bin *end)
{
  // Initialize table members
  struct bin *bins = malloc(size * sizeof *bins);
  *table =
      (struct hash_table){.bins = bins, .size = size, .used = 0, .active = 0};

  // Initialize bins
  struct bin empty_bin = {.in_probe = false, .is_empty = true};
  for (unsigned int i = 0; i < table->size; i++) {
    table->bins[i] = empty_bin;
  }

  // Copy the old bins to the new table
  for (struct bin *bin = begin; bin != end; bin++) {
    if (!bin->is_empty) {
      add_map(table, bin->key, bin->val); // FIXME: use hash_key from bin
    }
  }
}

static void
resize(struct hash_table *table, unsigned int new_size)
{
  // remember the old bins until we have moved them.
  struct bin *old_bins_begin = table->bins,
             *old_bins_end = old_bins_begin + table->size;

  // Update table and copy the old active bins to it.
  init_table(table, new_size, old_bins_begin, old_bins_end);

  // finally, free memory for old bins
  free(old_bins_begin);
}

struct hash_table *
new_table(struct key_type const *key_type, struct value_type const *value_type)
{
  struct hash_table *table = malloc(sizeof *table);
  init_table(table, MIN_SIZE, NULL, NULL);
  table->key_type = key_type;
  table->value_type = value_type;
  return table;
}

static inline void
free_key(struct key_type const *key_type, void *key)
{
  if (key_type->del)
    key_type->del(key);
}

static inline void
free_val(struct value_type const *value_type, void *val)
{
  if (value_type->del)
    value_type->del(val);
}

static inline unsigned int
hash(struct key_type const *key_type, void *key)
{
  return key_type->hash(key);
}

void
delete_table(struct hash_table *table)
{
  struct bin *end = table->bins + table->size;
  for (struct bin *bin = table->bins; bin != end; ++bin) {
    if (bin->in_probe && !bin->is_empty) {
      free_key(table->key_type, bin->key);
      free_val(table->value_type, bin->val);
    }
  }
  free(table->bins);
  free(table);
}

// Find the bin containing key, or the first bin past the end of its probe
struct bin *
find_key(struct hash_table *table, unsigned int hash_key, void *key)
{
  for (unsigned int i = 0; i < table->size; i++) {
    struct bin *bin = table->bins + p(hash_key, i, table->size);
    if (!bin->in_probe ||
        (bin->hash_key == hash_key && table->key_type->cmp(bin->key, bin->val)))
      return bin;
  }
  // The table is full. This should not happen!
  assert(false);
}

void
add_map(struct hash_table *table, void *key, void *value)
{
  uint32_t index;
  unsigned int hash_key = hash(table->key_type, key);
  bool contains = (bool)lookup_key(table, key); // FIXME: reuse hash_key
  for (uint32_t i = 0; i < table->size; ++i) {
    index = p(hash_key, i, table->size);
    struct bin *bin = &table->bins[index];

    if (!bin->in_probe) {
      bin->in_probe = true;
      bin->is_empty = false;
      bin->hash_key = hash_key;
      bin->key = key;
      bin->val = value;
      // we have one more active element
      // and one more unused cell changes character
      table->active++;
      table->used++;
      break;
    }

    if (bin->is_empty && !contains) {
      bin->is_empty = false;
      bin->hash_key = hash_key;
      bin->key = key;
      bin->val = value;
      // we have one more active element
      // but we do not use more cells since the
      // deleted cell was already used.
      table->active++;
      break;
    }

    if (bin->hash_key == hash_key) {
      if (table->key_type->cmp(bin->key, key)) {
        delete_key(table, key);     // FIXME: reuse hash key
        add_map(table, key, value); // FIXME: reuse hash key
        return;                     // Done
      } else {
        // we have found the key but with a
        // different value...
        continue;
      }
    }
  }

  if (table->used > table->size / 2)
    resize(table, table->size * 2);
}

void
delete_key(struct hash_table *table, void *key)
{
  unsigned int hash_key = hash(table->key_type, key);
  struct bin *bin = find_key(table, hash_key, key);
  if (bin->key != key)
    return; // Nothing more to do

  bin->is_empty = true; // Delete the bin
  table->active--;      // Same bins in use but one less active

  if (table->active < table->size / 8 && table->size > MIN_SIZE)
    resize(table, table->size / 2);
}

void *
lookup_key(struct hash_table *table, void *key)
{
  unsigned int hash_key = hash(table->key_type, key); // FIXME reuse
  for (uint32_t i = 0; i < table->size; ++i) {
    uint32_t index = p(hash_key, i, table->size);
    struct bin *bin = &table->bins[index];

    if (!bin->in_probe)
      return 0;

    if (!bin->is_empty && bin->hash_key == hash_key &&
        table->key_type->cmp(bin->key, key))
      return bin->val;
  }
  return 0;
}
