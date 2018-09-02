#include "BSP_Systick.h"
void BSP_Systick_Init(void)
{
	SysTick_Config(SystemCoreClock / 1000);	//Set SysTick Timer for 1ms interrupts	
}
#if RTE_USE_OS == 0
void SysTick_Handler(void)
{
	RTE_RoundRobin_TickHandler();
}
#endif
