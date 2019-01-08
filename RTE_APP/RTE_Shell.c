#include "RTE_Shell.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.3 2018.12.08
*** History: 1.0 创建，修改自tivaware
             2.0 为RTE的升级做适配，更改模块名称
						 2.1 动静态结合方式管理
						 2.2 引入RTE_Vec进行统一管理
						 2.3 单独提出ShellQuene，隔离物理层
*****************************************************************************/
#if RTE_USE_SHELL == 1
#include "RTE_RingQuene.h"
#include "RTE_UString.h"
#include "RTE_RoundRobin.h"
#include "RTE_LOG.h"
#include "RTE_MEM.h"
#define DEBUG_STR "[SHELL]"
/*************************************************
*** 管理Shell的结构体变量，动态管理
*************************************************/
static RTE_Shell_Control_t ShellHandle = {0};
static RTE_MessageQuene_t ShellQuene  = {0};  //数据环形队列
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
				if(ui8Argc < SHELL_MAX_ARGS)
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
	for(uint8_t i = 0;i<ShellHandle.g_psCmdTable.length;i++)
	{
		// If this command entry command string matches argv[0], then call
		// the function for this command, passing the command line
		// arguments.
		if(!ustrcmp(ShellHandle.g_ppcArgv[0], ShellHandle.g_psCmdTable.data[i].pcCmd))
		{
			
			RTE_Shell_Err_e retval;
			if(ShellHandle.g_psCmdTable.data[i].pfnCmdLine !=(void *)0)
			{
				retval = (RTE_Shell_Err_e)ShellHandle.g_psCmdTable.data[i].pfnCmdLine(ui8Argc, ShellHandle.g_ppcArgv);
			}
			return retval;
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
	if(ShellHandle.g_psCmdTable.length >= SHELL_MAX_NUM)
		return SHELL_NOSPACEFORNEW;
	for(uint8_t i = 0;i<ShellHandle.g_psCmdTable.length;i++)
	{
		if(!ustrcmp(cmd,ShellHandle.g_psCmdTable.data[i].pcCmd))
			return SHELL_ALREADYEXIST;
	}
	RTE_Shell_t v;
	v.pcCmd = cmd;
	v.pfnCmdLine = func;
	v.pcHelp = help;
	vec_push(&ShellHandle.g_psCmdTable, v);
	return SHELL_NOERR;
}
/*************************************************
*** Args:   *pcCmdLine 待处理命令行
*** Function: 删除一条Shell命令行处理
*************************************************/
RTE_Shell_Err_e RTE_Shell_DeleteCommand(const char *cmd)
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<ShellHandle.g_psCmdTable.length;i++)
	{
		if(!ustrcmp(cmd,ShellHandle.g_psCmdTable.data[i].pcCmd))
		{
			idx = i;
			break;
		}
	}
	if(idx!=-1)
	{
		vec_splice(&ShellHandle.g_psCmdTable, idx, 1);
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
	RTE_Printf("--------------------------------------------------\r\n");
	RTE_Printf("%10s    Available Command\r\n",DEBUG_STR);
	for(uint8_t i = 0;i<ShellHandle.g_psCmdTable.length;i++)
	{
		RTE_Printf("%10s    Name:%16s  Function:%s\r\n", 
			DEBUG_STR,
			ShellHandle.g_psCmdTable.data[i].pcCmd,
			ShellHandle.g_psCmdTable.data[i].pcHelp);
	}
	return(SHELL_NOERR);
}
/*************************************************
*** Args:   NULL
*** Function: shell自带的信息函数
*************************************************/
static RTE_Shell_Err_e RTE_Shell_CMD_RTEInfor(int argc, char *argv[])
{
	RTE_Printf("--------------------------------------------------\r\n");
#if RTE_USE_OS
	extern volatile uint8_t   StaticsCPUUsage; 
	RTE_Printf("RTE Version:%s CPU:%d%%\r\n",RTE_VERSION,StaticsCPUUsage);
#else
	RTE_Printf("RTE Version:%s\r\n",RTE_VERSION);
#endif
	RTE_Printf("--------------------------------------------------\r\n");
	RTE_Printf("%10s    Using SHELL Nums:%d Allow Max:%d VEC Capbility:%d\r\n",
		DEBUG_STR,
		ShellHandle.g_psCmdTable.length,
		SHELL_MAX_NUM,
		ShellHandle.g_psCmdTable.capacity);
	RTE_MEM_Monitor_t mon_infor = {0};
	for(uint8_t i=0;i<MEM_N;i++)
	{
		RTE_MEM_Monitor((RTE_MEM_Name_e)i,&mon_infor);
		RTE_Printf("--------------------------------------------------\r\n");
		RTE_Printf("%10s    Memery Bank:%d Infor Use/All:%d%%/%d Max Free/Free:%d/%d Percent of Frag:%d%%\r\n",
				DEBUG_STR,
				i,
				mon_infor.used_pct,
				mon_infor.total_size,
				mon_infor.free_biggest_size,
				mon_infor.free_size,
				mon_infor.frag_pct);
	}
	RTE_RoundRobin_Demon();
	return(SHELL_NOERR);
}
/*************************************************
*** Args:   NULL
*** Function: 初始化shell工具
*************************************************/
void RTE_Shell_Init(void)
{
	vec_init(&ShellHandle.g_psCmdTable);
	RTE_AssertParam(RTE_Shell_AddCommand("help",RTE_Shell_CMD_Help,"Available help when using Shell") == SHELL_NOERR);
	RTE_AssertParam(RTE_Shell_AddCommand("system",RTE_Shell_CMD_RTEInfor,"Information about running SL_RTE") == SHELL_NOERR);
	RTE_MessageQuene_Init(&ShellQuene,SHELL_BUFSIZE);
}
/*************************************************
*** Args:   NULL
*** Function: shell轮询
*************************************************/
void RTE_Shell_Poll(void *Params)
{
	uint16_t BufferLenth= 0;
	char *ShellBuffer = RTE_MEM_Alloc0(MEM_RTE,SHELL_BUFSIZE);
	if(RTE_MessageQuene_Out(&ShellQuene,(uint8_t *)ShellBuffer,&BufferLenth) == MSG_NO_ERR)
	{
		int iStatus;
		iStatus = RTE_Shell_CommandProcess(ShellBuffer);
		if(iStatus == SHELL_NOVALIDCMD)
		{
			RTE_LOG_WARN(DEBUG_STR,"Can't identify such command:%s!\r\n",ShellBuffer);
		}
		else if(iStatus == SHELL_TOOMANYARGS)
		{
			RTE_LOG_WARN(DEBUG_STR,"Input command's args' count is more than max!\r\n");
		}
	}
	RTE_MEM_Free(MEM_RTE,ShellBuffer);
}

void RTE_Shell_Input(uint8_t *Data,uint16_t Length)
{
	RTE_MessageQuene_In(&ShellQuene,Data,Length);
}
#endif
