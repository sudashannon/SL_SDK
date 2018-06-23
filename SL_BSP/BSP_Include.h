#ifndef __BSP_INCLUDE_H
#define __BSP_INCLUDE_H
/* Includes ------------------------------------------------------------------*/
#include "BSP_Config.h"
#if defined (STM32F1)
#include "stm32f10x.h"
#endif

#if defined (STM32F4)
#include "stm32f4xx.h"
#endif

#if defined (STM32F7)
#include "stm32f7xx_hal.h"
#endif

#if defined (STM32L4)
#include "stm32l4xx_hal.h"
#endif

#endif
/****************** (C) COPYRIGHT SuChow University Shannon*****END OF FILE****/
