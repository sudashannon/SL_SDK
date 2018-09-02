#ifndef __SL_APPUARTDMAF1_H
#define __SL_APPUARTDMAF1_H
#include "BSP_INCLUDE.h"
#include "APP_RingBuffer.h"
typedef enum
{
  COM_1 = 0,   // COM1 (TX=PA9, RX=PA10)
  COM_2 = 1,   // COM1 (TX=PA2, RX=PA3)
  COM_N        // COMNum
}BSP_USART_NAME_e;
typedef enum
{
	OK        = 0x00,
	TIME_OUT,
}BSP_USART_SendStatus_e;
//--------------------------------------------------------------
// 串口数据接收结构体
//--------------------------------------------------------------
typedef struct
{
	//结构体变量
	uint8_t *pu8Databuf;
	uint16_t u16Datalength;
	APP_MessageQuene_t   ComQuene;  //串口数据环形队列
}BSP_USART_Data_t;

//--------------------------------------------------------------
// Struktur einer UARTs
//--------------------------------------------------------------
typedef struct {
  BSP_USART_NAME_e ComName;    // Name
	USART_TypeDef* USARTx;
  GPIO_TypeDef* ComPort; // Port
  uint16_t ComPintx; // Pin
  uint16_t ComPinrx; // Pin
  uint32_t GpioClk; // Clock
  uint16_t WordLength; 
  uint16_t StopBits;
  uint16_t Parity; 
  uint16_t Mode; 
  uint16_t HardwareFlowControl; 
	DMA_Channel_TypeDef* DMARXChannel;
	DMA_Channel_TypeDef* DMATXChannel;
	uint16_t DataBufferLen;
	uint16_t DataQueneLen;
	BSP_USART_Data_t BSP_USARTBuffer;
}BSP_USART_Handle_t;
/* Wait for TX empty */
#define USART_TXEMPTY(USARTx)               ((USARTx->SR & USART_FLAG_TXE))
#define USART_WAIT(USARTx)                  while (!USART_TXEMPTY(USARTx))
#define USART_TX_REG(USARTx)                ((USARTx)->DR)
#define USART_WRITE_DATA(USARTx, data)      ((USARTx)->DR = (data))
BSP_USART_Data_t *BSP_USART_ReturnQue(BSP_USART_NAME_e usart_name);
void BSP_USART_Init(BSP_USART_NAME_e usart_name,uint32_t baudrate);
BSP_USART_Data_t* BSP_USART_ReturnQue(BSP_USART_NAME_e usart_name);
void BSP_USART_Putc(BSP_USART_NAME_e usart_name, volatile char c);
void BSP_USART_Puts(BSP_USART_NAME_e usart_name, char* str);
void BSP_USART_Send(BSP_USART_NAME_e usart_name, uint8_t* DataArray, uint32_t count);
#endif
