#ifndef __RTE_ROUNDROBIN_H
#define __RTE_ROUNDROBIN_H
#ifdef __cplusplus
extern "C" {
#endif
	#include "RTE_Config.h"
	#if RTE_USE_ROUNDROBIN == 1
	#include <stdint.h>
	#include <stdbool.h>
	#if RTE_USE_OS == 1
	#include "cmsis_os2.h"
	#endif
	#if RR_TYPE == 1||RR_TYPE == 2
	#include "RTE_Vector.h"
	#endif
	typedef enum
	{
		RR_NOERR = 0,
		RR_NOSPACEFORNEW = 1,
		RR_NOSUCHTIMER = 2,
		RR_NOSUCHGROUP = 3,
	}rr_error_e;
	#if RR_TYPE == 0
	typedef enum
	{
		TIMER_0 = 0,
		TIMER_N = 1,
	}rr_timer_name_e;
	#endif
	typedef struct
	{
		union {
			struct {
				uint8_t AREN:1;  /*!< Auto-reload enabled */
				uint8_t CNTEN:1; /*!< Count enabled */
			} F;
			uint8_t FlagsVal;
		} Flags;
	#if RR_TYPE == 1 || RR_TYPE == 2
		uint8_t TimerID;/*!< Timer ID */
	#endif
		volatile uint32_t ARR;                                        /*!< Auto reload value */
		volatile uint32_t CNT;                                        /*!< Counter value, counter counts down */
		void (*Callback)(void *);	                           /*!< Callback which will be called when timer reaches zero */
		void* UserParameters;                                /*!< Pointer to user parameters used for callback function */
	}rr_softtimer_t;
	#if RR_TYPE == 1
	typedef struct
	{
		vector_t *SoftTimerTable;
		volatile uint32_t RoundRobinRunTick;
	}rr_t;
	#endif
	#if RR_TYPE == 2
	typedef struct
	{
		uint8_t TimerGroupID;
		vector_t *SoftTimerTable;
	}rr_timergroup_t;
	typedef struct
	{
		uint8_t TimerGroupCnt;
		volatile uint32_t RoundRobinRunTick;
		rr_timergroup_t *TimerGroup;
	}rr_t;
	#endif

	extern void RoundRobin_Init(void);
	extern void RoundRobin_TickHandler(void);
	extern void RoundRobin_Run(
	#if RR_TYPE == 2 
		uint8_t GroupID
	#else
		void
	#endif
	);

	#if RR_TYPE == 2
	extern rr_error_e RoundRobin_CreateGroup(uint8_t GroupID);
	#endif
	extern rr_error_e RoundRobin_CreateTimer(
	#if RR_TYPE == 2
		uint8_t GroupID,
	#endif
		uint8_t TimerID,
		uint32_t ReloadValue,
		uint8_t ReloadEnable,
		uint8_t RunEnable,
		void (*TimerCallback)(void *),
		void* UserParameters);
	extern rr_error_e RoundRobin_RemoveTimer(
	#if RR_TYPE == 2
		uint8_t GroupID,
	#endif
		uint8_t TimerID);
	extern rr_error_e RoundRobin_ReadyTimer(
	#if RR_TYPE == 2
		uint8_t GroupID,
	#endif
		uint8_t TimerID);
	extern rr_error_e RoundRobin_ResetTimer(
	#if RR_TYPE == 2
		uint8_t GroupID,
	#endif
		uint8_t TimerID);
	extern rr_error_e RoundRobin_PauseTimer(
	#if RR_TYPE == 2
		uint8_t GroupID,
	#endif
		uint8_t TimerID);
	extern rr_error_e RoundRobin_ResumeTimer(
	#if RR_TYPE == 2
		uint8_t GroupID,
	#endif
		uint8_t TimerID);
	extern bool RoundRobin_IfRunTimer(
	#if RR_TYPE == 2
		uint8_t GroupID,
	#endif
		uint8_t TimerID);
	extern uint8_t RoundRobin_GetTimerNum(
	#if RR_TYPE == 2
		uint8_t GroupID
	#else
		void
	#endif
	);
	extern uint32_t RoundRobin_GetTimerCNT(
	#if RR_TYPE == 2
		uint8_t GroupID,
	#endif
		uint8_t TimerID
	);


	extern void RoundRobin_Demon(void);
	extern uint32_t RoundRobin_GetTick(void);
	extern uint32_t RoundRobin_TickElaps(uint32_t prev_tick);
	extern void RoundRobin_Delay(uint32_t Delay);
	#if RR_DWT == 1
	extern __inline void RoundRobin_DelayUS(volatile uint32_t micros);
	#endif
	#endif
#ifdef __cplusplus
}
#endif
#endif
