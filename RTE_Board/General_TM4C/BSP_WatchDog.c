#include "BSP_WatchDog.h"
void BSP_WatchDog_Init(void)
{
	//初始化看门狗
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
	IntEnable(INT_WATCHDOG);
	WatchdogReloadSet(WATCHDOG0_BASE, SystemCoreClock*10);
	WatchdogResetEnable(WATCHDOG0_BASE);
	WatchdogEnable(WATCHDOG0_BASE);
}
//系统软复位   
void BSP_WatchDog_SoftReset(void)
{   
	SCB->AIRCR =0X05FA0000|(uint32_t)0x04;	  
} 	
void WDT0_Handler(void)
{
	printf("[WATCHDOG]    Restart After 5000MS!\n");
	
	RTE_RoundRobin_DelayMS(5000);
	BSP_WatchDog_SoftReset();
}

