#include "APP_Debug.h"
//*****************************************************************************
//
// Input buffer for the command line interpreter.
//
//*****************************************************************************
//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int CMD_help(int argc, char *argv[])
{
	tCmdLineEntry *pEntry;

	//
	// Print some header text.
	//
	printf("Available commands\n");
	printf("------------------\n");

	//
	// Point at the beginning of the command table.
	//
	pEntry = &g_psCmdTable[0];

	//
	// Enter a loop to read each entry from the command table.  The end of the
	// table has been reached when the command name is NULL.
	//
	while(pEntry->pcCmd)
	{
		//
		// Print the command name and the brief description.
		//
		printf("%15s%s\n", pEntry->pcCmd, pEntry->pcHelp);

		//
		// Advance to the next entry in the table.
		//
		pEntry++;
	}

	//
	// Return success.
	//
	return(0);
}
//*****************************************************************************
//
// This function implements the "help" command.  It prints a simple list of the
// available commands with a brief description.
//
//*****************************************************************************
int CMD_MEMInfor(int argc, char *argv[])
{
	printf("[MEM_0]Total:%d Used:%d\n",APP_BGet_TotalSize(MEM_0),APP_BGet_AllocSize(MEM_0));
	//
	// Return success.
	//
	return(0);
}
#include "Comunication_APP.h"
extern Sensors_Data_t SensorsData;
int CMD_TempInfor(int argc, char *argv[])
{
	APP_AssertParam(argc==2);
	uint8_t DestID = htoi(argv[1]);
	uint8_t *SendBuffer = APP_BGetz(MEM_0,5);
	SendBuffer[0] = 0x04;
	SendBuffer[1] = 2;
	SendBuffer[2] = 0x65;
	SendBuffer[3] = (uint8_t)SensorsData.temp_value;
	SendBuffer[4] = (uint8_t)(SensorsData.temp_value*100-SendBuffer[3]*100);
	Comunication_SendFrame(DestID,SendBuffer,5);
	APP_BRel(MEM_0,SendBuffer);
	//
	// Return success.
	//
	return(0);
}
int CMD_HumInfor(int argc, char *argv[])
{
	APP_AssertParam(argc==2);
	uint8_t DestID = htoi(argv[1]);
	uint8_t *SendBuffer = APP_BGetz(MEM_0,5);
	SendBuffer[0] = 0x04;
	SendBuffer[1] = 2;
	SendBuffer[2] = 0x66;
	SendBuffer[3] = (uint8_t)SensorsData.humidity_value;
	SendBuffer[4] = (uint8_t)(SensorsData.humidity_value*100-SendBuffer[3]*100);
	Comunication_SendFrame(DestID,SendBuffer,5);
	APP_BRel(MEM_0,SendBuffer);
	//
	// Return success.
	//
	return(0);
}
int CMD_PressInfor(int argc, char *argv[])
{
	APP_AssertParam(argc==2);
	uint8_t DestID = htoi(argv[1]);
	uint8_t *SendBuffer = APP_BGetz(MEM_0,6);
	SendBuffer[0] = 0x04;
	SendBuffer[1] = 3;
	SendBuffer[2] = 0x67;
	SendBuffer[3] = ((uint16_t)SensorsData.press_value)/100;
	SendBuffer[4] = ((uint16_t)SensorsData.press_value-SendBuffer[3]*100);
	SendBuffer[5] = ((SensorsData.press_value*100)-SendBuffer[3]*10000-SendBuffer[4]*100);
	Comunication_SendFrame(DestID,SendBuffer,6);
	APP_BRel(MEM_0,SendBuffer);
	//
	// Return success.
	//
	return(0);
}
int CMD_ACCInfor(int argc, char *argv[])
{
	APP_AssertParam(argc==3);
	uint8_t DestID = htoi(argv[1]);
	uint8_t Axis = atoi(argv[2]);
	APP_AssertParam(Axis<=2);
	uint8_t *SendBuffer = APP_BGetz(MEM_0,6);
	SendBuffer[0] = 0x04;
	SendBuffer[1] = 2;
	if(Axis == 0)
		SendBuffer[2] = 0x68;
	else if(Axis == 1)
		SendBuffer[2] = 0x69;
	else if(Axis == 2)
		SendBuffer[2] = 0x6A;
	if(SensorsData.pAccDataXYZ[Axis]<0)
	{
		SensorsData.pAccDataXYZ[Axis] = -SensorsData.pAccDataXYZ[Axis];
		SendBuffer[3] = (uint8_t)(SensorsData.pAccDataXYZ[Axis]/100);
		SendBuffer[4] = (uint8_t)(SensorsData.pAccDataXYZ[Axis]-SendBuffer[3]*100);
		SendBuffer[5] = 1;
	}
	else
	{
		SendBuffer[3] = (uint8_t)(SensorsData.pAccDataXYZ[Axis]/100);
		SendBuffer[4] = (uint8_t)(SensorsData.pAccDataXYZ[Axis]-SendBuffer[3]*100);
		SendBuffer[5] = 0;
	}
	Comunication_SendFrame(DestID,SendBuffer,6);
	APP_BRel(MEM_0,SendBuffer);
	// Return success.
	return(0);
}
int CMD_MAGInfor(int argc, char *argv[])
{
	APP_AssertParam(argc==3);
	uint8_t DestID = htoi(argv[1]);
	uint8_t Axis = atoi(argv[2]);
	APP_AssertParam(Axis<=2);
	uint8_t *SendBuffer = APP_BGetz(MEM_0,6);
	SendBuffer[0] = 0x04;
	SendBuffer[1] = 2;
	if(Axis == 0)
		SendBuffer[2] = 0x6B;
	else if(Axis == 1)
		SendBuffer[2] = 0x6C;
	else if(Axis == 2)
		SendBuffer[2] = 0x6D;
	if(SensorsData.pMagDataXYZ[Axis]<0)
	{
		SensorsData.pMagDataXYZ[Axis] = -SensorsData.pMagDataXYZ[Axis];
		SendBuffer[3] = (uint8_t)(SensorsData.pMagDataXYZ[Axis]/100);
		SendBuffer[4] = (uint8_t)(SensorsData.pMagDataXYZ[Axis]-SendBuffer[3]*100);
		SendBuffer[5] = 1;
	}
	else
	{
		SendBuffer[3] = (uint8_t)(SensorsData.pMagDataXYZ[Axis]/100);
		SendBuffer[4] = (uint8_t)(SensorsData.pMagDataXYZ[Axis]-SendBuffer[3]*100);
		SendBuffer[5] = 0;
	}
	Comunication_SendFrame(DestID,SendBuffer,6);
	APP_BRel(MEM_0,SendBuffer);
	//
	// Return success.
	//
	return(0);
}
int CMD_GYROInfor(int argc, char *argv[])
{
	APP_AssertParam(argc==3);
	uint8_t DestID = htoi(argv[1]);
	uint8_t Axis = atoi(argv[2]);
	APP_AssertParam(Axis<=2);
	uint8_t *SendBuffer = APP_BGetz(MEM_0,7);
	SendBuffer[0] = 0x04;
	SendBuffer[1] = 2;
	if(Axis == 0)
		SendBuffer[2] = 0x6E;
	else if(Axis == 1)
		SendBuffer[2] = 0x6F;
	else if(Axis == 2)
		SendBuffer[2] = 0x70;
	if(SensorsData.pGyroDataXYZ[Axis]>0)
	{
		SendBuffer[3] = ((uint16_t)SensorsData.pGyroDataXYZ[Axis])/100;
		SendBuffer[4] = ((uint16_t)SensorsData.pGyroDataXYZ[Axis]-SendBuffer[3]*100);
		SendBuffer[5] = ((SensorsData.pGyroDataXYZ[Axis]*100)-SendBuffer[3]*10000-SendBuffer[4]*100);
		SendBuffer[6] = 0;
	}
	else
	{
		SensorsData.pGyroDataXYZ[Axis] = -SensorsData.pGyroDataXYZ[Axis];
		SendBuffer[3] = ((uint16_t)SensorsData.pGyroDataXYZ[Axis])/100;
		SendBuffer[4] = ((uint16_t)SensorsData.pGyroDataXYZ[Axis]-SendBuffer[3]*100);
		SendBuffer[5] = ((SensorsData.pGyroDataXYZ[Axis]*100)-SendBuffer[3]*10000-SendBuffer[4]*100);
		SendBuffer[6] = 1;
	}
	Comunication_SendFrame(DestID,SendBuffer,7);
	APP_BRel(MEM_0,SendBuffer);
	// Return success.
	return(0);
}
int CMD_RSSIInfor(int argc, char *argv[])
{
	APP_AssertParam(argc==2);
	uint8_t DestID = htoi(argv[1]);
	uint8_t *SendBuffer = APP_BGetz(MEM_0,7);
	SendBuffer[0] = 0x04;
	SendBuffer[1] = 2;
	SendBuffer[2] = 0x71;
	if(SensorsData.RSSIValue>0)
	{
		SendBuffer[3] = ((uint16_t)SensorsData.RSSIValue)/100;
		SendBuffer[4] = ((uint16_t)SensorsData.RSSIValue-SendBuffer[3]*100);
		SendBuffer[5] = ((SensorsData.RSSIValue*100)-SendBuffer[3]*10000-SendBuffer[4]*100);
		SendBuffer[6] = 0;
	}
	else
	{
		SensorsData.RSSIValue = -SensorsData.RSSIValue;
		SendBuffer[3] = ((uint16_t)SensorsData.RSSIValue)/100;
		SendBuffer[4] = ((uint16_t)SensorsData.RSSIValue-SendBuffer[3]*100);
		SendBuffer[5] = ((SensorsData.RSSIValue*100)-SendBuffer[3]*10000-SendBuffer[4]*100);
		SendBuffer[6] = 1;
	}
	printf("%f\n",SensorsData.RSSIValue);
	Comunication_SendFrame(DestID,SendBuffer,7);
	APP_BRel(MEM_0,SendBuffer);
	//
	// Return success.
	//
	return(0);
}
#include "Board_Wifi.h"
extern WIFIModule_Control_t WIFIModule_Handle;
int CMD_ConnectServer(int argc, char *argv[])
{
	APP_AssertParam(argc==3);
	char *strstart = argv[1];
	for(uint8_t i=0;i<4;i++)
	{
		WIFIModule_Handle.ServerIP[i] = atoi(strstart);
		strstart = strstr(strstart,".")+1;
	}
	WIFIModule_Handle.ServerPort = atoi(argv[2]);
	return(0);
}
//*****************************************************************************
//
// This is the table that holds the command names, implementing functions, and
// brief description.
//
//*****************************************************************************
tCmdLineEntry g_psCmdTable[] =
{
    {"help",   	CMD_help,   	" : ShowCommandOption" },
		{"meminfor",   	CMD_MEMInfor,   	" : ShowMemoryInformation" },
		{"temp",   	CMD_TempInfor,   	" : ShowTempInformation(arg:ID)" },
		{"hum",   	CMD_HumInfor,   	" : ShowHumInformation(arg:ID)" },
		{"acc",   	CMD_ACCInfor,   	" : ShowACCInformation((arg:ID axis)" },
		{"mag",   	CMD_MAGInfor,   	" : ShowMAGInformation((arg:ID axis)" },
		{"gyro",   	CMD_GYROInfor,   	" : ShowGYROInformation((arg:ID axis)" },
		{"press",   	CMD_PressInfor,   	" : ShowPressInformation((arg:ID)" },
		{"RSSI",   	CMD_RSSIInfor,   	" : ShowRSSIInformation((arg:ID)" },
		{"ConnectServer",   	CMD_ConnectServer,   	" : ConnectToTCPServer((arg:IP Port)" },
    { 0, 0, 0 }
};
//*****************************************************************************
//
// Prompts the user for a command, and blocks while waiting for the user's
// input. This function will return after the execution of a single command.
//
//*****************************************************************************
typedef struct
{
	//结构体变量
	bool ReadyFlag;
	uint8_t u8RecChar;
	uint8_t *pu8Databuf;
	uint16_t u16Datalength;
}BSP_USART_Data_t;
extern BSP_USART_Data_t BSP_USARTBuffer;
void APP_Debug_Poll(void)
{
	int iStatus;
	//
	// Get a user command back
	//
	if(BSP_USARTBuffer.ReadyFlag == false)
	{
		//
		// If not, return so other functions get a chance to run.
		//
		return;
	}
	//
	// If we do have commands, process them immediately in the order they were
	// received.
	//
	//
	// Process the received command
	//
	iStatus = CmdLineProcess((char *)BSP_USARTBuffer.pu8Databuf);

	//
	// Handle the case of bad command.
	//
	if(iStatus == CMDLINE_BAD_CMD)
	{
			printf("Bad command!\n");
	}

	//
	// Handle the case of too many arguments.
	//
	else if(iStatus == CMDLINE_TOO_MANY_ARGS)
	{
			printf("Too many arguments for command processor!\n");
	}
	memset(BSP_USARTBuffer.pu8Databuf,0,32);
	BSP_USARTBuffer.u16Datalength=0;
	BSP_USARTBuffer.ReadyFlag = false;
}

