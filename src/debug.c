#include "debug.h"
#include <stdio.h>

void disassemble_instruction(ZAM_code_t *code, int pc) {
  printf("%03d | ", pc);
  ZAM_instr_t *instr = &code->instr[pc];
  switch (instr->kind) {
  case ZAM_Ldi:
    printf("load %d", instr->const_int);
    break;
  case ZAM_Ldb:
    printf("load %s", (instr->const_bool ? "true" : "false"));
    break;
  case ZAM_Access:
    printf("acc  %d", instr->access);
    break;
  case ZAM_Closure:
    printf("clos %d", instr->addr);
    break;
  case ZAM_Let:
    printf("let");
    break;
  case ZAM_EndLet:
    printf("endlet");
    break;
  case ZAM_Jump:
    printf("b    %d", instr->addr);
    break;
  case ZAM_CJump:
    printf("bf   %d", instr->addr);
    break;
  case ZAM_Add:
    printf("add");
    break;
  case ZAM_Eq:
    printf("eq");
    break;
  case ZAM_Apply:
    printf("app");
    break;
  case ZAM_TailApply:
    printf("tapp");
    break;
  case ZAM_PushMark:
    printf("mark");
    break;
  case ZAM_Grab:
    printf("grab");
    break;
  case ZAM_Return:
    printf("ret");
    break;
  case ZAM_Halt:
    printf("halt");
    break;
  }
  printf("\n");
}
