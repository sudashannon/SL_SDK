#ifndef __BOARD_ESP8266_H
#define __BOARD_ESP8266_H
#include "APP_INCLUDE.h"
#include "BSP_INCLUDE.h"
#define ESP8266_COM COM_1
#define ESP8266_BUFLEN COM1_DATALEN
typedef enum
{
	SEND_OK = 0x00,
	CONNECTSERVER_FAIL = 0x01,
	CIPSEND_FAIL = 0x02,
	DATASEND_FAIL = 0x03,
	RECRESPOND_FAIL = 0x04,
	DISCONNECTSERVER_FAIL = 0x05,
	SERVERMODEEXIT_FAIL = 0x06,
	SINGLECONNECTMODE_FAIL = 0x07,
	SERVERMODEENTER_FAIL = 0x08,
}ESP8266_SendState_e;
typedef struct
{
	uint8_t SSIDNum;
	char *SSIDName;
}ESP8266_SSIDInfor_t;
typedef struct
{
	bool ServerSet;
	bool ClientConnect;
	bool WIFIConnect;
	uint8_t WifiCnt;
	uint8_t IP[4];
	uint8_t ConnectClientNum;
	uint32_t DataUploadTick;
	ESP8266_SSIDInfor_t *SSIDInfor;
}ESP8266_WifiScanInfor_t;
typedef struct
{
	uint8_t Temper[5];
	uint8_t Weather[20];
	uint8_t Hour;
	uint8_t Min;
	uint8_t Second;
	uint8_t Date[20];
}ESP8266_Weather_t;
extern uint8_t ESP8266RecBuf[ESP8266_BUFLEN];
extern ESP8266_WifiScanInfor_t WifiScanInfor;
extern ESP8266_Weather_t ESP8266_Weather;
bool ATComand_Send(char *SendStr,const char *ExpectRec,uint32_t TimeOut);
void ESP8266Reset(void);
void ESP8266Init(void);
ESP8266_SendState_e ESP8266SendWeatherGet(const char* City);
ESP8266_SendState_e ESP8266SendTimeGet(void);
void ESP8266WiFiScanRelease(void);
void ESP8266IPGet(void);
void ESP8266TCPServerPoll(void);
bool ESP8266SetTCPServer(void);
bool ESP8266DisconnectTCPServer(void);
#endif
