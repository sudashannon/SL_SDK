/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_COM_H
#define __BSP_COM_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "RTE_Include.h"
//--------------------------------------------------------------
//串口实例
//--------------------------------------------------------------
typedef enum
{
  COM_1 = 0,   // COM1 (TX=PA9, RX=PA10)
  COM_3 = 1,   // COM3 (TX=PB10, RX=PB11)
  COM_N        // COMNum
}BSP_COM_NAME_e;
//--------------------------------------------------------------
// 串口数据结构体
//--------------------------------------------------------------
typedef struct
{
	//结构体变量
	uint8_t *pu8Databuf;
	uint16_t u16Datalength;
	RTE_MessageQuene_t   ComQuene;  //串口数据环形队列
}BSP_COM_Data_t;
//--------------------------------------------------------------
// 串口控制结构体
//--------------------------------------------------------------
typedef struct {
  BSP_COM_NAME_e ComName;    // Name
	USART_TypeDef* USARTx;
  GPIO_TypeDef* ComPort; // Port
	uint8_t TxPinsource;
	uint8_t RxPinsource;
	uint8_t ComAF;
  uint16_t ComPintx; // Pin
  uint16_t ComPinrx; // Pin
  uint32_t GpioClk; // Clock
	DMA_Stream_TypeDef* DMAStreamx;
	uint32_t DMARxChannel;
	uint32_t DmaClk; // Clock
	uint16_t DataBufferLen;
	uint16_t DataQueneLen;
	BSP_COM_Data_t ComBuffer;
}BSP_COM_Handle_t;
/* Wait for TX empty */
#define USART_TXEMPTY(USARTx)               ((USARTx->SR & USART_FLAG_TXE))
#define USART_WAIT(USARTx)                  while (!USART_TXEMPTY(USARTx))
#define USART_TX_REG(USARTx)                ((USARTx)->DR)
#define USART_WRITE_DATA(USARTx, data)      ((USARTx)->DR = (data))
void BSP_COM_Init(BSP_COM_NAME_e usart_name,uint32_t baudrate);
void BSP_COM_Putc(BSP_COM_NAME_e usart_name, volatile char c);
void BSP_COM_Puts(BSP_COM_NAME_e usart_name, char* str);
void BSP_COM_Send(BSP_COM_NAME_e usart_name, uint8_t* DataArray, uint32_t count);
BSP_COM_Data_t *BSP_COM_ReturnQue(BSP_COM_NAME_e usart_name);

#endif
