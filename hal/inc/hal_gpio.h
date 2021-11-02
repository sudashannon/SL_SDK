/**
 * @file hal_gpio.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-04
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __HAL_GPIO_H
#define __HAL_GPIO_H
#include "stm32h7xx.h"
#include "hal.h"
typedef enum
{
	GPIO_RUN = 0,
	LCD_BLK = 1,
	LCD_CS = 2,
	LCD_DC = 3,
	CAM_LIGHT =4,
	CAM_RST = 5,
	CAM_PWR = 6,
	GPIO_N,
} gpio_name_t;

typedef uint8_t gpio_state_t;
#define	GPIO_OFF			(gpio_state_t)0
#define	GPIO_ON				(gpio_state_t)1

typedef enum {
	GPIO_UP_IN = 0,
	GPIO_DOWN_IN,
	GPIO_UP_OUT,
	GPIO_DOWN_OUT,
} gpio_mode_t;
/**
 * @brief Set a gpio to be low.
 *
 * @param gpio_name
 */
extern void gpio_set_low(gpio_name_t gpio_name);
/**
 * @brief Set a gpio to be high.
 *
 * @param gpio_name
 */
extern void gpio_set_high(gpio_name_t gpio_name);
/**
 * @brief Toggle a gpio.
 *
 * @param gpio_name
 */
extern void gpio_toggle(gpio_name_t gpio_name);
/**
 * @brief Init a gpio.
 *
 * @param gpio_name
 */
extern void gpio_init(gpio_name_t gpio_name, gpio_mode_t mode);
/**
 * @brief Read a gpio value.
 *
 * @param gpio_name
 * @return gpio_state_t
 */
extern gpio_state_t gpio_read_value(gpio_name_t gpio_name);
/**
 * @brief Change a gpio's mode to a new mode.
 *
 * @param gpio_name
 * @return gpio_mode_t
 */
extern void gpio_change_mode(gpio_name_t gpio_name, gpio_mode_t new_mode);

#endif
