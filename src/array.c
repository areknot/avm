
#include "array.h"
#include <string.h>
#define MAX(x, y) ((x) < (y) ? y : x)

array_t* make_array(size_t capacity) {
  array_t* array  = malloc(sizeof(array_t));
  array->data     = malloc(sizeof(void*) * capacity);
  array->size     = 0;
  array->capacity = capacity;
  return array;
}

void init_array(array_t* array, size_t capacity) {
  array->data     = malloc(sizeof(void*) * capacity);
  array->size     = 0;
  array->capacity = capacity;
}

void drop_array(array_t* array) {
  free(array->data);
  free(array);
}

void* array_elem(array_t* array, size_t idx) {
  return (idx < array->size) ? array->data[idx] : NULL;
}

void* array_last(array_t* array) {
  return array->size == 0 ? NULL : array->data[array->size - 1];
}

void* array_first(array_t* array) {
  return array_elem(array, 0);
}

size_t array_size(array_t* array) {
  return array->size;
}

array_reserve_code reserve_array(array_t* array, size_t capacity) {
  if (array->capacity >= capacity) return ARRAY_RESERVE_TRIVIAL;
  void** data_ = realloc(array->data, capacity * sizeof(void*));
  if (data_ == NULL) return ARRAY_RESERVE_FAILURE;
  array->data     = data_;
  array->capacity = capacity;
  return ARRAY_RESERVE_SUCCESS;
}

void clean_array(array_t* array) { array->size = 0; }

int push_array(array_t* array, void* data) {
  if (array->size >= array->capacity) {
    int code = reserve_array(array, ARRAY_BIGGER_CAP(array->capacity));
    if (code == ARRAY_RESERVE_FAILURE) return 0;
  }
  array->data[array->size] = data;
  ++array->size;
  return 1;
}

int push_array_offset(array_t* dst, array_t* src, size_t offset) {
  if (offset >= src->size) return 0;
  size_t sum = dst->size + (src->size - offset);
  if (sum >= dst->capacity) {
    int code = reserve_array(dst, MAX(sum, ARRAY_BIGGER_CAP(dst->capacity)));
    if (code == ARRAY_RESERVE_FAILURE) return -1;
  }
  for (size_t i = 0; i < src->size - offset; i++) dst->data[dst->size + i] = src->data[i + offset];
  dst->size = sum;
  return src->size;
}

int pop_array_n(array_t* array, size_t n) {
  if (array->size < n) return 0;
  array->size = array->size - n;
  return n;
}

array_t* copy(array_t* array) {
  array_t* tmp  = malloc(sizeof(array_t));
  tmp->data     = malloc(sizeof(void*) * array->capacity);
  tmp->size     = array->size;
  tmp->capacity = array->capacity;
  memcpy(tmp->data, array->data, sizeof(void*) * array->size);
  return tmp;
}
