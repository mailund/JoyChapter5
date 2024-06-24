
#include "open_addressing_map.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static uint32_t
random_key()
{
  uint32_t key = (uint32_t)random();
  return key;
}

char *
itoa(unsigned int i)
{
  // Not super safe itoa, but good enough for an example like this.
  char *buf = malloc(sizeof(char) * 20);
  sprintf(buf, "%d", i);
  return buf;
}

static void *
u32_dup(void const *p)
{
  uint32_t *new = malloc(sizeof(uint32_t));
  *new = *(uint32_t *)p;
  return new;
}

void *
str_dup(const void *p)
{
  const char *s = p;
  char *new = malloc(strlen(s) + 1);
  strcpy(new, s);
  return new;
}

static bool
u32_cmp(void const *ap, void const *bp)
{
  uint32_t a = *(uint32_t *)ap;
  uint32_t b = *(uint32_t *)bp;
  return a == b;
}

static bool
str_cmp(void const *ap, void const *bp)
{
  char *a = (char *)ap;
  char *b = (char *)bp;
  return strcmp(a, b) == 0;
}

unsigned int
u32_hash(void const *key)
{
  // truely stupid hash but we need something for the test
  return *(uint32_t *)key ^ 0xdeadbeef;
}

#define STR_EQ(A, B) (strcmp(A, B) == 0)
unsigned int
str_hash(void const *p)
{
  char *x = (char *)p;
  unsigned int h = 0;
  for (char *p = x; *p; p++) {
    h = 31 * h + *p;
  }
  return h;
}

struct key_type str_key_type = {
    .cmp = str_cmp, .del = free, .hash = str_hash, .cpy = str_dup};
struct value_type ui32_val_type = {.del = free, .cpy = u32_dup};

int
main(int argc, const char *argv[])
{
  struct hash_table *map = new_table(&str_key_type, &ui32_val_type);
  add_map(map, "foo", &(uint32_t){13});
  add_map(map, "bar", &(uint32_t){42});

  assert(*(uint32_t *)lookup_key(map, "foo") == 13);
  assert(*(uint32_t *)lookup_key(map, "bar") == 42);

  delete_table(map);

  return 0;
}
