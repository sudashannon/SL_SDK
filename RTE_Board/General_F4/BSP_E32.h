#ifndef __BSP_E32_H
#define __BSP_E32_H
#include "stm32f4xx.h"
#include "RTE_Include.h"
#include "BSP_Com.h"
typedef enum
{
	MODE_NORMAL = 0x00,
	MODE_LOWPWR = 0x01,
	MODE_WAKEUP = 0x02,
	MODE_SLEEP = 0x03,
}BSP_E32_WorkMode_e;
extern void BSP_E32_Init(void);
extern void BSP_E32_ModeSet(BSP_E32_WorkMode_e Mode);
extern void BSP_E32_ArgsSet(uint16_t DeviceAdd);
extern void BSP_E32_Send(uint8_t *Data,uint16_t Length);
#endif
