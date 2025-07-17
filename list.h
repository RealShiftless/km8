#pragma once
#include <stdint.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 32

typedef struct {
	void** values;
	uint32_t count;
	uint32_t capacity;
} List;

typedef enum {
	LIST_OK,
	LIST_ALLOC_FAILED,
	LIST_NULL_REFRENCE,
	LIST_NOT_INITIALIZED,
	LIST_OUT_OF_RANGE,
	LIST_DISPOSE_FAILED
} ListResult;

uint8_t list_init(List* list);
uint8_t list_add(List* list, void* value);
uint8_t list_remove(List* list, uint32_t index);
uint8_t list_resize(List* list, uint32_t capacity);

uint8_t list_dispose_child(List* list, uint32_t index);
uint8_t list_dispose_children(List* list);
uint8_t list_dispose(List* list);

const char* list_result_message(ListResult result);