#ifndef __BOARD_RTC_H
#define __BOARD_RTC_H
#include "stm32f4xx.h"
#include "RTE_Include.h"   
extern ErrorStatus Board_RTC_SetTime(uint8_t hour,uint8_t min,uint8_t sec,uint8_t ampm);
extern ErrorStatus Board_RTC_SetDate(uint8_t year,uint8_t month,uint8_t date,uint8_t week);
extern ErrorStatus Board_RTC_Init(void);
#endif
