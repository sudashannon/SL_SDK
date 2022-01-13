/**
 * @file sugar_arch.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-01-13
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __SUGAR_ARCH_H
#define __SUGAR_ARCH_H

#include "stm32h7xx_hal.h"
#include "sugar_offset.h"

/* Size of each stack entry / stack alignment size (4 bytes on Cortex-M without FPU) */
#define ARCH_STACK_ALIGN_SIZE    sizeof(uint32_t)

/** @brief Cortex M Mask interrupts
 *
 * This function switches the mask of the interrupts. If mask is true, the
 * interrupts will be disabled. The result of this function can be used for
 * restoring previous state of the mask.
 *
 * @param[in] mask uint32_t New state of the interrupt mask
 * @returns uint32_t old state of the interrupt mask
 */
static inline uint32_t port_mask_interrupts(uint32_t mask)
{
	register uint32_t old;
	__asm__ __volatile__("MRS %0, PRIMASK": "=r"(old));
	__asm__ __volatile__("": : : "memory");
	__asm__ __volatile__("MSR PRIMASK, %0": : "r"(mask));
	return old;
}


/**
 * Critical region protection: this should disable interrupts
 * to protect OS data structures during modification. It must
 * allow nested calls, which means that interrupts should only
 * be re-enabled when the outer CRITICAL_END() is reached.
 */
#define arch_critical_store()       bool __irq_flags
#define arch_critical_enter()       __irq_flags = port_mask_interrupts(true)
#define arch_critical_exit()        (void) port_mask_interrupts(__irq_flags)

/**
 * When using newlib, define port private field in atom_tcb to be a
 * struct _reent.
 */
#if defined(__NEWLIB__)
struct cortex_port_priv {
        struct _reent reent;
};
#define ARCH_PRIV_STRUCT_DEFINE    struct cortex_port_priv port_priv
#endif

/**
 * context saved automagically by exception entry
 */
struct isr_stack {
        uint32_t r0;
        uint32_t r1;
        uint32_t r2;
        uint32_t r3;
        uint32_t r12;
        uint32_t lr;
        uint32_t pc;
        uint32_t psr;
} __attribute__((packed));

struct isr_fpu_stack {
        uint32_t s0;
        uint32_t s1;
        uint32_t s2;
        uint32_t s3;
        uint32_t s4;
        uint32_t s5;
        uint32_t s6;
        uint32_t s7;
        uint32_t s8;
        uint32_t s9;
        uint32_t s10;
        uint32_t s11;
        uint32_t s12;
        uint32_t s13;
        uint32_t s14;
        uint32_t s15;
        uint32_t fpscr;
} __attribute__((packed));

/**
 *  remaining context saved by task switch ISR
 */
struct task_stack {
        uint32_t r4;
        uint32_t r5;
        uint32_t r6;
        uint32_t r7;
        uint32_t r8;
        uint32_t r9;
        uint32_t r10;
        uint32_t r11;
        uint32_t exc_ret;
} __attribute__((packed));

struct task_fpu_stack {
        uint32_t s16;
        uint32_t s17;
        uint32_t s18;
        uint32_t s19;
        uint32_t s20;
        uint32_t s21;
        uint32_t s22;
        uint32_t s23;
        uint32_t s24;
        uint32_t s25;
        uint32_t s26;
        uint32_t s27;
        uint32_t s28;
        uint32_t s29;
        uint32_t s30;
        uint32_t s31;
} __attribute__((packed));

/**
 * Info needed by pend_sv_handler used for delayed task switching.
 * Running_tcb is a pointer to the TCB currently running (gosh, really?!)
 * next_tcb is a pointer to a TCB that should be running.
 * archContextSwitch() will update next_tcb and trigger a pend_sv. The
 * pend_sv_handler will be called as soon as all other ISRs have returned,
 * do the real context switch and update running_tcb.
 */
struct sugar_tcb;
struct task_switch_info {
    volatile struct sugar_tcb *running_tcb;
    volatile struct sugar_tcb *next_tcb;
#if defined(__NEWLIB__)
    struct _reent *reent;
#endif
} __attribute__((packed));

#endif
