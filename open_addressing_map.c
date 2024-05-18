
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

void
add_map_hash(struct hash_table *table, unsigned int hash_key, void *key,
             void *value);

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
      add_map_hash(table, bin->hash_key, bin->key, bin->val);
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

static inline void
free_bin(struct hash_table *table, struct bin *bin)
{
  if (bin->in_probe && !bin->is_empty) {
    free_key(table->key_type, bin->key);
    free_val(table->value_type, bin->val);
    bin->is_empty = true; // Delete the bin
    table->active--;      // Same bins in use but one less active
  }
}

static inline bool
key_in_bin(struct hash_table *table, struct bin *bin, unsigned int hash_key,
           void const *key)
{
  return bin->in_probe && bin->hash_key == hash_key &&
         table->key_type->cmp(bin->key, key);
}

static inline void
store_in_bin(struct hash_table *table, struct bin *bin, unsigned int hash_key,
             void *key, void *value)
{
  table->active += bin->is_empty;
  table->used += !bin->in_probe;
  if (bin->in_probe && !bin->is_empty) {
    free_key(table->key_type, bin->key);
    free_val(table->value_type, bin->val);
  }
  *bin = (struct bin){.in_probe = true,
                      .is_empty = false,
                      .hash_key = hash_key,
                      .key = key,
                      .val = value};
}

static inline unsigned int
hash(struct key_type const *key_type, void const *key)
{
  return key_type->hash(key);
}

void
delete_table(struct hash_table *table)
{
  for (struct bin *bin = table->bins; bin != table->bins + table->size; ++bin) {
    free_bin(table, bin);
  }
  free(table->bins);
  free(table);
}

// Find the bin containing key, or the first bin past the end of its probe
struct bin *
find_key(struct hash_table *table, unsigned int hash_key, void const *key)
{
  for (unsigned int i = 0; i < table->size; i++) {
    struct bin *bin = table->bins + p(hash_key, i, table->size);
    if (!bin->in_probe ||                      // end of probe
        key_in_bin(table, bin, hash_key, key)) // found the key
      return bin;
  }
  // The table is full. This should not happen!
  assert(false);
}

// Find the first empty bin in its probe.
struct bin *
find_empty(struct hash_table *table, unsigned int hash_key)
{
  for (unsigned int i = 0; i < table->size; i++) {
    struct bin *bin = table->bins + p(hash_key, i, table->size);
    if (bin->is_empty)
      return bin;
  }
  // The table is full. This should not happen!
  assert(false);
}

void
add_map_hash(struct hash_table *table, unsigned int hash_key, void *key,
             void *value)
{
  struct bin *bin = find_key(table, hash_key, key);
  if (!bin->in_probe) {
    // Reached the end of the probe without finding the key.
    // Search again to find an empty bin.
    bin = find_empty(table, hash_key);
  }
  store_in_bin(table, bin, hash_key, key, value);

  if (table->used > table->size / 2)
    resize(table, table->size * 2);
}

void
add_map(struct hash_table *table, void *key, void *value)
{
  unsigned int hash_key = hash(table->key_type, key);
  add_map_hash(table, hash_key, key, value);
}

void
delete_key(struct hash_table *table, void const *key)
{
  unsigned int hash_key = hash(table->key_type, key);
  free_bin(table, find_key(table, hash_key, key));
  if (table->active < table->size / 8 && table->size > MIN_SIZE)
    resize(table, table->size / 2);
}

void *const
lookup_key(struct hash_table *table, void const *key)
{
  unsigned int hash_key = hash(table->key_type, key);
  struct bin *bin = find_key(table, hash_key, key);
  return bin->in_probe ? bin->val : NULL;
}
