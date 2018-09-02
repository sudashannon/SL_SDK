#ifndef __BSP_CAN_H
#define __BSP_CAN_H
/* Includes ------------------------------------------------------------------*/
#include "tm4c1294ncpdt.h"
#include "RTE_Include.h"
#include "sysctl.h"
#include "gpio.h"
#include "interrupt.h"
#include "can.h"
#include "pin_map.h"
#include "hw_ints.h"
#include "hw_can.h"
typedef enum
{
  CAN_1 = 0,
  CAN_N 
}BSP_CAN_NAME_e;
//--------------------------------------------------------------
// CAN数据接收结构体
//--------------------------------------------------------------
typedef struct
{
	//结构体变量
	uint8_t *pu8Databuf;
}BSP_CAN_Data_t;
//--------------------------------------------------------------
// Struktur einer UARTs
//--------------------------------------------------------------
typedef struct {
  BSP_CAN_NAME_e CANName;    // Name
	uint16_t DataBufferLen;
	BSP_CAN_Data_t CANBuffer;
	tCANMsgObject CANMessage;
}BSP_CAN_Handle_t;
extern BSP_CAN_Handle_t CANControlHandle[CAN_N];
void BSP_CAN_Init(BSP_CAN_NAME_e can_name);
uint8_t BSP_CAN1_WriteData(uint16_t ID,uint8_t *data,uint8_t datalen);
#endif /* __EEPROM_H */
