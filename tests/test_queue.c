#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../src/utils/queue.h"

static void test_basic_operations(void) {
    Queue *q = create_queue(2);
    assert(q != NULL);

    /* newly created queue should be empty */
    assert(queue_is_empty(q));
    
    int a = 1, b = 2, c = 3;
    queue_enqueue(q, &a);
    assert(!queue_is_empty(q));

    queue_enqueue(q, &b);
    /* force a resize by enqueueing a third element */
    queue_enqueue(q, &c);

    assert(queue_dequeue(q) == &a);
    assert(queue_dequeue(q) == &b);
    assert(queue_dequeue(q) == &c);
    assert(queue_is_empty(q));

    queue_destroy(q);
}

static void test_empty_dequeue(void) {
    Queue *q = create_queue(1);
    assert(q != NULL);
    assert(queue_dequeue(q) == NULL);
    queue_destroy(q);
}

static void test_wraparound_and_growth(void) {
    Queue *q = create_queue(3);
    assert(q != NULL);

    /* enqueue more than initial capacity to trigger growth */
    int items[20];
    for (int i = 0; i < 10; i++) {
        items[i] = i;
        queue_enqueue(q, &items[i]);
    }

    /* dequeue a few items to move the front index forward */
    for (int i = 0; i < 4; i++) {
        assert(queue_dequeue(q) == &items[i]);
    }

    /* enqueue additional items to exercise wraparound logic */
    for (int i = 10; i < 15; i++) {
        items[i] = i;
        queue_enqueue(q, &items[i]);
    }

    /* drain the rest of the queue and verify order */
    for (int i = 4; i < 15; i++) {
        assert(queue_dequeue(q) == &items[i]);
    }

    assert(queue_is_empty(q));
    queue_destroy(q);
}

int main(void) {
    test_basic_operations();
    test_empty_dequeue();
    test_wraparound_and_growth();

    printf("[PASS] queue tests\n");
    return 0;
}
