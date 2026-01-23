
#include "array.h"
#include "debug.h"
#include "interp.h"
#include "memory.h"
#include "runtime.h"
#include "vm.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void _run_code(AVM_code_t *src) {
  AVM_VM *vm = init_vm(src, false);

  if (vm->code == NULL) return;

  AVM_value_t res = run(vm);

  printf("Result: ");
  print_value(res);
  printf("\n");

  finalize_vm(vm);
}

AVM_value_t *_run_code_with_result(AVM_code_t *src) {
  AVM_VM *vm = init_vm(src, false);

  if (vm->code == NULL) return NULL;

  AVM_value_t res = run(vm);
  /* Copy the result */
  AVM_value_t* res_ = malloc(sizeof(AVM_value_t));
  memcpy(res_, &res, sizeof(AVM_value_t));
  finalize_vm(vm);
  return res_;
}

AVM_value_t run(AVM_VM* vm) {
  while (true) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("\n");
    disassemble_instruction(vm->code, vm->pc);
#endif

    AVM_instr_t *instr = vm->code->instr + vm->pc;
    vm->pc++;

    switch (instr->kind) {
    case AVM_Ldi:
      if (!apush(vm->astack, new_int(vm, instr->const_int)))
        error("AVM_Ldi: Couldn't push %d.", instr->const_int);
      break;

    case AVM_Ldb:
      if (!apush(vm->astack, new_bool(vm, instr->const_bool)))
        error("AVM_Ldb: Couldn't push %s.", instr->const_bool ? "true" : "false");
      break;

    case AVM_Access: {
      AVM_value_t val = lookup(vm->env, instr->access);

      if (!apush(vm->astack, val))
        error("AVM_Access: Couldn't push the found value.");

      break;
    }

    case AVM_Closure: {
      perpetuate(vm, vm->env);
      AVM_value_t clos = new_clos(vm, instr->addr, vm->env->penv);

      if (!apush(vm->astack, clos))
        error("AVM_CLosure: Couldn't push the closure.");
      break;

    }


    case AVM_Let: {
      vm->env = extend(vm->env, apop(vm->astack));
      if (vm->env == NULL)
        error("AVM_Let: Couldn't extend the environment.");

      break;
    }

    case AVM_EndLet:
      remove_head(vm, vm->env);
      break;

    case AVM_Jump:
      vm->pc = instr->addr;
      break;

    case AVM_CJump: {
      AVM_value_t val = apop(vm->astack);
      if (!is_bool(val)) {
        error("AVM_CJump: Expected a bool value.");
      } else if (!as_bool(val)) {
        vm->pc = instr->addr;
      }
      break;
    }

    case AVM_Add: {
      AVM_value_t val1 = apop(vm->astack);
      AVM_value_t val2 = apop(vm->astack);

      if (!is_int(val1) || !is_int(val2)) {
        error("AVM_Add: Expected two integer values.");
      }

      if (!apush(vm->astack, new_int(vm, as_int(val1) + as_int(val2)))) // x + y
        error("AVM_Add: Couldn't push the result.");
      break;
    }

    case AVM_Sub: {
      AVM_value_t val1 = apop(vm->astack); // y
      AVM_value_t val2 = apop(vm->astack); // x

      if (!is_int(val1) || !is_int(val2)) {
        error("AVM_Sub: Expected two integer values.");
      }

      if (!apush(vm->astack, new_int(vm, as_int(val2) - as_int(val1)))) // x - y
        error("AVM_Sub: Couldn't push the result.");
      break;
    }

    case AVM_Le: {
      AVM_value_t val1 = apop(vm->astack); // y
      AVM_value_t val2 = apop(vm->astack); // x

      if (!is_int(val1) || !is_int(val2)) {
        error("AVM_Le: Expected two integer values.");
      }

      if (!apush(vm->astack, new_bool(vm, as_int(val2) <= as_int(val1)))) // x <= y
        error("AVM_Le: Couldn't push the result.");
      break;
    }

    case AVM_Eq: {
      AVM_value_t val1 = apop(vm->astack);
      AVM_value_t val2 = apop(vm->astack);

      if (!is_int(val1) || !is_int(val2)) {
        error("AVM_Eq: Expected two integer values.");
      }

      if (!apush(vm->astack, new_bool(vm, as_int(val1) == as_int(val2)))) // x == y
        error("AVM_Eq: Couldn't push the result.");
      break;
    }

    case AVM_Apply: {
      // Pop a function and an argument from astack.
      AVM_value_t func = apop(vm->astack);
      AVM_value_t arg = apop(vm->astack);

      AVM_object_t *obj = as_obj(func);

      if (!is_obj(func) || obj->kind != AVM_ObjClos) {
        error("AVM_Apply: Expected function application.");
      }

      AVM_clos_t *clos = (AVM_clos_t*)(obj + 1);

      // Push the current address and the environment to rstack.
      AVM_ret_frame_t *new_frame = malloc(sizeof(AVM_ret_frame_t));
      new_frame->addr = vm->pc;
      new_frame->penv = vm->env->penv;
      new_frame->offset = vm->env->offset;
      if (!rpush(vm->rstack, new_frame))
        error("AVM_Apply: Couldn't push the return address");

      // Extend the environment.
      vm->env->offset = vm->env->cache->size;
      vm->env->penv = clos->penv;
      if (extend(vm->env, func) == NULL)
        error("AVM_Apply: Couldn't extend the environment.");
      if (extend(vm->env, arg) == NULL)
        error("AVM_Apply: Couldn't extend the environment.");

      // Jump to the given address.
      vm->pc = clos->addr;
      break;
    }

    case AVM_TailApply: {
      // Pop a function and an argument from astack.
      AVM_value_t func = apop(vm->astack);
      AVM_value_t arg = apop(vm->astack);

      AVM_object_t *obj = as_obj(func);

      if (!is_obj(func) || obj->kind != AVM_ObjClos) {
        error("AVM_Apply: Expected function application.");
      }

      AVM_clos_t *clos = (AVM_clos_t*)(obj + 1);

      // Extend the environment.
      /* vm->env->offset = vm->env->cache.size; */
      pop_array_n(vm->env->cache, vm->env->cache->size - vm->env->offset);
      vm->env->penv = clos->penv;
      if (extend(vm->env, func) == NULL)
        error("AVM_Apply: Couldn't extend the environment.");
      if (extend(vm->env, arg) == NULL)
        error("AVM_Apply: Couldn't extend the environment.");

      // Jump to the given address.
      vm->pc = clos->addr;
      break;
    }

    case AVM_PushMark:
      if (!apush(vm->astack, epsilon))
        error("AVM_PushMark: Couldn't push the mark.");
      break;

    case AVM_Grab: {
      // Pop an argument.
      AVM_value_t arg = apop(vm->astack);

      if (is_epsilon(arg)) {
        // Push the current address to astack.
        perpetuate(vm, vm->env);
        AVM_value_t tmp = new_clos(vm, vm->pc, vm->env->penv);
        if (!apush(vm->astack, tmp))
          error("AVM_Grab: Couldn't push the current address.");

        // Get the caller's address.
        AVM_ret_frame_t *ret_frame = rpop(vm->rstack);
        if (ret_frame == NULL)
          error("AVM_Grab: Couldn't get the caller's address.");

        // Jump back to the caller.
        vm->pc = ret_frame->addr;
        vm->env->penv = ret_frame->penv;
        vm->env->offset = ret_frame->offset;

        free(ret_frame);
      } else {
        // Extend the current environment and continue.
        /* perpetuate(vm->env); */
        /* AVM_value_t *tmp = new_clos(vm, vm->pc, vm->env); */
        /* if (tmp == NULL) */
        /*   error("AVM_Grab: Couldn't create a new closure."); */
        // Note: we do NOT allow recursive call to curried function.
        if (extend(vm->env, epsilon) == NULL)
          error("AVM_Grab: Couldn't extend the environment.");
        if (extend(vm->env, arg) == NULL)
          error("AVM_Grab: Couldn't extend the environment.");
      }
      break;
    }

    case AVM_Return: {
      // Pop two arguments.
      AVM_value_t arg1 = apop(vm->astack);
      AVM_value_t arg2 = apop(vm->astack);
      AVM_object_t *obj;

      if (is_epsilon(arg2)) {
        // Push the first argument to astack.
        if (!apush(vm->astack, arg1))
          error("AVM_Return: Couldn't push the result to the argument stack.");

        // Get the caller's address.
        AVM_ret_frame_t *ret_frame = rpop(vm->rstack);
        if (ret_frame == NULL)
          error("AVM_Return: Couldn't get the caller's address.");

        // Jump back to the caller.
        vm->pc = ret_frame->addr;
        vm->env->penv = ret_frame->penv;
        pop_array_n(vm->env->cache, vm->env->cache->size - vm->env->offset);
        vm->env->offset = ret_frame->offset;
        free(ret_frame);
      } else if ((obj = as_obj(arg1)) || obj->kind != AVM_ObjClos) {
          error("AVM_Return: Invalid return address.");
        } else {
          AVM_clos_t *clos = (AVM_clos_t*)(obj + 1);

          /* Pop all the contents of the current cache, change the penv to that of arg1,
             and extend the environment. */
          pop_array_n(vm->env->cache, vm->env->cache->size - vm->env->offset);
          vm->env->penv = clos->penv;
          if (extend(vm->env, arg1) == NULL)
            error("AVM_Return: Couldn't extend the environment.");
          if (extend(vm->env, arg2) == NULL)
            error("AVM_Grab: Couldn't extend the environment.");
          vm->pc = clos->addr;
        }
      break;
    }

    case AVM_Halt: {
      AVM_value_t res =  apop(vm->astack);

      return res;
    }

    }
  }
}
