#include "list.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct list {
  size_t size;
  size_t capacity;
  free_func_t freer;
  void **data;
} list_t;

list_t *list_init(size_t initial_size, free_func_t freer) {
    list_t *list = malloc(sizeof(list_t));
    assert(list != NULL);
    list->data = malloc(sizeof(void *) * initial_size);
    assert(list->data != NULL);

    list->size = 0;

    list->freer = freer;

    list->capacity = initial_size;

    return list;
}

void list_free(list_t *list) {
    if (list->freer != NULL) {
        for (size_t i = 0; i < list->size; i++) {
            list->freer((list->data[i]));
        }
    }
    free(list->data);
    free(list);
}

void **list_get_data(list_t *list) {
    return list->data;
}

size_t list_size(list_t *list) {
    return list->size;
}

void *list_get(list_t *list, size_t index) {
    assert(index < list->size);
    return list->data[index];
}

void list_resize(list_t *list) {
    size_t capacity = list->capacity;
    void **new_data = malloc(((2 * capacity) + 1) * sizeof(void*));
    for (size_t i = 0; i < capacity; i++) {
        new_data[i] = list->data[i];
    }
    list->capacity = (2 * capacity) + 1;
    free(list->data);
    list->data = new_data;
}

void list_add(list_t *list, void *value) {
    assert(value != NULL);
    if (list->size >= list->capacity) {
        list_resize(list);
    }
    list->data[list->size] = value;
    list->size++;
}

void *list_remove(list_t *list, size_t index) {
    assert(index < list->size);
    void *removed = list_get(list, index);
    list->size = list->size - 1;
    void *temp;
    for (size_t i = index; i < list->size; i++){
        temp = list->data[i];
        list->data[i] = list->data[i+1];
        list->data[i+1] = temp;
    }
    return removed;
}

void list_update(list_t *list, size_t index, void *value) {
  list->data[index] = value;
}
