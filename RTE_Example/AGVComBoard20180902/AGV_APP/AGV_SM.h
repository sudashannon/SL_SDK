#ifndef __AGV_SM_H
#define __AGV_SM_H
#include "RTE_Include.h"
typedef enum
{
	TASK_IDLE = 0,
	TASK_APPLY ,
	TASK_WAITEXCUTE ,
	TASK_EXCUTE ,
	TASK_WAITFINISH ,
	TASK_FINISH ,
	TASK_RELEASE ,
	TASK_WAITOVER ,
	TASK_ERROR ,
	TASK_CNT ,
}Task_Running_State_e;
typedef enum
{
	ERROR_NONE = 0,
	ERROR_NOTAGVMODE,
	ERROR_UNEXPECT_APPLY,
	ERROR_DIFFERENT_EXCUTE,
}Task_Error_e;
typedef struct
{
	Task_Error_e TaskError;
	uint8_t LastTaskDetailKey[50];
	//电梯用
	uint8_t TaskDest;
	uint8_t TaskLocation;
	//回馈用
	uint8_t MsgTypeCode[50];
	uint8_t LiftId[50];
	uint8_t TaskDetailKey[50];
	
	uint8_t OldsTaskDetailKey[50];
	uint8_t OldLiftId[50];
	
	uint8_t Error[10];
	uint8_t ErrMsg[50];
	//客户端用
	uint8_t ServerIP[4];
	uint16_t ServerPortNum;
  StateMachine_t AGVStateMachine;
}AGV_SM_Arg_t;
void AGV_SM_Init(void);
extern AGV_SM_Arg_t AGV_RunningTask;
#endif
