#include "Board_SDRAM.h"
#include "Board_Uart.h"
#include "Board_SDCard.h"
void HAL_SDRAM_DMA_XferCpltCallback(MDMA_HandleTypeDef *hmdma)
{
	
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
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{

}
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{

}
void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
  RTE_Assert(__FILE__, __LINE__);
}
extern osThreadId_t ThreadIDSensor;
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	osThreadFlagsSet(ThreadIDSensor,0x00000001U);
}
