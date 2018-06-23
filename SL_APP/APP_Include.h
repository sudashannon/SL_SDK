#ifndef __APP_INCLUDE_H
#define __APP_INCLUDE_H
#ifdef __cplusplus
 extern "C" {
#endif
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "APP_Config.h"
#include "RTE_Components.h"
#if USE_OS == 1
	#ifdef RTE_CMSIS_RTOS2
	#include "cmsis_os2.h"
	#else 
	#include "cmsis_os.h"
	#endif
#endif
#ifdef RTE_Compiler_EventRecorder
	#include "EventRecorder.h"
	#include "EventRecorderConf.h"
#endif
#include "APP_BGet.h"
#include "APP_SoftTimer.h"
#include "APP_RingBuffer.h"
#include "APP_String.h"
#include "APP_Debug.h"
#include "APP_Cmdline.h"
#include "APP_StateMachine.h"

#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,4 bits for subpriority */
#endif
#ifndef NVIC_PRIORITYGROUP_1
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,3 bits for subpriority */
#endif
#ifndef NVIC_PRIORITYGROUP_2
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,2 bits for subpriority */
#endif
#ifndef NVIC_PRIORITYGROUP_3
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,1 bit  for subpriority */
#endif
#ifndef NVIC_PRIORITYGROUP_4
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,0 bit  for subpriority */
#endif
#define APP_ArrayLengthCal(array)     ((int)(sizeof(array) / sizeof((array)[0])))
extern void APP_Standby(void);
extern void APP_SoftReset(void);
extern void APP_Assert(char *file, uint32_t line);
#if 1
#define APP_AssertParam(expr) {                                     \
																 if(!(expr))                        \
																 {                                  \
																		 APP_Assert(__FILE__, __LINE__);\
																 }                                  \
															}
#else
#define APP_AssertParam(expr) 
#endif															
extern void APP_RunEnvInit(void);
#ifdef __cplusplus
}
#endif
#endif
/****************** (C) COPYRIGHT SuChow University Shannon*****END OF FILE****/
