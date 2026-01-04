#include "runtime.h"
#include "debug.h"
#include <stdlib.h>
#include <stdio.h>

// TODO: Add better error handling
void print_stack(AVM_stack_t *st);
void print_env(AVM_env_t *env);

AVM_stack_t* init_stack() {
  return make_array(ARRAY_MINIMAL_CAP);
}

void drop_stack(AVM_stack_t* stack) {
  drop_array(stack);
}

AVM_value_t* _pop(AVM_stack_t* stp, const char* name) {
  (void)name;
  AVM_value_t* val = array_last(stp);
  int count = pop_array(stp);
  if (count == 0) return NULL;

#ifdef DEBUG_TRACE_EXECUTION
  printf("Popped from %s:\n  ", name);
  print_value(val);
  printf(" <- ");
  print_stack(stp);
  printf("\n");
#endif
  
  return val;
}

_Bool _push(AVM_stack_t* stp, AVM_value_t* val, const char* name) {
  (void)name;

#ifdef DEBUG_TRACE_EXECUTION
  printf("Pushed to %s:\n  ", name);
  print_value(val);
  printf(" -> ");
  print_stack(stp);
  printf("\n");
#endif
  int count = push_array(stp, val);
  if (count == 0) return false;

  return true;
}

AVM_env_t* extend(AVM_env_t *env, AVM_value_t *val) {
  AVM_env_t *node = NULL;
  node = malloc(sizeof(AVM_env_t));

  if (!node) return NULL; // Returns NULL if malloc failed.

#ifdef DEBUG_TRACE_EXECUTION
  printf("Extended env:\n  ");
  print_value(val);
  printf(" -> ");
  print_env(env);
  printf("\n");
#endif

  node->val = val;
  node->next = env;
  return node;
}

AVM_value_t* lookup(AVM_env_t *env, int index) {
  int i = 0;

  while (env != NULL) {
    if (i == index) {

#ifdef DEBUG_TRACE_EXECUTION
      printf("Found ");
      print_value(env->val);
      printf(" in ");
      print_env(env);
      printf("\n");
#endif

      return env->val;
    }
    i++;
    env = env->next;
  }

  return NULL;
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
    printf("<clos(%d)>", val->addr);
    break;

  case AVM_Epsilon:
    printf("<mark>");
    break;
  }
}

void print_stack(AVM_stack_t *st) {
  printf("[");
  int size = array_size(st);
  for (int i = 1; i <= size; ++i) {
    printf(" ");
    print_value(array_elem_unsafe(st, size - i));
  }
  printf(" ]");
}

void print_env(AVM_env_t *env) {
  printf("[");
  for (AVM_env_t *p = env; p != NULL; p = p->next) {
    printf(" ");
    print_value(p->val);
  }
  printf(" ]");
}
