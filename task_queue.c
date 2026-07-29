#include "task_queue.h"
#include <stddef.h>
#include <stdlib.h>

task_queue_t *task_queue_init(int max_size)
{
    task_queue_t *q = malloc(sizeof(task_queue_t));
    if (q == NULL) return NULL;
    q->arr = calloc(max_size, sizeof(task_t *));
    if (q->arr == NULL)
    {
        free(q);
        return NULL;
    }
    q->max_size = max_size;
    q->size = 0;
    q->head = 0;
    q->tail = 0;
    return q;
}

void task_queue_destroy(task_queue_t **q)
{
    if (q == NULL || *q == NULL) return;
    free((*q)->arr);
    free(*q);
    *q = NULL;
}

int enqueue(task_queue_t *q, task_t *t)
{
    if (is_full(q)) return 0;
    q->arr[q->tail] = t;
    q->tail++;
    q->tail %= q->max_size;
    q->size++;
    return 1;
}

int dequeue(task_queue_t *q, task_t **t)
{
    if (is_empty(q)) return 0;
    *t = q->arr[q->head];
    q->head++;
    q->head %= q->max_size;
    q->size--;
    return 1;
}

int is_full(const task_queue_t *q)
{
    return q->size == q->max_size;
}

int is_empty(const task_queue_t *q)
{
    return q->size == 0;
}