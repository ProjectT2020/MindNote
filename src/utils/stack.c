// utils/stack.c
#include <stdlib.h> // NULL, malloc, free

#include "stack.h"

Stack *stack_create(int capacity){
    Stack *s = (Stack *)malloc(sizeof(Stack));
    s->capacity = capacity;
    s->top = -1;
    s->items = (void **)malloc(sizeof(void *) * capacity);
    return s;
}

int stack_is_empty(Stack *s){
    return s->top == -1;
}

void stack_push(Stack *s, void *item){
    if(s->top == s->capacity - 1){
        s->capacity *= 2;
        s->items = (void **)realloc(s->items, sizeof(void *) * s->capacity);
    }
    s->items[++s->top] = item;
}

void* stack_pop(Stack *s){
    if(stack_is_empty(s)){
        return NULL;
    }
    return s->items[s->top--];
}

void* stack_peek(Stack *s){
    if(stack_is_empty(s)){
        return NULL;
    }
    return s->items[s->top];
}

void stack_destroy(Stack *s){
    free(s->items);
    free(s);
}