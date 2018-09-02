#ifndef __RTE_INCLUDE_H
#define __RTE_INCLUDE_H
#ifdef __cplusplus
extern "C" {
#endif
	//¶ÏÑÔÍ·ÎÄ¼þ
	#include <assert.h>
	//
	#include <string.h>
	#include <stdarg.h>
	#include <stdbool.h>
	#include <stdint.h>
	#include <stddef.h>
	#include <math.h>
	#include "RTE_Config.h"
	#if RTE_USE_BGET == 1
	#include "RTE_BGet.h"
	#endif
	#if RTE_USE_STDIO == 1
	#include <stdio.h>
	#else
	#include "RTE_Stdio.h"
	#endif
	#if RTE_USE_STDLIB == 1
	#include <stdlib.h>
	#else
	#include "RTE_Stdlib.h"
	#endif
	#if RTE_USE_HUMMANINTERFACE == 1
		#if HI_USE_SHELL == 1
		#include "RTE_Shell.h"
		#endif
		#if HI_USE_RINGBUF == 1
		#include "RTE_RingQuene.h"
		#endif
	#endif
	#if RTE_USE_ROUNDROBIN == 1
		#include "RTE_RoundRobin.h"
		#if RTE_USE_OS == 1
		#include "cmsis_os2.h"
		#endif
	#endif
	#if RTE_USE_STATEMACHINE == 1
		#include "RTE_StateMachine.h"
	#endif
	
	#if RTE_USE_KVDB == 1
	  #include "RTE_KVDB.h"
	#endif
	
	
	
	extern void RTE_Init(void);
	extern void RTE_Assert(char *file, uint32_t line);
	#if 1
	#define RTE_AssertParam(expr) {                                     \
																	 if(!(expr))                        \
																	 {                                  \
																			 RTE_Assert(__FILE__, __LINE__);\
																	 }                                  \
																}
	#else
	#define RTE_AssertParam(expr) 
	#endif		
	#ifdef __BIG_ENDIAN
	 #define U32_LE(v)      (U32)(__rev(v))
	 #define U16_LE(v)      (U16)(__rev(v) >> 16)
	 #define U32_BE(v)      (U32)(v)
	 #define U16_BE(v)      (U16)(v)
	#else
	 #define U32_BE(v)      (U32)(__rev(v))
	 #define U16_BE(v)      (U16)(__rev(v) >> 16)
	 #define U32_LE(v)      (U32)(v)
	 #define U16_LE(v)      (U16)(v)
	#endif
	#define ntohs(v)        U16_BE(v)
	#define ntohl(v)        U32_BE(v)
	#define htons(v)        ntohs(v)
	#define htonl(v)        ntohl(v)
#ifdef __cplusplus
}
#endif
#endif
/****************** (C) COPYRIGHT SuChow University Shannon********************/
