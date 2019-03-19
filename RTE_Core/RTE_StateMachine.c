#include "RTE_StateMachine.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.3 2019.2.28
*** History: 1.0 创建
             2.0 为RTE的升级做适配，更改模块名称
             2.1 增加了操作系统环境下的互斥锁（已删除）
             2.2 引入RTE_Vec进行统一管理
             2.3 回复互斥锁
*****************************************************************************/
#if RTE_USE_SM == 1
/*************************************************
*** Args:
					thisStateMachine 待处理状态机
					State 状态编号
					thisFunction 状态函数
*** Function: 为状态机的不同状态设置状态函数
*************************************************/
RTE_SM_Err_e RTE_StateMachine_Add(RTE_StateMachine_t *thisStateMachine,uint8_t State, uint8_t(*StateFunction)(void *))
{
	for(uint8_t i = 0;i<thisStateMachine->SMTable.length;i++)
	{
		if(thisStateMachine->SMTable.data[i].StateName == State)
			return SM_ALREADYEXIST;
	}
	RTE_State_t v;
	v.StateName = State;
	v.StateFunction = StateFunction;
	vec_push(&thisStateMachine->SMTable,v);
	return SM_NOERR;
}
/*************************************************
*** Args:
					thisStateMachine 待处理状态机
					InputArgs 状态机共享参数
*** Function: 运行一个状态机
*************************************************/
void RTE_StateMachine_Run(RTE_StateMachine_t *thisStateMachine,void *InputArgs)
{
#if RTE_USE_OS
	osMutexAcquire(thisStateMachine->mutexid,osWaitForever);
#endif
	thisStateMachine->RunningState = thisStateMachine->SMTable.data[thisStateMachine->RunningState].StateFunction(InputArgs);
#if RTE_USE_OS
	osMutexRelease(thisStateMachine->mutexid);
#endif
}
/*************************************************
*** Args:
					thisStateMachine 待处理状态机
*** Function: 删除一个状态机
*************************************************/
RTE_SM_Err_e RTE_StateMachine_Remove(RTE_StateMachine_t *thisStateMachine,uint8_t State)
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<thisStateMachine->SMTable.length;i++)
	{
		if(thisStateMachine->SMTable.data[i].StateName == State)
		{
			idx = i;
			break;
		}
	}
	if(idx!=-1)
	{
		vec_splice(&thisStateMachine->SMTable, idx, 1);
		return SM_NOERR;
	}
	return SM_NOSUCHSM;
}
/*************************************************
*** Args:
					thisStateMachine 待处理状态机
					NewStateNum 状态数目
*** Function: 初始化一个状态机
*************************************************/
void RTE_StateMachine_ChangeTo(RTE_StateMachine_t *thisStateMachine,uint8_t NewStateNum)
{
#if RTE_USE_OS
	osMutexAcquire(thisStateMachine->mutexid,osWaitForever);
#endif
	thisStateMachine->RunningState = NewStateNum;
#if RTE_USE_OS
	osMutexRelease(thisStateMachine->mutexid);
#endif
}
/*************************************************
*** Args:
					thisStateMachine 待处理状态机
					StateNum 状态数目
*** Function: 初始化一个状态机
*************************************************/
void RTE_StateMachine_Init(RTE_StateMachine_t *thisStateMachine,uint8_t InitialState)
{
	vec_init(&thisStateMachine->SMTable);
	thisStateMachine->RunningState = InitialState;
#if RTE_USE_OS
	thisStateMachine->mutexid = osMutexNew(NULL);
#endif
}
#endif
