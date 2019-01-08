#ifndef __RTE_ROUNDROBIN_H
#define __RTE_ROUNDROBIN_H
#ifdef __cplusplus
extern "C" {
#endif
#include "RTE_Config.h"
#if RTE_USE_ROUNDROBIN
#include <stdint.h>
#include <stdbool.h>
#if RTE_USE_OS
#include "cmsis_os2.h"
#endif
#include "RTE_Vec.h"
typedef enum
{
	RR_NOERR = 0,
	RR_HANDLEUNINIT = 1,
	RR_NOSPACEFORNEW = 2,
	RR_ALREADYEXIST = 3,
	RR_NOSUCHTIMER = 4,
	RR_NOSUCHGROUP = 5,
}RTE_RoundRobin_Err_e;
typedef struct
{
	union {
		struct {			
			uint8_t AREN:1;  /*!< Auto-reload enabled */
			uint8_t CNTEN:1; /*!< Count enabled */
		} F;
		uint8_t FlagsVal;
	} Flags;
	uint8_t TimerID;/*!< Timer ID */
	volatile uint32_t ARR;                                        /*!< Auto reload value */
	volatile uint32_t CNT;                                        /*!< Counter value, counter counts down */
	const char *TimerName;
	void (*Callback)(void *);	                           /*!< Callback which will be called when timer reaches zero */
	void* UserParameters;                                /*!< Pointer to user parameters used for callback function */
}RTE_SoftTimer_t;
typedef vec_t(RTE_SoftTimer_t) rrtimer_vec_t;
typedef struct
{
	uint8_t TimerGroupID;
	const char *TimerGroupName;
	rrtimer_vec_t SoftTimerTable;
}RTE_RoundRobin_TimerGroup_t;
typedef struct
{
	uint8_t TimerGroupCnt;
#if RTE_USE_OS == 0
	volatile uint32_t RoundRobinRunTick;
#endif
	RTE_RoundRobin_TimerGroup_t *TimerGroup;
}RTE_RoundRobin_t;
extern void RTE_RoundRobin_Init(void);
extern RTE_RoundRobin_Err_e RTE_RoundRobin_CreateGroup(const char *GroupName);
extern int8_t RTE_RoundRobin_GetGroupID(const char *GroupName);


extern RTE_RoundRobin_Err_e RTE_RoundRobin_CreateTimer(
	uint8_t GroupID,
	const char *TimerName,
	uint32_t ReloadValue, 
	uint8_t ReloadEnable, 
	uint8_t RunEnable,
	void (*TimerCallback)(void *), 
	void* UserParameters);
extern int8_t RTE_RoundRobin_GetTimerID(uint8_t GroupID,const char *TimerName);
extern RTE_RoundRobin_Err_e RTE_RoundRobin_RemoveTimer(uint8_t GroupID,uint8_t TimerID);
extern void RTE_RoundRobin_TickHandler(void);
extern void RTE_RoundRobin_Run(uint8_t GroupID);

extern RTE_RoundRobin_Err_e RTE_RoundRobin_ReadyTimer(uint8_t GroupID,uint8_t TimerID);
extern RTE_RoundRobin_Err_e RTE_RoundRobin_ResetTimer(uint8_t GroupID,uint8_t TimerID);
extern RTE_RoundRobin_Err_e RTE_RoundRobin_PauseTimer(uint8_t GroupID,uint8_t TimerID);
extern RTE_RoundRobin_Err_e RTE_RoundRobin_ResumeTimer(uint8_t GroupID,uint8_t TimerID);
extern bool RTE_RoundRobin_IfRunTimer(uint8_t GroupID,uint8_t TimerID);
	
extern void RTE_RoundRobin_Demon(void);

extern uint32_t RTE_RoundRobin_GetTick(void);
extern uint32_t RTE_RoundRobin_TickElaps(uint32_t prev_tick);
extern void RTE_RoundRobin_DelayMS(uint32_t Delay);
extern __inline void RTE_RoundRobin_DelayUS(volatile uint32_t micros);
#endif
#ifdef __cplusplus
}
#endif
#endif
