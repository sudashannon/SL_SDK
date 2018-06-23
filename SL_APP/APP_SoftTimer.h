#ifndef __APP_SOFTTIMER_H
#define __APP_SOFTTIMER_H
/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif
#include "APP_Include.h"
typedef enum _APP_SoftTimer_Name_e
{
	TIMER_SYS = 0,
	TIMER_LED = 1,
	TIMER_COM = 2,
	TIMER_CNT,
}APP_SoftTimer_Name_e;
typedef struct _APP_SoftTimer_t {
	union {
		struct {			
			uint8_t AREN:1;  /*!< Auto-reload enabled */
			uint8_t CNTEN:1; /*!< Count enabled */
		} F;
		uint8_t FlagsVal;
	} Flags;
	uint32_t ARR;                                        /*!< Auto reload value */
	uint32_t CNT;                                        /*!< Counter value, counter counts down */
	void (*Callback)(void *);	                           /*!< Callback which will be called when timer reaches zero */
	void* UserParameters;                                /*!< Pointer to user parameters used for callback function */
} APP_SoftTimer_t;
uint32_t APP_SoftTimer_Init(void);
__inline void APP_SoftTimer_DelayUS(volatile uint32_t micros);
__inline uint32_t APP_SoftTimer_GetDWTTime(void);
APP_SoftTimer_t* APP_SoftTimer_Create(APP_SoftTimer_Name_e TimerID,uint32_t ReloadValue, uint8_t AutoReloadCmd, uint8_t StartTimer, void (*CustomTimerCallback)(void *), void* UserParameters);
void APP_SoftTimer_Delete(APP_SoftTimer_Name_e TimerID);
APP_SoftTimer_t* APP_SoftTimer_Pause(APP_SoftTimer_Name_e TimerID);
APP_SoftTimer_t* APP_SoftTimer_Resume(APP_SoftTimer_Name_e TimerID);
APP_SoftTimer_t* APP_SoftTimer_Reset(APP_SoftTimer_Name_e TimerID);
APP_SoftTimer_t* APP_SoftTimer_AutoReloadCommand(APP_SoftTimer_Name_e TimerID, uint8_t AutoReloadCommand);
APP_SoftTimer_t* APP_SoftTimer_AutoReloadValue(APP_SoftTimer_Name_e TimerID, uint32_t AutoReloadValue);
uint32_t APP_SoftTimer_GetTick(void);
void APP_SoftTimer_TickHanlder(void);
APP_SoftTimer_t* APP_SoftTimer_GetHandler(APP_SoftTimer_Name_e TimerID);
void APP_SoftTimer_DelayMS(uint32_t Delay);
/* C++ detection */
#ifdef __cplusplus
}
#endif
#endif
