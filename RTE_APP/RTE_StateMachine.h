#ifndef __RTE_STATEMACHINE_H
#define __RTE_STATEMACHINE_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Include.h"
typedef uint8_t State;
typedef State(*StateFunction)(void *);
typedef struct {
	uint8_t StateNum;
	uint8_t RunningState;
	StateFunction *FunctionListTable;
}StateMachine_t;
extern void StateMachine_Init(StateMachine_t *thisStateMachine,uint8_t StateNum);
extern void StateMachine_SetFuction(StateMachine_t *thisStateMachine,uint8_t State,StateFunction thisFunction);
extern void StateMachine_Run(StateMachine_t *thisStateMachine,void * InputArgs);
extern void StateMachine_Delete(StateMachine_t *thisStateMachine);
#ifdef __cplusplus
}
#endif
#endif
