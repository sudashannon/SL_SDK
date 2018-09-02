#include "TCP_Server.h"
#include "Net_Config.h"
static uint8_t TCPServerSocket;
uint8_t TCPServerRealData[2048] = {0};
uint16_t TCPServerRealDataLenth = 0;
extern osThreadId_t ThreadIDWebService;
static uint16_t TCPServerCallback (uint8_t soc, uint8_t evt, uint8_t *ptr, uint16_t par)
{
	char buf[50];
	/* 确保是socket_tcp的回调 */
	if (soc != TCPServerSocket) 
	{
		return (0);
	}
	switch (evt) 
	{
		/*
			远程客户端连接消息
		    1、数组ptr存储远程设备的IP地址，par中存储端口号。
		    2、返回数值1允许连接，返回数值0禁止连接。
		*/
		case TCP_EVT_CONREQ:
				sprintf(buf, "[SERVER]    有客户端连接 IP: %d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
				printf("%s 端口号:%d\n", buf, par);
			return (1);
		/* 连接终止 */
		case TCP_EVT_ABORT:
			break;
		/* Socket远程连接已经建立 */
		case TCP_EVT_CONNECT:
				printf("[SERVER]    连接建立\n");
			break;
		/* 连接断开 */
		case TCP_EVT_CLOSE:
		   	printf("[SERVER]    连接断开\n");
			break;
		/* 发送的数据收到远程设备应答 */
		case TCP_EVT_ACK:
			break;
		/* 接收到TCP数据帧，ptr指向数据地址，par记录数据长度，单位字节 */
		case TCP_EVT_DATA:
		{
			char EndString[30] = {0};
			memcpy(TCPServerRealData + TCPServerRealDataLenth, ptr, par);
			TCPServerRealDataLenth = TCPServerRealDataLenth + par;
			memcpy(EndString , TCPServerRealData + TCPServerRealDataLenth - 30 , 30);
			if(strstr((char *)EndString,"</SOAP-ENV:Envelope>"))
			{
				printf("[WebService]    收到一帧数据！\n");
				osThreadFlagsSet(ThreadIDWebService,0x0001U);
			}
		}break;
	}
	return (0);
}
void TCPServerInit(void)
{
	/* 
	   创建TCP Socket并创建监听，客户端连接服务器后，10秒内无数据通信将断开连接。
	   但是由于这里使能了TCP_TYPE_KEEP_ALIVE，会一直保持连接，不受10秒的时间限制。
	*/
  TCPServerSocket = tcp_get_socket (TCP_TYPE_SERVER | TCP_TYPE_KEEP_ALIVE, 0, 10, TCPServerCallback);
	if(TCPServerSocket != 0)
	{
		printf("[SERVER]    创建服务器Socket:%d 监听:%d\n",TCPServerSocket,tcp_listen (TCPServerSocket, 8888));
	}
}
void TCPServerSendData(uint8_t *Data,uint16_t Lenth)
{
	int32_t iCount=Lenth;
	uint8_t *dataadd=Data;
	uint16_t maxlen;
	uint8_t *sendbuf;
	while(tcp_get_state(TCPServerSocket)!=TCP_STATE_CONNECT)
	{
		main_TcpNet();
	}
	do
	{
		main_TcpNet();
		if (tcp_check_send (TCPServerSocket) == true) 
		{
			maxlen = tcp_max_dsize (TCPServerSocket);
			iCount -= maxlen;
			if(iCount < 0)
			{
				/* 这么计算没问题的 */
				maxlen = iCount + maxlen;
			}
			sendbuf = tcp_get_buf(maxlen);
			memcpy(sendbuf,dataadd,maxlen);
			/* 测试发现只能使用获取的内存 */
			tcp_send (TCPServerSocket, sendbuf, maxlen);
			if(iCount > 0)
				dataadd=dataadd+maxlen;
		}
	}while(iCount > 0);		
}

