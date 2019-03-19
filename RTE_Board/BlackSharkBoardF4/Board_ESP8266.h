#ifndef __BOARD_ESP8266_H
#define __BOARD_ESP8266_H
#include "stm32f4xx.h"
#include "RTE_Include.h"   
typedef struct
{
	uint8_t ecn;
	int8_t rssi;
	uint8_t *ssid;
	uint8_t *mac;
}ESP8266_HotsPot_t;
extern void Board_ESP8266_Init(void);
extern void Board_ESP8266_Off(void);
extern bool Board_ESP8266_Boot(uint8_t retrytimes);
extern void Board_ESP8266_HardReset(void);
extern uint8_t *Board_ESP8266_ATSend(const char *command,const char *ideares,uint32_t timeout);


extern bool Board_ESP8266_SetWorkMode(uint8_t workmode);
extern bool Board_ESP8266_SetATE(uint8_t ifon);
extern uint8_t Board_ESP8266_ScanHotsPot(ESP8266_HotsPot_t **resultarray);
extern int8_t Board_ESP8266_FindHotsPot(ESP8266_HotsPot_t *resultarray,const char *ssid,uint8_t cnt);
extern void Board_ESP8266_ReleaseHotsPot(ESP8266_HotsPot_t **resultarray,uint8_t num);
extern bool Board_ESP8266_ConnectAP(const char *ssid,const char *password);
extern bool Board_ESP8266_DisConnectAP(void);
extern bool Board_ESP8266_GetIP(char **ip);
extern bool Board_ESP8266_OpenServer(char *portnum);
extern bool Board_ESP8266_CloseServer(void);
extern uint16_t Board_ESP8266_SendDataAsClient(uint8_t channel,uint8_t *data,uint16_t lenth);
#endif
