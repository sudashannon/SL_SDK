#include "stm32h7xx_hal.h"
#include "stm32h7xx.h"
#include "Board_Uart.h"
#include "Board_SPI.h"
#include "Board_DCMI.h"
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UartHandle[USART_DEBUG].UartHalHandle);
}
void DMA1_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&UartHandle[USART_DEBUG].UsartHalRxDmaHandle);
}
#if USART1_USE_DMA_TX == 1
void DMA1_Stream1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&UartHandle[USART_DEBUG].UsartHalTxDmaHandle);
}
#endif
void DMA1_Stream2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&BoardDcmiDma);
}
void DCMI_IRQHandler(void)
{
  HAL_DCMI_IRQHandler(&BoardDcmi);
}
void DMA1_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&SpiHandle[SPI_LCD].SpiHalTxDmaHandle);
}
void DMA1_Stream4_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&SpiHandle[SPI_WIFI].SpiHalTxDmaHandle);
}
void DMA1_Stream5_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&SpiHandle[SPI_WIFI].SpiHalRxDmaHandle);
}
void SPI1_IRQHandler(void)
{
  HAL_SPI_IRQHandler(&SpiHandle[SPI_WIFI].SpiHalHandle);
}

void SPI4_IRQHandler(void)
{
  HAL_SPI_IRQHandler(&SpiHandle[SPI_LCD].SpiHalHandle);
}
void EXTI9_5_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(LCD_PEN_Pin);
}
