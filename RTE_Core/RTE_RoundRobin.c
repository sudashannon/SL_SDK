#include "RTE_RoundRobin.h"
/**
  ******************************************************************************
  * @file    RTE_RoundRobin.c
  * @author  Shan Lei ->>lurenjia.tech ->>https://github.com/sudashannon
  * @brief   RTE自带的时间片轮询。
  * @version V3.0 2019.06.07
  * @History V1.0 创建，修改自tivaware
             V2.0 为RTE的升级做适配，更改模块名称
			 V2.1 动静态结合方式管理
			 V2.2 引入RTE_Vec进行统一管理
			 V2.3 多线程环境下引入ALONE机制，确保某些关键timer可以在独立的线程中运行
			 V2.4 引入多线程多Group机制，不同线程的Timer形成一个Group进行管理，简化已有的查询机制
             V2.5 使用LVGL的动态数组模块替换原来的vec
			 V3.0 归并不同模式，方便选择
  ******************************************************************************
  */
#if RTE_USE_ROUNDROBIN == 1
#include <string.h>
#if RR_DEBUG == 1
#include "RTE_Printf.h"
#endif
#define RR_STR "[RR]"
#include "RTE_Components.h"
#include CMSIS_device_header
#if RR_TYPE == 0
/*************************************************
*** 管理RoundRobin的结构体变量，静态管理
*************************************************/
static rr_softtimer_t RoundRobinTimerHandle[TIMER_N] = {0}
#if RTE_USE_OS == 0
static volatile uint32_t RoundRobinRunTick = 0;
#endif	
#elif RR_TYPE == 1 || RR_TYPE == 2
static rr_t RoundRobinHandle = {0};
#endif
/*************************************************
*** Args:   NULL
*** Function: RoundRobin初始化
*************************************************/
void RoundRobin_Init(void)
{
#if RR_TYPE == 0
	//简单模式不需要额外配置
#elif RR_TYPE == 1
	//基本模式需要配置心跳计数和定时器向量
	Vector_Init(&RoundRobinHandle.SoftTimerTable,MEM_RTE);
#if RTE_USE_OS == 0
	RoundRobinHandle.RoundRobinRunTick = 0;
#endif
#elif RR_TYPE == 2
	//完整模式需要配置定时器组和心跳计数
	RoundRobinHandle.TimerGroup = (rr_timergroup_t *)
		Memory_Alloc0(MEM_RTE,sizeof(rr_timergroup_t)*RR_MAX_GROUP_NUM);
	RoundRobinHandle.TimerGroupCnt = 0;
#if RTE_USE_OS == 0
	RoundRobinHandle.RoundRobinRunTick = 0;
#endif
#endif
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
*** Args:
          GroupID 定时器所属Group的ID 该参数只针对FULL模式生效
          TimerID 待处理定时器ID
*** Function: SoftTimer处理函数
*************************************************/
inline static void RoundRobin_CheckTimer(
#if RR_TYPE == 2
	uint8_t GroupID,
#endif
	uint8_t TimerID)
{
#if RR_TYPE == 0
	/* Check if count is zero */
	if(RoundRobinTimerHandle[TimerID].Flags.F.CNTEN && RoundRobinTimerHandle[TimerID].CNT == 0)
	{
		/* Call user callback function */
		RoundRobinTimerHandle[TimerID].Callback(RoundRobinTimerHandle[TimerID].UserParameters);
		/* Set new counter value */
		RoundRobinTimerHandle[TimerID].CNT = RoundRobinTimerHandle[TimerID].ARR;
		/* Disable timer if auto reload feature is not used */
		if (!RoundRobinTimerHandle[TimerID].Flags.F.AREN)
		{
			/* Disable counter */
			RoundRobin_RemoveTimer(TimerID);
		}
	}
#elif RR_TYPE == 1
    rr_softtimer_t *Timer = ((rr_softtimer_t*)Vector_GetElement(RoundRobinHandle.SoftTimerTable,TimerID));
	/* Check if count is zero */
	if(Timer->Flags.F.CNTEN && Timer->CNT == 0)
	{
		/* Call user callback function */
		Timer->Callback(Timer->UserParameters);
		/* Set new counter value */
		Timer->CNT = Timer->ARR;
		/* Disable timer if auto reload feature is not used */
		if (!Timer->Flags.F.AREN)
		{
			/* Disable counter */
			RoundRobin_RemoveTimer(TimerID);
		}
	}
#elif RR_TYPE == 2
    rr_softtimer_t *Timer = ((rr_softtimer_t *)Vector_GetElement(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable,TimerID));
	/* Check if count is zero */
	if(Timer->Flags.F.CNTEN && Timer->CNT == 0)
	{
		/* Call user callback function */
		Timer->Callback(Timer->UserParameters);
		/* Set new counter value */
		Timer->CNT = Timer->ARR;
		/* Disable timer if auto reload feature is not used */
		if (!Timer->Flags.F.AREN)
		{
			/* Disable counter */
			RoundRobin_RemoveTimer(GroupID,TimerID);
		}
	}
#endif
}
/*************************************************
*** Args:   Null
*** Function: RoundRobin时基函数，在总时基中断中调用
*************************************************/
void RoundRobin_TickHandler(void)
{
#if RR_TYPE == 0
#if RTE_USE_OS == 0
	RoundRobinRunTick++;
#endif
	// Loop through each timer in the timer table.
	for(uint8_t i = 0; i < TIMER_N; i++)
	{
		if (RoundRobinTimerHandle[i].Flags.F.CNTEN) 
		{
			/* Decrease counter if needed */
			if (RoundRobinTimerHandle[i].CNT)
				RoundRobinTimerHandle[i].CNT--;
		}
	}
#elif RR_TYPE == 1
	// Loop through each task in the task table.
	for(uint8_t i = 0; i < Vector_Length(RoundRobinHandle.SoftTimerTable); i++)
	{
		rr_softtimer_t *Timer = ((rr_softtimer_t*)Vector_GetElement(RoundRobinHandle.SoftTimerTable,i));
		/* Check if timer is enabled */
		if (Timer->Flags.F.CNTEN)  /*!< Timer is enabled */
		{
			/* Decrease counter if needed */
			if (Timer->CNT)
				Timer->CNT--;
		}
	}
#elif RR_TYPE == 2
	// Loop through each group in the group table.
	for(uint8_t i = 0; i < RoundRobinHandle.TimerGroupCnt; i++)
	{
		// Loop through each task in the task table.
		for(uint8_t j = 0; j < Vector_Length(RoundRobinHandle.TimerGroup[i].SoftTimerTable); j++)
		{
		    rr_softtimer_t *Timer = ((rr_softtimer_t *)Vector_GetElement(RoundRobinHandle.TimerGroup[i].SoftTimerTable,j));
			/* Check if timer is enabled */
			if (Timer->Flags.F.CNTEN)  /*!< Timer is enabled */
			{
				/* Decrease counter if needed */
				if (Timer->CNT)
					Timer->CNT--;
			}
		}
	}
#if RTE_USE_OS == 1
	RoundRobin_Run(0);
#endif
#endif
}
/*************************************************
*** Args: GroupID 定时器所属Group的ID 该参数只针对FULL模式生效
*** Function: RoundRobin运行函数，在主函数或线程中调用
*************************************************/
void RoundRobin_Run(
#if RR_TYPE == 2 
	uint8_t GroupID
#else
	void
#endif
)
{
#if RR_TYPE == 0
	// Loop through each timer in the timer table.
	for(uint8_t i = 0; i < TIMER_N; i++)
		RoundRobin_CheckTimer(i);
#elif RR_TYPE == 1
	// Loop through each task in the task table.
	for(uint8_t i = 0; i < Vector_Length(RoundRobinHandle.SoftTimerTable); i++)
		RoundRobin_CheckTimer(i);
#elif RR_TYPE == 2
	// Loop through each task in the task table.
	for(uint8_t i = 0; i < Vector_Length(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable); i++)
		RoundRobin_CheckTimer(GroupID,i);
#endif
}
/*************************************************
*** Args: NULL
*** Function: 创建一个定时器组，仅在FULL模式下可用
*************************************************/
rr_error_e RoundRobin_CreateGroup(uint8_t GroupID)
{
	if(RoundRobinHandle.TimerGroupCnt >= RR_MAX_GROUP_NUM)
		return RR_NOSPACEFORNEW;
	RTE_AssertParam(GroupID == RoundRobinHandle.TimerGroupCnt);
	RoundRobinHandle.TimerGroup[RoundRobinHandle.TimerGroupCnt].TimerGroupID = GroupID;
	Vector_Init(&RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable,MEM_RTE);
	RoundRobinHandle.TimerGroupCnt++;
	return RR_NOERR;
}
/*************************************************
*** Args:
          GroupID 定时器所属Group的ID 该参数只针对FULL模式生效
          TimerID 待添加定时器ID
          ReloadValue 重装载值
          ReloadEnable 重装载使能
          ReloadEnable 定时器运行使能
          *TimerCallback 定时器回调函数
          *UserParameters 回调函数输入参数
*** Function: 为当前RoundRobin环境的某一Group添加一个软定时器
*************************************************/
extern rr_error_e RoundRobin_CreateTimer(
#if RR_TYPE == 2
	uint8_t GroupID,
#endif
	uint8_t TimerID,
	uint32_t ReloadValue,
	uint8_t ReloadEnable,
	uint8_t RunEnable,
	void (*TimerCallback)(void *),
	void* UserParameters)
{
#if RR_TYPE == 0
	if(TimerID>TIMER_N)
		return RR_NOSPACEFORNEW;
	RoundRobinTimerHandle[TimerID].Flags.F.AREN = ReloadEnable;
	RoundRobinTimerHandle[TimerID].Flags.F.CNTEN = RunEnable;
	RoundRobinTimerHandle[TimerID].ARR = ReloadValue;
	RoundRobinTimerHandle[TimerID].CNT = ReloadValue;
	RoundRobinTimerHandle[TimerID].Callback = TimerCallback;
	RoundRobinTimerHandle[TimerID].UserParameters = UserParameters;
	return RR_NOERR;
#elif RR_TYPE == 1
	if(Vector_Length(RoundRobinHandle.SoftTimerTable) >= RR_MAX_NUM)
		return RR_NOSPACEFORNEW;
	rr_softtimer_t *v = Memory_Alloc0(MEM_RTE,sizeof(rr_softtimer_t));
	v->TimerID = TimerID;
	v->Flags.F.AREN = ReloadEnable;
	v->Flags.F.CNTEN = RunEnable;
	v->ARR = ReloadValue;
	v->CNT = ReloadValue;
	v->Callback = TimerCallback;
	v->UserParameters = UserParameters;
	Vector_Pushback(RoundRobinHandle.SoftTimerTable, v);
	return RR_NOERR;
#elif RR_TYPE == 2
	if(Vector_Length(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable) >= RR_MAX_NUM)
		return RR_NOSPACEFORNEW;
	rr_softtimer_t *v = Memory_Alloc0(MEM_RTE,sizeof(rr_softtimer_t));
	v->TimerID = TimerID;
	v->Flags.F.AREN = ReloadEnable;
	v->Flags.F.CNTEN = RunEnable;
	v->ARR = ReloadValue;
	v->CNT = ReloadValue;
	v->Callback = TimerCallback;
	v->UserParameters = UserParameters;
	Vector_Pushback(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable, v);
	return RR_NOERR;
#endif
}
/*************************************************
*** Args:
          GroupID 定时器所属Group的ID 该参数只针对FULL模式生效
          TimerID 待删除定时器ID
*** Function: 为当前RoundRobin环境删除一个软定时器
*************************************************/
rr_error_e RoundRobin_RemoveTimer(
#if RR_TYPE == 2
	uint8_t GroupID,
#endif
	uint8_t TimerID)
{
#if RR_TYPE == 0
	if(TimerID>TIMER_N)
		return RR_NOSUCHTIMER;
	memset(&RoundRobinTimerHandle[TimerID],0,sizeof(rr_softtimer_t));
	return RR_NOERR;
#elif RR_TYPE == 1
	if(TimerID>Vector_Length(RoundRobinHandle.SoftTimerTable))
		return RR_NOSUCHTIMER;
	Vector_Erase(RoundRobinHandle.SoftTimerTable, TimerID);
	return RR_NOERR;
#elif RR_TYPE == 2
	if(TimerID > Vector_Length(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable))
		return RR_NOSUCHTIMER;
	Vector_Erase(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable, TimerID);
	return RR_NOERR;
#endif
}
/*************************************************
*** Args:
          GroupID 定时器所属Group的ID 该参数只针对FULL模式生效
          TimerID 待就绪定时器ID
*** Function: 就绪当前RoundRobin环境中的一个软定时器
*************************************************/
rr_error_e RoundRobin_ReadyTimer(
#if RR_TYPE == 2
	uint8_t GroupID,
#endif
	uint8_t TimerID)
{
#if RR_TYPE == 0
	if(TimerID>TIMER_N)
		return RR_NOSUCHTIMER;
	RoundRobinTimerHandle[TimerID].CNT = 0;
	return RR_NOERR;
#elif RR_TYPE == 1
	if(TimerID>Vector_Length(RoundRobinHandle.SoftTimerTable))
		return RR_NOSUCHTIMER;
    rr_softtimer_t *Timer = ((rr_softtimer_t*)Vector_GetElement(RoundRobinHandle.SoftTimerTable,TimerID));
	RTE_AssertParam(Timer->TimerID == TimerID);
	Timer->CNT = 0;
	return RR_NOERR;
#elif RR_TYPE == 2
	if(TimerID>Vector_Length(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable))
		return RR_NOSUCHTIMER;
    rr_softtimer_t* Timer = ((rr_softtimer_t*)Vector_GetElement(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable,TimerID));
	RTE_AssertParam(Timer->TimerID == TimerID);
	Timer->CNT = 0;
	return RR_NOERR;
#endif
}
/*************************************************
*** Args:
          GroupID 定时器所属Group的ID 该参数只针对FULL模式生效
          TimerID 待复位定时器ID
*** Function: 复位当前RoundRobin环境中的一个软定时器
*************************************************/
rr_error_e RoundRobin_ResetTimer(
#if RR_TYPE == 2
	uint8_t GroupID,
#endif
	uint8_t TimerID)
{
#if RR_TYPE == 0
	if(TimerID>TIMER_N)
		return RR_NOSUCHTIMER;
	RoundRobinTimerHandle[TimerID].CNT = RoundRobinTimerHandle[TimerID].ARR;
	return RR_NOERR;
#elif RR_TYPE == 1
	if(TimerID>Vector_Length(RoundRobinHandle.SoftTimerTable))
		return RR_NOSUCHTIMER;
    rr_softtimer_t *Timer = ((rr_softtimer_t*)Vector_GetElement(RoundRobinHandle.SoftTimerTable,TimerID));
	RTE_AssertParam(Timer->TimerID == TimerID);
	Timer->CNT = Timer->ARR;
	return RR_NOERR;
#elif RR_TYPE == 2
	if(TimerID>Vector_Length(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable))
		return RR_NOSUCHTIMER;
    rr_softtimer_t *Timer = ((rr_softtimer_t*)Vector_GetElement(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable,TimerID));
	RTE_AssertParam(Timer->TimerID == TimerID);
	Timer->CNT = Timer->ARR;
	return RR_NOERR;
#endif	
}
/*************************************************
*** Args:
          GroupID 定时器所属Group的ID 该参数只针对FULL模式生效
          TimerID 待暂停定时器ID
*** Function: 暂停当前RoundRobin环境中的一个软定时器
*************************************************/
rr_error_e RoundRobin_PauseTimer(
#if RR_TYPE == 2
	uint8_t GroupID,
#endif
	uint8_t TimerID)
{
#if RR_TYPE == 0
	if(TimerID>TIMER_N)
		return RR_NOSUCHTIMER;
	RoundRobinTimerHandle[TimerID].Flags.F.CNTEN = 0;
	return RR_NOERR;
#elif RR_TYPE == 1
	if(TimerID>Vector_Length(RoundRobinHandle.SoftTimerTable))
		return RR_NOSUCHTIMER;
    rr_softtimer_t *Timer = ((rr_softtimer_t*)Vector_GetElement(RoundRobinHandle.SoftTimerTable,TimerID));
	RTE_AssertParam(Timer->TimerID == TimerID);
	Timer->Flags.F.CNTEN = 0;
	return RR_NOERR;
#elif RR_TYPE == 2
	if(TimerID>Vector_Length(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable))
		return RR_NOSUCHTIMER;
	rr_softtimer_t *Timer = ((rr_softtimer_t*)Vector_GetElement(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable,TimerID));
	RTE_AssertParam(Timer->TimerID == TimerID);
	Timer->Flags.F.CNTEN = 0;
	return RR_NOERR;
#endif
}
/*************************************************
*** Args:
          GroupID 定时器所属Group的ID 该参数只针对FULL模式生效
          TimerID 待恢复定时器ID
*** Function: 恢复当前RoundRobin环境中的一个软定时器
*************************************************/
rr_error_e RoundRobin_ResumeTimer(
#if RR_TYPE == 2
	uint8_t GroupID,
#endif
	uint8_t TimerID)
{
#if RR_TYPE == 0
	if(TimerID>TIMER_N)
		return RR_NOSUCHTIMER;
	RoundRobinTimerHandle[TimerID].Flags.F.CNTEN = 1;
	return RR_NOERR;
#elif RR_TYPE == 1
	if(TimerID>Vector_Length(RoundRobinHandle.SoftTimerTable))
		return RR_NOSUCHTIMER;
    rr_softtimer_t *Timer = ((rr_softtimer_t*)Vector_GetElement(RoundRobinHandle.SoftTimerTable,TimerID));
	RTE_AssertParam(Timer->TimerID == TimerID);
	Timer->Flags.F.CNTEN = 1;
	return RR_NOERR;
#elif RR_TYPE == 2
	if(TimerID>Vector_Length(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable))
		return RR_NOSUCHTIMER;
	rr_softtimer_t *Timer = ((rr_softtimer_t*)Vector_GetElement(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable,TimerID));
	RTE_AssertParam(Timer->TimerID == TimerID);
	Timer->Flags.F.CNTEN = 1;
	return RR_NOERR;
#endif
}
/*************************************************
*** Args:
          GroupID 定时器所属Group的ID 该参数只针对FULL模式生效
          TimerID 待判断定时器ID
*** Function: 判断当前RoundRobin环境中的一个软定时器是否正在运行
*************************************************/
bool RoundRobin_IfRunTimer(
#if RR_TYPE == 2
	uint8_t GroupID,
#endif
	uint8_t TimerID)
{
#if RR_TYPE == 0
	if(TimerID>TIMER_N)
		return RR_NOSUCHTIMER;
	return RoundRobinTimerHandle[TimerID].Flags.F.CNTEN;
#elif RR_TYPE == 1
	if(TimerID>Vector_Length(RoundRobinHandle.SoftTimerTable))
		return RR_NOSUCHTIMER;
    rr_softtimer_t *Timer = ((rr_softtimer_t*)Vector_GetElement(RoundRobinHandle.SoftTimerTable,TimerID));
	RTE_AssertParam(Timer->TimerID == TimerID);
	return Timer->Flags.F.CNTEN;
#elif RR_TYPE == 2
	if(TimerID>Vector_Length(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable))
		return RR_NOSUCHTIMER;
	rr_softtimer_t *Timer = ((rr_softtimer_t*)Vector_GetElement(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable,TimerID));
	RTE_AssertParam(Timer->TimerID == TimerID);
	return Timer->Flags.F.CNTEN;
#endif
}
/*************************************************
*** Args: Null
*** Function: 获取当前RoundRobin环境运行时间
*************************************************/
uint32_t RoundRobin_GetTick(void) 
{
	/* Return current time in milliseconds */
#if RTE_USE_OS == 1
	if (osKernelGetState () == osKernelRunning) 
		return osKernelGetTickCount();
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
#if RR_TYPE == 0
	return RoundRobinRunTick;
#elif RR_TYPE == 1||RR_TYPE == 2
	return RoundRobinHandle.RoundRobinRunTick;
#endif
#endif 
}
/*************************************************
*** Args: prev_tick a previous time stamp (return value of systick_get() )
*** Function: 获取两次tick之间时间差
*************************************************/
uint32_t RoundRobin_TickElaps(uint32_t prev_tick)
{
	uint32_t act_time = RoundRobin_GetTick();
	/*If there is no overflow in sys_time simple subtract*/
	if(act_time >= prev_tick) 
		prev_tick = act_time - prev_tick;
	else 
	{
		prev_tick = UINT32_MAX - prev_tick + 1;
		prev_tick += act_time;
	}
	return prev_tick;
}
/*************************************************
*** Args: Delay延时一段时钟基准 
*** Function: 延时一段毫秒
*************************************************/
void RoundRobin_Delay(uint32_t Delay) {
	/* Delay for amount of milliseconds */
	/* Check if we are called from ISR */
	if (__get_IPSR() == 0) 
	{
		/* Called from thread mode */
		uint32_t tickstart = RoundRobin_GetTick();
		/* Count interrupts */
		while ((RoundRobin_GetTick() - tickstart) < Delay)
		{
#if RTE_USE_OS == 0
			/* Go sleep, wait systick interrupt */
			__WFI();
#endif
		}
	}
	else 
	{
		/* Called from interrupt mode */
		while (Delay) 
		{
			/* Check if timer reached zero after we last checked COUNTFLAG bit */
			if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
				Delay--;
		}
	}
}
/*************************************************
*** Args: micros 微秒
*** Function: 延时微秒，不影响系统调度
*************************************************/
__inline void RoundRobin_DelayUS(volatile uint32_t micros) {
	uint32_t start = DWT->CYCCNT;
	/* Go to number of cycles for system */
	micros *= (SystemCoreClock / 1000000);
	/* Delay till end */
	while ((DWT->CYCCNT - start) < micros);
}
/*************************************************
*** Args: Null
*** Function: 获取当前RoundRobin环境信息
*************************************************/
void RoundRobin_Demon(void)
{
	uprintf("--------------------------------------------------\r\n");
#if RR_DEBUG == 1
#if RR_TYPE == 0
	for(uint8_t i=0;i<TIMER_N;i++)
	{
		uprintf("%10s    TIMER:%02d----AutoReload Enable:%x AutoReload Val:%6d Now Val:%6d Run Enable:%x\r\n",
		RR_STR,
		i,
		RoundRobinTimerHandle[i].Flags.F.AREN,
		RoundRobinTimerHandle[i].ARR,
		RoundRobinTimerHandle[i].CNT,
		RoundRobinTimerHandle[i].Flags.F.CNTEN);
	}
#elif RR_TYPE == 1
	uint8_t timer_n = Vector_Length(RoundRobinHandle.SoftTimerTable);
	for(uint8_t i=0;i<timer_n;i++)
	{
		rr_softtimer_t *Timer = ((rr_softtimer_t *)Vector_GetElement(RoundRobinHandle.SoftTimerTable,i));
		RTE_AssertParam(Timer->TimerID == i);
		uprintf("%10s    TIMER:%02d----AutoReload Enable:%x AutoReload Val:%6d Now Val:%6d Run Enable:%x\r\n",
		RR_STR,
		Timer->TimerID,
		Timer->Flags.F.AREN,
		Timer->ARR,
		Timer->CNT,
		Timer->Flags.F.CNTEN);
	}
#elif RR_TYPE == 2
	for(uint8_t i=0;i<RoundRobinHandle.TimerGroupCnt;i++)
	{
		uprintf("-------------------------\r\n");
		uprintf("%10s    Group:%02d Using Timer Count:%d Max Num:%d\r\n",
		RR_STR,
		RoundRobinHandle.TimerGroup[i].TimerGroupID,
		Vector_Length(RoundRobinHandle.TimerGroup[i].SoftTimerTable),
		RR_MAX_NUM);
		for(uint8_t j=0;j<Vector_Length(RoundRobinHandle.TimerGroup[i].SoftTimerTable);j++)
		{
		    rr_softtimer_t *Timer = ((rr_softtimer_t *)Vector_GetElement(RoundRobinHandle.TimerGroup[i].SoftTimerTable,j));
			uprintf("%10s    TIMER:%02d----AutoReload Enable:%x AutoReload Val:%6d Now Val:%6d Run Enable:%x\r\n",
			RR_STR,
			Timer->TimerID,
			Timer->Flags.F.AREN,
			Timer->ARR,
			Timer->CNT,
			Timer->Flags.F.CNTEN);
		}
	}
#endif
#endif
}
#endif
