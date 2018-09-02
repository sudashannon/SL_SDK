#ifndef __BOARD_DHT11_H
#define __BOARD_DHT11_H
#include "APP_INCLUDE.h"
#include "BSP_INCLUDE.h"
#define DHT11_DataPin  GPIO_Pin_12
#define DHT11_GPIO     GPIOF
#define DHT11_GPIOCLK  RCC_APB2Periph_GPIOF
typedef struct
{
	uint8_t TemperHigh8;
	uint8_t TemperLow8;
	uint8_t HumdityHigh8;
	uint8_t HumdityLow8;
	uint8_t CheckData;
}DHT11Result_t;
void DHT11_GetData(void);
extern DHT11Result_t DHT11Result;
#endif
