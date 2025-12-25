#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include "code.h"
#include "interp.h"


// left + right
ZAM_code_t make_add_program(int left, int right) {
  static ZAM_instr_t program[4];
  program[0] = (ZAM_instr_t){ .kind = ZAM_Ldi, .const_int = left };
  program[1] = (ZAM_instr_t){ .kind = ZAM_Ldi, .const_int = right};
  program[2] = (ZAM_instr_t){ .kind = ZAM_Add };
  program[3] = (ZAM_instr_t){ .kind = ZAM_Halt };

  return (ZAM_code_t){ .instr = program, .instr_size = 4 };
}

// left == right
ZAM_code_t make_eq_program(int left, int right) {
  static ZAM_instr_t program[4];
  program[0] = (ZAM_instr_t){ .kind = ZAM_Ldi, .const_int = left };
  program[1] = (ZAM_instr_t){ .kind = ZAM_Ldi, .const_int = right};
  program[2] = (ZAM_instr_t){ .kind = ZAM_Eq };
  program[3] = (ZAM_instr_t){ .kind = ZAM_Halt };

  return (ZAM_code_t){ .instr = program, .instr_size = 4 };
}

int main(void) {
  // Test 1: 2 + 3 => 5
  ZAM_code_t add_code = make_add_program(2, 3);
  ZAM_value_t *add_result = run_code_with_result(&add_code);
  assert(add_result != NULL);
  assert(add_result->kind == ZAM_IntVal);
  assert(add_result->int_value == 5);
  printf("Test 1 passed.\n");

  // Test 2: 4 == 4 => true
  ZAM_code_t eq_true_code = make_eq_program(4, 4);
  ZAM_value_t *eq_true_result = run_code_with_result(&eq_true_code);
  assert(eq_true_result != NULL);
  assert(eq_true_result->kind == ZAM_BoolVal);
  assert(eq_true_result->bool_value);
  printf("Test 2 passed.\n");

  // Test 3: 4 != 5 => false
  ZAM_code_t eq_false_code = make_eq_program(4, 5);
  ZAM_value_t *eq_false_result = run_code_with_result(&eq_false_code);
  assert(eq_false_result != NULL);
  assert(eq_false_result->kind == ZAM_BoolVal);
  assert(!eq_false_result->bool_value);
  printf("Test 3 passed.\n");

  return 0;
}
