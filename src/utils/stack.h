// utils/stack.h
#ifndef STACK_H
#define STACK_H

typedef struct {
    int capacity;
    int top;
    void **items;
} Stack;

Stack *stack_create(int capacity);
void  stack_destroy(Stack *s);
int   stack_is_empty(Stack *s);
void  stack_push(Stack *s, void *item);
void *stack_pop(Stack *s);
void* stack_peek(Stack *s);
void  stack_free(Stack *s);

#endif // STACK_H
