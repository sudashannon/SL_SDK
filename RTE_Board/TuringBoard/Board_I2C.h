#ifndef __BOARD_I2C_H
#define __BOARD_I2C_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
extern I2C_HandleTypeDef BoardI2C;
extern void Board_I2C_Init(void);
#endif
