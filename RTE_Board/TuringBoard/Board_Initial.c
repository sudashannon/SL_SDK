#include "Board_Initial.h"
const static Board_Clock_Control_t ClockHandle = 
{
	.SourceType = RCC_OSCILLATORTYPE_HSE,//8M晶振作为时钟源
	.PLLM = PLL_M,		//	8/4=2M
	.PLLN = PLL_N,	//	2*400=800M
	.PLLP = 2,		//	800/2=400M = SYSCLK
	.PLLQ = 2,		//	800/2=400M = PLL1Q
	.PLLR = 2,		//	800/2=400M = PLL1R
};
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;
  HAL_MPU_Disable();
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x20000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);	

  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = 0x30040000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER4;
  MPU_InitStruct.BaseAddress = 0x38000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
static void Clock_Config(const Board_Clock_Control_t *clock_control)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
	/**Supply configuration update enable 
	*/
  MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);
	/**Configure the main internal regulator output voltage 
	*/
	//设置调压器输出级别为1
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  while ((PWR->D3CR & (PWR_D3CR_VOSRDY)) != PWR_D3CR_VOSRDY) 
  {
    
  }
    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = clock_control->SourceType;
	switch(clock_control->SourceType)
	{
		case RCC_OSCILLATORTYPE_HSE:
		{
			RCC_OscInitStruct.HSEState = RCC_HSE_ON;
			RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
			RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
			RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
			RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
		}
		break;
		default://不支持其他种类时钟源
      RTE_Assert(__FILE__, __LINE__);	
	}
  RCC_OscInitStruct.PLL.PLLM = ClockHandle.PLLM;
  RCC_OscInitStruct.PLL.PLLN = ClockHandle.PLLN;
  RCC_OscInitStruct.PLL.PLLP = ClockHandle.PLLP;
  RCC_OscInitStruct.PLL.PLLQ = ClockHandle.PLLQ;
  RCC_OscInitStruct.PLL.PLLR = ClockHandle.PLLR;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    RTE_Assert(__FILE__, __LINE__);
  }
	//------时钟树配置
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  //CPU CLOCK 400MHz
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	//AHP CLOCK 200MHz
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	//APB3 CLOCK 100MHz
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
	//APB3 CLOCK 100MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
	//APB3 CLOCK 100MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
	//APB3 CLOCK 100MHz
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    RTE_Assert(__FILE__, __LINE__);
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_SPI4
                              |RCC_PERIPHCLK_SPI1|RCC_PERIPHCLK_SDMMC;
  PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
  PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    RTE_Assert(__FILE__, __LINE__);
  }
}
void Board_Initial(void)
{
	MPU_Config();
  SCB_EnableICache();
  SCB_EnableDCache();
	HAL_Init();
	Clock_Config(&ClockHandle);
/*
    使用IO的高速模式，要使能IO补偿，即调用下面三个函数 
	（1）使能CSI clock
	（2）使能SYSCFG clock
	（3）使能I/O补偿单元， 设置SYSCFG_CCCSR寄存器的bit0
*/
	__HAL_RCC_CSI_ENABLE() ;
	__HAL_RCC_SYSCFG_CLK_ENABLE() ;
	HAL_EnableCompensationCell();
/*
    开启全部RAM的时钟
*/
	MODIFY_REG(RCC->AHB2ENR, RCC_AHB2ENR_D2SRAM1EN, 1);
	MODIFY_REG(RCC->AHB2ENR, RCC_AHB2ENR_D2SRAM2EN, 1);
	MODIFY_REG(RCC->AHB2ENR, RCC_AHB2ENR_D2SRAM3EN, 1);
	MODIFY_REG(RCC->AHB4ENR, RCC_AHB4ENR_D3SRAM1EN, 1);
	MODIFY_REG(RCC->D3AMR, RCC_D3AMR_SRAM4AMEN, 1);
	MODIFY_REG(RCC->AHB2LPENR, RCC_AHB2LPENR_D2SRAM1LPEN, 1);
	MODIFY_REG(RCC->AHB2LPENR, RCC_AHB2LPENR_D2SRAM2LPEN, 1);
	MODIFY_REG(RCC->AHB2LPENR, RCC_AHB2LPENR_D2SRAM3LPEN, 1);
	MODIFY_REG(RCC->AHB4LPENR, RCC_AHB4LPENR_D3SRAM1LPEN, 1);
	/**Configure the Systick interrupt time 
	*/
  HAL_SYSTICK_Config(SystemCoreClock/1000);
	/**Configure the Systick 
	*/
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, SYSTICK_PreemptPriority, 0);
}
void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
  HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
  HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
  HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0);
  HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);
  HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
