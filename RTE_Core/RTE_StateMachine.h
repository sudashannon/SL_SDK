#ifndef __RTE_STATEMACHINE_H
#define __RTE_STATEMACHINE_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Config.h"
#if RTE_USE_SM
#include <string.h>
#include "RTE_Vec.h"
#if RTE_USE_OS
#include "cmsis_os2.h"
#endif
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
#if RTE_USE_OS
    osMutexId_t mutexid;
#endif
}RTE_StateMachine_t;
extern void RTE_StateMachine_Init(RTE_StateMachine_t *thisStateMachine,uint8_t InitialState);
extern RTE_SM_Err_e RTE_StateMachine_Add(RTE_StateMachine_t *thisStateMachine,uint8_t State, uint8_t(*StateFunction)(void *));
extern void RTE_StateMachine_Run(RTE_StateMachine_t *thisStateMachine,void *InputArgs);
extern RTE_SM_Err_e RTE_StateMachine_Remove(RTE_StateMachine_t *thisStateMachine,uint8_t State);
extern void RTE_StateMachine_ChangeTo(RTE_StateMachine_t *thisStateMachine,uint8_t NewStateNum);
#endif
#ifdef __cplusplus
}
#endif
#endif
