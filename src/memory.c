
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

void *allocate_object(struct AVM_VM *vm, size_t size, AVM_object_kind kind) {
  AVM_object_t *header = reallocate(NULL, 0, sizeof(AVM_object_t) + size);
  header->kind = kind;
  header->next = vm->objs;
  vm->objs = header;
  return (void *)(header + 1); // return the address right after the header.
}

AVM_value_t* new_int(struct AVM_VM *vm, int i) {
  AVM_value_t *res = allocate_object(vm, sizeof(AVM_value_t), AVM_ObjValue);
  res->kind = AVM_IntVal;
  res->int_value = i;
  return res;
}

AVM_value_t *new_bool(struct AVM_VM *vm, _Bool b) {
  AVM_value_t *res = allocate_object(vm, sizeof(AVM_value_t), AVM_ObjValue);
  res->kind = AVM_BoolVal;
  res->bool_value = b;
  return res;
}

AVM_value_t *new_clos(struct AVM_VM *vm, int l, AVM_env_t *env) {
  AVM_value_t *res = allocate_object(vm, sizeof(AVM_value_t), AVM_ObjValue);
  res->kind = AVM_ClosVal;
  res->clos_value.addr = l;
  res->clos_value.penv = env->penv;
  return res;
}


/* GC */

void free_object(AVM_object_t* header) {
  if (header->kind = AVM_ObjEnv) {
    /* implement here. */
    return;
  }

  AVM_value_t* value = (AVM_value_t*)(header + 1);
  switch (value->kind) {
  case AVM_Epsilon:
    /* Impossible case */
    break;
  case AVM_BoolVal:
  case AVM_IntVal:
    goto FREE_OBJ;
  case AVM_ClosVal:
    drop_array(value->clos_value.penv);
    goto FREE_OBJ;
  FREE_OBJ:
    reallocate(header, 0, 0);
  }
}
