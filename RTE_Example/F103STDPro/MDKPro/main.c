/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 #include "APP_Include.h"
 #include "BSP_Led.h"
 #include "BSP_ComF1.h"
/*----------------------------------------------------------------------------
 * System thread and a LEDTimer callback
 *---------------------------------------------------------------------------*/
static void LEDTimer_Callback(void* UserParameters) {
	BSP_LED_Toggle(LED_0);
}
const osThreadAttr_t SystemThreadControl = {
  .stack_size = 256 , 
	.priority = osPriorityRealtime,
};
__NO_RETURN void SystemThread (void *argument) {
	APP_RunEnvInit(); 
	BSP_LED_Init();
	BSP_USART_Init(COM_1,115200);
	APP_SoftTimer_Resume(TIMER_SYS);
	APP_SoftTimer_Create(TIMER_LED,200,1,1,LEDTimer_Callback,(void *)0);
	for (;;) 
	{
		APP_SoftTimer_TickHanlder();
		osDelay(1);
	}
}
int main (void) {
  // System Initialization
  SystemCoreClockUpdate();
  osKernelInitialize();                 // Initialize CMSIS-RTOS
  osThreadNew(SystemThread, NULL, &SystemThreadControl);    // Create application main thread
  osKernelStart();                      // Start thread execution
  for (;;) {}
}
