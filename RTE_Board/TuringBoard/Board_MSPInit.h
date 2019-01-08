#ifndef __BOARD_MSPINIT_H
#define __BOARD_MSPINIT_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
extern void Board_Uart1_MSPInitCallback(void);
extern void Board_Uart1_MSPDeInitCallback(void);
extern void Board_Spi4_MSPInitCallback(void);
extern void Board_Spi4_MSPDeInitCallback(void);
extern void Board_Spi1_MSPInitCallback(void);
extern void Board_Spi1_MSPDeInitCallback(void);
extern void Board_Tim1PWM_MSPInitCallback(void);
#endif
