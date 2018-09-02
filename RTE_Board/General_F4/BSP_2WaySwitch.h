#ifndef __BSP_2WAYSWITCH_H
#define __BSP_2WAYSWITCH_H
#include "stm32f4xx.h"
#include "RTE_Include.h"
typedef enum
{
	SWITCH_01 = 0 ,
	SWITCH_02 ,
	SWITCH_03 ,
	SWITCH_04 ,
	SWITCH_05 ,
	SWITCH_06 ,
	SWITCH_07 ,
	SWITCH_08 ,
	SWITCH_09 ,
	SWITCH_10 ,
	SWITCH_11 ,
	SWITCH_12 ,
	SWITCH_13 ,
	SWITCH_14 ,
	SWITCH_15 ,
	SWITCH_N ,
}BSP_2WaySwtch_Name_e;
typedef enum
{
	STATUS_NONE = 0x00,
	STATUS_NC = 0x01,
	STATUS_NO = 0x02,
}BSP_2WaySwtch_Status_e;
typedef struct
{
	BSP_2WaySwtch_Name_e SwitchName;
  GPIO_TypeDef* SwitchPort; // Port
	uint16_t NCPin;
	uint16_t NOPin;
	uint32_t SwitchClk; // Clock
	BSP_2WaySwtch_Status_e SwitchStatus;
}BSP_2WaySwitch_t;
extern void BSP_2WaySwitch_Init(void);
extern BSP_2WaySwtch_Status_e BSP_2WaySwitch_ReadStatus(BSP_2WaySwtch_Name_e i);
#endif
