#include "RTE_Include.h"
/*****************************************************************************
*** Author: Shannon
*** Version: 2.0 2018.8.11
*** History: 1.0 创建
             2.0 为RTE的升级做适配，更改模块名称
*****************************************************************************/
#if RTE_USE_STATEMACHINE == 1
/*************************************************
*** Args:   
					thisStateMachine 待处理状态机
					State 状态编号
					thisFunction 状态函数
*** Function: 为状态机的不同状态设置状态函数
*************************************************/
void StateMachine_SetFuction(StateMachine_t *thisStateMachine,uint8_t State,StateFunction thisFunction)
{
	if(thisStateMachine!=(void *)0&&thisStateMachine->FunctionListTable!=(void *)0&&State<thisStateMachine->StateNum)
		thisStateMachine->FunctionListTable[State] = thisFunction;
}
/*************************************************
*** Args:   
					thisStateMachine 待处理状态机
					InputArgs 状态机共享参数
*** Function: 运行一个状态机
*************************************************/
void StateMachine_Run(StateMachine_t *thisStateMachine,void * InputArgs)
{
	thisStateMachine->RunningState = thisStateMachine->FunctionListTable[thisStateMachine->RunningState](InputArgs);
}
/*************************************************
*** Args:   
					thisStateMachine 待处理状态机
*** Function: 删除一个状态机
*************************************************/
void StateMachine_Delete(StateMachine_t *thisStateMachine)
{
	thisStateMachine->StateNum = 0;
	thisStateMachine->FunctionListTable = (void *)0;
}
/*************************************************
*** Args:   
					thisStateMachine 待处理状态机
					StateNum 状态数目
*** Function: 初始化一个状态机
*************************************************/
void StateMachine_Init(StateMachine_t *thisStateMachine,uint8_t StateNum,StateFunction *thisFuctionList)
{
	thisStateMachine->StateNum = StateNum;
	thisStateMachine->FunctionListTable = thisFuctionList;
}
#endif
