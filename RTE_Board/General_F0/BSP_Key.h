#ifndef __BSP_KEY_H
#define __BSP_KEY_H
#include "stm32f0xx.h"
#include "RTE_Include.h"
typedef enum
{
    KEY_NONE = 0,
    KEY_PRESS = 1,
    KEY_RELEASE = 2,
    KEY_LONG = 3,
}Key_State_e;
typedef enum
{
    KEY_0 = 0,
    KEY_1 = 1,
    KEY_N,
}Key_Name_e;
#define KEY_FIFO_SIZE 10
typedef struct
{
	Key_State_e Key_Buffer[KEY_FIFO_SIZE];
	uint8_t WritePointer;
	uint8_t ReadPointer;
}Key_FIFO_t;
typedef struct
{
	Key_Name_e KeyName;
	Key_FIFO_t KeyFIFO;
	uint8_t KeyPressValue;
	GPIO_TypeDef* KeyPort;
	uint16_t KeyPin;
	uint32_t KeyClk; // Clock
	uint32_t KeyStartTick;
	uint32_t KeyLongTick;
}Key_Handle_t;


#endif
