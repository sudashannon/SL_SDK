#ifndef __BOARD_SWITCH_H
#define __BOARD_SWITCH_H
#include "stm32f10x.h"
#include "RTE_Include.h"
typedef enum 
{
	SWITCH_1=0,  		
	SWITCH_2=1, 
	SWITCH_3=2, 
	SWITCH_4=3, 
	SWITCH_5=4, 
	SWITCH_6=5, 
	SWITCH_7=6, 
	SWITCH_8=7, 
	SWITCH_9=8, 
	SWITCH_10=9, 
	SWITCH_11=10, 
	SWITCH_12=11, 
	SWITCH_13=12, 
	SWITCH_14=13, 
	SWITCH_15=14, 
	SWITCH_16=15, 
	SWITCH_17=16, 
	SWITCH_18=17, 
	SWITCH_19=18, 
	SWITCH_20=19, 
	SWITCH_21=20, 
	SWITCH_22=21, 
	SWITCH_23=22, 
	SWITCH_24=23, 
	SWITCH_25=24, 
	SWITCH_26=25, 
	SWITCH_27=26, 
	SWITCH_28=27, 
	SWITCH_29=28, 
	SWITCH_30=29, 
	SWITCH_31=30, 
	SWITCH_N,
}Board_Switch_e;
typedef struct {
  Board_Switch_e SwitchName;    // Name
  GPIO_TypeDef* SwitchPort; // Port
  uint16_t SwitchPin; // Pin
	uint32_t SwitchClk; // Clock
  bool SwitchInitStatus;  // Init
}Board_Switch_Handle_t;
void Board_SwitchInit(void);
void Board_SwitchOn(Board_Switch_e switch_name);
void Board_SwitchOff(Board_Switch_e switch_name);
uint8_t Board_SwitchGetState(Board_Switch_e switch_name);
#endif
