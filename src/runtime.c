#include "runtime.h"
#include <stdlib.h>

// TODO: Add error handling

ZAM_value_t* pop(ZAM_stack_t** stp) {
  if (stp == NULL || *stp == NULL) return NULL;

  ZAM_stack_t* st = *stp;
  ZAM_value_t* val = st->val;

  *stp = st->next;
  free(st);
  return val;
}

_Bool push(ZAM_stack_t** stp, ZAM_value_t* val) {
  ZAM_stack_t* node = NULL;
  node = malloc(sizeof(ZAM_stack_t));

  if (!node) return false;

  node->val = val;
  node->next = *stp;
  *stp = node;
  return true;
}

ZAM_env_t* extend(ZAM_env_t *env, ZAM_value_t *val) {
  ZAM_env_t *node = NULL;
  node = malloc(sizeof(ZAM_env_t));

  if (!node) return NULL; // Returns NULL if malloc failed.

  node->val = val;
  node->next = env;
  return node;
}

ZAM_value_t* lookup(ZAM_env_t *env, int index) {
  int i = 0;

  while (env != NULL) {
    if (i == index) return env->val;
    i++;
    env = env->next;
  }

  return NULL;
}
