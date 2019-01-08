#ifndef __BOARD_MSPINIT_H
#define __BOARD_MSPINIT_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
extern void Board_SDRAM_MspInit(void);
extern void Board_SDRAM_MspDeInit(void);
extern void Board_Uart1_MSPInitCallback(void);
extern void Board_Uart1_MSPDeInitCallback(void);
extern void Board_Tim1PWM_MSPInitCallback(void);
#endif
