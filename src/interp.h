#pragma once

#include "code.h"
#include "runtime.h"

void init_avm(ZAM_code_t *src, _Bool ignite);
ZAM_value_t *run();

// Functions for tests
ZAM_value_t *_run_code_with_result(ZAM_code_t *src);
void _run_code(ZAM_code_t *src);

