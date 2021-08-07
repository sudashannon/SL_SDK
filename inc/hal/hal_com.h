/**
 * @file hal_com.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-07-28
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __HAL_COM_H
#define __HAL_COM_H
#include "RTE_Components.h"
#include CMSIS_device_header
#include "hal.h"
/**
 * @brief List all provided com instance's name.
 *
 */
typedef enum {
    COM_1 = 0,
    COM_N,
} com_name_t;

typedef struct {
    uint32_t boundrate;
    void *user_arg1;
    void *user_arg2;
} com_configuration_t;

/**
 * @brief Create a selected com device.
 *
 * @param com_name
 * @param config
 * @param device
 * @return rte_error_t
 */
extern rte_error_t com_create(com_name_t com_name, com_configuration_t *config, hal_device_t **device);
/**
 * @brief Destroy a selected com device.
 *
 * @param com_name
 * @param device
 * @return rte_error_t
 */
extern rte_error_t com_destroy(com_name_t com_name, hal_device_t *device);

#endif
