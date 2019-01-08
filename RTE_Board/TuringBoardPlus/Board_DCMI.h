#ifndef __BOARD_DCMI_H
#define __BOARD_DCMI_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
extern DCMI_HandleTypeDef BoardDcmi;
extern DMA_HandleTypeDef BoardDcmiDma;
void Board_DCMI_Init(void);
#endif
