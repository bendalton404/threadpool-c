#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "task_queue.h"

// dummy functions to match signature in task_t
// don't need true functionality as the task_queue only passes around pointers
static void dummy_func(void *args) {;}
static void dummy_cleanup(void *args) {;}

static task_t *dummy_task(int check_int)
{
    task_t *t = malloc(sizeof(task_t));
    assert(t != NULL);

    t->func = dummy_func;
    t->cleanup = dummy_cleanup;

    int *arg = malloc(sizeof(int));
    assert(arg != NULL);
    *arg = check_int;
    t->args = (void *) arg;

    return t;
}

static void free_task(task_t *t)
{
    free(t->args);
    free(t);
}

static void test_init_destroy(void)
{
    printf("test: init_destroy...\n");

    task_queue_t *q = task_queue_init(4);
    assert(q != NULL);
    assert(is_empty(q));
    assert(!is_full(q));

    task_queue_destroy(&q);
    assert(q == NULL); // destroy nulls callers poiner

    printf("status: passed\n");
}

static void test_destroy_null_safety(void)
{
    printf("test: destroy_null_safety...\n");

    // must null the queue pointer
    task_queue_t *q = NULL;
    task_queue_destroy(&q);
    assert(q == NULL);

    // must not crash if the queue pointer is already null
    task_queue_destroy(&q);

    printf("status: passed\n");
}

static void test_enqueue_dequeue_order(void)
{
    printf("test: enqueue_dequeue_order...\n");

    task_queue_t *q = task_queue_init(3);
    assert(q != NULL);

    task_t *t1 = dummy_task(1);
    task_t *t2 = dummy_task(2);
    task_t *t3 = dummy_task(3);

    assert(enqueue(q, t1) == 1);
    assert(enqueue(q, t2) == 1);
    assert(enqueue(q, t3) == 1);
    assert(is_full(q));

    // queue is full, enqueue must fail
    task_t *t4 = dummy_task(4);
    assert(enqueue(q, t4) == 0);

    task_t *out;
    assert(dequeue(q, &out) == 1);
    assert(*(int *)out->args == 1);
    free_task(out);

    assert(dequeue(q, &out) == 1);
    assert(*(int *)out->args == 2);
    free_task(out);

    assert(dequeue(q, &out) == 1);
    assert(*(int *)out->args == 3);
    free_task(out);

    assert(is_empty(q));

    // queue is empty, dequeue must fail
    assert(dequeue(q, &out) == 0);

    free_task(t4); 
    task_queue_destroy(&q);

    printf("status: passed\n");
}

static void test_circularity(void)
{
    printf("test: circularity...\n");

    task_queue_t *q = task_queue_init(3);
    assert(q != NULL);

    task_t *a = dummy_task(10);
    task_t *b = dummy_task(20);
    task_t *c = dummy_task(30);
    task_t *d = dummy_task(40);
    task_t *e = dummy_task(50);

    // test whether the circularity of the queue works
    assert(enqueue(q, a) == 1);
    assert(enqueue(q, b) == 1);
    assert(enqueue(q, c) == 1);

    task_t *out;
    assert(dequeue(q, &out) == 1);
    assert(*(int *)out->args == 10);
    free_task(out);

    assert(dequeue(q, &out) == 1);
    assert(*(int *)out->args == 20);
    free_task(out);

    // should be enqueuing from the start again
    assert(enqueue(q, d) == 1);
    assert(enqueue(q, e) == 1);
    assert(is_full(q));

    assert(dequeue(q, &out) == 1);
    assert(*(int *)out->args == 30);
    free_task(out);

    assert(dequeue(q, &out) == 1);
    assert(*(int *)out->args == 40);
    free_task(out);

    assert(dequeue(q, &out) == 1);
    assert(*(int *)out->args == 50);
    free_task(out);

    assert(is_empty(q));

    task_queue_destroy(&q);

    printf("status: passed\n");
}


int main(void)
{
    test_init_destroy();
    test_destroy_null_safety();
    test_enqueue_dequeue_order();
    test_circularity();
    printf("\nAll task_queue tests passed.\n");
    return 0;
}