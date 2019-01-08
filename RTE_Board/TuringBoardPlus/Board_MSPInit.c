#include "Board_MSPInit.h"
#include "Board_SDRAM.h"
#include "Board_Uart.h"
void Board_SDRAM_MspInit(void)
{
  GPIO_InitTypeDef gpio_init_structure;
  /* Enable FMC clock */
  __HAL_RCC_FMC_CLK_ENABLE();
  /* Enable chosen MDMAx clock */
  __HAL_RCC_MDMA_CLK_ENABLE();
  /* Enable GPIOs clock */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();

  /* Common GPIO configuration */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF12_FMC;
  /* GPIOD configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8| GPIO_PIN_9 | GPIO_PIN_10 |\
                              GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &gpio_init_structure);
  /* GPIOE configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7| GPIO_PIN_8 | GPIO_PIN_9 |\
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &gpio_init_structure);
  /* GPIOF configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4 |\
                              GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOF, &gpio_init_structure);
  /* GPIOG configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |\
                              GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOG, &gpio_init_structure);
  /* GPIOH configuration */
  gpio_init_structure.Pin   = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |\
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOH, &gpio_init_structure);
  /* GPIOI configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |\
                              GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOI, &gpio_init_structure);
  /* Configure common MDMA parameters */
  SdarmHandle.SdramMdmaHalhandle.Init.Request = MDMA_REQUEST_SW;
  SdarmHandle.SdramMdmaHalhandle.Init.TransferTriggerMode = MDMA_BLOCK_TRANSFER;
  SdarmHandle.SdramMdmaHalhandle.Init.Priority = MDMA_PRIORITY_HIGH;
  SdarmHandle.SdramMdmaHalhandle.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
  SdarmHandle.SdramMdmaHalhandle.Init.SourceInc = MDMA_SRC_INC_WORD;
  SdarmHandle.SdramMdmaHalhandle.Init.DestinationInc = MDMA_DEST_INC_WORD;
  SdarmHandle.SdramMdmaHalhandle.Init.SourceDataSize = MDMA_SRC_DATASIZE_WORD;
  SdarmHandle.SdramMdmaHalhandle.Init.DestDataSize = MDMA_DEST_DATASIZE_WORD;
  SdarmHandle.SdramMdmaHalhandle.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
  SdarmHandle.SdramMdmaHalhandle.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
  SdarmHandle.SdramMdmaHalhandle.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
  SdarmHandle.SdramMdmaHalhandle.Init.BufferTransferLength = 128;
  SdarmHandle.SdramMdmaHalhandle.Init.SourceBlockAddressOffset = 0;
  SdarmHandle.SdramMdmaHalhandle.Init.DestBlockAddressOffset = 0;
  SdarmHandle.SdramMdmaHalhandle.Instance = SDRAM_MDMAx_CHANNEL;
   /* Associate the DMA handle */
  __HAL_LINKDMA(&SdarmHandle.SdramHalHandle, hmdma, SdarmHandle.SdramMdmaHalhandle);
  /* Deinitialize the stream for new transfer */
  HAL_MDMA_DeInit(&SdarmHandle.SdramMdmaHalhandle);
  /* Configure the DMA stream */
  HAL_MDMA_Init(&SdarmHandle.SdramMdmaHalhandle);
  /* NVIC configuration for DMA transfer complete interrupt */
  HAL_NVIC_SetPriority(SDRAM_MDMAx_IRQn, MDMA_PreemptPriority, 0);
  HAL_NVIC_EnableIRQ(SDRAM_MDMAx_IRQn);
}
void Board_SDRAM_MspDeInit(void)
{
	__HAL_RCC_FMC_CLK_DISABLE();
	/* Disable NVIC configuration for DMA interrupt */
	HAL_NVIC_DisableIRQ(SDRAM_MDMAx_IRQn);
	/* Deinitialize the stream for new transfer */
	HAL_MDMA_DeInit(&SdarmHandle.SdramMdmaHalhandle);
}
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

