#include <stdio.h>
#include "code.h"
#include "interp.h"
#include "runtime.h"
#include "macros.h"



static _Bool assert_int(ZAM_value_t *v, int expected) {
  if (!(v != NULL)) {
    fprintf(stderr, "Expected non NULL.\n");
    return false;
  }

  if (!(v->kind == ZAM_IntVal)) {
    fprintf(stderr, "Expected IntVal.\n");
    return false;
  }

  if (!(v->int_value == expected)) {
    fprintf(stderr, "Expected %d, but got %d.\n", expected, v->int_value);
    return false;
  }

  return true;
}

static _Bool assert_bool(ZAM_value_t *v, _Bool expected) {
  if (!(v != NULL)) {
    fprintf(stderr, "Expected non NULL.\n");
    return false;
  }

  if (!(v->kind == ZAM_BoolVal)) {
    fprintf(stderr, "Expected BoolVal.\n");
    return false;
  }

  if (!(v->bool_value == expected)) {
    fprintf(stderr, "Expected %d, but got %d.\n", expected, v->bool_value);
    return false;
  }

  return true;
}


// left + right
static ZAM_code_t make_add_program(int left, int right) {
  static ZAM_instr_t program[4];
  program[0] = LDI(left);
  program[1] = LDI(right);
  program[2] = ADD();
  program[3] = HALT();

  return CODE_OF(program);
}

// left == right
static ZAM_code_t make_eq_program(int left, int right) {
  static ZAM_instr_t program[4];
  program[0] = LDI(left);
  program[1] = LDI(right);
  program[2] = EQ();
  program[3] = HALT();

  return CODE_OF(program);
}

static ZAM_code_t make_ldb_program(_Bool b) {
  static ZAM_instr_t program[2];
  program[0] = LDB(b);
  program[1] = HALT();
  return CODE_OF(program);
}

// let x = v in x
static ZAM_code_t make_let_access_program(int v) {
  static ZAM_instr_t program[5];
  program[0] = LDI(v);
  program[1] = LET();
  program[2] = ACCESS(0);
  program[3] = ENDLET();
  program[4] = HALT();

  return CODE_OF(program);
}

// if cond then t else f
static ZAM_code_t make_if_program(_Bool cond, int t, int f) {
  static ZAM_instr_t program[6];
  program[0] = LDB(cond);
  program[1] = CJUMP(4); // CJump to else clause
  program[2] = LDI(t);
  program[3] = JUMP(5); // Join
  program[4] = LDI(f);
  program[5] = HALT();

  return CODE_OF(program);
}


// (fun x -> x) arg
static ZAM_code_t make_id_apply_program(int arg) {
  static ZAM_instr_t program[7];
  // Caller
  program[0] = PUSHMARK();
  program[1] = LDI(arg);
  program[2] = CLOSURE(5); // function entry
  program[3] = APPLY();
  program[4] = HALT();
  // Callee
  program[5] = ACCESS(0);
  program[6] = RETURN();

  return CODE_OF(program);
}

// (fun x -> k) arg
static ZAM_code_t make_const_apply_program(int arg, int k) {
  static ZAM_instr_t program[7];
  // Caller
  program[0] = PUSHMARK();
  program[1] = LDI(arg);
  program[2] = CLOSURE(5);
  program[3] = APPLY();
  program[4] = HALT();
  // Callee: PushMark; Ldi k; Return
  program[5] = LDI(k);
  program[6] = RETURN();

  return CODE_OF(program);
}

// Multiple arguments: (fun x -> fun y -> x + y) x y
static ZAM_code_t make_add2_full_program(int x, int y) {
  static ZAM_instr_t program[11];

  // caller:
  program[0] = PUSHMARK(); // [e]
  program[1] = LDI(y); // [y, e]
  program[2] = LDI(x); // [x, y, e]
  program[3] = CLOSURE(6); // [<add2>, x, y, e]
  program[4] = APPLY();
  program[5] = HALT();

  // callee
  // stack: [y, e]
  program[6] = GRAB(); // env[0]=y, env[1]=<cont>, env[2]=x, env[3]=<add2>
  // stack: [e]
  program[7] = ACCESS(0); // y
  program[8] = ACCESS(2); // x
  program[9] = ADD();
  // stack: [x+y, e]
  program[10] = RETURN();

  return CODE_OF(program);
}

// Partial application then second application:
// let f = (fun x -> fun y -> x + y) x in f y => x + y
static ZAM_code_t make_add2_partial_program(int x, int y) {
  static ZAM_instr_t program[16];

  // main:
  program[0] = PUSHMARK(); // [e]
  program[1] = LDI(x); // [x, e]
  program[2] = CLOSURE(11); // [<11, env>, x, e]
  program[3] = APPLY(); // [<11, env>, x, e] -> [<12, env'>]
  program[4] = LET();
  program[5] = PUSHMARK(); // [e]
  program[6] = LDI(y); // [y, e]
  program[7] = ACCESS(0); // [<12, env'>, y, e]
  program[8] = APPLY(); // [<12, env'>, y, e] -> [x+y, e]
  program[9] = ENDLET();
  program[10] = HALT();

  // (fun x -> fun y -> x + y) x
  // stack = [e], env = [x, <11, [...]>]
  program[11] = GRAB();
  // stack = [e], env = [y, <12, [...]>, x, <11, [...]>]
  program[12] = ACCESS(0);
  program[13] = ACCESS(2);
  program[14] = ADD(); // [x+y, e]
  program[15] = RETURN();

  return CODE_OF(program);
}

// TailApply:
// (fun x -> (fun y -> 1 + y) x) x => 1 + x
static ZAM_code_t make_tailapply_program(int x) {
  static ZAM_instr_t program[12];

  // main:
  program[0] = PUSHMARK();
  program[1] = LDI(x);
  program[2] = CLOSURE(5);
  program[3] = APPLY();
  program[4] = HALT();

  // (fun y -> 1 + y) x
  program[5] = ACCESS(0);
  program[6] = CLOSURE(8);
  program[7] = TAILAPPLY();

  program[8] = ACCESS(0);
  program[9] = LDI(1);
  program[10] = ADD();
  program[11] = RETURN();

  return CODE_OF(program);
}

// Call given function:
// (fun f -> fun x -> f x) (fun x -> fun y -> x + y) x y
static ZAM_code_t make_hof_program(int x, int y) {
  static ZAM_instr_t program[17];

  // main:
  program[0] = PUSHMARK();
  program[1] = LDI(y);
  program[2] = LDI(x);
  program[3] = CLOSURE(11);
  program[4] = CLOSURE(7);
  program[5] = APPLY();
  program[6] = JUMP(16);

  // T(fun x -> f x)
  program[7] = GRAB();
  program[8] = ACCESS(0);
  program[9] = ACCESS(2);
  program[10] = TAILAPPLY();

  // T(fun y -> x + y)
  program[11] = GRAB();
  program[12] = ACCESS(0);
  program[13] = ACCESS(2);
  program[14] = ADD();
  program[15] = RETURN();

  program[16] = HALT();

  return CODE_OF(program);
}

int main(void) {
  // Test 1: 2 + 3 => 5
  ZAM_code_t add_code = make_add_program(2, 3);
  ZAM_value_t *add_result = run_code_with_result(&add_code);
  if (assert_int(add_result, 5))
    printf("Test 1 passed.\n");

  // Test 2: 4 == 4 => true
  ZAM_code_t eq_true_code = make_eq_program(4, 4);
  ZAM_value_t *eq_true_result = run_code_with_result(&eq_true_code);
  if (assert_bool(eq_true_result, true))
    printf("Test 2 passed.\n");

  // Test 3: 4 != 5 => false
  ZAM_code_t eq_false_code = make_eq_program(4, 5);
  ZAM_value_t *eq_false_result = run_code_with_result(&eq_false_code);
  if (assert_bool(eq_false_result, false))
    printf("Test 3 passed.\n");

  // Test 4: Load true => true
  ZAM_code_t ldb_true_code = make_ldb_program(true);
  ZAM_value_t *ldb_true_result = run_code_with_result(&ldb_true_code);
  if (assert_bool(ldb_true_result, true))
    printf("Test 4 passed.\n");

  // Test 5: let x = 10 in x => 10
  ZAM_code_t let_access_code = make_let_access_program(10);
  ZAM_value_t *let_access_result = run_code_with_result(&let_access_code);
  if (assert_int(let_access_result, 10))
    printf("Test 5 passed.\n");

  // Test 6: if true then 10 else 20 => 10
  ZAM_code_t if_true_code = make_if_program(true, 10, 20);
  ZAM_value_t *if_true_result = run_code_with_result(&if_true_code);
  if (assert_int(if_true_result, 10))
    printf("Test 6 passed.\n");

  // Test 7: if false then 10 else 20 => 20
  ZAM_code_t if_false_code = make_if_program(false, 10, 20);
  ZAM_value_t *if_false_result = run_code_with_result(&if_false_code);
  if (assert_int(if_false_result, 20))
    printf("Test 7 passed.\n");

  // Test 8: (fun x -> x) 7 => 7
  ZAM_code_t id_apply_code = make_id_apply_program(7);
  ZAM_value_t *id_apply_result = run_code_with_result(&id_apply_code);
  if (assert_int(id_apply_result, 7))
    printf("Test 8 passed.\n");

  // Test 9: (fun x -> 123) 7 => 123
  ZAM_code_t const_apply_code = make_const_apply_program(7, 123);
  ZAM_value_t *const_apply_result = run_code_with_result(&const_apply_code);
  if (assert_int(const_apply_result, 123))
    printf("Test 9 passed.\n");

  // Test 10: (fun x -> fun y -> x + y) 10 1 => 11
  ZAM_code_t add2_full_code = make_add2_full_program(10, 1);
  ZAM_value_t *add2_full_result = run_code_with_result(&add2_full_code);
  if (assert_int(add2_full_result, 11))
    printf("Test 10 passed.\n");

  // Test 11: let f = (fun x -> fun y -> x + y) 100 in f 10 => 110
  ZAM_code_t add2_partial_code = make_add2_partial_program(100, 10);
  ZAM_value_t *add2_partial_result = run_code_with_result(&add2_partial_code);
  if (assert_int(add2_partial_result, 110))
    printf("Test 11 passed.\n");

  // Test 12: (fun x -> (fun y -> 1 + y) x) 10 => 11
  ZAM_code_t tailapply_code = make_tailapply_program(10);
  ZAM_value_t *tailapply_result = run_code_with_result(&tailapply_code);
  if (assert_int(tailapply_result, 11))
    printf("Test 12 passed.\n");

  // Test 13: (fun f -> fun x -> f x) (fun x -> fun y -> x + y) 1000 100
  ZAM_code_t hof_code = make_hof_program(1000, 100);
  ZAM_value_t *hof_result = run_code_with_result(&hof_code);
  if (assert_int(hof_result, 1100))
    printf("Test 13 passed.\n");

  return 0;
}
