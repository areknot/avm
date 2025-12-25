#include "code.h"

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
