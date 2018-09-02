#include "Board_GSMA9.h"
#include "BSP_ComF1.h"
#include "APP_SoftTimer.h"
#include "APP_StdLib.h"
#include "Board_Switch.h"
#include "Board_LCD.h"
#include "Board_PM2_5.h"
#include "Board_ADC.h"
#include "Board_DHT11.h"
static uint8_t *GSMModuleRecBuff;
bool GSMATComand_Send(char *SendStr,const char *ExpectRec,uint32_t TimeOut,uint8_t RetryCnts)
{
	uint32_t StartTick = 0;
	uint16_t RecLenth = 0;
	for(uint8_t i=0;i<RetryCnts;i++)
	{
		memset(GSMModuleRecBuff,0,512);
		BSP_USART_Puts(COM_1,SendStr);
		StartTick = APP_SoftTimer_GetTick();
		while(StartTick+TimeOut >= APP_SoftTimer_GetTick())
		{
			BSP_USART_Data_t *RecData;
			RecData = BSP_USART_ReturnQue(COM_1);
			if(APP_RingBuffer_DeQuene(&RecData->ComQuene,(uint8_t *)GSMModuleRecBuff,&RecLenth)==QUE_NO_ERR)
			{
				if(strstr((const char *)GSMModuleRecBuff,ExpectRec))
					return true;
				else
				{
					memset(GSMModuleRecBuff,0,512);
					RecLenth = 0;
				}
			}
		}
	}
	return false;
}
bool GSMModule_SendMessage(char *Text,char *ReceivePhone)
{
	char *SendString;
	SendString = bgetz(sizeof(ReceivePhone)+20);
	if(SendString == (void *)0)
		return false;
	usprintf(SendString,"AT+CMGS=\"%s\"\r\n",ReceivePhone);
	if(GSMATComand_Send(SendString,">",500,5)==false)
	{
		brel(SendString);
		return false;
	}
	if(GSMATComand_Send(Text,Text,2000,1)==false)
	{
		brel(SendString);
		return false;
	}
	memset(SendString, 0, sizeof(ReceivePhone)+20);    //清空
	SendString[0] = 0x1a;
	if(GSMATComand_Send(SendString,"OK",10000,5)==false)
	{
		brel(SendString);
		return false;
	}
	brel(SendString);
	return true;
}
bool GSMModule_Init(void)
{

	GSMModuleRecBuff = bget(512);
	memset(GSMModuleRecBuff,0,512);
//	uint32_t StartTick = 0;
//	uint16_t RecLenth = 0;
//	StartTick = APP_SoftTimer_GetTick();
//	while(StartTick + 20000 >= APP_SoftTimer_GetTick())
//	{
//		BSP_USART_Data_t *RecData;
//		RecData = BSP_USART_ReturnQue(COM_1);
//		if(APP_RingBuffer_DeQuene(&RecData->ComQuene,(uint8_t *)GSMModuleRecBuff,&RecLenth)==QUE_NO_ERR)
//		{
//			memset(GSMModuleRecBuff,0,512);
//			RecLenth = 0;
//		}
//	}
	//检测模块在位
	if(GSMATComand_Send("AT\r\n","OK",500,10)==false)
		return false;
	printf("[GSM]AT Success!\n");
	if(GSMATComand_Send("AT+RST\r\n","OK",500,10)==false)
		return false;
	printf("[GSM]RST Success!\n");
	//检测SIM卡在位
	if(GSMATComand_Send("AT+CPIN?\r\n","READY",500,5)==false)
		return false;
	printf("[GSM]CPIN Success!\n");
	if(GSMATComand_Send("AT+CREG?\r\n","CREG: 1",500,5)==false)
		return false;
	printf("[GSM]CREG Success!\n");
	if(GSMATComand_Send("AT+CMGF=1\r\n","OK",500,5)==false)
		return false;
	printf("[GSM]CMGF Success!\n");
	if(GSMATComand_Send("AT+CSCS=\"GSM\"\r\n","OK",500,5)==false)
		return false;
	printf("[GSM]CSCS Success!\n");
	if(GSMATComand_Send("AT+CNMI=2,2\r\n","OK",500,5)==false)
		return false;
	printf("[GSM]CNMI Success!\n");
//	if(GSMModule_SendMessage("WelCome To Use Air Detect System!","17315970080")==false)
//		return false;
	return true;
}

void GSMModule_Poll(void)
{
	uint16_t RecLenth = 0;
	BSP_USART_Data_t * debugdata;
	debugdata = BSP_USART_ReturnQue(COM_1);
	if(APP_RingBuffer_DeQuene(&debugdata->ComQuene,(uint8_t *)GSMModuleRecBuff,&RecLenth)==QUE_NO_ERR)
	{
		char *StringStart;
		char SendString[100];
		StringStart = strstr((char *)GSMModuleRecBuff,"+CMT:");
		if(StringStart)
		{
			StringStart = strstr((char *)StringStart,"\r\n");
			if(StringStart)
			{
				StringStart = StringStart+2;
				if(strstr((char *)StringStart,"PM25"))
				{
					memset(SendString,0,100);
					sprintf(SendString,"PM25:%d",PM2_5ModuleData.PPM);
					GSMModule_SendMessage(SendString,(char *)&LCD_ControlHandle.RemotePhonenum[LCD_ControlHandle.PhoneNumKeyValue-1][0]);
				}
				else if(strstr((char *)StringStart,"Temper"))
				{
					memset(SendString,0,100);
					sprintf(SendString,"Temper:%d.%d",DHT11Result.TemperHigh8,DHT11Result.TemperLow8);
					GSMModule_SendMessage(SendString,(char *)&LCD_ControlHandle.RemotePhonenum[LCD_ControlHandle.PhoneNumKeyValue-1][0]);
				}
				else if(strstr((char *)StringStart,"Humni"))
				{
					memset(SendString,0,100);
					sprintf(SendString,"Hum:%d.%d",DHT11Result.HumdityHigh8,DHT11Result.HumdityLow8);
					GSMModule_SendMessage(SendString,(char *)&LCD_ControlHandle.RemotePhonenum[LCD_ControlHandle.PhoneNumKeyValue-1][0]);
				}
				else if(strstr((char *)StringStart,"GAS"))
				{
					memset(SendString,0,100);
					sprintf(SendString,"GAS:%d",Board_ReadADC1DMA(ADC_PB0));
					GSMModule_SendMessage(SendString,(char *)&LCD_ControlHandle.RemotePhonenum[LCD_ControlHandle.PhoneNumKeyValue-1][0]);
				}	
				else if(strstr((char *)StringStart,"Window On"))
				{
					if(LCD_ControlHandle.ControlMode == true)
						Board_SwitchOn(SWITCH_0);
				}
				else if(strstr((char *)StringStart,"Window Off"))
				{
					if(LCD_ControlHandle.ControlMode == true)
						Board_SwitchOff(SWITCH_0);
				}
				else if(strstr((char *)StringStart,"AirCleaner On"))
				{
					if(LCD_ControlHandle.ControlMode == true)
						Board_SwitchOn(SWITCH_1);
				}
				else if(strstr((char *)StringStart,"AirCleaner Off"))
				{
					if(LCD_ControlHandle.ControlMode == true)
						Board_SwitchOff(SWITCH_1);
				}
				else if(strstr((char *)StringStart,"Auto Mode"))
				{
					if(LCD_ControlHandle.ControlMode == true)
						LCD_ControlHandle.ControlMode = false;
				}
				else if(strstr((char *)StringStart,"Hand Mode"))
				{
					if(LCD_ControlHandle.ControlMode == false)
						LCD_ControlHandle.ControlMode = true;
				}
			}
		}
		memset(GSMModuleRecBuff,0,512);
		RecLenth = 0;
	}
}
