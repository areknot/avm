#pragma once

#include <stdbool.h>
#include "array.h"

typedef struct AVM_value_node AVM_value_t;
typedef array_t AVM_astack_t;
typedef array_t AVM_rstack_t;

typedef struct {
  array_t *penv;
  array_t *cache;
  size_t offset;
} AVM_env_t;

typedef struct {
  int addr;
  array_t *penv;
  size_t offset;
} AVM_ret_frame_t;

// Value type
typedef enum {
    AVM_IntVal,
    AVM_BoolVal,
    AVM_ClosVal,
    AVM_Epsilon
} AVM_value_kind;

struct AVM_value_node {
  AVM_value_kind kind;
  union {
    int int_value;
    _Bool bool_value;
    struct {
      int addr;
      array_t *penv;
    } clos_value;
  };
};


void print_value(AVM_value_t*);

AVM_astack_t* init_astack();
void drop_astack(AVM_astack_t* stack);
AVM_value_t* apop(AVM_astack_t* stp);
_Bool apush(AVM_astack_t* stp, AVM_value_t* val);

AVM_rstack_t* init_rstack();
void drop_rstack(AVM_rstack_t* stack);
AVM_ret_frame_t* rpop(AVM_rstack_t* stp);
_Bool rpush(AVM_rstack_t* stp, AVM_ret_frame_t *frame);

struct AVM_VM;

AVM_env_t* init_env();
AVM_env_t* extend(AVM_env_t *env, AVM_value_t *val);
AVM_value_t* lookup(AVM_env_t *env, size_t index);
void perpetuate(AVM_env_t *env);

// Removes the head of `env`.
void remove_head(AVM_env_t *env);
