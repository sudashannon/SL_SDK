#ifndef __RTE_INCLUDE_H
#define __RTE_INCLUDE_H
#ifdef __cplusplus  
extern "C" {  
#endif  
	#include <string.h>
	#include <stdarg.h>
	#include <stdbool.h>
	#include <stdint.h>
	#include <stddef.h>
	#include <stdlib.h>
	#include <math.h>
	#include <stdarg.h>
	#include <time.h>
	#include "RTE_Config.h"
	#include "RTE_LOG.h"
	#include "RTE_MATH.h"
	#if RTE_USE_MEMMANAGE
	#include "RTE_MEM.h"
		#if RTE_USE_VEC
		#include "RTE_Vec.h"
		
		#endif
	#endif	
	#if RTE_USE_USTRING
	#include "RTE_UString.h"
	#endif
	#if RTE_USE_USTDOUT
	#include "RTE_UStdout.h"
	#endif
	#if RTE_USE_KVDB
	#include "RTE_KVDB.h"
	#endif
	#if RTE_USE_SM
	#include "RTE_StateMachine.h"
	#endif
	#if RTE_USE_RINGQUENE
	#include "RTE_RingQuene.h"
	#endif
	#if RTE_USE_SHELL
	#include "RTE_Shell.h"
	#endif
	#if RTE_USE_ROUNDROBIN
	#include "RTE_RoundRobin.h"
	#endif
	#define RTE_ALIGN_32BYTES(buf) buf __attribute__ ((aligned (32)))
	extern void RTE_Init(void);
	extern void RTE_Assert(char *file, uint32_t line);
	#if RTE_USE_ASSERT
	#define RTE_AssertParam(expr) {                                     \
																	 if(!(expr))                        \
																	 {                                  \
																			 RTE_Assert(__FILE__, __LINE__);\
																	 }                                  \
																}
	#else
	#define RTE_AssertParam(expr) 
	#endif		
#ifdef __cplusplus  
}  
#endif  
#endif
