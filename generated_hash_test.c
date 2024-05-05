
#include "generated_hash.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static unsigned int
random_key()
{
  unsigned int key = (unsigned int)rand();
  return key;
}

int
main(int argc, const char *argv[])
{
  if (argc != 2) {
    printf("Usage: %s no_elements\n", argv[0]);
    return EXIT_FAILURE;
  }

  int no_elms = atoi(argv[1]);
  unsigned int *keys = malloc(no_elms * sizeof *keys);
  for (int i = 0; i < no_elms; ++i) {
    keys[i] = random_key();
  }
  struct hash_table *table = new_table();
  clock_t start = clock();
  printf("Inserting %d elements\n", no_elms);
  for (int i = 0; i < no_elms; ++i) {
    printf("Inserting key %d\n", keys[i]);
    insert_key(table, keys[i]);
  }
  printf("Checking if the elemers are now in the table.\n");
  for (int i = 0; i < no_elms; ++i) {
    bool contains = contains_key(table, keys[i]);
    printf("Checking if %d is in the table? %d\n", keys[i], contains);
  }
  for (int i = 0; i < no_elms; ++i) {
    assert(contains_key(table, keys[i]));
  }

  printf("Deleting all elements.\n");
  for (int i = 0; i < no_elms; ++i) {
    delete_key(table, keys[i]);
  }
  printf("Checking if the elemers are still in the table.\n");
  for (int i = 0; i < no_elms; ++i) {
    bool contains = contains_key(table, keys[i]);
    printf("Checking if %d is in the table? %d\n", keys[i], contains);
  }
  for (int i = 0; i < no_elms; ++i) {
    assert(!contains_key(table, keys[i]));
  }

  clock_t end = clock();
  double elapsed_time = (end - start) / (double)CLOCKS_PER_SEC;
  printf("%g\n", elapsed_time);

  free(keys);
  free_table(table);

  return EXIT_SUCCESS;
}
