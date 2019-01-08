#ifndef __BSP_FLASH_H
#define __BSP_FLASH_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "RTE_Include.h"
#if defined(STM32F10X_LD) || defined(STM32F10X_LD_VL) || defined (STM32F10X_MD) || defined (STM32F10X_MD_VL)
#define PAGE_SIZE     1024
#else
#define PAGE_SIZE     2048
#endif
#endif /* __EEPROM_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
