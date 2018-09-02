#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H
#include "RTE_Include.h"
extern uint8_t TCPServerRealData[2048];
extern uint16_t TCPServerRealDataLenth;
extern void TCPServerInit(void);
extern void TCPServerSendData(uint8_t *Data,uint16_t Lenth);
#endif
