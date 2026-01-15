
#include "memory.h"
#include "debug.h"
#include "runtime.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

#define GC_HEAP_GROW_FACTOR 8

void *reallocate(struct AVM_VM *vm, void *ptr, size_t old_size, size_t new_size) {
  vm->allocated_bytes += new_size - old_size;

  if (new_size == 0) {
    free(ptr);
    return NULL;
  }

  void *tmp = realloc(ptr, new_size);

  if (tmp == NULL)
    error("reallocate: Couldn't reallocate a block.");
  return tmp;
}

void *allocate_object(struct AVM_VM *vm, size_t size, AVM_object_kind kind) {

#ifdef DEBUG_GC_TEST
  run_gc(vm);
#endif

  if (vm->allocated_bytes > vm->next_gc)
    run_gc(vm);

  size_t new_size = sizeof(AVM_object_t) + size;

  AVM_object_t *header = reallocate(vm, NULL, 0, new_size);
  header->kind = kind;
  header->next = vm->objs;
  header->is_marked = false;
  vm->objs = header;

#if DEBUG_GC_LOG_LEVEL >= 2
  printf("%zu bytes allocated at %p for type %d\n", new_size, (void*)header, kind);
#endif
  return (void *)(header + 1); // return the address right after the header.

}

AVM_value_t* new_int(struct AVM_VM *vm, int i) {
  AVM_value_t *res = allocate_object(vm, sizeof(AVM_value_t), AVM_ObjValue);
  res->kind = AVM_IntVal;
  res->int_value = i;
  return res;
}

AVM_value_t *new_bool(struct AVM_VM *vm, _Bool b) {
  AVM_value_t *res = allocate_object(vm, sizeof(AVM_value_t), AVM_ObjValue);
  res->kind = AVM_BoolVal;
  res->bool_value = b;
  return res;
}

AVM_value_t *new_clos(struct AVM_VM *vm, int l, array_t *penv) {
  AVM_value_t *res = allocate_object(vm, sizeof(AVM_value_t), AVM_ObjValue);
  res->kind = AVM_ClosVal;
  res->clos_value.addr = l;
  res->clos_value.penv = penv;
  return res;
}

array_t *new_penv(struct AVM_VM *vm) {
  array_t *res = allocate_object(vm, sizeof(array_t), AVM_ObjPEnv);
  init_array(res, ARRAY_MINIMAL_CAP);
  return res;
}


/* GC */

void free_object(struct AVM_VM *vm, AVM_object_t* header) {
#if DEBUG_GC_LOG_LEVEL >= 2
  printf("free_object: %p\n  contents: ", (void*)header);
  switch (header->kind) {
  case AVM_ObjValue:
    print_value((AVM_value_t*)(header + 1));
    break;
  case AVM_ObjPEnv:
    print_penv((array_t*)(header + 1));
    break;
  }
  printf("\n");
#endif

  if (header->kind == AVM_ObjPEnv) {
    array_t *penv = (array_t*)(header + 1);
    free(penv->data);
  }

  reallocate(vm, header,
             sizeof(AVM_object_t) +
               (header->kind == AVM_ObjValue ? sizeof(AVM_value_t) : sizeof(array_t)),
             0);

  return;
}

static void mark_penv(struct AVM_VM *vm, array_t *penv);

static void mark_value(struct AVM_VM *vm, AVM_value_t *val) {
  if (val->kind == AVM_Epsilon)
    return;

  AVM_object_t *header = (AVM_object_t*)val - 1;

  if (header->is_marked)
    return;

#if DEBUG_GC_LOG_LEVEL >= 2
  printf("mark_value: %p\n", (void*)header);
#endif

  header->is_marked = true;

  if (val->kind == AVM_ClosVal)
    mark_penv(vm, val->clos_value.penv);
}

static void mark_penv(struct AVM_VM *vm, array_t *penv) {
  AVM_object_t *header = (AVM_object_t*)penv - 1;

  if (header->is_marked)
    return;

  header->is_marked = true;

#if DEBUG_GC_LOG_LEVEL >= 2
  printf("mark_penv: %p\n", (void*)header);
#endif

  for (size_t i = 0; i < array_size(penv); ++i) {
    mark_value(vm, (AVM_value_t*)array_elem_unsafe(penv, i));
  }
}


static void mark(struct AVM_VM *vm) {
  size_t i;
  // Mark vm->astack
  for (i = 0; i < array_size(vm->astack); ++i) {
    mark_value(vm, (AVM_value_t*)array_elem_unsafe(vm->astack, i));
  }
  // mark vm->rstack
  for (i = 0; i < array_size(vm->rstack); ++i) {
    AVM_ret_frame_t *f = array_elem_unsafe(vm->rstack, i);
    mark_penv(vm, f->penv);
  }
  // mark vm->env
  for (i = 0; i < array_size(vm->env->cache); ++i) {
    mark_value(vm, (AVM_value_t*)array_elem_unsafe(vm->env->cache, i));
  }
  mark_penv(vm, vm->env->penv);
}

static void sweep(struct AVM_VM *vm) {
  AVM_object_t *prev, *cur;
  prev = NULL;
  cur = vm->objs;

  while (cur != NULL) {
    if (cur->is_marked) {
      cur->is_marked = false;
      prev = cur;
      cur = cur->next;
      continue;
    }

    AVM_object_t *tmp = cur;
    cur = cur->next;

    free_object(vm, tmp);

    if (prev != NULL) {
      prev->next = cur;
    } else {
      vm->objs = cur;
    }
  }
}

void run_gc(struct AVM_VM *vm) {
  if (vm->env == NULL)
    return;

#if DEBUG_GC_LOG_LEVEL >= 1
  printf("-- gc begin\n");
  size_t before = vm->allocated_bytes;
#endif

  mark(vm);
  sweep(vm);

  size_t next_gc_candidate = vm->allocated_bytes * GC_HEAP_GROW_FACTOR;
  if (next_gc_candidate < MIN_HEAP_SIZE)
    next_gc_candidate = MIN_HEAP_SIZE;
  else if (next_gc_candidate > MAX_HEAP_SIZE)
    next_gc_candidate = MAX_HEAP_SIZE;

  vm->next_gc = next_gc_candidate;

#if DEBUG_GC_LOG_LEVEL >= 1
  printf("-- gc end\n");
  printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
         before - vm->allocated_bytes, before, vm->allocated_bytes,
         vm->next_gc);
#endif
}
