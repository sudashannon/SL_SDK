#include "APP_SoftTimer.h"
#include "BSP_Include.h"
static volatile uint32_t APPSysRunTime = 0;
/* Private structure */
static APP_SoftTimer_t TimerHandlers[TIMER_CNT];
uint32_t APP_SoftTimer_Init(void) {
#if USE_OS == 0
  /* Set systick to 1ms */
  SysTick_Config(SystemCoreClock / 1000);
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 3));
#endif
#if USE_CPU_LOAD==1
	
#endif
#ifndef RTE_Compiler_EventRecorder
	uint32_t c;
	/* Enable TRC */
	CoreDebug->DEMCR &= ~0x01000000;
	CoreDebug->DEMCR |=  0x01000000;

	/* Enable counter */
	DWT->CTRL &= ~0x00000001;
	DWT->CTRL |=  0x00000001;

	/* Reset counter */
	DWT->CYCCNT = 0;
	
	/* Check if DWT has started */
	c = DWT->CYCCNT;
	
	/* 2 dummys */
	__ASM volatile ("NOP");
	__ASM volatile ("NOP");
	
	/* Return difference, if result is zero, DWT has not started */
	return (DWT->CYCCNT - c);
#endif
#ifdef RTE_Compiler_EventRecorder
	return true;
#endif
}
APP_SoftTimer_t* APP_SoftTimer_Create(APP_SoftTimer_Name_e TimerID,uint32_t ReloadValue, uint8_t AutoReloadCmd, uint8_t StartTimer, void (*CustomTimerCallback)(void *), void* UserParameters) {
	/* Check if available */
	if (TimerID >= TIMER_CNT) {
		return NULL;
	}
	/* Fill settings */
	TimerHandlers[TimerID].ARR = ReloadValue;
	TimerHandlers[TimerID].CNT = TimerHandlers[TimerID].ARR;
	TimerHandlers[TimerID].Flags.F.AREN = AutoReloadCmd;
	TimerHandlers[TimerID].Flags.F.CNTEN = StartTimer;
	TimerHandlers[TimerID].Callback = CustomTimerCallback;
	TimerHandlers[TimerID].UserParameters = UserParameters;
	/* Increase number of timers in memory */
	/* Return pointer to user */
	return &TimerHandlers[TimerID];
}
void APP_SoftTimer_Delete(APP_SoftTimer_Name_e TimerID) {
	/* Check for valid input */
	if (TimerID >= TIMER_CNT) {
		return;
	}
	memset(&TimerHandlers[TimerID],0,sizeof(APP_SoftTimer_t));
}
APP_SoftTimer_t* APP_SoftTimer_Pause(APP_SoftTimer_Name_e TimerID) {
	/* Check for valid input */
	if (TimerID >= TIMER_CNT) {
		return NULL;
	}
	/* Disable timer */
	TimerHandlers[TimerID].Flags.F.CNTEN = 0;
	/* Return pointer */
	return (&TimerHandlers[TimerID]);
}
APP_SoftTimer_t* APP_SoftTimer_Resume(APP_SoftTimer_Name_e TimerID) {
	/* Check for valid input */
	if (TimerID >= TIMER_CNT) {
		return NULL;
	}
	/* Enable timer */
	TimerHandlers[TimerID].Flags.F.CNTEN = 1;
	/* Return pointer */
	return (&TimerHandlers[TimerID]);
}
APP_SoftTimer_t* APP_SoftTimer_Reset(APP_SoftTimer_Name_e TimerID) {
	/* Check for valid input */
	if (TimerID >= TIMER_CNT) {
		return NULL;
	}
	/* Reset timer */
	TimerHandlers[TimerID].CNT = TimerHandlers[TimerID].ARR;
	/* Return pointer */
	return &TimerHandlers[TimerID];
}
APP_SoftTimer_t* APP_SoftTimer_AutoReloadCommand(APP_SoftTimer_Name_e TimerID, uint8_t AutoReloadCommand) {
	/* Check for valid input */
	if (TimerID >= TIMER_CNT) {
		return NULL;
	}
	/* Set new auto reload command */
	TimerHandlers[TimerID].Flags.F.AREN = AutoReloadCommand ? 1 : 0;
	/* Return pointer */
	return &TimerHandlers[TimerID];
}
APP_SoftTimer_t* APP_SoftTimer_AutoReloadValue(APP_SoftTimer_Name_e TimerID, uint32_t AutoReloadValue) {
	/* Check for valid input */
	if (TimerID >= TIMER_CNT) {
		return NULL;
	}
	/* Reset timer */
	TimerHandlers[TimerID].ARR = AutoReloadValue;
	/* Return pointer */
	return &TimerHandlers[TimerID];
}
/* Called from Systick handler */
void APP_SoftTimer_TickHanlder(void) {
	uint8_t i;
	/* Increase system time */
	APPSysRunTime++;
	/* Check for timers */
	/* Check custom timers */
	for (i = 0; i < TIMER_CNT; i++) {
		/* Check if timer is enabled */
		if (
			TimerHandlers[i].Flags.F.CNTEN /*!< Timer is enabled */
		) {
			/* Decrease counter if needed */
			if (TimerHandlers[i].CNT) {
				TimerHandlers[i].CNT--;
			}
			/* Check if count is zero */
			if (TimerHandlers[i].CNT == 0) {
				/* Call user callback function */
				TimerHandlers[i].Callback(TimerHandlers[i].UserParameters);
				/* Set new counter value */
				TimerHandlers[i].CNT = TimerHandlers[i].ARR;
				/* Disable timer if auto reload feature is not used */
				if (!TimerHandlers[i].Flags.F.AREN) {
					/* Disable counter */
					APP_SoftTimer_Delete((APP_SoftTimer_Name_e)i);
				}
			}
		}
	}
#ifdef USE_HAL_DRIVER
	HAL_SYSTICK_IRQHandler();
#endif
}
uint32_t APP_SoftTimer_GetTick(void) {
	/* Return current time in milliseconds */
#if USE_OS == 1
	#ifdef RTE_CMSIS_RTOS
	if (osKernelRunning () == true) 
	{
    return APPSysRunTime;
  }
	#else
	if (osKernelGetState () == osKernelRunning) 
	{
    return APPSysRunTime;
  }
	#endif
	else
	{
		static uint32_t ticks = 0U;
					 uint32_t i;
		/* If Kernel is not running wait approximately 1 ms then increment 
			 and return auxiliary tick counter value */
		for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
			__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
			__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		}
		return ++ticks;
	}
#else
	return APPSysRunTime;
#endif 
}
APP_SoftTimer_t* APP_SoftTimer_GetHandler(APP_SoftTimer_Name_e TimerID) {
	/* Check for valid input */
	if (TimerID >= TIMER_CNT) {
		return NULL;
	}
	return &TimerHandlers[TimerID];
}
/**
 * @brief  Delays for amount of micro seconds
 * @param  micros: Number of microseconds for delay
 * @retval None
 */
__inline void APP_SoftTimer_DelayUS(__IO uint32_t micros) {
	uint32_t start = DWT->CYCCNT;
	/* Go to number of cycles for system */
	micros *= (SystemCoreClock / 1000000);
	/* Delay till end */
	while ((DWT->CYCCNT - start) < micros);
};

__inline uint32_t APP_SoftTimer_GetDWTTime(void) {
	uint32_t start = DWT->CYCCNT;
	return start;
};
__inline uint32_t APP_SoftTimer_CalculateDWTTime(__IO uint32_t start,__IO uint32_t end) {
	uint32_t temp;
	if(end >= start)
		temp = end - start;
	else
		temp = end + 0xFFFFFFFF - start;
	return temp*1000000/SystemCoreClock;
};
//note:在使用操作系统的情况下调用此函数进行延时 系统有可能调度到其他任务，若需要绝对延时，请调用DelayUS
void APP_SoftTimer_DelayMS(uint32_t Delay) {
	/* Delay for amount of milliseconds */
	/* Check if we are called from ISR */
	if (__get_IPSR() == 0) {
		/* Called from thread mode */
		uint32_t tickstart = APP_SoftTimer_GetTick();
		
		/* Count interrupts */
		while ((APP_SoftTimer_GetTick() - tickstart) < Delay) {
#if USE_LOWPOWER == 1
			/* Go sleep, wait systick interrupt */
			__WFI();
#endif
		}
	} else {
		/* Called from interrupt mode */
		while (Delay) {
			/* Check if timer reached zero after we last checked COUNTFLAG bit */
			if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
				Delay--;
			}
		}
	}
}
#if USE_OS == 0
/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	APP_SoftTimer_TickHanlder();
}
#endif
#ifdef USE_HAL_DRIVER
/**
  * @brief Provide a tick value in millisecond.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval tick value
  */
uint32_t HAL_GetTick(void)
{
  return APP_SoftTimer_GetTick();
}

/**
  * @brief This function provides minimum delay (in milliseconds) based 
  *        on variable incremented. 
  * @note In the default implementation , SysTick timer is the source of time base.
  *       It is used to generate interrupts at regular time intervals where uwTick
  *       is incremented.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @param Delay  specifies the delay time length, in milliseconds.
  * @retval None
  */
void HAL_Delay(uint32_t Delay)
{
	APP_SoftTimer_DelayMS(Delay);
}
#endif
