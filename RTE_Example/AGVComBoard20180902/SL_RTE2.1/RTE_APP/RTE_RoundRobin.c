#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.0 2018.8.7
*** History: 1.0 创建，修改自tivaware
             2.0 为RTE的升级做适配，更改模块名称
*****************************************************************************/
#if RTE_USE_ROUNDROBIN == 1
#if RTE_USE_OS == 1
#include "RTE_Components.h"
#include CMSIS_device_header
#endif
/*************************************************
*** 管理RoundRobin的结构体变量，动态管理
*************************************************/
static RTE_RoundRobin_t RoundRobinHandle = 
{
	.SoftTimerCnt = 0,
	.RoundRobinRunTick = 0,
	.SoftTimerTable = (void *)0,
};
/*************************************************
*** Args:   Timer 待处理定时器
*** Function: SoftTimer按时处理
*************************************************/
static void RTE_RoundRobin_CheckTimer(uint8_t Timer)
{
	/* Check if count is zero */
	if (RoundRobinHandle.SoftTimerTable[Timer].CNT == 0) {
		/* Call user callback function */
		RoundRobinHandle.SoftTimerTable[Timer].Callback(RoundRobinHandle.SoftTimerTable[Timer].UserParameters);
		RoundRobinHandle.SoftTimerTable[Timer].CNT = RoundRobinHandle.SoftTimerTable[Timer].ARR;
		/* Pause timer if auto reload feature is not used */
		if (!RoundRobinHandle.SoftTimerTable[Timer].AREN) {
			/* Set new counter value */
			RoundRobinHandle.SoftTimerTable[Timer].CNTEN = 0;
		}
	}
}
/*************************************************
*** Args:   Null
*** Function: RoundRobin时基函数
*************************************************/
void RTE_RoundRobin_TickHandler(void)
{
  RoundRobinHandle.RoundRobinRunTick++;
	// Loop through each task in the task table.
	for(uint8_t i = 0; i < RoundRobinHandle.SoftTimerCnt; i++)
	{
    if(RoundRobinHandle.SoftTimerTable[i].CNTEN)
		{
			/* Decrease counter if needed */
			if (RoundRobinHandle.SoftTimerTable[i].CNT)
				RoundRobinHandle.SoftTimerTable[i].CNT--;
#if RTE_USE_OS == 1
			RTE_RoundRobin_CheckTimer(i);
#endif
		}
	}
}
#if RTE_USE_OS == 0
/*************************************************
*** Args:   Null
*** Function: RoundRobin运行函数 在非操作系统环境下调用
*************************************************/
void RTE_RoundRobin_Run(void)
{
	// Loop through each task in the task table.
	for(uint8_t i = 0; i < RoundRobinHandle.SoftTimerCnt; i++)
	{
		RTE_RoundRobin_CheckTimer(i);
	}
}
#endif
/*************************************************
*** Args:   
					*Name 待添加定时器名称
					ReloadValue 重装载值
          ReloadEnable 重装载使能
          ReloadEnable 定时器运行使能
          *TimerCallback 定时器回调函数
          *UserParameters 回调函数输入参数
*** Function: 为当前RoundRobin环境添加一个软定时器
*************************************************/
uint8_t RTE_RoundRobin_CreateTimer(const char *Name,
	uint32_t ReloadValue, 
	uint8_t ReloadEnable, 
	uint8_t RunEnable, 
	void (*TimerCallback)(void *), 
	void* UserParameters)
{
	for(uint8_t i = 0;i<RoundRobinHandle.SoftTimerCnt;i++)
	{
		if(!strcmp(Name,RoundRobinHandle.SoftTimerTable[i].TimerName))
		{
			//该定时器已经存在
			return 1;
		}
	}
	if(RoundRobinHandle.SoftTimerTable == (void*)0)
	{
		RoundRobinHandle.SoftTimerTable = (RTE_SoftTimer_t *)RTE_BGetz(MEM_RTE,sizeof(RTE_SoftTimer_t));
		RTE_AssertParam(RoundRobinHandle.SoftTimerTable != (void*)0);
		//return 1;
	}
	else
	{
		RoundRobinHandle.SoftTimerTable = (RTE_SoftTimer_t *)RTE_BGetr(MEM_RTE,
						RoundRobinHandle.SoftTimerTable,
						(RoundRobinHandle.SoftTimerCnt+1) * sizeof(RTE_SoftTimer_t));
		RTE_AssertParam(RoundRobinHandle.SoftTimerTable != (void*)0);
		//return 1;
	}
	RoundRobinHandle.SoftTimerTable[RoundRobinHandle.SoftTimerCnt].TimerName = Name;
	RoundRobinHandle.SoftTimerTable[RoundRobinHandle.SoftTimerCnt].AREN = ReloadEnable;
	RoundRobinHandle.SoftTimerTable[RoundRobinHandle.SoftTimerCnt].CNTEN = RunEnable;
	RoundRobinHandle.SoftTimerTable[RoundRobinHandle.SoftTimerCnt].ARR = ReloadValue;
	RoundRobinHandle.SoftTimerTable[RoundRobinHandle.SoftTimerCnt].CNT = ReloadValue;
	RoundRobinHandle.SoftTimerTable[RoundRobinHandle.SoftTimerCnt].Callback = TimerCallback;
	RoundRobinHandle.SoftTimerTable[RoundRobinHandle.SoftTimerCnt].UserParameters = UserParameters;
	RoundRobinHandle.SoftTimerCnt++;
	return 0;
}
/*************************************************
*** Args:   
					*Name 待删除定时器名称
*** Function: 为当前RoundRobin环境删除一个软定时器
*************************************************/
uint8_t RTE_RoundRobin_RemoveTimer(const char *Name)
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<RoundRobinHandle.SoftTimerCnt;i++)
	{
		if(!strcmp(Name,RoundRobinHandle.SoftTimerTable[i].TimerName))
		{
			idx = i;
			break;
		}
	}
	if(idx!=-1)
	{
		RTE_SoftTimer_t *tmp = (void *)0;
		tmp = (RTE_SoftTimer_t *)RTE_BGetz(MEM_RTE,
						(RoundRobinHandle.SoftTimerCnt-1) * sizeof(RTE_SoftTimer_t));
		RTE_AssertParam(tmp != (void*)0);
    if ((1 < RoundRobinHandle.SoftTimerCnt) && (idx < (RoundRobinHandle.SoftTimerCnt - 1))) 
		{
			//0 1 2 3 4 5
			//a b c d e f
			//memcpy(tmp,add,idx)
			//0 1 2 3 4
			//a b c e f
			//memcpy(tmp,add,idx)
			memcpy(tmp,RoundRobinHandle.SoftTimerTable,(idx) * sizeof(RTE_SoftTimer_t));
			memcpy(tmp+idx, RoundRobinHandle.SoftTimerTable+idx+1, (RoundRobinHandle.SoftTimerCnt-idx-1) * sizeof(RTE_SoftTimer_t));
			RTE_BRel(MEM_RTE,RoundRobinHandle.SoftTimerTable);
			RoundRobinHandle.SoftTimerTable = tmp;
			//return 2;
		}
		else if(RoundRobinHandle.SoftTimerCnt == 1)
		{
			RTE_BRel(MEM_RTE,RoundRobinHandle.SoftTimerTable);
		}
		else
		{
			memcpy(tmp,RoundRobinHandle.SoftTimerTable,(idx) * sizeof(RTE_SoftTimer_t));
			RTE_BRel(MEM_RTE,RoundRobinHandle.SoftTimerTable);
			RoundRobinHandle.SoftTimerTable = tmp;
		}
		RoundRobinHandle.SoftTimerCnt--;
		return 0;
	}
	return 1;
}
/*************************************************
*** Args:   
					*Name 待暂停定时器名称
*** Function: 暂停当前RoundRobin环境中的一个软定时器
*************************************************/
uint8_t RTE_RoundRobin_PauseTimer(const char *Name) 
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<RoundRobinHandle.SoftTimerCnt;i++)
	{
		if(!strcmp(Name,RoundRobinHandle.SoftTimerTable[i].TimerName))
		{
			idx = i;
			break;
		}
	}
	if(idx!=-1)
	{
		RoundRobinHandle.SoftTimerTable[idx].CNTEN = 0;
	}
	else
		return 1;
	return 0;
}
/*************************************************
*** Args:   
					*Name 待暂停定时器名称
*** Function: 恢复当前RoundRobin环境中的一个软定时器
*************************************************/
uint8_t RTE_RoundRobin_ResumeTimer(const char *Name) 
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<RoundRobinHandle.SoftTimerCnt;i++)
	{
		if(!strcmp(Name,RoundRobinHandle.SoftTimerTable[i].TimerName))
		{
			idx = i;
			break;
		}
	}
	if(idx!=-1)
	{
		RoundRobinHandle.SoftTimerTable[idx].CNTEN = 1;
	}
	else
		return 1;
	return 0;
}
/*************************************************
*** Args:   
					*Name 待暂停定时器名称
*** Function: 获取当前RoundRobin环境中的一个软定时器状态
*************************************************/
uint8_t RTE_RoundRobin_GetTimerState(const char *Name) 
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<RoundRobinHandle.SoftTimerCnt;i++)
	{
		if(!strcmp(Name,RoundRobinHandle.SoftTimerTable[i].TimerName))
		{
			idx = i;
			break;
		}
	}
	if(idx!=-1)
	{
		return RoundRobinHandle.SoftTimerTable[idx].CNTEN;
	}
	else
		return 0;
}
/*************************************************
*** Args:   
					*Name 待复位定时器名称
*** Function: 复位当前RoundRobin环境中的一个软定时器
*************************************************/
uint8_t RTE_RoundRobin_ResetTimer(const char *Name) 
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<RoundRobinHandle.SoftTimerCnt;i++)
	{
		if(!strcmp(Name,RoundRobinHandle.SoftTimerTable[i].TimerName))
		{
			idx = i;
			break;
		}
	}
	if(idx!=-1)
	{
		RoundRobinHandle.SoftTimerTable[idx].CNT = RoundRobinHandle.SoftTimerTable[idx].ARR;
		RoundRobinHandle.SoftTimerTable[idx].CNTEN = 1;
	}
	else
		return 1;
	return 0;
}
/*************************************************
*** Args:   Delay
					Null
*** Function: 延时一段毫秒
*************************************************/
void RTE_RoundRobin_DelayMS(uint32_t Delay) {
	/* Delay for amount of milliseconds */
	/* Check if we are called from ISR */
	if (__get_IPSR() == 0) {
		/* Called from thread mode */
		uint32_t tickstart = RTE_RoundRobin_GetTick();
		/* Count interrupts */
		while ((RTE_RoundRobin_GetTick() - tickstart) < Delay) {
#if USE_LOWPOWER == 1
			/* Go sleep, wait systick interrupt */
			__WFI();
#endif
		}
	} else {
		/* Called from interrupt mode */
		while (Delay) {
			/* Check if timer reached zero after we last checked COUNTFLAG bit */
			if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
				Delay--;
			}
		}
	}
}
/*************************************************
*** Args:   
					Null
*** Function: 获取当前RoundRobin环境运行时间
*************************************************/
uint32_t RTE_RoundRobin_GetTick(void) 
{
	/* Return current time in milliseconds */
#if RTE_USE_OS == 1
	#ifdef RTE_CMSIS_RTOS
	if (osKernelRunning () == true) 
	{
    return APPSysRunTime;
  }
	#else
	if (osKernelGetState () == osKernelRunning) 
	{
    return osKernelGetTickCount();
  }
	#endif
	else
	{
		static uint32_t ticks = 0U;
					 uint32_t i;
		/* If Kernel is not running wait approximately 1 ms then increment 
			 and return auxiliary tick counter value */
		for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
			__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
			__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		}
		return ++ticks;
	}
#else
	return RoundRobinHandle.RoundRobinRunTick;
#endif 
}
#endif
