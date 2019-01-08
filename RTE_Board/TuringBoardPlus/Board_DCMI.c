#include "Board_DCMI.h"
DCMI_HandleTypeDef BoardDcmi;
DMA_HandleTypeDef BoardDcmiDma;
void Board_DCMI_Init(void)
{
  BoardDcmi.Instance = DCMI;
  BoardDcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
  BoardDcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
  BoardDcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
  BoardDcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
  BoardDcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
  BoardDcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  BoardDcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
  BoardDcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
  BoardDcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
  BoardDcmi.Init.LineSelectMode = DCMI_LSM_ALL;
  BoardDcmi.Init.LineSelectStart = DCMI_OELS_ODD;
  if (HAL_DCMI_Init(&BoardDcmi) != HAL_OK)
  {
    RTE_Assert(__FILE__, __LINE__);
  }

}
void HAL_DCMI_MspInit(DCMI_HandleTypeDef* dcmiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(dcmiHandle->Instance==DCMI)
  {
  /* USER CODE BEGIN DCMI_MspInit 0 */

  /* USER CODE END DCMI_MspInit 0 */
    /* DCMI clock enable */
    __HAL_RCC_DCMI_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();
		__HAL_RCC_GPIOG_CLK_ENABLE();
    /**DCMI GPIO Configuration    
    PC11     ------> DCMI_D4
    PB9     ------> DCMI_D7
    PA4     ------> DCMI_HSYNC
    PA6     ------> DCMI_PIXCLK
    PC6     ------> DCMI_D0
    PC7     ------> DCMI_D1
    PD3     ------> DCMI_D5
    PG9     ------> DCMI_VSYNC
    PB8     ------> DCMI_D6
    PC8     ------> DCMI_D2
    PC9     ------> DCMI_D3 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_I2CEx_EnableFastModePlus(SYSCFG_PMCR_I2C_PB8_FMP);
		
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
		
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
		
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

		/* DMA controller clock enable */
		__HAL_RCC_DMA1_CLK_ENABLE();
    /* DCMI DMA Init */
    /* DCMI Init */
    BoardDcmiDma.Instance = DMA1_Stream2;
    BoardDcmiDma.Init.Request = DMA_REQUEST_DCMI;
    BoardDcmiDma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    BoardDcmiDma.Init.PeriphInc = DMA_PINC_DISABLE;
    BoardDcmiDma.Init.MemInc = DMA_MINC_ENABLE;
    BoardDcmiDma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    BoardDcmiDma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    BoardDcmiDma.Init.Mode = DMA_NORMAL;
    BoardDcmiDma.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    BoardDcmiDma.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    BoardDcmiDma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    BoardDcmiDma.Init.MemBurst = DMA_MBURST_SINGLE;
    BoardDcmiDma.Init.PeriphBurst = DMA_MBURST_SINGLE;
    if (HAL_DMA_Init(&BoardDcmiDma) != HAL_OK)
    {
      RTE_Assert(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(dcmiHandle,DMA_Handle,BoardDcmiDma);

		/* DMA interrupt init */
		/* DMA2_Stream1_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, DCMI_PreemptPriority, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
    /* DCMI interrupt Init */
    HAL_NVIC_SetPriority(DCMI_IRQn, DCMI_PreemptPriority, 0);
    HAL_NVIC_EnableIRQ(DCMI_IRQn);

  /* USER CODE BEGIN DCMI_MspInit 1 */

  /* USER CODE END DCMI_MspInit 1 */
  }
}

void HAL_DCMI_MspDeInit(DCMI_HandleTypeDef* dcmiHandle)
{

  if(dcmiHandle->Instance==DCMI)
  {
    /* Peripheral clock disable */
    __HAL_RCC_DCMI_CLK_DISABLE();
  
    /**DCMI GPIO Configuration    
    PE4     ------> DCMI_D4
    PE6     ------> DCMI_D7
    PA4     ------> DCMI_HSYNC
    PA6     ------> DCMI_PIXCLK
    PC6     ------> DCMI_D0
    PC7     ------> DCMI_D1
    PD3     ------> DCMI_D5
    PG9     ------> DCMI_VSYNC
    PB8     ------> DCMI_D6
    PE0     ------> DCMI_D2
    PE1     ------> DCMI_D3 
    */


    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4|GPIO_PIN_6);
		
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);
		
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_11);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_3);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_9);

    /* DCMI DMA DeInit */
    HAL_DMA_DeInit(dcmiHandle->DMA_Handle);

    /* DCMI interrupt Deinit */
    HAL_NVIC_DisableIRQ(DCMI_IRQn);
  }
} 
