#ifndef __BOARD_TOUCH_H
#define __BOARD_TOUCH_H
#include "stm32f4xx.h"
#include "RTE_Include.h"   
#define TOUCH_SPI                           SPI2
#define TOUCH_SPI_CLK                       RCC_APB1Periph_SPI2
#define TOUCH_SPI_CLK_INIT                  RCC_APB1PeriphClockCmd

#define TOUCH_SPI_SCK_PIN                   GPIO_Pin_13
#define TOUCH_SPI_SCK_GPIO_PORT             GPIOB
#define TOUCH_SPI_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOB
#define TOUCH_SPI_SCK_SOURCE                GPIO_PinSource13
#define TOUCH_SPI_SCK_AF                    GPIO_AF_SPI2

#define TOUCH_SPI_MISO_PIN                  GPIO_Pin_14
#define TOUCH_SPI_MISO_GPIO_PORT            GPIOB
#define TOUCH_SPI_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define TOUCH_SPI_MISO_SOURCE               GPIO_PinSource15
#define TOUCH_SPI_MISO_AF                   GPIO_AF_SPI2

#define TOUCH_SPI_MOSI_PIN                  GPIO_Pin_15
#define TOUCH_SPI_MOSI_GPIO_PORT            GPIOB
#define TOUCH_SPI_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define TOUCH_SPI_MOSI_SOURCE               GPIO_PinSource15
#define TOUCH_SPI_MOSI_AF                   GPIO_AF_SPI2

#define TOUCH_CS_PIN                        GPIO_Pin_12
#define TOUCH_CS_GPIO_PORT                  GPIOB
#define TOUCH_CS_GPIO_CLK                   RCC_AHB1Periph_GPIOB

#define TOUCH_CS_LOW()       GPIO_ResetBits(TOUCH_CS_GPIO_PORT, TOUCH_CS_PIN)
#define TOUCH_CS_HIGH()      GPIO_SetBits(TOUCH_CS_GPIO_PORT, TOUCH_CS_PIN)   


extern void Board_Touch_Init(void);
extern bool Board_TouchScan(uint16_t *x,uint16_t *y);
#if RTE_USE_GUI
extern bool GUI_TouchScan(lv_indev_data_t *data);
#endif
#endif
