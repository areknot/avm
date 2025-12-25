#pragma once

#include "code.h"
#include "runtime.h"

ZAM_value_t *run_code_with_result(ZAM_code_t *src);
void run_code(ZAM_code_t *src);
