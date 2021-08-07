/**
 * @file hal_com_stm32h7.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-07
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../inc/hal/hal_com.h"
#include "cmsis_os2.h"

typedef struct {
    // Configuration section.
    com_name_t name;
    uint16_t capacity;
    uint16_t recv_length;
    // Resource section.
    UART_HandleTypeDef *uart_handle;
    DMA_HandleTypeDef *dma_handle;
    uint8_t *buffer;
    rte_mutex_t mutex;
    osSemaphoreId_t sema;
} bsp_com_handle_t;

static bsp_com_handle_t com_control_handle[COM_N] = {
    {
		.name = COM_1,
        .capacity = 128,
	},
};

rte_error_t com_init(com_name_t com_name, com_configuration_t *config)
{
    if (RTE_UNLIKELY(config == NULL))
        return RTE_ERR_PARAM;
    // Given hal lib's resources
    com_control_handle[com_name].uart_handle = config->user_arg1;
    com_control_handle[com_name].dma_handle = config->user_arg2;
    // Create mutex
    com_control_handle[com_name].mutex.mutex = osMutexNew(NULL);
    com_control_handle[com_name].mutex.lock = rte_mutex_lock;
    com_control_handle[com_name].mutex.unlock = rte_mutex_unlock;
    com_control_handle[com_name].mutex.trylock = NULL;
    com_control_handle[com_name].sema = osSemaphoreNew(com_control_handle[com_name].capacity, 0, NULL);
    // Check if use dma
    if (com_control_handle[com_name].dma_handle != NULL) {
        com_control_handle[com_name].buffer = memory_calloc(BANK_DMA, com_control_handle[com_name].capacity);
        __HAL_UART_ENABLE_IT(com_control_handle[com_name].uart_handle, UART_IT_IDLE);
        __HAL_UART_CLEAR_IDLEFLAG(com_control_handle[com_name].uart_handle);
    }
    return RTE_SUCCESS;
}

rte_error_t com_send_sync(com_name_t com_name, uint8_t *data, uint16_t size)
{
    if (RTE_UNLIKELY(data == NULL))
        return RTE_ERR_PARAM;
    if (size == 0)
        return RTE_SUCCESS;
    HAL_UART_Transmit(com_control_handle[com_name].uart_handle, data, size, HAL_MAX_DELAY);
    return RTE_SUCCESS;
}

rte_error_t com_recv_async(com_name_t com_name, uint8_t *buffer, uint16_t *size, uint32_t timeout_ms)
{
    if(com_control_handle[com_name].dma_handle != NULL) {
        HAL_UART_Receive_DMA(com_control_handle[com_name].uart_handle,
                            com_control_handle[com_name].buffer,
                            *size);
        HAL_UART_DMAResume(com_control_handle[com_name].uart_handle);
        if(osSemaphoreAcquire(com_control_handle[com_name].sema, timeout_ms) == osErrorTimeout) {
            HAL_UART_DMAStop(com_control_handle[com_name].uart_handle);
            return RTE_ERR_TIMEOUT;
        }
        *size = com_control_handle[com_name].recv_length;
        memcpy(buffer, com_control_handle[com_name].buffer, *size);
        return RTE_SUCCESS;
    } else {
        HAL_UART_Receive_IT(com_control_handle[com_name].uart_handle, buffer, *size);
        if(osSemaphoreAcquire(com_control_handle[com_name].sema, timeout_ms) == osErrorTimeout)
            return RTE_ERR_TIMEOUT;
        return RTE_SUCCESS;
    }
}

void com_recv_callback(com_name_t com_name)
{
    if (com_control_handle[com_name].dma_handle != NULL) {
        HAL_UART_DMAStop(com_control_handle[com_name].uart_handle);
        com_control_handle[com_name].recv_length = com_control_handle[com_name].capacity -
                                (uint16_t)__HAL_DMA_GET_COUNTER(com_control_handle[com_name].dma_handle);
        /*
                the SCB_InvalidateDCache_by_Addr() requires a 32-Byte aligned address,
                adjust the address and the D-Cache size to invalidate accordingly.
            */
        uint32_t alignedAddr = (uint32_t)com_control_handle[com_name].buffer & ~0x1F;
        SCB_InvalidateDCache_by_Addr((uint32_t*)alignedAddr,
                                    (int32_t)(com_control_handle[com_name].recv_length +
                                    ((uint32_t)com_control_handle[com_name].buffer - alignedAddr)));
        osSemaphoreRelease(com_control_handle[com_name].sema);
    } else {
        osSemaphoreRelease(com_control_handle[com_name].sema);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    for(com_name_t com_name = 0; com_name < COM_N; com_name++) {
        if (com_control_handle[com_name].uart_handle == huart) {
            com_recv_callback(com_name);
        }
    }
}
