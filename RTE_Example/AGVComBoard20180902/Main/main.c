#include "RTE_Include.h"
#include "BSP_Led.h"
#include "BSP_Systick.h"
#include "BSP_Com.h"
#include "BSP_CAN.h"
#include "BSP_WatchDog.h"
#include "CAN_STATE.h"
#include "CAN_SEND.h"
#include "Net_Config.h"
#include "WebService_Parser.h"
#include "TCP_Server.h"
#include "TCP_Client.h"
#include "AGV_SM.h"
#include "BSP_Time.h"
#define CAN_DEBUG 0
/*----------------------------------------------------------------------------
 * AGV梯控转接板程序
 * Author：单雷
 * Version:5.1.1_0902
 * 改动说明：
 *         5.0.0_0204 使用RTX5以及RL_NET的原始程序版本，未实地运行；
 *         5.1.0_0827 同周在昆山实地运行，存在任务结束告知服务器仅一次的问题，以及电梯模式导致的状态机假死；
 *         5.1.1_0902 同周在昆山实地运行，修正以上问题，待观察新问题；
 *---------------------------------------------------------------------------*/
const uint8_t MontherSoftVer = 5;
const uint8_t SonSoftVer = 1;
const uint8_t SubSoftVer = 1;
const unsigned short UpdateDate = (unsigned short)(18<<9)|(9<<5)|(2);
/* default ENV set for user */
const ef_env default_env_set[] = {
        {"device_id","1"},
        {"boot_times","0"},
				{"taskid","0"},
				{"adminpassword","admin"},
				{"agvpassword","e3r4y6p0"},
};
int RTE_Putchar (int ch)
{
	UARTCharPut(DEBUG_USART,ch);
	return ch;
}
int RTE_Getchar (void)
{
	return 0;
}
/*----------------------------------------------------------------------------
 * AGV thread
 *---------------------------------------------------------------------------*/
osThreadId_t ThreadIDAGV;
extern int TCPServerSock;
static uint64_t agv_thread_stk[8192 / 8];
const osThreadAttr_t AGVThreadControl = {
	.stack_mem = &agv_thread_stk[0],
  .stack_size =  sizeof(agv_thread_stk) , 
	.priority = osPriorityNormal,
};
static void RTCTimer_Callback(void* UserParameters) {
	struct tm sTime;
	if(BSP_Time_Get(&sTime) == false)
	{
		return;
	}
	if(
			((sTime.tm_hour==11)&&(sTime.tm_min==0))
			||((sTime.tm_hour==16)&&(sTime.tm_min==0))
			||((sTime.tm_hour==23)&&(sTime.tm_min==0))
			||((sTime.tm_hour==4)&&(sTime.tm_min==0))
		)
	{
		if(EleStatus.WorkMode == 0x0f)
		{
			if(AGV_RunningTask.TaskDetailKey[0] == 0)
			{
				printf("[ELE]    管制时间 准备退出自动模式...\n");
				CANAGVModeControl(false);
			}
		}
		else if(EleStatus.WorkMode == 0x02)
		{
			printf("[ELE]    已经是自动模式！\n");
		}
	}
	else 	if(
			((sTime.tm_hour==11)&&(sTime.tm_min==30))
			||((sTime.tm_hour==16)&&(sTime.tm_min==30))
			||((sTime.tm_hour==23)&&(sTime.tm_min==30))
			||((sTime.tm_hour==4)&&(sTime.tm_min==30))
			)
	{
		if(EleStatus.WorkMode == 0x02)
		{
			printf("[ELE]    非管制时间 准备进入自动模式...\n");
			CANAGVModeControl(true);
		}
		else if(EleStatus.WorkMode == 0x0f)
		{
			printf("[ELE]    已经是手动模式！\n");
		}
	}
}
__NO_RETURN void AGVThread (void *argument){
	AGV_SM_Init();
	RTE_RoundRobin_CreateTimer("RTCTimer",30000,1,1,RTCTimer_Callback,(void *)0);
	for(;;)
	{
		StateMachine_Run(&AGV_RunningTask.AGVStateMachine,&AGV_RunningTask);
		osDelay(5);
	}
}
/*----------------------------------------------------------------------------
 * WebService thread
 *---------------------------------------------------------------------------*/
osThreadId_t ThreadIDWebService;
static uint64_t webservice_thread_stk[1024 / 8];
const osThreadAttr_t WebServiceThreadControl = {
	.stack_mem = &webservice_thread_stk[0],
  .stack_size =  sizeof(webservice_thread_stk) , 
	.priority = osPriorityBelowNormal,
};
__NO_RETURN void WebServiceThread (void *argument){
	ThreadIDAGV = osThreadNew(AGVThread, NULL, &AGVThreadControl);
	for(;;)
	{
		osThreadFlagsWait(0x0001U, osFlagsWaitAny, osWaitForever);
		WebServiceDataParser(TCPServerRealData,TCPServerRealDataLenth);
		memset(TCPServerRealData,0,2048);
		TCPServerRealDataLenth = 0;
	}
}
/*----------------------------------------------------------------------------
 * NET thread
 *---------------------------------------------------------------------------*/
 // thread id
osThreadId_t ThreadIDNET;
static uint64_t net_thread_stk[1024 / 8];
const osThreadAttr_t NETThreadControl = {
	.stack_mem = &net_thread_stk[0],
  .stack_size =  sizeof(net_thread_stk) , 
	.priority = osPriorityHigh,
};
/*Timer Callback function for Network */
static void NetWorkTimer_Callback(void* UserParameters) {
	timer_tick();
	osThreadFlagsSet(ThreadIDNET,0x0001U);
}
__NO_RETURN void NETThread (void *argument){
	/* 初始化网络协议栈 */
	init_TcpNet();
	RTE_RoundRobin_CreateTimer("NETTimer",50,1,1,NetWorkTimer_Callback,(void *)0);
	TCPServerInit();
	TCPClientInit();
	ThreadIDWebService =  osThreadNew(WebServiceThread, NULL, &WebServiceThreadControl);
  for (;;) {
		osThreadFlagsWait(0x0001U, osFlagsWaitAny, osWaitForever);
		main_TcpNet();
  }
}
/*----------------------------------------------------------------------------
 * CAN thread
 *---------------------------------------------------------------------------*/
 // thread id
osThreadId_t ThreadIDCAN;
static uint64_t can_thread_stk[1024 / 8];
const osThreadAttr_t CANThreadControl = {
	.stack_mem = &can_thread_stk[0],
  .stack_size =  sizeof(can_thread_stk) , 
	.priority = osPriorityAboveNormal,
};
static void CANModeTimer_Callback(void* UserParameters) {
	if(EleStatus.WorkMode == 0x02)
	{
		printf("[ELE]    准备进入自动模式...\n");
		CANAGVModeControl(true);
	}
	else if(EleStatus.WorkMode == 0x0f)
	{
		printf("[ELE]    已经是自动模式\n");
	}
	RTE_RoundRobin_RemoveTimer("CANModeTimer");
}
__NO_RETURN void CANThread (void *argument) {
	BSP_CAN_Init(CAN_1);
	RTE_RoundRobin_CreateTimer("CANModeTimer",10000,0,1,CANModeTimer_Callback,(void *)0);
	for(;;)
	{
		osThreadFlagsWait(0x0001U, osFlagsWaitAny, osWaitForever);
		if(CANControlHandle[CAN_1].CANMessage.ui32MsgID == AGV_MAINBOARD)
		{
			uint8_t CRCCheck = 0;
			CRCCheck = CANCRC8Check(CANControlHandle[CAN_1].CANBuffer.pu8Databuf,CANControlHandle[CAN_1].CANMessage.ui32MsgLen-1);
			if(CRCCheck == CANControlHandle[CAN_1].CANBuffer.pu8Databuf[CANControlHandle[CAN_1].CANMessage.ui32MsgLen-1])
			{
#if CAN_DEBUG == 1
				printf("[CAN]    CRC Check Success!\n");
				printf("[CAN]    Get Data:");
				for(uint8_t i=0;i<CANControlHandle[CAN_1].CANMessage.ui32MsgLen;i++)
				{
					printf("%02x ",CANControlHandle[CAN_1].CANBuffer.pu8Databuf[i]);
				}
				printf("  CNT:%d ID:%x\n",CANControlHandle[CAN_1].CANMessage.ui32MsgLen,CANControlHandle[CAN_1].CANMessage.ui32MsgID);
#endif
				CANDataParser(CANControlHandle[CAN_1].CANBuffer.pu8Databuf);
			}
		}
		memset(CANControlHandle[CAN_1].CANBuffer.pu8Databuf,0,CANControlHandle[CAN_1].DataBufferLen);
		CANControlHandle[CAN_1].CANMessage.ui32MsgLen = 0;
		CANControlHandle[CAN_1].CANMessage.ui32MsgID = 0;
		IntEnable(INT_CAN1);
	}
}
/*----------------------------------------------------------------------------
 * System thread and a LEDTimer callback
 *---------------------------------------------------------------------------*/
static void LEDTimer_Callback(void* UserParameters) {
	BSP_LED_Toggle(LED_0);
}
static void Boot_Information(void) {
	uint32_t i_boot_times = NULL;
	char *c_old_boot_times, c_new_boot_times[11] = {0};
	char *taskid;
	/* get the boot count number from Env */
	c_old_boot_times = ef_get_env("boot_times");
	taskid = ef_get_env("taskid");
	i_boot_times = atol(c_old_boot_times);
	/* boot count +1 */
	i_boot_times ++;
	printf("[KVDB]    系统第 %d 次启动\n", i_boot_times);
	printf("[KVDB]    上一次的任务ID为 %s\n", taskid);
	if(strcmp(taskid,"0"))
	{
		printf("[KVDB]    上次任务未完成！\n");
		memcpy(AGV_RunningTask.LastTaskDetailKey,taskid,strlen(taskid));
		memcpy(AGV_RunningTask.TaskDetailKey,taskid,strlen(taskid));
	}
	/* interger to string */
	sprintf(c_new_boot_times,"%ld", i_boot_times);
	/* set and store the boot count number to Env */
	ef_set_env("boot_times", c_new_boot_times);
	ef_save_env();
}
static uint64_t system_thread_stk[2048 / 8];
const osThreadAttr_t SystemThreadControl = {
	.stack_mem = &system_thread_stk[0],
  .stack_size =  sizeof(system_thread_stk) , 
	.priority = osPriorityRealtime,
};
__NO_RETURN void SystemThread (void *argument) {
	RTE_Init();
	BSP_LED_Init();
#if DEBUG_USART == UART0_BASE
	BSP_COM_Init(COM_0);
#else
	BSP_COM_Init(COM_2);
#endif
	RTE_RoundRobin_CreateTimer("LEDTimer",200,1,1,LEDTimer_Callback,(void *)0);
	size_t default_env_set_size = sizeof(default_env_set) / sizeof(default_env_set[0]);
	EfErrCode result = EF_NO_ERR;
	if (result == EF_NO_ERR) 
	{
		result = ef_env_init(default_env_set, default_env_set_size);
		if(result == EF_NO_ERR)
			Boot_Information();
	}
	BSP_Time_Init();
	ThreadIDCAN = osThreadNew(CANThread, NULL, &CANThreadControl);
	ThreadIDNET = osThreadNew(NETThread, NULL, &NETThreadControl);
	for (;;) 
	{
		RTE_RoundRobin_TickHandler();
		osDelay(1);
	}
}
int main(void)
{
	SystemCoreClockUpdate();
	NVIC_SetPriorityGrouping((uint32_t)0x00000003U); //中断分组4
	BSP_Systick_Init();
  osKernelInitialize();                 // Initialize CMSIS-RTOS
  osThreadNew(SystemThread, NULL, &SystemThreadControl);    // Create SYSTEM thread
  osKernelStart();                      // Start thread execution
  for (;;) {}
}
