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
#include "RTE_UStdout.h"
#include "RTE_RoundRobin.h"
#include "RTE_Memory.h"
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
	bool bFindModule = true;
	bool bFindFuction = false;
	bool bFindArg = false;
	// Initialize the argument counter, and point to the beginning of the
	// command line string.
	ui8Argc = 0;
	pcChar = pcCmdLine;
	// Advance through the command line until a zero character is found.
	while(*pcChar)
	{
	    if(*pcChar == '.')
        {
			*pcChar = 0;
			bFindModule = true;
        }
        else if(*pcChar == '(')
        {
			*pcChar = 0;
			bFindFuction = true;
        }
		// If there is a space, then replace it with a zero, and set the flag
		// to search for the next argument.
		else if(*pcChar == ',')
		{
			*pcChar = 0;
			bFindArg = true;
		}
		else if(*pcChar == ')')
        {
            *pcChar = 0;
        }
		// Otherwise it is not a space, so it must be a character that is part
		// of an argument.
		else
		{
		    if(bFindModule)
		    {
                ShellHandle.g_ppcArgv[ui8Argc] = pcChar;
                ui8Argc++;
                bFindModule = false;
		    }
		    else if(bFindFuction)
            {
                ShellHandle.g_ppcArgv[ui8Argc] = pcChar;
                ui8Argc++;
                bFindFuction = false;
            }
			// If bFindArg is set, then that means we are looking for the start
			// of the next argument.
			else if(bFindArg)
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
	for(uint8_t i = 0;i<ShellHandle.ShellModuleCnt&&ui8Argc>=2;i++)
    {
		if(!ustrcmp(ShellHandle.g_ppcArgv[0],ShellHandle.ShellModule[i].ShellModuleName))
		{
            // If one or more arguments was found, then process the command.
            for(uint8_t i2 = 0;i2<ShellHandle.ShellModule[i].ModuleFucTable.length;i2++)
            {
                // If this command entry command string matches argv[0], then call
                // the function for this command, passing the command line
                // arguments.
                if(!ustrcmp(ShellHandle.g_ppcArgv[1], ShellHandle.ShellModule[i].ModuleFucTable.data[i2].pcCmd))
                {

                    RTE_Shell_Err_e retval;
                    if(ShellHandle.ShellModule[i].ModuleFucTable.data[i2].pfnCmdLine !=(void *)0)
                    {
                        retval = (RTE_Shell_Err_e)ShellHandle.ShellModule[i].ModuleFucTable.data[i2].pfnCmdLine(ui8Argc, ShellHandle.g_ppcArgv);
                    }
                    return retval;
                }
            }
		}
    }
	return(SHELL_NOVALIDCMD);
}
/*************************************************
*** Args:   *module 模块名
*** Function: 增加一个Shell命令行处理模块
*************************************************/
RTE_Shell_Err_e RTE_Shell_CreateModule(const char *module)
{
	if(ShellHandle.ShellModuleCnt>=SHELL_MAX_MODULE)
		return SHELL_NOSPACEFORNEW;
	for(uint8_t i = 0;i<ShellHandle.ShellModuleCnt;i++)
	{
		if(!strcmp(module,ShellHandle.ShellModule[i].ShellModuleName))
			return SHELL_ALREADYEXIST;
	}
	ShellHandle.ShellModule[ShellHandle.ShellModuleCnt].ShellModuleID = ShellHandle.ShellModuleCnt;
	ShellHandle.ShellModule[ShellHandle.ShellModuleCnt].ShellModuleName = module;
	vec_init(&ShellHandle.ShellModule[ShellHandle.ShellModuleCnt].ModuleFucTable);
	ShellHandle.ShellModuleCnt++;
	return SHELL_NOERR;
}
/*************************************************
*** Args:   *cmd 命令行字符串
            *func 命令行执行函数
            *help 帮助字符串
*** Function: 增加一条Shell命令行处理
*************************************************/
RTE_Shell_Err_e RTE_Shell_AddCommand(const char *module,const char *cmd,RTE_Shell_Err_e (*func)(int argc, char *argv[]),const char *help)
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<ShellHandle.ShellModuleCnt;i++)
	{
		if(!ustrcmp(module,ShellHandle.ShellModule[i].ShellModuleName))
		{
			idx = i;
			break;
		}
	}
	if(idx == -1)
        return SHELL_NOMODULE;
	if(ShellHandle.ShellModule[idx].ModuleFucTable.length >= SHELL_MAX_MODULE_FUC)
		return SHELL_NOSPACEFORNEW;
	for(uint8_t i = 0;i<ShellHandle.ShellModule[idx].ModuleFucTable.length;i++)
	{
		if(!strcmp(cmd,ShellHandle.ShellModule[idx].ModuleFucTable.data[i].pcCmd))
			return SHELL_ALREADYEXIST;
	}
	RTE_Shell_t v;
	v.pcCmd = cmd;
	v.pfnCmdLine = func;
	v.pcHelp = help;
	vec_push(&ShellHandle.ShellModule[idx].ModuleFucTable,v);
	return SHELL_NOERR;
}
/*************************************************
*** Args:   *pcCmdLine 待处理命令行
*** Function: 删除一条Shell命令行处理
*************************************************/
RTE_Shell_Err_e RTE_Shell_DeleteCommand(const char*module,const char *cmd)
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<ShellHandle.ShellModuleCnt;i++)
	{
		if(!ustrcmp(module,ShellHandle.ShellModule[i].ShellModuleName))
		{
			idx = i;
			break;
		}
	}
	if(idx == -1)
        return SHELL_NOMODULE;
	int8_t idx2 = -1;
	for(uint8_t i = 0;i<ShellHandle.ShellModule[idx].ModuleFucTable.length;i++)
	{
		if(!ustrcmp(cmd,ShellHandle.ShellModule[idx].ModuleFucTable.data[i].pcCmd))
		{
			idx2 = i;
			break;
		}
	}
	if(idx2 == -1)
        return SHELL_NOSUCHCMD;
    vec_splice(&ShellHandle.ShellModule[idx].ModuleFucTable, idx2, 1);
    return SHELL_NOERR;
}
/*************************************************
*** Args:   NULL
*** Function: shell自带的帮助函数
*************************************************/
static RTE_Shell_Err_e RTE_Shell_CMD_Help(int argc, char *argv[])
{
	RTE_Printf("--------------------------------------------------\r\n");
	RTE_Printf("RTE Version:%s\r\n",RTE_VERSION);
	RTE_Printf("--------------------------------------------------\r\n");
	RTE_Printf("%10s    Available Module\r\n",DEBUG_STR);
	for(uint8_t i = 0;i<ShellHandle.ShellModuleCnt;i++)
	{
	    RTE_Printf("---------------------------\r\n");
	    RTE_Printf("%10s      Module Name:%s\r\n",DEBUG_STR,ShellHandle.ShellModule[i].ShellModuleName);
	    for(uint8_t i2 = 0;i2<ShellHandle.ShellModule[i].ModuleFucTable.length;i2++)
        {
            RTE_Printf("%10s        Fuction Name:%16s---:%s\r\n",
                DEBUG_STR,
                ShellHandle.ShellModule[i].ModuleFucTable.data[i2].pcCmd,
                ShellHandle.ShellModule[i].ModuleFucTable.data[i2].pcHelp);
        }
	}
	return(SHELL_NOERR);
}
/*************************************************
*** Args:   NULL
*** Function: shell自带的输出函数
*************************************************/
static RTE_Shell_Err_e RTE_Shell_CMD_Print(int argc, char *argv[])
{
    if(argc == 3)
        RTE_Printf("%s\r\n",argv[2]);
    else if(argc%2 == 0)
    {
        for(uint8_t i=2;i<argc;i++)
        {
            if(!ustrcmp("%d",argv[i]))
            {

            }
            else if(!ustrcmp("%s",argv[i]))
            {

            }
            else if(!ustrcmp("%x",argv[i]))
            {

            }
            else if(!ustrcmp("%f",argv[i]))
            {

            }
        }
    }
	return(SHELL_NOERR);
}
/*************************************************
*** Args:   NULL
*** Function: 初始化shell工具
*************************************************/
void RTE_Shell_Init(void)
{
	ShellHandle.ShellModule = (RTE_Shell_Module_t *)
	Memory_Alloc0(BANK_RTE,sizeof(RTE_Shell_Module_t)*SHELL_MAX_MODULE);
    ShellHandle.ShellModuleCnt = 0;
    RTE_Shell_CreateModule("system");
	RTE_Shell_AddCommand("system","help",RTE_Shell_CMD_Help,"Available help when using Shell Example:system.help");
	RTE_Shell_AddCommand("system","print",RTE_Shell_CMD_Print,"Simple printf Example:system.print(str)");
	RTE_MessageQuene_Init(&ShellQuene,SHELL_BUFSIZE);
}
/*************************************************
*** Args:   NULL
*** Function: shell轮询
*************************************************/
void RTE_Shell_Poll(void *Params)
{
	char ShellBuffer[SHELL_BUFSIZE] = {0};
	if(RTE_MessageQuene_Out(&ShellQuene,(uint8_t *)ShellBuffer,NULL) == MSG_NO_ERR)
	{
		int iStatus;
		iStatus = RTE_Shell_CommandProcess(ShellBuffer);
		if(iStatus == SHELL_NOVALIDCMD)
		{
			RTE_Printf("%10s    Can't identify such command:%s!\r\n",DEBUG_STR,ShellBuffer);
		}
		else if(iStatus == SHELL_TOOMANYARGS)
		{
			RTE_Printf("%10s    Input command's args' count is more than max!\r\n",DEBUG_STR);
		}
		else if(iStatus == SHELL_ARGSERROR)
        {
            RTE_Printf("%10s    Input command's args' num error!\r\n",DEBUG_STR);
        }
	}
}

void RTE_Shell_Input(uint8_t *Data,uint16_t Length)
{
	RTE_MessageQuene_In(&ShellQuene,Data,Length);
}
#endif
