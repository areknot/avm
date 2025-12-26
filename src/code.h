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
  void*            payload;
} ZAM_instr_t;

typedef struct ZAM_code {
  ZAM_instr_t* instr;
  int          instr_size;
} ZAM_code_t;

#define HALT()      ((ZAM_instr_t){ .kind = ZAM_Halt })
#define LDI(n)      ((ZAM_instr_t){ .kind = ZAM_Ldi,     .const_int  = (n) })
#define LDB(b)      ((ZAM_instr_t){ .kind = ZAM_Ldb,     .const_bool = (b) })
#define ACCESS(i)      ((ZAM_instr_t){ .kind = ZAM_Access,  .access     = (i) })

#define ADD()       ((ZAM_instr_t){ .kind = ZAM_Add })
#define EQ()        ((ZAM_instr_t){ .kind = ZAM_Eq })

#define LET()       ((ZAM_instr_t){ .kind = ZAM_Let })
#define ENDLET()    ((ZAM_instr_t){ .kind = ZAM_EndLet })

#define PUSHMARK()  ((ZAM_instr_t){ .kind = ZAM_PushMark })
#define GRAB()      ((ZAM_instr_t){ .kind = ZAM_Grab })

#define CLOSURE(a)  ((ZAM_instr_t){ .kind = ZAM_Closure, .addr = (a) })
#define APPLY()     ((ZAM_instr_t){ .kind = ZAM_Apply })
#define TAILAPPLY() ((ZAM_instr_t){ .kind = ZAM_TailApply })
#define RETURN()    ((ZAM_instr_t){ .kind = ZAM_Return })

#define JUMP(a)     ((ZAM_instr_t){ .kind = ZAM_Jump,  .addr = (a) })
#define CJUMP(a)    ((ZAM_instr_t){ .kind = ZAM_CJump, .addr = (a) })

#define CODE_OF(arr) ((ZAM_code_t){ .instr = (arr), .instr_size = (int)(sizeof(arr)/sizeof((arr)[0]))})
