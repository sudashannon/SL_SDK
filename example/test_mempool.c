#include <stdio.h>
#include "rte_include.h"
#include "SDL.h"

static int membench(void *arg);
static double diff_in_second(struct timespec t1, struct timespec t2);

typedef struct {
    int alloc_size;
    int count;
    int loop;
    bool verbose;
} thread_args_t;

void test_mempool(int thread_num, int max_malloc_size, int count, int loop, bool if_verbose)
{
    thread_args_t thread_args = {
        .alloc_size = 1,
        .count = 1,
        .loop = 1,
        .verbose = false,
    };
    thread_args.alloc_size = max_malloc_size;
    thread_args.count = count;
    thread_args.loop = loop;
    thread_args.verbose = if_verbose;

    SDL_Thread *p[thread_num];
    struct timespec start, end;

    RTE_LOGI("Number of threads : %d", thread_num);
    RTE_LOGI("Malloc size : %d KB", thread_args.alloc_size / (1024));
    RTE_LOGI("Number of councurrent malloc per thread: %d", thread_args.count);
    RTE_LOGI("Number of loops per thread : %d", thread_args.loop);

    clock_gettime(CLOCK_REALTIME, &start);

    for (int i = 0; i < thread_num; i++)
        p[i] = SDL_CreateThread(membench, NULL, &thread_args);

    for (int i = 0; i < thread_num; i++)
        SDL_WaitThread(p[i], NULL);

    clock_gettime(CLOCK_REALTIME, &end);

    printf("%lf sec\n", diff_in_second(start, end));
}

/* memory benchmark */
static int membench(void *arg)
{
    thread_args_t *thread_args = (thread_args_t *) arg;

    int alloc_size = thread_args->alloc_size;
    int count = thread_args->count;
    int loop = thread_args->loop;
    int verbose = thread_args->verbose;

    int *mem[count];
    int size[count];

    int loopcnt = 0;

    while (1) {
        if (loop != 0 && ++loopcnt > loop)
            break;

        for (int i = 0; i < count; i++) {
            size[i] = (rand() % alloc_size);

            mem[i]  = (int *) memory_alloc(BANK_DEFAULT, size[i]);
            if (!mem[i]) {
                RTE_LOGF("Fail to allocate memory.");
                return -1;
            }
            memset(mem[i], 1, size[i]);
            memset(mem[i], 0, size[i]);
            if (verbose) {
                RTE_LOGI("thread: %d, loop: %d, mem[%d] %d Bytes malloc",
                       SDL_ThreadID(), loopcnt, i, size[i]);
            }
            mem[i]  = (int *) memory_realloc(BANK_DEFAULT, mem[i], size[i]);
            if (!mem[i]) {
                RTE_LOGF("Fail to allocate memory.");
                return -1;
            }
            size[i] = (rand() % alloc_size);
            if (verbose) {
                RTE_LOGI("thread: %d, loop: %d, mem[%d] %d Bytes realloc",
                       SDL_ThreadID(), loopcnt, i, size[i]);
            }
        }

        for (int i = 0; i < count; i++) {
            memory_free(BANK_DEFAULT, mem[i]);
            if (verbose) {
                RTE_LOGI("thread: %d, loop: %d, mem[%d] %d Bytes free",
                       SDL_ThreadID(), loopcnt, i, size[i]);
            }
        }
    }
    return 0;
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec - t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}