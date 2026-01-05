#pragma once
#include <stddef.h>
#include "runtime.h"
#include "vm.h"

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
  union {
    AVM_value_t val;
    struct AVM_env_node env_frame;
  } as;
  AVM_object_t *next;
};

AVM_object_t *allocate_object(size_t size, AVM_object_kind type);

AVM_value_t* new_int(int i);
AVM_value_t *new_bool(_Bool b);
AVM_value_t *new_clos(int l, AVM_env_t *env);
