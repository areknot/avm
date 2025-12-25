
#include <stdio.h>
#include <code.h>
#include <avm_parser.h>

#define MAX_INPUT 1000

int main(void) {
  char buffer[MAX_INPUT + 10] = {};
  size_t i = 0;
  for (; i < MAX_INPUT; ++i) {
    char c = getc(stdin);
    if (c == EOF)
      break;
    buffer[i] = c;
  }
  buffer[i] = 0;
  ZAM_code_t *code = parse(buffer, i);
  if (code == NULL) {
    /* Error handling */
    AVM_parse_error* e = last_parse_error();
    printf("Encountered error around (Row %d, Column %d) to (Row %d, Column "
           "%d), i.e.,\n\n",
           e->start_row, e->start_col, e->end_row, e->end_col);
    printf("    %.*s\n\n", e->end_byte - e->start_byte, buffer + e->start_byte);
    printf("(%s)\n", e->message);
  } else {
    /* Echo  */
    for (int i = 0; i < code->instr_size; ++i) {
      printf("%3d | ", i);
      ZAM_instr_t instr = code->instr[i];
      switch (instr.kind) {
      case ZAM_Ldi:
        printf("load %d", instr.const_int);
        break;
      case ZAM_Ldb:
        printf("load %s", (instr.const_bool ? "true" : "false"));
        break;
      case ZAM_Access:
        printf("acc  %d", instr.access);
        break;
      case ZAM_Closure:
        printf("clos %d", instr.addr);
        break;
      case ZAM_Let:
        printf("let");
        break;
      case ZAM_EndLet:
        printf("endlet");
	break;
      case ZAM_Jump:
        printf("b    %d", instr.addr);
	break;
      case ZAM_CJump:
        printf("bf   %d", instr.addr);
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
  }
}
