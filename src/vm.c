
#include "vm.h"
#include "array.h"
#include "memory.h"
#include "runtime.h"
#include <stdlib.h>

AVM_value_t epsilon = VAL_EPSILON;

AVM_VM* init_vm(AVM_code_t *src, _Bool ignite) {
  AVM_VM *vm = malloc(sizeof(AVM_VM));
  vm->code = src;
  vm->pc = 0;
  vm->objs = NULL;
  vm->astack = init_astack();
  vm->rstack = init_rstack();
  vm->env = init_env(vm);
  vm->allocated_bytes = 0;
  vm->next_gc = MAX_HEAP_SIZE;    /* 1 MiB */

  if (ignite) {
    apush(vm->astack, epsilon);
    AVM_ret_frame_t *end_frame = malloc(sizeof(AVM_ret_frame_t));
    end_frame->addr = src->instr_size;
    end_frame->offset = 0;
    end_frame->penv = make_array(ARRAY_MINIMAL_CAP);
    rpush(vm->rstack, end_frame);
  }

  return vm;
}

void finalize_vm(AVM_VM *vm) {
  /* Free objs */
  while (vm->objs != NULL) {
    AVM_object_t* hd = vm->objs;
    vm->objs = vm->objs->next;
    free_object(vm, hd);
  }
  /* Free return-frames */
  for (size_t i = 0; i < array_size(vm->rstack); ++i) {
    free(array_elem_unsafe(vm->rstack, i));
  }
  drop_array(vm->rstack);
  /* Free environment */
  drop_array(vm->env->cache);
  /* `penv` has been freed. */
  free(vm->env);
  /* Free argument-stack */
  drop_array(vm->astack);
  /* Free the VM */
  free(vm);
}
