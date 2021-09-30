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

#include "../inc/hal_com.h"
#include "cmsis_os2.h"

typedef struct {
    // Configuration section.
    uint16_t capacity;
    uint16_t recv_length;
    // Resource section.
    DMA_HandleTypeDef *rx_dma_handle;
    DMA_HandleTypeDef *tx_dma_handle;
    uint8_t *buffer;
    // General resource section.
    HAL_DEVICE_DEFINE(com);
} com_device_t;

static com_device_t com_control_handle[com_N] = {
    {
        .capacity = 128,
	},
};

static rte_error_t com_send(hal_device_t *device, uint8_t *data, uint32_t size, uint32_t timeout_ms)
{
    HAL_StatusTypeDef result = HAL_ERROR;
    if (size == 0)
        return RTE_SUCCESS;
    result = HAL_UART_Transmit(
                com_control_handle[device->device_id].com_hal_obj,
                data, size, timeout_ms);
    if (result == HAL_OK)
        return RTE_SUCCESS;
    else if(result == HAL_TIMEOUT)
        return RTE_ERR_TIMEOUT;
    else
        return RTE_ERR_UNDEFINE;
}

static rte_error_t com_recv(hal_device_t *device, uint8_t *buffer, uint32_t *size, uint32_t timeout_ms)
{
    HAL_StatusTypeDef result = HAL_ERROR;
    com_name_t com_name = device->device_id;
    result = HAL_UARTEx_ReceiveToIdle(
                com_control_handle[com_name].com_hal_obj, buffer,
                *size, (uint16_t *)size, timeout_ms);
    if (result == HAL_OK) {
        return RTE_SUCCESS;
    } else if(result == HAL_TIMEOUT) {
        *size = 0;
        return RTE_ERR_TIMEOUT;
    } else {
        *size = 0;
        return RTE_ERR_UNDEFINE;
    }
}

static rte_error_t com_send_async(hal_device_t *device, uint8_t *data, uint32_t size, uint32_t timeout_ms)
{
    com_name_t com_name = device->device_id;
    if (com_control_handle[com_name].tx_dma_handle != NULL) {
        uint8_t *tx_buffer = memory_alloc_align(BANK_DMA, 32, size);
        if (tx_buffer == NULL)
            return RTE_ERR_NO_RSRC;
        memcpy(tx_buffer, data, size);
        HAL_RAM_CLEAN_PRE_SEND(tx_buffer, size);
        HAL_UART_Transmit_DMA(com_control_handle[com_name].com_hal_obj,
                            tx_buffer,
                            size);
        if (osSemaphoreAcquire(com_control_handle[com_name].tx_sema, timeout_ms) == osOK) {
            memory_free(BANK_DMA, tx_buffer);
            return RTE_SUCCESS;
        }
        HAL_UART_AbortTransmit_IT(com_control_handle[com_name].com_hal_obj);
        memory_free(BANK_DMA, tx_buffer);
        return RTE_ERR_TIMEOUT;
    } else {
        HAL_UART_Transmit_IT(com_control_handle[com_name].com_hal_obj, data, size);
        if (osSemaphoreAcquire(com_control_handle[com_name].tx_sema, timeout_ms) == osOK) {
            return RTE_SUCCESS;
        }
        HAL_UART_AbortTransmit_IT(com_control_handle[com_name].com_hal_obj);
        return RTE_ERR_TIMEOUT;
    }
}

static rte_error_t com_recv_async(hal_device_t *device, uint8_t *buffer, uint32_t *size, uint32_t timeout_ms)
{
    com_name_t com_name = device->device_id;
    if (com_control_handle[com_name].rx_dma_handle != NULL) {
        HAL_UARTEx_ReceiveToIdle_DMA(com_control_handle[com_name].com_hal_obj,
                            com_control_handle[com_name].buffer,
                            com_control_handle[com_name].capacity);
        if (osSemaphoreAcquire(com_control_handle[com_name].rx_sema, timeout_ms) == osOK) {
            HAL_RAM_CLEAN_AFTER_REC(com_control_handle[com_name].buffer, com_control_handle[com_name].recv_length);
            *size = RTE_MIN(com_control_handle[com_name].recv_length, *size);
            memcpy(buffer, com_control_handle[com_name].buffer, *size);
            return RTE_SUCCESS;
        }
        *size = 0;
        HAL_UART_AbortReceive_IT(com_control_handle[com_name].com_hal_obj);
        return RTE_ERR_TIMEOUT;
    } else {
        HAL_UARTEx_ReceiveToIdle_IT(com_control_handle[com_name].com_hal_obj, buffer, *size);
        if (osSemaphoreAcquire(com_control_handle[com_name].rx_sema, timeout_ms) == osOK) {
            *size = com_control_handle[com_name].recv_length;
            return RTE_SUCCESS;
        }
        *size = 0;
        HAL_UART_AbortReceive_IT(com_control_handle[com_name].com_hal_obj);
        return RTE_ERR_TIMEOUT;
    }
}

rte_error_t com_create(com_name_t com_name, com_configuration_t *config, hal_device_t **device)
{
    if (RTE_UNLIKELY(config == NULL) ||
        RTE_UNLIKELY(device == NULL))
        return RTE_ERR_PARAM;
    // Given hal lib's resources
    com_control_handle[com_name].com_hal_obj = config->huart;
    com_control_handle[com_name].rx_dma_handle = config->hrx_dma;
    com_control_handle[com_name].tx_dma_handle = config->htx_dma;
    // Check if use rx dma
    if (com_control_handle[com_name].rx_dma_handle != NULL) {
        com_control_handle[com_name].buffer = memory_alloc_align(BANK_DMA, 32, com_control_handle[com_name].capacity);
    }

    // Create general resources
    HAL_DEVICE_INIT_GENERAL(com, com_name, com_recv, com_send, com_recv_async, com_send_async, config->huart)
    return RTE_SUCCESS;
}


/**
 * @brief Will be called by hal lib only in normal mode.
 *
 * @param huart
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    for(com_name_t com_name = 0; com_name < com_N; com_name++) {
        if (com_control_handle[com_name].com_hal_obj == huart &&
            huart->ReceptionType == HAL_UART_RECEPTION_STANDARD) {
            com_control_handle[com_name].recv_length = size;
            osSemaphoreRelease(com_control_handle[com_name].rx_sema);
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_DEVICE_OP_COMPLET_HANDLE(com, tx, huart)
}
