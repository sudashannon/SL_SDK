/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 2014 - 2021 SEGGER Microcontroller GmbH             *
*                                                                    *
*           www.segger.com     Support: support@segger.com           *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* conditions are met:                                                *
*                                                                    *
* - Redistributions of source code must retain the above copyright   *
*   notice, this list of conditions and the following disclaimer.    *
*                                                                    *
* - Neither the name of SEGGER Microcontroller GmbH                  *
*   nor the names of its contributors may be used to endorse or      *
*   promote products derived from this software without specific     *
*   prior written permission.                                        *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED.                                                        *
* IN NO EVENT SHALL SEGGER Microcontroller GmbH BE LIABLE FOR        *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : SEGGER_HardFaultHandler.c
Purpose : Generic SEGGER HardFault handler for Cortex-M
Literature:
  [1]  Analyzing HardFaults on Cortex-M CPUs (https://www.segger.com/downloads/appnotes/AN00016_AnalyzingHardFaultsOnCortexM.pdf)

Additional information:
  This HardFault handler enables user-friendly analysis of hard faults
  in debug configurations.
  If a release configuration requires a HardFault handler,
  a specific HardFault handler should be included instead,
  which for example issues a reset or turns on an error LED.
--------  END-OF-HEADER  ---------------------------------------------
*/

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define SCB_SHCSR  (*(volatile unsigned int*)  (0xE000ED24u))  // System Handler Control and State Register
#define SCB_MMFSR  (*(volatile unsigned char*) (0xE000ED28u))  // Memory Management Fault Status Register
#define SCB_BFSR   (*(volatile unsigned char*) (0xE000ED29u))  // Bus Fault Status Register
#define SCB_UFSR   (*(volatile unsigned short*)(0xE000ED2Au))  // Usage Fault Status Register
#define SCB_HFSR   (*(volatile unsigned int*)  (0xE000ED2Cu))  // Hard Fault Status Register
#define SCB_DFSR   (*(volatile unsigned int*)  (0xE000ED30u))  // Debug Fault Status Register
#define SCB_BFAR   (*(volatile unsigned int*)  (0xE000ED38u))  // Bus Fault Manage Address Register
#define SCB_AFSR   (*(volatile unsigned int*)  (0xE000ED3Cu))  // Auxiliary Fault Status Register

#ifndef   DEBUG         // Should be overwritten by project settings
  #define DEBUG   (0)   // in debug builds
#endif

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/
#ifdef __cplusplus
  extern "C" {
#endif
void HardFaultHandler(unsigned int* pStack);
#ifdef __cplusplus
  }
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static struct {
  struct {
    volatile unsigned int r0;            // Register R0
    volatile unsigned int r1;            // Register R1
    volatile unsigned int r2;            // Register R2
    volatile unsigned int r3;            // Register R3
    volatile unsigned int r12;           // Register R12
    volatile unsigned int lr;            // Link register
    volatile unsigned int pc;            // Program counter
    union {
      volatile unsigned int word;
      struct {
        unsigned int IPSR :  8;          // Interrupt Program Status register (IPSR)
        unsigned int EPSR : 19;          // Execution Program Status register (EPSR)
        unsigned int APSR :  5;          // Application Program Status register (APSR)
      } bits;
    } psr;                               // Program status register.
  } SavedRegs;

  union {
    volatile unsigned int word;
    struct {
      unsigned int MEMFAULTACT       :  1;   // [0]  Read as 1 if memory management fault is active
      unsigned int BUSFAULTACT       :  1;   // [1]  Read as 1 if bus fault exception is active
      unsigned int HARDFAULTACT      :  1;   // [2]  Read as 1 if hard fault exception is active (ARMv8-M)
      unsigned int USGFAULTACT       :  1;   // [3]  Read as 1 if usage fault exception is active
      unsigned int SECUREFAULTACT    :  1;   // [4]  Read as 1 if secure fault exception is active (ARMv8-M)
      unsigned int NMIACT            :  1;   // [5]  Read as 1 if NMI exception is active (ARMv8-M)
      unsigned int                   :  1;
      unsigned int SVCALLACT         :  1;   // [7]  Read as 1 if SVC exception is active
      unsigned int MONITORACT        :  1;   // [8]  Read as 1 if debug monitor exception is active
      unsigned int                   :  1;
      unsigned int PENDSVACT         :  1;   // [10] Read as 1 if PendSV exception is active
      unsigned int SYSTICKACT        :  1;   // [11] Read as 1 if SYSTICK exception is active
      unsigned int USGFAULTPENDED    :  1;   // [12] Usage fault pending; higher priority exception active
      unsigned int MEMFAULTPENDED    :  1;   // [13] Memory management fault pending; higher priority exception active
      unsigned int BUSFAULTPENDED    :  1;   // [14] Bus fault pending; higher priority exception active
      unsigned int SVCALLPENDED      :  1;   // [15] SVC pending; higher priority exception active
      unsigned int MEMFAULTENA       :  1;   // [16] Memory management fault exception enable
      unsigned int BUSFAULTENA       :  1;   // [17] Bus fault exception enable
      unsigned int USGFAULTENA       :  1;   // [18] Usage fault exception enable
      unsigned int SECUREFAULTENA    :  1;   // [19] Secure fault exception enable (ARMv8-M)
      unsigned int SECUREFAULTPENDED :  1;   // [20] Secure fault exception pending; higher priority exception active (ARMv8-M)
      unsigned int HARDFAULTPENDED   :  1;   // [21] Hard fault exception pending (ARMv8-M)
      unsigned int                   : 10;
    } bits;
  } shcsr;                                   // System Handler Control and State Register (0xE000ED24)

  union {
    volatile unsigned char byte;
    struct {
      unsigned int IACCVIOL    :  1;     // [0] Instruction access violation
      unsigned int DACCVIOL    :  1;     // [1] Data access violation
      unsigned int             :  1;
      unsigned int MUNSTKERR   :  1;     // [3] Unstacking error
      unsigned int MSTKERR     :  1;     // [4] Stacking error
      unsigned int MLSPERR     :  1;     // [5] MemManage fault during FP lazy state preservation
      unsigned int             :  1;
      unsigned int MMARVALID   :  1;     // [7] Indicates the MMAR is valid
      unsigned int             : 24;
    } bits;
  } mmfsr;                               // Memory Management Fault Status Register (0xE000ED28)

  union {
    volatile unsigned char byte;
    struct {
      unsigned int IBUSERR    :  1;      // [0] Instruction access violation
      unsigned int PRECISERR  :  1;      // [1] Precise data access violation
      unsigned int IMPREISERR :  1;      // [2] Imprecise data access violation
      unsigned int UNSTKERR   :  1;      // [3] Unstacking error
      unsigned int STKERR     :  1;      // [4] Stacking error
      unsigned int LSPERR     :  1;      // [5] Bus fault during FP lazy state preservation
      unsigned int            :  1;
      unsigned int BFARVALID  :  1;      // [7] Indicates BFAR is valid
      unsigned int            : 24;
    } bits;
  } bfsr;                                // Bus Fault Status Register (0xE000ED29)
  volatile unsigned int bfar;            // Bus Fault Manage Address Register (0xE000ED38)

  union {
    volatile unsigned short halfword;
    struct {
      unsigned int UNDEFINSTR :  1;      // [0] Attempts to execute an undefined instruction
      unsigned int INVSTATE   :  1;      // [1] Attempts to switch to an invalid state (e.g., ARM)
      unsigned int INVPC      :  1;      // [2] Attempts to do an exception with a bad value in the EXC_RETURN number
      unsigned int NOCP       :  1;      // [3] Attempts to execute a coprocessor instruction
      unsigned int STKOF      :  1;      // [4] Indicates whether a stack overflow error has occurred (ARMv8-M)
      unsigned int            :  3;
      unsigned int UNALIGNED  :  1;      // [8] Indicates that an unaligned access fault has taken place
      unsigned int DIVBYZERO  :  1;      // [9] Indicates a divide by zero has taken place (can be set only if DIV_0_TRP is set)
      unsigned int            : 22;
    } bits;
  } ufsr;                                // Usage Fault Status Register (0xE000ED2A)

  union {
    volatile unsigned int word;
    struct {
      unsigned int             :  1;
      unsigned int VECTTBL     :  1;     // [1] Indicates hard fault is caused by failed vector fetch
      unsigned int             : 28;
      unsigned int FORCED      :  1;     // [30] Indicates hard fault is taken because of bus fault/memory management fault/usage fault
      unsigned int DEBUGEVT    :  1;     // [31] Indicates hard fault is triggered by debug event
    } bits;
  } hfsr;                                // Hard Fault Status Register (0xE000ED2C)

  union {
    volatile unsigned int word;
    struct {
      unsigned int HALTED   :  1;        // [0] Halt requested in NVIC
      unsigned int BKPT     :  1;        // [1] BKPT instruction executed
      unsigned int DWTTRAP  :  1;        // [2] DWT match occurred
      unsigned int VCATCH   :  1;        // [3] Vector fetch occurred
      unsigned int EXTERNAL :  1;        // [4] EDBGRQ signal asserted
      unsigned int PMU      :  1;        // [5] PMU counter overflow event has occurred
      unsigned int          : 26;
    } bits;
  } dfsr;                                // Debug Fault Status Register (0xE000ED30)

  volatile unsigned int afsr;            // Auxiliary Fault Status Register (0xE000ED3C), Vendor controlled (optional)
} HardFaultRegs;

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       HardFaultHandler()
*
*  Function description
*    C part of the hard fault handler which is called by the assembler
*    function HardFault_Handler
*/
void HardFaultHandler(unsigned int* pStack) {
  extern void shell_printf(const char * restrict format, ...);
  //
  // In case we received a hard fault because of a breakpoint instruction, we return.
  // This may happen when using semihosting for printf outputs and no debugger is connected,
  // i.e. when running a "Debug" configuration in release mode.
  //
  if (SCB_HFSR & (1u << 31)) {
    SCB_HFSR |=  (1u << 31);      // Reset Hard Fault status
    *(pStack + 6u) += 2u;         // PC is located on stack at SP + 24 bytes. Increment PC by 2 to skip break instruction.
    return;                       // Return to interrupted application
  }
  shell_printf("hard fault happens!");
  //
  // Read NVIC registers
  //
  HardFaultRegs.shcsr.word    = SCB_SHCSR;  // System Handler Control and State Register
  HardFaultRegs.mmfsr.byte    = SCB_MMFSR;  // Memory Fault Status Register
  HardFaultRegs.bfsr.byte     = SCB_BFSR;   // Bus Fault Status Register
  HardFaultRegs.bfar          = SCB_BFAR;   // Bus Fault Manage Address Register
  HardFaultRegs.ufsr.halfword = SCB_UFSR;   // Usage Fault Status Register
  HardFaultRegs.hfsr.word     = SCB_HFSR;   // Hard Fault Status Register
  HardFaultRegs.dfsr.word     = SCB_DFSR;   // Debug Fault Status Register
  HardFaultRegs.afsr          = SCB_AFSR;   // Auxiliary Fault Status Register
  if (HardFaultRegs.mmfsr.byte) {
    while(1);
  }
  //
  // Read saved registers from the stack.
  //
  HardFaultRegs.SavedRegs.r0       = pStack[0];  // Register R0
  HardFaultRegs.SavedRegs.r1       = pStack[1];  // Register R1
  HardFaultRegs.SavedRegs.r2       = pStack[2];  // Register R2
  HardFaultRegs.SavedRegs.r3       = pStack[3];  // Register R3
  HardFaultRegs.SavedRegs.r12      = pStack[4];  // Register R12
  HardFaultRegs.SavedRegs.lr       = pStack[5];  // Link register LR
  HardFaultRegs.SavedRegs.pc       = pStack[6];  // Program counter PC
  HardFaultRegs.SavedRegs.psr.word = pStack[7];  // Program status word PSR
  //
  // Halt execution
  //
  while(1);
}

/*************************** End of file ****************************/
