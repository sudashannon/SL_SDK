/******************************************************************************
 * @file     system_ARMCA7.c
 * @brief    CMSIS Device System Source File for Arm Cortex-A7 Device Series
 * @version  V1.0.1
 * @date     13. February 2019
 *
 * @note
 *
 ******************************************************************************/
/*
 * Copyright (c) 2009-2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ARMCA7.h"
#include "irq_ctrl.h"

#define  SYSTEM_CLOCK  12000000U

/*----------------------------------------------------------------------------
	System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = SYSTEM_CLOCK;

/*----------------------------------------------------------------------------
	System Core Clock update function
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (uint32_t clk)
{
	int p = 0;
	int k = 1;
	int m = 1;
	uint32_t val;

	if(clk > 1152000000)
	{
		k = 2;
	}
	else if(clk > 768000000)
	{
		k = 3;
		m = 2;
	}

	/* Switch to 24MHz clock while changing cpu pll */
	val = (2 << 0) | (1 << 8) | (1 << 16);
	WRITE_REG(V3S_CPU_AXI_CFG_REG_BASE, val);

	/* cpu pll rate = ((24000000 * n * k) >> p) / m */
	val = (0x1 << 31);
	val |= ((p & 0x3) << 16);
	val |= ((((clk / (24000000 * k / m)) - 1) & 0x1f) << 8);
	val |= (((k - 1) & 0x3) << 4);
	val |= (((m - 1) & 0x3) << 0);
	WRITE_REG(V3S_PLL_CPU_CTRL_REG_BASE, val);
	sdelay(200);

	/* Switch clock source */
	val = (2 << 0) | (1 << 8) | (2 << 16);
	WRITE_REG(V3S_CPU_AXI_CFG_REG_BASE, val);
	SystemCoreClock = clk;
}

/*----------------------------------------------------------------------------
	System Initialization
 *----------------------------------------------------------------------------*/
void SystemInit (void)
{
	/* do not use global variables because this function is called before
	reaching pre-main. RW section may be overwritten afterwards.          */

	// Invalidate entire Unified TLB
	__set_TLBIALL(0);

	// Invalidate entire branch predictor array
	__set_BPIALL(0);
	__DSB();
	__ISB();

	//  Invalidate instruction cache and flush branch target cache
	__set_ICIALLU(0);
	__DSB();
	__ISB();

	//  Invalidate data cache
	L1C_InvalidateDCacheAll();

	#if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
	// Enable FPU
	__FPU_Enable();
	#endif

	//   // Create Translation Table
	//   MMU_CreateTranslationTable();

	//   // Enable MMU
	//   MMU_Enable();

	//   // Enable Caches
	//   L1C_EnableCaches();
	//   L1C_EnableBTAC();

	// #if (__L2C_PRESENT == 1)
	//   // Enable GIC
	//   L2C_Enable();
	// #endif

	// IRQ Initialize
	IRQ_Initialize();
	SystemCoreClockUpdate(1008000000);
	/* pll video - 396MHZ */
	WRITE_REG(V3S_PLL_VIDEO_CTRL_REG_BASE, 0x91004107);
	/* pll periph0 - 600MHZ */
	WRITE_REG(V3S_PLL_PERIPH0_CTRL_REG_BASE, 0x90041811);
	while(!(READ_REG(V3S_PLL_PERIPH0_CTRL_REG_BASE) & (1 << 28)));
	/* ahb1 = pll periph0 / 3, apb1 = ahb1 / 2 */
	WRITE_REG(V3S_AHB_APB0_CFG_REG_BASE, 0x00003180);
	/* mbus  = pll periph0 / 4 */
	WRITE_REG(V3S_MBUS_CLK_REG_BASE, 0x81000003);
}
