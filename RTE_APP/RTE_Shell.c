#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.1 2018.8.31
*** History: 1.0 创建，修改自tivaware
             2.0 为RTE的升级做适配，更改模块名称
						 2.1 动静态结合方式管理
*****************************************************************************/
#if HI_USE_SHELL == 1
/*************************************************
*** 管理Shell的结构体变量，动态管理
*************************************************/
static RTE_Shell_Control_t ShellHandle = 
{
	.CmdCnt = 0,
	.CmdBuffer = (void *)0,
	.g_psCmdTable = (void *)0,
};
/*************************************************
*** Args:   *pcCmdLine 待处理命令行
*** Function: Shell命令行处理
*************************************************/
static RTE_Shell_Err_e RTE_Shell_CommandProcess(char *pcCmdLine)
{
	char *pcChar;
	uint_fast8_t ui8Argc;
	bool bFindArg = true;
	// Initialize the argument counter, and point to the beginning of the
	// command line string.
	ui8Argc = 0;
	pcChar = pcCmdLine;
	// Advance through the command line until a zero character is found.
	while(*pcChar)
	{
		// If there is a space, then replace it with a zero, and set the flag
		// to search for the next argument.
		if(*pcChar == ' ')
		{
			*pcChar = 0;
			bFindArg = true;
		}
		// Otherwise it is not a space, so it must be a character that is part
		// of an argument.
		else
		{
			// If bFindArg is set, then that means we are looking for the start
			// of the next argument.
			if(bFindArg)
			{
				// As long as the maximum number of arguments has not been
				// reached, then save the pointer to the start of this new arg
				// in the argv array, and increment the count of args, argc.
				if(ui8Argc < HI_SHELL_MAX_ARGS)
				{
						ShellHandle.g_ppcArgv[ui8Argc] = pcChar;
						ui8Argc++;
						bFindArg = false;
				}
				// The maximum number of arguments has been reached so return
				// the error.
				else
				{
						return(SHELL_TOOMANYARGS);
				}
			}
		}
		// Advance to the next character in the command line.
		pcChar++;
	}
	// If one or more arguments was found, then process the command.
	if(ui8Argc)
	{
		for(uint8_t i = 0;i<ShellHandle.CmdCnt;i++)
		{
			// If this command entry command string matches argv[0], then call
			// the function for this command, passing the command line
			// arguments.
			if(!strcmp(ShellHandle.g_ppcArgv[0], ShellHandle.g_psCmdTable[i].pcCmd))
			{
				RTE_Shell_Err_e retval;
				if(ShellHandle.g_psCmdTable[i].pfnCmdLine !=(void *)0)
				{
					retval = (RTE_Shell_Err_e)ShellHandle.g_psCmdTable[i].pfnCmdLine(ui8Argc, ShellHandle.g_ppcArgv);
				}
				return retval;
			}
		}
	}
	return(SHELL_NOVALIDCMD);
}
/*************************************************
*** Args:   *cmd 命令行字符串
            *func 命令行执行函数
            *help 帮助字符串
*** Function: 增加一条Shell命令行处理
*************************************************/
RTE_Shell_Err_e RTE_Shell_AddCommand(const char *cmd,RTE_Shell_Err_e (*func)(int argc, char *argv[]),const char *help)
{
	if(ShellHandle.g_psCmdTable == (void*)0)
		return SHELL_UNINITHANDLE;
	if(ShellHandle.CmdCnt >= HI_SHELL_MAX_NUM)
		return SHELL_NOSPACEFORNEW;
	for(uint8_t i = 0;i<ShellHandle.CmdCnt;i++)
	{
		if(!strcmp(cmd,ShellHandle.g_psCmdTable[i].pcCmd))
			return SHELL_ALREADYEXIST;
	}
	ShellHandle.g_psCmdTable[ShellHandle.CmdCnt].pcCmd = cmd;
	ShellHandle.g_psCmdTable[ShellHandle.CmdCnt].pfnCmdLine = func;
	ShellHandle.g_psCmdTable[ShellHandle.CmdCnt].pcHelp = help;
	ShellHandle.CmdCnt++;
	return SHELL_NOERR;
}
/*************************************************
*** Args:   *pcCmdLine 待处理命令行
*** Function: 删除一条Shell命令行处理
*************************************************/
RTE_Shell_Err_e RTE_Shell_DeleteCommand(const char *cmd)
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<ShellHandle.CmdCnt;i++)
	{
		if(!strcmp(cmd,ShellHandle.g_psCmdTable[i].pcCmd))
		{
			idx = i;
			break;
		}
	}
	if(idx!=-1)
	{
		for(uint8_t i = idx;i<ShellHandle.CmdCnt-1;i++)
		{
			ShellHandle.g_psCmdTable[i].pcCmd = 
				ShellHandle.g_psCmdTable[i+1].pcCmd;
			ShellHandle.g_psCmdTable[i].pfnCmdLine = 
				ShellHandle.g_psCmdTable[i+1].pfnCmdLine;
			ShellHandle.g_psCmdTable[i].pcHelp = 
				ShellHandle.g_psCmdTable[i+1].pcHelp;
		}
		ShellHandle.g_psCmdTable[ShellHandle.CmdCnt].pcCmd = (void *)0;
		ShellHandle.g_psCmdTable[ShellHandle.CmdCnt].pfnCmdLine = (void *)0;
		ShellHandle.g_psCmdTable[ShellHandle.CmdCnt].pcHelp = (void *)0;
		ShellHandle.CmdCnt--;
		return SHELL_NOERR;
	}
	return SHELL_NOSUCHCMD;
}
/*************************************************
*** Args:   NULL
*** Function: shell自带的帮助函数
*************************************************/
static RTE_Shell_Err_e RTE_Shell_CMD_Help(int argc, char *argv[])
{
	RTE_Printf("*********************************\r\n");
	RTE_Printf("[SHELL]    可用指令\r\n");
	RTE_Printf("[SHELL]    ------------------\r\n");
	for(uint8_t i = 0;i<ShellHandle.CmdCnt;i++)
	{
		RTE_Printf("[SHELL]    ComandName:%16s----功能:%s\r\n", ShellHandle.g_psCmdTable[i].pcCmd
					,ShellHandle.g_psCmdTable[i].pcHelp);
	}
	return(SHELL_NOERR);
}
#if RTE_USE_BGET == 1
/*************************************************
*** Args:   NULL
*** Function: shell自带的信息函数
*************************************************/
static RTE_Shell_Err_e RTE_Shell_CMD_RTEInfor(int argc, char *argv[])
{
	RTE_BGet_Monitor_t mon_infor = {0};
	RET_BGet_Monitor(MEM_RTE,&mon_infor);
	RTE_Printf("*********************************\r\n");
	RTE_Printf("[SHELL]    RTE版本号：%s\r\n",RTE_VERSION);
	RTE_Printf("[SHELL]    系统详细信息\r\n");
	RTE_Printf("[SHELL]    ------------------\r\n");
	RTE_Printf("[MEM]    当前内存使用情况 已用/全部:%d%%/%d 最大空余/空余:%d/%d 碎片比例:%d%%\r\n",
			mon_infor.pct_used,
			mon_infor.size_total,
	    mon_infor.size_free,
			mon_infor.size_free_big,
			mon_infor.pct_frag);
	RTE_Printf("[SHELL]    ------------------\r\n");
	RTE_Printf("[SHELL]    当前SHELL指令使用数目：%d 最大数目：%d\r\n",ShellHandle.CmdCnt,HI_SHELL_MAX_NUM);
	RTE_Printf("[SHELL]    ------------------\r\n");
	RTE_RoundRobin_Demon();
	return(SHELL_NOERR);
}
#endif
/*************************************************
*** Args:   NULL
*** Function: 初始化shell工具
*************************************************/
void RTE_Shell_Init(void)
{
	ShellHandle.CmdBuffer = (char *)RTE_BGetz(MEM_RTE,HI_SHELL_MAX_BUFSIZE);
	RTE_AssertParam(ShellHandle.CmdBuffer);
	ShellHandle.g_psCmdTable = (RTE_Shell_t *)RTE_BGetz(MEM_RTE,(HI_SHELL_MAX_NUM + 1) * sizeof(RTE_Shell_t));
	ShellHandle.g_psCmdTable[HI_SHELL_MAX_NUM].pcCmd = (void *)0;
	ShellHandle.g_psCmdTable[HI_SHELL_MAX_NUM].pfnCmdLine = (void *)0;
	ShellHandle.g_psCmdTable[HI_SHELL_MAX_NUM].pcHelp = (void *)0;
	RTE_AssertParam(ShellHandle.g_psCmdTable);
	RTE_AssertParam(RTE_Shell_AddCommand("帮助",RTE_Shell_CMD_Help,"SHELL使用帮助") == SHELL_NOERR);
	RTE_AssertParam(RTE_Shell_AddCommand("系统信息",RTE_Shell_CMD_RTEInfor,"显示RTE系统信息") == SHELL_NOERR);
}
/*************************************************
*** Args:   NULL
*** Function: 初始化shell工具
*************************************************/
#include "BSP_Com.h"
void RTE_Shell_Poll(void)
{
	int iStatus;
	uint16_t lenth;
	BSP_COM_Data_t *debugdata;
	debugdata = BSP_COM_ReturnQue(COM_1);
	if(RTE_MessageQuene_Out(&debugdata->ComQuene,(uint8_t *)ShellHandle.CmdBuffer,&lenth)!=MSG_NO_ERR)
	{
		return;
	}
	iStatus = RTE_Shell_CommandProcess(ShellHandle.CmdBuffer);
	if(iStatus == SHELL_NOVALIDCMD)
	{
		RTE_Printf("[SHELL]    不能识别的CMD!\r\n");
	}
	else if(iStatus == SHELL_TOOMANYARGS)
	{
		RTE_Printf("[SHELL]    输入参数数目超限!\r\n");
	}
	memset(ShellHandle.CmdBuffer,0,HI_SHELL_MAX_BUFSIZE);
}
#endif
