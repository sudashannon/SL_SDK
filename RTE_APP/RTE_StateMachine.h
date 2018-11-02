#ifndef __RTE_STATEMACHINE_H
#define __RTE_STATEMACHINE_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Include.h"
#if RTE_USE_SM
typedef enum
{
	SM_NOERR = 0,
	SM_HANDLEUNINIT = 1,
	SM_NOSPACEFORNEW = 2,
	SM_ALREADYEXIST = 3,
	SM_NOSUCHSM = 4,
}RTE_SM_Err_e;
typedef struct
{
	uint8_t StateName;
	uint8_t (*StateFunction)(void *);
}RTE_State_t;
typedef vec_t(RTE_State_t) sm_vec_t;
typedef struct {
	uint8_t RunningState;
	sm_vec_t SMTable;
}RTE_StateMachine_t;
extern void StateMachine_Init(RTE_StateMachine_t *thisStateMachine);
extern RTE_SM_Err_e StateMachine_Add(RTE_StateMachine_t *thisStateMachine,uint8_t State, uint8_t(*StateFunction)(void *));
extern void StateMachine_Run(RTE_StateMachine_t *thisStateMachine,void * InputArgs);
extern RTE_SM_Err_e StateMachine_Remove(RTE_StateMachine_t *thisStateMachine,uint8_t State);
#endif
#ifdef __cplusplus
}
#endif
#endif
