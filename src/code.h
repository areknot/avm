#pragma once

#include <stdbool.h>

typedef enum {
  ZAM_Ldi      , ZAM_Ldb   , ZAM_Access    ,
  ZAM_Closure  , ZAM_Let   , ZAM_EndLet    ,
  ZAM_Jump     , ZAM_CJump , ZAM_Add       ,
  ZAM_Eq       , ZAM_Apply , ZAM_TailApply ,
  ZAM_PushMark , ZAM_Grab  , ZAM_Return    ,
  ZAM_Halt
} ZAM_instr_kind;

struct ZAM_instr;
struct ZAM_code;

typedef struct ZAM_instr {
  ZAM_instr_kind   kind;
  int              const_int;
  _Bool            const_bool;
  int              access;
  int              addr; // for Closure and Jumps
} ZAM_instr_t;

typedef struct ZAM_code {
  ZAM_instr_t* instr;
  int          instr_size;
} ZAM_code_t;
