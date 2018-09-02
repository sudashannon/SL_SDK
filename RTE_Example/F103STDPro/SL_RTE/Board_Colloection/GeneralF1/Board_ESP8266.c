#include "Board_ESP8266.h"
#include "APP_SoftTimer.h"
#include "APP_StdLib.h"
#include "BSP_ComF1.h"
#include "Board_Switch.h"
#include "Board_LCD.h"
#include "Board_PM2_5.h"
#include "Board_ADC.h"
#include "Board_DHT11.h"
uint8_t ESP8266RecBuf[ESP8266_BUFLEN];
static int Num0fstr(char *Mstr, char *substr)
{
	int number = 0;
	char *p;//字符串辅助指针
	char *q;//字符串辅助指针
	while(*Mstr != '\0')
	{
		p = Mstr;
		q = substr;
		while((*p == *q)&&(*p!='\0')&&(*q!='\0'))
		{
				p++;
				q++; 
		} 
		if(*q=='\0')
		{
				number++; 
		}
		Mstr++;
	}
	return number;   
}
bool ATComand_Send(char *SendStr,const char *ExpectRec,uint32_t TimeOut)
{
	uint32_t StartTick = 0;
	uint16_t RecLenth = 0;
	memset(ESP8266RecBuf,0,ESP8266_BUFLEN);
	BSP_USART_Puts(ESP8266_COM,SendStr);
	StartTick = APP_SoftTimer_GetTick();
	while(StartTick+TimeOut >= APP_SoftTimer_GetTick())
	{
		BSP_USART_Data_t *RecData;
		RecData=BSP_USART_ReturnQue(ESP8266_COM);
		if(APP_RingBuffer_DeQuene(&RecData->ComQuene,(uint8_t *)ESP8266RecBuf,&RecLenth)==QUE_NO_ERR)
		{
			if(strstr((const char *)ESP8266RecBuf,ExpectRec))
				return true;
			else
			{
				memset(ESP8266RecBuf,0,ESP8266_BUFLEN);
			  RecLenth = 0;
			}
		}
	}
	return false;
}
ESP8266_WifiScanInfor_t WifiScanInfor;
void ESP8266WiFiScan(void)
{
	char *SSIDInforStart;
	char *SSIDInforEnd;
	uint8_t SSIDLenth=0;
	if(ATComand_Send("AT+CWLAP\r\n","OK",10000) != true)
		return;
	WifiScanInfor.WifiCnt  = Num0fstr((char *)ESP8266RecBuf,"+CWLAP:");

	WifiScanInfor.SSIDInfor = bgetz(WifiScanInfor.WifiCnt  * sizeof(ESP8266_SSIDInfor_t));
	SSIDInforStart = (char *)ESP8266RecBuf;
	for(uint8_t i=0;i<WifiScanInfor.WifiCnt;i++)
	{
		SSIDInforStart = strstr((const char *)SSIDInforStart,"+CWLAP:");
		SSIDInforStart = strstr((const char *)SSIDInforStart,",\"")+2;
		SSIDInforEnd = strstr((const char *)SSIDInforStart,"\",");
		SSIDLenth = SSIDInforEnd-SSIDInforStart;
		WifiScanInfor.SSIDInfor[i].SSIDNum = i;
		WifiScanInfor.SSIDInfor[i].SSIDName = bgetz(SSIDLenth);
		if(WifiScanInfor.SSIDInfor[i].SSIDName != (void*)0)
			memcpy(WifiScanInfor.SSIDInfor[i].SSIDName,SSIDInforStart,SSIDLenth);
	}
}
void ESP8266WiFiScanRelease(void)
{
	for(uint8_t i=0;i<WifiScanInfor.WifiCnt;i++)
	{
		if(WifiScanInfor.SSIDInfor[i].SSIDName)
			brel(WifiScanInfor.SSIDInfor[i].SSIDName);
	}
	if(WifiScanInfor.SSIDInfor)
		brel(WifiScanInfor.SSIDInfor);
}
void ESP8266IPGet(void)
{
	if(ATComand_Send("AT+CIFSR\r\n","OK",2000) != true)
		return;
	char *IPStart;
	char *IPEnd;
	char IPStringTemp[3];
	IPEnd = strstr((const char *)ESP8266RecBuf,"+CIFSR:STAIP")+13;
	IPStart = IPEnd+1;
	IPEnd = strstr((const char *)IPStart,".");
	memset(IPStringTemp,0,3);
	memcpy(IPStringTemp,IPStart,IPEnd-IPStart);
	WifiScanInfor.IP[0] = atoi(IPStringTemp);
	IPStart = IPEnd+1;
	IPEnd = strstr((const char *)IPStart,".");
	memset(IPStringTemp,0,3);
	memcpy(IPStringTemp,IPStart,IPEnd-IPStart);
	WifiScanInfor.IP[1] = atoi(IPStringTemp);
	IPStart = IPEnd+1;
	IPEnd = strstr((const char *)IPStart,".");
	memset(IPStringTemp,0,3);
	memcpy(IPStringTemp,IPStart,IPEnd-IPStart);
	WifiScanInfor.IP[2] = atoi(IPStringTemp);
	IPStart = IPEnd+1;
	IPEnd = strstr((const char *)IPStart,"\"");
	memset(IPStringTemp,0,3);
	memcpy(IPStringTemp,IPStart,IPEnd-IPStart);
	WifiScanInfor.IP[3] = atoi(IPStringTemp);
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
				if(APP_RingBuffer_DeQuene(&RecData->ComQuene,(uint8_t *)ESP8266RecBuf,&RecLenth)==QUE_NO_ERR)
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
//	printf("[WIFI]模块启动成功!\n");
	//关闭回显
//	printf("[WIFI]关闭回显!\n");
	if(ATComand_Send("ATE0\r\n","OK",500) != true)
		return;
	//设置模块工作模式
	//printf("[WIFI]配置模式!\n");
	if(ATComand_Send("AT+CWMODE_CUR=3\r\n","OK",500) != true)
		return;
	//扫描可用WIFI
	//printf("[WIFI]扫描热点!\n");
	ESP8266WiFiScan();
}

ESP8266_Weather_t ESP8266_Weather;
ESP8266_SendState_e ESP8266SendWeatherGet(const char* City) {
	if(WifiScanInfor.ServerSet == true)
		if(ESP8266DisconnectTCPServer() != true)
			return SERVERMODEEXIT_FAIL;
	if(ATComand_Send("AT+CIPMUX=0\r\n","OK",500) != true)
		return SINGLECONNECTMODE_FAIL;
	if(ATComand_Send("AT+CIPSTART=\"TCP\",\"api.thinkpage.cn\",80\r\n","OK",5000) != true)
		return CONNECTSERVER_FAIL;
  // We now create a URI for the request
  //心知天气
  char *GetString = bgetz(ESP8266_BUFLEN);
	sprintf(GetString,"GET https://api.thinkpage.cn/v3/weather/now.json?key=24qbvr1mjsnukavo&location=%s&language=en HTTP/1.1\r\nHost: api.thinkpage.cn\r\n\r\nConnection: close",City);
	char TempSendStr[20];
	memset(TempSendStr,0,20);
	sprintf(TempSendStr,"AT+CIPSEND=%d\r\n",strlen(GetString));
	if(ATComand_Send(TempSendStr,">",500) != true)
	{
		if(ATComand_Send("AT+CIPCLOSE\r\n","OK",500) != true)
		{
			printf("%s\n",ESP8266RecBuf);
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
			printf("%s\n",ESP8266RecBuf);
			brel(GetString);
			return DISCONNECTSERVER_FAIL;
		}
		brel(GetString);
		return DATASEND_FAIL;
	}
	if(strstr((char *)ESP8266RecBuf,"\"text\""))
	{
		char *StringStart;
		char *StringEnd;
		uint16_t StringLength = 0;
		StringStart = strstr((char *)ESP8266RecBuf,"\"text\"")+8;
		if(StringStart)
		{
			StringEnd = strstr(StringStart,"\",");
			StringLength = StringEnd-StringStart;
			memset(ESP8266_Weather.Weather,0,20);
			memcpy(ESP8266_Weather.Weather,StringStart,StringLength);
			StringStart = strstr((char *)ESP8266RecBuf,"\"temperature\"")+15;
			StringEnd = strstr(StringStart,"\"");
			StringLength = StringEnd-StringStart;
			memset(ESP8266_Weather.Temper,0,5);
			memcpy(ESP8266_Weather.Temper,StringStart,StringLength);
			if(strstr((char *)ESP8266RecBuf,"CLOSED"))
			{
				brel(GetString);
				if(WifiScanInfor.ServerSet == false)
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
		memset(ESP8266RecBuf,0,ESP8266_BUFLEN);
		RecData=BSP_USART_ReturnQue(ESP8266_COM);
		if(APP_RingBuffer_DeQuene(&RecData->ComQuene,(uint8_t *)ESP8266RecBuf,&RecLenth)==QUE_NO_ERR)
		{
			char *StringStart;
			char *StringEnd;
			uint16_t StringLength = 0;
			StringStart = strstr((char *)ESP8266RecBuf,"\"text\"")+8;
			if(StringStart)
			{
				StringEnd = strstr(StringStart,"\",");
				StringLength = StringEnd-StringStart;
				memset(ESP8266_Weather.Weather,0,20);
				memcpy(ESP8266_Weather.Weather,StringStart,StringLength);
				StringStart = strstr((char *)ESP8266RecBuf,"\"temperature\"")+15;
				StringEnd = strstr(StringStart,"\"");
				StringLength = StringEnd-StringStart;
				memset(ESP8266_Weather.Temper,0,5);
				memcpy(ESP8266_Weather.Temper,StringStart,StringLength);
				if(strstr((char *)ESP8266RecBuf,"CLOSED"))
				{
					brel(GetString);
					if(WifiScanInfor.ServerSet == false)
						if(ESP8266SetTCPServer()!=true)
							return SERVERMODEENTER_FAIL;
					return SEND_OK;
				}
			}
		}
	}
	if(ATComand_Send("AT+CIPCLOSE\r\n","OK",500) != true)
	{
		printf("%s\n",ESP8266RecBuf);
		brel(GetString);
		return DISCONNECTSERVER_FAIL;
	}
	brel(GetString);
	return RECRESPOND_FAIL;
}
ESP8266_SendState_e ESP8266SendTimeGet(void)
{
	if(WifiScanInfor.ServerSet == true)
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
			printf("%s\n",ESP8266RecBuf);
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
			printf("%s\n",ESP8266RecBuf);
			brel(GetString);
			return DISCONNECTSERVER_FAIL;
		}
		brel(GetString);
		return DATASEND_FAIL;
	}
	if(strstr((char *)ESP8266RecBuf,"\"datetime_1\""))
	{
		char *StringStart;
		char *StringEnd;
		uint16_t StringLength = 0;
		uint8_t TempTime[5];
		StringStart = strstr((char *)ESP8266RecBuf,"\"datetime_1\"")+14;
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
			if(strstr((char *)ESP8266RecBuf,"CLOSED"))
			{
				brel(GetString);
				if(WifiScanInfor.ServerSet == false)
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
		memset(ESP8266RecBuf,0,ESP8266_BUFLEN);
		if(APP_RingBuffer_DeQuene(&RecData->ComQuene,(uint8_t *)ESP8266RecBuf,&RecLenth)==QUE_NO_ERR)
		{
			char *StringStart;
			char *StringEnd;
			uint16_t StringLength = 0;
			uint8_t TempTime[5];
			StringStart = strstr((char *)ESP8266RecBuf,"\"datetime_1\"")+14;
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
				if(strstr((char *)ESP8266RecBuf,"CLOSED"))
				{
					brel(GetString);
					if(WifiScanInfor.ServerSet == false)
						if(ESP8266SetTCPServer()!=true)
							return SERVERMODEENTER_FAIL;
					return SEND_OK;
				}
			}
		}
	}
	if(ATComand_Send("AT+CIPCLOSE\r\n","OK",500) != true)
	{
		printf("%s\n",ESP8266RecBuf);
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
	WifiScanInfor.ServerSet = true;
	return true;
}
void ESP8266TCPServerPoll(void)
{
	uint16_t RecLenth = 0;
	BSP_USART_Data_t *RecData;
	RecData=BSP_USART_ReturnQue(ESP8266_COM);
	memset(ESP8266RecBuf,0,ESP8266_BUFLEN);
	if(APP_RingBuffer_DeQuene(&RecData->ComQuene,(uint8_t *)ESP8266RecBuf,&RecLenth)==QUE_NO_ERR)
	{
		if(WifiScanInfor.ClientConnect == false)
		{
			if(strstr((char *)ESP8266RecBuf,"CONNECT"))
			{
				if(ESP8266RecBuf[1]==',')
					WifiScanInfor.ConnectClientNum = (uint8_t)(ESP8266RecBuf[0] - '0');
				else if(ESP8266RecBuf[2]==',')
				{
					char Temp[2];
					Temp[0] = ESP8266RecBuf[0];
					Temp[1] = ESP8266RecBuf[1];
					WifiScanInfor.ConnectClientNum = atoi(Temp);
				}
				WifiScanInfor.ClientConnect = true;
			}
		}
		else if(WifiScanInfor.ClientConnect == true)
		{
			if(strstr((char *)ESP8266RecBuf,"CLOSED"))
			{
				WifiScanInfor.ClientConnect = false;
				return;
			}
			char *StringStart;
			StringStart = (char *)ESP8266RecBuf;
			if(StringStart)
			{
				if(strstr((char *)StringStart,"ControlMode"))
				{
					if(strstr((char *)StringStart,":Auto"))
					{
						if(LCD_ControlHandle.ControlMode == true)
							LCD_ControlHandle.ControlMode = false;
					}
					else if(strstr((char *)StringStart,":Hand"))
					{
						if(LCD_ControlHandle.ControlMode == false)
							LCD_ControlHandle.ControlMode = true;
					}
				}
				else if(strstr((char *)StringStart,"FeedAct"))
				{ 
					if(LCD_ControlHandle.ControlMode == true)
					{
						if(strstr((char *)StringStart,":Close"))
						{
							if(LCD_ControlHandle.FeedStatus == true)
							{
								Board_SwitchOff(SWITCH_0);
								LCD_ControlHandle.FeedStatus = false;
							}
						}
						else if(strstr((char *)StringStart,":Open"))
						{
							if(LCD_ControlHandle.FeedStatus == false)
							{
								Board_SwitchOn(SWITCH_0);
								LCD_ControlHandle.FeedStatus = true;
							}
						}
					}
				}
				else if(strstr((char *)StringStart,"AirCleanAct"))
				{
					if(LCD_ControlHandle.ControlMode == true)
					{
						if(strstr((char *)StringStart,":Close"))
						{
							if(LCD_ControlHandle.AirCleanerStatus == true)
							{
								Board_SwitchOff(SWITCH_1);
								LCD_ControlHandle.AirCleanerStatus = false;
							}
						}
						else if(strstr((char *)StringStart,":Open"))
						{
							if(LCD_ControlHandle.AirCleanerStatus == false)
							{
								Board_SwitchOn(SWITCH_1);
								LCD_ControlHandle.AirCleanerStatus = true;
							}
						}
					}
				}
				else if(strstr((char *)StringStart,"Hum"))
				{
					if(LCD_ControlHandle.ControlMode == true)
					{
						if(strstr((char *)StringStart,":Close"))
						{
							if(LCD_ControlHandle.HumStatus == true)
							{
								Board_SwitchOff(SWITCH_2);
								LCD_ControlHandle.HumStatus = false;
							}
						}
						else if(strstr((char *)StringStart,":Open"))
						{
							if(LCD_ControlHandle.HumStatus == false)
							{
								Board_SwitchOn(SWITCH_2);
								LCD_ControlHandle.HumStatus = true;
							}
						}
					}
				}
				else if(strstr((char *)StringStart,"Temper"))
				{
					if(LCD_ControlHandle.ControlMode == true)
					{
						if(strstr((char *)StringStart,":Close"))
						{
							if(LCD_ControlHandle.TemperStatus == true)
							{
								Board_SwitchOff(SWITCH_3);
								LCD_ControlHandle.TemperStatus = false;
							}
						}
						else if(strstr((char *)StringStart,":Open"))
						{
							if(LCD_ControlHandle.TemperStatus == false)
							{
								Board_SwitchOn(SWITCH_3);
								LCD_ControlHandle.TemperStatus = true;
							}
						}
					}
				}
			}
		}
	}
}
bool ESP8266DisconnectTCPServer(void)
{
	if(ATComand_Send("AT+CIPSERVER=0\r\n","OK",1000) != true)
		return false;
	WifiScanInfor.ServerSet = false;
	WifiScanInfor.ClientConnect = false;
	return true;
}

