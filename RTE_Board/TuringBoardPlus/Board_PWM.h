#ifndef __BOARD_PWM_H
#define __BOARD_PWM_H
#include "stm32h7xx_hal.h"
#include "Board_Config.h"
#include "RTE_Include.h"
typedef enum
{
	PWM_DCMI = 0,
	PWM_N,
}Board_Pwm_Name_e;
typedef struct
{
	Board_Pwm_Name_e PwmName;
	TIM_TypeDef *Instance;
	TIM_HandleTypeDef  TIMHalHandle;
	TIM_OC_InitTypeDef TIMOCHandle;
	uint32_t Channel;
	void (*MSPInitCallback)(void);	   
	void (*MSPDeInitCallback)(void);	 
}Board_Pwm_Control_t;
int Board_PWM_Config(Board_Pwm_Name_e PwmName,int frequency,void (*MSPInitCallback)(void));
#endif
