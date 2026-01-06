
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "code.h"
#include "avm_parser.h"
#include "runtime.h"
#include "vm.h"
#include "interp.h"

#define MAX_INPUT_SIZE 8192

int read_file(char *buf, size_t size, FILE *fp);

int main(int argc, char *argv[]) {
  if (argc > 2) {
    fprintf(stderr, "Usage: %s <filename>?\n", argv[0]);
    return 1;
  }

  FILE *fp;

  if (argc == 1) {
    fp = stdin;
  } else {
    fp = fopen(argv[1], "r");
  }

  if (fp == NULL) {
    fprintf(stderr, "Failed to open file\n");
    return 1;
  }

  char buf[MAX_INPUT_SIZE] = {};
  int size = read_file(buf, MAX_INPUT_SIZE, fp);

  if (fclose(fp) == EOF) {
    fprintf(stderr, "Failed to close file\n");
    return 1;
  }

  AVM_code_t *code = parse(buf, size);

  if (code == NULL) {
    AVM_parse_error *e = last_parse_error();
    printf("Encountered error around (Row %d, Column %d) to (Row %d, Column "
           "%d), i.e.,\n\n",
           e->start_row, e->start_col, e->end_row, e->end_col);
    printf("    %.*s\n\n", e->end_byte - e->start_byte, buf + e->start_byte);
    printf("(%s)\n", e->message);
    return 1;
  }

  AVM_instr_t *new_instr = realloc(code->instr, (code->instr_size + 10) * sizeof(AVM_instr_t));
  if (new_instr == NULL) {
    fprintf(stderr, "Failed to reallocate instructions.\n");
    return 1;
  }

  code->instr = new_instr;
  code->instr[code->instr_size] = HALT();

  AVM_VM *vm = init_vm(code, true);
  AVM_value_t *res = run(vm);

  printf("Result: ");
  print_value(res);
  printf("\n");

  finalize_vm(vm);

  return 0;
}

int read_file(char *buf, size_t size, FILE *fp) {
  size_t i = 0;

  for (; i < size; ++i) {
    char c = fgetc(fp);
    if(c == EOF) {
      if (feof(fp)) {
        buf[i] = 0;
        return i;
        break;
      } else {
        fprintf(stderr, "Failed to read file\n");
        exit(EXIT_FAILURE);
      }
    }
    buf[i] = c;
  }

  fprintf(stderr, "Input is too large\n");
  exit(EXIT_FAILURE);
}
