#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.0 2018.8.7
*** History: 1.0 创建
             2.0 为RTE的升级做适配
*****************************************************************************/
/*************************************************
*** Args:   *file  断言失败的文件;
            line 断言失败的行;
*** Function: 断言功能
*************************************************/
void RTE_Assert(char *file, uint32_t line)
{ 
	printf("[ASSERT]    Wrong parameters value: file %s on line %d\n", file, line);
	while (1)
	{
		
	}
}
/*************************************************
*** RTE所管理的内存，静态分配，32位对齐
*************************************************/
#if RTE_USE_BGET == 1
#define ALIGN_32BYTES(buf) buf __attribute__ ((aligned (32)))
ALIGN_32BYTES (uint8_t RTE_RAM[BGET_MEMSIZE]) = {0};
#endif
/*************************************************
*** RTE_Shell的回调函数
*************************************************/
#if HI_USE_SHELL == 1
static void Shell_TimerCallBack(void *Params)
{
	RTE_Shell_Poll();
}
#endif
/*************************************************
*** Args:   NULL
*** Function: RTE初始化
*************************************************/
void RTE_Init(void)
{
#if RTE_USE_BGET == 1
	RTE_BPool(MEM_RTE,RTE_RAM,BGET_MEMSIZE);
#endif
	
#if RTE_USE_HUMMANINTERFACE == 1
	#if HI_USE_SHELL == 1
		RTE_Shell_Init();
		RTE_RoundRobin_CreateTimer("ShellTimer",10,1,1,Shell_TimerCallBack,(void *)0);
	#endif
#endif
	
}
