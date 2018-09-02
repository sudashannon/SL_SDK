#include "BSP_Systick.h"
void BSP_Systick_Init(void)
{
	SysTickPeriodSet(SystemCoreClock/1000);
	// Enable the SysTick Interrupt.
	SysTickIntEnable();
	// Enable SysTick.
	SysTickEnable();
}
#if RTE_USE_OS == 0
void SysTick_Handler(void)
{
	RTE_RoundRobin_TickHandler();
}
#endif
