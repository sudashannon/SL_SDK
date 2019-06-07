#ifndef __RTE_STATEMACHINE_H
#define __RTE_STATEMACHINE_H
#ifdef __cplusplus
extern "C" {
#endif
	#include "RTE_Config.h"
	#if RTE_USE_SM == 1
	#include <string.h>
	#include "RTE_Vector.h"
	#if RTE_USE_OS == 1
	#include "cmsis_os2.h"
	#endif
	typedef enum
	{
		SM_NOERR = 0,
		SM_NOSPACEFORNEW = 1,
		SM_ALREADYEXIST = 2,
		SM_NOSUCHSM = 3,
	}sm_error_e;
	typedef struct
	{
		uint8_t StateName;
		uint8_t (*StateFunction)(void *);
	}sm_state_t;
	typedef struct {
		uint8_t RunningState;
		vector_t *SMTable;
	#if RTE_USE_OS == 1
		osMutexId_t mutexid;
	#endif
	}sm_t;
	extern void StateMachine_Init(sm_t *thisStateMachine,uint8_t InitialState);
	extern sm_error_e StateMachine_Add(sm_t *thisStateMachine,uint8_t State, uint8_t(*StateFunction)(void *));
	extern void StateMachine_Run(sm_t *thisStateMachine,void *InputArgs);
	extern sm_error_e StateMachine_Remove(sm_t *thisStateMachine,uint8_t State);
	extern void StateMachine_ChangeTo(sm_t *thisStateMachine,uint8_t NewStateNum);
	#endif
#ifdef __cplusplus
}
#endif
#endif
