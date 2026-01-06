#pragma once
#include <stddef.h>
#include "runtime.h"

/*
  old_size == 0 => allocate a new block of size new_size.
  old_size > 0 and new_size == 0 => free ptr.
  old_size > 0 and new_size > 0 => reallocate.
*/
void *reallocate(void *ptr, size_t old_size, size_t new_size);

// Runtime objects
typedef enum {
  AVM_ObjValue,
  AVM_ObjEnvFrame
} AVM_object_kind;

typedef struct AVM_object AVM_object_t;

struct AVM_object {
  AVM_object_kind kind;
  AVM_object_t *next;
};

struct AVM_VM;

void *allocate_object(struct AVM_VM *vm, size_t size, AVM_object_kind kind);

AVM_value_t *new_int(struct AVM_VM *vm, int i);
AVM_value_t *new_bool(struct AVM_VM *vm, _Bool b);
AVM_value_t *new_clos(struct AVM_VM *vm, int l, AVM_env_t *env);
