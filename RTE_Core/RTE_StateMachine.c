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
sm_error_e StateMachine_Add(sm_t *thisStateMachine,uint8_t State, uint8_t(*StateFunction)(void *))
{
	for(uint8_t i = 0;i<Vector_Length(thisStateMachine->SMTable);i++)
	{
		if(((sm_state_t *)(Vector_GetElement(thisStateMachine->SMTable,i)))->StateName == State)
			return SM_ALREADYEXIST;
	}
	sm_state_t *v = Memory_Alloc0(MEM_RTE,sizeof(sm_state_t));
	v->StateName = State;
	v->StateFunction = StateFunction;
	Vector_Pushback(thisStateMachine->SMTable,v);
	return SM_NOERR;
}
/*************************************************
*** Args:
					thisStateMachine 待处理状态机
					InputArgs 状态机共享参数
*** Function: 运行一个状态机
*************************************************/
void StateMachine_Run(sm_t *thisStateMachine,void *InputArgs)
{
#if RTE_USE_OS == 1
	osMutexAcquire(thisStateMachine->mutexid,osWaitForever);
#endif
	thisStateMachine->RunningState = ((sm_state_t *)(Vector_GetElement(thisStateMachine->SMTable,thisStateMachine->RunningState)))->StateFunction(InputArgs);
#if RTE_USE_OS == 1
	osMutexRelease(thisStateMachine->mutexid);
#endif
}
/*************************************************
*** Args:
					thisStateMachine 待处理状态机
*** Function: 删除一个状态机
*************************************************/
sm_error_e StateMachine_Remove(sm_t *thisStateMachine,uint8_t State)
{
	int8_t idx = -1;
	for(uint8_t i = 0;i<Vector_Length(thisStateMachine->SMTable);i++)
	{
		if(((sm_state_t *)(Vector_GetElement(thisStateMachine->SMTable,i)))->StateName == State)
		{
			idx = i;
			break;
		}
	}
	if(idx!=-1)
	{
		Vector_Erase(thisStateMachine->SMTable, idx);
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
void StateMachine_ChangeTo(sm_t *thisStateMachine,uint8_t NewStateNum)
{
#if RTE_USE_OS == 1
	osMutexAcquire(thisStateMachine->mutexid,osWaitForever);
#endif
	thisStateMachine->RunningState = NewStateNum;
#if RTE_USE_OS == 1
	osMutexRelease(thisStateMachine->mutexid);
#endif
}
/*************************************************
*** Args:
					thisStateMachine 待处理状态机
					StateNum 状态数目
*** Function: 初始化一个状态机
*************************************************/
void StateMachine_Init(sm_t *thisStateMachine,uint8_t InitialState)
{
	Vector_Init(&thisStateMachine->SMTable,MEM_RTE);
	thisStateMachine->RunningState = InitialState;
#if RTE_USE_OS == 1
	thisStateMachine->mutexid = osMutexNew(NULL);
#endif
}
#endif
