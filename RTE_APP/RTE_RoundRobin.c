#include "RTE_RoundRobin.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.4 2018.10.8
*** History: 1.0 创建，修改自tivaware
             2.0 为RTE的升级做适配，更改模块名称
						 2.1 动静态结合方式管理
						 2.2 引入RTE_Vec进行统一管理
						 2.3 多线程环境下引入ALONE机制，确保某些关键timer可以在独立得线程中运行
						 2.4 引入多线程多Timer机制，不同线程的Timer按不同线程进行管理，简化已有的查询timer机制
*****************************************************************************/
#if RTE_USE_ROUNDROBIN == 1
#include "RTE_Log.h"
#include "RTE_MEM.h"
#include "RTE_UString.h"
#define RR_STR "[RR]"
#include "RTE_Components.h"
#include CMSIS_device_header
/*************************************************
*** 管理RoundRobin的结构体变量，动态管理
*************************************************/
static RTE_RoundRobin_t RoundRobinHandle = {0};
/*************************************************
*** Args:   NULL
*** Function: RoundRobin初始化
*************************************************/
void RTE_RoundRobin_Init(void)
{
	RoundRobinHandle.TimerGroup = (RTE_RoundRobin_TimerGroup_t *)
	RTE_MEM_Alloc0(MEM_RTE,sizeof(RTE_RoundRobin_TimerGroup_t)*ROUNDROBIN_MAX_GROUP_NUM);
#if RTE_USE_OS == 0
	RoundRobinHandle.RoundRobinRunTick = 0;
#endif
	RoundRobinHandle.TimerGroupCnt = 0;
#ifndef RTE_Compiler_EventRecorder
	/* Enable TRC */
	CoreDebug->DEMCR &= ~0x01000000;
	CoreDebug->DEMCR |=  0x01000000;
	/* Enable counter */
	DWT->CTRL &= ~0x00000001;
	DWT->CTRL |=  0x00000001;
	/* Reset counter */
	DWT->CYCCNT = 0;	
	/* 2 dummys */
	__ASM volatile ("NOP");
	__ASM volatile ("NOP");
#endif
}
/*************************************************
*** Args:   NULL
*** Function: RoundRobin TimerGroup初始化
*************************************************/
RTE_RoundRobin_Err_e RTE_RoundRobin_CreateGroup(const char *GroupName)
{
	if(RoundRobinHandle.TimerGroupCnt>=ROUNDROBIN_MAX_GROUP_NUM)
		return RR_NOSPACEFORNEW;
	for(uint8_t i = 0;i<RoundRobinHandle.TimerGroupCnt;i++)
	{
		if(!strcmp(GroupName,RoundRobinHandle.TimerGroup[i].TimerGroupName))
			return RR_ALREADYEXIST;
	}
	RoundRobinHandle.TimerGroup[RoundRobinHandle.TimerGroupCnt].TimerGroupID = RoundRobinHandle.TimerGroupCnt;
	RoundRobinHandle.TimerGroup[RoundRobinHandle.TimerGroupCnt].TimerGroupName = GroupName;
	vec_init(&RoundRobinHandle.TimerGroup[RoundRobinHandle.TimerGroupCnt].SoftTimerTable);
	RoundRobinHandle.TimerGroupCnt++;
	return RR_NOERR;
}
/*************************************************
*** Args:   NULL
*** Function: 获取RoundRobin TimerGroup ID
*************************************************/
int8_t RTE_RoundRobin_GetGroupID(const char *GroupName)
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<RoundRobinHandle.TimerGroupCnt;i++)
	{
		if(!ustrcmp(GroupName,RoundRobinHandle.TimerGroup[i].TimerGroupName))
		{
			idx = i;
			break;
		}
	}
	if(idx == -1)
		RTE_LOG_WARN(RR_STR,"No such group");
	return idx;
}
/*************************************************
*** Args:   
          GroupID 定时器所属Group的ID
					*TimerName 待添加定时器名称
					ReloadValue 重装载值
          ReloadEnable 重装载使能
          ReloadEnable 定时器运行使能
          *TimerCallback 定时器回调函数
          *UserParameters 回调函数输入参数
*** Function: 为当前RoundRobin环境的某一Group添加一个软定时器
*************************************************/
extern RTE_RoundRobin_Err_e RTE_RoundRobin_CreateTimer(
	uint8_t GroupID,
	const char *TimerName,
	uint32_t ReloadValue, 
	uint8_t ReloadEnable, 
	uint8_t RunEnable,
	void (*TimerCallback)(void *), 
	void* UserParameters)
{
	if(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.length >= ROUNDROBIN_MAX_NUM)
		return RR_NOSPACEFORNEW;
	for(uint8_t i = 0;i<RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.length;i++)
	{
		if(!strcmp(TimerName,RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[i].TimerName))
			return RR_ALREADYEXIST;
	}
	RTE_SoftTimer_t v = {0};
	v.TimerID = RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.length;
	v.TimerName = TimerName;
	v.Flags.F.AREN = ReloadEnable;
	v.Flags.F.CNTEN = RunEnable;
	v.ARR = ReloadValue;
	v.CNT = ReloadValue;
	v.Callback = TimerCallback;
	v.UserParameters = UserParameters;
	vec_push(&RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable, v);
	return RR_NOERR;
}
/*************************************************
*** Args:   NULL
*** Function: 获取RoundRobin Timer ID
*************************************************/
int8_t RTE_RoundRobin_GetTimerID(uint8_t GroupID,const char *TimerName)
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.length;i++)
	{
		if(!ustrcmp(TimerName,RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[i].TimerName))
		{
			idx = i;
			break;
		}
	}
	if(idx == -1)
		RTE_LOG_WARN(RR_STR,"No such group");
	return idx;
}
/*************************************************
*** Args:   
					*Name 待删除定时器名称
*** Function: 为当前RoundRobin环境删除一个软定时器
*************************************************/
RTE_RoundRobin_Err_e RTE_RoundRobin_RemoveTimer(uint8_t GroupID,uint8_t TimerID)
{
	vec_splice(&RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable, TimerID, 1);
	return RR_NOERR;
}

/*************************************************
*** Args:   Timer 待处理定时器
*** Function: SoftTimer按时处理
*************************************************/
inline static void RTE_RoundRobin_CheckTimer(uint8_t GroupID,uint8_t TimerID)
{
	/* Check if count is zero */
	if(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].CNT == 0) 
	{
		/* Call user callback function */
		RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Callback(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].UserParameters);
		/* Set new counter value */
		RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].CNT = RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].ARR;
		/* Disable timer if auto reload feature is not used */
		if (!RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Flags.F.AREN)
		{
			/* Disable counter */
			RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Flags.F.CNTEN = 0;
			//RTE_RoundRobin_RemoveTimer(GroupID,TimerID);
		}
	}
}
/*************************************************
*** Args:   Null
*** Function: RoundRobin时基函数
*************************************************/
void RTE_RoundRobin_TickHandler(void)
{
#if RTE_USE_OS == 0
	RoundRobinHandle.RoundRobinRunTick++;
#endif
	// Loop through each group in the group table.
	for(uint8_t i = 0; i < RoundRobinHandle.TimerGroupCnt; i++)
	{
		// Loop through each task in the task table.
		for(uint8_t j = 0; j < RoundRobinHandle.TimerGroup[i].SoftTimerTable.length; j++)
		{
			/* Check if timer is enabled */
			if (RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].Flags.F.CNTEN)  /*!< Timer is enabled */
			{
				/* Decrease counter if needed */
				if (RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].CNT) 
					RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].CNT--;
			}
		}
	}
#if RTE_USE_OS == 1
	RTE_RoundRobin_Run(0);
#endif
}
/*************************************************
*** Args:   Null
*** Function: RoundRobin运行函数 在非操作系统环境下调用
*************************************************/
void RTE_RoundRobin_Run(uint8_t GroupID)
{
	// Loop through each task in the task table.
	for(uint8_t i = 0; i < RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.length; i++)
	{
		RTE_RoundRobin_CheckTimer(GroupID,i);
	}
}
/*************************************************
*** Args:   
					*Name 待就绪定时器名称
*** Function: 复位当前RoundRobin环境中的一个软定时器
*************************************************/
RTE_RoundRobin_Err_e RTE_RoundRobin_ReadyTimer(uint8_t GroupID,uint8_t TimerID)
{
	RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].CNT = 0;
	RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Flags.F.CNTEN = 1;
	return RR_NOERR;
}
/*************************************************
*** Args:   
					*Name 待复位定时器名称
*** Function: 复位当前RoundRobin环境中的一个软定时器
*************************************************/
RTE_RoundRobin_Err_e RTE_RoundRobin_ResetTimer(uint8_t GroupID,uint8_t TimerID)
{
	RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].CNT = RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].ARR;
	RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Flags.F.CNTEN = 1;
	return RR_NOERR;
}
/*************************************************
*** Args:   
					*Name 待暂停定时器名称
*** Function: 暂停当前RoundRobin环境中的一个软定时器
*************************************************/
RTE_RoundRobin_Err_e RTE_RoundRobin_PauseTimer(uint8_t GroupID,uint8_t TimerID)
{
	RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Flags.F.CNTEN = 0;
	return RR_NOERR;
}
/*************************************************
*** Args:   
					*Name 待暂停定时器名称
*** Function: 恢复当前RoundRobin环境中的一个软定时器
*************************************************/
RTE_RoundRobin_Err_e RTE_RoundRobin_ResumeTimer(uint8_t GroupID,uint8_t TimerID)
{
	RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Flags.F.CNTEN = 1;
	return RR_NOERR;
}
/*************************************************
*** Args:   
					*Name 待暂停定时器名称
*** Function: 恢复当前RoundRobin环境中的一个软定时器
*************************************************/
bool RTE_RoundRobin_IfRunTimer(uint8_t GroupID,uint8_t TimerID)
{
	return RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable.data[TimerID].Flags.F.CNTEN;
}
/*************************************************
*** Args:   
					Null
*** Function: 获取当前RoundRobin环境信息
*************************************************/
void RTE_RoundRobin_Demon(void)
{
	RTE_Printf("--------------------------------------------------\r\n");
	for(uint8_t i=0;i<RoundRobinHandle.TimerGroupCnt;i++)
	{
		RTE_Printf("-------------------------\r\n");
		RTE_Printf("%10s    Group:%02d %16s Using Timer Count:%d Max Num:%d VEC Capbility:%d\r\n",
		RR_STR,
		RoundRobinHandle.TimerGroup[i].TimerGroupID,
		RoundRobinHandle.TimerGroup[i].TimerGroupName,
		RoundRobinHandle.TimerGroup[i].SoftTimerTable.length,
		ROUNDROBIN_MAX_NUM,
		RoundRobinHandle.TimerGroup[i].SoftTimerTable.capacity);
		for(uint8_t j=0;j<RoundRobinHandle.TimerGroup[i].SoftTimerTable.length;j++)
		{
			RTE_Printf("%10s    %02d.TIMER:%16s----AutoReload Enable:%x AutoReload Val:%6d Now Val:%6d Run Enable:%x\r\n",
			RR_STR,
			RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].TimerID,
			RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].TimerName,
			RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].Flags.F.AREN,
			RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].ARR,
			RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].CNT,
			RoundRobinHandle.TimerGroup[i].SoftTimerTable.data[j].Flags.F.CNTEN);
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
	if (osKernelGetState () == osKernelRunning) 
	{
    return osKernelGetTickCount();
  }
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
/*************************************************
*** Args:   
					prev_tick a previous time stamp (return value of systick_get() )
*** Function: 获取两次tick之间时间差
*************************************************/
uint32_t RTE_RoundRobin_TickElaps(uint32_t prev_tick)
{
	uint32_t act_time = RTE_RoundRobin_GetTick();
	/*If there is no overflow in sys_time simple subtract*/
	if(act_time >= prev_tick) {
		prev_tick = act_time - prev_tick;
	} else {
		prev_tick = UINT32_MAX - prev_tick + 1;
		prev_tick += act_time;
	}
	return prev_tick;
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
#if RTE_USE_OS == 0
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
*** Args:   micros 微秒
*** Function: 延时微秒，不影响系统调度
*************************************************/
__inline void RTE_RoundRobin_DelayUS(volatile uint32_t micros) {
	uint32_t start = DWT->CYCCNT;
	/* Go to number of cycles for system */
	micros *= (SystemCoreClock / 1000000);
	/* Delay till end */
	while ((DWT->CYCCNT - start) < micros);
}
#endif
