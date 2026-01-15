#pragma once

#include "code.h"

/* #define DEBUG_TRACE_EXECUTION */
/* #define DEBUG_GC_TEST */

#define DEBUG_GC_LOG_LEVEL 0    /* 0 = quiet, 1 = brief, 2 = verbose */

void error(char *fmt, ... );
void disassemble_instruction(AVM_code_t *code, int pc);
