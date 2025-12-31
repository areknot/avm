#pragma once

#include <stdbool.h>

typedef enum {
  AVM_Ldi      , AVM_Ldb   , AVM_Access    ,
  AVM_Closure  , AVM_Let   , AVM_EndLet    ,
  AVM_Jump     , AVM_CJump , AVM_Add       ,
  AVM_Eq       , AVM_Apply , AVM_TailApply ,
  AVM_PushMark , AVM_Grab  , AVM_Return    ,
  AVM_Halt
} AVM_instr_kind;

struct AVM_instr;
struct AVM_code;

typedef struct AVM_instr {
  AVM_instr_kind   kind;
  int              const_int;
  _Bool            const_bool;
  int              access;
  int              addr; // for Closure and Jumps
  void*            payload;
} AVM_instr_t;

typedef struct AVM_code {
  AVM_instr_t* instr;
  int          instr_size;
} AVM_code_t;

#define HALT()      ((AVM_instr_t){ .kind = AVM_Halt })
#define LDI(n)      ((AVM_instr_t){ .kind = AVM_Ldi,     .const_int  = (n) })
#define LDB(b)      ((AVM_instr_t){ .kind = AVM_Ldb,     .const_bool = (b) })
#define ACCESS(i)   ((AVM_instr_t){ .kind = AVM_Access,  .access     = (i) })

#define ADD()       ((AVM_instr_t){ .kind = AVM_Add })
#define EQ()        ((AVM_instr_t){ .kind = AVM_Eq })

#define LET()       ((AVM_instr_t){ .kind = AVM_Let })
#define ENDLET()    ((AVM_instr_t){ .kind = AVM_EndLet })

#define PUSHMARK()  ((AVM_instr_t){ .kind = AVM_PushMark })
#define GRAB()      ((AVM_instr_t){ .kind = AVM_Grab })

#define CLOSURE(a)  ((AVM_instr_t){ .kind = AVM_Closure, .addr = (a) })
#define APPLY()     ((AVM_instr_t){ .kind = AVM_Apply })
#define TAILAPPLY() ((AVM_instr_t){ .kind = AVM_TailApply })
#define RETURN()    ((AVM_instr_t){ .kind = AVM_Return })

#define JUMP(a)     ((AVM_instr_t){ .kind = AVM_Jump,  .addr = (a) })
#define CJUMP(a)    ((AVM_instr_t){ .kind = AVM_CJump, .addr = (a) })

#define CODE_OF(arr) ((AVM_code_t){ .instr = (arr), .instr_size = (int)(sizeof(arr)/sizeof((arr)[0]))})
