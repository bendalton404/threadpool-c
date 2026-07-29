# threadpool-c

A small, portable thread pool implementation in C for executing arbitrary tasks with custom arguments and cleanup handlers.

## Overview

This repository implements a simple thread pool using POSIX threads (`pthread`). It includes a bounded task queue and worker threads that execute submitted `task_t` jobs. Each task carries a function pointer, optional arguments, and an optional cleanup callback.

The implementation is designed for projects that need a lightweight worker pool for concurrent task execution in C.

## Building

You can compile the code and test it with `gcc` and the POSIX thread library. From the repository root:

```sh
# for compiling to shared library
gcc -shared -fPIC -o threadpoollib.so threadpool.c task_queue.c

# for testing threadpool
gcc -pthread -o threadpool_test unit_tests/threadpool_test.c threadpool.c task_queue.c 
# run with 
./threadpool_test

# for testing task_queue
gcc -o task_queue_test unit_tests/task_queue_test.c task_queue.c 
# run with
./task_queue_test
```

## Interface

The public API is defined in `threadpool.h` and `task_queue.h`.

### Thread pool lifecycle

- `threadpool_init(int max_tasks, int max_threads)`
  - Creates a new thread pool with a bounded task queue and a maximum number of worker threads.
  - Returns a pointer to a `threadpool_t` object or `NULL` on failure.

- `threadpool_start(threadpool_t *pool)`
  - Starts worker threads for the pool.
  - Returns `1` on success and `0` on failure.

- `threadpool_wait(threadpool_t *pool)`
  - Blocks until the queue is empty and all active tasks have finished.
  - Returns `1` on success.

- `threadpool_end(threadpool_t *pool)`
  - Signals worker threads to stop after current tasks complete, joins them, and cleans up internal synchronization state.
  - Returns `1` on success.

- `threadpool_destroy(threadpool_t **pool)`
  - Frees pool resources and sets the caller's pointer to `NULL`.

### Task submission

Tasks are represented by `task_t` in `task_queue.h`.

```c
typedef struct {
    void (*func)(void *args);
    void *args;
    void (*cleanup)(void *args);
} task_t;
```

To submit a task:

1. Allocate a `task_t`.
2. Set `func`, `args`, and optionally `cleanup`.
3. Call `threadpool_add(pool, &task)`.
4. After successful submission, the caller's task pointer is set to `NULL` and the pool owns the task.

Example:

```c
int *my_task_arg = malloc(sizeof(int));
*my_task_arg = 999;

task_t *task = malloc(sizeof(task_t));
task->func = my_task_function;
task->args = (void *) my_task_arg;
task->cleanup = free;

threadpool_add(pool, &task);
// `task` is now NULL and the thread pool owns the allocated task.
// assumes the pool is initialized and started already.
```

More detailed example code can be found in `threadpool_test.c`.

## Notes

- The thread pool uses a bounded queue, so `threadpool_add` may block until space is available.
- The pool executes each submitted task and then frees the `task_t` structure internally. If a task allocates arguments, provide a cleanup callback to free them.
- This implementation uses POSIX threads and is intended for Unix-like environments that support `pthread`.
