#include "runtime.h"
#include <stdlib.h>
#include <stdio.h>

// #define DEBUG

// TODO: Add better error handling

ZAM_value_t* pop(ZAM_stack_t** stp) {
  if (stp == NULL || *stp == NULL) return NULL;

  ZAM_stack_t* st = *stp;
  ZAM_value_t* val = st->val;

#ifdef DEBUG
  printf("Popped ");
  print_value(val);
#endif

  *stp = st->next;
  free(st);
  return val;
}

_Bool push(ZAM_stack_t** stp, ZAM_value_t* val) {
  ZAM_stack_t* node = NULL;
  node = malloc(sizeof(ZAM_stack_t));

  if (!node) return false;

#ifdef DEBUG
  printf("Pushed ");
  print_value(val);
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

#ifdef DEBUG
  printf("Extended env with ");
  print_value(val);
#endif

  node->val = val;
  node->next = env;
  return node;
}

ZAM_value_t* lookup(ZAM_env_t *env, int index) {
  int i = 0;

  while (env != NULL) {
    if (i == index) {

#ifdef DEBUG
      printf("Found ");
      print_value(env->val);
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
    printf("%d\n", val->int_value);
    break;

  case ZAM_BoolVal:
    printf("%s\n", val->bool_value ? "true" : "false");
    break;

  case ZAM_ClosVal:
    printf("clos(%d)\n", val->addr);
    break;

  case ZAM_Epsilon:
    printf("<mark>\n");
    break;
  }
}
