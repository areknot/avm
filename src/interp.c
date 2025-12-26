#include "code.h"
#include "runtime.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

ZAM_code_t *code;
int pc;
ZAM_stack_t *astack;
ZAM_stack_t *rstack;
ZAM_env_t *env;
ZAM_value_t epsilon = { .kind = ZAM_Epsilon };

ZAM_value_t *new_int(int i);
ZAM_value_t *new_bool(_Bool b);
ZAM_value_t *new_clos(int l, ZAM_env_t *env);
ZAM_value_t *run();

void init_avm(ZAM_code_t *src, _Bool ignite) {
  code = src;
  pc = 0;
  astack = NULL;
  rstack = NULL;
  env = NULL;

  if (ignite) {
    push(&astack, &epsilon);
    push(&rstack, new_clos(src->instr_size, NULL));
  }
}

void _run_code(ZAM_code_t *src) {
  init_avm(src, false);

  if (code == NULL) return;

  ZAM_value_t *res = run();

  printf("Result: ");
  print_value(res);
}

ZAM_value_t *_run_code_with_result(ZAM_code_t *src) {
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

ZAM_value_t *run() {
  while (true) {
    ZAM_instr_t *instr = code->instr + pc;
    pc++;

    switch (instr->kind) {
    case ZAM_Ldi:
      push(&astack, new_int(instr->const_int));
      break;

    case ZAM_Ldb:
      push(&astack, new_bool(instr->const_bool));
      break;

    case ZAM_Access:
      push(&astack, lookup(env, instr->access));
      break;

    case ZAM_Closure:
      push(&astack, new_clos(instr->addr, env));
      break;

    case ZAM_Let:
      env = extend(env, pop(&astack));
      break;

    case ZAM_EndLet:
      env = env->next;
      break;

    case ZAM_Jump:
      pc = instr->addr;
      break;

    case ZAM_CJump: {
      ZAM_value_t *val = pop(&astack);
      if (val->kind == ZAM_BoolVal && !(val->bool_value)) {
        pc = instr->addr;
      }
      break;
    }

    case ZAM_Add: {
      ZAM_value_t *val1 = pop(&astack);
      ZAM_value_t *val2 = pop(&astack);

      if (val1->kind != ZAM_IntVal || val2->kind != ZAM_IntVal) {
        error("ZAM_Add: Expected two integer values.");
      }

      push(&astack, new_int(val1->int_value + val2->int_value));
      break;
    }

    case ZAM_Eq: {
      ZAM_value_t *val1 = pop(&astack);
      ZAM_value_t *val2 = pop(&astack);

      if (val1->kind != ZAM_IntVal || val2->kind != ZAM_IntVal) {
        error("ZAM_Eq: Expected two integer values.");
      }

      push(&astack, new_bool(val1->int_value == val2->int_value));
      break;
    }

    case ZAM_Apply: {
      // Pop a function and an argument from astack.
      ZAM_value_t *func = pop(&astack);
      ZAM_value_t *arg = pop(&astack);

      if (func->kind != ZAM_ClosVal) {
        error("ZAM_Apply: Expected function application.");
      }

      // Push the current address and the environment to rstack.
      push(&rstack, new_clos(pc, env));
      // Extend the environment.
      env = extend(extend(func->env, func), arg);

      // Jump to the given address.
      pc = func->addr;
      break;
    }

    case ZAM_TailApply: {
      // Pop a function and an argument from astack.
      ZAM_value_t *func = pop(&astack);
      ZAM_value_t *arg = pop(&astack);

      if (func->kind != ZAM_ClosVal) {
        error("ZAM_Apply: Expected function application.");
      }

      // Extend the environment.
      env = extend(extend(func->env, func), arg);

      // Jump to the given address.
      pc = func->addr;
      break;
    }

    case ZAM_PushMark:
      push(&astack, &epsilon);
      break;

    case ZAM_Grab: {
      // Pop an argument.
      ZAM_value_t *arg = pop(&astack);

      if (arg->kind == ZAM_Epsilon) {
        // Get the caller's address.
        ZAM_value_t *ret_addr = pop(&rstack);

        // Push the current address to astack.
        push(&astack, new_clos(pc, env));

        // Jump back to the caller.
        pc = ret_addr->addr;
        env = ret_addr->env;
      } else {
        // Extend the current environment and continue.
        env = extend(extend(env, new_clos(pc, env)), arg);
      }
      break;
    }

    case ZAM_Return: {
      // Pop two arguments.
      ZAM_value_t *arg1 = pop(&astack);
      ZAM_value_t *arg2 = pop(&astack);

      if (arg2->kind == ZAM_Epsilon) {
        // Get the caller's address.
        ZAM_value_t *ret_addr = pop(&rstack);

        // Push the first argument to astack.
        push(&astack, arg1);

        // Jump back to the caller.
        pc = ret_addr->addr;
        env = ret_addr->env;
      } else {
        // Call the first argument with its environment extended.
        env = extend(extend(arg1->env, arg1), arg2);
        pc = arg1->addr;
      }
      break;
    }

    case ZAM_Halt:
      return pop(&astack);
    }
  }
}

ZAM_value_t* new_int(int i) {
  ZAM_value_t* node = malloc(sizeof(ZAM_value_t));
  node->kind = ZAM_IntVal;
  node->int_value = i;
  return node;
}

ZAM_value_t *new_bool(_Bool b) {
  ZAM_value_t* node = malloc(sizeof(ZAM_value_t));
  node->kind = ZAM_BoolVal;
  node->bool_value = b;
  return node;
}

ZAM_value_t *new_clos(int l, ZAM_env_t *env) {
  ZAM_value_t* node = malloc(sizeof(ZAM_value_t));
  node->kind = ZAM_ClosVal;
  node->addr = l;
  node->env = env;
  return node;
}
