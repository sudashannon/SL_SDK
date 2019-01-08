#ifndef __BSP_KEY_H
#define __BSP_KEY_H
#include "stm32f10x.h"
#include "RTE_Include.h"
#define KEY_FIFO_SIZE 16
#define KEY_FILTER_TIME 25
#define KEY_LONGPRESS_ENABLE 1
#if KEY_LONGPRESS_ENABLE
#define KEY_LONGPRESS_TIME 500
#endif
typedef enum
{
    KEY_0 = 0,
	KEY_1 = 1,
    KEY_N,
}BSP_Key_Name_e;
typedef enum
{
    KEY_NONE = 0,
    KEY_PRESS = 1,
    KEY_RELEASE = 2,
#if KEY_LONGPRESS_ENABLE
    KEY_LONG = 3,
#endif
}BSP_Key_State_e;
typedef struct
{
    BSP_Key_State_e *Key_Buffer;
    uint8_t WritePointer;
    uint8_t ReadPointer;
}BSP_Key_FIFO_t;
typedef enum
{
    KEY_SM_POLL = 0,
    KEY_SM_PRESS = 1,
    KEY_SM_RELEASE = 2,
}BSP_Key_SM_e;
typedef struct
{
    BSP_Key_Name_e KeyName;
    uint8_t KeyPressValue;
	GPIO_TypeDef* KeyPort; // Port
	uint16_t KeyPin; // Pin
	uint32_t KeyClk; // Clock
#if KEY_LONGPRESS_ENABLE
    uint32_t KeyStartTick;
#endif
    BSP_Key_SM_e KeySM;
    BSP_Key_FIFO_t KeyFIFO;
}BSP_Key_Handle_t;
extern BSP_Key_Handle_t KeyHandle[KEY_N] ;

extern void BSP_Key_Init(BSP_Key_Name_e KeyName);
extern bool BSP_Key_ReadInstant(BSP_Key_Name_e KeyName);

extern void BSP_Key_ClearBuffer(BSP_Key_Name_e KeyName);
extern BSP_Key_State_e BSP_Key_Read(BSP_Key_Name_e KeyName);
extern void BSP_Key_Timer_Callback(void* UserParameters);
#endif
