#include "AGV_SM.h"
#include "WebService_Parser.h"
#include "WebService_Send.h"
#include "CAN_STATE.h"
#include "CAN_SEND.h"
#include "TCP_Server.h"
#include "TCP_Client.h"
//#define DEBUG_WEBSERVICE == 1
AGV_SM_Arg_t AGV_RunningTask = {0};
static uint8_t AGV_SendBuffer[2048] = {0};
static State AGV_IDLE_Function(void *Args);
static State AGV_APPLY_Function(void *Args);
static State AGV_WAITEXCUTE_Function(void *Args);
static State AGV_EXCUTE_Function(void *Args);
static State AGV_WAITFINISH_Function(void *Args);
static State AGV_FINISH_Function(void *Args);
static State AGV_RELEASE_Function(void *Args);
static State AGV_WAITOVER_Function(void *Args);
static State AGV_ERROR_Function(void *Args);
static StateFunction AGV_FuctionTable[TASK_CNT] = {0};
static void Client_Callback(void* UserParameters) {
	AGV_SM_Arg_t *RunArgs = (AGV_SM_Arg_t *)UserParameters;	
	TCPClientConnect(RunArgs->ServerIP,RunArgs->ServerPortNum);
	printf("[AGV]    告知AGV服务器：%d.%d.%d.%d:%d 任务完成！\n"
	,RunArgs->ServerIP[0],RunArgs->ServerIP[1],RunArgs->ServerIP[2],RunArgs->ServerIP[3],RunArgs->ServerPortNum);
	if(TCPClientConnect(RunArgs->ServerIP,RunArgs->ServerPortNum) != false)
	{
		WebService_ErrorGenerate(0,RunArgs);
		WebService_ClientDataGenerate(FinishTask,AGV_SendBuffer,RunArgs);
		TCPClientSendData(AGV_SendBuffer,strlen((char *)AGV_SendBuffer));
	}
	else
	{
		printf("[AGV]    告知AGV服务器失败！\n");
	}
}
void AGV_SM_Init(void)
{
	StateMachine_Init(&AGV_RunningTask.AGVStateMachine,TASK_CNT,AGV_FuctionTable);
	AGV_RunningTask.AGVStateMachine.RunningState = TASK_IDLE;
	StateMachine_SetFuction(&AGV_RunningTask.AGVStateMachine,TASK_IDLE,AGV_IDLE_Function);
	StateMachine_SetFuction(&AGV_RunningTask.AGVStateMachine,TASK_APPLY,AGV_APPLY_Function);
	StateMachine_SetFuction(&AGV_RunningTask.AGVStateMachine,TASK_WAITEXCUTE,AGV_WAITEXCUTE_Function);
	StateMachine_SetFuction(&AGV_RunningTask.AGVStateMachine,TASK_EXCUTE,AGV_EXCUTE_Function);
	StateMachine_SetFuction(&AGV_RunningTask.AGVStateMachine,TASK_WAITFINISH,AGV_WAITFINISH_Function);
	StateMachine_SetFuction(&AGV_RunningTask.AGVStateMachine,TASK_FINISH,AGV_FINISH_Function);
	StateMachine_SetFuction(&AGV_RunningTask.AGVStateMachine,TASK_RELEASE,AGV_RELEASE_Function);
	StateMachine_SetFuction(&AGV_RunningTask.AGVStateMachine,TASK_WAITOVER,AGV_WAITOVER_Function);
	StateMachine_SetFuction(&AGV_RunningTask.AGVStateMachine,TASK_ERROR,AGV_ERROR_Function);
	RTE_RoundRobin_CreateTimer("ClientTimer",1000,1,0,Client_Callback,&AGV_RunningTask);
}
static State AGV_IDLE_Function(void *Args)
{
	AGV_SM_Arg_t *RunArgs = (AGV_SM_Arg_t *)Args;	
	switch(ParserName)
	{
		case Nofuncion:
		{
			
		}break;
		case ApplyResource:
		{
			if(RunArgs->TaskDetailKey[0] == 0)
			{
				//拷贝参数
				memcpy(RunArgs->LastTaskDetailKey,ApplyTempInfor.TaskDetailKey,50);
				memcpy(RunArgs->TaskDetailKey,ApplyTempInfor.TaskDetailKey,50);
				memcpy(RunArgs->LiftId,ApplyTempInfor.LiftId,50);
				memcpy(RunArgs->MsgTypeCode,ApplyTempInfor.MsgTypeCode,50);
				RunArgs->TaskDest = atoi((char *)ApplyTempInfor.Destination);
				RunArgs->TaskLocation = atoi((char *)ApplyTempInfor.InstantLocation);
				//保存ID
				printf("[AGV]    有新的任务申请!\n");
				ef_set_env("taskid", (char *)RunArgs->TaskDetailKey);
				ef_save_env();
				//返回APPLY
				ParserName = Nofuncion;
				return TASK_APPLY;
			}
			else
			{
				if(!memcmp(RunArgs->TaskDetailKey,ApplyTempInfor.TaskDetailKey,50))
				{
					memcpy(RunArgs->MsgTypeCode,ApplyTempInfor.MsgTypeCode,50);
					printf("[AGV]    已有相同任务在申请！\n");
					printf("[ELE]    当前电梯楼层：%d\n",EleStatus.CurrentFloor);
					printf("[ELE]    AGV所在楼层：%d\n",atoi((char *)ApplyTempInfor.InstantLocation));
					if(EleStatus.CurrentFloor == atoi((char *)ApplyTempInfor.InstantLocation))
					{
						printf("[AGV]    申请已完成！\n");
						WebService_ErrorGenerate(0,RunArgs);
						WebService_ServerDataGenerate(ApplyResource,AGV_SendBuffer,RunArgs);
						TCPServerSendData(AGV_SendBuffer,strlen((char *)AGV_SendBuffer));
					}
					ParserName = Nofuncion;
					memset(RunArgs->MsgTypeCode,0,50);
					return TASK_IDLE;
				}
				else
				{
					printf("[AGV]    电梯已被占用，忙碌中！\n");
					ParserName = Nofuncion;
					return TASK_IDLE;
				}
			}
		};
		case ApplyForExecution:
		{
			if(!memcmp(RunArgs->TaskDetailKey,ExcuteTempInfor.TaskDetailKey,50))
			{
				printf("[AGV]    有任务请求执行!\n");
				printf("[ELE]    当前电梯楼层：%d\n",EleStatus.CurrentFloor);
				printf("[ELE]    AGV目标楼层：%d\n",atoi((char *)ExcuteTempInfor.Destination));
				memcpy(RunArgs->MsgTypeCode,ExcuteTempInfor.MsgTypeCode,50);
				memcpy(RunArgs->OldLiftId,ExcuteTempInfor.OldLiftId,50);
				RunArgs->ServerPortNum = ExcuteTempInfor.AGVServerPortNum;
				memcpy(RunArgs->ServerIP,ExcuteTempInfor.AGVServerIP,4);
				if(EleStatus.CurrentFloor == atoi((char *)ExcuteTempInfor.Destination))
				{
					WebService_ErrorGenerate(0,RunArgs);
					WebService_ServerDataGenerate(ApplyForExecution,AGV_SendBuffer,RunArgs);
					TCPServerSendData(AGV_SendBuffer,strlen((char *)AGV_SendBuffer));
					printf("[AGV]    执行已完成！\n");
					EleStatus.DoorReady = true;
					memset(RunArgs->MsgTypeCode,0,50);
					memset(RunArgs->OldLiftId,0,50);
					ParserName = Nofuncion;
					return TASK_FINISH;
				}
				else
				{
					if(atoi((char *)ExcuteTempInfor.InstantLocation) == RunArgs->TaskLocation&&
						atoi((char *)ExcuteTempInfor.Destination) == RunArgs->TaskDest)
					{
						printf("[AGV]    信息一致，开始执行!\n");
						ParserName = Nofuncion;
						return TASK_EXCUTE;
					}
					else if(RunArgs->TaskLocation == 0&&RunArgs->TaskDest == 0)
					{
						memcpy(RunArgs->LiftId,ExcuteTempInfor.OldLiftId,50);
						RunArgs->TaskDest = atoi((char *)ExcuteTempInfor.Destination);
						RunArgs->TaskLocation = atoi((char *)ExcuteTempInfor.InstantLocation);
						ParserName = Nofuncion;
						return TASK_EXCUTE;
					}
					else
					{
						RunArgs->TaskError = ERROR_DIFFERENT_EXCUTE;
						ParserName = Nofuncion;
						return TASK_ERROR;
					}
				}
			}
			else
			{
				printf("[AGV]    请求执行的任务与当前任务ID不一致！\n");
				if(RunArgs->TaskDetailKey[0] == 0)
					printf("[AGV]    当前无任务！\n");
				ParserName = Nofuncion;
				return TASK_IDLE;
			}
		};
		case ReleaseResource:
		{
			if(!memcmp(RunArgs->TaskDetailKey,ReleaseTempInfor.TaskDetailKey,50))
			{
				printf("[AGV]    有任务请求释放!\n");
				memcpy(RunArgs->MsgTypeCode,ReleaseTempInfor.MsgTypeCode,50);
				
				ParserName = Nofuncion;
				return TASK_RELEASE;
			}
			else
			{
				printf("[AGV]    请求释放的任务与当前任务ID不一致！\n");
				if(RunArgs->TaskDetailKey[0] == 0)
				{
					printf("[AGV]    当前无任务！\n");
					if(!memcmp(RunArgs->LastTaskDetailKey,ReleaseTempInfor.TaskDetailKey,50))
					{
						printf("[AGV]    是上次的任务！\n");
						memcpy(RunArgs->MsgTypeCode,ReleaseTempInfor.MsgTypeCode,50);
						WebService_ErrorGenerate(0,RunArgs);
						WebService_ServerDataGenerate(ReleaseResource,AGV_SendBuffer,RunArgs);
						TCPServerSendData(AGV_SendBuffer,strlen((char *)AGV_SendBuffer));
						memset(RunArgs->MsgTypeCode,0,50);
					}
				}
				ParserName = Nofuncion;
			}
		}break;
	}
	return RunArgs->AGVStateMachine.RunningState;	
}
static State AGV_APPLY_Function(void *Args)
{
	AGV_SM_Arg_t *RunArgs = (AGV_SM_Arg_t *)Args;	
#ifdef DEBUG_WEBSERVICE
	EleStatus.RunStatus = WAITGET;
	return TASK_WAITEXCUTE;
#else
	if(EleStatus.AGVModeFlag==true)
	{
		switch(EleStatus.RunStatus)
		{
			case SERVICEOVER:
			{
				printf("[AGV]    符合条件，开始申请！\n");
				EleStatus.DoorReady = false;
				EleStatus.ApplyStatus = false;
				EleStatus.ExcuteStatus = false;
				EleStatus.FinishStatus = false;
				EleStatus.ReleaseStatus = false;
				EleStatus.DoorReady = false;
				EleStatus.GetToDestination = false;
				EleStatus.DoorOpenAction = false;
				EleStatus.DoorClosedAction = false;
				EleStatus.UpAction = false;
				EleStatus.DownAction = false;
				if(CANDataSend(AGV_APPLY_CMD,RunArgs->TaskDest,RunArgs->TaskLocation) == true)
				{
					EleStatus.RunStatus = WAITGET;
					return TASK_WAITEXCUTE;
				}
				else
				{
					WebService_ErrorGenerate(7,RunArgs);
					WebService_ServerDataGenerate(ApplyResource,AGV_SendBuffer,RunArgs);
					TCPServerSendData(AGV_SendBuffer,strlen((char *)AGV_SendBuffer));
					memset(RunArgs->MsgTypeCode,0,50);
					memset(RunArgs->TaskDetailKey,0,50);
					memset(RunArgs->LastTaskDetailKey,0,50);
					memset(RunArgs->LiftId,0,50);
					RunArgs->TaskDest = 0;
					RunArgs->TaskLocation = 0;
					return TASK_IDLE;
				}
			}
			default:
			{
				printf("[ELE]    当前电梯运行模式：%x\n",EleStatus.RunStatus);
				EleStatus.RunStatus = WAITGET;
				printf("[ELE]    等待下一次申请\n");
				memset(RunArgs->MsgTypeCode,0,50);
				memset(RunArgs->TaskDetailKey,0,50);
				memset(RunArgs->LastTaskDetailKey,0,50);
				memset(RunArgs->LiftId,0,50);
				RunArgs->TaskDest = 0;
				RunArgs->TaskLocation = 0;
				return TASK_IDLE;
			};
		}
	}
	else
	{
		printf("[AGV]    申请时不是AGV模式！\n");
		memset(RunArgs->MsgTypeCode,0,50);
		memset(RunArgs->TaskDetailKey,0,50);
		memset(RunArgs->LastTaskDetailKey,0,50);
		memset(RunArgs->LiftId,0,50);
		RunArgs->TaskDest = 0;
		RunArgs->TaskLocation = 0;
		return TASK_IDLE;
	}
#endif
	return RunArgs->AGVStateMachine.RunningState;	
}
static State AGV_WAITEXCUTE_Function(void *Args)
{
	AGV_SM_Arg_t *RunArgs = (AGV_SM_Arg_t *)Args;
#ifdef DEBUG_WEBSERVICE	
	WebService_ServerDataGenerate(ApplyResource,AGV_SendBuffer,RunArgs);
	TCPServerSendData(AGV_SendBuffer,strlen((char *)AGV_SendBuffer));
	return TASK_IDLE;	
#else
	if(EleStatus.DownAction == true)
	{
		EleStatus.DownAction = false;
	}
	if(EleStatus.UpAction == true)
	{
		EleStatus.UpAction = false;
	}
	if(EleStatus.GetToDestination == true)
	{
		printf("[AGV]    电梯已到达AGV所在楼层！\n");
		EleStatus.GetToDestination = false;
	}
	if(EleStatus.DoorOpenAction == true)
	{
		printf("[AGV]    任务申请成功，电梯在AGV所在楼层开门！\n");
		EleStatus.DoorOpenAction = false;
	}
	if(EleStatus.DoorReady == true)
	{
		printf("[AGV]    任务申请完毕，AGV所在楼层门已经打开！\n");
		EleStatus.DoorReady = false;
		osDelay(1000);
		WebService_ErrorGenerate(0,RunArgs);
		WebService_ServerDataGenerate(ApplyResource,AGV_SendBuffer,RunArgs);
		TCPServerSendData(AGV_SendBuffer,strlen((char *)AGV_SendBuffer));
		memset(RunArgs->MsgTypeCode,0,50);
		return TASK_IDLE;	
	}
#endif
	return RunArgs->AGVStateMachine.RunningState;	
}
static State AGV_EXCUTE_Function(void *Args)
{
	AGV_SM_Arg_t *RunArgs = (AGV_SM_Arg_t *)Args;	
	if(EleStatus.AGVModeFlag==true)
	{
		printf("[AGV]    符合条件，开始执行！\n");
		EleStatus.ApplyStatus = false;
		EleStatus.ExcuteStatus = false;
		EleStatus.FinishStatus = false;
		EleStatus.ReleaseStatus = false;
		EleStatus.DoorReady = false;
		EleStatus.GetToDestination = false;
		EleStatus.DoorOpenAction = false;
		EleStatus.DoorClosedAction = false;
		EleStatus.UpAction = false;
		EleStatus.DownAction = false;
		if(CANDataSend(AGV_EXCUTE_CMD,RunArgs->TaskDest,RunArgs->TaskLocation) == true)
		{
			WebService_ErrorGenerate(0,RunArgs);
			WebService_ServerDataGenerate(ApplyForExecution,AGV_SendBuffer,RunArgs);
			TCPServerSendData(AGV_SendBuffer,strlen((char *)AGV_SendBuffer));
			EleStatus.RunStatus = WAITGET;
			memset(RunArgs->MsgTypeCode,0,50);
			memset(RunArgs->OldLiftId,0,50);
			printf("[AGV]    第一次连接服务器！\n");
			TCPClientConnect(RunArgs->ServerIP,RunArgs->ServerPortNum);
			return TASK_WAITFINISH;
		}
		else
		{
			WebService_ErrorGenerate(8,RunArgs);
			WebService_ServerDataGenerate(ApplyForExecution,AGV_SendBuffer,RunArgs);
			TCPServerSendData(AGV_SendBuffer,strlen((char *)AGV_SendBuffer));
			memset(RunArgs->MsgTypeCode,0,50);
			memset(RunArgs->OldLiftId,0,50);
			RunArgs->ServerPortNum = 0;
			memset(RunArgs->ServerIP,0,4);
			return TASK_IDLE;
		}
	}
	else
	{
		printf("[AGV]    执行时不是AGV模式！\n");
		memset(RunArgs->MsgTypeCode,0,50);
		memset(RunArgs->OldLiftId,0,50);
		RunArgs->ServerPortNum = 0;
		memset(RunArgs->ServerIP,0,4);
		return TASK_IDLE;
	}
	return RunArgs->AGVStateMachine.RunningState;	
}
static State AGV_WAITFINISH_Function(void *Args)
{
	AGV_SM_Arg_t *RunArgs = (AGV_SM_Arg_t *)Args;	
	if(EleStatus.DownAction == true)
	{
		EleStatus.DownAction = false;
	}
	if(EleStatus.UpAction == true)
	{
		EleStatus.UpAction = false;
	}
	if(EleStatus.GetToDestination == true)
	{
		printf("[AGV]    电梯已到达目的楼层！\n");
		EleStatus.GetToDestination = false;
	}
	if(EleStatus.DoorOpenAction == true)
	{
		printf("[AGV]    电梯在目的楼层开门！\n");
		EleStatus.DoorOpenAction = false;
	}
	if(EleStatus.DoorClosedAction == true)
	{
		printf("[AGV]    电梯关门，开始执行任务！\n");
		EleStatus.DoorClosedAction = false;
	}
	if(EleStatus.DoorReady == true)
	{
		printf("[AGV]    电梯门状态有变！\n");
		EleStatus.DoorReady = false;
	}
	if(EleStatus.FinishStatus == true)
	{
		printf("[AGV]    任务上下行结束 电梯已到达目的楼层 准备告知AGV！\n");
		EleStatus.FinishStatus = false;
		return TASK_FINISH;
	}
	return RunArgs->AGVStateMachine.RunningState;	
}
static State AGV_FINISH_Function(void *Args)
{
	AGV_SM_Arg_t *RunArgs = (AGV_SM_Arg_t *)Args;	
	if(EleStatus.DoorReady == true)
	{
		printf("[AGV]    电梯门打开完毕，等待告知AGV服务器！\n");
		if(RTE_RoundRobin_GetTimerState("ClientTimer")==0)
		{
			RTE_RoundRobin_ResumeTimer("ClientTimer");
		}
		EleStatus.DoorReady = false;
	}
	return RunArgs->AGVStateMachine.RunningState;	
}
static State AGV_RELEASE_Function(void *Args)
{
	AGV_SM_Arg_t *RunArgs = (AGV_SM_Arg_t *)Args;	
	if(EleStatus.AGVModeFlag==true)
	{
		printf("[AGV]    符合条件，开始释放！\n");
		EleStatus.ApplyStatus = false;
		EleStatus.ExcuteStatus = false;
		EleStatus.FinishStatus = false;
		EleStatus.ReleaseStatus = false;
		EleStatus.DoorReady = false;
		EleStatus.GetToDestination = false;
		EleStatus.DoorOpenAction = false;
		EleStatus.DoorClosedAction = false;
		EleStatus.UpAction = false;
		EleStatus.DownAction = false;
		if(CANDataSend(AGV_RELEASE_CMD,RunArgs->TaskDest,RunArgs->TaskLocation) == true)
		{
			EleStatus.RunStatus = WAITGET;
			return TASK_WAITOVER;
		}
		else
		{
			WebService_ErrorGenerate(9,RunArgs);
			WebService_ServerDataGenerate(ApplyForExecution,AGV_SendBuffer,RunArgs);
			TCPServerSendData(AGV_SendBuffer,strlen((char *)AGV_SendBuffer));
			memset(RunArgs->MsgTypeCode,0,50);
			return TASK_IDLE;
		}
	}
	else
	{
		printf("[AGV]    释放时不是AGV模式！\n");
		memset(RunArgs->MsgTypeCode,0,50);
		return TASK_IDLE;
	}
	return RunArgs->AGVStateMachine.RunningState;	
}
static State AGV_WAITOVER_Function(void *Args)
{
	AGV_SM_Arg_t *RunArgs = (AGV_SM_Arg_t *)Args;	
	if(EleStatus.DoorClosedAction == true)
	{
		printf("[AGV]    释放关门！\n");
		EleStatus.DoorClosedAction = false;
	}
	if(EleStatus.DoorReady == true)
	{
		printf("[AGV]    电梯门状态有变！\n");
		EleStatus.DoorReady = false;
		printf("[AGV]    任务大流程结束！\n");
		osDelay(1000);
		WebService_ErrorGenerate(0,RunArgs);
		WebService_ServerDataGenerate(ReleaseResource,AGV_SendBuffer,RunArgs);
		TCPServerSendData(AGV_SendBuffer,strlen((char *)AGV_SendBuffer));
		RunArgs->TaskDest = 0;
		RunArgs->TaskLocation = 0;
		memset(RunArgs->TaskDetailKey,0,50);
		memset(RunArgs->LiftId,0,50);
		memset(RunArgs->MsgTypeCode,0,50);
		memset(RunArgs->OldLiftId,0,50);
		RunArgs->ServerPortNum = 0;
		memset(RunArgs->ServerIP,0,4);
		ef_set_env("taskid", "0");
		ef_save_env();
		return TASK_IDLE;
	}
	return RunArgs->AGVStateMachine.RunningState;	
}
static State AGV_ERROR_Function(void *Args)
{
	AGV_SM_Arg_t *RunArgs = (AGV_SM_Arg_t *)Args;	
	switch(RunArgs->TaskError)
	{
		case ERROR_UNEXPECT_APPLY:
		{
			printf("[ERROR]    未被期望的申请！\n");
		}break;
		case ERROR_DIFFERENT_EXCUTE:
		{
			printf("[ERROR]    收到的执行任务信息与申请不一致！\n");
		}break;
		default:
			break;
	}
	return RunArgs->AGVStateMachine.RunningState;	
}
