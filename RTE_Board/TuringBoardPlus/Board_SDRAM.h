#ifndef __BOARD_SDRAM_H
#define __BOARD_SDRAM_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
/**
  * @brief  FMC SDRAM Mode definition register defines
  */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)
typedef struct
{
	MDMA_HandleTypeDef SdramMdmaHalhandle;
	FMC_SDRAM_CommandTypeDef SdramCommandHandle;
	SDRAM_HandleTypeDef SdramHalHandle;
	void (*MSPInitCallback)(void);	   
	void (*MSPDeInitCallback)(void);	 
}Board_Sdram_Control_t;
extern Board_Sdram_Control_t SdarmHandle;
void Board_SDRAM_Init(void (*MSPInitCallback)(void),void (*MSPDeInitCallback)(void));
void Board_SDRAM_DeInit(void);
void    Board_SDRAM_Initialization_sequence(uint32_t RefreshCount);
#endif
