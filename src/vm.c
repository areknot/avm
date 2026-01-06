
#include "vm.h"
#include "memory.h"
#include "runtime.h"
#include <stdlib.h>

AVM_value_t epsilon = { .kind = AVM_Epsilon };

AVM_VM* init_vm(AVM_code_t *src, _Bool ignite) {
  AVM_VM *vm = malloc(sizeof(AVM_VM));
  vm->code = src;
  vm->pc = 0;
  vm->astack = init_stack();
  vm->rstack = init_stack();
  vm->env = init_env(vm);
  vm->objs = NULL;

  if (ignite) {
    push(vm->astack, &epsilon);
    push(vm->rstack, new_clos(vm, src->instr_size, NULL));
  }

  return vm;
}


void finalize_vm(AVM_VM *vm) { (void)vm; } /* TODO: implement this! */
