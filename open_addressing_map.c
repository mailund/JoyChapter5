
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
  table->bins = malloc(size * sizeof *table->bins);
  table->size = size;
  table->used = 0;
  table->active = 0;

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
  table->key_type = key_type;
  table->value_type = value_type;
  init_table(table, MIN_SIZE, NULL, NULL);
  return table;
}

static inline unsigned int
hash(struct hash_table *table, void const *key)
{
  return table->key_type->hash(key);
}

static inline void
free_key(struct hash_table *table, void *key)
{
  if (table->key_type->del)
    table->key_type->del(key);
}

static inline void
free_val(struct hash_table *table, void *val)
{
  if (table->value_type->del)
    table->value_type->del(val);
}

#define HASH(KEY) hash(table, (KEY))

static inline bool
is_active_bin(struct bin *bin)
{
  return bin->in_probe && !bin->is_empty;
}

static inline void
free_bin(struct hash_table *table, struct bin *bin)
{
  if (is_active_bin(bin)) {
    free_key(table, bin->key);
    free_val(table, bin->val);
    bin->is_empty = true; // Delete the bin
    table->active--;      // Same bins in use but one less active
  }
}

static inline bool
key_in_bin(struct hash_table *table, struct bin *bin, unsigned int hash_key,
           void const *key)
{
  return is_active_bin(bin) && bin->hash_key == hash_key &&
         table->key_type->cmp(bin->key, key);
}

static inline void
store_in_bin(struct hash_table *table, struct bin *bin, unsigned int hash_key,
             void *key, void *value)
{
  table->active += bin->is_empty; // FIXME: this isn't as clear as it should be
  table->used += !bin->in_probe;  // FIXME: this isn't as clear as it should be
  free_bin(table, bin);
  *bin = (struct bin){.in_probe = true,
                      .is_empty = false,
                      .hash_key = hash_key,
                      .key = key,
                      .val = value};
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

// Find the bin containing key, or the first bin past the end of its probe.
// It will never return a bin that is in a probe and empty, since those
// cannot contain the key and if we need an empty bin we will search for
// the earliest in the probe using find_empty().
struct bin *
find_key(struct hash_table *table, unsigned int hash_key, void const *key)
{
  for (unsigned int i = 0; i < table->size; i++) {
    struct bin *bin = table->bins + p(hash_key, i, table->size);
    if (!bin->in_probe ||                      // end of probe, or
        key_in_bin(table, bin, hash_key, key)) // found the key
      return bin;
  }
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
}

void
add_map_hash(struct hash_table *table, unsigned int hash_key, void *key,
             void *value)
{
  struct bin *bin = find_key(table, hash_key, key);
  bin = bin->in_probe ? bin // bin with key or empty
                      : find_empty(table, hash_key);

  store_in_bin(table, bin, hash_key, key, value);

  if (table->used > table->size / 2)
    resize(table, table->size * 2);
}

void
add_map(struct hash_table *table, void *key, void *value)
{
  add_map_hash(table, hash(table, key), key, value);
}

void
delete_key(struct hash_table *table, void const *key)
{
  struct bin *bin = find_key(table, hash(table, key), key);
  free_bin(table, bin);

  if (table->active < table->size / 8 && table->size > MIN_SIZE)
    resize(table, table->size / 2);
}

void *const
lookup_key(struct hash_table *table, void const *key)
{
  struct bin *bin = find_key(table, hash(table, key), key);
  return bin->in_probe ? bin->val : NULL;
}
