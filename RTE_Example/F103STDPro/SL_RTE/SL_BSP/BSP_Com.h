/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_COM_H
#define __BSP_COM_H
/* Includes ------------------------------------------------------------------*/
#include "APP_Include.h"
#include "BSP_Include.h"
#include "APP_RingBuffer.h"
//--------------------------------------------------------------
// Liste aller UARTs
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum
{
  COM_1 = 0,   // COM1 (TX=PA9, RX=PA10)
  COM_2 = 1,   // COM2 (TX=PA2, RX=PA3)
  COM_3 = 2,   // COM3 (TX=PB10, RX=PB11)
  COM_4 = 3,   // COM3 (TX=PA0, RX=PA1)
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
#ifdef STM32F4
	uint8_t ComAF;         // AF
#endif
  uint16_t ComPintx; // Pin
  uint16_t ComPinrx; // Pin
  uint32_t GpioClk; // Clock
  uint16_t WordLength; 
  uint16_t StopBits;
  uint16_t Parity; 
  uint16_t Mode; 
  uint16_t HardwareFlowControl; 
	uint16_t BufferSize;
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
