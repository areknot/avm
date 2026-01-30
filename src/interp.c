
#include "array.h"
#include "code.h"
#include "debug.h"
#include "interp.h"
#include "memory.h"
#include "runtime.h"
#include "vm.h"
#include <stdarg.h>
#include <stddef.h>
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

void print_instr(AVM_VM* vm) {
  printf("\n");
  int pc = vm->pc == 0 ? 0 : vm->pc - 1;
  disassemble_instruction(vm->code, pc);
}

/* Note that `vm->accu` will only be updated before a potential GC. */
AVM_value_t run(AVM_VM* vm) {

  static void* dispatch_table[] = {
    [AVM_Push]      = &&OP_AVM_Push,
    [AVM_Ldi]       = &&OP_AVM_Ldi,
    [AVM_Ldb]       = &&OP_AVM_Ldb,
    [AVM_Access]    = &&OP_AVM_Access,
    [AVM_Closure]   = &&OP_AVM_Closure,
    [AVM_Let]       = &&OP_AVM_Let,
    [AVM_EndLet]    = &&OP_AVM_EndLet,
    [AVM_Jump]      = &&OP_AVM_Jump,
    [AVM_CJump]     = &&OP_AVM_CJump,
    [AVM_Add]       = &&OP_AVM_Add,
    [AVM_Sub]       = &&OP_AVM_Sub,
    [AVM_Le]        = &&OP_AVM_Le,
    [AVM_Eq]        = &&OP_AVM_Eq,
    [AVM_Apply]     = &&OP_AVM_Apply,
    [AVM_TailApply] = &&OP_AVM_TailApply,
    [AVM_PushMark]  = &&OP_AVM_PushMark,
    [AVM_Grab]      = &&OP_AVM_Grab,
    [AVM_Return]    = &&OP_AVM_Return,
    [AVM_Dummies]   = &&OP_AVM_Dummies,
    [AVM_Update]    = &&OP_AVM_Update,
    [AVM_Halt]      = &&OP_AVM_Halt,
  };

  AVM_instr_t* instr = NULL;
  register AVM_value_t  accu  = 0;
  
#define DISPATCH()                                        \
  do {                                                    \
    instr = vm->code->instr + vm->pc;                     \
    goto *dispatch_table[vm->code->instr[vm->pc++].kind]; \
} while (0)
  /* Debug is disabled now. */
#ifdef DEBUG_TRACE_EXECUTION
#define DEBUG_MESSAGE() print_instr(vm)
#else
#define DEBUG_MESSAGE()
#endif

  DISPATCH();

 OP_AVM_Push:
  DEBUG_MESSAGE();
  if (!apush(vm->astack, accu))
    error("AVM_Push: Couldn't push %X.", accu);
  DISPATCH();
  
 OP_AVM_Ldi:
  DEBUG_MESSAGE();
  accu = new_int(vm, instr->const_int);
  DISPATCH();

 OP_AVM_Ldb:
  DEBUG_MESSAGE();
  accu = new_bool(vm, instr->const_bool);
  DISPATCH();

 OP_AVM_Access: 
  DEBUG_MESSAGE();
  /* To be confirmed: `lookup` is total? */
  accu = lookup(vm->env, instr->access);
  DISPATCH();
  

 OP_AVM_Closure: 
  DEBUG_MESSAGE();
  vm->accu = accu;
  perpetuate(vm, vm->env);
  accu = new_clos(vm, instr->addr, vm->env->penv);
  DISPATCH();

 OP_AVM_Let:
  DEBUG_MESSAGE();
  vm->env = extend(vm->env, accu);
  if (vm->env == NULL)
    error("AVM_Let: Couldn't extend the environment.");
  DISPATCH();

 OP_AVM_EndLet:
  DEBUG_MESSAGE();
  remove_head(vm, vm->env);
  DISPATCH();

 OP_AVM_Jump:
  DEBUG_MESSAGE();
  vm->pc = instr->addr;
  DISPATCH();

 OP_AVM_CJump: 
  DEBUG_MESSAGE();
  if (!is_bool(accu)) {
    error("AVM_CJump: Expected a bool value.");
  } else if (!as_bool(accu)) {
    vm->pc = instr->addr;
  }
  DISPATCH();

 OP_AVM_Add: {
    DEBUG_MESSAGE();
    AVM_value_t val1 = accu;
    AVM_value_t val2 = apop(vm->astack);

    if (!is_int(val1) || !is_int(val2)) {
      error("AVM_Add: Expected two integer values.");
    }

    accu = new_int(vm, as_int(val1) + as_int(val2));
    DISPATCH();
  }

 OP_AVM_Sub: {
    DEBUG_MESSAGE();
    AVM_value_t val1 = apop(vm->astack); // y
    AVM_value_t val2 = accu; // x

    if (!is_int(val1) || !is_int(val2)) {
      error("AVM_Sub: Expected two integer values.");
    }

    accu = new_int(vm, as_int(val2) - as_int(val1));
    DISPATCH();
  }

 OP_AVM_Le: {
    DEBUG_MESSAGE();
    AVM_value_t val1 = apop(vm->astack); // y
    AVM_value_t val2 = accu; // x

    if (!is_int(val1) || !is_int(val2)) {
      print_value(val1);
      putchar('\n');
      print_value(val2);
      putchar('\n');
      print_env(vm->env);
      putchar('\n');
      error("AVM_Le: Expected two integer values.");
    }

    accu = new_bool(vm, as_int(val2) <= as_int(val1));
    DISPATCH();
  }

 OP_AVM_Eq: {
    DEBUG_MESSAGE();
    AVM_value_t val1 = accu;
    AVM_value_t val2 = apop(vm->astack);

    if (!is_int(val1) || !is_int(val2)) {
      error("AVM_Eq: Expected two integer values.");
    }

    accu = new_bool(vm, as_int(val1) == as_int(val2));
    DISPATCH();
  }

 OP_AVM_Apply: {
    DEBUG_MESSAGE();
    AVM_value_t func = accu;
    AVM_value_t arg = apop(vm->astack);

    AVM_object_t *obj = as_obj(func);

    if (!is_obj(func) || obj->kind != AVM_ObjClos) {
      error("AVM_Apply: Expected function application.");
    }

    AVM_clos_t *clos = (AVM_clos_t*)(obj + 1);

    // Push the current address and the environment to rstack.
    /* To be confirmed: Do not we store vm->cache here? */
    AVM_ret_frame_t *new_frame = malloc(sizeof(AVM_ret_frame_t));
    new_frame->addr   = vm->pc;
    new_frame->penv   = vm->env->penv;
    new_frame->offset = vm->env->offset;
    if (!rpush(vm->rstack, new_frame))
      error("AVM_Apply: Couldn't push the return address");

    // Extend the environment.
    vm->env->offset = vm->env->cache->size;
    vm->env->penv   = clos->penv;
    if (extend(vm->env, arg) == NULL)
      error("AVM_Apply: Couldn't extend the environment.");

    // Jump to the given address.
    vm->pc = clos->addr;
    DISPATCH();
  }

 OP_AVM_TailApply: {
    DEBUG_MESSAGE();
    // Pop a function and an argument from astack.
    AVM_value_t func = accu;
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
    if (extend(vm->env, arg) == NULL)
      error("AVM_Apply: Couldn't extend the environment.");

    // Jump to the given address.
    vm->pc = clos->addr;
    DISPATCH();
  }

 OP_AVM_PushMark:
  DEBUG_MESSAGE();
  if (!apush(vm->astack, epsilon))
    error("AVM_PushMark: Couldn't push the mark.");
  DISPATCH();

 OP_AVM_Grab: {
    DEBUG_MESSAGE();
    // Pop an argument.
    AVM_value_t arg = apop(vm->astack);
    vm->accu = accu;

    if (is_epsilon(arg)) {
      // Push the current address to astack.
      perpetuate(vm, vm->env);
      accu = new_clos(vm, vm->pc, vm->env->penv);

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
      if (extend(vm->env, arg) == NULL)
	error("AVM_Grab: Couldn't extend the environment.");
    }
    DISPATCH();
  }

 OP_AVM_Return: {
    DEBUG_MESSAGE();
    AVM_value_t arg1 = accu;
    AVM_value_t arg2 = apop(vm->astack);
    AVM_object_t *obj;

    if (is_epsilon(arg2)) {
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
      if (extend(vm->env, arg2) == NULL)
	error("AVM_Grab: Couldn't extend the environment.");
      vm->pc = clos->addr;
    }
    DISPATCH();
  }

 OP_AVM_Dummies: {
    DEBUG_MESSAGE();
    vm->accu = accu;
    int count = instr->access;
    for (int i = 0; i < count; ++i)
      if (!extend(vm->env, new_clos(vm, 0, NULL)))
	error("AMV_Dummies: Couldn't extend the environment.");
    DISPATCH();
  }

 OP_AVM_Update: {
    DEBUG_MESSAGE();
    int idx = instr->access;
    AVM_value_t dummy = lookup(vm->env, idx);
    AVM_object_t* dummy_obj = NULL;
    AVM_object_t* accu_obj = NULL;

    if (!(is_obj(dummy)
	  && (dummy_obj = as_obj(dummy), dummy_obj->kind == AVM_ObjClos)))
      error("AVM_Update: Expected env[%d] to be a closure", idx);
    if (!(is_obj(accu)
	  && (accu_obj = as_obj(accu), accu_obj->kind == AVM_ObjClos)))
      error("AVM_Update: Expected accu to be a closure");

    AVM_clos_t* dummy_clos = (AVM_clos_t*)(dummy_obj + 1);
    AVM_clos_t* accu_clos  = (AVM_clos_t*)(accu_obj  + 1);
    dummy_clos->addr = accu_clos->addr;
    dummy_clos->penv = accu_clos->penv;
    DISPATCH();
  }

 OP_AVM_Halt:
    return accu;
}
