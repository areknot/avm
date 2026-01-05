
#include "memory.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include "runtime.h"


void *reallocate(void *ptr, size_t old_size, size_t new_size) {
  (void)old_size;

  if (new_size == 0) {
    free(ptr);
    return NULL;
  }

  void *tmp = realloc(ptr, new_size);

  if (tmp == NULL) {
    fprintf(stderr, "reallocate: Couldn't reallocate a block.");
    exit(1);
  }
  return tmp;
}

AVM_value_t* new_int(int i) {
  AVM_value_t* node = malloc(sizeof(AVM_value_t));
  if (node == NULL)
    error("new_int: Failed to allocate memory.");
  node->kind = AVM_IntVal;
  node->int_value = i;
  return node;
}

AVM_value_t *new_bool(_Bool b) {
  AVM_value_t* node = malloc(sizeof(AVM_value_t));
  if (node == NULL)
    error("new_int: Failed to allocate memory.");
  node->kind = AVM_BoolVal;
  node->bool_value = b;
  return node;
}

AVM_value_t *new_clos(int l, AVM_env_t *env) {
  AVM_value_t* node = malloc(sizeof(AVM_value_t));
  if (node == NULL)
    error("new_int: Failed to allocate memory.");
  node->kind = AVM_ClosVal;
  node->addr = l;
  node->env = env;
  return node;
}
