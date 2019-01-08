#include "Board_SPI.h"
const Board_Spi_HardWare_Config_t LCDSpiConfig = 
{
	.Mode = SPI_MODE_MASTER,
	.Direction = SPI_DIRECTION_2LINES,
	.DataSize = SPI_DATASIZE_8BIT,
	.CLKPolarity = SPI_POLARITY_LOW,
	.CLKPhase = SPI_PHASE_1EDGE,
	.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2,//100/2=50M
};
const Board_Spi_HardWare_Config_t WIFISpiConfig = 
{
	.Mode = SPI_MODE_MASTER,
	.Direction = SPI_DIRECTION_2LINES,
	.DataSize = SPI_DATASIZE_8BIT,
	.CLKPolarity = SPI_POLARITY_HIGH,
	.CLKPhase = SPI_PHASE_2EDGE,
	.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4,//400/4=100M
};
Board_Spi_Control_t SpiHandle[SPI_N] = 
{
	{
		.SpiName = SPI_LCD,
		.Instance = SPI4,
		.HardConfig = (Board_Spi_HardWare_Config_t*)&LCDSpiConfig,
	},
	{
		.SpiName = SPI_WIFI,
		.Instance = SPI1,
		.HardConfig = (Board_Spi_HardWare_Config_t*)&WIFISpiConfig,
	}
};
void Board_Spi_Init(Board_Spi_Name_e spiname,void (*MSPInitCallback)(void),void (*MSPDeInitCallback)(void))
{
	SpiHandle[spiname].MSPInitCallback = MSPInitCallback;
	SpiHandle[spiname].MSPDeInitCallback = MSPDeInitCallback;
  SpiHandle[spiname].SpiHalHandle.Instance = SpiHandle[spiname].Instance;
  SpiHandle[spiname].SpiHalHandle.Init.Mode = SpiHandle[spiname].HardConfig->Mode;
  SpiHandle[spiname].SpiHalHandle.Init.Direction = SpiHandle[spiname].HardConfig->Direction;
  SpiHandle[spiname].SpiHalHandle.Init.DataSize = SpiHandle[spiname].HardConfig->DataSize;
  SpiHandle[spiname].SpiHalHandle.Init.CLKPolarity = SpiHandle[spiname].HardConfig->CLKPolarity;
  SpiHandle[spiname].SpiHalHandle.Init.CLKPhase = SpiHandle[spiname].HardConfig->CLKPhase;
  SpiHandle[spiname].SpiHalHandle.Init.NSS = SPI_NSS_SOFT;
  SpiHandle[spiname].SpiHalHandle.Init.BaudRatePrescaler = SpiHandle[spiname].HardConfig->BaudRatePrescaler;
  SpiHandle[spiname].SpiHalHandle.Init.FirstBit = SPI_FIRSTBIT_MSB;
  SpiHandle[spiname].SpiHalHandle.Init.TIMode = SPI_TIMODE_DISABLE;
  SpiHandle[spiname].SpiHalHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  SpiHandle[spiname].SpiHalHandle.Init.CRCPolynomial = 7;
  SpiHandle[spiname].SpiHalHandle.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  SpiHandle[spiname].SpiHalHandle.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  SpiHandle[spiname].SpiHalHandle.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  SpiHandle[spiname].SpiHalHandle.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  SpiHandle[spiname].SpiHalHandle.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  SpiHandle[spiname].SpiHalHandle.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  SpiHandle[spiname].SpiHalHandle.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  SpiHandle[spiname].SpiHalHandle.Init.MasterReceiverAutoSusp = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  SpiHandle[spiname].SpiHalHandle.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  SpiHandle[spiname].SpiHalHandle.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&SpiHandle[spiname].SpiHalHandle) != HAL_OK)
  {
    RTE_Assert(__FILE__, __LINE__);
  }
}
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
	for(Board_Spi_Name_e i = (Board_Spi_Name_e)0;i<SPI_N;i++)
	{
		if(hspi->Instance == SpiHandle[i].Instance)
		{
			SpiHandle[i].MSPInitCallback();
			break;
		}
	}
}
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
	for(Board_Spi_Name_e i = (Board_Spi_Name_e)0;i<SPI_N;i++)
	{
		if(hspi->Instance == SpiHandle[i].Instance)
		{
			SpiHandle[i].MSPDeInitCallback();
			break;
		}
	}
}
