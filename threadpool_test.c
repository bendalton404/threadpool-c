#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "task_queue.h"
#include "threadpool.h"

static void interleaving_func(void *args)
{
    int *thread_num = (int *) args;
    printf("Thread %d starting...\n", *thread_num);
    sleep(1);
    printf("...thread %d ended.\n", *thread_num);
}

static void interleaving_cleanup(void *args) {
    int *thread_num = (int *) args;
    printf("Thread cleanup: %d\n", *thread_num);
    free(thread_num);
}

static task_t *interleaving_task(int thread_num) {
    task_t *t = malloc(sizeof(task_t));
    t->func = interleaving_func;
    t->cleanup = interleaving_cleanup;
    int *args = malloc(sizeof(int));
    *args = thread_num;
    t->args = (void *) args;
     return t;
}

static void test_init_destroy(void)
{   
    printf("test: init_destroy...\n");

    threadpool_t *pool = threadpool_init(10, 3);
    assert(pool != NULL);
    
    threadpool_destroy(&pool);
    assert (pool == NULL);

    printf("status: passed\n\n");
}

static void test_destroy_null_safety(void)
{
    printf("test: destroy_null_safety...\n");

    threadpool_t *pool = NULL;
    threadpool_destroy(&pool);
    assert(pool == NULL);
    threadpool_destroy(&pool);

    printf("status: passed\n\n");
}

static void test_start_end(void)
{
    printf("test: start_end...\n");

    threadpool_t *pool = threadpool_init(10, 3);
    assert(pool != NULL);

    assert(threadpool_start(NULL) == 0);
    assert(threadpool_end(NULL) == 0);
    assert(threadpool_start(pool) == 1);
    assert(threadpool_start(pool) == 0);
    assert(threadpool_end(pool) == 1);
    assert(threadpool_end(pool) == 0);
    
    // don't want to destroy pool, only stop the threads running
    assert(pool != NULL);
    assert(pool->threads_count == 0);
    assert(pool->threads_active == 0);
    assert(pool->threads_exit == 0);

    // make sure the pool can be restarted (and re-ended)
    assert(threadpool_start(pool) == 1);
    assert(threadpool_end(pool) == 1);

    threadpool_destroy(&pool);
    assert(pool == NULL);

    printf("status: passed\n\n");
}

static void test_add(void)
{
    printf("test: add...\n");

    threadpool_t *pool = threadpool_init(10, 3);
    task_t *task = interleaving_task(999);
    threadpool_start(pool);
    assert(threadpool_add(NULL, &task) == 0);
    assert(threadpool_add(pool, NULL) == 0);
    assert(threadpool_add(pool, &task) == 1);
    assert(task == NULL);

    threadpool_end(pool);

    printf("status: passed\n\n");
}

static void test_empty_wait(void)
{
    printf("test: empty_wait...\n");

    threadpool_t *pool = threadpool_init(10, 3);
    threadpool_wait(pool);
    threadpool_wait(NULL);

    printf("status: passed\n\n");
}

static void test_interleaving(void)
{
    printf("test: interleaving...\n");

    threadpool_t *pool = threadpool_init(10, 3);
    task_t *t1 = interleaving_task(1);
    task_t *t2 = interleaving_task(2);
    threadpool_start(pool);
    threadpool_add(pool, &t1);
    threadpool_add(pool, &t2);
    threadpool_end(pool);
    threadpool_destroy(&pool);

    printf("status: passed\n\n");
}

int main(void)
{
    test_init_destroy();
    test_destroy_null_safety();
    test_start_end();
    test_add();
    test_empty_wait();
    test_interleaving();
    printf("\nAll task_queue tests passed.\n");
    return 0;
}