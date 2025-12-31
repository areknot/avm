#pragma once

#include <stdbool.h>

struct AVM_code;
typedef struct AVM_code AVM_code_t;

typedef struct AVM_value_node AVM_value_t;
typedef struct AVM_stack_node AVM_stack_t;
typedef struct AVM_env_node AVM_env_t;

// Value type
typedef enum {
    AVM_IntVal,
    AVM_BoolVal,
    AVM_ClosVal,
    AVM_Epsilon
} AVM_value_kind;


struct AVM_value_node {
  AVM_value_kind kind;
  int int_value;
  _Bool bool_value;
  int addr;
  AVM_env_t* env;
};


struct AVM_stack_node {
  AVM_value_t* val;
  struct AVM_stack_node* next;
};

AVM_value_t* _pop(AVM_stack_t** stp, const char* name);
_Bool _push(AVM_stack_t** stp, AVM_value_t* val, const char* name);

#define pop(stp) _pop(stp, #stp)
#define push(stp, val) _push(stp, val, #stp)

struct AVM_env_node {
  AVM_value_t* val;
  struct AVM_env_node* next;
};

AVM_env_t* extend(AVM_env_t *env, AVM_value_t *val);

AVM_value_t* lookup(AVM_env_t *env, int index);

void print_value(AVM_value_t*);
