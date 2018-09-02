#ifndef __BOARD_SR501_H
#define __BOARD_SR501_H
#include "APP_INCLUDE.h"
#include "BSP_INCLUDE.h"
#define SR501_DataPin  GPIO_Pin_5
#define SR501_GPIO     GPIOB
#define SR501_GPIOCLK  RCC_APB2Periph_GPIOB
typedef enum
{
	IDLE = 0x00,
	TIMER = 0x01,
	END= 0x02,
}SR501State_e;
typedef struct
{
	SR501State_e SR501State;
	bool IfPeople;
	uint32_t HighTime;
}SR501Result_t;
extern SR501Result_t SR501Result;
void SR501_Check(void);
#endif
