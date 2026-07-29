#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include "task_queue.h"

/**
 * @brief Allocates memory for a threadpool and initializes its values.
 */
typedef struct
{
    task_queue_t *task_queue;
    pthread_t *threads_arr;
    int threads_max;
    int threads_count;
    int threads_active;
    int threads_exit;
    pthread_mutex_t mutex_pool;
    pthread_cond_t cond_queue_not_empty;
    pthread_cond_t cond_queue_not_full;
    pthread_cond_t cond_check_idle;
} threadpool_t;

/**
 * @brief Loops the thread to monitor the task queue for incoming tasks.
 * @param pool The threadpool the thread is a part of.
 */
void *thread_mainloop(
    void *pool);

/**
 * @brief Initializes a threadpool_t struct.
 * @param max_tasks The maximum number of tasks which can be in the queue at once.
 * @param max_threads The number of threads in the threadpool.
 */
threadpool_t *threadpool_init(
    int max_tasks,
    int max_threads);

/**
 * @brief Handles deallocation of threadpool_t struct.
 * Sets the caller's pointer to NULL.
 */
void threadpool_destroy(
    threadpool_t **pool);

/**
 * @brief Starts running the threads in the pool.
 * @returns Truthy value of whether the pool started correctly.
 */
int threadpool_start(
    threadpool_t *pool);

/**
 * @brief Adds a task to the threadpool.
 * The caller allocates the task_t. The ownership is transferred to the threadpool and the caller's pointer is nulled.
 * The thread is responsible for freeing the task and its args
 * @returns Truthy value of whether the task was added successfully.
 */
int threadpool_add(
    threadpool_t *pool,
    task_t **t);

/**
 * @brief Sleeps calling code until the threadpool has no tasks in the queue and no active threads.
 * @returns Truthy value of whether the pool is now idle (if pool is NULL then wait will fail).
 */
int threadpool_wait(
    threadpool_t *pool);

/**
 * @brief Terminates threads once all tasks in the queue are complete.
 * Nulls the caller's threadpool pointer.
 * @returns Truthy value of whether all threads joined correctly.
 */
int threadpool_end(
    threadpool_t *pool);

#endif