/**
 * @file rte_adapter.c
 * @author Leon Shan (813475603@qq.com)
 * @brief Adapter for SDL2.
 * @version 1.0.0
 * @date 2021-07-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../inc/rte_include.h"
#include <stdio.h>
#include "SDL.h"
#include "shell.h"
/**
 * @brief Ram buffer and allocator used for memory pool.
 *
 */
MEM_ALIGN_NBYTES (static uint8_t mempool_buffer[RTE_MEMPOOL_SIZE], MEM_BLOCK_ALIGN) = {0};
MEM_ALIGN_NBYTES (static uint8_t dma_buffer[10 * 1024], MEM_BLOCK_ALIGN) = {0};
static rte_allocator_t rte_allocator_instance = {
    .malloc = NULL,
    .calloc = NULL,
    .realloc = NULL,
    .free = NULL,
};
/**
 * @brief Used for main timer group internal.
 *
 */
static timer_group_id_t rte_timer_group = 0;
/**
 * @brief Mutex used for log, which is adapted for CMSIS-RTOS2.
 *
 */
static rte_mutex_t log_mutex_instance = {NULL};
/**
 * @brief Mutex used for general allocator, which is adapted for CMSIS-RTOS2.
 *
 */
static rte_mutex_t mem_mutex_instance[BANK_CNT] = {NULL};
/**
 * @brief Wrapper for mutex lock, which is adapted for CMSIS-RTOS2.
 *
 * @param mutex
 * @return rte_error_t
 */
rte_error_t rte_mutex_lock(void *mutex)
{
    return (SDL_LockMutex(mutex) == 0) ? RTE_SUCCESS : RTE_ERR_UNDEFINE;
}
/**
 * @brief Wrapper for mutex unlock, which is adapted for CMSIS-RTOS2.
 *
 * @param mutex
 * @return rte_error_t
 */
rte_error_t rte_mutex_unlock(void *mutex)
{
    return (SDL_UnlockMutex(mutex) == 0) ? RTE_SUCCESS : RTE_ERR_UNDEFINE;
}
/**
 * @brief Wrapper for get system's tick, which is adapted for CMSIS-RTOS2.
 *
 * @return uint32_t
 */
uint32_t rte_get_tick(void)
{
    return SDL_GetTicks();
}
unsigned int userGetTick()
{
    return rte_get_tick();
}
/**
 * @brief Wrapper for system yield.
 *
 */
void rte_yield(void)
{
    SDL_Delay(0);
}
/**
 * @brief Wrapper for memory malloc, which is adapted for internal memory pool.
 *
 * @param size
 * @return void*
 */
static void *rte_malloc(uint32_t size)
{
    return memory_alloc(BANK_DEFAULT, size);
}
/**
 * @brief Wrapper for memory calloc, which is adapted for internal memory pool.
 *
 * @param size
 * @return void*
 */
static void *rte_calloc(uint32_t size)
{
    return memory_calloc(BANK_DEFAULT, size);
}
/**
 * @brief Wrapper for memory realloc, which is adapted for internal memory pool.
 *
 * @param ptr
 * @param size
 * @return void*
 */
static void *rte_realloc(void *ptr, uint32_t size)
{
    return memory_realloc(BANK_DEFAULT, ptr, size);
}
/**
 * @brief Wrapper for memory free, which is adapted for internal memory pool.
 *
 * @param ptr
 */
static void rte_free(void *ptr)
{
    memory_free(BANK_DEFAULT, ptr);
}
/**
 * @brief Wrapper for log output.
 *
 * @param ptr
 */
static size_t rte_log_output(uint8_t *data, size_t length)
{
    printf("%.*s", (int)length, (char *)data);
    fflush(stdout);
    return length;
}
/**
 * @brief Init the rte, must be called before tbe system begins.
 *
 */
void rte_init(void)
{
    mem_mutex_instance[BANK_DEFAULT].mutex = SDL_CreateMutex();
    mem_mutex_instance[BANK_DEFAULT].lock = rte_mutex_lock;
    mem_mutex_instance[BANK_DEFAULT].unlock = rte_mutex_unlock;
    mem_mutex_instance[BANK_DEFAULT].trylock = NULL;
    memory_pool(BANK_DEFAULT, &mem_mutex_instance[BANK_DEFAULT], mempool_buffer, sizeof(mempool_buffer));
    mem_mutex_instance[BANK_DMA].mutex = SDL_CreateMutex();
    mem_mutex_instance[BANK_DMA].lock = rte_mutex_lock;
    mem_mutex_instance[BANK_DMA].unlock = rte_mutex_unlock;
    mem_mutex_instance[BANK_DMA].trylock = NULL;
    memory_pool(BANK_DMA, &mem_mutex_instance[BANK_DMA], dma_buffer, sizeof(dma_buffer));
    rte_allocator_instance.malloc = rte_malloc;
    rte_allocator_instance.calloc = rte_calloc;
    rte_allocator_instance.realloc = rte_realloc;
    rte_allocator_instance.free = rte_free;
    log_mutex_instance.mutex = SDL_CreateMutex();
    log_mutex_instance.lock = rte_mutex_lock;
    log_mutex_instance.unlock = rte_mutex_unlock;
    log_mutex_instance.trylock = NULL;
    log_init(NULL, rte_log_output, rte_get_tick);
    timer_init(4, true);
    timer_create_group(&rte_timer_group, NULL);
}

/**
 * @brief Deinit the rte.
 *
 * @return rte_error_t
 */
rte_error_t rte_deinit(void)
{
    for (uint8_t i = 0; i < BANK_CNT; i++)
        SDL_DestroyMutex(mem_mutex_instance[i].mutex);
    SDL_DestroyMutex(log_mutex_instance.mutex);
    timer_deinit();
    return RTE_SUCCESS;
}
/**
 * @brief Get the general rte allocator.
 *
 * @return rte_allocator_t*
 */
rte_allocator_t *rte_get_general_allocator(void)
{
    return &rte_allocator_instance;
}
/**
 * @brief Get the main timer group.
 *
 * @return timer_group_id_t
 */
timer_group_id_t rte_get_main_timergroup(void)
{
    return rte_timer_group;
}
