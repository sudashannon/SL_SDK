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
#include "../rte_include.h"
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
 * @brief Init a selected com via configuration table and the input baudrate.
 *
 * @param com_name
 * @param config
 * @return rte_error_t
 */
extern rte_error_t com_init(com_name_t com_name, com_configuration_t *config);
/**
 * @brief Deinit a selected com.
 *
 * @param com_name
 * @return rte_error_t
 */
extern rte_error_t com_deinit(com_name_t com_name);
/**
 * @brief Called in the com's recv interrupt.
 *
 */
extern void com_recv_callback(com_name_t com_name);
/**
 * @brief Send data via a selected com.
 *
 * @param com_name
 * @param data
 * @param size
 * @return rte_error_t
 */
extern rte_error_t com_send_sync(com_name_t com_name, uint8_t *data, uint16_t size);
/**
 * @brief Read data via a selected com in async mode, the user must defined the size he wants to read from the com.
 *
 * @param com_name
 * @param data
 * @param size
 * @param timeout_ms
 * @return rte_error_t
 */
extern rte_error_t com_recv_async(com_name_t com_name, uint8_t *buffer, uint16_t *size, uint32_t timeout_ms);

#endif
