
#ifndef LIST_H
#define LIST_H

typedef struct List {
    void *data;
    int capacity;
    int size;
} List;

List* list_create(int initial_capacity);
void  list_destroy(List **r_list);
int   list_append(List *list, void *item);
int   list_insert(List *list, int index, void *item);
void* list_get(List *list, int index);
int  list_remove(List *list, int index);

#endif // LIST_H