#pragma once

#include "code.h"
#include "runtime.h"

void init_avm(AVM_code_t *src, _Bool ignite);
AVM_value_t *run();

// Functions for tests
AVM_value_t *_run_code_with_result(AVM_code_t *src);
void _run_code(AVM_code_t *src);

