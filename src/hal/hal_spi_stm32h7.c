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
#include "../../inc/hal/hal_spi.h"
#include "cmsis_os2.h"

spi_device_t spi_control_handle[SPI_N] = {0};

static rte_error_t spi_send(hal_device_t *device, uint8_t *data, uint32_t size, uint32_t timeout_ms)
{
    HAL_StatusTypeDef result = HAL_ERROR;
    if (size == 0)
        return RTE_SUCCESS;
    result = HAL_SPI_Transmit(
                spi_control_handle[device->device_id].spi_handle,
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
                spi_control_handle[device->device_id].spi_handle,
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
                spi_control_handle[device->device_id].spi_handle,
                data, size);
    if (result == HAL_OK) {
        osSemaphoreAcquire(spi_control_handle[device->device_id].tx_sema, timeout_ms);
        return RTE_SUCCESS;
    }
    else if(result == HAL_TIMEOUT)
        return RTE_ERR_TIMEOUT;
    else
        return RTE_ERR_UNDEFINE;
}

rte_error_t spi_create(spi_name_t spi_name, spi_configuration_t *config, hal_device_t **device)
{
    if (RTE_UNLIKELY(config == NULL) ||
        RTE_UNLIKELY(device == NULL))
        return RTE_ERR_PARAM;
    // Given hal lib's resources
    spi_control_handle[spi_name].spi_handle = config->spi_handle;
    spi_control_handle[spi_name].tx_dma_handle = config->tx_dma_handle;
    spi_control_handle[spi_name].rx_dma_handle = config->rx_dma_handle;
    // Create sema.
    spi_control_handle[spi_name].tx_sema = osSemaphoreNew(1, 0, NULL);
    // Add device api.
    spi_control_handle[spi_name].device.device_id = spi_name;
    spi_control_handle[spi_name].device.read = spi_recv;
    spi_control_handle[spi_name].device.write = spi_send;
    spi_control_handle[spi_name].device.read_async = NULL;
    spi_control_handle[spi_name].device.write_async = spi_send_async;
    spi_control_handle[spi_name].device.op_callback = NULL;
    spi_control_handle[spi_name].device.user_arg = NULL;
    *device = &spi_control_handle[spi_name].device;
    return RTE_SUCCESS;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    for(spi_name_t spi_name = 0; spi_name < SPI_N; spi_name++) {
        if (spi_control_handle[spi_name].spi_handle == hspi) {
            osSemaphoreRelease(spi_control_handle[spi_name].tx_sema);
        }
    }
}