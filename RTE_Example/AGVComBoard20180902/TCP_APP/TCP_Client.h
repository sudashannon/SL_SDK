#ifndef __TCP_CLIENT_H
#define __TCP_CLIENT_H
#include "RTE_Include.h"
extern uint8_t TCPClientRealData[2048];
extern uint16_t TCPClientRealDataLenth;
extern void TCPClientInit(void);
extern bool TCPClientConnect(uint8_t *RemoteIP,uint16_t RemotePort);
extern void TCPClientSendData(uint8_t *Data,uint16_t Lenth);
#endif
