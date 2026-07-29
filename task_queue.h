#ifndef QUEUE_H
#define QUEUE_H

/**
 * @brief Stores a function, arguments pair as a task_t for execution by a thread.
 */
typedef struct
{
    void (*func)(void *args);
    void *args;
    void (*cleanup) (void *args);
} task_t;

/**
 * @brief Circular queue which holds task_t structs.
 */
typedef struct
{
    task_t **arr;
    int max_size;
    int size;
    int head;
    int tail;
} task_queue_t;

/**
 * @brief Creates a task_queue_t struct and returns a pointer to it.
 * @param max_size Maximum size of the queue.
 * @returns The queue, unless calloc fails, in which case NULL.
 */
task_queue_t *task_queue_init(
    int max_size);

/**
 * @brief Cleanup procedure for freeing a task_queue_t struct.
 * @param q A double pointer to the queue struct.
 */
void task_queue_destroy(
    task_queue_t **q);

/**
 * @brief Enqueues the a copy of the pointer to the queue.
 * @param q The queue being enqueued to.
 * @param t The task_t being enqueued.
 * @returns `1` for successful enqueue, `0` if queue is full.
 */
int enqueue(
    task_queue_t *q,
    task_t *t);

/**
 * @brief Dequeues from q.
 * @param q The queue being dequeued from.
 * @param t A double pointer to the task struct being dequeued.
 * @returns `1` if successful, `0` otherwise.
 */
int dequeue(
    task_queue_t *q,
    task_t **t);

/**
 * @brief Returns whether q is full.
 * @param q The queue.
 * @returns `1` if the queue is full, else `0`.
 */
int is_full(
    const task_queue_t *q);

/**
 * @brief Returns whether q is empty.
 * @param q The queue.
 * @returns `1` if the queue is empty, else `0`.
 */
int is_empty(
    const task_queue_t *q);

#endif