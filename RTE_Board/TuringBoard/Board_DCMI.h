#ifndef __BOARD_DCMI_H
#define __BOARD_DCMI_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
#define DCMI_HSYNC_PIN          (GPIO_PIN_4)
#define DCMI_VSYNC_PIN          (GPIO_PIN_7)
#define DCMI_PXCLK_PIN          (GPIO_PIN_6)
#define DCMI_HSYNC_PORT         (GPIOA)
#define DCMI_VSYNC_PORT         (GPIOB)
#define DCMI_PXCLK_PORT         (GPIOA)
extern DCMI_HandleTypeDef BoardDcmi;
extern DMA_HandleTypeDef BoardDcmiDma;
void Board_DCMI_Init(void);
#endif
