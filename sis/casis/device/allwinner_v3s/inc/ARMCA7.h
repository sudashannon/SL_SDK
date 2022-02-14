/******************************************************************************
 * @file     ARMCA7.h
 * @brief    CMSIS Cortex-A7 Core Peripheral Access Layer Header File
 * @version  V1.1.0
 * @date     15. May 2019
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

#ifndef __ARMCA7_H__
#define __ARMCA7_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------  Interrupt Number Definition  ------------------------ */

/** Device specific Interrupt IDs */
typedef enum IRQn
{
    /******  SGI Interrupts Numbers                 ****************************************/
    SGI0_IRQn            =  0,        /*!< Software Generated Interrupt 0 */
    SGI1_IRQn            =  1,        /*!< Software Generated Interrupt 1 */
    SGI2_IRQn            =  2,        /*!< Software Generated Interrupt 2 */
    SGI3_IRQn            =  3,        /*!< Software Generated Interrupt 3 */
    SGI4_IRQn            =  4,        /*!< Software Generated Interrupt 4 */
    SGI5_IRQn            =  5,        /*!< Software Generated Interrupt 5 */
    SGI6_IRQn            =  6,        /*!< Software Generated Interrupt 6 */
    SGI7_IRQn            =  7,        /*!< Software Generated Interrupt 7 */
    SGI8_IRQn            =  8,        /*!< Software Generated Interrupt 8 */
    SGI9_IRQn            =  9,        /*!< Software Generated Interrupt 9 */
    SGI10_IRQn           = 10,        /*!< Software Generated Interrupt 10 */
    SGI11_IRQn           = 11,        /*!< Software Generated Interrupt 11 */
    SGI12_IRQn           = 12,        /*!< Software Generated Interrupt 12 */
    SGI13_IRQn           = 13,        /*!< Software Generated Interrupt 13 */
    SGI14_IRQn           = 14,        /*!< Software Generated Interrupt 14 */
    SGI15_IRQn           = 15,        /*!< Software Generated Interrupt 15 */

    /******  Cortex-A7 Processor Exceptions Numbers ****************************************/
    SecurePhyTimer_IRQn  = 29,        /*!< Physical Timer Interrupt                      */

    /******  Platform Exceptions Numbers ***************************************************/
    Watchdog_IRQn        = 32,        /*!< SP805 Interrupt        */
    Timer0_IRQn          = 34,        /*!< SP804 Interrupt        */
    Timer1_IRQn          = 35,        /*!< SP804 Interrupt        */
    RTClock_IRQn         = 36,        /*!< PL031 Interrupt        */
    UART0_IRQn           = 37,        /*!< PL011 Interrupt        */
    UART1_IRQn           = 38,        /*!< PL011 Interrupt        */
    UART2_IRQn           = 39,        /*!< PL011 Interrupt        */
    UART3_IRQn           = 40,        /*!< PL011 Interrupt        */
    MCI0_IRQn            = 41,        /*!< PL180 Interrupt (1st)  */
    MCI1_IRQn            = 42,        /*!< PL180 Interrupt (2nd)  */
    AACI_IRQn            = 43,        /*!< PL041 Interrupt        */
    Keyboard_IRQn        = 44,        /*!< PL050 Interrupt        */
    Mouse_IRQn           = 45,        /*!< PL050 Interrupt        */
    CLCD_IRQn            = 46,        /*!< PL111 Interrupt        */
    Ethernet_IRQn        = 47,        /*!< SMSC_91C111 Interrupt  */
    VFS2_IRQn            = 73,        /*!< VFS2 Interrupt         */
} IRQn_Type;

/* =========================================================================================================================== */
/* ================                            Device Specific Peripheral Section                             ================ */
/* =========================================================================================================================== */
/* ToDo: add here your device specific peripheral access structure typedefs
         following is an example for a timer */

/* =========================================================================================================================== */
/* ================                                            CCU                                            ================ */
/* =========================================================================================================================== */





/* =========================================================================================================================== */
/* ================                          Device Specific Peripheral Address Map                           ================ */
/* =========================================================================================================================== */

/* Peripheral and RAM base address */
/* =========================================================================================================================== */
/* ================                                            CCU                                            ================ */
/* =========================================================================================================================== */
#define V3S_CCU_BASE                          (0x01C20000UL)
#define V3S_PLL_CPU_CTRL_REG_BASE             (0x00000000UL + V3S_CCU_BASE)
#define V3S_PLL_VIDEO_CTRL_REG_BASE           (0x00000010UL + V3S_CCU_BASE)
#define V3S_PLL_PERIPH0_CTRL_REG_BASE         (0x00000028UL + V3S_CCU_BASE)
#define V3S_CPU_AXI_CFG_REG_BASE              (0x00000050UL + V3S_CCU_BASE)
#define V3S_AHB_APB0_CFG_REG_BASE             (0x00000054UL + V3S_CCU_BASE)
#define V3S_MBUS_CLK_REG_BASE                 (0x0000015CUL + V3S_CCU_BASE)

/* =========================================================================================================================== */
/* ================                                            GIC                                            ================ */
/* =========================================================================================================================== */
#define V3S_MP_PRIVATE_PERIPH_BASE            (0x01C80000UL)                                /*!< (Peripheral ) Base Address */
#define V3S_MP_GIC_DISTRIBUTOR_BASE           (0x00001000UL + V3S_MP_PRIVATE_PERIPH_BASE) /*!< (GIC DIST   ) Base Address */
#define V3S_MP_GIC_INTERFACE_BASE             (0x00002000UL + V3S_MP_PRIVATE_PERIPH_BASE) /*!< (GIC CPU IF ) Base Address */
#define GIC_DISTRIBUTOR_BASE                  V3S_MP_GIC_DISTRIBUTOR_BASE
#define GIC_INTERFACE_BASE                    V3S_MP_GIC_INTERFACE_BASE

/* Peripheral and RAM base address */
/* --------  Configuration of the Cortex-A7 Processor and Core Peripherals  ------- */
#define __CA_REV        0x0000U    /* Core revision r0p0                            */
#define __CORTEX_A           7U    /* Cortex-A7 Core                                */
#define __FPU_PRESENT        1U    /* FPU present                                   */
#define __GIC_PRESENT        1U    /* GIC present                                   */
#define __TIM_PRESENT        1U    /* TIM present                                   */
#define __L2C_PRESENT        0U    /* L2C present                                   */

#include "core_ca.h"
#include "system_ARMCA7.h"

/* =========================================================================================================================== */
/* ================                                  Peripheral declaration                                   ================ */
/* =========================================================================================================================== */




/** @addtogroup Exported_macros
  * @{
  */
#define SET_BIT(REG, BIT)     (*((volatile __typeof__(REG) *)REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)   (*((volatile __typeof__(REG) *)REG) &= ~(BIT))
#define READ_BIT(REG, BIT)    (*((volatile __typeof__(REG) *)REG) & (BIT))
#define CLEAR_REG(REG)        (*((volatile __typeof__(REG) *)REG) = (0x0))
#define WRITE_REG(REG, VAL)   (*((volatile __typeof__(REG) *)REG) = (VAL))
#define READ_REG(REG)         (*((volatile __typeof__(REG) *)REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))
#define POSITION_VAL(VAL)     (__CLZ(__RBIT(VAL)))

static inline void sdelay(int loops)
{
	__asm__ __volatile__ ("1:\n" "subs %0, %1, #1\n"
		"bne 1b":"=r" (loops):"0"(loops));
}

#ifdef __cplusplus
}
#endif

#endif  // __ARMCA7_H__
