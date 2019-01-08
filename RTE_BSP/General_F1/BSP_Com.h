#ifndef __SL_APPUARTDMAF1_H
#define __SL_APPUARTDMAF1_H
#include "stm32f10x.h"
#include "RTE_Include.h"
typedef enum
{
  COM_1 = 0,   // COM1 (TX=PA9, RX=PA10)
  COM_3 = 1,   // COM3 (TX=PB10, RX=PB11)
  COM_N        // COMNum
}BSP_Com_Name_e;
//--------------------------------------------------------------
// 串口数据接收结构体
//--------------------------------------------------------------
typedef struct
{
	//结构体变量
	uint8_t *pu8Databuf;
	uint16_t u16Datalength;
	RTE_MessageQuene_t ComQuene;  //串口数据环形队列
}BSP_Com_Data_t;

//--------------------------------------------------------------
// Struktur einer UARTs
//--------------------------------------------------------------
typedef struct {
	BSP_Com_Name_e ComName;    // Name
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
	DMA_Channel_TypeDef* DMATXChannel;
	DMA_Channel_TypeDef* DMARXChannel;
	uint16_t DataBufferLen;
	uint16_t DataQueneLen;
	BSP_Com_Data_t ComData;
}BSP_Com_Handle_t;
/* Wait for TX empty */
#define USART_TXEMPTY(USARTx)               ((USARTx->SR & USART_FLAG_TXE))
#define USART_WAIT(USARTx)                  while (!USART_TXEMPTY(USARTx))
#define USART_TX_REG(USARTx)                ((USARTx)->DR)
#define USART_WRITE_DATA(USARTx, data)      ((USARTx)->DR = (data))

void BSP_Com_Init(BSP_Com_Name_e usart_name,uint32_t baudrate);
BSP_Com_Data_t* BSP_Com_ReturnQue(BSP_Com_Name_e usart_name);
void BSP_Com_Putc(BSP_Com_Name_e usart_name, volatile char c);
void BSP_Com_Puts(BSP_Com_Name_e usart_name, char* str);
void BSP_Com_Send(BSP_Com_Name_e usart_name, uint8_t* DataArray, uint32_t count);
#endif
