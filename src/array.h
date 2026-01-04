#pragma once

#include <stdlib.h>

#define ARRAY_MINIMAL_CAP   32
#define ARRAY_BIGGER_CAP(x) (((x) << 1) + 32)

typedef struct {
  void** data;
  size_t size;
  size_t capacity;
} array_t;

/* Allocating a fresh, empty array */
array_t* make_array(size_t capacity);

/* Deallocating a array */
void drop_array(array_t* array);

/* Retrieving the data in array->data[idx]; NULL is returned if `idx`
   is out of bound. */
void* array_elem(array_t* array, size_t idx);

size_t array_size(array_t* array);

typedef enum {
  /* Memory reservation succeeded. */
  ARRAY_RESERVE_SUCCESS,
  /* Nothing happened. */
  ARRAY_RESERVE_TRIVIAL,
  /* Memory reservation failed. */
  ARRAY_RESERVE_FAILURE
} array_reserve_code;
/* Reserving spaces. Only works when the new capacity is greater than
   the original one. An `array_reserve_code` is returned. */
array_reserve_code reserve_array(array_t* array, size_t capacity);

/* Popping all elements from `array` */
void clean_array(array_t* array);

/* Pushing a new element into `array`. The number of actually pushed
   element is returned. */
int push_array(array_t* array, void* data);

/* Popping an element from `array`. the number of actually popped
   element is returned. */
int pop_array(array_t* array);
