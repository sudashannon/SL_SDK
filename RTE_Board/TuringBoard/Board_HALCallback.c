#include "Board_HALCallback.h"
#include "Board_Uart.h"
#include "Board_SPI.h"
#include "Board_WIFI.h"
extern osThreadId_t ThreadIDSensor;
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	osThreadFlagsSet(ThreadIDSensor,0x00000001U);
}
extern osThreadId_t ThreadIDWIFI;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
#if WIFI_IO_INTERRUPT_USE == 1
  if (GPIO_Pin == WIFI_INT_PIN)
  {
		if(WIFI_CTX.rw_connect_status == STATUS_OK)
			osThreadFlagsSet(ThreadIDWIFI,0x0001U);
  }
	else if(GPIO_Pin == LCD_PEN_Pin)
	{
		
	}
#endif
}
#if RTE_USE_OS == 0
extern volatile bool LCDSPIReadyFlag = false;
extern volatile bool WIFISPIReadyFlag = false;
#else 
extern osEventFlagsId_t EvtIDGUI;
extern osEventFlagsId_t EvtIDWIFI;
#endif
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi == &SpiHandle[SPI_LCD].SpiHalHandle)
	{
	#if RTE_USE_OS == 0
		if(LCDSPIReadyFlag == false)
			LCDSPIReadyFlag = true;
	#else
		osEventFlagsSet(EvtIDGUI,0x00000001ul);
	#endif
	}
	if(hspi == &SpiHandle[SPI_WIFI].SpiHalHandle)
	{
		osEventFlagsSet(EvtIDWIFI,0x00000001ul);
	}
}
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi == &SpiHandle[SPI_WIFI].SpiHalHandle)
	{
		osEventFlagsSet(EvtIDWIFI,0x00000002ul);
	}
}
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi == &SpiHandle[SPI_WIFI].SpiHalHandle)
	{
		osEventFlagsSet(EvtIDWIFI,0x00000003ul);
	}
}
