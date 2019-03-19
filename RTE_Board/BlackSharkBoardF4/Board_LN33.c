#include "Board_LN33.h"
#include "BSP_Com.h"
static uint8_t LN33TXBuffer[64] = {0xFE,0x05,0x90,0x21,0x00,0x00,0x01,0xFF};
uint16_t Board_LN33_GetAddress(uint32_t timeout)
{
	memset(LN33TXBuffer,0,64);
	LN33TXBuffer[0] = 0xFE;
	LN33TXBuffer[1] = 0x05;
	LN33TXBuffer[2] = 0x90;
	LN33TXBuffer[3] = 0x21;
	LN33TXBuffer[4] = 0x00;
	LN33TXBuffer[5] = 0x00;
	LN33TXBuffer[6] = 0x01;
	LN33TXBuffer[7] = 0xFF;
	BSP_COM_SendArray(COM_ZIGBEE,LN33TXBuffer,8);
	uint8_t buf[64] = {0};
	uint16_t lenth = 0;
	uint16_t RetVal = 0;
	while(timeout)
	{
		memset(buf,0,64);
		lenth = 0;
		if(RTE_MessageQuene_Out(&BSP_COM_ReturnQue(COM_ZIGBEE)->ComQuene,buf,&lenth) == MSG_NO_ERR)
		{
			if(buf[0] == 0xFE&&buf[lenth-1]==0xFF&&buf[1] == 0x07&&buf[6]==0x21)
			{
				RetVal = buf[8]<<8|buf[7];
				break; 
			}
		}
		osThreadFlagsWait(0x00000001U,osFlagsWaitAny,100);
		timeout = timeout - 100;
	}
	return RetVal;
}
uint16_t Board_LN33_GetID(uint32_t timeout)
{
	memset(LN33TXBuffer,0,64);
	LN33TXBuffer[0] = 0xFE;
	LN33TXBuffer[1] = 0x05;
	LN33TXBuffer[2] = 0x90;
	LN33TXBuffer[3] = 0x21;
	LN33TXBuffer[4] = 0x00;
	LN33TXBuffer[5] = 0x00;
	LN33TXBuffer[6] = 0x02;
	LN33TXBuffer[7] = 0xFF;
	BSP_COM_SendArray(COM_ZIGBEE,LN33TXBuffer,8);
	uint8_t buf[64] = {0};
	uint16_t lenth = 0;
	uint16_t RetVal = 0;
	while(timeout)
	{
		memset(buf,0,64);
		lenth = 0;
		if(RTE_MessageQuene_Out(&BSP_COM_ReturnQue(COM_ZIGBEE)->ComQuene,buf,&lenth) == MSG_NO_ERR)
		{
			if(buf[0] == 0xFE&&buf[lenth-1]==0xFF&&buf[1] == 0x07&&buf[6]==0x22)
			{
				RetVal = buf[8]<<8|buf[7];
				break; 
			}
		}
		osThreadFlagsWait(0x00000001U,osFlagsWaitAny,100);
		timeout = timeout - 100;
	}
	return RetVal;
}
uint8_t Board_LN33_GetChannel(uint32_t timeout)
{
	memset(LN33TXBuffer,0,64);
	LN33TXBuffer[0] = 0xFE;
	LN33TXBuffer[1] = 0x05;
	LN33TXBuffer[2] = 0x90;
	LN33TXBuffer[3] = 0x21;
	LN33TXBuffer[4] = 0x00;
	LN33TXBuffer[5] = 0x00;
	LN33TXBuffer[6] = 0x03;
	LN33TXBuffer[7] = 0xFF;
	BSP_COM_SendArray(COM_ZIGBEE,LN33TXBuffer,8);
	uint8_t buf[64] = {0};
	uint16_t lenth = 0;
	uint8_t RetVal = 0;
	while(timeout)
	{
		memset(buf,0,64);
		lenth = 0;
		if(RTE_MessageQuene_Out(&BSP_COM_ReturnQue(COM_ZIGBEE)->ComQuene,buf,&lenth) == MSG_NO_ERR)
		{
			if(buf[0] == 0xFE&&buf[lenth-1]==0xFF&&buf[1] == 0x06&&buf[6]==0x23)
			{
				RetVal = buf[7];
				break; 
			}
		}
		osThreadFlagsWait(0x00000001U,osFlagsWaitAny,100);
		timeout = timeout - 100;
	}
	return RetVal;
}
void Board_LN33_SendData(uint8_t lenth,uint8_t port,uint16_t destadd,uint8_t *data)
{
	memset(LN33TXBuffer,0,64);
	LN33TXBuffer[0] = 0xFE;
	LN33TXBuffer[1] = lenth+4;
	LN33TXBuffer[2] = port;
	LN33TXBuffer[3] = port;
	LN33TXBuffer[4] = destadd&0xFF;
	LN33TXBuffer[5] = (destadd>>8)&0xFF;
	memcpy(LN33TXBuffer+6,data,lenth);
	LN33TXBuffer[lenth+6] = 0xFF;
	BSP_COM_SendArray(COM_ZIGBEE,LN33TXBuffer,lenth+7);
}

