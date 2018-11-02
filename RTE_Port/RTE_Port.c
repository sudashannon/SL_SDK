#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.2 2018.10.02
*** History: 1.0 创建
             2.0 为RTE的升级做适配
						 2.1 增加了几个互斥锁
						 2.2 为增加得GUI内核做适配
*****************************************************************************/
/*************************************************
*** Args:   *file  断言失败的文件;
            line 断言失败的行;
*** Function: 断言功能
*************************************************/
void RTE_Assert(char *file, uint32_t line)
{ 
	RTE_Printf("[ASSERT]    Wrong parameters value: file %s on line %d\r\n", file, line);
	while (1)
	{
		
	}
}
/*************************************************
*** RTE_Stdout 依赖的输出函数
*************************************************/
void RTE_Puts (const char *pcString,uint16_t length)
{
#if RTE_USE_OS == 1
	osMutexAcquire(MutexIDStdio,osWaitForever);
	HAL_UART_Transmit(&UsartHandle[USART_DEBUG].UsartHalHandle, (uint8_t *)pcString,length,HAL_MAX_DELAY);
	osMutexRelease(MutexIDStdio);
#else
	;
#endif
}
/*************************************************
*** RTE所管理的内存，静态分配，32位对齐
*************************************************/
#if RTE_USE_MEMMANAGE == 1
RTE_ALIGN_32BYTES (uint8_t RTE_RAM[RTE_MEM_SIZE * 1024]) = {0};
#endif
/*************************************************
*** RTE_Shell的回调函数
*************************************************/
#if RTE_USE_SHELL == 1
//#include "Board_Usart.h"
static void Shell_TimerCallBack(void *Params)
{
	uint8_t *ShellBuffer = RTE_MEM_Alloc0(MEM_RTE,SHELL_BUFSIZE);
	uint16_t BufferLenth= 0;
//	Board_Usart_Data_t *ShellData;
//	ShellData = Board_Usart_ReturnQue(USART_DEBUG);
//	if(RTE_MessageQuene_Out(&ShellData->ComQuene,(uint8_t *)ShellBuffer,&BufferLenth) == MSG_NO_ERR)
//	{
//		if(BufferLenth)
//		{
//			RTE_Shell_Poll((char *)ShellBuffer);
//		}
//	}
	RTE_MEM_Free(MEM_RTE,ShellBuffer);
}
#endif
/*************************************************
*** Args:   NULL
*** Function: RTE初始化
*************************************************/
void RTE_Init(void)
{
#if RTE_USE_MEMMANAGE == 1
	RTE_MEM_Pool(MEM_RTE,RTE_RAM,RTE_MEM_SIZE*1024);
#endif
#if RTE_USE_ROUNDROBIN == 1
	RTE_RoundRobin_Init();
	RTE_RoundRobin_CreateGroup("RTEGroup");
	#if RTE_USE_SHELL == 1
		RTE_Shell_Init();
		RTE_RoundRobin_CreateTimer(0,"ShellTimer",20,1,1,Shell_TimerCallBack,(void *)0);
	#endif
#endif
#if RTE_USE_STDOUT != 1
	#if RTE_USE_OS == 1
	static const osMutexAttr_t MutexIDStdioAttr = {
		"StdioMutex",     // human readable mutex name
		0U,    					// attr_bits
		NULL,                // memory for control block   
		0U                   // size for control block
	};
	MutexIDStdio = osMutexNew(&MutexIDStdioAttr);
	#endif
#endif
}
