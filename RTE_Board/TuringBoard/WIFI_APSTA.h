#ifndef __WIFI_APP_H
#define __WIFI_APP_H

#define   ALLOW_MAX_NUMS      6
#define   INVAILD_SOCK_FD     -1
typedef enum
{    
  STATUS_FAIL = -1 , 
  STATUS_INIT = 0 ,
  STATUS_OK   = 1
}rw_CmdStatus_t;
typedef struct 
{
  int             tcpc_sockfd;
  int             loopback_count;
  SOCKADDR_IN     tcpc_info;
}TCPC_INFO_T;
typedef struct
{
   int                  rw_connect_status;    //save wifi network connet status
   int                  rw_ipquery_status;    //save wifi network ipquery status
                                              // customer can add needed status here
   rw_ConfigMode_t      easywps_mode;
   int                  rw_easywps_status;    //save easywps cmd excute result success or fail
   int                  easy_sockfd;         //save ludp socket fd for easyconfig to response app
   int                  easy_rsptimeout; 
	
   int                  ltcps_sockfd;        //save local tcp server service socket fd
   int                  tcpc_num;            //current tcp client nums connect to the server 
   TCPC_INFO_T          ltcps_clientinfos[ALLOW_MAX_NUMS];
	
   int                  tcp_cloud_sockfd;    //save cloud tcp client connect socket fd
   int                  tcp_cloud_status;    //save cloud tcp client connect status whether connected
   
   int                  ludps_sockfd;        //save local udp server service socket fd 
   
   int                  udpc_sockfd;    //save cloud tcp client connect socket fd
   int                  udpc_status;
}RW_APP_CTX;
void WIFI_APP_Init(void);
int WIFI_NetWork_Init(rw_WlanConnect_t* pconn, rw_IpConfigMode_t ipmode, rw_IpConfig_t* ipconfig);
int WIFI_Start_AP(void);
int WIFI_Start_Station(void);
int WIFI_Start_SmartConfig(rw_ConfigMode_t mode);
#endif
