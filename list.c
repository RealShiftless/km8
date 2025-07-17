#include "list.h"

uint8_t list_init(List* list) {
    list->count = 0;
    list->capacity = INITIAL_CAPACITY;
    list->values = (void**)malloc(sizeof(void*) * list->capacity);

    if (list->values == NULL)
        return 1;
     return 0;
}

uint8_t list_add(List* list, void* value) {
    if (list == NULL || list->values == NULL)
        return 1;

    if (list->count + 1 >= list->capacity) {
        if (list_resize(list, list->capacity * 2))
            return LIST_ALLOC_FAILED;
    }

    list->values[list->count++] = value;
    return LIST_OK;
}

uint8_t list_remove(List* list, uint32_t index) {
    if (list == NULL)
        return LIST_NULL_REFRENCE;

    if (list->values == NULL)
        return LIST_NOT_INITIALIZED;

    if (index >= list->count)
        return LIST_OUT_OF_RANGE;

    // Shift all elements after the index down by one
    for (uint32_t i = index; i < list->count - 1; ++i) {
        list->values[i] = list->values[i + 1];
    }

    // Clear the last element
    list->values[list->count - 1] = NULL;
    list->count--;

    // Shrink the list if it's too sparse 
    if (list->count <= list->capacity / 4) {
        // Reallocate to a smaller size (half the current capacity)
        void** new_values = realloc(list->values, sizeof(void*) * list->capacity / 2);

        if (new_values != NULL) {
            list->values = new_values;
            list->capacity = list->capacity / 2;
        }
    }

    return LIST_OK;  // Success
}

uint8_t list_resize(List* list, uint32_t capacity) {
    list->values = realloc(list->values, list->capacity * sizeof(void*));  // Resize the array

    if (list->values == NULL)
        return LIST_ALLOC_FAILED;  // Error

    list->capacity = capacity;

    return LIST_OK;
}

uint8_t list_dispose_child(List* list, uint32_t index) {
    if (list == NULL) 
        return LIST_NULL_REFRENCE;

    if (list->values == NULL)
        return LIST_NOT_INITIALIZED;

    if (index >= list->count)
        return LIST_OUT_OF_RANGE;

    void* ptr = list->values[index];
    uint8_t result = list_remove(list, index);
    free(ptr);

    return result;
}

uint8_t list_dispose_children(List* list) {
    if (list == NULL)
        return LIST_NULL_REFRENCE;

    if (list->values != NULL) {
        uint32_t count = list->count;

        for (uint32_t i = 0; i < count; i++) {

            if (list_dispose_child(list, list->count - 1) != LIST_OK)
                return LIST_DISPOSE_FAILED;
        }
    }

    return LIST_OK;
}

uint8_t list_dispose(List* list) {
    uint8_t result = list_dispose_children(list);
    if (result != LIST_OK)
        return result;
    
    // Free the structure
    free(list->values);  // Free the array holding the items
    list->values = NULL;

    return LIST_OK;
}

const char* list_result_message(ListResult result) {
    switch (result) {
        case LIST_OK:               return "OK";
        case LIST_ALLOC_FAILED:     return "Allocation Failed";
        case LIST_NULL_REFRENCE:    return "Null Refrence";
        case LIST_NOT_INITIALIZED:  return "Not Initialized";
        case LIST_OUT_OF_RANGE:     return "Index Out Of Range";
        case LIST_DISPOSE_FAILED:   return "Disposal Failed";
        default:                    return "Unknown Error";
    }
}