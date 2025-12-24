#include <stdbool.h>

struct ZAM_code;
typedef struct ZAM_code ZAM_code_t;

typedef struct ZAM_value_node ZAM_value_t;
typedef struct ZAM_stack_node ZAM_stack_t;
typedef struct ZAM_env_node ZAM_env_t;

// Value type
typedef enum {
    ZAM_IntVal,
    ZAM_BoolVal,
    ZAM_ClosVal,
    ZAM_Epsilon
} ZAM_value_kind;


struct ZAM_value_node {
  ZAM_value_kind kind;
  int int_value;
  _Bool bool_value;
  ZAM_code_t* closure_code;
  ZAM_env_t* closure_env;
};


struct ZAM_stack_node {
  ZAM_value_t* val;
  struct ZAM_stack_node* next;
};

ZAM_value_t* pop(ZAM_stack_t** stp);

_Bool push(ZAM_stack_t** stp, ZAM_value_t* val);


struct ZAM_env_node {
  ZAM_value_t* val;
  struct ZAM_env_node* next;
};

ZAM_env_t* extend(ZAM_env_t *env, ZAM_value_t *val);

ZAM_value_t* lookup(ZAM_env_t *env, int index);
