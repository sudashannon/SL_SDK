#ifndef __BOARD_RFID_H
#define __BOARD_RFID_H
#include "stm32f4xx.h"
#include "RTE_Include.h"   
typedef struct
{
	uint16_t type;
	uint32_t num;
}RFID_Num_t;
extern bool Board_RFID_Init(void);
extern bool Board_RFID_ReadCardNum(RFID_Num_t *Num,uint8_t retrytimes);
#endif
