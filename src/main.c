
#include <stdio.h>
#include <string.h>
#include "code.h"
#include "parser.h"

int main(void) {
  char *source = "load 1; ret;; endlet\n";
  int size = strlen(source);
  ZAM_code_t *code = parse(source, size);
  if (code != NULL) {
    printf("%d\n", code->instr_size);
  } else {
    AVM_parse_error* err = last_parse_error();
    printf("%s: %d:%d ~ %d:%d (%.*s)\n", err->message, err->start_row,
           err->start_col, err->end_row, err->end_col,
           err->end_byte - err->start_byte, source + err->start_byte);
  }
}
