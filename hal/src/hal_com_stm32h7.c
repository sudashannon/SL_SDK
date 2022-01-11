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
#include "usart.h"

typedef struct {
    // Configuration section.
    uint16_t capacity;
    uint16_t recv_length;
    // Resource section.
    bool if_trans_enable_dma;
    bool if_recv_enable_dma;
    bool if_recv_enable_fifo;
    uint8_t *buffer;
    void *driver_handle;
    // General resource section.
    hal_device_t device;
} com_device_t;

static com_device_t com_control_handle[com_N] = {
    {
        .capacity = 2048,
        .recv_length = 0,
        .driver_handle = &huart1,
        .if_trans_enable_dma = true,
        .if_recv_enable_dma = true,
        .if_recv_enable_fifo = false,
	},
};

static rte_error_t com_send(hal_device_t *device, uint8_t *data, uint32_t size, uint32_t timeout_ms)
{
    HAL_StatusTypeDef result = HAL_ERROR;
    if (size == 0)
        return RTE_SUCCESS;
    result = HAL_UART_Transmit(
                com_control_handle[device->device_id].driver_handle,
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
    result = HAL_UART_Receive(
                com_control_handle[com_name].driver_handle, buffer,
                *size, timeout_ms);
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
    if (com_control_handle[com_name].if_trans_enable_dma) {
        uint8_t *tx_buffer = memory_alloc_align(BANK_DMA, __SCB_DCACHE_LINE_SIZE, size);
        if (tx_buffer == NULL)
            return RTE_ERR_NO_RSRC;
        memcpy(tx_buffer, data, size);
        HAL_RAM_CLEAN_PRE_SEND(tx_buffer, size);
        hal_device_prepare_wait(device, tx);
        HAL_UART_Transmit_DMA(com_control_handle[com_name].driver_handle,
                            tx_buffer,
                            size);
        if (hal_device_wait_tx_ready(device, timeout_ms) == RTE_SUCCESS) {
            memory_free(BANK_DMA, tx_buffer);
            return RTE_SUCCESS;
        }
        HAL_UART_AbortTransmit_IT(com_control_handle[com_name].driver_handle);
        memory_free(BANK_DMA, tx_buffer);
        return RTE_ERR_TIMEOUT;
    } else {
        hal_device_prepare_wait(device, tx);
        HAL_UART_Transmit_IT(com_control_handle[com_name].driver_handle, data, size);
        if (hal_device_wait_tx_ready(device, timeout_ms) == RTE_SUCCESS) {
            return RTE_SUCCESS;
        }
        HAL_UART_AbortTransmit_IT(com_control_handle[com_name].driver_handle);
        return RTE_ERR_TIMEOUT;
    }
}

static rte_error_t com_recv_async(hal_device_t *device, uint8_t *buffer, uint32_t *size, uint32_t timeout_ms)
{
    com_name_t com_name = device->device_id;
    if (com_control_handle[com_name].if_recv_enable_dma) {
        hal_device_prepare_wait(device, rx);
        HAL_UARTEx_ReceiveToIdle_DMA(com_control_handle[com_name].driver_handle,
                            com_control_handle[com_name].buffer,
                            com_control_handle[com_name].capacity);
        if (hal_device_wait_rx_ready(device, timeout_ms) == RTE_SUCCESS) {
            HAL_RAM_CLEAN_AFTER_REC(com_control_handle[com_name].buffer, com_control_handle[com_name].recv_length);
            *size = RTE_MIN(com_control_handle[com_name].recv_length, *size);
            memcpy(buffer, com_control_handle[com_name].buffer, *size);
            return RTE_SUCCESS;
        }
        *size = 0;
        HAL_UART_AbortReceive_IT(com_control_handle[com_name].driver_handle);
        return RTE_ERR_TIMEOUT;
    } else {
        hal_device_prepare_wait(device, rx);
        HAL_UARTEx_ReceiveToIdle_IT(com_control_handle[com_name].driver_handle, buffer, *size);
        if (hal_device_wait_rx_ready(device, timeout_ms) == RTE_SUCCESS) {
            *size = com_control_handle[com_name].recv_length;
            return RTE_SUCCESS;
        }
        *size = 0;
        HAL_UART_AbortReceive_IT(com_control_handle[com_name].driver_handle);
        return RTE_ERR_TIMEOUT;
    }
}

static rte_error_t com_create(com_name_t com_name, hal_device_t **device)
{
    if (RTE_UNLIKELY(device == NULL))
        return RTE_ERR_PARAM;
    // Check if use rx dma
    if (com_control_handle[com_name].if_recv_enable_dma) {
        com_control_handle[com_name].buffer = memory_alloc_align(BANK_DMA, __SCB_DCACHE_LINE_SIZE, com_control_handle[com_name].capacity);
    }
    // Create general resources
    hal_device_initialize(com, com_name,
                            com_recv, com_send,
                            com_recv_async, com_send_async, device);
    return RTE_SUCCESS;
}

static rte_error_t com_destroy(com_name_t com_name)
{
    return RTE_SUCCESS;
}

HAL_DECLARE_CONSTRUCTOR(com_regist)
{
    hal_device_register(com, "com");
}

HAL_DECLARE_DESTRUCTOR(com_unregist)
{
    hal_device_unregister(com);
}

/**
 * @brief Will be called by hal lib only in normal mode.
 *
 * @param huart
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    hal_device_poll(com) {
        if (com_control_handle[this_device].driver_handle == huart) {
            com_control_handle[this_device].recv_length = size;
            hal_device_active(&com_control_handle[this_device].device, rx);
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    hal_device_poll(com) {
        if (com_control_handle[this_device].driver_handle == huart) {
            hal_device_active(&com_control_handle[this_device].device, tx);
        }
    }
}
