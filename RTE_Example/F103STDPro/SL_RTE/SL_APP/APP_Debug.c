#include "APP_Debug.h"
#include "BSP_Com.h"
//*****************************************************************************
//
// Input buffer for the command line interpreter.
//
//*****************************************************************************
static char *DebugBuffer;
void APP_Debug_Init(void)
{
	DebugBuffer = (char *)APP_BGetz(MEM_0,DEBUG_BUF_SIZE);
	if(!DebugBuffer)
		APP_Assert((char *)__FILE__, __LINE__);
}
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
    { 0, 0, 0 }
};
//*****************************************************************************
//
// Prompts the user for a command, and blocks while waiting for the user's
// input. This function will return after the execution of a single command.
//
//*****************************************************************************
void APP_Debug_Poll(void)
{
	int iStatus;
	uint16_t lenth;
	BSP_USART_Data_t *debugdata;
	debugdata=BSP_USART_ReturnQue(COM_1);
	//
	// Get a user command back
	//
	if(APP_MessageQuene_Out(&debugdata->ComQuene,(uint8_t *)DebugBuffer,&lenth)!=MSG_NO_ERR)
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
	iStatus = CmdLineProcess(DebugBuffer);

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
	memset(DebugBuffer,0,DEBUG_BUF_SIZE);
}

