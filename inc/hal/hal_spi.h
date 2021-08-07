/**
 * @file hal_spi.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-07
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __HAL_SPI_H
#define __HAL_SPI_H
#include "RTE_Components.h"
#include CMSIS_device_header
#include "hal.h"

typedef enum
{
	SPI_LCD = 0,
	SPI_N,
} spi_name_t;

typedef struct {
    void *spi_handle;
    void *tx_dma_handle;
    void *rx_dma_handle;
} spi_configuration_t;

/**
 * @brief Create a selected spi device.
 *
 * @param com_name
 * @param config
 * @param device
 * @return rte_error_t
 */
extern rte_error_t spi_create(spi_name_t spi_name, spi_configuration_t *config, hal_device_t **device);
/**
 * @brief Destroy a selected com device.
 *
 * @param com_name
 * @param device
 * @return rte_error_t
 */
extern rte_error_t spi_destroy(spi_name_t com_name, hal_device_t *device);

#endif
