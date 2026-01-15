#pragma once
#include "code.h"
#include "runtime.h"
#include "memory.h"

#define MAX_HEAP_SIZE  128 * 1024 * 1024
#define MIN_HEAP_SIZE    4 * 1024 * 1024

typedef struct AVM_VM {
  AVM_code_t *code;
  int pc;
  AVM_astack_t *astack;
  AVM_rstack_t *rstack;
  AVM_env_t *env;
  AVM_object_t *objs;
  size_t allocated_bytes;
  size_t next_gc;
} AVM_VM;

extern AVM_value_t epsilon;

AVM_VM* init_vm(AVM_code_t *src, _Bool ignite);

void finalize_vm(AVM_VM *vm);
