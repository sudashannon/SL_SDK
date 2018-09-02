#ifndef __BOARD_PM25_H
#define __BOARD_PM25_H
#include "APP_INCLUDE.h"
#include "BSP_INCLUDE.h"
void PM2_5Module_Init(void);
void PM2_5Module_Poll(void);
typedef struct
{
	uint8_t *RecBuffer;
	uint16_t PPM; //µ¥Î»Îªug/m3
	uint16_t RecLength;
	int PPMArray[32];
}PM2_5Data_t;
extern PM2_5Data_t PM2_5ModuleData;
#endif
