/**
 * @file hal_gpio_stm32h7.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-07
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "../inc/hal_gpio.h"
/**
 * @brief Define a gpio handle.
 *
 */
typedef struct {
    gpio_name_t name;    // Name
    GPIO_TypeDef* port; // Port
    uint16_t pin; // Pin
} gpio_t;
/**
 * @brief All gpio will be handled.
 *
 */
static gpio_t gpio_table[GPIO_N] = {
    // Name    ,PORT , PIN
    {GPIO_RUN, GPIOA, GPIO_PIN_1},
    {LCD_BLK, GPIOD, GPIO_PIN_1},
    {LCD_CS, GPIOB, GPIO_PIN_12},
    {LCD_DC, GPIOB, GPIO_PIN_1},
    {LCD_RST, GPIOD, GPIO_PIN_14},
    {CAM_LIGHT, GPIOA, GPIO_PIN_5},
    {CAM_RST, GPIOC, GPIO_PIN_4},
    {CAM_PWR, GPIOA, GPIO_PIN_7},
};
/**
 * @brief Init a gpio.
 *
 * @param gpio_name
 */
void gpio_init(gpio_name_t gpio_name, gpio_mode_t mode)
{
    RTE_UNUSED(gpio_name);
    RTE_UNUSED(mode);
}
/**
 * @brief Set a gpio to be low.
 *
 * @param gpio_name
 */
void gpio_set_low(gpio_name_t gpio_name)
{
    HAL_GPIO_WritePin(gpio_table[gpio_name].port, gpio_table[gpio_name].pin, GPIO_PIN_RESET);
}
/**
 * @brief Set a gpio to be high.
 *
 * @param gpio_name
 */
void gpio_set_high(gpio_name_t gpio_name)
{
    HAL_GPIO_WritePin(gpio_table[gpio_name].port, gpio_table[gpio_name].pin, GPIO_PIN_SET);
}
/**
 * @brief Toggle a gpio.
 *
 * @param gpio_name
 */
void gpio_toggle(gpio_name_t gpio_name)
{
    HAL_GPIO_TogglePin(gpio_table[gpio_name].port, gpio_table[gpio_name].pin);
}

gpio_state_t gpio_read_value(gpio_name_t gpio_name)
{
    return(HAL_GPIO_ReadPin(gpio_table[gpio_name].port, gpio_table[gpio_name].pin));
}

void gpio_change_mode(gpio_name_t gpio_name, gpio_mode_t new_mode)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    // Config als Digital-Ausgang
    GPIO_InitStructure.Pin = gpio_table[gpio_name].pin;
    switch(new_mode) {
        case GPIO_UP_IN: {
            GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
            GPIO_InitStructure.Pull = GPIO_PULLUP;
            break;
        }
        case GPIO_UP_OUT: {
            GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStructure.Pull = GPIO_PULLUP;
            break;
        }
        case GPIO_DOWN_IN: {
            GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
            GPIO_InitStructure.Pull = GPIO_PULLDOWN;
            break;
        }
        case GPIO_DOWN_OUT: {
            GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStructure.Pull = GPIO_PULLDOWN;
            break;
        }
    }
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(gpio_table[gpio_name].port, &GPIO_InitStructure);
}
