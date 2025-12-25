#pragma once

#include "code.h"

ZAM_code_t *parse(char *source, int size);

typedef struct {
  char *message;
  int start_byte;
  int start_row;
  int start_col;
  int end_byte;
  int end_row;
  int end_col;
} AVM_parse_error;

AVM_parse_error* last_parse_error();
