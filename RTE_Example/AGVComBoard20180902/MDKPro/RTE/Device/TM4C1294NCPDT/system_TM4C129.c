/**************************************************************************//**
 * @file     system_TM4C129.c
 * @brief    CMSIS Device System Source File for
 *           TI TIVA TM4C129 Snowflake Device Series
 * @version  V1.00
 * @date     15. May 2013
 *
 * @note
 *
 *                                                             modified by Shannon
 ******************************************************************************/

#include <stdint.h>
#include "RTE_Include.h"
#include "TM4C129.h"
#include "sysctl.h"
#include "interrupt.h"
/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define __SYSTEM_CLOCK    (120000000ul)


/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = 0; /*!< System Clock Frequency (Core Clock)*/


/*----------------------------------------------------------------------------
  Clock functions
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)            /* Get Core Clock Frequency      */
{
  /* TODO: Updates required to fully work with TM4C129 series devices */
	//
	// Make sure the main oscillator is enabled because this is required by
	// the PHY.  The system must have a 25MHz crystal attached to the OSC
	// pins. The SYSCTL_MOSC_HIGHFREQ parameter is used when the crystal
	// frequency is 10MHz or higher.
	//
	SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);
	//
	// Set the clocking to run directly from the crystal at 120MHz.
	//
  SystemCoreClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480), __SYSTEM_CLOCK); 
	//
	// Enable peripherals to operate when CPU is in sleep.
	//
  SysCtlPeripheralClockGating(true);
	IntMasterEnable();
}

/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System.
 */
void SystemInit (void)
{
  /* TODO: Updates required to fully work with TM4C129 series devices */
#if (__FPU_USED == 1)
	SCB->CPACR |= ((3UL << 10*2) |                 /* set CP10 Full Access */
								 (3UL << 11*2)  );               /* set CP11 Full Access */
#endif

#ifdef UNALIGNED_SUPPORT_DISABLE
  SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif

  SystemCoreClock = __SYSTEM_CLOCK;

}
