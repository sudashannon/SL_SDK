#ifndef __BOARD_LN33_H
#define __BOARD_LN33_H
#include "stm32f4xx.h"
#include "RTE_Include.h"   
#define CONTROL_PORT 0x21
#define READ_PORT    0x90
#define EGG_PORT     0x91
#define WATER_PORT   0x92
#define NOISE_PORT   0x93
extern uint16_t Board_LN33_GetAddress(uint32_t timeout);
extern uint16_t Board_LN33_GetID(uint32_t timeout);
extern uint8_t Board_LN33_GetChannel(uint32_t timeout);
extern void Board_LN33_SendData(uint8_t lenth,uint8_t port,uint16_t destadd,uint8_t *data);
#endif
