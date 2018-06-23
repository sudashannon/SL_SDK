#include "Board_ESP8266.h"
#include "BSP_Com.h"
ESP8266_WifiInfor_t WifiInfor;
bool ATComand_Send(char *SendStr,const char *ExpectRec,uint32_t TimeOut)
{
	uint32_t StartTick = 0;
	uint16_t RecLenth = 0;
	memset(WifiInfor.ESP8266RecBuf,0,ESP8266_BUFLEN);
	BSP_USART_Puts(ESP8266_COM,SendStr);
	StartTick = APP_SoftTimer_GetTick();
	while(StartTick+TimeOut >= APP_SoftTimer_GetTick())
	{
		BSP_USART_Data_t *RecData;
		RecData=BSP_USART_ReturnQue(ESP8266_COM);
		if(APP_MessageQuene_Out(&RecData->ComQuene,(uint8_t *)WifiInfor.ESP8266RecBuf,&RecLenth)==MSG_NO_ERR)
		{
				if(strstr((const char *)WifiInfor.ESP8266RecBuf,ExpectRec))
					return true;
				else
				{
					memset(WifiInfor.ESP8266RecBuf,0,ESP8266_BUFLEN);
					RecLenth = 0;
				}
		}
	}
	return false;
}
void ESP8266WiFiScan(void)
{
	char *SSIDInforStart;
	char *SSIDInforEnd;
	uint8_t SSIDLenth=0;
	if(ATComand_Send("AT+CWLAP\r\n","OK",10000) != true)
		return;
	WifiInfor.WifiCnt  = unumofstr((char *)WifiInfor.ESP8266RecBuf,"+CWLAP:");

	WifiInfor.SSIDInfor.SSIDName = bgetz(WifiInfor.WifiCnt  * sizeof(uint8_t*));
	SSIDInforStart = (char *)WifiInfor.ESP8266RecBuf;
	for(uint8_t i=0;i<WifiInfor.WifiCnt;i++)
	{
		SSIDInforStart = strstr((const char *)SSIDInforStart,"+CWLAP:");
		SSIDInforStart = strstr((const char *)SSIDInforStart,",\"")+2;
		SSIDInforEnd = strstr((const char *)SSIDInforStart,"\",");
		SSIDLenth = SSIDInforEnd-SSIDInforStart;
		WifiInfor.SSIDInfor.SSIDName[i] = bgetz(SSIDLenth);
		if(WifiInfor.SSIDInfor.SSIDName[i] != (void*)0)
			memcpy(WifiInfor.SSIDInfor.SSIDName[i],SSIDInforStart,SSIDLenth);
		printf("[%d] SSID:%s\n",i,WifiInfor.SSIDInfor.SSIDName[i]);
	}
}
void ESP8266WiFiScanRelease(void)
{
	for(uint8_t i=0;i<WifiInfor.WifiCnt;i++)
	{
		if(WifiInfor.SSIDInfor.SSIDName[i])
			brel(WifiInfor.SSIDInfor.SSIDName[i]);
	}
	if(WifiInfor.SSIDInfor.SSIDName)
		brel(WifiInfor.SSIDInfor.SSIDName);
}
void ESP8266IPGet(void)
{
	if(ATComand_Send("AT+CIFSR\r\n","OK",2000) != true)
		return;
	char *IPStart;
	char *IPEnd;
	char IPStringTemp[3];
	IPEnd = strstr((const char *)WifiInfor.ESP8266RecBuf,"+CIFSR:STAIP")+13;
	IPStart = IPEnd+1;
	IPEnd = strstr((const char *)IPStart,".");
	memset(IPStringTemp,0,3);
	memcpy(IPStringTemp,IPStart,IPEnd-IPStart);
	WifiInfor.IP[0] = atoi(IPStringTemp);
	IPStart = IPEnd+1;
	IPEnd = strstr((const char *)IPStart,".");
	memset(IPStringTemp,0,3);
	memcpy(IPStringTemp,IPStart,IPEnd-IPStart);
	WifiInfor.IP[1] = atoi(IPStringTemp);
	IPStart = IPEnd+1;
	IPEnd = strstr((const char *)IPStart,".");
	memset(IPStringTemp,0,3);
	memcpy(IPStringTemp,IPStart,IPEnd-IPStart);
	WifiInfor.IP[2] = atoi(IPStringTemp);
	IPStart = IPEnd+1;
	IPEnd = strstr((const char *)IPStart,"\"");
	memset(IPStringTemp,0,3);
	memcpy(IPStringTemp,IPStart,IPEnd-IPStart);
	WifiInfor.IP[3] = atoi(IPStringTemp);
}
void ESP8266Reset(void)
{
	if(ATComand_Send("AT\r\n","OK",500) == true)
	{
		if(ATComand_Send("AT+RST\r\n","OK",500) == true)
		{
			uint32_t StartTick = 0;
			uint16_t RecLenth = 0;
			StartTick = APP_SoftTimer_GetTick();
			while(StartTick+20000 >= APP_SoftTimer_GetTick())
			{
				BSP_USART_Data_t *RecData;
				RecData=BSP_USART_ReturnQue(ESP8266_COM);
				if(APP_MessageQuene_Out(&RecData->ComQuene,(uint8_t *)WifiInfor.ESP8266RecBuf,&RecLenth)==MSG_NO_ERR)
				{
				}
			}
		}
	}
}
void ESP8266Init(void)
{
	//模块在位检测
	if(ATComand_Send("AT\r\n","OK",500) != true)
		return;
	APP_Printf("[WIFI]模块启动成功!\n");
	//关闭回显
	APP_Printf("[WIFI]关闭回显!\n");
	if(ATComand_Send("ATE0\r\n","OK",500) != true)
		return;
	//设置模块工作模式
	APP_Printf("[WIFI]配置模式!\n");
	if(ATComand_Send("AT+CWMODE_CUR=3\r\n","OK",500) != true)
		return;
	//扫描可用WIFI
	APP_Printf("[WIFI]扫描热点!\n");
	APP_Printf("[RAM_0]Total:%d Used:%d Remain:%d\n",APP_MemGetSize(RAM_0),APP_MemGetUsed(RAM_0),APP_MemGetRemain(RAM_0));
	ESP8266WiFiScan();
	APP_Printf("[RAM_0]Total:%d Used:%d Remain:%d\n",APP_MemGetSize(RAM_0),APP_MemGetUsed(RAM_0),APP_MemGetRemain(RAM_0));
	WifiInfor.ModuleReady = true;
}

ESP8266_Weather_t ESP8266_Weather;
ESP8266_SendState_e ESP8266SendWeatherGet(const char* City) {
	if(WifiInfor.ServerSet == true)
		if(ESP8266DisconnectTCPServer() != true)
			return SERVERMODEEXIT_FAIL;
	if(ATComand_Send("AT+CIPMUX=0\r\n","OK",500) != true)
		return SINGLECONNECTMODE_FAIL;
	if(ATComand_Send("AT+CIPSTART=\"TCP\",\"api.thinkpage.cn\",80\r\n","OK",5000) != true)
		return CONNECTSERVER_FAIL;
  // We now create a URI for the request
  //心知天气
  char *GetString = bgetz(ESP8266_BUFLEN);
	usprintf(GetString,"GET https://api.thinkpage.cn/v3/weather/now.json?key=24qbvr1mjsnukavo&location=%s&language=en HTTP/1.1\r\nHost: api.thinkpage.cn\r\n\r\nConnection: close",City);
	char TempSendStr[20];
	memset(TempSendStr,0,20);
	usprintf(TempSendStr,"AT+CIPSEND=%d\r\n",strlen(GetString));
	if(ATComand_Send(TempSendStr,">",500) != true)
	{
		if(ATComand_Send("AT+CIPCLOSE\r\n","OK",500) != true)
		{
			APP_Printf("%s\n",WifiInfor.ESP8266RecBuf);
			brel(GetString);
			return DISCONNECTSERVER_FAIL;
		}
		brel(GetString);
		return CIPSEND_FAIL;
	}
	if(ATComand_Send(GetString,"SEND OK",500) != true)
	{
		if(ATComand_Send("AT+CIPCLOSE\r\n","OK",500) != true)
		{
			APP_Printf("%s\n",WifiInfor.ESP8266RecBuf);
			brel(GetString);
			return DISCONNECTSERVER_FAIL;
		}
		brel(GetString);
		return DATASEND_FAIL;
	}
	if(strstr((char *)WifiInfor.ESP8266RecBuf,"\"text\""))
	{
		char *StringStart;
		char *StringEnd;
		uint16_t StringLength = 0;
		StringStart = strstr((char *)WifiInfor.ESP8266RecBuf,"\"text\"")+8;
		if(StringStart)
		{
			StringEnd = strstr(StringStart,"\",");
			StringLength = StringEnd-StringStart;
			memset(ESP8266_Weather.Weather,0,20);
			memcpy(ESP8266_Weather.Weather,StringStart,StringLength);
			StringStart = strstr((char *)WifiInfor.ESP8266RecBuf,"\"temperature\"")+15;
			StringEnd = strstr(StringStart,"\"");
			StringLength = StringEnd-StringStart;
			memset(ESP8266_Weather.Temper,0,5);
			memcpy(ESP8266_Weather.Temper,StringStart,StringLength);
			if(strstr((char *)WifiInfor.ESP8266RecBuf,"CLOSED"))
			{
				brel(GetString);
				if(WifiInfor.ServerSet == false)
					if(ESP8266SetTCPServer()!=true)
						return SERVERMODEENTER_FAIL;
				return SEND_OK;
			}
		}
	}
	uint32_t StartTick = 0;
	uint16_t RecLenth = 0;
	StartTick = APP_SoftTimer_GetTick();
	while(StartTick+2000 >= APP_SoftTimer_GetTick())
	{
		BSP_USART_Data_t *RecData;
		memset(WifiInfor.ESP8266RecBuf,0,ESP8266_BUFLEN);
		RecData=BSP_USART_ReturnQue(ESP8266_COM);
		if(APP_MessageQuene_Out(&RecData->ComQuene,(uint8_t *)WifiInfor.ESP8266RecBuf,&RecLenth)==MSG_NO_ERR)
		{
			char *StringStart;
			char *StringEnd;
			uint16_t StringLength = 0;
			StringStart = strstr((char *)WifiInfor.ESP8266RecBuf,"\"text\"")+8;
			if(StringStart)
			{
				StringEnd = strstr(StringStart,"\",");
				StringLength = StringEnd-StringStart;
				memset(ESP8266_Weather.Weather,0,20);
				memcpy(ESP8266_Weather.Weather,StringStart,StringLength);
				StringStart = strstr((char *)WifiInfor.ESP8266RecBuf,"\"temperature\"")+15;
				StringEnd = strstr(StringStart,"\"");
				StringLength = StringEnd-StringStart;
				memset(ESP8266_Weather.Temper,0,5);
				memcpy(ESP8266_Weather.Temper,StringStart,StringLength);
				if(strstr((char *)WifiInfor.ESP8266RecBuf,"CLOSED"))
				{
					brel(GetString);
					if(WifiInfor.ServerSet == false)
						if(ESP8266SetTCPServer()!=true)
							return SERVERMODEENTER_FAIL;
					return SEND_OK;
				}
			}
		}
	}
	if(ATComand_Send("AT+CIPCLOSE\r\n","OK",500) != true)
	{
		APP_Printf("%s\n",WifiInfor.ESP8266RecBuf);
		brel(GetString);
		return DISCONNECTSERVER_FAIL;
	}
	brel(GetString);
	return RECRESPOND_FAIL;
}
ESP8266_SendState_e ESP8266SendTimeGet(void)
{
	if(WifiInfor.ServerSet == true)
		if(ESP8266DisconnectTCPServer() != true)
			return SERVERMODEEXIT_FAIL;
	if(ATComand_Send("AT+CIPMUX=0\r\n","OK",500) != true)
		return SINGLECONNECTMODE_FAIL;
	if(ATComand_Send("AT+CIPSTART=\"TCP\",\"api.k780.com\",80\r\n","OK",5000) != true)
		return CONNECTSERVER_FAIL;
  char *GetString = bgetz(ESP8266_BUFLEN);
	strcat(GetString,"GET http://api.k780.com:88/?app=life.time&appkey=32791&sign=b90250a06b5e6ee53315ec0d6fd2b1ad&format=json HTTP/1.0\r\n\r\n");
	char TempSendStr[50];
	memset(TempSendStr,0,50);
	usprintf(TempSendStr,"AT+CIPSEND=%d\r\n",strlen(GetString));
	if(ATComand_Send(TempSendStr,">",500) != true)
	{
		if(ATComand_Send("AT+CIPCLOSE\r\n","OK",500) != true)
		{
			APP_Printf("%s\n",WifiInfor.ESP8266RecBuf);
			brel(GetString);
			return DISCONNECTSERVER_FAIL;
		}
		brel(GetString);
		return CIPSEND_FAIL;
	}
	if(ATComand_Send(GetString,"SEND OK",500) != true)
	{
		if(ATComand_Send("AT+CIPCLOSE\r\n","OK",500) != true)
		{
			APP_Printf("%s\n",WifiInfor.ESP8266RecBuf);
			brel(GetString);
			return DISCONNECTSERVER_FAIL;
		}
		brel(GetString);
		return DATASEND_FAIL;
	}
	if(strstr((char *)WifiInfor.ESP8266RecBuf,"\"datetime_1\""))
	{
		char *StringStart;
		char *StringEnd;
		uint16_t StringLength = 0;
		uint8_t TempTime[5];
		StringStart = strstr((char *)WifiInfor.ESP8266RecBuf,"\"datetime_1\"")+14;
		if(StringStart)
		{
			StringEnd = strstr(StringStart," ");
			StringLength = StringEnd-StringStart;
			memset(ESP8266_Weather.Date,0,20);
			memcpy(ESP8266_Weather.Date,StringStart,StringLength);
			StringStart = strstr((char *)StringStart," ")+1;
			StringEnd = strstr(StringStart,":");
			StringLength = StringEnd-StringStart;
			memset(TempTime,0,5);
			memcpy(TempTime,StringStart,StringLength);
			ESP8266_Weather.Hour = atoi((char *)TempTime);
			StringStart = StringEnd+1;
			StringEnd = strstr(StringStart,":");
			StringLength = StringEnd-StringStart;
			memset(TempTime,0,5);
			memcpy(TempTime,StringStart,StringLength);
			ESP8266_Weather.Min = atoi((char *)TempTime);
			StringStart = StringEnd+1;
			StringEnd = strstr(StringStart,"\",");
			StringLength = StringEnd-StringStart;
			memset(TempTime,0,5);
			memcpy(TempTime,StringStart,StringLength);
			ESP8266_Weather.Second = atoi((char *)TempTime);	
			if(strstr((char *)WifiInfor.ESP8266RecBuf,"CLOSED"))
			{
				brel(GetString);
				if(WifiInfor.ServerSet == false)
					if(ESP8266SetTCPServer()!=true)
						return SERVERMODEENTER_FAIL;
				return SEND_OK;
			}
		}
	}
	uint32_t StartTick = 0;
	uint16_t RecLenth = 0;
	StartTick = APP_SoftTimer_GetTick();
	while(StartTick+2000 >= APP_SoftTimer_GetTick())
	{
		BSP_USART_Data_t *RecData;
		RecData=BSP_USART_ReturnQue(ESP8266_COM);
		memset(WifiInfor.ESP8266RecBuf,0,ESP8266_BUFLEN);
		if(APP_MessageQuene_Out(&RecData->ComQuene,(uint8_t *)WifiInfor.ESP8266RecBuf,&RecLenth)==MSG_NO_ERR)
		{
			char *StringStart;
			char *StringEnd;
			uint16_t StringLength = 0;
			uint8_t TempTime[5];
			StringStart = strstr((char *)WifiInfor.ESP8266RecBuf,"\"datetime_1\"")+14;
			if(StringStart)
			{
				StringEnd = strstr(StringStart," ");
				StringLength = StringEnd-StringStart;
				memset(ESP8266_Weather.Date,0,20);
				memcpy(ESP8266_Weather.Date,StringStart,StringLength);
				StringStart = strstr((char *)StringStart," ")+1;
				StringEnd = strstr(StringStart,":");
				StringLength = StringEnd-StringStart;
				memset(TempTime,0,5);
				memcpy(TempTime,StringStart,StringLength);
				ESP8266_Weather.Hour = atoi((char *)TempTime);
				StringStart = StringEnd+1;
				StringEnd = strstr(StringStart,":");
				StringLength = StringEnd-StringStart;
				memset(TempTime,0,5);
				memcpy(TempTime,StringStart,StringLength);
				ESP8266_Weather.Min = atoi((char *)TempTime);
				StringStart = StringEnd+1;
				StringEnd = strstr(StringStart,"\",");
				StringLength = StringEnd-StringStart;
				memset(TempTime,0,5);
				memcpy(TempTime,StringStart,StringLength);
				ESP8266_Weather.Second = atoi((char *)TempTime);	
				if(strstr((char *)WifiInfor.ESP8266RecBuf,"CLOSED"))
				{
					brel(GetString);
					if(WifiInfor.ServerSet == false)
						if(ESP8266SetTCPServer()!=true)
							return SERVERMODEENTER_FAIL;
					return SEND_OK;
				}
			}
		}
	}
	if(ATComand_Send("AT+CIPCLOSE\r\n","OK",500) != true)
	{
		APP_Printf("%s\n",WifiInfor.ESP8266RecBuf);
		brel(GetString);
		return DISCONNECTSERVER_FAIL;
	}
	brel(GetString);
	return RECRESPOND_FAIL;
}
bool ESP8266SetTCPServer(void)
{
	if(ATComand_Send("AT+CIPMUX=1\r\n","OK",500) != true)
		return false;
	if(ATComand_Send("AT+CIPSERVER=1,8888\r\n","OK",1000) != true)
		return false;
	WifiInfor.ServerSet = true;
	return true;
}
void ESP8266TCPServerPoll(void)
{
	uint16_t RecLenth = 0;
	BSP_USART_Data_t *RecData;
	RecData=BSP_USART_ReturnQue(ESP8266_COM);
	memset(WifiInfor.ESP8266RecBuf,0,ESP8266_BUFLEN);
	if(APP_MessageQuene_Out(&RecData->ComQuene,(uint8_t *)WifiInfor.ESP8266RecBuf,&RecLenth)==MSG_NO_ERR)
	{
		if(WifiInfor.ClientConnect == false)
		{
			if(strstr((char *)WifiInfor.ESP8266RecBuf,"CONNECT"))
			{
				if(WifiInfor.ESP8266RecBuf[1]==',')
					WifiInfor.ConnectClientNum = (uint8_t)(WifiInfor.ESP8266RecBuf[0] - '0');
				else if(WifiInfor.ESP8266RecBuf[2]==',')
				{
					char Temp[2];
					Temp[0] = WifiInfor.ESP8266RecBuf[0];
					Temp[1] = WifiInfor.ESP8266RecBuf[1];
					WifiInfor.ConnectClientNum = atoi(Temp);
				}
				WifiInfor.ClientConnect = true;
			}
		}
		else if(WifiInfor.ClientConnect == true)
		{
			if(strstr((char *)WifiInfor.ESP8266RecBuf,"CLOSED"))
			{
				WifiInfor.ClientConnect = false;
				return;
			}
		}
	}
}
bool ESP8266DisconnectTCPServer(void)
{
	if(ATComand_Send("AT+CIPSERVER=0\r\n","OK",1000) != true)
		return false;
	WifiInfor.ServerSet = false;
	WifiInfor.ClientConnect = false;
	return true;
}
bool ESP8266WifiConnect(uint8_t *SSID,uint8_t *Password)
{
	uint8_t *SendBuf;
	SendBuf = bgetz(128);
	sprintf((char *)SendBuf,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",
		SSID,Password);
	printf("%s",SendBuf);
	if(ATComand_Send((char *)SendBuf,"OK",10000) != true)
	{
		printf("%s\n",WifiInfor.ESP8266RecBuf);
		brel(SendBuf);
		return false;
	}
	brel(SendBuf);
	return true;
}

/******************************************************************* 
*  Copyright(c) 2018-2028 Suda Shannon 
*  All rights reserved. 
*   
*  文件名称: Board_ESP8266.c
*  简要描述: 适用于AT固件的8266模块驱动
*   
*  当前版本:0.0 
*  作者: 单雷
*  日期: 2018.6.8
*  说明: 第一版
*   
*  取代版本:0.1
*  作者: 
*  日期: 
*  说明: 
******************************************************************/ 
/** 
*  功能描述: 利用SL_LIB功能发送AT指令并判断返回结果 使用操作系统时非阻塞 不使用操作系统时阻塞
*  @param ESP_General_Control_t *Handle ESP控制句柄 
*  @param char *SendStr 待发送AT指令 
*  @param const char *ExpectRec 期待结果 
*  @param uint32_t TimeOut 超时时间 
*  @return ESP_ATComSend_State_e 发送状态
*/  
static ESP_ATComSend_State_e ESP_ATComand_Send(ESP_General_Control_t *Handle,char *SendStr,const char *ExpectRec,uint32_t TimeOut)
{
	uint32_t StartTick = 0;
	uint16_t RecLenth = 0;
	BSP_USART_Data_t *RecData;
	RecData = BSP_USART_ReturnQue(ESP_COM);
	if(Handle->ESP_RecBuf == (void *)0)
		return AT_RECBUF_ERROR;
	BSP_USART_Puts(ESP_COM,SendStr);
	StartTick = APP_SoftTimer_GetTick();
	while(StartTick+TimeOut >= APP_SoftTimer_GetTick())
	{
		memset(Handle->ESP_RecBuf,0,ESP_BUFLEN);
		RecLenth = 0;
		if(APP_MessageQuene_Out(&RecData->ComQuene,(uint8_t *)Handle->ESP_RecBuf,&RecLenth)==MSG_NO_ERR)
		{
			if(strstr((const char *)Handle->ESP_RecBuf,ExpectRec))
				return AT_SEND_OK;
		}
	}
	return AT_SEND_TIMEOUT;
}
/** 
*  功能描述: 初始化一个ESP控制句柄
*  @param ESP_General_Control_t* ESP控制句柄 
*  @param uint16_t BuffSize ESP控制句柄接收缓冲大小
*  @return bool
*/  
bool ESP_InitControlHandle(ESP_General_Control_t *Handle,uint16_t BuffSize)
{
	Handle->ESP_RecBuf = bgetz(BuffSize);
	if(Handle->ESP_RecBuf)
		return true;
	return false;
}
/** 
*  功能描述: 获得当前ESP工作模式
*  @param ESP_General_Control_t *Handle ESP控制句柄 
*  @return ESP_Mode_e 工作模式
*/ 
ESP_Mode_e ESP_GetWorkMode(ESP_General_Control_t *Handle)
{
	if(ESP_ATComand_Send(Handle,"AT+CWMODE?\r\n","OK",200)!=AT_SEND_OK)
		return UNKNOWN_MODE;
	char *ModeChar = uposofseg("+CWMODE:",8,(char *)Handle->ESP_RecBuf);
	switch(*ModeChar)
	{
		case '1':
			return ESP_STATION;
		case '2':
			return ESP_AP;
		case '3':
			return ESP_MIX;
	}
	return UNKNOWN_MODE;
}
/** 
*  功能描述: 设置当前ESP工作模式 不保存到FLASH
*  @param ESP_General_Control_t *Handle ESP控制句柄 
*  @param ESP_Mode_e Mode
*  @return bool 
*/ 
bool ESP_SetWorkMode(ESP_General_Control_t *Handle,ESP_Mode_e Mode)
{
	switch(Mode)
	{
		case ESP_STATION:
		{
			if(ESP_ATComand_Send(Handle,"AT+CWMODE_CUR=1\r\n","OK",200)!=AT_SEND_OK)
			{
				printf("%s\n",Handle->ESP_RecBuf);
				return false;
			}
		}return true;
		case ESP_AP:
		{
			if(ESP_ATComand_Send(Handle,"AT+CWMODE_CUR=2\r\n","OK",200)!=AT_SEND_OK)
			{
				printf("%s\n",Handle->ESP_RecBuf);
				return false;
			}
		}return true;
		case ESP_MIX:
		{
			if(ESP_ATComand_Send(Handle,"AT+CWMODE_CUR=3\r\n","OK",200)!=AT_SEND_OK)
			{
				printf("%s\n",Handle->ESP_RecBuf);
				return false;
			}
		}return true;
		default:
			break;
	}
	return false;
}
/** 
*  功能描述: ESP相关配置
*  @param ESP_General_Control_t *Handle ESP控制句柄 
*  @param ESP_ATComTypes_e Command 需要配置的内容
*  @return ESP_Config_State_t 配置结果 
*/ 
ESP_Config_State_t ESP_Config(ESP_General_Control_t *Handle,ESP_ATComTypes_e Command)
{
	switch(Command)
	{
		case ESP_CMD_RESET:
		{
			while(ESP_ATComand_Send(Handle,"AT+RST\r\n","OK",200)!=AT_SEND_OK)
			{
				APP_SoftTimer_DelayMS(500);
			}
			uint32_t StartTick = 0;
			uint16_t RecLenth = 0;
			BSP_USART_Data_t *RecData;
			RecData = BSP_USART_ReturnQue(ESP_COM);
			StartTick = APP_SoftTimer_GetTick();
			while(StartTick+5000 >= APP_SoftTimer_GetTick())
			{
				memset(Handle->ESP_RecBuf,0,ESP_BUFLEN);
				RecLenth = 0;
				if(APP_MessageQuene_Out(&RecData->ComQuene,(uint8_t *)Handle->ESP_RecBuf,&RecLenth)==MSG_NO_ERR)
				{
				}
			}
		}return CONFIG_OK;
		case ESP_CMD_RESTORE:
		{
			if(ESP_ATComand_Send(Handle,"AT+RESTORE\r\n","ready",2500)!=AT_SEND_OK)
				return CONFIG_FAILED;
		}return CONFIG_OK;
		case ESP_CMD_ATE0:
		{
			if(ESP_ATComand_Send(Handle,"ATE0\r\n","OK",200)!=AT_SEND_OK)
				return CONFIG_FAILED;
		}return CONFIG_OK;
		case ESP_CMD_ATE1:
		{
			if(ESP_ATComand_Send(Handle,"ATE1\r\n","OK",200)!=AT_SEND_OK)
				return CONFIG_FAILED;
		}return CONFIG_OK;
		case ESP_CMD_GMR:
		{
			if(ESP_ATComand_Send(Handle,"AT+GMR\r\n","OK",200)!=AT_SEND_OK)
				return CONFIG_FAILED;
		}return CONFIG_OK;
		case ESP_CMD_SYSMSG_CUR:
		{
			if(ESP_ATComand_Send(Handle,"AT+SYSMSG_CUR=3\r\n","OK",200)!=AT_SEND_OK)
				return CONFIG_FAILED;
		}return CONFIG_OK;
		case ESP_CMD_WIFI_CWLAPOPT:
		{
			if(ESP_ATComand_Send(Handle,"AT+CWLAPOPT=1,14\r\n","OK",200)!=AT_SEND_OK)
				return CONFIG_FAILED;
		}return CONFIG_OK;
		default:
			return UNKNOWN_CMD;
	}
	
	
	
}
/** 
*  功能描述: 扫描当前ESP范围内热点
*  @param ESP_General_Control_t *Handle ESP控制句柄 
*  @return bool 
*/ 
bool ESP_ScanWIFI(ESP_General_Control_t *Handle)
{
	if(ESP_ATComand_Send(Handle,"AT+CWLAP\r\n","OK",5000)!=AT_SEND_OK)
		return false;
	Handle->ESP_ScanInfor.SSIDCnt = unumofstr((char *)Handle->ESP_RecBuf,"+CWLAP:");
	Handle->ESP_ScanInfor.SSIDName = bgetz(Handle->ESP_ScanInfor.SSIDCnt * sizeof(uint8_t*));
	Handle->ESP_ScanInfor.SSIDMac = bgetz(Handle->ESP_ScanInfor.SSIDCnt * sizeof(uint8_t*));
	Handle->ESP_ScanInfor.SSIDSignalStrength = bgetz(Handle->ESP_ScanInfor.SSIDCnt * sizeof(int8_t));
	char *SSIDInforStart = (char *)Handle->ESP_RecBuf;
	char *SSIDInforEnd;
	uint8_t SSIDLenth=0;
	for(uint8_t i=0;i<Handle->ESP_ScanInfor.SSIDCnt;i++)
	{
		SSIDInforStart = uposofseg("+CWLAP:",9,SSIDInforStart);
		SSIDInforEnd = strstr((const char *)SSIDInforStart,"\",");
		SSIDLenth = SSIDInforEnd-SSIDInforStart;
		Handle->ESP_ScanInfor.SSIDName[i] = bgetz(SSIDLenth+1);
		if(Handle->ESP_ScanInfor.SSIDName[i] != (void*)0)
			memcpy(Handle->ESP_ScanInfor.SSIDName[i],SSIDInforStart,SSIDLenth);
		Handle->ESP_ScanInfor.SSIDName[i][SSIDLenth] = '\0';
		Handle->ESP_ScanInfor.SSIDSignalStrength[i] = uparsenumber(&SSIDInforEnd);
		
		SSIDInforStart = SSIDInforEnd+1;
		SSIDInforEnd = strstr((const char *)SSIDInforStart,"\"");
		SSIDLenth = SSIDInforEnd-SSIDInforStart;		
		Handle->ESP_ScanInfor.SSIDMac[i] = bgetz(SSIDLenth+1);
		if(Handle->ESP_ScanInfor.SSIDMac[i] != (void*)0)
			memcpy(Handle->ESP_ScanInfor.SSIDMac[i],SSIDInforStart,SSIDLenth);
		Handle->ESP_ScanInfor.SSIDMac[i][SSIDLenth] = '\0';
		printf("[%d]Power:%d SSID:%s Mac:%s\n",i,Handle->ESP_ScanInfor.SSIDSignalStrength[i],Handle->ESP_ScanInfor.SSIDName[i]
			,Handle->ESP_ScanInfor.SSIDMac[i]);
	}
	return true;
}
/** 
*  功能描述: 连接指定热点
*  @param ESP_General_Control_t *Handle ESP控制句柄 
*  @return bool 
*/ 
bool ESP_ConnectWifi(ESP_General_Control_t *Handle,uint8_t *SSID,uint8_t *Password)
{
	uint8_t SendBuf[128];
	sprintf((char *)SendBuf,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",
		SSID,Password);
	if(ESP_ATComand_Send(Handle,(char *)SendBuf,"OK",10000) != AT_SEND_OK)
	{
		
		return false;
	}
	return true;
}

bool ESP_DisconnectWifi(ESP_General_Control_t *Handle)
{
	if(ESP_ATComand_Send(Handle,"AT+CWQAP","OK",5000) != AT_SEND_OK)
	{
		return false;
	}
	return true;
}

bool ESP_GetIP(ESP_General_Control_t *Handle)
{
	if(ESP_ATComand_Send(Handle,"AT+CIFSR\r\n","OK",2000) != AT_SEND_OK)
		return false;
	char *IPStart;
	char *IPEnd;
	char IPStringTemp[3];
	IPEnd = strstr((const char *)Handle->ESP_RecBuf,"+CIFSR:STAIP")+13;
	IPStart = IPEnd+1;
	IPEnd = strstr((const char *)IPStart,".");
	memset(IPStringTemp,0,3);
	memcpy(IPStringTemp,IPStart,IPEnd-IPStart);
	Handle->ESP_TCPServerInfor.ServerIP[0] = atoi(IPStringTemp);
	IPStart = IPEnd+1;
	IPEnd = strstr((const char *)IPStart,".");
	memset(IPStringTemp,0,3);
	memcpy(IPStringTemp,IPStart,IPEnd-IPStart);
	Handle->ESP_TCPServerInfor.ServerIP[1] = atoi(IPStringTemp);
	IPStart = IPEnd+1;
	IPEnd = strstr((const char *)IPStart,".");
	memset(IPStringTemp,0,3);
	memcpy(IPStringTemp,IPStart,IPEnd-IPStart);
	Handle->ESP_TCPServerInfor.ServerIP[2] = atoi(IPStringTemp);
	IPStart = IPEnd+1;
	IPEnd = strstr((const char *)IPStart,"\"");
	memset(IPStringTemp,0,3);
	memcpy(IPStringTemp,IPStart,IPEnd-IPStart);
	Handle->ESP_TCPServerInfor.ServerIP[3] = atoi(IPStringTemp);
	printf("%d.%d.%d.%d\n",
		Handle->ESP_TCPServerInfor.ServerIP[0],Handle->ESP_TCPServerInfor.ServerIP[1],Handle->ESP_TCPServerInfor.ServerIP[2],Handle->ESP_TCPServerInfor.ServerIP[3]);
	return true;
}

bool ESP_SetTCPServer(ESP_General_Control_t *Handle,uint16_t PortNum)
{
	if(Handle->ESP_TCPServerInfor.TCPServerState == true)
		return false;
	Handle->ESP_TCPServerInfor.TCPServerState = true;
	return true;
}

bool ESP_CancelTCPServer(ESP_General_Control_t *Handle)
{
	if(Handle->ESP_TCPServerInfor.TCPServerState == false)
		return false;
	if(ESP_ATComand_Send(Handle,"AT+CIPSERVER=0\r\n","OK",2000) != AT_SEND_OK)
		return false;
	Handle->ESP_TCPServerInfor.TCPServerState = false;
	return true;
}

bool ESP_ConnectServerAsClient(ESP_General_Control_t *Handle,uint8_t *Server,uint16_t PortNum)
{
	if(Handle->ESP_TCPClientInfor.TCPClientState == true)
		return false;
	uint8_t SendBuf[128];
	sprintf((char *)SendBuf,"AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",
		Server,PortNum);
	if(ESP_ATComand_Send(Handle,(char *)SendBuf,"OK",500) != AT_SEND_OK)
	{
		return false;
	}
	
	Handle->ESP_TCPClientInfor.TCPClientState = true;
	return true;
}

bool ESP_SendDataAsServer(ESP_General_Control_t *Handle,uint8_t *Data,uint16_t Length)
{
	if(Handle->ESP_TCPServerInfor.TCPServerState != true)
		return false;
	return true;
}

bool ESP_SendDataAsClient(ESP_General_Control_t *Handle,uint8_t *Data,uint16_t Length)
{
	if(Handle->ESP_TCPClientInfor.TCPClientState != true)
		return false;
	uint8_t SendBuf[128];
	sprintf((char *)SendBuf,"AT+CIPSEND=%d\r\n",
		Length);
	if(ESP_ATComand_Send(Handle,(char *)SendBuf,">",500) != AT_SEND_OK)
	{
		return false;
	}
	if(ESP_ATComand_Send(Handle,(char *)Data,"SEND OK",10000) != AT_SEND_OK)
	{
		return false;
	}
	return true;
}
uint8_t* ESP_ParserDataAsClient(ESP_General_Control_t *Handle,uint16_t *RecDataLenth)
{
	char *strstart = uposofseg("+IPD,",5,(char *)Handle->ESP_RecBuf);
	*RecDataLenth = uparsenumber(&strstart);
	return (uint8_t *)(++strstart);
}
uint8_t* ESP_ParserDataAsServer(ESP_General_Control_t *Handle,uint16_t *RecDataLenth)
{
	char *strstart = uposofseg("+IPD,",5,(char *)Handle->ESP_RecBuf);
	*RecDataLenth = uparsenumber(&strstart);
	return (uint8_t *)(++strstart);
}
static uint8_t* ESP_ParserData(ESP_General_Control_t *Handle,uint16_t *DataLenth,uint8_t Type)
{
	switch(Type)
	{
		case 0:
		{
			if(Handle->ESP_TCPClientInfor.TCPClientState == true)
				return ESP_ParserDataAsClient(Handle,DataLenth);
			else
				return NULL;
		}
		case 1:
		{
			if(Handle->ESP_TCPServerInfor.TCPServerState == true)
				return ESP_ParserDataAsServer(Handle,DataLenth);
			else
				return NULL;
		};
		default:
			return NULL;
	}
}

ESP_Receive_State_t ESP_ReceiveData(ESP_General_Control_t *Handle,uint8_t *Buffer,uint16_t ReadLength,uint32_t Timeout)
{
	uint16_t RecLenth = 0;
	uint32_t StartTick =0;
	BSP_USART_Data_t *RecData;
	RecData = BSP_USART_ReturnQue(ESP_COM);
	StartTick = APP_SoftTimer_GetTick();
	if(Timeout!=ESP_MAX_WAIT)
	{
    while(StartTick + Timeout >= APP_SoftTimer_GetTick())
		{
			if(Handle->ESP_RealDataLength == 0)
			{
				memset(Handle->ESP_RecBuf,0,ESP_BUFLEN);
				RecLenth = 0;
				if(APP_MessageQuene_Out(&RecData->ComQuene,(uint8_t *)Handle->ESP_RecBuf,&RecLenth)==MSG_NO_ERR)
				{
					uint16_t DataLenth = 0;
					uint8_t* Data = ESP_ParserData(Handle,&DataLenth,0);
					if(Data!=(void *)0)
					{
						Handle->ESP_RealData = bgetz(DataLenth);
						memcpy(Handle->ESP_RealData,Data,DataLenth);
						Handle->ESP_RealDataLength = DataLenth;
						if(ReadLength <= Handle->ESP_RealDataLength)
						{
							memcpy(Buffer,Handle->ESP_RealData+Handle->ESP_RealDataAdd,ReadLength);
							Handle->ESP_RealDataLength = Handle->ESP_RealDataLength-ReadLength;
							Handle->ESP_RealDataAdd = Handle->ESP_RealDataAdd+ReadLength;
						}
						else
						{
							memcpy(Buffer,Handle->ESP_RealData,Handle->ESP_RealDataLength);
							Handle->ESP_RealDataLength = 0;
							Handle->ESP_RealDataAdd = 0;
							brel(Handle->ESP_RealData);
						}
						return REC_OK;
					}
				}
			}
			else
			{
				if(ReadLength > Handle->ESP_RealDataLength)
				{
					memcpy(Buffer,Handle->ESP_RealData+Handle->ESP_RealDataAdd,Handle->ESP_RealDataLength);
					Handle->ESP_RealDataLength = 0;
					Handle->ESP_RealDataAdd = 0;
					brel(Handle->ESP_RealData);
				}
				else
				{
					memcpy(Buffer,Handle->ESP_RealData+Handle->ESP_RealDataAdd,ReadLength);
					Handle->ESP_RealDataLength = Handle->ESP_RealDataLength-ReadLength;
					Handle->ESP_RealDataAdd = Handle->ESP_RealDataAdd+ReadLength;
					if(Handle->ESP_RealDataLength <= 0)
					{
						Handle->ESP_RealDataLength = 0;
						Handle->ESP_RealDataAdd = 0;
						brel(Handle->ESP_RealData);
					}
				}
				return REC_OK;
			}
		}
		return REC_TIMEOUT;
	}
	else
	{
		while(true)
		{
			if(Handle->ESP_RealDataLength == 0)
			{
				memset(Handle->ESP_RecBuf,0,ESP_BUFLEN);
				RecLenth = 0;
				if(APP_MessageQuene_Out(&RecData->ComQuene,(uint8_t *)Handle->ESP_RecBuf,&RecLenth)==MSG_NO_ERR)
				{
					uint16_t DataLenth = 0;
					uint8_t* Data = ESP_ParserData(Handle,&DataLenth,0);
					if(Data!=(void *)0)
					{
						Handle->ESP_RealData = bgetz(DataLenth);
						memcpy(Handle->ESP_RealData,Data,DataLenth);
						Handle->ESP_RealDataLength = DataLenth;
						if(ReadLength <= Handle->ESP_RealDataLength)
						{
							memcpy(Buffer,Handle->ESP_RealData+Handle->ESP_RealDataAdd,ReadLength);
							Handle->ESP_RealDataLength = Handle->ESP_RealDataLength-ReadLength;
							Handle->ESP_RealDataAdd = Handle->ESP_RealDataAdd+ReadLength;
						}
						else
						{
							memcpy(Buffer,Handle->ESP_RealData,Handle->ESP_RealDataLength);
							Handle->ESP_RealDataLength = 0;
							Handle->ESP_RealDataAdd = 0;
							brel(Handle->ESP_RealData);
						}
						return REC_OK;
					}
				}
			}
			else
			{
				if(ReadLength > Handle->ESP_RealDataLength)
				{
					memcpy(Buffer,Handle->ESP_RealData+Handle->ESP_RealDataAdd,Handle->ESP_RealDataLength);
					Handle->ESP_RealDataLength = 0;
					Handle->ESP_RealDataAdd = 0;
					brel(Handle->ESP_RealData);
				}
				else
				{
					memcpy(Buffer,Handle->ESP_RealData+Handle->ESP_RealDataAdd,ReadLength);
					Handle->ESP_RealDataLength = Handle->ESP_RealDataLength-ReadLength;
					Handle->ESP_RealDataAdd = Handle->ESP_RealDataAdd+ReadLength;
					if(Handle->ESP_RealDataLength <= 0)
					{
						Handle->ESP_RealDataLength = 0;
						Handle->ESP_RealDataAdd = 0;
						brel(Handle->ESP_RealData);
					}
				}
				return REC_OK;
			}
		}
	}
}

void ESP_Poll(ESP_General_Control_t *Handle)
{
	
}
