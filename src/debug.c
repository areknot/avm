#include "debug.h"
#include <stdio.h>

void disassemble_instruction(AVM_code_t *code, int pc) {
  printf("%03d | ", pc);
  AVM_instr_t *instr = &code->instr[pc];
  switch (instr->kind) {
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
  case AVM_Halt:
    printf("halt");
    break;
  }
  printf("\n");
}
