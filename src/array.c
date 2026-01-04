
#include "array.h"

array_t* make_array(size_t capacity) {
  array_t* array  = malloc(sizeof(array));
  array->data     = malloc(sizeof(void*) * capacity);
  array->size     = 0;
  array->capacity = capacity;
  return array;
}

void drop_array(array_t* array) {
  free(array->data);
  free(array);
}

void* array_elem(array_t* array, size_t idx) {
  return (idx < array->size) ? array->data[idx] : NULL;
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

int pop_array(array_t* array) {
  if (array->size == 0) return 0;
  --array->size;
  return 1;
}
