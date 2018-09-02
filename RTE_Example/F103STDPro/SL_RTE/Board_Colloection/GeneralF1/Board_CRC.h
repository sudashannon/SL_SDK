#ifndef __BOARD_CRC_H
#define __BOARD_CRC_H
#include "APP_Include.h"
unsigned short CRC16_CCITT(unsigned char *puchMsg, unsigned int usDataLen);
unsigned short CRC16_CCITT_FALSE(unsigned char *puchMsg, unsigned int usDataLen)  ;
unsigned short CRC16_XMODEM(unsigned char *puchMsg, unsigned int usDataLen)  ;
unsigned short CRC16_X25(unsigned char *puchMsg, unsigned int usDataLen)  ;
unsigned short CRC16_MODBUS(unsigned char *puchMsg, unsigned int usDataLen)  ;
unsigned short CRC16_IBM(unsigned char *puchMsg, unsigned int usDataLen)  ;
unsigned short CRC16_MAXIM(unsigned char *puchMsg, unsigned int usDataLen)  ;
unsigned short CRC16_USB(unsigned char *puchMsg, unsigned int usDataLen)  ;

#endif
