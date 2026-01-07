
#include "vm.h"
#include "array.h"
#include "runtime.h"
#include <stdlib.h>

AVM_value_t epsilon = { .kind = AVM_Epsilon };

AVM_VM* init_vm(AVM_code_t *src, _Bool ignite) {
  AVM_VM *vm = malloc(sizeof(AVM_VM));
  vm->code = src;
  vm->pc = 0;
  vm->astack = init_astack();
  vm->rstack = init_rstack();
  vm->env = init_env();
  vm->objs = NULL;

  if (ignite) {
    apush(vm->astack, &epsilon);
    AVM_ret_frame_t *end_frame = malloc(sizeof(AVM_ret_frame_t));
    end_frame->addr = src->instr_size;
    end_frame->offset = 0;
    end_frame->penv = make_array(ARRAY_MINIMAL_CAP);
    rpush(vm->rstack, end_frame);
  }

  return vm;
}


void finalize_vm(AVM_VM *vm) { (void)vm; } /* TODO: implement this! */
