#include <stdio.h>
#include "code.h"
#include "interp.h"
#include "runtime.h"


static _Bool assert_int(AVM_value_t *v, int expected) {
  if (!(v != NULL)) {
    fprintf(stderr, "Expected non NULL.\n");
    return false;
  }

  if (!(is_int(*v))) {
    fprintf(stderr, "Expected IntVal.\n");
    return false;
  }

  if (!(as_int(*v) == expected)) {
    fprintf(stderr, "Expected %d, but got %d.\n", expected, as_int(*v));
    return false;
  }

  return true;
}

static _Bool assert_bool(AVM_value_t *v, _Bool expected) {
  if (!(v != NULL)) {
    fprintf(stderr, "Expected non NULL.\n");
    return false;
  }

  if (!(is_bool(*v))) {
    fprintf(stderr, "Expected BoolVal.\n");
    return false;
  }

  if (!(as_bool(*v) == expected)) {
    fprintf(stderr, "Expected %d, but got %d.\n", expected, as_int(*v));
    return false;
  }

  return true;
}


// left + right
static AVM_code_t make_add_program(int left, int right) {
  static AVM_instr_t program[5];
  program[0] = LDI(right);
  program[1] = PUSH();
  program[2] = LDI(left);
  program[3] = ADD();
  program[4] = HALT();

  return CODE_OF(program);
}

// left == right
static AVM_code_t make_eq_program(int left, int right) {
  static AVM_instr_t program[5];
  program[0] = LDI(left);
  program[1] = PUSH();
  program[2] = LDI(right);
  program[3] = EQ();
  program[4] = HALT();

  return CODE_OF(program);
}

static AVM_code_t make_ldb_program(_Bool b) {
  static AVM_instr_t program[2];
  program[0] = LDB(b);
  program[1] = HALT();
  return CODE_OF(program);
}

// let x = v in x
static AVM_code_t make_let_access_program(int v) {
  static AVM_instr_t program[5];
  program[0] = LDI(v);
  program[1] = LET();
  program[2] = ACCESS(0);
  program[3] = ENDLET();
  program[4] = HALT();

  return CODE_OF(program);
}

// if cond then t else f
static AVM_code_t make_if_program(_Bool cond, int t, int f) {
  static AVM_instr_t program[6];
  program[0] = LDB(cond);
  program[1] = CJUMP(4); // CJump to else clause
  program[2] = LDI(t);
  program[3] = JUMP(5); // Join
  program[4] = LDI(f);
  program[5] = HALT();

  return CODE_OF(program);
}


// (fun x -> x) arg
static AVM_code_t make_id_apply_program(int arg) {
  static AVM_instr_t program[8];
  // Caller
  program[0] = PUSHMARK();
  program[1] = LDI(arg);
  program[2] = PUSH();
  program[3] = CLOSURE(6); // function entry
  program[4] = APPLY();
  program[5] = HALT();
  // Callee
  program[6] = ACCESS(0);
  program[7] = RETURN();

  return CODE_OF(program);
}

// (fun x -> k) arg
static AVM_code_t make_const_apply_program(int arg, int k) {
  static AVM_instr_t program[8];
  // Caller
  program[0] = PUSHMARK();
  program[1] = LDI(arg);
  program[2] = PUSH();
  program[3] = CLOSURE(6);
  program[4] = APPLY();
  program[5] = HALT();
  // Callee: PushMark; Ldi k; Return
  program[6] = LDI(k);
  program[7] = RETURN();

  return CODE_OF(program);
}

// Multiple arguments: (fun x -> fun y -> x + y) x y
static AVM_code_t make_add2_full_program(int x, int y) {
  static AVM_instr_t program[14];

  // caller:
  program[0] = PUSHMARK(); // [e]
  program[1] = LDI(y);
  program[2] = PUSH(); // [y, e]
  program[3] = LDI(x);
  program[4] = PUSH(); // [x, y, e]
  program[5] = CLOSURE(8);
  program[6] = APPLY();
  program[7] = HALT();

  // callee
  // stack: [y, e]
  program[8] = GRAB(); // env = [y, x]
  // stack: [e]
  program[9] = ACCESS(0); // y
  program[10] = PUSH();
  program[11] = ACCESS(1); // x
  program[12] = ADD();
  // stack: [e]
  program[13] = RETURN();

  return CODE_OF(program);
}

// Partial application then second application:
// let f = (fun x -> fun y -> x + y) x in f y => x + y
static AVM_code_t make_add2_partial_program(int x, int y) {
  static AVM_instr_t program[19];

  program[0]  = PUSHMARK();
  program[1]  = LDI(x);
  program[2]  = PUSH();
  program[3]  = CLOSURE(13);
  program[4]  = APPLY();
  program[5]  = LET();
  program[6]  = PUSHMARK();
  program[7]  = LDI(y);
  program[8]  = PUSH();
  program[9]  = ACCESS(0);
  program[10]  = APPLY();
  program[11] = ENDLET();
  program[12] = HALT();
  program[13] = GRAB();
  program[14] = ACCESS(0);
  program[15] = PUSH();
  program[16] = ACCESS(1);
  program[17] = ADD();
  program[18] = RETURN();

  return CODE_OF(program);
}

// TailApply:
// (fun x -> (fun y -> 1 + y) x) x => 1 + x
static AVM_code_t make_tailapply_program(int x) {
  static AVM_instr_t program[15];

  program[0]  = PUSHMARK();
  program[1]  = LDI(x);
  program[2]  = PUSH();
  program[3]  = CLOSURE(6);
  program[4]  = APPLY();
  program[5]  = HALT();
  program[6]  = ACCESS(0);
  program[7]  = PUSH();
  program[8]  = CLOSURE(10);
  program[9]  = TAILAPPLY();
  program[10] = ACCESS(0);
  program[11] = PUSH();
  program[12] = LDI(1);
  program[13] = ADD();
  program[14] = RETURN();

  return CODE_OF(program);
}

// Call given function:
// (fun f -> fun x -> f x) (fun x -> fun y -> x + y) x y
static AVM_code_t make_hof_program(int x, int y) {
  static AVM_instr_t program[22];

  program[0]  = PUSHMARK();
  program[1]  = LDI(y);
  program[2]  = PUSH();
  program[3]  = LDI(x);
  program[4]  = PUSH();
  program[5]  = CLOSURE(15);
  program[6]  = PUSH();
  program[7]  = CLOSURE(10);
  program[8]  = APPLY();
  program[9]  = JUMP(21);
  program[10] = GRAB();
  program[11] = ACCESS(0);
  program[12] = PUSH();
  program[13] = ACCESS(1);
  program[14] = TAILAPPLY();
  program[15] = GRAB();
  program[16] = ACCESS(0);
  program[17] = PUSH();
  program[18] = ACCESS(1);
  program[19] = ADD();
  program[20] = RETURN();
  program[21] = HALT();

  return CODE_OF(program);
}

// x - y
static AVM_code_t make_sub_program(int x, int y) {
  static AVM_instr_t program[5];

  // main:
  program[0] = LDI(y);
  program[1] = PUSH();
  program[2] = LDI(x);
  program[3] = SUB();
  program[4] = HALT();

  return CODE_OF(program);
}

// x >= y
static AVM_code_t make_le_program(int x, int y) {
  static AVM_instr_t program[5];

  // main:
  program[0] = LDI(y);
  program[1] = PUSH();
  program[2] = LDI(x);
  program[3] = LE();
  program[4] = HALT();

  return CODE_OF(program);
}

int main(void) {
  // Test 1: 2 + 3 => 5
  AVM_code_t add_code = make_add_program(2, 3);
  AVM_value_t *add_result = _run_code_with_result(&add_code);
  if (assert_int(add_result, 5))
    printf("Test 1 passed.\n");

  // Test 2: 4 == 4 => true
  AVM_code_t eq_true_code = make_eq_program(4, 4);
  AVM_value_t *eq_true_result = _run_code_with_result(&eq_true_code);
  if (assert_bool(eq_true_result, true))
    printf("Test 2 passed.\n");

  // Test 3: 4 != 5 => false
  AVM_code_t eq_false_code = make_eq_program(4, 5);
  AVM_value_t *eq_false_result = _run_code_with_result(&eq_false_code);
  if (assert_bool(eq_false_result, false))
    printf("Test 3 passed.\n");

  // Test 4: Load true => true
  AVM_code_t ldb_true_code = make_ldb_program(true);
  AVM_value_t *ldb_true_result = _run_code_with_result(&ldb_true_code);
  if (assert_bool(ldb_true_result, true))
    printf("Test 4 passed.\n");

  // Test 5: let x = 10 in x => 10
  AVM_code_t let_access_code = make_let_access_program(10);
  AVM_value_t *let_access_result = _run_code_with_result(&let_access_code);
  if (assert_int(let_access_result, 10))
    printf("Test 5 passed.\n");

  // Test 6: if true then 10 else 20 => 10
  AVM_code_t if_true_code = make_if_program(true, 10, 20);
  AVM_value_t *if_true_result = _run_code_with_result(&if_true_code);
  if (assert_int(if_true_result, 10))
    printf("Test 6 passed.\n");

  // Test 7: if false then 10 else 20 => 20
  AVM_code_t if_false_code = make_if_program(false, 10, 20);
  AVM_value_t *if_false_result = _run_code_with_result(&if_false_code);
  if (assert_int(if_false_result, 20))
    printf("Test 7 passed.\n");

  // Test 8: (fun x -> x) 7 => 7
  AVM_code_t id_apply_code = make_id_apply_program(7);
  AVM_value_t *id_apply_result = _run_code_with_result(&id_apply_code);
  if (assert_int(id_apply_result, 7))
    printf("Test 8 passed.\n");

  // Test 9: (fun x -> 123) 7 => 123
  AVM_code_t const_apply_code = make_const_apply_program(7, 123);
  AVM_value_t *const_apply_result = _run_code_with_result(&const_apply_code);
  if (assert_int(const_apply_result, 123))
    printf("Test 9 passed.\n");

  // Test 10: (fun x -> fun y -> x + y) 10 1 => 11
  AVM_code_t add2_full_code = make_add2_full_program(10, 1);
  AVM_value_t *add2_full_result = _run_code_with_result(&add2_full_code);
  if (assert_int(add2_full_result, 11))
    printf("Test 10 passed.\n");

  // Test 11: let f = (fun x -> fun y -> x + y) 100 in f 10 => 110
  AVM_code_t add2_partial_code = make_add2_partial_program(100, 10);
  AVM_value_t *add2_partial_result = _run_code_with_result(&add2_partial_code);
  if (assert_int(add2_partial_result, 110))
    printf("Test 11 passed.\n");

  // Test 12: (fun x -> (fun y -> 1 + y) x) 10 => 11
  AVM_code_t tailapply_code = make_tailapply_program(10);
  AVM_value_t *tailapply_result = _run_code_with_result(&tailapply_code);
  if (assert_int(tailapply_result, 11))
    printf("Test 12 passed.\n");

  // Test 13: (fun f -> fun x -> f x) (fun x -> fun y -> x + y) 1000 100
  AVM_code_t hof_code = make_hof_program(1000, 100);
  AVM_value_t *hof_result = _run_code_with_result(&hof_code);
  if (assert_int(hof_result, 1100))
    printf("Test 13 passed.\n");

  // Test 14: 10 - 3 = 7
  AVM_code_t sub_code = make_sub_program(10, 3);
  AVM_value_t *sub_result = _run_code_with_result(&sub_code);
  if (assert_int(sub_result, 7))
    printf("Test 14 passed.\n");

  // Test 15: 1 <= 3 = true
  AVM_code_t le_true_code = make_le_program(1, 3);
  AVM_value_t *le_true_result = _run_code_with_result(&le_true_code);
  if (assert_bool(le_true_result, true))
    printf("Test 15 passed.\n");

  // Test 16: 10 <= 3 = false
  AVM_code_t le_false_code = make_le_program(10, 3);
  AVM_value_t *le_false_result = _run_code_with_result(&le_false_code);
  if (assert_bool(le_false_result, false))
    printf("Test 15 passed.\n");

  return 0;
}
