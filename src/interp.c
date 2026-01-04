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
      push(astack, new_int(instr->const_int));
      break;

    case AVM_Ldb:
      push(astack, new_bool(instr->const_bool));
      break;

    case AVM_Access:
      push(astack, lookup(env, instr->access));
      break;

    case AVM_Closure:
      push(astack, new_clos(instr->addr, env));
      break;

    case AVM_Let:
      env = extend(env, pop(astack));
      break;

    case AVM_EndLet:
      env = env->next;
      break;

    case AVM_Jump:
      pc = instr->addr;
      break;

    case AVM_CJump: {
      AVM_value_t *val = pop(astack);
      if (val->kind == AVM_BoolVal && !(val->bool_value)) {
        pc = instr->addr;
      }
      break;
    }

    case AVM_Add: {
      AVM_value_t *val1 = pop(astack);
      AVM_value_t *val2 = pop(astack);

      if (val1->kind != AVM_IntVal || val2->kind != AVM_IntVal) {
        error("AVM_Add: Expected two integer values.");
      }

      push(astack, new_int(val1->int_value + val2->int_value));
      break;
    }

    case AVM_Sub: {
      AVM_value_t *val1 = pop(astack); // y
      AVM_value_t *val2 = pop(astack); // x

      if (val1->kind != AVM_IntVal || val2->kind != AVM_IntVal) {
        error("AVM_Sub: Expected two integer values.");
      }

      push(astack, new_int(val2->int_value - val1->int_value)); // x - y
      break;
    }

    case AVM_Le: {
      AVM_value_t *val1 = pop(astack); // y
      AVM_value_t *val2 = pop(astack); // x

      if (val1->kind != AVM_IntVal || val2->kind != AVM_IntVal) {
        error("AVM_Le: Expected two integer values.");
      }

      push(astack, new_bool(val2->int_value <= val1->int_value)); // x <= y
      break;
    }

    case AVM_Eq: {
      AVM_value_t *val1 = pop(astack);
      AVM_value_t *val2 = pop(astack);

      if (val1->kind != AVM_IntVal || val2->kind != AVM_IntVal) {
        error("AVM_Eq: Expected two integer values.");
      }

      push(astack, new_bool(val1->int_value == val2->int_value));
      break;
    }

    case AVM_Apply: {
      // Pop a function and an argument from astack.
      AVM_value_t *func = pop(astack);
      AVM_value_t *arg = pop(astack);

      if (func->kind != AVM_ClosVal) {
        error("AVM_Apply: Expected function application.");
      }

      // Push the current address and the environment to rstack.
      push(rstack, new_clos(pc, env));
      // Extend the environment.
      env = extend(extend(func->env, func), arg);

      // Jump to the given address.
      pc = func->addr;
      break;
    }

    case AVM_TailApply: {
      // Pop a function and an argument from astack.
      AVM_value_t *func = pop(astack);
      AVM_value_t *arg = pop(astack);

      if (func->kind != AVM_ClosVal) {
        error("AVM_Apply: Expected function application.");
      }

      // Extend the environment.
      env = extend(extend(func->env, func), arg);

      // Jump to the given address.
      pc = func->addr;
      break;
    }

    case AVM_PushMark:
      push(astack, &epsilon);
      break;

    case AVM_Grab: {
      // Pop an argument.
      AVM_value_t *arg = pop(astack);

      if (arg->kind == AVM_Epsilon) {
        // Get the caller's address.
        AVM_value_t *ret_addr = pop(rstack);

        // Push the current address to astack.
        push(astack, new_clos(pc, env));

        // Jump back to the caller.
        pc = ret_addr->addr;
        env = ret_addr->env;
      } else {
        // Extend the current environment and continue.
        env = extend(extend(env, new_clos(pc, env)), arg);
      }
      break;
    }

    case AVM_Return: {
      // Pop two arguments.
      AVM_value_t *arg1 = pop(astack);
      AVM_value_t *arg2 = pop(astack);

      if (arg2->kind == AVM_Epsilon) {
        // Get the caller's address.
        AVM_value_t *ret_addr = pop(rstack);

        // Push the first argument to astack.
        push(astack, arg1);

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

    case AVM_Halt:
      return pop(astack);
    }
  }
}

AVM_value_t* new_int(int i) {
  AVM_value_t* node = malloc(sizeof(AVM_value_t));
  node->kind = AVM_IntVal;
  node->int_value = i;
  return node;
}

AVM_value_t *new_bool(_Bool b) {
  AVM_value_t* node = malloc(sizeof(AVM_value_t));
  node->kind = AVM_BoolVal;
  node->bool_value = b;
  return node;
}

AVM_value_t *new_clos(int l, AVM_env_t *env) {
  AVM_value_t* node = malloc(sizeof(AVM_value_t));
  node->kind = AVM_ClosVal;
  node->addr = l;
  node->env = env;
  return node;
}
