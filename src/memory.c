
#include "memory.h"
#include "debug.h"
#include "runtime.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

void *reallocate(void *ptr, size_t old_size, size_t new_size) {
  (void)old_size;

  if (new_size == 0) {
    free(ptr);
    return NULL;
  }

  void *tmp = realloc(ptr, new_size);

  if (tmp == NULL)
    error("reallocate: Couldn't reallocate a block.");
  return tmp;
}

AVM_object_t *allocate_object(struct AVM_VM *vm, AVM_object_kind kind) {
  AVM_object_t *res = reallocate(NULL, 0, sizeof(AVM_object_t));
  res->kind = kind;
  res->next = vm->objs;
  vm->objs = res;
  return res;
}

AVM_value_t* new_int(struct AVM_VM *vm, int i) {
  AVM_object_t *node = allocate_object(vm, AVM_ObjValue);
  AVM_value_t *res = &node->as.val;
  res->kind = AVM_IntVal;
  res->int_value = i;
  return res;
}

AVM_value_t *new_bool(struct AVM_VM *vm, _Bool b) {
  AVM_object_t *node = allocate_object(vm, AVM_ObjValue);
  AVM_value_t *res = &node->as.val;
  res->kind = AVM_BoolVal;
  res->bool_value = b;
  return res;
}

AVM_value_t *new_clos(struct AVM_VM *vm, int l, AVM_env_t *env) {
  AVM_object_t *node = allocate_object(vm, AVM_ObjValue);
  AVM_value_t *res = &node->as.val;
  res->kind = AVM_ClosVal;
  res->addr = l;
  res->env = env;
  return res;
}
