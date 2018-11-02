#include "RTE_StateMachine.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.2 2018.9.30
*** History: 1.0 创建
             2.0 为RTE的升级做适配，更改模块名称
						 2.1 增加了操作系统环境下的互斥锁（已删除）
						 2.2 引入RTE_Vec进行统一管理
*****************************************************************************/
#if RTE_USE_SM == 1
/*************************************************
*** Args:   
					thisStateMachine 待处理状态机
					State 状态编号
					thisFunction 状态函数
*** Function: 为状态机的不同状态设置状态函数
*************************************************/
RTE_SM_Err_e StateMachine_Add(RTE_StateMachine_t *thisStateMachine,uint8_t State, uint8_t(*StateFunction)(void *))
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
void StateMachine_Run(RTE_StateMachine_t *thisStateMachine,void * InputArgs)
{
	thisStateMachine->RunningState = thisStateMachine->SMTable.data[thisStateMachine->RunningState].StateFunction(InputArgs);
}
/*************************************************
*** Args:   
					thisStateMachine 待处理状态机
*** Function: 删除一个状态机
*************************************************/
RTE_SM_Err_e StateMachine_Remove(RTE_StateMachine_t *thisStateMachine,uint8_t State)
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
					StateNum 状态数目
*** Function: 初始化一个状态机
*************************************************/
void StateMachine_Init(RTE_StateMachine_t *thisStateMachine)
{
	vec_init(&thisStateMachine->SMTable);
}
#endif
