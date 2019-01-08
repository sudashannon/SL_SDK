#include "Board_WIFI.h"
WIFI_TCPClient_Handle_t TCPClientHandle = 
{
	.RecData = NULL,
	.Reclen = 0,
	.SendRecvCnt = 0,
};
WIFI_TCPServer_Handle_t TCPServerHandle =
{
	.RecData = NULL,
	.Reclen = 0,
	.SendRecvCnt = 0,
	.CurrentClient = 0xFF,
	.ClientConnect = false,
};
int WIFI_TCPClient_Connect(WIFI_TCPClient_Handle_t *thisHandle,uint16_t destPort, int destIp)
{
	if(!thisHandle->RecData)
	{
		thisHandle->RecData = RTE_MEM_Alloc0(MEM_RTE,MAX_RECV_PACKET_LEN);
		RTE_AssertParam(thisHandle->RecData);
	}
	SOCKADDR_IN destAddr;
	SOCKADDR_IN LocalAddr;
	static int tmp_sockfd = INVAILD_SOCK_FD;
	int sockfd;
	int ret;
	int lPort;
	//filling the TCP client socket address
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons((uint16_t)destPort);
	destAddr.sin_addr = htonl((int)destIp);
	// creating a TCP socket  
	if(tmp_sockfd ==INVAILD_SOCK_FD)
	{
		tmp_sockfd = socket( AF_INET,SOCK_STREAM, 0);
		if (tmp_sockfd < RW_OK)
		{
			RTE_MEM_Free(MEM_RTE,thisHandle->RecData);
			return RW_ERR;
		}
	}
	srand((int) RTE_RoundRobin_GetTick());
	/*port : 1024 - 32768 */
	lPort = (rand() % (32768-1024+1))+ 1024; 
	//filling the TCP client local port
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port = htons((uint16_t)lPort);
	LocalAddr.sin_addr = 0;
	// binding the TCP socket to the TCP server address
	ret = bind(tmp_sockfd, (SOCKADDR_IN *)&LocalAddr, sizeof(SOCKADDR_IN));
	if (ret == RW_ERR || ret == RW_ERR_TIME_OUT || ret ==RW_ERR_SOCKET_INVAILD)
	{
		 close(tmp_sockfd);
		 tmp_sockfd = INVAILD_SOCK_FD;
		RTE_MEM_Free(MEM_RTE,thisHandle->RecData);
		 return ret;
	}
	// connecting to TCP server
	ret = connect(tmp_sockfd, (SOCKADDR_IN *)&destAddr, sizeof(destAddr));
	if (ret == RW_ERR || ret == RW_ERR_TIME_OUT || ret ==RW_ERR_SOCKET_INVAILD)
	{
		close(tmp_sockfd);
		tmp_sockfd = INVAILD_SOCK_FD;
		RTE_MEM_Free(MEM_RTE,thisHandle->RecData);
		return ret;
	}
	if (ret == RW_OK) 
	{     
		sockfd = tmp_sockfd ;
		tmp_sockfd = INVAILD_SOCK_FD;
		RTE_MEM_Free(MEM_RTE,thisHandle->RecData);
		return sockfd;
	}
	return ret;
}
void WIFI_TCPClient_EventHandle(WIFI_TCPClient_Handle_t *thisHandle)
{
	int ret = 0;
	if (WIFI_CTX.tcp_cloud_sockfd == INVAILD_SOCK_FD)
	{
		if((ret = WIFI_TCPClient_Connect(thisHandle, 25001, 0xC0A8006D)) >= 0)
		{
			WIFI_CTX.tcp_cloud_sockfd = ret;
			WIFI_CTX.tcp_cloud_status = STATUS_OK;
			RTE_Printf("[TCPCLIENT]    RAK_TcpClient sockfd = %u creat\n",WIFI_CTX.tcp_cloud_sockfd);
		}
		else
		{
			if(ret == RW_ERR || ret==RW_ERR_TIME_OUT) 
			{ 
				RTE_Printf("[TCPCLIENT]    RAK_TcpClient creat failed\n");
			}
			return;
		}    
	}
	if (WIFI_CTX.tcp_cloud_status <= STATUS_INIT)
	{
		 if(WIFI_CTX.tcp_cloud_status < STATUS_INIT)
		 {
				close(WIFI_CTX.tcp_cloud_sockfd);
				WIFI_CTX.tcp_cloud_status = STATUS_INIT;
				WIFI_CTX.tcp_cloud_sockfd = INVAILD_SOCK_FD; //close tcp ,for next reconnect.
		 }
		 return ;
	}
	ret = recv(WIFI_CTX.tcp_cloud_sockfd, thisHandle->RecData, MAX_RECV_PACKET_LEN, 0);
	if (ret <= 0 )
	{
		if(ret ==RW_ERR_SOCKET_INVAILD){
			RTE_Printf("[TCPCLIENT]    recv fd = %u  disconnect \n", WIFI_CTX.tcp_cloud_sockfd);
			WIFI_CTX.tcp_cloud_sockfd = INVAILD_SOCK_FD; //close tcp ,for next reconnect.
		}
	}
	else
	{
		thisHandle->Reclen = ret;
		RTE_Printf("[TCPCLIENT]    recv packets on sockfd=%d  data_len=%d\n", WIFI_CTX.tcp_cloud_sockfd,ret );  
		thisHandle->SendRecvCnt += ret;
	}
}
int WIFI_TCPClient_Send(WIFI_TCPClient_Handle_t *thisHandle,uint8_t *SendData,uint16_t SendLength)
{
	int ret;
	uint16_t RealSendLen = 0;
	RealSendLen = SendLength >MAX_SEND_PACKET_LEN ? MAX_SEND_PACKET_LEN :SendLength;
	ret = send(WIFI_CTX.tcp_cloud_sockfd,SendData, RealSendLen, 0);
	if (ret <= 0 )
	{
		RTE_Printf("[TCPCLIENT]    send errorcode =%d\n",ret);
		if(ret ==RW_ERR_SOCKET_INVAILD)
		{
			WIFI_CTX.tcp_cloud_sockfd =INVAILD_SOCK_FD;
		}                  
	}
	else
	{
		RTE_Printf("[TCPCLIENT]    send ok\n");
		thisHandle->SendRecvCnt += ret;
	}
	return ret;
}


int WIFI_TCPServer_Create(WIFI_TCPServer_Handle_t *thisHandle,uint16_t lPort)
{
	if(!thisHandle->RecData)
	{
		thisHandle->RecData = RTE_MEM_Alloc0(MEM_RTE,MAX_RECV_PACKET_LEN);
		RTE_AssertParam(thisHandle->RecData);
	}
	SOCKADDR_IN LocalAddr;
	static int tmp_sockfd = INVAILD_SOCK_FD;    
	int sockfd;
	int ret;
	//filling the TCP server socket address
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port = htons((uint16_t)lPort);
	LocalAddr.sin_addr = 0;
	// creating a TCP socket
	if(tmp_sockfd == INVAILD_SOCK_FD)
	{
		tmp_sockfd = socket(AF_INET,SOCK_STREAM, 0);
		if (tmp_sockfd < RW_OK )
		{
			RTE_MEM_Free(MEM_RTE,thisHandle->RecData);
			return RW_ERR;
		}
	}
	// binding the TCP socket to the TCP server address
	ret = bind(tmp_sockfd, (SOCKADDR_IN *)&LocalAddr, sizeof(SOCKADDR_IN));
	if (ret == RW_ERR || ret == RW_ERR_TIME_OUT || ret ==RW_ERR_SOCKET_INVAILD)
	{
		close(sockfd);
		RTE_MEM_Free(MEM_RTE,thisHandle->RecData);
		return ret;
	}
	// putting the socket for listening to the incoming TCP connection
	ret = listen(tmp_sockfd, 1);
	if (ret == RW_ERR || ret == RW_ERR_TIME_OUT || ret ==RW_ERR_SOCKET_INVAILD)
	{
		close(tmp_sockfd);
		RTE_MEM_Free(MEM_RTE,thisHandle->RecData);
		return ret;
	}
	if (ret == RW_OK) 
	{
		sockfd = tmp_sockfd;
		tmp_sockfd = INVAILD_SOCK_FD;
		RTE_MEM_Free(MEM_RTE,thisHandle->RecData);
		return sockfd;
	}
	return ret;
}
static int WIFI_TCPServer_RecvClients(WIFI_TCPServer_Handle_t *thisHandle,int sever_fd, RW_APP_CTX* app_ctx)
{
	UNUSED(thisHandle);
	SOCKADDR_IN new_tcpinfo;
	int ret = 0;
	int info_size;
	int sockfd;
	int i = 0;
	ret = select(sever_fd, 1);
	if (ret == RW_ERR || ret == RW_ERR_TIME_OUT) 
	{
		return ret;
	}
	sockfd = accept(sever_fd, (SOCKADDR_IN *)&new_tcpinfo, (int*)&info_size);   //the socket as non blocking
	if (sockfd > 0 )
	{
    RTE_Printf("[TCPSERVER]    recv new sockfd=%d from ip=0x%lx ,port=%d\n", sockfd, ntohl(new_tcpinfo.sin_addr),ntohs(new_tcpinfo.sin_port));
		for(i=0; i<ALLOW_MAX_NUMS; i++)
		{
			if(app_ctx->ltcps_clientinfos[i].tcpc_sockfd == -1)
			{
				RTE_Printf("[TCPSERVER]    new client:%d\n",i);
				app_ctx->ltcps_clientinfos[i].tcpc_sockfd =sockfd; 
				app_ctx->ltcps_clientinfos[i].tcpc_info.sin_addr = ntohl(new_tcpinfo.sin_addr);
				app_ctx->ltcps_clientinfos[i].tcpc_info.sin_port = ntohs(new_tcpinfo.sin_port);
				app_ctx->ltcps_clientinfos[i].tcpc_info.sin_family = ntohs(new_tcpinfo.sin_family);
				app_ctx->tcpc_num++;
				break;
			}
		}
		if(i == ALLOW_MAX_NUMS)
		{
			RTE_Printf("[TCPSERVER]    recv too many  connections close it \n\r");
			close(sockfd);
		}             
  }
	else if (sockfd == RW_ERR || sockfd == RW_ERR_TIME_OUT || ret ==RW_ERR_SOCKET_INVAILD) 
	{
		return sockfd;
	}
	return RW_OK;
}
const char HTTPHead[] = 
{
	"HTTP/1.1 200 OK\r\n" \
	"Server: OpenMV\r\n" \
	"Content-Type: multipart/x-mixed-replace;boundary=openmv\r\n" \
	"Cache-Control: no-cache\r\n" \
	"Pragma: no-cache\r\n\r\n"
};
void WIFI_TCPServer_EventHandle(WIFI_TCPServer_Handle_t *thisHandle)
{
	int ret = 0, i=0;
	WIFI_TCPServer_RecvClients(thisHandle,WIFI_CTX.ltcps_sockfd , &WIFI_CTX);
	// send and recv loopback testing
	if (WIFI_CTX.tcpc_num > 0)
	{
		for (i=0; i< ALLOW_MAX_NUMS; i++)
		{
			if (WIFI_CTX.ltcps_clientinfos[i].tcpc_sockfd ==INVAILD_SOCK_FD)
				continue;
			ret = recv(WIFI_CTX.ltcps_clientinfos[i].tcpc_sockfd, thisHandle->RecData, MAX_RECV_PACKET_LEN, 0);
			if (ret <= 0 )
			{
				if (ret ==RW_ERR_SOCKET_INVAILD){
					RTE_Printf("[TCPSERVER]    recv fd = %u  disconnect \n\r", WIFI_CTX.ltcps_clientinfos[i].tcpc_sockfd);
					close(WIFI_CTX.ltcps_clientinfos[i].tcpc_sockfd);
					WIFI_CTX.ltcps_clientinfos[i].tcpc_sockfd =INVAILD_SOCK_FD;
					WIFI_CTX.tcpc_num --;
					WIFI_CTX.ltcps_clientinfos[i].loopback_count = 0;
					if(i == thisHandle->CurrentClient)
						thisHandle->ClientConnect = false;
					break;
				}
			}
			else
			{
				thisHandle->Reclen = ret;
				RTE_Printf("%s\n",thisHandle->RecData);
				RTE_Printf("[TCPSERVER]    recv packets on sockfd=%d  data_len=%d\n", WIFI_CTX.ltcps_clientinfos[i].tcpc_sockfd,ret);   
				if(strstr((char *)thisHandle->RecData,"GET / HTTP/1.1"))
				{
					uint16_t remain = strlen(HTTPHead);
					thisHandle->CurrentClient = i;
					thisHandle->ClientConnect = true;
					WIFI_TCPServer_Send(thisHandle,thisHandle->CurrentClient,(uint8_t *)HTTPHead,strlen(HTTPHead),5);
				}
				WIFI_CTX.ltcps_clientinfos[i].loopback_count += ret;
			}
		}
	}         
}
int WIFI_TCPServer_Send(WIFI_TCPServer_Handle_t *thisHandle,int ClientID,uint8_t *Data,uint16_t DataLen,uint32_t TimeOut)
{
	RTE_AssertParam(WIFI_CTX.tcpc_num>0);
	RTE_AssertParam(ClientID<=ALLOW_MAX_NUMS);
	int bytesremain = DataLen;
	uint8_t *RealData = Data;
	while (bytesremain > 0) 
	{
		int realsend = 0;
		// Split the packet into smaller ones.
		if(bytesremain <= MAX_SEND_PACKET_LEN)
		{
			realsend = bytesremain;
		}
		else
			realsend = MAX_SEND_PACKET_LEN;
		int ret = send(WIFI_CTX.ltcps_clientinfos[ClientID].tcpc_sockfd, RealData , realsend, 0);
		if(ret)
		{
			WIFI_CTX.ltcps_clientinfos[ClientID].loopback_count  -=ret;
			uint32_t StartTick = RTE_RoundRobin_GetTick();
			while(StartTick+TimeOut>=RTE_RoundRobin_GetTick())
			{
				rw_sysDriverLoop();
			}
		}
		else
		{
			RTE_Printf("[TCPSERVER]    send to client:%d errorcode =%d\n",ClientID,ret);
			if(ret ==RW_ERR_SOCKET_INVAILD)
			{
				WIFI_CTX.ltcps_clientinfos[ClientID].tcpc_sockfd =INVAILD_SOCK_FD;
				WIFI_CTX.tcpc_num --;
				WIFI_CTX.ltcps_clientinfos[ClientID].loopback_count = 0;
			}  
			return ret;
		}
    bytesremain -= ret;
		RealData = RealData + ret;
	}
  return DataLen;
}
