#include "Board_WIFI.h"
#include "Board_SPI.h"
#include "Board_HALCallback.h"
RW_APP_CTX        WIFI_CTX;
rw_WlanConnect_t  WIFI_ConnectInfo;
rw_WlanEasyConfigWpsResponse_t       WIFI_EasyWPSInfo;
rw_DriverParams_t WIFIDriver;
rw_IpConfig_t WIFI_APIP;
static void Board_WIFI_IO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	WIFI_RESET_GPIO_CLK_ENABLE();
	WIFI_CS_GPIO_CLK_ENABLE();
	
	GPIO_InitStructure.Pin = WIFI_RESET_PIN;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;  //推挽输出
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;     //高速
  GPIO_InitStructure.Pull = GPIO_PULLUP;          //上拉
  HAL_GPIO_Init(WIFI_RESET_GPIO_PORT,&GPIO_InitStructure);
	HAL_GPIO_WritePin(WIFI_RESET_GPIO_PORT,WIFI_RESET_PIN,GPIO_PIN_SET);	//PB10置1 
	
	GPIO_InitStructure.Pin = WIFI_CS_PIN;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;  //推挽输出
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;     //高速
  GPIO_InitStructure.Pull = GPIO_PULLUP;          //上拉
  HAL_GPIO_Init(WIFI_CS_GPIO_PORT,&GPIO_InitStructure);
	HAL_GPIO_WritePin(WIFI_CS_GPIO_PORT,WIFI_CS_PIN,GPIO_PIN_SET);	//PB12置1 
}
static void Board_WIFI_INT_Init(void)
{
	WIFI_INT_GPIO_CLK_ENABLE();
	GPIO_InitTypeDef  GPIO_InitStructure;
  /*Configure GPIO pin : PtPin */
  GPIO_InitStructure.Pin = WIFI_INT_PIN;
  GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(WIFI_INT_GPIO_PORT, &GPIO_InitStructure);
  /* EXTI interrupt init*/
#if WIFI_IO_INTERRUPT_USE == 1
  HAL_NVIC_SetPriority(WIFI_IO_IRQn, WIFI_IO_PreemptPriority, 0);
  HAL_NVIC_EnableIRQ(WIFI_IO_IRQn);
#endif
}
#if WIFI_IO_INTERRUPT_USE == 1
void WIFI_IO_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(WIFI_INT_PIN);
}
#endif
static void Board_WIFI_SPI_Deinit(void)
{
	__HAL_SPI_DISABLE(&SpiHandle[SPI_WIFI].SpiHalHandle);               //先关闭SPI2
	HAL_SPI_DeInit(&SpiHandle[SPI_WIFI].SpiHalHandle);
}
static uint32_t Board_WIFI_InterfaceInit(void)
{
	Board_WIFI_INT_Init();
	return 0;
}
static uint32_t Board_WIFI_InterfaceDeInit(void)
{
	Board_WIFI_SPI_Deinit();
	return 0;
}
static void Board_WIFI_PowerSet(uint8_t status)
{    
  if (status) 
	{
		HAL_GPIO_WritePin(WIFI_RESET_GPIO_PORT,WIFI_RESET_PIN,GPIO_PIN_SET);
  } 
	else 
	{
		HAL_GPIO_WritePin(WIFI_RESET_GPIO_PORT,WIFI_RESET_PIN,GPIO_PIN_RESET);
		RTE_RoundRobin_DelayMS(200);
  }
}
extern osEventFlagsId_t EvtIDWIFI;
#define WIFI_SPI_DELAY 1
static void Board_WIFI_SPITransRec(uint8_t* write, uint8_t* read, uint16_t len)
{
  uint8_t dummy = 0x00;
  HAL_GPIO_WritePin(WIFI_CS_GPIO_PORT, WIFI_CS_PIN, GPIO_PIN_RESET);
  if(read == NULL) 
	{
		if(write == NULL)  
		{
#if WIFI_SPI_DELAY == 1
			HAL_SPI_Transmit(&SpiHandle[SPI_WIFI].SpiHalHandle,&dummy,len,1000);
#else
			HAL_SPI_Transmit_IT(&SpiHandle[SPI_WIFI].SpiHalHandle,&dummy,len);
			osEventFlagsWait (EvtIDWIFI,0x00000001ul,osFlagsWaitAny, osWaitForever);
#endif
		}
		else 
		{
#if WIFI_SPI_DELAY == 1
			HAL_SPI_Transmit(&SpiHandle[SPI_WIFI].SpiHalHandle,write,len,1000);
#else
			HAL_SPI_Transmit_IT(&SpiHandle[SPI_WIFI].SpiHalHandle,write,len);
			osEventFlagsWait (EvtIDWIFI,0x00000001ul,osFlagsWaitAny, osWaitForever);
#endif
			}
  } 
  else 
	{
		if(write == NULL) 
		{
#if WIFI_SPI_DELAY == 1
			HAL_SPI_Receive(&SpiHandle[SPI_WIFI].SpiHalHandle,read,len,1000);
#else
			HAL_SPI_Receive_IT(&SpiHandle[SPI_WIFI].SpiHalHandle,read,len);
			osEventFlagsWait (EvtIDWIFI,0x00000002ul,osFlagsWaitAny, osWaitForever);
#endif
			}
		else 
		{
#if WIFI_SPI_DELAY == 1
			HAL_SPI_TransmitReceive(&SpiHandle[SPI_WIFI].SpiHalHandle,write,read,len,1000);
#else
			HAL_SPI_TransmitReceive_IT(&SpiHandle[SPI_WIFI].SpiHalHandle,write,read,len);
			osEventFlagsWait (EvtIDWIFI,0x00000003ul,osFlagsWaitAny, osWaitForever);
#endif
			}
  }
	HAL_GPIO_WritePin(WIFI_CS_GPIO_PORT, WIFI_CS_PIN, GPIO_PIN_SET);
}
static void Board_WIFI_Delay(int ms)
{
	RTE_RoundRobin_DelayMS(ms);
}
static uint32_t Board_WIFI_GetTick(void)
{
	return RTE_RoundRobin_GetTick();
}
void Board_WIFI_InterruptSet(uint8_t enable)
{
	if (enable)
	{
#if WIFI_IO_INTERRUPT_USE == 1
		HAL_NVIC_EnableIRQ(WIFI_IO_IRQn);
#endif
	}
	else
	{
#if WIFI_IO_INTERRUPT_USE == 1
		HAL_NVIC_DisableIRQ(WIFI_IO_IRQn);
#endif
	}
}
static void Board_WIFI_Free( void *pv )
{
	RTE_MEM_Free(MEM_RTE,pv);
}
static void *Board_WIFI_Malloc(size_t xWantedSize)
{
	return RTE_MEM_Alloc0(MEM_RTE,xWantedSize);
}
static void Board_WIFI_Assert(const char *file, int line)
{
	RTE_Assert((char*)file,(uint32_t)line);
}
static void User_WIFIScan_Callback(rw_WlanNetworkInfoList_t* scan_info) 
{
	RTE_Printf("[WIFI]    scan num = 0x%x\r\n", scan_info->num);
	Board_WIFI_Free(scan_info->WlanNetworkInfo);
}
static void User_WIFIConnect_Callback(uint8_t event ,  rw_WlanConnect_t* wlan_info, RW_DISCONNECT_REASON dis_code)
{
	RTE_Printf("[WIFI]    connect_callback event = 0x%x\r\n", event);
	switch(event) 
	{
		//连接到路由 建立TCP服务器
		case CONN_STATUS_STA_CONNECTED:
		{
			if (wlan_info != NULL) 
			{
				RTE_Printf("Connected AP info list\r\n");
				RTE_Printf("------------------\r\n");
				RTE_Printf("[WIFI]    bssid = %02X:%02X:%02X:%02X:%02X:%02X\r\n", wlan_info->bssid[0]
																													 , wlan_info->bssid[1] 
																													 , wlan_info->bssid[2]  
																													 , wlan_info->bssid[3]  
																													 , wlan_info->bssid[4]  
																													 , wlan_info->bssid[5] );
				RTE_Printf("[WIFI]    channel = %d\r\n", wlan_info->channel);
				RTE_Printf("[WIFI]    ssid = %s\r\n", wlan_info->ssid);
				RTE_Printf("[WIFI]    psk = %s\r\n", wlan_info->psk);
				RTE_Printf("[WIFI]    sec_mode = %d\r\n", wlan_info->sec_mode);
				RTE_Printf("[WIFI]    auth_mode = %d\r\n", wlan_info->auth_mode);         
			}
			WIFI_CTX.rw_connect_status =STATUS_OK;
			RTE_Printf("[WIFI]    sta wifi connected!\r\n");
		}break;
		//与路由断开连接
		case CONN_STATUS_STA_DISCONNECT:
		{
			WIFI_CTX.rw_connect_status =STATUS_FAIL;
			RTE_Printf("[WIFI]    sta wifi disconnected Code = %d\r\n",dis_code);
		}break;
		//AP建立成功 建立TCP服务器
		case CONN_STATUS_AP_ESTABLISH:
		{
			WIFI_CTX.rw_connect_status = STATUS_OK;
			RTE_Printf("[WIFI]    ap wifi establish\r\n");
		}break;
		//有客户端连接到AP
		case CONN_STATUS_AP_CLT_CONNECTED:
		{
			RTE_Printf("[WIFI]    ap wifi clt connected!\r\n");
		}break;
		//客户端与AP断开连接
		case CONN_STATUS_AP_CLT_DISCONNECT:
			RTE_Printf("[WIFI]    ap wifi clt disconnected Code = %d\r\n",dis_code);
		break;
		default:
			RTE_Printf("[WIFI]    user connect callback err\r\n");
		break;
	}
}
static void User_WIFIDHCP_Callback(rw_IpConfig_t* ip_addr, int status)
{
	if(status ==RW_OK)
	{
		RTE_Printf("[WIFI]    ip get success addr = 0x%x\r\n", ip_addr->addr);
		WIFI_CTX.rw_ipquery_status =STATUS_OK; 
	}
	else
	{   
		RTE_Printf("[WIFI]    ip get fail code = %d\r\n", status);
		WIFI_CTX.rw_ipquery_status =STATUS_FAIL; 
	}
}
static void User_WIFIDNS_Callback(int dns_ip)
{
	if(dns_ip !=0)
		RTE_Printf("[WIFI]    dns success ip addr = 0x%x\r\n", dns_ip);
	else
		RTE_Printf("[WIFI]    dns fail ,please retry!!!\r\n");
}
static void User_WIFI_WPS_EASY_Callback(rw_WlanEasyConfigWpsResponse_t* info , int status)
{
	if (status == 0) 
	{      
		RTE_Printf("[WIFI]    bssid = %02X:%02X:%02X:%02X:%02X:%02X\r\n", info->bssid[0]
																											, info->bssid[1] 
																											, info->bssid[2]  
																											, info->bssid[3]  
																											, info->bssid[4]  
																											, info->bssid[5] );
		RTE_Printf("[WIFI]    channel =%d\r\n", info->channel);
		RTE_Printf("[WIFI]    ssid =%s\r\n", info->ssid);
		RTE_Printf("[WIFI]    psk =%s\r\n", info->psk); 
		WIFI_CTX.rw_easywps_status = STATUS_OK;
		memcpy(&WIFI_EasyWPSInfo,info,sizeof(WIFI_EasyWPSInfo));
		memset(&WIFI_ConnectInfo, 0, sizeof(WIFI_ConnectInfo));
		if(WIFI_CTX.easywps_mode ==CONFIG_WPS)
		{
			WIFI_ConnectInfo.bssid = NULL;
			WIFI_ConnectInfo.channel = 0;
		}
		else
		{
			WIFI_ConnectInfo.bssid = WIFI_EasyWPSInfo.bssid; 
			WIFI_ConnectInfo.channel = WIFI_EasyWPSInfo.channel;
		}         
		WIFI_ConnectInfo.ssid = (char*)WIFI_EasyWPSInfo.ssid; 
		WIFI_ConnectInfo.psk = (char*)WIFI_EasyWPSInfo.psk;
		WIFI_ConnectInfo.pmk = NULL;
		if(info->psk[0] != 0)
		{
			WIFI_ConnectInfo.sec_mode = RW_SEC_TYPE_SEC;
		}
		else
		{
			WIFI_ConnectInfo.sec_mode = RW_SEC_TYPE_OPEN;
		}    
		WIFI_ConnectInfo.auth_mode = RW_AUTH_TYPE_AUTO;
		WIFI_ConnectInfo.role_mode = ROLE_STA;     
	}
	else
	{    
		WIFI_CTX.rw_easywps_status = STATUS_FAIL;
		RTE_Printf("[WIFI]    wps_easy config fail \r\r\n");
	}
}

void Board_WIFI_Driver_Init(rw_DriverParams_t* params)
{
	//平台接口
	params->driver_cb.hw_init = Board_WIFI_InterfaceInit;    
	params->driver_cb.hw_deinit = Board_WIFI_InterfaceDeInit;
  params->driver_cb.hw_power = Board_WIFI_PowerSet;
  params->driver_cb.spi_io_buffer = Board_WIFI_SPITransRec;    
	params->driver_cb.time_delay = Board_WIFI_Delay;
	params->driver_cb.Stamp_get = Board_WIFI_GetTick;   
	params->driver_cb.toggle_irq = Board_WIFI_InterruptSet;   
	params->driver_cb.driver_free = Board_WIFI_Free;
	params->driver_cb.driver_malloc = Board_WIFI_Malloc;
	params->driver_cb.driver_assert = Board_WIFI_Assert;
  //应用回调接口
	params->app_cb.conn_cb = User_WIFIConnect_Callback; 
	params->app_cb.scan_cb = User_WIFIScan_Callback;
#if WIFI_USE_OS == 0
	params->app_cb.dhcp_cb = User_WIFIDHCP_Callback;
	params->app_cb.dns_cb = User_WIFIDNS_Callback;
#endif
	params->app_cb.easy_wps_cb = User_WIFI_WPS_EASY_Callback;
	//中断使能
	params->spi_int_enable = false;
	//RX Buffer个数 一个大小为1664字节
	//适当增加接收buffer的个数可以提高模块双向数据时的吞吐率
	params->rx_queue_num = 5;
	params->tx_queue_num = 5;
	params->tx_max_len = MAX_SEND_PACKET_LEN;
	//扫描buffer个数 一个为44字节
	params->scan_max_num = 10;
	//tcp重传次数
	params->tcp_retry_num = 5;
	//socket个数 一个buffer为48字节
	params->socket_max_num = 8;
	params->country_code = "CN";
	//主机名
	params->host_name = "TuringBoard";
}

int Board_WIFI_Init(void)
{
	Board_WIFI_IO_Init();
	HAL_GPIO_WritePin(WIFI_RESET_GPIO_PORT,WIFI_RESET_PIN,GPIO_PIN_SET);
	RTE_RoundRobin_DelayMS(200);
	HAL_GPIO_WritePin(WIFI_RESET_GPIO_PORT,WIFI_RESET_PIN,GPIO_PIN_RESET);
	RTE_RoundRobin_DelayMS(200);
	HAL_GPIO_WritePin(WIFI_RESET_GPIO_PORT,WIFI_RESET_PIN,GPIO_PIN_SET);
  int ret =0;
  char libVersion[20]="";
  char module_mac[6] ="";
  //rak module driver init
  Board_WIFI_Driver_Init(&WIFIDriver);
  ret = rw_sysDriverInit(&WIFIDriver);
	while(ret != RW_OK)
  {
    RTE_Printf("[WIFI]    RAK module platform init...failed code=%d\r\n", ret);
    ret =rw_sysDriverInit(&WIFIDriver);
		RTE_RoundRobin_DelayMS(200);
  }
	RTE_Printf("[WIFI]    rw_sysDriverInit...success\r\n");
  rw_getLibVersion(libVersion); 
  RTE_Printf("[WIFI]    rak wifi LibVersion:%s\r\n", libVersion);
  rw_getMacAddr(module_mac);
  RTE_Printf("[WIFI]    rak wifi module-MAC:%02X:%02X:%02X:%02X:%02X:%02X\r\n", module_mac[0],module_mac[1],module_mac[2],module_mac[3],module_mac[4],module_mac[5]);
  return RW_OK;
}
