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
#include "stm32h7xx.h"
#include "hal.h"
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
/**
 * @brief List all provided com instance's name.
 *
 */
typedef enum {
    com_debug = 0,
    com_N,
} com_name_t;

#endif
