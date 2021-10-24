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
    com_debug = 0,
    com_N,
} com_name_t;

#endif
