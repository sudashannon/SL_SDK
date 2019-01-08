#ifndef __BOARD_SPI_H
#define __BOARD_SPI_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
typedef enum
{
	SPI_LCD = 0,
	SPI_WIFI = 1,
	SPI_N,
}Board_Spi_Name_e;
typedef struct
{
	uint32_t Mode;
	uint32_t Direction;
	uint32_t DataSize;
	uint32_t CLKPolarity;
	uint32_t CLKPhase;
	uint32_t BaudRatePrescaler;
}Board_Spi_HardWare_Config_t;
typedef struct
{
	Board_Spi_Name_e SpiName;
	SPI_TypeDef                *Instance;
	Board_Spi_HardWare_Config_t *HardConfig;
	SPI_HandleTypeDef SpiHalHandle;
  DMA_HandleTypeDef SpiHalRxDmaHandle;
  DMA_HandleTypeDef SpiHalTxDmaHandle;
	void (*MSPInitCallback)(void);	   
	void (*MSPDeInitCallback)(void);	 
}Board_Spi_Control_t;
extern Board_Spi_Control_t SpiHandle[SPI_N];
extern void Board_Spi_Init(Board_Spi_Name_e spiname,void (*MSPInitCallback)(void),void (*MSPDeInitCallback)(void));
#endif
