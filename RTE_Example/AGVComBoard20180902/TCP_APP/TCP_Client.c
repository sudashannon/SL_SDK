#include "TCP_Client.h"
#include "Net_Config.h"
#include "AGV_SM.h"
static uint8_t TCPClientrSocket;
static uint16_t LocalPortNum = 0;
uint8_t TCPClientRealData[2048] = {0};
uint16_t TCPClientRealDataLenth = 0;
/*
*********************************************************************************************************
*	函 数 名: tcp_callback
*	功能说明: TCP Socket的回调函数
*	形    参: soc  TCP Socket类型
*             evt  事件类型
*             ptr  事件类型是TCP_EVT_DATA，ptr指向的缓冲区记录着接收到的TCP数据，其余事件记录IP地址
*             par  事件类型是TCP_EVT_DATA，记录接收到的数据个数，其余事件记录端口号
*	返 回 值: 
*********************************************************************************************************
*/
static uint16_t TCPClientCallback (uint8_t soc, uint8_t evt, uint8_t *ptr, uint16_t par)
{
	char buf[50];
	/* 确保是socket_tcp的回调 */
	if (soc != TCPClientrSocket) 
	{
		return (0);
	}

	switch (evt) 
	{
		/*
			链接服务器连接消息
		    1、数组ptr存储远程设备的IP地址，par中存储端口号。
		    2、返回数值1允许连接，返回数值0禁止连接。
		*/
		case TCP_EVT_CONREQ:
				sprintf(buf, "连接到服务器 IP: %d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
				printf("[CLIENT]    %s 端口号Port:%d\n", buf, par);
			return (1);
		/* 连接终止 */
		case TCP_EVT_ABORT:
		{
			printf("[CLIENT]    连接终止\n");
		}break;
		/* Socket远程连接已经建立 */
		case TCP_EVT_CONNECT:
		{
			printf("[CLIENT]    连接建立\n");
		}break;
		/* 连接断开 */
		case TCP_EVT_CLOSE:
		{
			printf("[CLIENT]    连接被关闭\n");
		}break;
		/* 发送的数据收到远程设备应答 */
		case TCP_EVT_ACK:
			break;
		/* 接收到TCP数据帧，ptr指向数据地址，par记录数据长度，单位字节 */
		case TCP_EVT_DATA:
		{
			printf("[CLIENT]    服务器已经收到！\n");
			extern AGV_SM_Arg_t AGV_RunningTask;
			AGV_RunningTask.ServerPortNum = 0;
			memset(AGV_RunningTask.ServerIP,0,4);
			RTE_RoundRobin_PauseTimer("ClientTimer");
			AGV_RunningTask.AGVStateMachine.RunningState = TASK_IDLE;
		}break;
	}
	return (0);
}
void TCPClientInit(void)
{
	/* 
	   创建TCP Socket并连接，客户端连接服务器后，10秒内无数据通信将断开连接。
	   但是由于这里使能了TCP_TYPE_KEEP_ALIVE，会一直保持连接，不受10秒的时间限制。
	*/
  TCPClientrSocket = tcp_get_socket (TCP_TYPE_CLIENT | TCP_TYPE_KEEP_ALIVE, 0, 10, TCPClientCallback);
	printf("[CLIENT]    创建客户端Socket:%d\n",TCPClientrSocket);
}
bool TCPClientConnect(uint8_t *RemoteIP,uint16_t RemotePort)
{
	if(TCPClientrSocket != 0)
	{
		if(tcp_get_state(TCPClientrSocket)!=TCP_STATE_CONNECT)
		{
			LocalPortNum = (uint16_t)RTE_RoundRobin_GetTick();
			bool rev = tcp_connect (TCPClientrSocket, RemoteIP, RemotePort, LocalPortNum);
			printf("[CLIENT]    连接到服务器：%d.%d.%d.%d:%d 状态:%d\n",RemoteIP[0],RemoteIP[1],RemoteIP[2]
			,RemoteIP[3],RemotePort,rev);
			return rev;
		}
		else
			return true;
	}
	return false;
}
void TCPClientSendData(uint8_t *Data,uint16_t Lenth)
{
	int32_t iCount=Lenth;
	uint8_t *dataadd=Data;
	uint8_t SocketState;
	uint16_t maxlen;
	uint8_t *sendbuf;
	SocketState = tcp_get_state(TCPClientrSocket);
	if(SocketState == TCP_STATE_CONNECT)
	{
		do
		{
			main_TcpNet();
			if (tcp_check_send (TCPClientrSocket) == true) 
			{
				maxlen = tcp_max_dsize (TCPClientrSocket);
				iCount -= maxlen;
				if(iCount < 0)
				{
					/* 这么计算没问题的 */
					maxlen = iCount + maxlen;
				}
				sendbuf = tcp_get_buf(maxlen);
				memcpy(sendbuf,dataadd,maxlen);
				/* 测试发现只能使用获取的内存 */
				tcp_send (TCPClientrSocket, sendbuf, maxlen);
				if(iCount > 0)
					dataadd=dataadd+maxlen;
			}
		}while(iCount > 0);		
	}
}
