#include "stm32h7xx_hal.h"
#include "Board_SDRAM.h"
#include "Board_RGBLCD.h"
#include "Board_Uart.h"
#include "Board_SDCard.h"
#include "Board_DCMI.h"
void MDMA_IRQHandler(void)
{
  HAL_MDMA_IRQHandler(&SdarmHandle.SdramMdmaHalhandle);
}
void DMA2D_IRQHandler(void)
{
  HAL_DMA2D_IRQHandler(&RGBLCDHandle.RGBLCDHdma2dHalHandle);
}
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UartHandle[USART_DEBUG].UartHalHandle);
}
void DMA1_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&UartHandle[USART_DEBUG].UsartHalRxDmaHandle);
}
void DMA1_Stream2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&BoardDcmiDma);
}
void DCMI_IRQHandler(void)
{
  HAL_DCMI_IRQHandler(&BoardDcmi);
}
#if USART1_USE_DMA_TX == 1
void DMA1_Stream1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&UartHandle[USART_DEBUG].UsartHalTxDmaHandle);
}
#endif
#if SDMMCx_INTERRUPT_USE == 1
void SDMMCx_IRQHandler(void)
{
  HAL_SD_IRQHandler(&BoardSDcard);
}
#endif
