/**
 * @file rte_adapter.c
 * @author Leon Shan (813475603@qq.com)
 * @brief Adapter for cmsis_rtos2.
 * @version 1.0.0
 * @date 2021-07-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../inc/rte_include.h"
#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"

/**
 * @brief Ram buffer used for memory pool.
 *
 */
MEM_ALIGN_NBYTES (__attribute__((section (".RAM_RTE"))) static uint8_t mempool_buffer[RTE_MEMPOOL_SIZE], MEM_BLOCK_ALIGN) = {0};
MEM_ALIGN_NBYTES (__attribute__((section (".RAM_DMA"))) static uint8_t dma_buffer[32 * 1024], MEM_BLOCK_ALIGN) = {0};
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
static const osMutexAttr_t mem_mutex_attr[BANK_CNT] = {
    {
        LOG_STR(BANK_DEFAULT),
        osMutexPrioInherit | osMutexRecursive,
        NULL,
        0U
    },
    {
        LOG_STR(BANK_DMA),
        osMutexPrioInherit | osMutexRecursive,
        NULL,
        0U
    }
};
static rte_mutex_t mem_mutex_instance[BANK_CNT] = {NULL};
/**
 * @brief Wrapper for mutex lock, which is adapted for CMSIS-RTOS2.
 *
 * @param mutex
 * @return rte_error_t
 */
rte_error_t rte_mutex_lock(void *mutex)
{
    return (osOK == osMutexAcquire(mutex, osWaitForever) ? RTE_SUCCESS : RTE_ERR_UNDEFINE );
}
/**
 * @brief Wrapper for mutex unlock, which is adapted for CMSIS-RTOS2.
 *
 * @param mutex
 * @return rte_error_t
 */
rte_error_t rte_mutex_unlock(void *mutex)
{
    return (osOK == osMutexRelease(mutex) ? RTE_SUCCESS : RTE_ERR_UNDEFINE );
}
/**
 * @brief Wrapper for get hal's tick, which is adapted for CMSIS-RTOS2.
 *
 * @return uint32_t
 */
uint32_t HAL_GetTick(void)
{
    return osKernelGetTickCount();
}
/**
 * @brief Wrapper for get system's tick, which is adapted for CMSIS-RTOS2.
 *
 * @return uint32_t
 */
uint32_t rte_get_tick(void)
{
    return osKernelGetTickCount();
}
/**
 * @brief Wrapper for hal delay, which is adapted for CMSIS-RTOS2.
 *
 * @return uint32_t
 */
void HAL_Delay(uint32_t Delay)
{
    osDelay(Delay);
}
/**
 * @brief Wrapper for system delay, which is adapted for CMSIS-RTOS2.
 *
 * @return uint32_t
 */
void rte_delay_ms(uint32_t ms)
{
    osDelay(ms);
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
 * @brief Init the rte, must be called before tbe system begins.
 *
 */
void rte_init(void)
{
    mem_mutex_instance[BANK_DEFAULT].mutex = (void *)osMutexNew(&mem_mutex_attr[BANK_DEFAULT]);
    mem_mutex_instance[BANK_DEFAULT].lock = rte_mutex_lock;
    mem_mutex_instance[BANK_DEFAULT].unlock = rte_mutex_unlock;
    mem_mutex_instance[BANK_DEFAULT].trylock = NULL;
    memory_pool(BANK_DEFAULT, &mem_mutex_instance[BANK_DEFAULT], mempool_buffer, sizeof(mempool_buffer));
    mem_mutex_instance[BANK_DMA].mutex = (void *)osMutexNew(&mem_mutex_attr[BANK_DMA]);
    mem_mutex_instance[BANK_DMA].lock = rte_mutex_lock;
    mem_mutex_instance[BANK_DMA].unlock = rte_mutex_unlock;
    mem_mutex_instance[BANK_DMA].trylock = NULL;
    memory_pool(BANK_DMA, &mem_mutex_instance[BANK_DMA], dma_buffer, sizeof(dma_buffer));
    rte_allocator_instance.malloc = rte_malloc;
    rte_allocator_instance.calloc = rte_calloc;
    rte_allocator_instance.realloc = rte_realloc;
    rte_allocator_instance.free = rte_free;
    log_mutex_instance.mutex = (void *)osMutexNew(NULL);
    log_mutex_instance.lock = rte_mutex_lock;
    log_mutex_instance.unlock = rte_mutex_unlock;
    log_mutex_instance.trylock = NULL;
    log_init(&log_mutex_instance, NULL, rte_get_tick);
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
    for(uint8_t i = 0; i < BANK_CNT; i++)
        osMutexDelete(mem_mutex_instance[i].mutex);
    osMutexDelete(log_mutex_instance.mutex);
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
