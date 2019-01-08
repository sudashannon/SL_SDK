#ifndef __WIFI_TCPUDP_H
#define __WIFI_TCPUDP_H
typedef struct
{
	uint8_t *RecData;
	int Reclen;
	int SendRecvCnt;
}WIFI_TCPClient_Handle_t;
typedef struct
{
	uint8_t *RecData;
	int Reclen;
	int SendRecvCnt;
	uint8_t CurrentClient;
	bool ClientConnect;
}WIFI_TCPServer_Handle_t;
extern WIFI_TCPClient_Handle_t TCPClientHandle;
extern WIFI_TCPServer_Handle_t TCPServerHandle;

int WIFI_TCPClient_Connect(WIFI_TCPClient_Handle_t *thisHandle,uint16_t destPort, int destIp);
void WIFI_TCPClient_EventHandle(WIFI_TCPClient_Handle_t *thisHandle);
int WIFI_TCPClient_Send(WIFI_TCPClient_Handle_t *thisHandle,uint8_t *SendData,uint16_t SendLength);

int WIFI_TCPServer_Create(WIFI_TCPServer_Handle_t *thisHandle,uint16_t lPort);
void WIFI_TCPServer_EventHandle(WIFI_TCPServer_Handle_t *thisHandle);
int WIFI_TCPServer_Send(WIFI_TCPServer_Handle_t *thisHandle,int ClientID,uint8_t *Data,uint16_t DataLen,uint32_t TimeOut);

#endif
