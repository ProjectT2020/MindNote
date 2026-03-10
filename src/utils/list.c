#include <stdlib.h>
#include <string.h>

#include "list.h"

List* list_create(int initial_capacity) {
    List *list = (List *)malloc(sizeof(List));
    list->data = malloc(initial_capacity * sizeof(void *));
    list->capacity = initial_capacity;
    list->size = 0;
    return list;
}

void list_destroy(List **list) {
    List *listp = *list;
    if (listp->data) {
        free(listp->data);
        listp->data = NULL;
    }
    if (listp) {
        free(listp);
        *list = NULL;
    }
}

int list_append(List *list, void *item) {
    if (list->size >= list->capacity) {
        int new_capacity = list->capacity * 2;
        list->data = realloc(list->data, new_capacity * sizeof(void *));
        list->capacity = new_capacity;
    }
    ((void **)list->data)[list->size++] = item;
    return 0;
}

int list_insert(List *list, int index, void *item) {
    if (index < 0 || index > list->size) {
        return -1;
    }
    if (list->size >= list->capacity) {
        int new_capacity = list->capacity * 2;
        list->data = realloc(list->data, new_capacity * sizeof(void *));
        list->capacity = new_capacity;
    }
    for (int i = list->size; i > index; i--) {
        ((void **)list->data)[i] = ((void **)list->data)[i - 1];
    }
    ((void **)list->data)[index] = item;
    list->size++;
    return 0;
}

void* list_get(List *list, int index) {
    if (index < 0 || index >= list->size) {
        return NULL;
    }
    return ((void **)list->data)[index];
}

int list_remove(List *list, int index) {
    if (index < 0 || index >= list->size) {
        return -1;
    }
    for (int i = index; i < list->size - 1; i++) {
        ((void **)list->data)[i] = ((void **)list->data)[i + 1];
    }
    list->size--;
    return 0;
}