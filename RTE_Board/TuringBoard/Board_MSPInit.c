#include "Board_MSPInit.h"
#include "Board_Uart.h"
#include "Board_SPI.h"
#include "Board_WIFI.h"
void Board_Uart1_MSPInitCallback(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
	/* Peripheral clock enable */
	__HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
	/**USART1 GPIO Configuration    
	PA9     ------> USART1_TX
	PA10     ------> USART1_RX 
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, USART1_DMARX_PreemptPriority, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
#if USART1_USE_DMA_TX == 1
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
#endif
	/* USART1 DMA Init */
	/* USART1_RX Init */
	UartHandle[USART_DEBUG].UsartHalRxDmaHandle.Instance = DMA1_Stream0;
	UartHandle[USART_DEBUG].UsartHalRxDmaHandle.Init.Request = DMA_REQUEST_USART1_RX;
	UartHandle[USART_DEBUG].UsartHalRxDmaHandle.Init.Direction = DMA_PERIPH_TO_MEMORY;
	UartHandle[USART_DEBUG].UsartHalRxDmaHandle.Init.PeriphInc = DMA_PINC_DISABLE;
	UartHandle[USART_DEBUG].UsartHalRxDmaHandle.Init.MemInc = DMA_MINC_ENABLE;
	UartHandle[USART_DEBUG].UsartHalRxDmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	UartHandle[USART_DEBUG].UsartHalRxDmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	UartHandle[USART_DEBUG].UsartHalRxDmaHandle.Init.Mode = DMA_NORMAL;
	UartHandle[USART_DEBUG].UsartHalRxDmaHandle.Init.Priority = DMA_PRIORITY_LOW;
	UartHandle[USART_DEBUG].UsartHalRxDmaHandle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	if (HAL_DMA_Init(&UartHandle[USART_DEBUG].UsartHalRxDmaHandle) != HAL_OK)
	{
		RTE_Assert(__FILE__, __LINE__);
	}

	__HAL_LINKDMA(&UartHandle[USART_DEBUG].UartHalHandle,hdmarx,UartHandle[USART_DEBUG].UsartHalRxDmaHandle);
#if USART1_USE_DMA_TX == 1
	/* USART1_TX Init */
	UartHandle[USART_DEBUG].UsartHalTxDmaHandle.Instance = DMA1_Stream1;
	UartHandle[USART_DEBUG].UsartHalTxDmaHandle.Init.Request = DMA_REQUEST_USART1_TX;
	UartHandle[USART_DEBUG].UsartHalTxDmaHandle.Init.Direction = DMA_MEMORY_TO_PERIPH;
	UartHandle[USART_DEBUG].UsartHalTxDmaHandle.Init.PeriphInc = DMA_PINC_DISABLE;
	UartHandle[USART_DEBUG].UsartHalTxDmaHandle.Init.MemInc = DMA_MINC_ENABLE;
	UartHandle[USART_DEBUG].UsartHalTxDmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	UartHandle[USART_DEBUG].UsartHalTxDmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	UartHandle[USART_DEBUG].UsartHalTxDmaHandle.Init.Mode = DMA_NORMAL;
	UartHandle[USART_DEBUG].UsartHalTxDmaHandle.Init.Priority = DMA_PRIORITY_LOW;
	UartHandle[USART_DEBUG].UsartHalTxDmaHandle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	if (HAL_DMA_Init(&UartHandle[USART_DEBUG].UsartHalTxDmaHandle) != HAL_OK)
	{
		RTE_Assert(__FILE__, __LINE__);
	}

	__HAL_LINKDMA(&UartHandle[USART_DEBUG].UartHalHandle,hdmatx,UartHandle[USART_DEBUG].UsartHalTxDmaHandle);
#endif
	/* USART1 interrupt Init */
	HAL_NVIC_SetPriority(USART1_IRQn, USART1_PreemptPriority, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
}
void Board_Uart1_MSPDeInitCallback(void)
{
	/* Peripheral clock disable */
	__HAL_RCC_USART1_CLK_DISABLE();
	/**USART1 GPIO Configuration    
	PA9     ------> USART1_TX
	PA10     ------> USART1_RX 
	*/
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);
	/* USART1 DMA DeInit */
	HAL_DMA_DeInit(UartHandle[USART_DEBUG].UartHalHandle.hdmarx);
#if USART1_USE_DMA_TX == 1
	HAL_DMA_DeInit(UartHandle[USART_DEBUG].UartHalHandle.hdmatx);
#endif
	/* USART1 interrupt DeInit */
	HAL_NVIC_DisableIRQ(USART1_IRQn);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  for(Board_Uart_Name_e USART_Name=(Board_Uart_Name_e)0;USART_Name<USART_N;USART_Name++) 
	{
		if(huart == &UartHandle[USART_Name].UartHalHandle)
		{
			Board_Uart_RecCallback(USART_Name);
		}
	}
}
#if USART1_USE_DMA_TX == 1
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	
}
#endif
void Board_Spi4_MSPInitCallback(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
	/* Peripheral clock enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_SPI4_CLK_ENABLE();
	/**SPI4 GPIO Configuration    
	PE2     ------> SPI4_SCK
	PE5     ------> SPI4_MISO
	PE14     ------> SPI4_MOSI 
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_5|GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI4;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, SPI4_DMATX_PreemptPriority, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
	/* SPI4 DMA Init */
	/* SPI4_TX Init */
	SpiHandle[SPI_LCD].SpiHalTxDmaHandle.Instance = DMA1_Stream3;
	SpiHandle[SPI_LCD].SpiHalTxDmaHandle.Init.Request = DMA_REQUEST_SPI4_TX;
	SpiHandle[SPI_LCD].SpiHalTxDmaHandle.Init.Direction = DMA_MEMORY_TO_PERIPH;
	SpiHandle[SPI_LCD].SpiHalTxDmaHandle.Init.PeriphInc = DMA_PINC_DISABLE;
	SpiHandle[SPI_LCD].SpiHalTxDmaHandle.Init.MemInc = DMA_MINC_ENABLE;
	SpiHandle[SPI_LCD].SpiHalTxDmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	SpiHandle[SPI_LCD].SpiHalTxDmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	SpiHandle[SPI_LCD].SpiHalTxDmaHandle.Init.Mode = DMA_NORMAL;
	SpiHandle[SPI_LCD].SpiHalTxDmaHandle.Init.Priority = DMA_PRIORITY_HIGH;
	SpiHandle[SPI_LCD].SpiHalTxDmaHandle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	
	if (HAL_DMA_Init(&SpiHandle[SPI_LCD].SpiHalTxDmaHandle) != HAL_OK)
	{
		RTE_Assert(__FILE__, __LINE__);
	}
	__HAL_LINKDMA(&SpiHandle[SPI_LCD].SpiHalHandle,hdmatx,SpiHandle[SPI_LCD].SpiHalTxDmaHandle);
	
	/* SPI4 interrupt Init */
	HAL_NVIC_SetPriority(SPI4_IRQn, SPI4_PreemptPriority, 0);
	HAL_NVIC_EnableIRQ(SPI4_IRQn);
}
void Board_Spi4_MSPDeInitCallback(void)
{
	__HAL_RCC_SPI4_CLK_DISABLE();
	/**SPI4 GPIO Configuration    
	PE2     ------> SPI4_SCK
	PE5     ------> SPI4_MISO
	PE14     ------> SPI4_MOSI 
	*/
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2|GPIO_PIN_5|GPIO_PIN_14);
  /* SPI4 DMA DeInit */
  HAL_DMA_DeInit(&SpiHandle[SPI_LCD].SpiHalTxDmaHandle);
	/* SPI4 interrupt DeInit */
	HAL_NVIC_DisableIRQ(SPI4_IRQn);
}
void Board_Spi1_MSPInitCallback(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
	/* Peripheral clock enable */
	/* Enable GPIO TX/RX clock */
	WIFI_SPI_CLK_ENABLE();
	WIFI_SCK_GPIO_CLK_ENABLE();
	WIFI_MISO_GPIO_CLK_ENABLE();
	WIFI_MOSI_GPIO_CLK_ENABLE();  
  
	/**SPI1 GPIO Configuration    
	PD7     ------> SPI1_MOSI
	PB3     ------> SPI1_SCK
	PB4     ------> SPI1_MISO 
	*/
	GPIO_InitStruct.Pin = WIFI_MOSI_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = WIFI_MOSI_AF;
	HAL_GPIO_Init(WIFI_MOSI_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = WIFI_SCK_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate = WIFI_SCK_AF;
	HAL_GPIO_Init(WIFI_SCK_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = WIFI_MISO_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate = WIFI_MISO_AF;
	HAL_GPIO_Init(WIFI_MISO_GPIO_PORT, &GPIO_InitStruct);
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
	/* SPI1 DMA Init */
	/* SPI1_TX Init */
	SpiHandle[SPI_WIFI].SpiHalTxDmaHandle.Instance = DMA1_Stream4;
	SpiHandle[SPI_WIFI].SpiHalTxDmaHandle.Init.Request = DMA_REQUEST_SPI1_TX;
	SpiHandle[SPI_WIFI].SpiHalTxDmaHandle.Init.Direction = DMA_MEMORY_TO_PERIPH;
	SpiHandle[SPI_WIFI].SpiHalTxDmaHandle.Init.PeriphInc = DMA_PINC_DISABLE;
	SpiHandle[SPI_WIFI].SpiHalTxDmaHandle.Init.MemInc = DMA_MINC_ENABLE;
	SpiHandle[SPI_WIFI].SpiHalTxDmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	SpiHandle[SPI_WIFI].SpiHalTxDmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	SpiHandle[SPI_WIFI].SpiHalTxDmaHandle.Init.Mode = DMA_NORMAL;
	SpiHandle[SPI_WIFI].SpiHalTxDmaHandle.Init.Priority = DMA_PRIORITY_MEDIUM;
	SpiHandle[SPI_WIFI].SpiHalTxDmaHandle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	if (HAL_DMA_Init(&SpiHandle[SPI_WIFI].SpiHalTxDmaHandle) != HAL_OK)
	{
		RTE_Assert(__FILE__, __LINE__);
	}

	__HAL_LINKDMA(&SpiHandle[SPI_WIFI].SpiHalHandle,hdmatx,SpiHandle[SPI_WIFI].SpiHalTxDmaHandle);

	/* SPI1_RX Init */
	SpiHandle[SPI_WIFI].SpiHalRxDmaHandle.Instance = DMA1_Stream5;
	SpiHandle[SPI_WIFI].SpiHalRxDmaHandle.Init.Request = DMA_REQUEST_SPI1_RX;
	SpiHandle[SPI_WIFI].SpiHalRxDmaHandle.Init.Direction = DMA_PERIPH_TO_MEMORY;
	SpiHandle[SPI_WIFI].SpiHalRxDmaHandle.Init.PeriphInc = DMA_PINC_DISABLE;
	SpiHandle[SPI_WIFI].SpiHalRxDmaHandle.Init.MemInc = DMA_MINC_ENABLE;
	SpiHandle[SPI_WIFI].SpiHalRxDmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	SpiHandle[SPI_WIFI].SpiHalRxDmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	SpiHandle[SPI_WIFI].SpiHalRxDmaHandle.Init.Mode = DMA_NORMAL;
	SpiHandle[SPI_WIFI].SpiHalRxDmaHandle.Init.Priority = DMA_PRIORITY_MEDIUM;
	SpiHandle[SPI_WIFI].SpiHalRxDmaHandle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	if (HAL_DMA_Init(&SpiHandle[SPI_WIFI].SpiHalRxDmaHandle) != HAL_OK)
	{
		RTE_Assert(__FILE__, __LINE__);
	}

	__HAL_LINKDMA(&SpiHandle[SPI_WIFI].SpiHalHandle,hdmarx,SpiHandle[SPI_WIFI].SpiHalRxDmaHandle);

  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, WIFI_SPI_DMATX_PreemptPriority, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, WIFI_SPI_DMARX_PreemptPriority, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
		
	HAL_NVIC_SetPriority(SPI1_IRQn, WIFI_SPI_PreemptPriority, 0);
	HAL_NVIC_EnableIRQ(SPI1_IRQn);
}
void Board_Spi1_MSPDeInitCallback(void)
{
	/*##-1- Reset peripherals ##################################################*/
	WIFI_FORCE_RESET();
	WIFI_RELEASE_RESET();
	__HAL_RCC_SPI1_CLK_DISABLE();
	/*##-2- Disable peripherals and GPIO Clocks ################################*/
	/* Deconfigure SPI SCK */
	HAL_GPIO_DeInit(WIFI_SCK_GPIO_PORT, WIFI_SCK_PIN);
	/* Deconfigure SPI MISO */
	HAL_GPIO_DeInit(WIFI_MISO_GPIO_PORT, WIFI_MISO_PIN);
	/* Deconfigure SPI MOSI */
	HAL_GPIO_DeInit(WIFI_MOSI_GPIO_PORT, WIFI_MOSI_PIN);
	/* SPI1 DMA DeInit */
	HAL_DMA_DeInit(SpiHandle[SPI_WIFI].SpiHalHandle.hdmatx);
	HAL_DMA_DeInit(SpiHandle[SPI_WIFI].SpiHalHandle.hdmarx);
	/*##-3- Disable the NVIC for SPI ###########################################*/
	HAL_NVIC_DisableIRQ(SPI1_IRQn);
}
void Board_Tim1PWM_MSPInitCallback(void)
{
  GPIO_InitTypeDef   GPIO_InitStruct;
  /* TIMx Peripheral clock enable */
  TIMx_CLK_ENABLE();
  /* Enable GPIO Channels Clock */
  TIMx_GPIO_CLK_ENABLE();
  /* Common configuration for all channels */
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  GPIO_InitStruct.Alternate = TIMx_AF;
  GPIO_InitStruct.Pin = TIMx_PIN;
  HAL_GPIO_Init(TIMx_PORT, &GPIO_InitStruct);
}
