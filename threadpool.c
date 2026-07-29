#include <pthread.h>
#include <stdlib.h>
#include "threadpool.h"
#include "task_queue.h"

void *thread_mainloop(void *pool)
{
    threadpool_t* p = (threadpool_t*) pool;
    task_t* t;
    while (1)
    {
        pthread_mutex_lock(&p->mutex_pool);
        while (is_empty(p->task_queue) && !p->threads_exit)
        {
            pthread_cond_wait(&p->cond_queue_not_empty, &p->mutex_pool);
        }
        if (p->threads_exit)
        {
            pthread_mutex_unlock(&p->mutex_pool);
            break;
        }
        // should be possible now exited the loop and not been signalled to exit the thread
        int success = dequeue(p->task_queue, &t);
        if (success) {
            p->threads_active++;
            pthread_cond_signal(&p->cond_queue_not_full);
            pthread_mutex_unlock(&p->mutex_pool);

            t->func(t->args);
            if (t->cleanup != NULL) t->cleanup(t->args);
            free(t);
            t = NULL;

            pthread_mutex_lock(&p->mutex_pool);
            p->threads_active--;
            pthread_cond_signal(&p->cond_check_idle);
        }
        pthread_mutex_unlock(&p->mutex_pool);
    }
    return NULL;
}

threadpool_t *threadpool_init(int max_tasks, int max_threads)
{
    threadpool_t *pool = malloc(sizeof(threadpool_t));
    if (pool == NULL) return NULL;
    pool->task_queue = task_queue_init(max_tasks);
    if (pool->task_queue == NULL)
    {
        free(pool);
        return NULL;
    }
    pool->threads_arr = calloc(max_threads, sizeof(pthread_t));
    if (pool->threads_arr == NULL)
    {
        task_queue_destroy(&pool->task_queue);
        free(pool);
        return NULL;
    }
    pool->threads_max = max_threads;
    pool->threads_count = 0;
    pool->threads_active = 0;
    pool->threads_exit = 0;
    return pool;
}

void threadpool_destroy(threadpool_t **pool)
{
    if (pool == NULL || *pool == NULL) return;
    free((*pool)->threads_arr);
    task_queue_destroy(&(*pool)->task_queue);
    free(*pool);
    *pool = NULL;
}

int threadpool_start(threadpool_t *pool)
{
    if (pool == NULL) return 0;
    if (pool->threads_count > 0) return 0;

    pthread_mutex_init(&pool->mutex_pool, NULL);
    pthread_cond_init(&pool->cond_queue_not_empty, NULL);
    pthread_cond_init(&pool->cond_queue_not_full, NULL);
    pthread_cond_init(&pool->cond_check_idle, NULL);

    void* args = (void*) pool;


    for (int i = 0; i < pool->threads_max; i++)
    {
        if (pthread_create(pool->threads_arr + i, NULL, &thread_mainloop, args) != 0) break;
        pool->threads_count++;
    }

    if (pool->threads_count < pool->threads_max)
    {
        threadpool_end(pool);
        return 0;
    }

    return 1;
}

int threadpool_add(threadpool_t *pool, task_t **t)
{
    if (pool == NULL || t == NULL || *t == NULL) return 0;

    pthread_mutex_lock(&pool->mutex_pool);
    while (is_full(pool->task_queue))
    {
        pthread_cond_wait(&pool->cond_queue_not_full, &pool->mutex_pool);
    }
    // should be possible now we've been signalled the queue is not full
    if (enqueue(pool->task_queue, *t) == 0) return 0; 
    // force ownership of the task to the threadpool by removing caller's access to the task
    *t = NULL; 
    pthread_cond_signal(&pool->cond_queue_not_empty);
    pthread_mutex_unlock(&pool->mutex_pool);
    return 1;
}

int threadpool_wait(threadpool_t *pool)
{
    if (pool == NULL) return 0;

    pthread_mutex_lock(&pool->mutex_pool);
    while (!is_empty(pool->task_queue) || pool->threads_active > 0)
    {
        pthread_cond_wait(&pool->cond_check_idle, &pool->mutex_pool);
    }
    pthread_mutex_unlock(&pool->mutex_pool);
    return 1;
}

int threadpool_end(threadpool_t *pool)
{   
    if (pool == NULL) return 0;
    if (pool->threads_count == 0) return 0;

    threadpool_wait(pool);
    pthread_mutex_lock(&pool->mutex_pool);
    pool->threads_exit = 1;
    pthread_cond_broadcast(&pool->cond_queue_not_empty);
    pthread_mutex_unlock(&pool->mutex_pool);

    for (int i = 0; i < pool->threads_count; i++)
    {
        if (pthread_join(pool->threads_arr[i], NULL) != 0) return 0;
    }
    pool->threads_count = 0;
    pool->threads_exit = 0;
    pthread_mutex_destroy(&pool->mutex_pool);
    pthread_cond_destroy(&pool->cond_queue_not_empty);
    pthread_cond_destroy(&pool->cond_queue_not_full);
    pthread_cond_destroy(&pool->cond_check_idle);
    return 1;
}