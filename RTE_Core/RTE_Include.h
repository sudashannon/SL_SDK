#ifndef __RTE_INCLUDE_H
#define __RTE_INCLUDE_H
#ifdef __cplusplus
extern "C" {
#endif
    #include <stdio.h>
	#include <string.h>
	#include <stdarg.h>
	#include <stdbool.h>
	#include <stdint.h>
	#include <stddef.h>
	#include <math.h>
	#include <stdarg.h>
	#include <time.h>
	#include "RTE_Config.h"
	#include "RTE_MATH.h"
	#if RTE_USE_OS == 1
	#include "cmsis_os2.h"
	#endif
	#if RTE_USE_MEMMANAGE == 1
	#include "RTE_Memory.h"
	#endif
	#if RTE_USE_VEC == 1
	#include "RTE_Vec.h"
	#endif
	#if RTE_USE_LL == 1
	#include "RTE_LinkList.h"
	#endif
	#if RTE_USE_USTRING == 1
	#include "RTE_UString.h"
	#endif
	#if RTE_USE_USTDOUT == 1
	#include "RTE_UStdout.h"
	#endif
	#if RTE_USE_KVDB == 1
	#include "RTE_KVDB.h"
	extern void RTE_KVDB_Init(void);
	#endif
	#if RTE_USE_SM == 1
	#include "RTE_StateMachine.h"
	#endif
	#if RTE_USE_RINGQUENE == 1
	#include "RTE_RingQuene.h"
	#endif
	#if RTE_USE_SHELL == 1
	#include "RTE_Shell.h"
	#endif
	#if RTE_USE_ROUNDROBIN == 1
	#include "RTE_RoundRobin.h"
	#endif
	#if RTE_USE_GUI == 1
	#include "GUI_Include.h"
	#endif
	extern void RTE_Init(void);
	#ifndef UNUSED
	#define UNUSED(x) ((void)(x))
	#endif
#ifdef __cplusplus
}
#endif
#endif
