#ifndef __BSP_TIME_H
#define __BSP_TIME_H
#include "tm4c1294ncpdt.h"
#include "RTE_Include.h"
#include <time.h>
#include "sysctl.h"
#include "hibernate.h"
#include "hw_hibernate.h"
extern void BSP_Time_Init(void);
extern void BSP_Time_Set(uint32_t g_ui32HourIdx,uint32_t g_ui32MinIdx,uint32_t g_ui32MonthIdx,uint32_t g_ui32DayIdx
	,uint32_t g_ui32YearIdx);
extern bool BSP_Time_Display(void);
extern bool BSP_Time_Get(struct tm *sTime);
#endif
