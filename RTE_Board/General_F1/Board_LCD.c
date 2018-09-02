#include "Board_LCD.h"
#include "BSP_ComF1.h"
#include "Board_ESP8266.h"
#include "APP_SoftTimer.h"
#include "APP_StdLib.h"
#include "Board_DHT11.h"
#include "Board_ESP8266.h"
#include "Board_Switch.h"
#include "Board_ADC.h"
static uint8_t LCDRecBuf[LCD_COMBUFLEN];
static uint8_t LCDSendBuf[128];
static bool LCDComand_Send(char *SendStr,const char *ExpectRec,uint32_t TimeOut,uint8_t RetryTimes)
{
	uint32_t StartTick = 0;
	uint16_t RecLenth = 0;
	memset(LCDRecBuf,0,LCD_COMBUFLEN);
	for(uint8_t i=0;i<RetryTimes;i++)
	{
		BSP_USART_Puts(LCD_COM,SendStr);
		StartTick = APP_SoftTimer_GetTick();
		while(StartTick+TimeOut >= APP_SoftTimer_GetTick())
		{
			BSP_USART_Data_t *RecData;
			RecData=BSP_USART_ReturnQue(LCD_COM);
			if(APP_RingBuffer_DeQuene(&RecData->ComQuene,(uint8_t *)LCDRecBuf,&RecLenth)==QUE_NO_ERR)
			{
				if(strstr((const char *)LCDRecBuf,ExpectRec))
					return true;
				else
				{
					memset(LCDRecBuf,0,LCD_COMBUFLEN);
					RecLenth = 0;
				}
			}
		}
	}
	return false;
}
static uint8_t LCD_ScanKey(uint8_t *StringInput)
{
	uint16_t RecLenth = 0;
	BSP_USART_Data_t *RecData;
	memset(LCDRecBuf,0,LCD_COMBUFLEN);
	RecData=BSP_USART_ReturnQue(LCD_COM);
	if(APP_RingBuffer_DeQuene(&RecData->ComQuene,(uint8_t *)LCDRecBuf,&RecLenth)==QUE_NO_ERR)
	{
		char *NumStart = strstr((const char *)LCDRecBuf,"[BN:");
		if(NumStart)
		{		
			uint8_t NumString[2];
			memset(NumString,0,2);
			char *NumEnd =  strstr((const char *)NumStart,"]");
			memcpy(NumString,NumStart+4,NumEnd-NumStart-4);
			if(StringInput)
			{
				NumStart = strstr((const char *)LCDRecBuf,"{");
				if(NumStart)
				{
					memset(StringInput,0,(uint8_t)sizeof(StringInput));
					NumEnd =  strstr((const char *)NumStart,"}");
					memcpy(StringInput,NumStart+1,NumEnd-NumStart-1);
				}
			}
			return atoi((char *)NumString);
		}
		else
		{
			memset(LCDRecBuf,0,LCD_COMBUFLEN);
			RecLenth = 0;
		}
	}
	return 0x00;
}

static void LCD_WIFIInforDisplay(uint8_t SubPage)
{
	if((WifiScanInfor.WifiCnt - SubPage*9) >= 9)
	{
		for(uint8_t i=0;i<9;i++)
		{
			memset(LCDSendBuf,0,128);
			sprintf((char *)LCDSendBuf,"W8UE(%d);DS12(2,2,'%d.%s',15);SXY(0,0);\r\n",i+1,WifiScanInfor.SSIDInfor[i+SubPage*9].SSIDNum,WifiScanInfor.SSIDInfor[i+SubPage*9].SSIDName);
			LCDComand_Send((char *)LCDSendBuf,"OK",500,2);
		}
	}
	else
	{
		for(uint8_t i=0;i<WifiScanInfor.WifiCnt - SubPage*9;i++)
		{
			memset(LCDSendBuf,0,128);
			sprintf((char *)LCDSendBuf,"W8UE(%d);DS12(2,2,'%d.%s',15);SXY(0,0);\r\n",i+1,WifiScanInfor.SSIDInfor[i+SubPage*9].SSIDNum,WifiScanInfor.SSIDInfor[i+SubPage*9].SSIDName);
			LCDComand_Send((char *)LCDSendBuf,"OK",500,2);
		}
	}
}
static void LCD_WIFISelect(uint8_t SubPage)
{
	if(LCDComand_Send("SPG(37);\r\n","OK",200,5) != true)
		return;
	memset(LCDSendBuf,0,128);
	sprintf((char *)LCDSendBuf,"DS12(220,37,'WIFI总数：%02d',1,0);\r\n",WifiScanInfor.WifiCnt);
	LCDComand_Send((char *)LCDSendBuf,"OK",500,2);
	LCD_WIFIInforDisplay(SubPage);
}
static void LCD_PasswordInput(uint8_t KeyValue,uint8_t SubPage)
{
	memset(LCDSendBuf,0,128);
	if(LCDComand_Send("SPG(38);\r\n","OK",200,5) != true)
		return;
	sprintf((char *)LCDSendBuf,"DS12(8,150,'当前选择WIFI：%02d',1,0);DS12(8,180,'SSID：%s',1,0);\r\n",
		WifiScanInfor.SSIDInfor[KeyValue-1+SubPage*9].SSIDNum,WifiScanInfor.SSIDInfor[KeyValue-1+SubPage*9].SSIDName);
	LCDComand_Send((char *)LCDSendBuf,"OK",500,2);
}
static void LCD_ShowIP(void)
{
	memset(LCDSendBuf,0,128);
	sprintf((char *)LCDSendBuf,"W8UE(3);DS12(2,75,'%d.%d.%d.%d ',15);SXY(0,0);\r\n",
		WifiScanInfor.IP[0],WifiScanInfor.IP[1],WifiScanInfor.IP[2],WifiScanInfor.IP[3]);
	if(LCDComand_Send((char *)LCDSendBuf,"OK",500,2) != true)
		return;
}
static void LCD_RefreshTime(void)
{
	memset(LCDSendBuf,0,100);
	sprintf((char *)LCDSendBuf,"SBC(0);DS16(163,20,'%s',29,0);STIM(%d,%d,%d);\r\n",
		ESP8266_Weather.Date,ESP8266_Weather.Hour,ESP8266_Weather.Min,ESP8266_Weather.Second);
	LCDComand_Send((char *)LCDSendBuf,"OK",500,2);
}
static void LCD_RefreshFeedState(void)
{
	if(LCD_ControlHandle.ControlMode == false)
		LCDComand_Send("DS24(196,39,'自动',1,0);\r\n","OK",200,5);
	else
		LCDComand_Send("DS24(196,39,'手动',1,0);\r\n","OK",200,5);
	memset(LCDSendBuf,0,128);
	sprintf((char *)LCDSendBuf,"DS24(112,127,'%d小时',1,0);\r\n",LCD_ControlHandle.FeedTimePeriod);
	LCDComand_Send((char *)LCDSendBuf,"OK",500,2);
}
static void LCD_RefreshAirCleanerState(void)
{
	if(LCD_ControlHandle.ControlMode == false)
		LCDComand_Send("DS24(196,39,'自动',1,0);\r\n","OK",200,5);
	else
		LCDComand_Send("DS24(196,39,'手动',1,0);\r\n","OK",200,5);
	if(LCD_ControlHandle.HumStatus == false)
		LCDComand_Send("DS24(112,127,'关',1,0);\r\n","OK",200,5);
	else
		LCDComand_Send("DS24(112,127,'开',1,0);\r\n","OK",200,5);
	if(LCD_ControlHandle.TemperStatus == false)
		LCDComand_Send("DS24(112,207,'关',1,0);\r\n","OK",200,5);
	else
		LCDComand_Send("DS24(112,207,'开',1,0);\r\n","OK",200,5);
	if(LCD_ControlHandle.AirCleanerStatus == false)
		LCDComand_Send("DS24(259,126,'关',1,0);\r\n","OK",200,5);
	else
		LCDComand_Send("DS24(259,126,'开',1,0);\r\n","OK",200,5);
}
#define X_MIN 10
#define X_MAX 135
#define Y_MIN 60
#define Y_MAX 10
static void LCD_RefreshHarmfulAir(void)
{
	static uint8_t LastXPoint = 0;
	static uint8_t LastYValue = 60;
	uint8_t NowXPoint = 0;
	uint8_t NowYValue = 60-Board_ReadADC1DMA(ADC_PB0)*(50)/4096;
	NowXPoint = LastXPoint + 1;
	if(NowXPoint >=25)
	{
		LastXPoint = 0;
		LCDComand_Send("W8UE(6);BOXF(2,5,145,65,0);PL(5,10,140,10,2);PL(5,11,140,11,2);PL(5,20,140,20,2);PL(5,30,140,30,2);PL(5,40,140,40,2);PL(5,50,140,50,2);PL(5,59,140,59,2);PL(5,60,140,60,2);SXY(0,0);\r\n","OK",500,5);
		return;
	}
	memset(LCDSendBuf,0,128);
	sprintf((char *)LCDSendBuf,"W8UE(6);PL(%d,%d,%d,%d,5);SXY(0,0);\r\n",
			(LastXPoint+1)*5,LastYValue,(NowXPoint+1)*5,NowYValue);
	LCDComand_Send((char *)LCDSendBuf,"OK",200,5);
	LastXPoint = NowXPoint;
	LastYValue = NowYValue;
}





LCD_ControlHandle_t LCD_ControlHandle;
static State LCD_IDLEFunction(void *Args);
static State LCD_WIFIInitFunction(void *Args);
static State LCD_WIFISelectFunction(void *Args);
static State LCD_PassWordInputFunction(void *Args);
static State LCD_MainPageFunction(void *Args);
static State LCD_FoodControlFunction(void *Args);
static State LCD_AirControlFunction(void *Args);
const uint8_t LCDFuncionTimerCnt[LCD_TASKCNT] = {1,2,1,1,7,1,1};
void LCD_TaskInit(void)
{
	StateMachine_Init(&LCD_ControlHandle.LCDStateMachine,LCD_TASKCNT);
	LCD_ControlHandle.LCDStateMachine.RunningState = LCD_INIT;
	LCD_ControlHandle.FeedTimePeriod = 5;
	StateMachine_SetFuction(&LCD_ControlHandle.LCDStateMachine,LCD_IDLE,LCD_IDLEFunction);
	StateMachine_SetFuction(&LCD_ControlHandle.LCDStateMachine,LCD_INIT,LCD_WIFIInitFunction);
	StateMachine_SetFuction(&LCD_ControlHandle.LCDStateMachine,LCD_WIFISELECT,LCD_WIFISelectFunction);
	StateMachine_SetFuction(&LCD_ControlHandle.LCDStateMachine,LCD_PASSWORDINPUT,LCD_PassWordInputFunction);
	StateMachine_SetFuction(&LCD_ControlHandle.LCDStateMachine,LCD_MAINPAGE,LCD_MainPageFunction);
	StateMachine_SetFuction(&LCD_ControlHandle.LCDStateMachine,LCD_FOODCONRROLCENTRE,LCD_FoodControlFunction);
	StateMachine_SetFuction(&LCD_ControlHandle.LCDStateMachine,LCD_AIRCONRROLCENTRE,LCD_AirControlFunction);
	for(uint8_t i=0;i<(uint8_t)LCD_TASKCNT;i++)
	{
		LCD_ControlHandle.TimerCnt[i] = LCDFuncionTimerCnt[i];
		LCD_ControlHandle.TimerIfRun[i] = bgetz(LCD_ControlHandle.TimerCnt[i]*sizeof(bool));
		LCD_ControlHandle.TimerStartTick[i] =  bgetz(LCD_ControlHandle.TimerCnt[i]*sizeof(uint32_t));
	}
	for(uint8_t i=0;i<(uint8_t)LCD_TASKCNT;i++)
	{
		for(uint8_t j=0;j<2;j++)
		{
			LCD_ControlHandle.TimerIfRun[i][j] = true; 
		}
	}
}

static State LCD_IDLEFunction(void *Args)
{
	LCD_ControlHandle_t *RunArgs = (LCD_ControlHandle_t *)Args;	
	for(uint8_t i=0;i<RunArgs->TimerCnt[RunArgs->LCDStateMachine.RunningState];i++)
	{
		if(RunArgs->TimerIfRun[RunArgs->LCDStateMachine.RunningState][i] == true)
		{
			
		}
	}
	return RunArgs->LCDStateMachine.RunningState;	
}
static State LCD_WIFIInitFunction(void *Args)
{
	LCD_ControlHandle_t *RunArgs = (LCD_ControlHandle_t *)Args;	
	for(uint8_t i=0;i<RunArgs->TimerCnt[RunArgs->LCDStateMachine.RunningState];i++)
	{
		if(RunArgs->TimerIfRun[RunArgs->LCDStateMachine.RunningState][i] == true)
		{
			if(i==0)
			{
				
			}
			else if(i==1)
			{
				if(LCDComand_Send("SPG(36);\r\n","OK",200,5) == true)
				{
					ESP8266Reset();
					ESP8266Init();
					LCD_WIFISelect(0);
					RunArgs->TimerIfRun[RunArgs->LCDStateMachine.RunningState][i] = false;
					return LCD_WIFISELECT;
				}
			}
		}
	}
	return RunArgs->LCDStateMachine.RunningState;	
}
static State LCD_WIFISelectFunction(void *Args)
{
	LCD_ControlHandle_t *RunArgs = (LCD_ControlHandle_t *)Args;	
	for(uint8_t i=0;i<RunArgs->TimerCnt[RunArgs->LCDStateMachine.RunningState];i++)
	{
		if(RunArgs->TimerIfRun[RunArgs->LCDStateMachine.RunningState][i] == true)
		{
			uint8_t KeyValue = LCD_ScanKey((void *)0);
			switch(KeyValue)
			{
				case 0:
					break;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				{
					RunArgs->PasswordKeyValue = KeyValue;
					LCD_PasswordInput(RunArgs->PasswordKeyValue,RunArgs->SubPage);
				}return LCD_PASSWORDINPUT;
				case 10:
				{
					if(RunArgs->SubPage > 0)
						RunArgs->SubPage--;
					LCD_WIFISelect(RunArgs->SubPage);
				}break;
				case 11:
				{
					if(RunArgs->SubPage < WifiScanInfor.WifiCnt/9)
						RunArgs->SubPage++;
					LCD_WIFISelect(RunArgs->SubPage);
				}break;
				case 12:
				{
					ESP8266WiFiScanRelease();
					if(LCDComand_Send("SPG(40);\r\n","OK",500,5) != true)
						break;
				}return LCD_MAINPAGE;
				default:
					printf("%d\n",KeyValue);
					break;
			}
		}
	}
	return RunArgs->LCDStateMachine.RunningState;	
}
static State LCD_PassWordInputFunction(void *Args)
{
	LCD_ControlHandle_t *RunArgs = (LCD_ControlHandle_t *)Args;	
	for(uint8_t i=0;i<RunArgs->TimerCnt[RunArgs->LCDStateMachine.RunningState];i++)
	{
		if(RunArgs->TimerIfRun[RunArgs->LCDStateMachine.RunningState][i] == true)
		{
			uint8_t KeyValue = LCD_ScanKey(RunArgs->SSIDPassword);
			switch(KeyValue)
			{
				case 0:
					break;
				case 12:
					LCD_WIFISelect(RunArgs->SubPage);
				return LCD_WIFISELECT;
				case 13:
				{
					if(LCDComand_Send("SPG(39);\r\n","OK",200,5) != true)
						break;
					//连接WIFI
					memset(LCDSendBuf,0,128);
					sprintf((char *)LCDSendBuf,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",
						WifiScanInfor.SSIDInfor[RunArgs->PasswordKeyValue-1+RunArgs->SubPage*9].SSIDName,RunArgs->SSIDPassword);
					if(ATComand_Send((char *)LCDSendBuf,"OK",10000) != true)
					{
						printf("%s\n",LCDRecBuf);
						break;
					}
					WifiScanInfor.WIFIConnect = true;
					//获取信息
					ESP8266WiFiScanRelease();
					ESP8266IPGet();
					if(LCDComand_Send("SPG(40);\r\n","OK",500,5) != true)
						break;
					LCD_ShowIP();
					RunArgs->TimerIfRun[LCD_MAINPAGE][1] = true;
				}return LCD_MAINPAGE;
				default:
					break;
			}
		}
	}
	return RunArgs->LCDStateMachine.RunningState;	
}
static State LCD_MainPageFunction(void *Args)
{
	LCD_ControlHandle_t *RunArgs = (LCD_ControlHandle_t *)Args;	
	for(uint8_t i=0;i<RunArgs->TimerCnt[RunArgs->LCDStateMachine.RunningState];i++)
	{
		if(RunArgs->TimerIfRun[RunArgs->LCDStateMachine.RunningState][i] == true)
		{
			if(i==0)
			{
				uint8_t KeyValue = LCD_ScanKey((void *)0);
				switch(KeyValue)
				{
					case 0:
					{
						//服务器状态下
						if(WifiScanInfor.ServerSet == true)
							ESP8266TCPServerPoll();
					}break;
					case 3:
					{
						if(WifiScanInfor.WIFIConnect == true)
						{
							ESP8266_SendState_e SendResult;
							SendResult = ESP8266SendTimeGet();
							switch(SendResult)
							{
								case SEND_OK:
								{
									LCD_RefreshTime();
								}break;
								default:
								{
								}break;
							}
						}
					}break;
					case 4:
					{
						if(LCDComand_Send("SPG(42);\r\n","OK",200,5) != true)
							break;
						LCD_RefreshFeedState();
					}return LCD_FOODCONRROLCENTRE;
					case 5:
					{
						if(LCDComand_Send("SPG(41);\r\n","OK",200,5) != true)
							break;
						LCD_RefreshAirCleanerState();
					}return LCD_AIRCONRROLCENTRE;
					default:
						printf("%d\n",KeyValue);
						break;
				}
			}
			else if(i==1)
			{
				if(WifiScanInfor.WIFIConnect == true)
				{
					if(ESP8266SetTCPServer()!=true)
						return LCD_MAINPAGE;
				}
				RunArgs->TimerIfRun[RunArgs->LCDStateMachine.RunningState][i] = false;
				for(uint8_t j=i+1;j<LCDFuncionTimerCnt[RunArgs->LCDStateMachine.RunningState];j++)
				{
					RunArgs->TimerIfRun[RunArgs->LCDStateMachine.RunningState][j] = true;
					RunArgs->TimerStartTick[RunArgs->LCDStateMachine.RunningState][j] = APP_SoftTimer_GetTick();
				}
			}
			else
			{
				switch(i)
				{
					case 2:{
						if(APP_SoftTimer_GetTick()>=RunArgs->TimerStartTick[RunArgs->LCDStateMachine.RunningState][i]+1000)
						{
							memset(LCDSendBuf,0,128);
							sprintf((char *)LCDSendBuf,"W8UE(1);DS32(12,40,'%d.%d',23);DS16(50,50,'℃',23);SXY(0,0);\r\n",
									DHT11Result.TemperHigh8,DHT11Result.TemperLow8);
							LCDComand_Send((char *)LCDSendBuf,"OK",500,2);
							sprintf((char *)LCDSendBuf,"W8UE(2);DS32(12,40,'%d.%d',23);DS16(50,50,'%%',23);SXY(0,0);\r\n",
									DHT11Result.HumdityHigh8,DHT11Result.HumdityLow8);
							LCDComand_Send((char *)LCDSendBuf,"OK",500,2);
							if(RunArgs->ControlMode == false)
							{
								if(DHT11Result.TemperHigh8>=30)
									Board_SwitchOn(SWITCH_3);
								else
									Board_SwitchOff(SWITCH_3);
								if(DHT11Result.HumdityHigh8<=30)
									Board_SwitchOn(SWITCH_2);
								else
									Board_SwitchOff(SWITCH_2);
							}
							if(WifiScanInfor.ClientConnect == true)
							{
								char SendCommand[20];
								char SendString[256];
								memset(SendCommand,0,20);
								memset(SendString,0,256);
								sprintf(SendString,"Temper:%d.%d ",DHT11Result.TemperHigh8,DHT11Result.TemperLow8);
								sprintf(SendString,"%sHum:%d.%d ",SendString,DHT11Result.HumdityHigh8,DHT11Result.HumdityLow8);
								if(RunArgs->ControlMode == false)
									sprintf(SendString,"%sControlMode:Auto ",SendString);
								else
									sprintf(SendString,"%sControlMode:Hand ",SendString);
								if(RunArgs->FeedStatus == false)
									sprintf(SendString,"%sFeed:Close ",SendString);
								else
									sprintf(SendString,"%sFeed:Open ",SendString);
								if(RunArgs->AirCleanerStatus == false)
									sprintf(SendString,"%sAirCleaner:Close ",SendString);
								else
									sprintf(SendString,"%sAirCleaner:Open ",SendString);
								if(RunArgs->TemperStatus == false)
									sprintf(SendString,"%sTemperState:Close ",SendString);
								else
									sprintf(SendString,"%sTemperState:Open ",SendString);
								if(RunArgs->HumStatus == false)
									sprintf(SendString,"%sHumState:Close ",SendString);
								else
									sprintf(SendString,"%sHumState:Open ",SendString);
								sprintf(SendString,"%sGAS:%d",SendString,Board_ReadADC1DMA(ADC_PB0));
								sprintf(SendCommand,"AT+CIPSEND=%d,%d\r\n",WifiScanInfor.ConnectClientNum,strlen(SendString));
								if(ATComand_Send(SendCommand,">",1000) != true)
									printf("%s\n",ESP8266RecBuf);
								if(ATComand_Send(SendString,"SEND OK",1000) != true)
									printf("%s\n",ESP8266RecBuf);
							}
							RunArgs->TimerStartTick[RunArgs->LCDStateMachine.RunningState][i] = APP_SoftTimer_GetTick();
						}
					}break;
					case 3:{
						if(APP_SoftTimer_GetTick()>=RunArgs->TimerStartTick[RunArgs->LCDStateMachine.RunningState][i]+2000)
						{
							LCD_RefreshHarmfulAir();
							RunArgs->TimerStartTick[RunArgs->LCDStateMachine.RunningState][i] = APP_SoftTimer_GetTick();
						}
					}break;
					case 4:{
						if(APP_SoftTimer_GetTick()>=RunArgs->TimerStartTick[RunArgs->LCDStateMachine.RunningState][i]+5000)
						{
							memset(LCDSendBuf,0,128);
							sprintf((char *)LCDSendBuf,"W8UE(3);DS32(15,25,'%d  ',23);SXY(0,0);\r\n",
									Board_ReadADC1DMA(ADC_PB0));
							LCDComand_Send((char *)LCDSendBuf,"OK",500,2);
							memset(LCDSendBuf,0,128);
							sprintf((char *)LCDSendBuf,"W8UE(6);DS16(75,70,'%d  ',15);SXY(0,0);\r\n",
									Board_ReadADC1DMA(ADC_PB0));
							LCDComand_Send((char *)LCDSendBuf,"OK",500,2);	
							if(RunArgs->ControlMode == false)
							{
								if(Board_ReadADC1DMA(ADC_PB0)>=3000)
									Board_SwitchOn(SWITCH_1);
								else
									Board_SwitchOff(SWITCH_1);
							}
							RunArgs->TimerStartTick[RunArgs->LCDStateMachine.RunningState][i] = APP_SoftTimer_GetTick();
						}
					}break;
					case 5:{
						if(APP_SoftTimer_GetTick()>=RunArgs->TimerStartTick[RunArgs->LCDStateMachine.RunningState][i]+3600000)
						{
							if(WifiScanInfor.WIFIConnect == true)
							{
								ESP8266_SendState_e SendResult;
								SendResult = ESP8266SendTimeGet();
								switch(SendResult)
								{
									case SEND_OK:
									{
										memset(LCDSendBuf,0,100);
										sprintf((char *)LCDSendBuf,"SBC(0);DS16(148,10,'%s',29,0);STIM(%d,%d,%d);\r\n",
											ESP8266_Weather.Date,ESP8266_Weather.Hour,ESP8266_Weather.Min,ESP8266_Weather.Second);
										LCDComand_Send((char *)LCDSendBuf,"OK",500,2);
										RunArgs->TimerStartTick[RunArgs->LCDStateMachine.RunningState][i] = APP_SoftTimer_GetTick();
									}break;
									default:
									{
										RunArgs->TimerStartTick[RunArgs->LCDStateMachine.RunningState][i] = APP_SoftTimer_GetTick();
									}break;
								}
							}
						}
					}break;
					case 6:{
						if(APP_SoftTimer_GetTick()>=RunArgs->TimerStartTick[RunArgs->LCDStateMachine.RunningState][i]+RunArgs->FeedTimePeriod*3600000)
						{
							Board_SwitchOn(SWITCH_0);
							APP_SoftTimer_DelayMS(5000);
							Board_SwitchOff(SWITCH_0);
							RunArgs->TimerStartTick[RunArgs->LCDStateMachine.RunningState][i] = APP_SoftTimer_GetTick();
						}
					}break;
				}
			}
		}
	}
	return RunArgs->LCDStateMachine.RunningState;	
}
static State LCD_FoodControlFunction(void *Args)
{
	LCD_ControlHandle_t *RunArgs = (LCD_ControlHandle_t *)Args;	
	for(uint8_t i=0;i<RunArgs->TimerCnt[RunArgs->LCDStateMachine.RunningState];i++)
	{
		if(RunArgs->TimerIfRun[RunArgs->LCDStateMachine.RunningState][i] == true)
		{
			uint8_t KeyValue = LCD_ScanKey((void *)0);
			switch(KeyValue)
			{
				case 0:
					break;
				case 1:
				{
					if(RunArgs->ControlMode == true)
						RunArgs->ControlMode = false;
					LCD_RefreshFeedState();
				}break;
				case 2:
				{
					if(RunArgs->ControlMode == false)
						RunArgs->ControlMode = true;
					LCD_RefreshFeedState();
				}break;
				case 3:
				{
					if(RunArgs->ControlMode == true)
					{
						if(RunArgs->FeedTimePeriod <= 23)
							RunArgs->FeedTimePeriod++;
					}
					LCD_RefreshFeedState();
				}break;
				case 4:
				{
					if(RunArgs->ControlMode == true)
					{
						if(RunArgs->FeedTimePeriod >= 1)
							RunArgs->FeedTimePeriod--;
					}
					LCD_RefreshFeedState();
				}break;
				case 5:
				{
					if(LCDComand_Send("SPG(40);\r\n","OK",500,5) != true)
						break;
					LCD_ShowIP();
					LCD_RefreshTime();
					RunArgs->TimerIfRun[LCD_MAINPAGE][1] = true;
				}return LCD_MAINPAGE;
				default:
					printf("%d\n",KeyValue);
					break;
			}
		}
	}
	return RunArgs->LCDStateMachine.RunningState;	
}

static State LCD_AirControlFunction(void *Args)
{
	LCD_ControlHandle_t *RunArgs = (LCD_ControlHandle_t *)Args;	
	for(uint8_t i=0;i<RunArgs->TimerCnt[RunArgs->LCDStateMachine.RunningState];i++)
	{
		if(RunArgs->TimerIfRun[RunArgs->LCDStateMachine.RunningState][i] == true)
		{
			uint8_t KeyValue = LCD_ScanKey((void *)0);
			switch(KeyValue)
			{
				case 0:
					break;
				case 1:
				{
					if(RunArgs->ControlMode == true)
						RunArgs->ControlMode = false;
					LCD_RefreshAirCleanerState();
				}break;
				case 2:
				{
					if(RunArgs->ControlMode == false)
						RunArgs->ControlMode = true;
					LCD_RefreshAirCleanerState();
				}break;
				case 3:
				{
					if(RunArgs->ControlMode == true)
					{
						if(RunArgs->HumStatus == false)
						{
							RunArgs->HumStatus =true;
							Board_SwitchOn(SWITCH_2);
						}
					}
					LCD_RefreshAirCleanerState();
				}break;
				case 4:
				{
					if(RunArgs->ControlMode == true)
					{
						if(RunArgs->HumStatus == true)
						{
							RunArgs->HumStatus =false;
							Board_SwitchOff(SWITCH_2);
						}
					}
					LCD_RefreshAirCleanerState();
				}break;
				case 6:
				{
					if(RunArgs->ControlMode == true)
					{
						if(RunArgs->AirCleanerStatus == false)
						{
							RunArgs->AirCleanerStatus =true;
							Board_SwitchOn(SWITCH_1);
						}
					}
					LCD_RefreshAirCleanerState();
				}break;
				case 7:
				{
					if(RunArgs->ControlMode == true)
					{
						if(RunArgs->AirCleanerStatus == true)
						{
							RunArgs->AirCleanerStatus =false;
							Board_SwitchOff(SWITCH_1);
						}
					}
					LCD_RefreshAirCleanerState();
				}break;
				case 8:
				{
					if(RunArgs->ControlMode == true)
					{
						if(RunArgs->TemperStatus == false)
						{
							RunArgs->TemperStatus =true;
							Board_SwitchOn(SWITCH_3);
						}
					}
					LCD_RefreshAirCleanerState();
				}break;
				case 9:
				{
					if(RunArgs->ControlMode == true)
					{
						if(RunArgs->TemperStatus == true)
						{
							RunArgs->TemperStatus =false;
							Board_SwitchOff(SWITCH_3);
						}
					}
					LCD_RefreshAirCleanerState();
				}break;
				case 10:
				{
					if(LCDComand_Send("SPG(40);\r\n","OK",500,5) != true)
						break;
					LCD_ShowIP();
					LCD_RefreshTime();
					RunArgs->TimerIfRun[LCD_MAINPAGE][1] = true;
				}return LCD_MAINPAGE;
				default:
					printf("%d\n",KeyValue);
					break;
			}
		}
	}
	return RunArgs->LCDStateMachine.RunningState;	
}
