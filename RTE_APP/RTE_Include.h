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
	#if RTE_USE_OS 
	#include "cmsis_os2.h"
	#endif
	#if RTE_USE_MEMMANAGE
	#include "RTE_MEM.h"
	#endif	
	#if RTE_USE_VEC
	#include "RTE_Vec.h"
	#endif
	#if RTE_USE_LL
	#include "RTE_LinkList.h"
	#endif
	#if RTE_USE_USTRING
	#include "RTE_UString.h"
	#endif
	#if RTE_USE_USTDOUT
	#include "RTE_UStdout.h"
	#endif
	#if RTE_USE_KVDB
	#include "RTE_KVDB.h"
	extern void RTE_KVDB_Init(void);
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
	#if RTE_USE_GUI
	#include "GUI_Include.h"
	#endif
	#if RTE_USE_OPENMV
	#include "MV_Include.h"
	#endif
	#define RTE_ALIGN_32BYTES(buf) buf __attribute__ ((aligned (32)))
	extern void RTE_Init(void);
	#ifndef UNUSED
	#define UNUSED(x) ((void)(x))
	#endif
#ifdef __cplusplus  
}  
#endif  
#endif
