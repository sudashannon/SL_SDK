#ifndef __BOARD_UART_H
#define __BOARD_UART_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
typedef enum
{
//  COM_1 = 0,   // COM1 (TX=PA9, RX=PA10)
	USART_DEBUG = 0,
  USART_N        // COMNum
}Board_Uart_Name_e;
typedef struct
{
  uint32_t BaudRate;
  uint32_t WordLength;
  uint32_t StopBits;
  uint32_t Parity;  
}Board_Uart_HardWareConfig_t;
typedef struct
{
	uint8_t *pu8Databuf;
	uint16_t u16Datalength;
	RTE_MessageQuene_t   ComQuene;  //串口数据环形队列
}Board_Uart_Data_t;
typedef struct
{
	Board_Uart_Name_e UsartName;
	uint16_t BufferSize;
	uint16_t RingBufferSize;
	Board_Uart_Data_t UsartData;
	USART_TypeDef *Instance;
	Board_Uart_HardWareConfig_t *HardConfig;
	UART_HandleTypeDef UartHalHandle;
  DMA_HandleTypeDef UsartHalRxDmaHandle;
  DMA_HandleTypeDef UsartHalTxDmaHandle;
	void (*MSPInitCallback)(void);	   
	void (*MSPDeInitCallback)(void);	 
}Board_Uart_Control_t;
extern Board_Uart_Control_t UartHandle[USART_N];
extern void Board_Uart_Init(Board_Uart_Name_e usartname,void (*MSPInitCallback)(void),void (*MSPDeInitCallback)(void));
extern void Board_Uart_RecCallback(Board_Uart_Name_e usart_name);
extern Board_Uart_Data_t *Board_Uart_ReturnQue(Board_Uart_Name_e usart_name);
#endif





