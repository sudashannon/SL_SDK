#include "APP_StateMachine.h"
void StateMachine_SetFuction(StateMachine_t *thisStateMachine,uint8_t State,StateFunction thisFunction)
{
	if(thisStateMachine!=(void *)0&&thisStateMachine->FunctionListTable!=(void *)0&&State<thisStateMachine->StateNum)
		thisStateMachine->FunctionListTable[State] = thisFunction;
}
void StateMachine_Run(StateMachine_t *thisStateMachine,void * InputArgs)
{
	thisStateMachine->RunningState = thisStateMachine->FunctionListTable[thisStateMachine->RunningState](InputArgs);
}
void StateMachine_Delete(StateMachine_t *thisStateMachine)
{
	thisStateMachine->StateNum = 0;
	APP_BRel(MEM_0,thisStateMachine->FunctionListTable);
}
void StateMachine_Init(StateMachine_t *thisStateMachine,uint8_t StateNum)
{
	thisStateMachine->StateNum = StateNum;
	thisStateMachine->FunctionListTable = (StateFunction*)APP_BGetz(MEM_0,StateNum*sizeof(StateFunction));
}
