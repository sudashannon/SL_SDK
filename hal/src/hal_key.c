/**
 * @file bsp_key.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2020-11-15
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "../inc/hal_gpio.h"
#include "../inc/hal_key.h"

/**
 * @brief Internal key timer callback;
 *
 * @param param
 */
static void key_handle_timer(void* param)
{
    key_handle_t *key = (key_handle_t *)param;
    switch (key->state_machine) {
        case KEY_SM_POLL: {
            if (key_read_io(key)) {
                key->state_machine = KEY_SM_PRESS;
#if KEY_LONGPRESS_ENABLE
                key->longpress_start_tick = rte_get_tick();
#endif
            }
        } break;
        case KEY_SM_PRESS: {
            if (key_read_io(key)) {
                key->fifo.buffer[key->fifo.write_point] = KEY_PRESS;
                key->fifo.write_point++;
                key->state_machine = KEY_SM_RELEASE;
                if(key->fifo.write_point == KEY_FIFO_SIZE) {
                    key->fifo.write_point = 0;
                }
            } else {
                key->state_machine = KEY_SM_POLL;
            }
        } break;
        case KEY_SM_RELEASE: {
            if(!key_read_io(key)) {
                key->fifo.buffer[key->fifo.write_point] = KEY_RELEASE;
                key->fifo.write_point++;
                key->state_machine = KEY_SM_POLL;
                if(key->fifo.write_point == KEY_FIFO_SIZE) {
                    key->fifo.write_point = 0;
                }
            }
#if KEY_LONGPRESS_ENABLE
            else if(key->longpress_start_tick + KEY_LONGPRESS_TIME <= rte_get_tick()) {
                key->fifo.buffer[key->fifo.write_point] = KEY_LONG;
                key->fifo.write_point++;
                if(key->fifo.write_point == KEY_FIFO_SIZE) {
                    key->fifo.write_point = 0;
                }
                key->longpress_start_tick = rte_get_tick();
            }
#endif
        } break;
    }
}
/**
 * @brief Create a key instance via a selected gpio.
 *
 * @param gpio_name
 * @param pressed_io
 * @param key_handle
 * @return rte_error_t
 */
rte_error_t key_create(gpio_name_t gpio_name, uint8_t pressed_value, key_handle_t **key_handle)
{
    if (pressed_value == GPIO_OFF) {
        gpio_init(gpio_name, GPIO_UP_IN);
    } else if(pressed_value == GPIO_ON) {
        gpio_init(gpio_name, GPIO_DOWN_IN);
    } else {
        return RTE_ERR_PARAM;
    }

    key_handle_t *obj = rte_calloc(sizeof(key_handle_t));
    if (!obj)
        return RTE_ERR_NO_MEM;
    obj->pressed_value = pressed_value;
    obj->gpio_name = gpio_name;
    obj->fifo.write_point = 0;
    obj->fifo.read_point = 0;
    obj->state_machine = KEY_SM_POLL;
    timer_configuration_t config = TIMER_CONFIG_INITIALIZER;
    timer_id_t running_timer_id = 0;
    config.repeat_period_ms = KEY_FILTER_TIME;
    config.timer_callback = key_handle_timer;
    config.parameter = obj;
    rte_error_t result = timer_create_new(rte_get_main_timergroup(), &config, &running_timer_id);
    if (result != RTE_SUCCESS) {
        obj->timer_index = running_timer_id;
    } else {
        key_destroy(&obj);
        return RTE_ERR_UNDEFINE;
    }
    *key_handle = obj;
    return RTE_SUCCESS;
}
/**
 * @brief Destroy a key instance.
 *
 * @param key_handle
 * @return rte_error_t
 */
rte_error_t key_destroy(key_handle_t **key_handle)
{
    if (!*key_handle)
        return RTE_ERR_PARAM;
    timer_delete(rte_get_main_timergroup(), (*key_handle)->timer_index);
    rte_free(*key_handle);
    *key_handle = NULL;
    return RTE_SUCCESS;
}
/**
 * @brief
 *
 * @param key
 * @return true
 * @return false
 */
bool key_read_io(key_handle_t *key)
{
    return (gpio_read_value(key->gpio_name) ==key->pressed_value);
}
/**
 * @brief
 *
 * @param key
 * @return key_value_t
 */
key_value_t key_read_value(key_handle_t *key)
{
    if (key->fifo.write_point == key->fifo.read_point) {
        return KEY_NONE;
    } else {
        key_value_t ret = key->fifo.buffer[key->fifo.read_point];
        key->fifo.read_point++;
        if (key->fifo.read_point >= KEY_FIFO_SIZE) {
            key->fifo.read_point = 0;
        }
        return ret;
    }
}

rte_error_t key_change_mode(key_handle_t *key, uint8_t pressed_value)
{
    if (pressed_value == GPIO_OFF) {
        gpio_change_mode(key->gpio_name, GPIO_UP_IN);
    } else if(pressed_value == GPIO_ON) {
        gpio_change_mode(key->gpio_name, GPIO_DOWN_IN);
    }
    key->pressed_value = pressed_value;
    return RTE_SUCCESS;
}
