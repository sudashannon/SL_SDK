/**
 * @file hal_spi_stm32h7.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-07
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "../inc/hal_spi.h"
#include "cmsis_os2.h"
#include "spi.h"

typedef struct {
    // General resource section.
    hal_device_t device;
    // Configuration section.
    // Resource section.
    void *driver_handle;
} spi_device_t;

spi_device_t spi_control_handle[spi_N] = {
    {
        .driver_handle = &hspi2,
    }
};

static rte_error_t spi_send(hal_device_t *device, uint8_t *data, uint32_t size, uint32_t timeout_ms)
{
    HAL_StatusTypeDef result = HAL_ERROR;
    if (size == 0)
        return RTE_SUCCESS;
    result = HAL_SPI_Transmit(
                spi_control_handle[device->device_id].device.fd,
                data, size, timeout_ms);
    if (result == HAL_OK)
        return RTE_SUCCESS;
    else if(result == HAL_TIMEOUT)
        return RTE_ERR_TIMEOUT;
    else
        return RTE_ERR_UNDEFINE;
}

static rte_error_t spi_recv(hal_device_t *device, uint8_t *buffer, uint32_t *size, uint32_t timeout_ms)
{
    HAL_StatusTypeDef result = HAL_ERROR;
    if (*size == 0)
        return RTE_SUCCESS;
    result = HAL_SPI_Receive(
                spi_control_handle[device->device_id].device.fd,
                buffer, *size, timeout_ms);
    if (result == HAL_OK)
        return RTE_SUCCESS;
    else if(result == HAL_TIMEOUT)
        return RTE_ERR_TIMEOUT;
    else
        return RTE_ERR_UNDEFINE;
}

static rte_error_t spi_send_async(hal_device_t *device, uint8_t *data, uint32_t size, uint32_t timeout_ms)
{
    HAL_StatusTypeDef result = HAL_ERROR;
    if (size == 0)
        return RTE_SUCCESS;
    result = HAL_SPI_Transmit_DMA(
                spi_control_handle[device->device_id].device.fd,
                data, size);
    if (result == HAL_OK) {
        osSemaphoreAcquire(spi_control_handle[device->device_id].device.tx_sema, timeout_ms);
        return RTE_SUCCESS;
    }
    else if(result == HAL_TIMEOUT)
        return RTE_ERR_TIMEOUT;
    else
        return RTE_ERR_UNDEFINE;
}

rte_error_t spi_create(spi_name_t spi_name, hal_device_t **device)
{
    if (RTE_UNLIKELY(device == NULL))
        return RTE_ERR_PARAM;

    HAL_DEVICE_INIT_GENERAL(spi, spi_name, spi_recv, spi_send, NULL, spi_send_async, spi_control_handle[spi_name].driver_handle);
    return RTE_SUCCESS;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    HAL_DEVICE_OP_COMPLET_HANDLE(spi, tx, hspi);
}