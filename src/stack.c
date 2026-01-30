
#include "stack.h"
#include "runtime.h"
#include <assert.h>
#include <stdlib.h>

AVM_segment_t* make_segment() {
  return malloc(sizeof(AVM_segment_t));
}

void drop_segment(AVM_segment_t* seg) {
  free(seg);
}

AVM_stack_t* init_stack() {
  AVM_segment_t* init_seg = make_segment();
  init_seg->prompt = AVM_INIT_PROMPT;
  init_seg->size = 0;
  init_seg->next = NULL;
  AVM_stack_t* stack = malloc(sizeof(AVM_stack_t));
  stack->cache = NULL;
  stack->top   = init_seg;
  return stack;
}

void drop_stack(AVM_stack_t* stack) {
  /* Dropping the segments */
  if (stack->cache != NULL) drop_segment(stack->cache);
  AVM_segment_t* seg = stack->top;
  while (seg != NULL) {
    AVM_segment_t* seg_ = seg;
    seg = seg->next;
    drop_segment(seg_);
  }
  /* Dropping the stack */
  free(stack);
}

AVM_stack_push_code push_stack(AVM_stack_t* stack, AVM_value_t elt) {
  AVM_stack_push_code code = AVM_stack_push_success;
  if (stack->top->size >= AVM_SEGMENT_CAP) {
    /* Stack overflow */
    code = AVM_stack_push_overflow;
    AVM_segment_t* top_ = NULL;
    if (stack->cache != NULL) {
      top_ = stack->cache;
      stack->cache = NULL;
      assert(top_->size == 0);
    } else {
      top_ = make_segment();
      if (top_ == NULL) return AVM_stack_push_failure;
      top_->size = 0;
    }
    top_->prompt = stack->top->prompt;
    top_->next   = stack->top;
    stack->top   = top_;
  }
  stack->top->contents[stack->top->size++] = elt;
  return code;
}

AVM_stack_pop_code pop_stack(AVM_stack_t* stack, AVM_value_t* popped) {
  assert(popped != NULL);
  AVM_stack_pop_code code = AVM_stack_pop_success;
  AVM_segment_t* top = stack->top;
  assert(top != NULL);
  if (top->size == 0) {
    assert(top->next == NULL);
    return AVM_stack_pop_failure; 
  }
  *popped = top->contents[--top->size];
  if (top->size == 0 && top->next != NULL) {
    /* Stack underflow */
    code = AVM_stack_pop_underflow;
    if (stack->cache != NULL) drop_segment(stack->cache);
    stack->cache = top;
    stack->top   = top->next;;
  }
  return code;
}

void stack_foreach(AVM_stack_t* stack, void (*action)(AVM_value_t)) {
  AVM_segment_t* seg = stack->top;
  while (seg != NULL) {
    for (AVM_value_t* vp = seg->contents + seg->size - 1; vp >= seg->contents; --vp)
      action(*vp);
    seg = seg->next;
  }
}
