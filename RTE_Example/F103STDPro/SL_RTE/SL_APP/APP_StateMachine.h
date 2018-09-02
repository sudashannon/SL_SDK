#ifndef __APP_STATEMACHINE_H
#define __APP_STATEMACHINE_H
#include "APP_Include.h"
typedef uint8_t State;
typedef State(*StateFunction)(void *);
typedef struct {
	uint8_t StateNum;
	uint8_t RunningState;
	StateFunction *FunctionListTable;
}StateMachine_t;
void StateMachine_Init(StateMachine_t *thisStateMachine,uint8_t StateNum);
void StateMachine_SetFuction(StateMachine_t *thisStateMachine,uint8_t State,StateFunction thisFunction);
void StateMachine_Run(StateMachine_t *thisStateMachine,void * InputArgs);
void StateMachine_Delete(StateMachine_t *thisStateMachine);
#endif
