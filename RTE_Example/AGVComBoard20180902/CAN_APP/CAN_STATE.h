#ifndef __CAN_STATE_H
#define __CAN_STATE_H
#include "RTE_Include.h"
/*
*********************************************************************************************************
*CAN通信规约V1.4中的宏定义
*********************************************************************************************************
*/
#define AGV_TRANSBOARD             0x0D
#define AGV_MAINBOARD              0x01

#define AGV_MAINBOARD_INFOR_CMD    0x02
#define AGV_RUNSTATUS_CMD          0X21
#define AGV_DOORSTATUS_CMD         0X23
#define AGV_MODE_CMD               0X31
#define AGV_MODE_CTRL_CMD          0x80
#define AGV_GET_LIFT_NUM_CMD       0x81
#define AGV_APPLY_CMD              0x88
#define AGV_RELEASE_CMD            0x89
#define AGV_EXCUTE_CMD            0x8A
#define AGV_FINISH_CMD             0x8B
#define AFV_HEATBEAT_CMD           0X42
#define AGV_SOFTWARE_CMD           0X43
//电梯运行参数结构体
typedef enum
{
	WAITGET = 0x00,
	DYNATICUP = 0x01,
	DYNATICDOWN = 0x02,
	STATICUP = 0x03,
	STATICDOWN = 0x04,
	SERVICEOVER = 0x05,
}EleRunStatus_t;
typedef struct
{
	//电梯参数
	uint8_t EleNumID[6];
	uint8_t WorkMode;
	//即时状态
	bool AGVModeFlag;
	uint8_t DoorStatus;
	EleRunStatus_t RunStatus;
	uint8_t CurrentFloor;
	uint8_t DestFloor;
	bool ApplyStatus;
	bool ExcuteStatus;
	bool FinishStatus;
	bool ReleaseStatus;
	bool ModeCtrlStatus;
	bool DoorReady;
	//旧状态
	uint8_t LastStatus;
	//变化提示信息 使用过后需要置零
	bool GetToDestination;
	bool DoorOpenAction;
	bool DoorClosedAction;
	bool UpAction;
	bool DownAction;
}EleStatus_t;



extern EleStatus_t EleStatus;
extern void CANDataParser(uint8_t *CanData);
extern unsigned int CANCRC8Check(unsigned char *dt, unsigned int length);

#endif
