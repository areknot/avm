#pragma once
#include "code.h"
#include "runtime.h"
#include "memory.h"

typedef struct AVM_VM {
  AVM_code_t *code;
  int pc;
  AVM_astack_t *astack;
  AVM_rstack_t *rstack;
  AVM_env_t *env;
  AVM_object_t *objs;
} AVM_VM;

AVM_value_t epsilon;

AVM_VM* init_vm(AVM_code_t *src, _Bool ignite);

void finalize_vm(AVM_VM *vm);
