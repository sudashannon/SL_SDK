/**
 * @file rte_adapter.c
 * @author Leon Shan (813475603@qq.com)
 * @brief Adapter for metal environment.
 * @version 1.0.0
 * @date 2021-07-27
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "rte_include.h"
#include "hal_include.h"
#include "stm32h7xx.h"

/**
 * @brief Ram buffer used for memory pool.
 *
 */
MEM_ALIGN_NBYTES (__attribute__((section (".rte_memory"))) uint8_t mempool_buffer[RTE_MEMPOOL_SIZE], MEM_BLOCK_ALIGN) = {0};
MEM_ALIGN_NBYTES (__attribute__((section (".dma_memory"))) uint8_t dma_buffer[288 * 1024], MEM_BLOCK_ALIGN) = {0};

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
    /* Configure the SysTick to have interrupt in 1ms time basis*/
    RTE_ASSERT(HAL_SYSTICK_Config(SystemCoreClock / RTE_SUGAR_KERNEL_FREQUENCY) == 0U);
    memory_pool(BANK_DEFAULT, NULL, mempool_buffer, sizeof(mempool_buffer));
    memory_pool(BANK_DMA, NULL, dma_buffer, sizeof(dma_buffer));
    log_init(NULL, NULL, HAL_GetTick);
    timer_init(RTE_MAX_TIMER_GROUP_SIZE);
    /* Init sugar kernel */
    sugar_kernel_init(NULL, 512, true);
    shell_init();
}

void rte_delay_us(volatile uint32_t micros)
{
	uint32_t start = DWT->CYCCNT;
	/* Go to number of cycles for system */
	micros *= (SystemCoreClock / 1000000);
	/* Delay till end */
	while ((DWT->CYCCNT - start) < micros);
}

void rte_yield(void)
{
    __WFI();
}

uint32_t HAL_GetTick(void)
{
    return rte_get_tick() * (1000 / RTE_SUGAR_KERNEL_FREQUENCY);
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
    timer_tick_handle();
}
