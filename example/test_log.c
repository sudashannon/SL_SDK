#include <stdio.h>
#include "rte_include.h"
#include "SDL.h"


SDL_Thread *log_thread1_handle, *log_thread2_handle;

static int log_thread2(void *param)
{
    SDL_threadID tid = SDL_ThreadID();
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    return 0;
}

static int log_thread1(void *param)
{
    log_thread2_handle = SDL_CreateThread(log_thread2, "log_2", NULL);
    SDL_threadID tid = SDL_ThreadID();
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    RTE_LOGI("This is a log from %d", tid);
    return 0;
}

void test_log(void)
{
    log_thread1_handle = SDL_CreateThread(log_thread1, "log_1", NULL);
    SDL_WaitThread(log_thread1_handle, NULL);
    SDL_WaitThread(log_thread2_handle, NULL);
}
