/**
 * @file bsp_gpio.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2020-10-08
 *
 * @copyright Copyright (c) 2020
 *
 */
#include "../../inc/bsp_gpio.h"
#include "sl_include.h"
/**
 * @brief Define a gpio handle.
 *
 */
typedef struct {
    gpio_name_t name;    // Name
    GPIO_TypeDef* port; // Port
    uint16_t pin; // Pin
    uint32_t gpio_clock; // Clock
    gpio_state_t initial_state;  // Init
} gpio_t;
/**
 * @brief All gpio will be handled.
 *
 */
static gpio_t gpio_table[GPIO_N] = {
    // Name    ,PORT , PIN ,                              Init
    {GPIO_CH1_1, GPIOA, GPIO_Pin_2,	 RCC_AHBPeriph_GPIOA, GPIO_OFF},
    {GPIO_CH1_2, GPIOA, GPIO_Pin_3,	 RCC_AHBPeriph_GPIOA, GPIO_OFF},
    {GPIO_CH2_1, GPIOA, GPIO_Pin_4,	 RCC_AHBPeriph_GPIOA, GPIO_OFF},
    {GPIO_CH2_2, GPIOA, GPIO_Pin_5,	 RCC_AHBPeriph_GPIOA, GPIO_OFF},
    {GPIO_CH3_1, GPIOA, GPIO_Pin_6,	 RCC_AHBPeriph_GPIOA, GPIO_OFF},
    {GPIO_CH3_2, GPIOA, GPIO_Pin_7,	 RCC_AHBPeriph_GPIOA, GPIO_OFF},
    {GPIO_CH4_1, GPIOB, GPIO_Pin_0,	 RCC_AHBPeriph_GPIOB, GPIO_OFF},
    {GPIO_CH4_2, GPIOB, GPIO_Pin_1,	 RCC_AHBPeriph_GPIOB, GPIO_OFF},
    {GPIO_RUN,   GPIOF, GPIO_Pin_1,	 RCC_AHBPeriph_GPIOF, GPIO_OFF},
    {GPIO_BEEP,  GPIOF, GPIO_Pin_0,	 RCC_AHBPeriph_GPIOF, GPIO_OFF},
    {GPIO_IN_MODE, GPIOA, GPIO_Pin_0, RCC_AHBPeriph_GPIOA, GPIO_OFF},
    {GPIO_OUT_MODE, GPIOA, GPIO_Pin_1, RCC_AHBPeriph_GPIOA, GPIO_OFF},
    {GPIO_KEY_1, GPIOA, GPIO_Pin_15, RCC_AHBPeriph_GPIOA, GPIO_OFF},
    {GPIO_KEY_2, GPIOA, GPIO_Pin_12, RCC_AHBPeriph_GPIOA, GPIO_OFF},
    {GPIO_KEY_3, GPIOA, GPIO_Pin_11, RCC_AHBPeriph_GPIOA, GPIO_OFF},
    {GPIO_KEY_4, GPIOA, GPIO_Pin_8,	 RCC_AHBPeriph_GPIOA, GPIO_OFF},
    {GPIO_CODE_1, GPIOB, GPIO_Pin_3, RCC_AHBPeriph_GPIOB, GPIO_OFF},
    {GPIO_CODE_2, GPIOB, GPIO_Pin_4, RCC_AHBPeriph_GPIOB, GPIO_OFF},
    {GPIO_CODE_3, GPIOB, GPIO_Pin_5, RCC_AHBPeriph_GPIOB, GPIO_OFF},
    {GPIO_CODE_4, GPIOB, GPIO_Pin_6, RCC_AHBPeriph_GPIOB, GPIO_OFF},
    {GPIO_CODE_5, GPIOB, GPIO_Pin_7, RCC_AHBPeriph_GPIOB, GPIO_OFF},
    {GPIO_CH1_ON, GPIOB, GPIO_Pin_7, RCC_AHBPeriph_GPIOB, GPIO_OFF},
    {GPIO_CH1_OFF, GPIOB, GPIO_Pin_6, RCC_AHBPeriph_GPIOB, GPIO_OFF},
};
/**
 * @brief Set a gpio to be low.
 *
 * @param gpio_name
 */
void gpio_set_low(gpio_name_t gpio_name)
{
    gpio_table[gpio_name].port->BRR = gpio_table[gpio_name].pin;
}
/**
 * @brief Set a gpio to be high.
 *
 * @param gpio_name
 */
void gpio_set_high(gpio_name_t gpio_name)
{
    gpio_table[gpio_name].port->BSRR = gpio_table[gpio_name].pin;
}
/**
 * @brief Toggle a gpio.
 *
 * @param gpio_name
 */
void gpio_toggle(gpio_name_t gpio_name)
{
    gpio_table[gpio_name].port->ODR ^= gpio_table[gpio_name].pin;
}
/**
 * @brief Init a gpio.
 *
 * @param gpio_name
 */
void gpio_init(gpio_name_t gpio_name, gpio_mode_t mode)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    // Clock Enable
    RCC_AHBPeriphClockCmd(gpio_table[gpio_name].gpio_clock, ENABLE);
    // Config als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = gpio_table[gpio_name].pin;
    switch(mode) {
        case GPIO_UP_IN: {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
            break;
        }
        case GPIO_UP_OUT: {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
            break;
        }
        case GPIO_DOWN_IN: {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
            break;
        }
        case GPIO_DOWN_OUT: {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
            break;
        }
    }
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(gpio_table[gpio_name].port, &GPIO_InitStructure);
    if(mode == GPIO_UP_OUT) {
        if(gpio_table[gpio_name].initial_state == GPIO_ON)
            gpio_set_low(gpio_name);
        else
            gpio_set_high(gpio_name);
    } else if(mode == GPIO_DOWN_OUT) {
        if(gpio_table[gpio_name].initial_state == GPIO_ON)
            gpio_set_high(gpio_name);
        else
            gpio_set_low(gpio_name);
    }
}

gpio_state_t gpio_read_value(gpio_name_t gpio_name)
{
    return(GPIO_ReadInputDataBit(gpio_table[gpio_name].port, gpio_table[gpio_name].pin));
}

void gpio_change_mode(gpio_name_t gpio_name, gpio_mode_t new_mode)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    // Config als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = gpio_table[gpio_name].pin;
    switch(new_mode) {
        case GPIO_UP_IN: {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
            break;
        }
        case GPIO_UP_OUT: {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
            break;
        }
        case GPIO_DOWN_IN: {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
            break;
        }
        case GPIO_DOWN_OUT: {
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
            GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
            break;
        }
    }
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(gpio_table[gpio_name].port, &GPIO_InitStructure);
}