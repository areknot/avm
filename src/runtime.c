#include "runtime.h"
#include "debug.h"
#include <stdlib.h>
#include <stdio.h>

// TODO: Add better error handling
void print_stack(ZAM_stack_t *st);
void print_env(ZAM_env_t *env);

ZAM_value_t* _pop(ZAM_stack_t** stp, const char* name) {
  (void)name;
  if (stp == NULL || *stp == NULL) return NULL;

  ZAM_stack_t* st = *stp;
  ZAM_value_t* val = st->val;

#ifdef DEBUG_TRACE_EXECUTION
  printf("Popped from %s:\n  ", name);
  print_value(val);
  printf(" <- ");
  print_stack(st->next);
  printf("\n");
#endif

  *stp = st->next;
  free(st);
  return val;
}

_Bool _push(ZAM_stack_t** stp, ZAM_value_t* val, const char* name) {
  (void)name;
  ZAM_stack_t* node = NULL;
  node = malloc(sizeof(ZAM_stack_t));

  if (!node) return false;

#ifdef DEBUG_TRACE_EXECUTION
  printf("Pushed to %s:\n  ", name);
  print_value(val);
  printf(" -> ");
  print_stack(*stp);
  printf("\n");
#endif

  node->val = val;
  node->next = *stp;
  *stp = node;
  return true;
}

ZAM_env_t* extend(ZAM_env_t *env, ZAM_value_t *val) {
  ZAM_env_t *node = NULL;
  node = malloc(sizeof(ZAM_env_t));

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

ZAM_value_t* lookup(ZAM_env_t *env, int index) {
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

void print_value(ZAM_value_t *val) {
  switch (val->kind) {
  case ZAM_IntVal:
    printf("%d", val->int_value);
    break;

  case ZAM_BoolVal:
    printf("%s", val->bool_value ? "true" : "false");
    break;

  case ZAM_ClosVal:
    printf("<clos(%d)>", val->addr);
    break;

  case ZAM_Epsilon:
    printf("<mark>");
    break;
  }
}

void print_stack(ZAM_stack_t *st) {
  printf("[");
  for (ZAM_stack_t *p = st; p != NULL; p = p->next) {
    printf(" ");
    print_value(p->val);
  }
  printf(" ]");
}

void print_env(ZAM_env_t *env) {
  printf("[");
  for (ZAM_env_t *p = env; p != NULL; p = p->next) {
    printf(" ");
    print_value(p->val);
  }
  printf(" ]");
}
