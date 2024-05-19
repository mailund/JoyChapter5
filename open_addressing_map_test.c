
#include "open_addressing_map.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static uint32_t
random_key()
{
  uint32_t key = (uint32_t)random();
  return key;
}

static void
nop_del(void *p)
{
}

static void *
nop_cpy(void const *p)
{
  return (void *)p;
}

static bool
compare_keys(void const *ap, void const *bp)
{
  uint32_t a = *(uint32_t *)ap;
  uint32_t b = *(uint32_t *)bp;
  return a == b;
}

unsigned int
hash(void const *key)
{
  // truely stupid hash but we need something for the test
  return *(uint32_t *)key ^ 0xdeadbeef;
}

struct key_type ui32_key_type = {
    .cmp = compare_keys, .del = nop_del, .hash = hash, .cpy = nop_cpy};
struct value_type ui32_val_type = {.del = nop_del, .cpy = nop_cpy};

static void
test_intp(int no_elms)
{
  uint32_t *keys = (uint32_t *)malloc(no_elms * sizeof(uint32_t));
  for (int i = 0; i < no_elms; ++i) {
    keys[i] = random_key();
  }
  struct hash_table *map = new_table(&ui32_key_type, &ui32_val_type);
  clock_t start = clock();
  for (int i = 0; i < no_elms; ++i) {
    add_map(map, &keys[i], &keys[i]);
  }
  for (int i = 0; i < no_elms; ++i) {
    void *val = lookup_key(map, &keys[i]);
    assert(val == &keys[i]);
  }
  uint32_t unused_key = 0;
  for (int i = 0; i < no_elms; ++i) {
    void *val = lookup_key(map, &unused_key);
    assert(val == 0);
  }
  for (int i = 0; i < no_elms; ++i) {
    delete_key(map, &keys[i]);
  }
  for (int i = 0; i < no_elms; ++i) {
    void *val = lookup_key(map, &keys[i]);
    assert(val == 0);
  }
  clock_t end = clock();
  double elapsed_time = (end - start) / (double)CLOCKS_PER_SEC;
  printf("%g\n", elapsed_time);

  free(keys);

  printf("active: %u\n", map->active);
  printf("used: %u\n", map->used);

  delete_table(map);
}

int
main(int argc, const char *argv[])
{
  if (argc != 2) {
    printf("Usage: %s no_elements\n", argv[0]);
    return EXIT_FAILURE;
  }

  int no_elms = atoi(argv[1]);
  test_intp(no_elms);

  return EXIT_SUCCESS;
}
