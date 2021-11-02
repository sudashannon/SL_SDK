/**
 * @file hal_qspi.h
 * @author Leon Shan (813475603@qq.qspi)
 * @brief
 * @version 1.0.0
 * @date 2021-09-12
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __HAL_QSPI_H
#define __HAL_QSPI_H
#include "stm32h7xx.h"
#include "hal.h"
/**
 * @brief List all provided qspi instance's name.
 *
 */
typedef enum {
    qspi_1 = 0,
    qspi_N,
} qspi_name_t;

typedef struct {
    void *hqspi;
    void *hmdma;
} qspi_configuration_t;

/**
 * @brief Create a selected qspi device.
 *
 * @param qspi_name
 * @param config
 * @param device
 * @return rte_error_t
 */
extern rte_error_t qspi_create(qspi_name_t qspi_name, qspi_configuration_t *config, hal_device_t **device);
/**
 * @brief Destroy a selected qspi device.
 *
 * @param qspi_name
 * @param device
 * @return rte_error_t
 */
extern rte_error_t qspi_destroy(qspi_name_t qspi_name, hal_device_t *device);

#endif
