#include "Board_SDCard.h"
#include "Board_FatFS.h"
SD_HandleTypeDef BoardSDcard;
osEventFlagsId_t EvtIDGUISD;
void Board_SD_Init(void)
{
	//初始化时的时钟不能大于400KHZ 
  GPIO_InitTypeDef GPIO_InitStruct;		
	SD_CD_GPIO_CLK_ENABLE();
	/**SDMMCx GPIO Configuration **/
	GPIO_InitStruct.Pin = SD_CD_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(SD_CD_GPIO_Port, &GPIO_InitStruct);
	if(Board_SD_IsDetected()==0)
	{
		//时钟为400M（PLL1）/(3+1) = 100M
		BoardSDcard.Instance = SDMMCx;
		BoardSDcard.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
		BoardSDcard.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
		BoardSDcard.Init.BusWide = SDMMC_BUS_WIDE_4B;
		BoardSDcard.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
		BoardSDcard.Init.ClockDiv = SDMMC_NSpeed_CLK_DIV;
			/* Configure SD Bus width (4 bits mode selected) */
		if (HAL_SD_Init(&BoardSDcard) != HAL_OK)
		{
			RTE_Assert(__FILE__, __LINE__);
		}
		HAL_SD_CardInfoTypeDef  SDCardInfo;					//SD卡信息结构体
		HAL_SD_GetCardInfo(&BoardSDcard,&SDCardInfo);
		
#if RTE_USE_OS
		EvtIDGUISD = osEventFlagsNew(NULL);
#endif
	}
	else
		RTE_Printf("%10s    No SDCard!\r\n","[SDCARD]");
		return;
}
uint8_t Board_SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
	uint8_t retval = 0x01;
	__disable_irq();
  if (HAL_SD_ReadBlocks(&BoardSDcard, (uint8_t *)pData, ReadAddr, NumOfBlocks,HAL_MAX_DELAY) == HAL_OK)
  {
		while(Board_SD_GetCardState()!=0)
    {
	
    }
		retval = 0x00;  
  }
	else
		retval = 0x01;
	__enable_irq();
	return retval;
}
uint8_t Board_SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
	uint8_t retval = 0x01;
	__disable_irq();
  if (HAL_SD_WriteBlocks(&BoardSDcard, (uint8_t *)pData, WriteAddr, NumOfBlocks,HAL_MAX_DELAY) == HAL_OK)
  {
		while(Board_SD_GetCardState()!=0)
    {
	
    }
		retval = 0x00;  
  }
	else
		retval = 0x01;
	__enable_irq();
	return retval; 
}
uint8_t Board_SD_GetCardState(void)
{
  return ((HAL_SD_GetCardState(&BoardSDcard) == HAL_SD_CARD_TRANSFER ) ? 0x00 : 0x01);
}
void Board_SD_GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo)
{
  /* Get SD card Information */
  HAL_SD_GetCardInfo(&BoardSDcard, CardInfo);
}
uint8_t	Board_SD_IsDetected(void) {
  uint8_t status = (uint8_t)0x01;
  /* Check SD card detect pin */
  if (HAL_GPIO_ReadPin(SD_CD_GPIO_Port,SD_CD_Pin) == GPIO_PIN_RESET) {
    status = (uint8_t)0x00;
  }
  /* USER CODE BEGIN 1 */
  /* user code can be inserted here */
  /* USER CODE END 1 */ 
  return status;
}  

void HAL_SD_MspInit(SD_HandleTypeDef* sdHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if(sdHandle->Instance==SDMMCx)
  {
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
	  SDMMCx_D0_GPIO_CLK_ENABLE();
    SDMMCx_D1_GPIO_CLK_ENABLE();
		SDMMCx_D2_GPIO_CLK_ENABLE();
		SDMMCx_D3_GPIO_CLK_ENABLE();
		SDMMCx_CK_GPIO_CLK_ENABLE();
		SDMMCx_CMD_GPIO_CLK_ENABLE();
		/* Enable SDMMC clock */
		SDMMCx_CLK_ENABLE();

		
    GPIO_InitStruct.Pin = SDMMCx_D0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = SDMMCx_D0_AF;
    HAL_GPIO_Init(SDMMCx_D0_GPIO_Port, &GPIO_InitStruct);
		
		GPIO_InitStruct.Pin = SDMMCx_D1_Pin;
		GPIO_InitStruct.Alternate = SDMMCx_D1_AF;
		HAL_GPIO_Init(SDMMCx_D1_GPIO_Port, &GPIO_InitStruct);
		GPIO_InitStruct.Pin = SDMMCx_D2_Pin;
		GPIO_InitStruct.Alternate = SDMMCx_D2_AF;
		HAL_GPIO_Init(SDMMCx_D2_GPIO_Port, &GPIO_InitStruct);
		GPIO_InitStruct.Pin = SDMMCx_D3_Pin;
		GPIO_InitStruct.Alternate = SDMMCx_D3_AF;
		HAL_GPIO_Init(SDMMCx_D3_GPIO_Port, &GPIO_InitStruct);
		GPIO_InitStruct.Pin = SDMMCx_CK_Pin;
		GPIO_InitStruct.Alternate = SDMMCx_CK_AF;
		HAL_GPIO_Init(SDMMCx_CK_GPIO_Port, &GPIO_InitStruct);
		GPIO_InitStruct.Pin = SDMMCx_CMD_Pin;
		GPIO_InitStruct.Alternate = SDMMCx_CMD_AF;
		HAL_GPIO_Init(SDMMCx_CMD_GPIO_Port, &GPIO_InitStruct);
#if SDMMCx_INTERRUPT_USE == 1
    HAL_NVIC_SetPriority(SDMMCx_IRQn, SDMMCx_PreemptPriority, 0);
    HAL_NVIC_EnableIRQ(SDMMCx_IRQn);
#endif
  }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef* sdHandle)
{

  if(sdHandle->Instance==BoardSDcard.Instance)
  {
    /*##-1- Reset peripherals ##################################################*/
    SDMMCx_FORCE_RESET();
    SDMMCx_RELEASE_RESET();
    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    HAL_GPIO_DeInit(SDMMCx_D0_GPIO_Port, SDMMCx_D0_Pin);
		HAL_GPIO_DeInit(SDMMCx_D1_GPIO_Port, SDMMCx_D1_Pin);
		HAL_GPIO_DeInit(SDMMCx_D2_GPIO_Port, SDMMCx_D2_Pin);
		HAL_GPIO_DeInit(SDMMCx_D3_GPIO_Port, SDMMCx_D3_Pin);
		HAL_GPIO_DeInit(SDMMCx_CK_GPIO_Port, SDMMCx_CK_Pin);
		HAL_GPIO_DeInit(SDMMCx_CMD_GPIO_Port, SDMMCx_CMD_Pin);
#if SDMMCx_INTERRUPT_USE == 1
		HAL_NVIC_DisableIRQ(SDMMCx_IRQn);
#endif
  }
} 

