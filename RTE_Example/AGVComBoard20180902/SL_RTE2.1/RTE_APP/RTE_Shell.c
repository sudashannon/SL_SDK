#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.0 2018.8.7
*** History: 1.0 创建，修改自tivaware
             2.0 为RTE的升级做适配
*****************************************************************************/
#if HI_USE_SHELL == 1
/*************************************************
*** 管理Shell的结构体变量，动态管理
*************************************************/
static RTE_Shell_Control_t Shell_ControlHandle = 
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
	RTE_Shell_t *psCmdEntry;
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
						Shell_ControlHandle.g_ppcArgv[ui8Argc] = pcChar;
						ui8Argc++;
						bFindArg = false;
				}
				// The maximum number of arguments has been reached so return
				// the error.
				else
				{
						return(CMDLINE_TOO_MANY_ARGS);
				}
			}
		}
		// Advance to the next character in the command line.
		pcChar++;
	}
	if(Shell_ControlHandle.CmdCnt>0)
	{
		// If one or more arguments was found, then process the command.
		if(ui8Argc)
		{
			// Start at the beginning of the command table, to look for a matching
			// command.
			psCmdEntry = &Shell_ControlHandle.g_psCmdTable[0];
			// Search through the command table until a null command string is
			// found, which marks the end of the table.
			while(psCmdEntry->pcCmd)
			{
				// If this command entry command string matches argv[0], then call
				// the function for this command, passing the command line
				// arguments.
				if(!strcmp(Shell_ControlHandle.g_ppcArgv[0], psCmdEntry->pcCmd))
				{
					RTE_Shell_Err_e retval;
					retval = (RTE_Shell_Err_e)psCmdEntry->pfnCmdLine(ui8Argc, Shell_ControlHandle.g_ppcArgv);
					return retval;
				}
				// Not found, so advance to the next entry.
				psCmdEntry++;
			}
		}
		// Fall through to here means that no matching command was found, so return
		// an error.
		return(CMDLINE_BAD_CMD);
	}
	return(CMDLINE_NO_CMDS);
}
///*************************************************
//*** Args:   *cmd 命令行字符串
//            *func 命令行执行函数
//            *help 帮助字符串
//*** Function: 增加一条Shell命令行处理
//*************************************************/
//RTE_Shell_Err_e RTE_Shell_AddCommand(const char *cmd,int (*func)(int argc, char *argv[]),const char *help)
//{
//	if(Shell_ControlHandle.g_psCmdTable == (void*)0)
//	{
//		Shell_ControlHandle.g_psCmdTable = (RTE_Shell_t *)RTE_BGetz(MEM_RTE,sizeof(RTE_Shell_t));
//		RTE_AssertParam(Shell_ControlHandle.g_psCmdTable != (void*)0);
//	}
//	else
//	{
//		Shell_ControlHandle.g_psCmdTable = (RTE_Shell_t *)RTE_BGetr(MEM_RTE,Shell_ControlHandle.g_psCmdTable,(Shell_ControlHandle.CmdCnt+1) * sizeof(RTE_Shell_t));
//		RTE_AssertParam(Shell_ControlHandle.g_psCmdTable != (void*)0);
//	}
//	Shell_ControlHandle.g_psCmdTable[Shell_ControlHandle.CmdCnt].pcCmd = cmd;
//	Shell_ControlHandle.g_psCmdTable[Shell_ControlHandle.CmdCnt].pfnCmdLine = func;
//	Shell_ControlHandle.g_psCmdTable[Shell_ControlHandle.CmdCnt].pcHelp = help;
//	Shell_ControlHandle.CmdCnt++;
//	return SHELL_NO_ERR;
//}
///*************************************************
//*** Args:   *pcCmdLine 待处理命令行
//*** Function: 删除一条Shell命令行处理
//*************************************************/
//RTE_Shell_Err_e RTE_Shell_DeleteCommand(const char *cmd)
//{
//	int8_t idx = -1;
//	for(uint8_t i = 0;i<Shell_ControlHandle.CmdCnt;i++)
//	{
//		if(!strcmp(cmd,Shell_ControlHandle.g_psCmdTable[i].pcCmd))
//		{
//			idx = i;
//			break;
//		}
//	}
//	if(idx!=-1)
//	{
//		RTE_Shell_t *tmp = (void *)0;
//		tmp = (RTE_Shell_t *)RTE_BGetz(MEM_RTE,
//						(Shell_ControlHandle.CmdCnt-1) * sizeof(RTE_Shell_t));
//		RTE_AssertParam(tmp != (void*)0);
//    if ((1 < Shell_ControlHandle.CmdCnt) && (idx < (Shell_ControlHandle.CmdCnt - 1))) 
//		{
//			memcpy(tmp,Shell_ControlHandle.g_psCmdTable,(idx) * sizeof(RTE_Shell_t));
//			memcpy(tmp+idx, Shell_ControlHandle.g_psCmdTable+idx+1, (Shell_ControlHandle.CmdCnt-idx-1) * sizeof(RTE_Shell_t));
//			RTE_BRel(MEM_RTE,Shell_ControlHandle.g_psCmdTable);
//			Shell_ControlHandle.g_psCmdTable = tmp;
//			//return 2;
//		}
//		else if(Shell_ControlHandle.CmdCnt == 1)
//		{
//			RTE_BRel(MEM_RTE,Shell_ControlHandle.g_psCmdTable);
//		}
//		else
//		{
//			memcpy(tmp,Shell_ControlHandle.g_psCmdTable,(idx) * sizeof(RTE_Shell_t));
//			RTE_BRel(MEM_RTE,Shell_ControlHandle.g_psCmdTable);
//			Shell_ControlHandle.g_psCmdTable = tmp;
//		}
//		Shell_ControlHandle.CmdCnt--;
//		return SHELL_NO_ERR;
//	}
//	return CDMLINE_DELETE_FAIL;
//}
#include "CAN_STATE.h"
#include "CAN_SEND.h"
/*************************************************
*** Args:   NULL
*** Function: shell自带的帮助函数
*************************************************/
static int RTE_Shell_CMD_Help(int argc, char *argv[])
{
	RTE_Shell_t *pEntry;
	printf("[SHELL]    Available commands\n");
	printf("[SHELL]    ------------------\n");
	pEntry = &Shell_ControlHandle.g_psCmdTable[0];
	while(pEntry->pcCmd)
	{
		printf("[SHELL]    ComandName:%s    功能:%s\n", pEntry->pcCmd, pEntry->pcHelp);
		pEntry++;
	}
	return(0);
}
#if RTE_USE_BGET == 1
/*************************************************
*** Args:   NULL
*** Function: shell自带的内存信息显示函数
*************************************************/
static int RTE_Shell_CMD_RamInfor(int argc, char *argv[])
{
	RTE_BGet_Monitor_t mon_infor = {0};
	RET_BGet_Monitor(MEM_RTE,&mon_infor);
	printf("[RAM]    Total:%d UsedPercent:%d%% TotalFree:%d MaxFree:%d GarbagePercent:%d%%\n",
			mon_infor.size_total,
			mon_infor.pct_used,
	    mon_infor.size_free,
			mon_infor.size_free_big,
			mon_infor.pct_frag);
	return(0);
}
#endif
/*************************************************
*** Args:   NULL
*** Function: shell自带的Apply
*************************************************/
static int RTE_Shell_CMD_Apply(int argc, char *argv[])
{
	if(argc != 4)
	{
		printf("[SHELL]    缺少参数！\n");
		return(0);
	}
	if(!strcmp(argv[3],ef_get_env("agvpassword")))
	{
		uint8_t DestFloor = atoi(argv[1]);
		uint8_t CurrentFloor = atoi(argv[2]);
		printf("[SHELL]    Apply目的楼层：%d 当前楼层：%d\n",DestFloor,CurrentFloor);
		CANDataSend(AGV_APPLY_CMD,DestFloor,CurrentFloor);
	}
	else
	{
		printf("[SHELL]    密码错误！\n");
		return(0);
	}
	return(0);
}
/*************************************************
*** Args:   NULL
*** Function: shell自带的Excute
*************************************************/
static int RTE_Shell_CMD_Excute(int argc, char *argv[])
{
	if(argc != 4)
	{
		printf("[SHELL]    缺少参数！\n");
		return(0);
	}
	if(!strcmp(argv[3],ef_get_env("agvpassword")))
	{
		uint8_t DestFloor = atoi(argv[1]);
		uint8_t CurrentFloor = atoi(argv[2]);
		printf("[SHELL]    Excute目的楼层：%d 当前楼层：%d\n",DestFloor,CurrentFloor);
		CANDataSend(AGV_EXCUTE_CMD,DestFloor,CurrentFloor);
	}
	else
	{
		printf("[SHELL]    密码错误！\n");
		return(0);
	}
	return(0);
}
/*************************************************
*** Args:   NULL
*** Function: shell自带的Release
*************************************************/
static int RTE_Shell_CMD_Release(int argc, char *argv[])
{
	if(argc != 4)
	{
		printf("[SHELL]    缺少参数！\n");
		return(0);
	}
	if(!strcmp(argv[3],ef_get_env("agvpassword")))
	{
		uint8_t DestFloor = atoi(argv[1]);
		uint8_t CurrentFloor = atoi(argv[2]);
		printf("[SHELL]    Release目的楼层：%d 当前楼层：%d\n",DestFloor,CurrentFloor);
		CANDataSend(AGV_RELEASE_CMD,DestFloor,CurrentFloor);
	}
	else
	{
		printf("[SHELL]    密码错误！\n");
		return(0);
	}
	return(0);
}
/*************************************************
*** Args:   NULL
*** Function: shell自带的Release
*************************************************/
static int RTE_Shell_CMD_Admin(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("[SHELL]    缺少参数！\n");
		return(0);
	}
	if(!strcmp(argv[1],ef_get_env("adminpassword")))
	{
    ef_set_env("agvpassword", argv[2]);
    ef_save_env();
		printf("[SHELL]    新密码为：%s\n",ef_get_env("agvpassword"));
	}
	else
	{
		printf("[SHELL]    管理员密码错误！\n");
		return(0);
	}
	return(0);
}
/*************************************************
*** Args:   NULL
*** Function: shell自带的Clear
*************************************************/
#include "AGV_SM.h"
static int RTE_Shell_CMD_Clear(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("[SHELL]    缺少参数！\n");
		return(0);
	}
	if(!strcmp(argv[2],ef_get_env("agvpassword")))
	{
    ef_set_env("taskid", argv[1]);
    ef_save_env();
		printf("[SHELL]    新ID为：%s\n",ef_get_env("taskid"));
		if(!strcmp(argv[1],"0"))
		{
			printf("[SHELL]    清除任务\n");
			memset(AGV_RunningTask.LastTaskDetailKey,0,50);
			memset(AGV_RunningTask.TaskDetailKey,0,50);
		}
		else
		{
			memcpy(AGV_RunningTask.LastTaskDetailKey,ef_get_env("taskid"),strlen(ef_get_env("taskid")));
			memcpy(AGV_RunningTask.TaskDetailKey,ef_get_env("taskid"),strlen(ef_get_env("taskid")));
		}
	}
	else
	{
		printf("[SHELL]    管理员密码错误！\n");
		return(0);
	}
	return(0);
}
/*************************************************
*** Args:   NULL
*** Function: shell自带的手动模式
*************************************************/
static int RTE_Shell_CMD_Hand(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("[SHELL]    缺少参数！\n");
		return(0);
	}
	if(!strcmp(argv[1],ef_get_env("agvpassword")))
	{
		if(EleStatus.WorkMode == 0x02)
		{
			printf("[SHELL]    已经是手动模式\n");
		}
		else if(EleStatus.WorkMode == 0x0f)
		{
			if(!strcmp("0",ef_get_env("taskid")))
			{
				printf("[SHELL]    准备进入手动模式...\n");
				CANAGVModeControl(false);
			}
			else
			{
				printf("[SHELL]    进入手动模式失败 有未完成的任务\n");
			}
		}
	}
	else
	{
		printf("[SHELL]    密码错误！\n");
		return(0);
	}
	return(0);
}
/*************************************************
*** Args:   NULL
*** Function: shell自带的自动模式
*************************************************/
static int RTE_Shell_CMD_Auto(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("[SHELL]    缺少参数！\n");
		return(0);
	}
	if(!strcmp(argv[1],ef_get_env("agvpassword")))
	{
		if(EleStatus.WorkMode == 0x0f)
		{
			printf("[SHELL]    已经是自动模式\n");
		}
		else if(EleStatus.WorkMode == 0x02)
		{
			printf("[SHELL]    准备进入自动模式...\n");
			CANAGVModeControl(true);
		}
	}
	else
	{
		printf("[SHELL]    密码错误！\n");
		return(0);
	}
	return(0);
}
#include "BSP_Time.h"
/*************************************************
*** Args:   NULL
*** Function: shell自带的自动模式
*************************************************/
static int RTE_Shell_CMD_TimeSet(int argc, char *argv[])
{
	if(argc != 7)
	{
		printf("[SHELL]    缺少参数！\n");
		return(0);
	}
	if(!strcmp(argv[1],ef_get_env("agvpassword")))
	{
		uint32_t Hour,Min,Mon,Day,Year = 0;
		Year = atol(argv[2]);
		Mon = atol(argv[3]);
		Day = atol(argv[4]);
		Hour = atol(argv[5]);
		Min = atol(argv[6]);
		printf("[SHELL]    设置时间成功！");
		printf("%d %d %d %d %d\n",Year,Mon,Day,Hour,Min);
		BSP_Time_Set(Hour,Min,Mon-1,Day,Year);
	}
	else
	{
		printf("[SHELL]    密码错误！\n");
		return(0);
	}
	return(0);
}
/*************************************************
*** Args:   NULL
*** Function: shell自带的自动模式
*************************************************/
static int RTE_Shell_CMD_TimeShow(int argc, char *argv[])
{
	BSP_Time_Display();
	return(0);
}
static RTE_Shell_t g_psCmdTable[] =
{
    {"帮助",   	RTE_Shell_CMD_Help,   	"显示指令项" },
		{"内存",   	RTE_Shell_CMD_RamInfor, "显示内存使用情况" },
		{"申请",			RTE_Shell_CMD_Apply,		"手动申请电梯"},
		{"执行",			RTE_Shell_CMD_Excute,		"手动电梯执行"},
		{"释放",			RTE_Shell_CMD_Release,	"手动释放电梯"},
		{"设置任务",			RTE_Shell_CMD_Clear,"设置任务ID"},
		{"设置时间",			RTE_Shell_CMD_TimeSet,"设置系统时间"},
		{"时间",			RTE_Shell_CMD_TimeShow,"显示系统时间"},
		{"手动模式",			RTE_Shell_CMD_Hand,"进入手动模式"},
		{"自动模式",			RTE_Shell_CMD_Auto,"进入自动模式"},
		{"Admin",		RTE_Shell_CMD_Admin,		"管理员"},
    { 0, 0, 0 }
};
/*************************************************
*** Args:   NULL
*** Function: 初始化shell工具
*************************************************/
void RTE_Shell_Init(void)
{
	Shell_ControlHandle.CmdBuffer = (char *)RTE_BGetz(MEM_RTE,HI_SHELL_MAX_BUFSIZE);
	RTE_AssertParam(Shell_ControlHandle.CmdBuffer);
	Shell_ControlHandle.CmdCnt = sizeof(g_psCmdTable)/sizeof(g_psCmdTable[0]);
	Shell_ControlHandle.g_psCmdTable = g_psCmdTable;
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
#if DEBUG_USART == UART0_BASE
	debugdata = BSP_COM_ReturnQue(COM_0);
#else
	debugdata = BSP_COM_ReturnQue(COM_2);
#endif
	if(RTE_MessageQuene_Out(&debugdata->ComQuene,(uint8_t *)Shell_ControlHandle.CmdBuffer,&lenth)!=MSG_NO_ERR)
	{
		return;
	}
	iStatus = RTE_Shell_CommandProcess(Shell_ControlHandle.CmdBuffer);
	if(iStatus == CMDLINE_BAD_CMD)
	{
		printf("[SHELL]    不能识别的CMD!\n");
	}
	else if(iStatus == CMDLINE_TOO_MANY_ARGS)
	{
		printf("[SHELL]    输入参数过多!\n");
	}
	memset(Shell_ControlHandle.CmdBuffer,0,HI_SHELL_MAX_BUFSIZE);
}
#endif
