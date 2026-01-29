#include "debug.h"
#include "code.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// Custom error report function.
// Takes an error message in the same format as printf.
void error(char *fmt, ... ) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void disassemble_instruction(AVM_code_t *code, int pc) {
  printf("%03d | ", pc);
  AVM_instr_t *instr = &code->instr[pc];
  switch (instr->kind) {
  case AVM_Push:
    printf("push");
    break;
  case AVM_Ldi:
    printf("load %d", instr->const_int);
    break;
  case AVM_Ldb:
    printf("load %s", (instr->const_bool ? "true" : "false"));
    break;
  case AVM_Access:
    printf("acc  %d", instr->access);
    break;
  case AVM_Closure:
    printf("clos %d", instr->addr);
    break;
  case AVM_Let:
    printf("let");
    break;
  case AVM_EndLet:
    printf("endlet");
    break;
  case AVM_Jump:
    printf("b    %d", instr->addr);
    break;
  case AVM_CJump:
    printf("bf   %d", instr->addr);
    break;
  case AVM_Add:
    printf("add");
    break;
  case AVM_Sub:
    printf("sub");
    break;
  case AVM_Le:
    printf("le");
    break;
  case AVM_Eq:
    printf("eq");
    break;
  case AVM_Apply:
    printf("app");
    break;
  case AVM_TailApply:
    printf("tapp");
    break;
  case AVM_PushMark:
    printf("mark");
    break;
  case AVM_Grab:
    printf("grab");
    break;
  case AVM_Return:
    printf("ret");
    break;
  case AVM_Dummies:
    printf("dum  %d", instr->access);
    break;
  case AVM_Update:
    printf("upd  %d", instr->access);
    break;
  case AVM_Halt:
    printf("halt");
    break;
  }
  printf("\n");
}
