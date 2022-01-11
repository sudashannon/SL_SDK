/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 2014 - 2021 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* condition is met:                                                  *
*                                                                    *
* - Redistributions of source code must retain the above copyright   *
*   notice, this condition and the following disclaimer.             *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
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

-------------------------- END-OF-HEADER -----------------------------

File      : ARM_Startup.s
Purpose   : Generic startup and exception handlers for ARM devices.

Additional information:
  Preprocessor Definitions
    __FLASH_BUILD
      If defined,
        additional code can be executed in Reset_Handler,
        which might be needed to run from Flash but not to run from RAM.

    __SUPERVISOR_START
      If defined,
        application starts in supervisor mode instead of system mode.

    __SOFTFP__
      Defined by the build system.
      If not defined, the FPU is enabled for floating point operations.
*/

        .syntax unified  
        
/*********************************************************************
*
*       Macros
*
**********************************************************************
*/
//
// Declare an interrupt handler
//
.macro ISR_HANDLER Name=
        //
        // Insert vector in vector table
        //
        .section .vectors, "ax"
        ldr     PC, =\Name
        //
        // Insert dummy handler in init section
        //
        .section .init.\Name, "ax"
        .code 32
        .type \Name, function
        .weak \Name
        .balign 4
\Name:
        1: b 1b   // Endless loop
        END_FUNC \Name
.endm

//
// Place a reserved isr handler
//
.macro ISR_RESERVED
        .section .vectors, "ax"
        nop
.endm

//
// Mark the end of a function and calculate its size
//
.macro END_FUNC name
        .size \name,.-\name
.endm

#if defined(__ARM_ARCH_4T__)
.macro blx reg
  mov lr, pc
  bx \reg
.endm
#endif 

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/
/*********************************************************************
*
*  Setup of the exception table
*
*/
        .section .vectors, "ax"
        .code 32
        .balign 4
        .global _vectors
_vectors:
        ldr     PC, =Reset_Handler
        ISR_HANDLER undef_handler
        ISR_HANDLER swi_handler
        ISR_HANDLER pabort_handler
        ISR_HANDLER dabort_handler
        ISR_RESERVED
        ISR_HANDLER irq_handler
        ISR_HANDLER fiq_handler

        .section .vectors, "ax"
        .size _vectors, .-_vectors
_vectors_end:

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
/*********************************************************************
*
*       Reset_Handler
*
*  Function description
*    Exception handler for reset.
*    Generic bringup of a system.
*/
        .section .init.Reset_Handler, "ax"
        .code 32
        .balign 4
        .global reset_handler
        .global Reset_Handler
        .equ reset_handler, Reset_Handler
        .type Reset_Handler, function
Reset_Handler:
        //
        // Setup Stacks
        //
        mrs     R0, CPSR
        bic     R0, R0, #0x1F
        //
        orr     R1, R0, #0x1B           // Setup CPSR for undefined mode
        msr     CPSR_cxsf, R1           // Put CPU in mode
        ldr     SP, =__stack_und_end__  // Load mode's stack pointer
        bic     SP, SP, #0x7            // Align stack pointer if necessary
        orr     R1, R0, #0x17           // Setup CPSR for abort mode
        msr     CPSR_cxsf, R1           // Put CPU in mode
        ldr     SP, =__stack_abt_end__  // Load mode's stack pointer
        bic     SP, SP, #0x7            // Align stack pointer if necessary
        orr     R1, R0, #0x12           // Setup CPSR for IRQ mode
        msr     CPSR_cxsf, R1           // Put CPU in mode
        ldr     SP, =__stack_irq_end__  // Load mode's stack pointer
        bic     SP, SP, #0x7            // Align stack pointer if necessary
        orr     R1, R0, #0x11           // Setup CPSR for FIQ mode
        msr     CPSR_cxsf, R1           // Put CPU in mode
        ldr     SP, =__stack_fiq_end__  // Load mode's stack pointer
        bic     SP, SP, #0x7            // Align stack pointer if necessary
        orr     R1, R0, #0x13           // Setup CPSR for supervisor mode
        msr     CPSR_cxsf, R1           // Put CPU in mode
        ldr     SP, =__stack_svc_end__  // Load mode's stack pointer
        bic     SP, SP, #0x7            // Align stack pointer if necessary
#ifdef __SUPERVISOR_START
        //
        // Application to be started in supervisor mode
        // Setup user mode stack
        //
        ldr     R1, =__stack_end__
        bic     R1, R1, #0x7            // Align stack pointer if necessary
        mov     R2, SP                  // Load current stack pointer to R2
        stmfd   R2!, {R1}               // Store user mode SP on supervisor stack
        ldmfd   R2, {SP}^               // Load user mode stack pointer (without switching to user mode)
#else
        //
        // Application to be started in system mode
        // Switch mode and setup stack
        //
        orr     R1, R0, #0x1F           // Setup CPSR for system mode
        msr     CPSR_cxsf, R1           // Put CPU in mode
        ldr     SP, =__stack_end__      // Load system mode stack pointer
        bic     SP, SP, #0x7            // Align stack pointer if necessary
#endif
  /****************************************************************************
   * TODO: Configure target system.                                           *
   ****************************************************************************/
#ifdef __FLASH_BUILD
  /****************************************************************************
   * TODO: Put any FLASH build configuation specific code here                *
   ****************************************************************************/
#endif
#if !defined(__SOFTFP__)
        //
        // Enable CP11, CP10 and VFP
        //
        mrc     P15, #0x00, R0, C1, C0, #0x02
        orr     R0, R0, #0x00F00000
        mcr     P15, #0x00, R0, C1, C0, #0x02
        mov     R0, #0x40000000
        fmxr    FPEXC, r0
#endif
        //
        // Jump to runtime initialization, which calls main().
        //
        b       _start
END_FUNC Reset_Handler

/*************************** End of file ****************************/
