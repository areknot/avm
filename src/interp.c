
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

  AVM_value_t *res = run(vm);

  printf("Result: ");
  print_value(res);
  printf("\n");

  finalize_vm(vm);
}

AVM_value_t *_run_code_with_result(AVM_code_t *src) {
  AVM_VM *vm = init_vm(src, false);

  if (vm->code == NULL) return NULL;

  AVM_value_t *res = run(vm);
  /* Copy the result */
  AVM_value_t* res_ = malloc(sizeof(AVM_value_t));
  memcpy(res_, res, sizeof(AVM_value_t));
  finalize_vm(vm);
  return res_;
}

void print_instr(AVM_VM* vm) {
  printf("\n");
  disassemble_instruction(vm->code, vm->pc - 1);
}

AVM_value_t *run(AVM_VM* vm) {

  static void* dispatch_table[] = {
    [AVM_Ldi] = &&OP_AVM_Ldi,
    [AVM_Ldb] = &&OP_AVM_Ldb,
    [AVM_Access] = &&OP_AVM_Access,
    [AVM_Closure] = &&OP_AVM_Closure,
    [AVM_Let] = &&OP_AVM_Let,
    [AVM_EndLet] = &&OP_AVM_EndLet,
    [AVM_Jump] = &&OP_AVM_Jump,
    [AVM_CJump] = &&OP_AVM_CJump,
    [AVM_Add] = &&OP_AVM_Add,
    [AVM_Sub] = &&OP_AVM_Sub,
    [AVM_Le] = &&OP_AVM_Le,
    [AVM_Eq] = &&OP_AVM_Eq,
    [AVM_Apply] = &&OP_AVM_Apply,
    [AVM_TailApply] = &&OP_AVM_TailApply,
    [AVM_PushMark] = &&OP_AVM_PushMark,
    [AVM_Grab] = &&OP_AVM_Grab,
    [AVM_Return] = &&OP_AVM_Return,
    [AVM_Halt] = &&OP_AVM_Halt,

  };
#define DISPATCH() goto *dispatch_table[vm->code->instr[vm->pc++].kind]

  AVM_instr_t *instr;
  DISPATCH();

 OP_AVM_Ldi: {
    instr = vm->code->instr + vm->pc - 1;
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    if (!apush(vm->astack, new_int(vm, instr->const_int)))
      error("AVM_Ldi: Couldn't push %d.", instr->const_int);
    DISPATCH();
  }

 OP_AVM_Ldb: {
    instr = vm->code->instr + vm->pc - 1;
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    if (!apush(vm->astack, new_bool(vm, instr->const_bool)))
      error("AVM_Ldb: Couldn't push %s.", instr->const_bool ? "true" : "false");

    DISPATCH();
  }

 OP_AVM_Access: {
    instr = vm->code->instr + vm->pc - 1;
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    AVM_value_t *val = lookup(vm->env, instr->access);
    if (val == NULL)
      error("AVM_Access: Couldn't find the variable %d.", instr->access);

    if (!apush(vm->astack, val))
      error("AVM_Access: Couldn't push the found value.");

    DISPATCH();
  }

 OP_AVM_Closure: {
    instr = vm->code->instr + vm->pc - 1;
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    perpetuate(vm, vm->env);
    AVM_value_t *clos = new_clos(vm, instr->addr, vm->env->penv);
    if (clos == NULL)
      error("AVM_Closure: Couldn't create a new closure.");

    if (!apush(vm->astack, clos))
      error("AVM_CLosure: Couldn't push the closure.");

    DISPATCH();
  }


 OP_AVM_Let: {
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    vm->env = extend(vm->env, apop(vm->astack));
    if (vm->env == NULL)
      error("AVM_Let: Couldn't extend the environment.");

    DISPATCH();
  }

 OP_AVM_EndLet:
#ifdef DEBUG_TRACE_EXECUTION
  print_instr(vm);
#endif
  remove_head(vm, vm->env);
  DISPATCH();

 OP_AVM_Jump: {
    instr = vm->code->instr + vm->pc - 1;
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    vm->pc = instr->addr;
    DISPATCH();
  }

 OP_AVM_CJump: {
    instr = vm->code->instr + vm->pc - 1;
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    AVM_value_t *val = apop(vm->astack);
    if (val == NULL || val->kind != AVM_BoolVal) {
      error("AVM_CJump: Expected a bool value.");
    } else if (!val->bool_value) {
      vm->pc = instr->addr;
    }
    DISPATCH();
  }

 OP_AVM_Add: {
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    AVM_value_t *val1 = apop(vm->astack);
    AVM_value_t *val2 = apop(vm->astack);

    if (val1 == NULL || val2 == NULL || val1->kind != AVM_IntVal || val2->kind != AVM_IntVal) {
      error("AVM_Add: Expected two integer values.");
    }

    if (!apush(vm->astack, new_int(vm, val1->int_value + val2->int_value))) // x + y
      error("AVM_Add: Couldn't push the result.");
    DISPATCH();
  }

 OP_AVM_Sub: {
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    AVM_value_t *val1 = apop(vm->astack); // y
    AVM_value_t *val2 = apop(vm->astack); // x

    if (val1 == NULL || val2 == NULL || val1->kind != AVM_IntVal || val2->kind != AVM_IntVal) {
      error("AVM_Sub: Expected two integer values.");
    }

    if (!apush(vm->astack, new_int(vm, val2->int_value - val1->int_value))) // x - y
      error("AVM_Sub: Couldn't push the result.");
    DISPATCH();
  }

 OP_AVM_Le: {
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    AVM_value_t *val1 = apop(vm->astack); // y
    AVM_value_t *val2 = apop(vm->astack); // x

    if (val1 == NULL || val2 == NULL || val1->kind != AVM_IntVal || val2->kind != AVM_IntVal) {
      error("AVM_Le: Expected two integer values.");
    }

    if (!apush(vm->astack, new_bool(vm, val2->int_value <= val1->int_value))) // x <= y
      error("AVM_Le: Couldn't push the result.");
    DISPATCH();
  }

 OP_AVM_Eq: {
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    AVM_value_t *val1 = apop(vm->astack);
    AVM_value_t *val2 = apop(vm->astack);

    if (val1 == NULL || val2 == NULL || val1->kind != AVM_IntVal || val2->kind != AVM_IntVal) {
      error("AVM_Eq: Expected two integer values.");
    }

    if (!apush(vm->astack, new_bool(vm, val1->int_value == val2->int_value))) // x == y
      error("AVM_Eq: Couldn't push the result.");
    DISPATCH();
  }

 OP_AVM_Apply: {
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    // Pop a function and an argument from astack.
    AVM_value_t *func = apop(vm->astack);
    AVM_value_t *arg = apop(vm->astack);

    if (func == NULL || arg == NULL || func->kind != AVM_ClosVal) {
      error("AVM_Apply: Expected function application.");
    }

    // Push the current address and the environment to rstack.
    AVM_ret_frame_t *new_frame = malloc(sizeof(AVM_ret_frame_t));
    new_frame->addr = vm->pc;
    new_frame->penv = vm->env->penv;
    new_frame->offset = vm->env->offset;
    if (!rpush(vm->rstack, new_frame))
      error("AVM_Apply: Couldn't push the return address");

    // Extend the environment.
    vm->env->offset = vm->env->cache->size;
    vm->env->penv = func->clos_value.penv;
    if (extend(vm->env, func) == NULL)
      error("AVM_Apply: Couldn't extend the environment.");
    if (extend(vm->env, arg) == NULL)
      error("AVM_Apply: Couldn't extend the environment.");

    // Jump to the given address.
    vm->pc = func->clos_value.addr;
    DISPATCH();
  }

 OP_AVM_TailApply: {
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    // Pop a function and an argument from astack.
    AVM_value_t *func = apop(vm->astack);
    AVM_value_t *arg = apop(vm->astack);

    if (func == NULL || arg == NULL || func->kind != AVM_ClosVal) {
      error("AVM_Apply: Expected function application.");
    }

    // Extend the environment.
    /* vm->env->offset = vm->env->cache.size; */
    pop_array_n(vm->env->cache, vm->env->cache->size - vm->env->offset);
    vm->env->penv = func->clos_value.penv;
    if (extend(vm->env, func) == NULL)
      error("AVM_Apply: Couldn't extend the environment.");
    if (extend(vm->env, arg) == NULL)
      error("AVM_Apply: Couldn't extend the environment.");

    // Jump to the given address.
    vm->pc = func->clos_value.addr;
    DISPATCH();
  }

 OP_AVM_PushMark:
#ifdef DEBUG_TRACE_EXECUTION
  print_instr(vm);
#endif
  if (!apush(vm->astack, &epsilon))
    error("AVM_PushMark: Couldn't push the mark.");
  DISPATCH();

 OP_AVM_Grab: {
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    // Pop an argument.
    AVM_value_t *arg = apop(vm->astack);

    if (arg == NULL) {
      error("AVM_Grab: Expected an argument.");
    } else if (arg->kind == AVM_Epsilon) {
      // Push the current address to astack.
      perpetuate(vm, vm->env);
      AVM_value_t *tmp = new_clos(vm, vm->pc, vm->env->penv);
      if (tmp == NULL)
        error("AVM_Grab: Couldn't create a new closure.");
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
      if (extend(vm->env, &epsilon) == NULL)
        error("AVM_Grab: Couldn't extend the environment.");
      if (extend(vm->env, arg) == NULL)
        error("AVM_Grab: Couldn't extend the environment.");
    }
    DISPATCH();
  }

 OP_AVM_Return: {
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    // Pop two arguments.
    AVM_value_t *arg1 = apop(vm->astack);
    AVM_value_t *arg2 = apop(vm->astack);

    if (arg1 == NULL || arg2 == NULL) {
      error("AVM_Return: Expected two items on the argument stack.");
    } else if (arg2->kind == AVM_Epsilon) {
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
    } else if (arg1->kind != AVM_ClosVal) {
      error("AVM_Return: Invalid return address.");
    } else {
      /* Pop all the contents of the current cache, change the penv to that of arg1,
         and extend the environment. */
      pop_array_n(vm->env->cache, vm->env->cache->size - vm->env->offset);
      vm->env->penv = arg1->clos_value.penv;
      if (extend(vm->env, arg1) == NULL)
        error("AVM_Return: Couldn't extend the environment.");
      if (extend(vm->env, arg2) == NULL)
        error("AVM_Grab: Couldn't extend the environment.");
      vm->pc = arg1->clos_value.addr;
    }
    DISPATCH();
  }

 OP_AVM_Halt: {
#ifdef DEBUG_TRACE_EXECUTION
    print_instr(vm);
#endif
    AVM_value_t *res =  apop(vm->astack);
    if (res == NULL)
      error("AVM_Halt: Expected a result.");

    return res;
  }

}
