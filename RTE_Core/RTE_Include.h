#ifndef __RTE_INCLUDE_H
#define __RTE_INCLUDE_H
#ifdef __cplusplus
extern "C" {
#endif
	#include <stdlib.h>
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
	#include "RTE_Printf.h"
	#include "RTE_Memory.h"
	#include "RTE_Vector.h"
	#include "RTE_RoundRobin.h"
	#include "RTE_LinkList.h"
	#include "RTE_Stream.h"
	#include "RTE_StateMachine.h"
	#include "RTE_Shell.h"
	#include "RTE_KVDB.h"
	#include "RTE_Math.h"
	extern void RTE_Init(void);
	#define ALIGN_NBYTES(buf,n) buf __attribute__ ((aligned (n)))
	#ifndef UNUSED
	#define UNUSED(x) ((void)(x))
#endif

#ifdef __cplusplus
}
#endif
#endif
