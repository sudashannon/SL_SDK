/**
 * @file hal_key.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2020-11-15
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef __HAL_KEY_H
#define __HAL_KEY_H

#include "stm32h7xx.h"
#include "hal.h"
#include "hal_gpio.h"

/**
 * @brief Module configuration.
 *
 */
#define KEY_FIFO_SIZE               16
#define KEY_FILTER_TIME             25
#define KEY_LONGPRESS_ENABLE        0
#if KEY_LONGPRESS_ENABLE
#define KEY_LONGPRESS_TIME          500
#endif
/**
 * @brief All value of a key instance.
 *
 */
typedef uint8_t key_value_t;
#define KEY_NONE                    (key_value_t)0
#define KEY_PRESS                   (key_value_t)1
#define KEY_RELEASE                 (key_value_t)2
#if KEY_LONGPRESS_ENABLE
#define KEY_LONG                    (key_value_t)3
#endif
/**
 * @brief All state of a key instace's state machine.
 *
 */
typedef uint8_t key_state_t;
#define KEY_SM_POLL                 (key_state_t)0
#define KEY_SM_PRESS                (key_state_t)1
#define KEY_SM_RELEASE              (key_state_t)2
/**
 * @brief Fifo to store a key instace's state.
 *
 */
typedef struct {
    key_value_t buffer[KEY_FIFO_SIZE];
    uint8_t write_point;
    uint8_t read_point;
} key_fifo_t;
/**
 * @brief Key instance's define.
 *
 */
typedef struct {
    uint8_t gpio_name;
    uint8_t pressed_value;
#if KEY_LONGPRESS_ENABLE
    uint32_t longpress_start_tick;
#endif
    key_state_t state_machine;
    key_fifo_t fifo;
    timer_impl_t *timer;
} key_handle_t;

rte_error_t key_create(gpio_name_t gpio_name, uint8_t pressed_value, key_handle_t **key_handle);
rte_error_t key_destroy(key_handle_t **key_handle);
bool key_read_io(key_handle_t *key);
key_value_t key_read_value(key_handle_t *key);
rte_error_t key_change_mode(key_handle_t *key, uint8_t pressed_value);

#endif
