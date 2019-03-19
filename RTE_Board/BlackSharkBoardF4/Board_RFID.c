#include "Board_RFID.h"
#include "BSP_Com.h"
static uint8_t RFID_CheckSum(uint8_t *data,uint16_t lenth)
{
	uint8_t RetVal = 0;
	RTE_AssertParam(data[1]==lenth);
	for(uint8_t i=0;i<lenth-1;i++)
		RetVal ^= data[i];
	return (~RetVal);
}
uint16_t Board_RFID_SendData(uint8_t *data,uint8_t *recdata,uint16_t lenth,uint32_t timeout)
{
	RTE_MessageQuene_t *RFIDMSG = &BSP_COM_ReturnQue(COM_RFID)->ComQuene;
	BSP_COM_SendArray(COM_RFID,data,lenth);
	uint16_t reclenth = 0;
	uint32_t StartTick = RTE_RoundRobin_GetTick();
	while(timeout)
	{
		if(RTE_MessageQuene_Out(RFIDMSG,recdata,&reclenth) == MSG_NO_ERR)
		{
			for(uint8_t i=0;i<reclenth;i++)
			{
				RTE_Printf("%x ",recdata[i]);
			}
			RTE_Printf("\r\n");
			return reclenth;
		}
		osThreadFlagsWait(0x00000001U,osFlagsWaitAny,100);
		timeout = timeout - 100;
	}
	return 0;
}
bool Board_RFID_SetWorkMode(uint8_t mode,uint8_t bolcknum,uint8_t upmode)
{
	bool RetVal = false;
	uint8_t RecBuf[8] = {0};
	uint8_t SendBuf[8] = {0};
	uint16_t RecLenth = 0;
	SendBuf[0] = 0x03;
	SendBuf[1] = 0x08;
	SendBuf[2] = 0xC1;
	SendBuf[3] = 0x20;
	SendBuf[4] = mode;
	SendBuf[5] = bolcknum;
	SendBuf[6] = upmode;
	SendBuf[7] = RFID_CheckSum(SendBuf,8);
	RecLenth = Board_RFID_SendData(SendBuf,RecBuf,8,2000);
	if(RecLenth == 8)
	{
		RetVal = (RecBuf[4]==0x00);
	}
	return RetVal;
}
bool Board_RFID_Init(void)
{
	BSP_COM_Init(COM_RFID,9600);
	return(Board_RFID_SetWorkMode(0x01,0x00,0x01));
}
bool Board_RFID_ReadCardNum(RFID_Num_t *Num,uint8_t retrytimes)
{
	RTE_AssertParam(Num);
	bool RetVal = false;
	uint8_t RecBuf[32] = {0};
	uint8_t SendBuf[8] = {0};
	uint16_t RecLenth = 0;
	SendBuf[0] = 0x01;
	SendBuf[1] = 0x08;
	SendBuf[2] = 0xA1;
	SendBuf[3] = 0x20;
	SendBuf[4] = 0x00;
	SendBuf[5] = 0x01;
	SendBuf[6] = 0x00;
	SendBuf[7] = RFID_CheckSum(SendBuf,8);
	while(retrytimes)
	{
		RecLenth = Board_RFID_SendData(SendBuf,RecBuf,8,2000);
		if(RecLenth == 12)
		{
			Num->type = (uint16_t)RecBuf[5]<<8|RecBuf[6];
			Num->num = (uint32_t)RecBuf[7]<<24|(uint32_t)RecBuf[8]<<16|(uint32_t)RecBuf[9]<<8|(uint32_t)RecBuf[10];
			RetVal = true;
			break;
		}
		retrytimes--;
		osDelay(500);
	}
	return RetVal;
}
bool Board_RFID_ReadBlockData(uint8_t *data,uint8_t key,uint8_t blocknum)
{
	RTE_AssertParam(data);
	bool RetVal = false;
	uint8_t RecBuf[32] = {0};
	uint8_t SendBuf[8] = {0};
	uint16_t RecLenth = 0;
	SendBuf[0] = 0x01;
	SendBuf[1] = 0x08;
	SendBuf[2] = key;
	SendBuf[3] = 0x20;
	SendBuf[4] = blocknum;
	SendBuf[5] = 0x01;
	SendBuf[6] = 0x00;
	SendBuf[7] = RFID_CheckSum(SendBuf,8);
	RecLenth = Board_RFID_SendData(SendBuf,RecBuf,8,2000);
	if(RecLenth == 22)
	{
		memcpy(data,RecBuf+5,16);
		RetVal = true;
	}
	return RetVal;
}
bool Board_RFID_WriteBlockData(uint8_t *data,uint8_t key,uint8_t blocknum)
{
	RTE_AssertParam(data);
	bool RetVal = false;
	uint8_t SendBuf[32] = {0};
	uint8_t RecBuf[8] = {0};
	uint16_t RecLenth = 0;
	SendBuf[0] = 0x01;
	SendBuf[1] = 0x17;
	SendBuf[2] = key;
	SendBuf[3] = 0x20;
	SendBuf[4] = blocknum;
	SendBuf[5] = 0x01;
	memcpy(SendBuf+6,data,16);
	SendBuf[22] = RFID_CheckSum(SendBuf,22);
	RecLenth = Board_RFID_SendData(SendBuf,RecBuf,22,2000);
	if(RecLenth == 8)
	{
		if(RecBuf[4] == 0x00)
			RetVal = true;
	}
	return RetVal;
}

