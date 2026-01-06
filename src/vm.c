
#include "vm.h"
#include "memory.h"

AVM_value_t epsilon = { .kind = AVM_Epsilon };

void init_vm(AVM_VM *vm, AVM_code_t *src, _Bool ignite) {
  vm->code = src;
  vm->pc = 0;
  vm->astack = init_stack();
  vm->rstack = init_stack();
  vm->env = NULL;
  vm->objs = NULL;

  if (ignite) {
    push(vm->astack, &epsilon);
    push(vm->rstack, new_clos(src->instr_size, NULL));
  }
}


void finalize_vm(AVM_VM *vm) { (void)vm; } /* TODO: implement this! */
