#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "array.h"

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

typedef uint64_t AVM_value_t;

#define QNAN        ((uint64_t)0x7ffc000000000000)

#define TAG_MASK    ((uint64_t)0xffff000000000000)
#define PTR_MASK    ((uint64_t)0x0000ffffffffffff)

#define TAG_OBJ     ((uint64_t)0x7ffc000000000000)
#define TAG_INT     ((uint64_t)0x7ffd000000000000)
#define TAG_MISC    ((uint64_t)0x7ffe000000000000)

#define TAG_EPSILON 1
#define TAG_FALSE   2
#define TAG_TRUE    3

#define VAL_EPSILON (TAG_MISC | TAG_EPSILON)
#define VAL_FALSE   (TAG_MISC | TAG_FALSE)
#define VAL_TRUE    (TAG_MISC | TAG_TRUE)

_Static_assert(sizeof(void*) == sizeof(uint64_t), "AVM requires a 64-bit architecture.");

static inline _Bool is_obj(AVM_value_t v) {
  return ((v & TAG_MASK) == TAG_OBJ);
}

static inline _Bool is_int(AVM_value_t v) {
  return ((v & TAG_MASK) == TAG_INT);
}

static inline _Bool is_bool(AVM_value_t v) {
  return ((v | 1) == VAL_TRUE);
}

static inline _Bool is_epsilon(AVM_value_t v) {
  return (v == VAL_EPSILON);
}

static inline AVM_value_t mk_int(int v) {
  return TAG_INT | (uint32_t)v;
}

static inline AVM_value_t mk_obj(void *p) {
  return TAG_OBJ | ((uint64_t)(uintptr_t)p & PTR_MASK);
}

static inline AVM_value_t mk_bool(_Bool b) {
  return b ? VAL_TRUE : VAL_FALSE;
}

static inline int as_int(AVM_value_t v) {
  return (int)v;
}

struct AVM_object;

static inline struct AVM_object *as_obj(AVM_value_t v) {
  return (void*)(v & 0x0000ffffffffffff);
}

static inline _Bool as_bool(AVM_value_t v) {
  return v == VAL_TRUE;
}

typedef struct {
  int addr;
  array_t *penv;
} AVM_clos_t;

AVM_astack_t* init_astack();
void drop_astack(AVM_astack_t* stack);
AVM_value_t apop(AVM_astack_t* stp);
_Bool apush(AVM_astack_t* stp, AVM_value_t val);

AVM_rstack_t* init_rstack();
void drop_rstack(AVM_rstack_t* stack);
AVM_ret_frame_t* rpop(AVM_rstack_t* stp);
_Bool rpush(AVM_rstack_t* stp, AVM_ret_frame_t *frame);

struct AVM_VM;

AVM_env_t* init_env(struct AVM_VM *vm);
AVM_env_t* extend(AVM_env_t *env, AVM_value_t val);
AVM_value_t lookup(AVM_env_t *env, size_t index);
void perpetuate(struct AVM_VM *vm, AVM_env_t *env);

// Removes the head of `env`.
void remove_head(struct AVM_VM *vm, AVM_env_t *env);

void print_value(AVM_value_t val);
void print_clos(AVM_clos_t *clos);
void print_astack(AVM_astack_t *st);
void print_rstack(AVM_astack_t *st);
void print_ret_frame(AVM_ret_frame_t *f);
void print_env(AVM_env_t *env);
void print_penv(array_t *env);
