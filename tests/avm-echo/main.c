
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
  AVM_code_t *code = parse(buffer, i);
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
      AVM_instr_t instr = code->instr[i];
      switch (instr.kind) {
      case AVM_Ldi:
        printf("load %d", instr.const_int);
        break;
      case AVM_Ldb:
        printf("load %s", (instr.const_bool ? "true" : "false"));
        break;
      case AVM_Access:
        printf("acc  %d", instr.access);
        break;
      case AVM_Closure:
        printf("clos %d", instr.addr);
        break;
      case AVM_Let:
        printf("let");
        break;
      case AVM_EndLet:
        printf("endlet");
	break;
      case AVM_Jump:
        printf("b    %d", instr.addr);
	break;
      case AVM_CJump:
        printf("bf   %d", instr.addr);
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
  }
}
