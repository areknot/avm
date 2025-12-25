
#include <stdio.h>
#include <string.h>
#include "code.h"
#include "parser.h"

int main(int argc, char **argv) {
  char *source = "load 1; rext;; endlet\n";
  int size = strlen(source);
  ZAM_code_t *code = parse(source, size, NULL);
  printf("%p\n", code);
  if (code != NULL) {
    printf("%d\n", code->instr_size);
  }
}
