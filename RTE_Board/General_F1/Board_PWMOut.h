#ifndef __BOARD_PWMOut_H
#define __BOARD_PWMOut_H
#include "APP_INCLUDE.h"
#include "BSP_INCLUDE.h"
void PWM_Init(uint32_t TimerClk,uint32_t PWMFrequency);
void PWM_ChannelNInit(uint8_t Channel,double Period);
void PWM_TimerStart(void);
void PWM_ChannelNSetPeriod(uint8_t Channel,double Period);
#endif
