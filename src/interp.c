#include "interp.h"
#include "debug.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

AVM_code_t *code;
int pc;
AVM_stack_t *astack;
AVM_stack_t *rstack;
AVM_env_t *env;
AVM_value_t epsilon = { .kind = AVM_Epsilon };

AVM_value_t *new_int(int i);
AVM_value_t *new_bool(_Bool b);
AVM_value_t *new_clos(int l, AVM_env_t *env);
AVM_value_t *run();

void init_avm(AVM_code_t *src, _Bool ignite) {
  code = src;
  pc = 0;
  astack = init_stack();
  rstack = init_stack();
  env = NULL;

  if (ignite) {
    push(astack, &epsilon);
    push(rstack, new_clos(src->instr_size, NULL));
  }
}

void _run_code(AVM_code_t *src) {
  init_avm(src, false);

  if (code == NULL) return;

  AVM_value_t *res = run();

  printf("Result: ");
  print_value(res);
  printf("\n");
}

AVM_value_t *_run_code_with_result(AVM_code_t *src) {
  init_avm(src, false);

  if (code == NULL) return NULL;

  return run();
}

// Custom error report function.
// Takes an error message in the same format as printf.
void error(char *fmt, ... ) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

AVM_value_t *run() {
  while (true) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("\n");
    disassemble_instruction(code, pc);
#endif

    AVM_instr_t *instr = code->instr + pc;
    pc++;

    switch (instr->kind) {
    case AVM_Ldi:
      if (!push(astack, new_int(instr->const_int)))
        error("AVM_Ldi: Couldn't push %d.", instr->const_int);
      break;

    case AVM_Ldb:
      if (!push(astack, new_bool(instr->const_bool)))
        error("AVM_Ldb: Couldn't push %s.", instr->const_bool ? "true" : "false");
      break;

    case AVM_Access: {
      AVM_value_t *val = lookup(env, instr->access);
      if (val == NULL)
        error("AVM_Access: Couldn't find the variable %d.", instr->access);

      if (!push(astack, val))
        error("AVM_Access: Couldn't push the found value.");

      break;
    }

    case AVM_Closure: {
      AVM_value_t *clos = new_clos(instr->addr, env);
      if (clos == NULL)
        error("AVM_Closure: Couldn't create a new closure.");

      if (!push(astack, clos))
        error("AVM_CLosure: Couldn't push the closure.");
      break;

    }


    case AVM_Let: {
      env = extend(env, pop(astack));
      if (env == NULL)
        error("AVM_Let: Couldn't extend the environment.");

      break;
    }

    case AVM_EndLet:
      env = env->next;
      break;

    case AVM_Jump:
      pc = instr->addr;
      break;

    case AVM_CJump: {
      AVM_value_t *val = pop(astack);
      if (val == NULL || val->kind != AVM_BoolVal) {
        error("AVM_CJump: Expected a bool value.");
      } else if (!val->bool_value) {
        pc = instr->addr;
      }
      break;
    }

    case AVM_Add: {
      AVM_value_t *val1 = pop(astack);
      AVM_value_t *val2 = pop(astack);

      if (val1 == NULL || val2 == NULL || val1->kind != AVM_IntVal || val2->kind != AVM_IntVal) {
        error("AVM_Add: Expected two integer values.");
      }

      if (!push(astack, new_int(val1->int_value + val2->int_value))) // x + y
        error("AVM_Add: Couldn't push the result.");
      break;
    }

    case AVM_Sub: {
      AVM_value_t *val1 = pop(astack); // y
      AVM_value_t *val2 = pop(astack); // x

      if (val1 == NULL || val2 == NULL || val1->kind != AVM_IntVal || val2->kind != AVM_IntVal) {
        error("AVM_Sub: Expected two integer values.");
      }

      if (!push(astack, new_int(val2->int_value - val1->int_value))) // x - y
        error("AVM_Sub: Couldn't push the result.");
      break;
    }

    case AVM_Le: {
      AVM_value_t *val1 = pop(astack); // y
      AVM_value_t *val2 = pop(astack); // x

      if (val1 == NULL || val2 == NULL || val1->kind != AVM_IntVal || val2->kind != AVM_IntVal) {
        error("AVM_Le: Expected two integer values.");
      }

      if (!push(astack, new_bool(val2->int_value <= val1->int_value))) // x <= y
        error("AVM_Le: Couldn't push the result.");
      break;
    }

    case AVM_Eq: {
      AVM_value_t *val1 = pop(astack);
      AVM_value_t *val2 = pop(astack);

      if (val1 == NULL || val2 == NULL || val1->kind != AVM_IntVal || val2->kind != AVM_IntVal) {
        error("AVM_Eq: Expected two integer values.");
      }

      if (!push(astack, new_bool(val1->int_value == val2->int_value))) // x == y
        error("AVM_Eq: Couldn't push the result.");
      break;
    }

    case AVM_Apply: {
      // Pop a function and an argument from astack.
      AVM_value_t *func = pop(astack);
      AVM_value_t *arg = pop(astack);

      if (func == NULL || arg == NULL || func->kind != AVM_ClosVal) {
        error("AVM_Apply: Expected function application.");
      }

      // Push the current address and the environment to rstack.
      if (!push(rstack, new_clos(pc, env)))
        error("AVM_Apply: Couldn't push the return address");

      // Extend the environment.
      AVM_env_t *tmp = extend(func->env, func);
      if (tmp == NULL)
        error("AVM_Apply: Couldn't extend the environment.");

      env = extend(tmp, arg);
      if (env == NULL)
        error("AVM_Apply: Couldn't extend the environment.");

      // Jump to the given address.
      pc = func->addr;
      break;
    }

    case AVM_TailApply: {
      // Pop a function and an argument from astack.
      AVM_value_t *func = pop(astack);
      AVM_value_t *arg = pop(astack);

      if (func == NULL || arg == NULL || func->kind != AVM_ClosVal) {
        error("AVM_Apply: Expected function application.");
      }

      // Extend the environment.
      AVM_env_t *tmp = extend(func->env, func);
      if (tmp == NULL)
        error("AVM_Apply: Couldn't extend the environment.");

      env = extend(tmp, arg);
      if (env == NULL)
        error("AVM_Apply: Couldn't extend the environment.");

      // Jump to the given address.
      pc = func->addr;
      break;
    }

    case AVM_PushMark:
      if (!push(astack, &epsilon))
        error("AVM_PushMark: Couldn't push the mark.");
      break;

    case AVM_Grab: {
      // Pop an argument.
      AVM_value_t *arg = pop(astack);

      if (arg == NULL) {
        error("AVM_Grab: Expected an argument.");
      } else if (arg->kind == AVM_Epsilon) {
        // Get the caller's address.
        AVM_value_t *ret_addr = pop(rstack);
        if (ret_addr == NULL)
          error("AVM_Grab: Couldn't get the caller's address.");

        // Push the current address to astack.
        AVM_value_t *tmp = new_clos(pc, env);
        if (tmp == NULL)
          error("AVM_Grab: Couldn't create a new closure.");
        if (!push(astack, tmp))
          error("AVM_Grab: Couldn't push the current address.");

        // Jump back to the caller.
        pc = ret_addr->addr;
        env = ret_addr->env;
      } else {
        // Extend the current environment and continue.
        AVM_value_t *cur_addr = new_clos(pc, env);
        if (cur_addr == NULL)
          error("AVM_Grab: Couldn't create a new closure.");

        AVM_env_t * tmp = extend(env, cur_addr);
        if (tmp == NULL)
          error("AVM_Grab: Couldn't extend the environment.");
        env = extend(tmp, arg);
        if (env == NULL)
          error("AVM_Grab: Couldn't extend the environment.");
      }
      break;
    }

    case AVM_Return: {
      // Pop two arguments.
      AVM_value_t *arg1 = pop(astack);
      AVM_value_t *arg2 = pop(astack);

      if (arg1 == NULL || arg2 == NULL) {
        error("AVM_Return: Expected two items on the argument stack.");
      } else if (arg2->kind == AVM_Epsilon) {
        // Get the caller's address.
        AVM_value_t *ret_addr = pop(rstack);
        if (ret_addr == NULL)
          error("AVM_Return: Couldn't get the caller's address.");

        // Push the first argument to astack.
        if (!push(astack, arg1))
          error("AVM_Return: Couldn't push the result to the argument stack.");

        // Jump back to the caller.
        pc = ret_addr->addr;
        env = ret_addr->env;
      } else if (arg1->kind != AVM_ClosVal) {
        error("AVM_Return: Invalid return address.");
      } else {
        // Call the first argument with its environment extended.
        AVM_env_t *tmp = extend(arg1->env, arg1);
        if (tmp == NULL)
          error("AVM_Return: Couldn't extend the environment.");

        env = extend(tmp, arg2);
        if (env == NULL)
          error("AVM_Grab: Couldn't extend the environment.");
        pc = arg1->addr;
      }
      break;
    }

    case AVM_Halt: {
      AVM_value_t *res =  pop(astack);
      if (res == NULL)
        error("AVM_Halt: Expected a result.");

      return res;
    }

    }
  }
}

AVM_value_t* new_int(int i) {
  AVM_value_t* node = malloc(sizeof(AVM_value_t));
  if (node == NULL)
    error("new_int: Failed to allocate memory.");
  node->kind = AVM_IntVal;
  node->int_value = i;
  return node;
}

AVM_value_t *new_bool(_Bool b) {
  AVM_value_t* node = malloc(sizeof(AVM_value_t));
  if (node == NULL)
    error("new_int: Failed to allocate memory.");
  node->kind = AVM_BoolVal;
  node->bool_value = b;
  return node;
}

AVM_value_t *new_clos(int l, AVM_env_t *env) {
  AVM_value_t* node = malloc(sizeof(AVM_value_t));
  if (node == NULL)
    error("new_int: Failed to allocate memory.");
  node->kind = AVM_ClosVal;
  node->addr = l;
  node->env = env;
  return node;
}
