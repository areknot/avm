#pragma once

#include "runtime.h"

#define AVM_INIT_PROMPT   0
#define AVM_SEGMENT_CAP 128

typedef struct AVM_segment {
  unsigned int prompt;		/* Unused currently. */
  size_t size;
  AVM_value_t contents[AVM_SEGMENT_CAP];
  struct AVM_segment* next;
} AVM_segment_t;

typedef struct AVM_stack {
  AVM_segment_t* cache;
  AVM_segment_t* top;
} AVM_stack_t;

AVM_stack_t* init_stack();

void drop_stack(AVM_stack_t* stack);

typedef enum {
  AVM_stack_push_failure,
  AVM_stack_push_overflow,
  AVM_stack_push_success
} AVM_stack_push_code;
AVM_stack_push_code push_stack(AVM_stack_t* stack, AVM_value_t elt);

typedef enum {
  AVM_stack_pop_failure,
  AVM_stack_pop_success,
  AVM_stack_pop_underflow
} AVM_stack_pop_code;
AVM_stack_pop_code pop_stack(AVM_stack_t* stack, AVM_value_t* popped);


