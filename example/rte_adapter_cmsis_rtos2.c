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

#include "rte_include.h"
#include "stm32h7xx.h"
#include "cmsis_os2.h"
#include "RTX_Config.h"
#include "hal_com.h"

/**
 * @brief Ram buffer used for memory pool.
 *
 */
MEM_ALIGN_NBYTES (__attribute__((section (".RAM_RTE"))) static uint8_t mempool_buffer[RTE_MEMPOOL_SIZE], MEM_BLOCK_ALIGN) = {0};
MEM_ALIGN_NBYTES (__attribute__((section (".RAM_DMA"))) static uint8_t dma_buffer[32 * 1024], MEM_BLOCK_ALIGN) = {0};
MEM_ALIGN_NBYTES (__attribute__((section (".RAM_MATH"))) static uint8_t math_buffer[128 * 1024], MEM_BLOCK_ALIGN) = {0};
static bool rte_ready = false;
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
 * @brief Wrapper for mutex try lock, which is adapted for CMSIS-RTOS2.
 *
 * @param mutex
 * @return rte_error_t
 */
rte_error_t rte_mutex_trylock(void *mutex, uint32_t timeout_ms)
{
    return (osOK == osMutexAcquire(mutex, timeout_ms) ? RTE_SUCCESS : RTE_ERR_TIMEOUT );
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
    return rte_get_tick();
}
/**
 * @brief Wrapper for get system's tick, which is adapted for CMSIS-RTOS2.
 *
 * @return uint32_t
 */
uint32_t rte_get_tick(void)
{
    /* Return current time in milliseconds */
    if (osKernelGetState () == osKernelRunning) {
        return osKernelGetTickCount();
	} else {
        static uint32_t ticks = 0U;
        uint32_t i;
        /* If Kernel is not running wait approximately 1 ms then increment
                and return auxiliary tick counter value */
        for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
            __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        }
        ticks++;
        return ticks;
    }
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
    osDelay(ms * 1000/OS_TICK_FREQ);
}
/**
 * @brief Wrapper for system delay, which is adapted for CMSIS-RTOS2.
 *
 * @return uint32_t
 */
void rte_delay_tick(uint32_t tick)
{
    osDelay(tick);
}
/**
 * @brief Wrapper for system delay, which is adapted for cortex-M.
 *
 * @return uint32_t
 */
void rte_delay_us(volatile uint32_t micros)
{
	uint32_t start = DWT->CYCCNT;
	/* Go to number of cycles for system */
	micros *= (SystemCoreClock / 1000000);
	/* Delay till end */
	while ((DWT->CYCCNT - start) < micros);
}
/**
 * @brief Wrapper for system yield.
 *
 */
void rte_yield(void)
{
    osThreadYield();
}
/**
 * @brief Init the rte, must be called before tbe system begins.
 *
 */
void rte_init(void)
{
	/* Enable TRC */
	CoreDebug->DEMCR &= ~0x01000000;
	CoreDebug->DEMCR |=  0x01000000;
	/* Enable counter */
	DWT->CTRL &= ~0x00000001;
	DWT->CTRL |=  0x00000001;
	/* Reset counter */
	DWT->CYCCNT = 0;
	/* 2 dummys */
	__ASM volatile ("NOP");
	__ASM volatile ("NOP");
    osMutexAttr_t mem_mutex_attr[BANK_CNT] = {
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
    osMutexAttr_t log_mutex_attr = {
        "log",
        osMutexPrioInherit | osMutexRecursive,
        NULL,
        0U
    };
    log_mutex_instance.mutex = (void *)osMutexNew(&log_mutex_attr);
    log_mutex_instance.lock = rte_mutex_lock;
    log_mutex_instance.unlock = rte_mutex_unlock;
    log_mutex_instance.trylock = NULL;
    log_init(&log_mutex_instance, NULL, rte_get_tick);
    timer_init(4);
    timer_create_group(&rte_timer_group, NULL, 8);
    shell_init();
    rte_ready = true;
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
    rte_ready = false;
    return RTE_SUCCESS;
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
