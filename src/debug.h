#pragma once

#include "code.h"

/* #define DEBUG_TRACE_EXECUTION */

void error(char *fmt, ... );
void disassemble_instruction(AVM_code_t *code, int pc);
