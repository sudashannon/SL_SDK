#ifndef __BSP_COM_H
#define __BSP_COM_H
#include "RTE_Include.h"
#include "tm4c1294ncpdt.h"
#include "sysctl.h"
#include "gpio.h"
#include "interrupt.h"
#include "uart.h"
#include "pin_map.h"
#include "hw_ints.h"
//--------------------------------------------------------------
// Liste aller UARTs
// (keine Nummer doppelt und von 0 beginnend)
//--------------------------------------------------------------
typedef enum
{
  COM_0 = 0,   // COM1 (TX=PA9, RX=PB7)
  COM_2 = 1,   // COM1 (TX=PA9, RX=PB7)
  COM_N        // COMNum
}BSP_COM_NAME_e;
//--------------------------------------------------------------
// 串口数据接收结构体
//--------------------------------------------------------------
typedef struct
{
	//结构体变量
	uint8_t *pu8Databuf;
	uint16_t u16Datalength;
	RTE_MessageQuene_t   ComQuene;  //串口数据环形队列
}BSP_COM_Data_t;
//--------------------------------------------------------------
// Struktur einer UARTs
//--------------------------------------------------------------
typedef struct {
  BSP_COM_NAME_e ComName;    // Name
	uint16_t DataBufferLen;
	uint16_t DataQueneLen;
	BSP_COM_Data_t ComBuffer;
}BSP_COM_Handle_t;
void BSP_COM_Init(BSP_COM_NAME_e usart_name);
void BSP_COM_Send(BSP_COM_NAME_e usart_name, uint8_t* DataArray, uint32_t count);
BSP_COM_Data_t* BSP_COM_ReturnQue(BSP_COM_NAME_e usart_name);
#endif
