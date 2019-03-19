#include "Board_ESP8266.h"
#include "BSP_Com.h"
#include <stdlib.h>
void Board_ESP8266_HardReset(void)
{
	//硬件复位
	GPIO_ResetBits(GPIOA, GPIO_Pin_0);
	osDelay(10);
	GPIO_SetBits(GPIOA, GPIO_Pin_0);
	osDelay(200);
}
static RTE_Shell_Err_e ESP8266_Shell_CMD_ATSend(int argc, char *argv[])
{
	if(argc == 2)
	{
		BSP_COM_SendArray(COM_WIFI,(uint8_t *)argv[1],ustrlen(argv[1]));
		BSP_COM_SendArray(COM_WIFI,(uint8_t *)"\r\n",2);
	}
	return(SHELL_NOERR);
}
void Board_ESP8266_Init(void)
{
	//初始化GPIO
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化
	GPIO_SetBits(GPIOA, GPIO_Pin_0);
	BSP_COM_Init(COM_WIFI,115200);
	RTE_Shell_CreateModule("ESP8266");
	RTE_AssertParam(RTE_Shell_AddCommand("ESP8266","AT",ESP8266_Shell_CMD_ATSend,"ESP8266 Debug") == SHELL_NOERR);
}
void Board_ESP8266_Off(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_0);
}
bool Board_ESP8266_Boot(uint8_t retrytimes)
{
	uint8_t *TempBuf = Memory_Alloc0(BANK_RTE,1024);
	RTE_MessageQuene_t *WIFIMSG = &BSP_COM_ReturnQue(COM_WIFI)->ComQuene;
	while(retrytimes>0)
	{
		Board_ESP8266_HardReset();
		uint32_t timeout = 10000;
		while(timeout)
		{
			if(RTE_MessageQuene_Out(WIFIMSG,TempBuf,NULL) == MSG_NO_ERR)
			{
				if(ustrstr((char *)TempBuf,"eady"))
				{
					Memory_Free(BANK_RTE,TempBuf);
					return true;
				}
				else
				{
					memset(TempBuf,0,1024);
					continue;
				}
			}
			osThreadFlagsWait(0x00000001U,osFlagsWaitAny,100);
			timeout = timeout-100;
		}
		retrytimes--;
	}
	Memory_Free(BANK_RTE,TempBuf);
	return false;
}
uint8_t *Board_ESP8266_ATSend(const char *command,const char *ideares,uint32_t timeout)
{
	uint8_t *TempBuf = Memory_Alloc0(BANK_RTE,128);
	RTE_MessageQuene_t *WIFIMSG = &BSP_COM_ReturnQue(COM_WIFI)->ComQuene;
	BSP_COM_SendArray(COM_WIFI,(uint8_t *)command,ustrlen(command));
	uint32_t StartTick = RTE_RoundRobin_GetTick();
	while(timeout)
	{
		if(RTE_MessageQuene_Out(WIFIMSG,TempBuf,NULL) == MSG_NO_ERR)
		{
			if(ustrstr((char *)TempBuf,ideares))
			{
				return TempBuf;
			}
			else
			{
				RTE_Printf("Error:%s\r\n",TempBuf);
				memset(TempBuf,0,128);
				continue;
			}
		}
		osThreadFlagsWait(0x00000001U,osFlagsWaitAny,100);
		timeout = timeout - 100;
	}
	Memory_Free(BANK_RTE,TempBuf);
	return 0;
}
bool Board_ESP8266_SetWorkMode(uint8_t workmode)
{
	workmode = workmode + 0x30;//dec2ascii
	BSP_COM_SendArray(COM_WIFI,(uint8_t *)"AT+CWMODE_CUR=",ustrlen("AT+CWMODE_CUR="));
	BSP_COM_SendByte(COM_WIFI,workmode);
	uint8_t *rec = Board_ESP8266_ATSend("\r\n","OK",1000);
	if(rec)
	{
		Memory_Free(BANK_RTE,rec);
		return true;
	}
	return false;
}

bool Board_ESP8266_SetATE(uint8_t ifon)
{
	ifon = ifon + 0x30;
	BSP_COM_SendArray(COM_WIFI,(uint8_t *)"ATE",ustrlen("ATE"));
	BSP_COM_SendByte(COM_WIFI,ifon);
	uint8_t *rec = Board_ESP8266_ATSend("\r\n","OK",1000);
	if(rec)
	{
		Memory_Free(BANK_RTE,rec);
		return true;
	}
	return false;
}

uint8_t Board_ESP8266_ScanHotsPot(ESP8266_HotsPot_t **resultarray)
{
	uint8_t *TempBuf = Memory_Alloc0(BANK_RTE,2048);
	BSP_COM_SendArray(COM_WIFI,(uint8_t *)"AT+CWLAP\r\n",ustrlen("AT+CWLAP\r\n"));
	uint32_t StartTick = RTE_RoundRobin_GetTick();
	uint32_t timeout = 10000;
	while(timeout)
	{
		if(RTE_MessageQuene_Out(&BSP_COM_ReturnQue(COM_WIFI)->ComQuene,TempBuf,NULL) == MSG_NO_ERR)
		{
			break;
		}	
		osThreadFlagsWait(0x00000001U,osFlagsWaitAny,100);
		timeout = timeout - 100;
	}
	uint8_t retval = 0;
	char *startstr = (char *)TempBuf;
	char *endstr = (char *)TempBuf;
	for(;;)
	{
		startstr = ustrstr(endstr,":(");
		if(!startstr)
			break;
		endstr = ustrstr(startstr+5,"\"");
		retval++;
	}
	*resultarray = Memory_Alloc0(BANK_RTE,retval*sizeof(ESP8266_HotsPot_t));
	startstr = (char *)TempBuf;
	endstr = (char *)TempBuf;
	for(uint8_t i=0;i<retval;i++)
	{
		startstr = ustrstr(endstr,":(");
		endstr = ustrstr(startstr+5,"\"");
		(*resultarray)[i].ecn = *(startstr+2)-0x30;
		(*resultarray)[i].ssid = Memory_Alloc0(BANK_RTE,endstr-(startstr+5)+1);
		memcpy((*resultarray)[i].ssid,(startstr+5),endstr-(startstr+5));
		startstr = ustrstr(endstr,",");
		endstr = ustrstr(startstr+1,",");
		char temp[8] = {0};
		memcpy(temp,startstr+1,endstr-(startstr+1)+1);
		(*resultarray)[i].rssi = atoi(temp);
		startstr = ustrstr(endstr,"\"");
		endstr = ustrstr(startstr+2,"\"");
		(*resultarray)[i].mac = Memory_Alloc0(BANK_RTE,endstr-(startstr+1)+1);
		memcpy((*resultarray)[i].mac,(startstr+1),endstr-(startstr+1));
	}
	Memory_Free(BANK_RTE,TempBuf);
	return retval;
}
int8_t Board_ESP8266_FindHotsPot(ESP8266_HotsPot_t *resultarray,const char *ssid,uint8_t cnt)
{
		for(int8_t i=0;i<cnt;i++)
		{
			if(ustrstr((char *)resultarray[i].ssid,ssid))
				return i;
		}
		return -1;
}
void Board_ESP8266_ReleaseHotsPot(ESP8266_HotsPot_t **resultarray,uint8_t num)
{
	for(uint8_t i=0;i<num;i++)
	{
		Memory_Free(BANK_RTE,(*resultarray)[i].ssid);
		Memory_Free(BANK_RTE,(*resultarray)[i].mac);
	}
	Memory_Free(BANK_RTE,*resultarray);
}

bool Board_ESP8266_ConnectAP(const char *ssid,const char *password)
{
	BSP_COM_SendArray(COM_WIFI,(uint8_t *)"AT+CWJAP_CUR=\"",ustrlen("AT+CWJAP_CUR=\""));
	BSP_COM_SendArray(COM_WIFI,(uint8_t *)ssid,ustrlen(ssid));
	BSP_COM_SendArray(COM_WIFI,(uint8_t *)"\",\"",ustrlen("\",\""));
	BSP_COM_SendArray(COM_WIFI,(uint8_t *)password,ustrlen(password));
	uint8_t *rec = Board_ESP8266_ATSend("\"\r\n","OK",10000);
	if(rec)
	{
		Memory_Free(BANK_RTE,rec);
		return true;
	}
	return false;
}
bool Board_ESP8266_DisConnectAP(void)
{
	uint8_t *rec = Board_ESP8266_ATSend("AT+CWQAP\r\n","WIFI DISCONNECT",2000);
	if(rec)
	{
		Memory_Free(BANK_RTE,rec);
		return false;
	}
	else 
	{
		uint8_t *rec = Board_ESP8266_ATSend("AT+CWQAP\r\n","OK",2000);
		if(rec)
		{
			Memory_Free(BANK_RTE,rec);
			return false;
		}
	}
	return true;
}
bool Board_ESP8266_GetIP(char **ip)
{
	bool retval = false;
	uint8_t *TempBuf = Memory_Alloc0(BANK_RTE,2048);
	BSP_COM_SendArray(COM_WIFI,(uint8_t *)"AT+CIFSR\r\n",ustrlen("AT+CIFSR\r\n"));
	uint32_t StartTick = RTE_RoundRobin_GetTick();
	uint32_t timeout = 10000;
	while(timeout)
	{
		if(RTE_MessageQuene_Out(&BSP_COM_ReturnQue(COM_WIFI)->ComQuene,TempBuf,NULL) == MSG_NO_ERR)
			break;
		osThreadFlagsWait(0x00000001U,osFlagsWaitAny,100);
		timeout = timeout - 100;
	}
	char *startstr = (char *)TempBuf;
	char *endstr = (char *)TempBuf;
	startstr = ustrstr(endstr,"STAIP");
	endstr = ustrstr(startstr+7,"\"");
	if(startstr&&endstr)
	{
		*ip = Memory_Alloc0(BANK_RTE,endstr-(startstr+7)+1);
		memcpy(*ip,startstr+7,endstr-(startstr+7));
		RTE_Printf("%s\r\n",*ip);
		retval = true;
	}
	Memory_Free(BANK_RTE,TempBuf);
	return retval;
}
bool Board_ESP8266_OpenServer(char *portnum)
{
	bool retval = false;
	uint8_t *rec = Board_ESP8266_ATSend("AT+CIPMUX=1\r\n","OK",2000);
	if(rec)
	{
		Memory_Free(BANK_RTE,rec);
		BSP_COM_SendArray(COM_WIFI,(uint8_t *)"AT+CIPSERVER=1,",ustrlen("AT+CIPSERVER=1,"));
		BSP_COM_SendArray(COM_WIFI,(uint8_t *)portnum,ustrlen(portnum));
		rec = Board_ESP8266_ATSend("\r\n","OK",5000);
		if(rec)
		{
			Memory_Free(BANK_RTE,rec);
			retval = true;
		}
	}
	return retval;
}
bool Board_ESP8266_CloseServer(void)
{
	uint8_t *rec = Board_ESP8266_ATSend("AT+CIPSERVER=0\r\n","OK",5000);
	if(rec)
	{
		Memory_Free(BANK_RTE,rec);
		return false;
	}
	return true;
}
uint16_t Board_ESP8266_SendDataAsClient(uint8_t channel,uint8_t *data,uint16_t lenth)
{
	char SenBuf[32] = {0};
	usprintf(SenBuf,"AT+CIPSEND=%d,%d\r\n",channel,lenth);
	uint8_t *rec = Board_ESP8266_ATSend(SenBuf,">",10000);
	uint16_t RetVal = lenth;
	uint8_t *TempBuf = Memory_Alloc0(BANK_RTE,2048);
	if(rec)
	{
		Memory_Free(BANK_RTE,rec);	
		BSP_COM_SendArray(COM_WIFI,data,lenth);
		uint32_t StartTick = RTE_RoundRobin_GetTick();
		uint32_t timeout = 10000;
		while(timeout)
		{
			memset(TempBuf,0,2048);
			if(RTE_MessageQuene_Out(&BSP_COM_ReturnQue(COM_WIFI)->ComQuene,TempBuf,NULL) == MSG_NO_ERR)
			{
				if(ustrstr((char *)TempBuf,"SEND OK"))
					break;
			}
			timeout = timeout -100;
			osThreadFlagsWait(0x00000001U,osFlagsWaitAny,100);
		}
	}
	Memory_Free(BANK_RTE,TempBuf); 
	return RetVal;
}

