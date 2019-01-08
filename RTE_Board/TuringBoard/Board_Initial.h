#ifndef __BOARD_INITIAL_H
#define __BOARD_INITIAL_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
typedef struct
{
	uint32_t SourceType;
  uint32_t PLLM;
  uint32_t PLLN;
  uint32_t PLLP;
  uint32_t PLLQ;
  uint32_t PLLR;
}Board_Clock_Control_t;
extern void Board_Initial(void);
#endif
