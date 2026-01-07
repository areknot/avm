
#include "runtime.h"
#include "array.h"
/* #include "memory.h" */
#include "debug.h"
#include <stdlib.h>
#include <stdio.h>

// TODO: Add better error handling
void print_astack(AVM_astack_t *st);
void print_rstack(AVM_astack_t *st);
void print_ret_frame(AVM_ret_frame_t *f);
void print_env(AVM_env_t *env);

AVM_astack_t* init_astack() {
  return make_array(ARRAY_MINIMAL_CAP);
}

void drop_astack(AVM_astack_t* stack) {
  drop_array(stack);
}

AVM_value_t* apop(AVM_astack_t* stp) {
  AVM_value_t* val = array_last(stp);
  int count = pop_array(stp);
  if (count == 0) return NULL;

#ifdef DEBUG_TRACE_EXECUTION
  printf("Popped from astack:\n  ");
  print_value(val);
  printf(" <- ");
  print_astack(stp);
  printf("\n");
#endif

  return val;
}

_Bool apush(AVM_astack_t* stp, AVM_value_t* val) {
#ifdef DEBUG_TRACE_EXECUTION
  printf("Pushed to astack:\n  ");
  print_value(val);
  printf(" -> ");
  print_astack(stp);
  printf("\n");
#endif
  int count = push_array(stp, val);
  return (count != 0);
}

AVM_rstack_t* init_rstack() {
  return make_array(ARRAY_MINIMAL_CAP);
}

void drop_rstack(AVM_rstack_t* stack) {
  drop_array(stack);
}

AVM_ret_frame_t* rpop(AVM_rstack_t* stp) {
  AVM_ret_frame_t* f = array_last(stp);
  int count = pop_array(stp);
  if (count == 0) return NULL;

#ifdef DEBUG_TRACE_EXECUTION
  printf("Popped from rstack:\n  ");
  print_ret_frame(f);
  printf(" <- ");
  print_rstack(stp);
  printf("\n");
#endif

  return f;
}

_Bool rpush(AVM_rstack_t* stp, AVM_ret_frame_t *frame) {
#ifdef DEBUG_TRACE_EXECUTION
  printf("Pushed to rstack:\n  ");
  print_ret_frame(frame);
  printf(" -> ");
  print_rstack(stp);
  printf("\n");
#endif
  int count = push_array(stp, frame);
  return (count != 0);
}

AVM_env_t* init_env() {
  AVM_env_t *new_env = malloc(sizeof(AVM_env_t));
  new_env->cache = *make_array(ARRAY_MINIMAL_CAP);
  new_env->penv = make_array(ARRAY_MINIMAL_CAP);
  new_env->offset = 0;

/* #ifdef DEBUG_TRACE_EXECUTION */
/*   printf("Env initialized:\n"); */
/*   printf("penv @ %p, size: %zu, capacity: %zu\n", new_env->penv, new_env->penv->size, new_env->penv->capacity); */
/*   printf("cache @ %p, size: %zu, capacity: %zu, offset: %zu\n  ", &new_env->cache, new_env->cache.size, new_env->cache.capacity, new_env->offset); */
/* #endif */
  return new_env;
}

AVM_env_t* extend(AVM_env_t *env, AVM_value_t *val) {
#ifdef DEBUG_TRACE_EXECUTION
  printf("Extended env:\n  ");
  print_value(val);
  printf(" -> ");
  print_env(env);
  printf("\n");
#endif
  if (push_array(&env->cache, val) == 0)
    return NULL;
  return env;
}

#define GET_CURRENT_SIZE(env) ((env)->cache.size - (env)->offset)

AVM_value_t* lookup(AVM_env_t *env, size_t index) {
  if (index < GET_CURRENT_SIZE(env)) {
    AVM_value_t* res = array_elem(&env->cache, env->cache.size - index - 1);
    if (res == NULL)
      error("lookup: Failed to find the variable %d.", index);
#ifdef DEBUG_TRACE_EXECUTION
    printf("Found ");
    print_value(res);
    printf(" in ");
    print_env(env);
    printf("\n");
#endif
    return res;
  } else {
    AVM_value_t* res = array_elem(env->penv, env->penv->size - (index - GET_CURRENT_SIZE(env)) - 1);
    if (res == NULL)
      error("lookup: Failed to find the variable %d.", index);
#ifdef DEBUG_TRACE_EXECUTION
    printf("Found ");
    print_value(res);
    printf(" in ");
    print_env(env);
    printf("\n");
#endif
    return res;
  }
}

void perpetuate(AVM_env_t *env) {
  array_t *new_penv = copy(env->penv);
  if (push_array_offset(new_penv, &env->cache, env->offset) == -1)
    error("perpetuate: Failed to reserve the memory for a new environment.");

  pop_array_n(&env->cache, env->cache.size - env->offset);
  env->penv = new_penv;
}

void remove_head(AVM_env_t *env) {
  // Case 1: cache is not empty
  if (env->cache.size > env->offset) {
    pop_array(&env->cache);
    return;
  }

  // Case 2: cache is empty
  push_array_all(&env->cache, env->penv);
  if (pop_array(&env->cache) == 0)
    error("remove_head: The environment is empty.");

  env->penv = make_array(ARRAY_MINIMAL_CAP);
}

void print_value(AVM_value_t *val) {
  switch (val->kind) {
  case AVM_IntVal:
    printf("%d", val->int_value);
    break;

  case AVM_BoolVal:
    printf("%s", val->bool_value ? "true" : "false");
    break;

  case AVM_ClosVal:
    printf("<clos(%d, %p)>", val->clos_value.addr, val->clos_value.penv);
    break;

  case AVM_Epsilon:
    printf("<mark>");
    break;
  }
}

void print_astack(AVM_astack_t *st) {
  printf("[");
  int size = array_size(st);
  for (int i = 1; i <= size; ++i) {
    printf(" ");
    print_value(array_elem_unsafe(st, size - i));
  }
  printf(" ]");
}

void print_rstack(AVM_astack_t *st) {
  printf("[");
  int size = array_size(st);
  for (int i = 1; i <= size; ++i) {
    printf(" ");
    print_ret_frame(array_elem_unsafe(st, size - i));
  }
  printf(" ]");
}

void print_ret_frame(AVM_ret_frame_t *f) {
  printf("(%d, %p, %zu)", f->addr, f->penv, f->offset);
}

void print_env(AVM_env_t *env) {
  printf("[");
  for (size_t i = env->cache.size; i > env->offset; i--) {
    printf(" ");
    print_value(array_elem(&env->cache, i-1));
  }
  printf(" |");
  for (size_t i = env->penv->size; i > 0; i--) {
    printf(" ");
    print_value(array_elem(env->penv, i-1));
  }
  printf(" ]");
}
