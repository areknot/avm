#pragma once

#include "code.h"
#include "runtime.h"
#include "vm.h"

AVM_value_t *run(AVM_VM* vm);

// Functions for tests
AVM_value_t *_run_code_with_result(AVM_code_t *src);
void _run_code(AVM_code_t *src);

