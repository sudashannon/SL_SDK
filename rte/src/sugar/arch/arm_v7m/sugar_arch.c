/**
 * @file sugar_arch.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-01-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "../../../../inc/sugar/sugar_kernel.h"
#include "../../../../inc/sugar/sugar_queue.h"
#include "../../../../inc/sugar/sugar_scheduler.h"
#include "../../../../inc/middle_layer/rte_memory.h"
#include "../../../../inc/middle_layer/rte_log.h"
#include "../../../../inc/middle_layer/rte_atomic.h"

struct task_switch_info ctx_switch_info asm("CTX_SW_NFO") =
{
    .running_tcb = NULL,
    .next_tcb    = NULL,
};

extern void asm_arch_first_thread_restore(sugar_tcb_t *);
void arch_first_thread_restore(sugar_tcb_t *new_tcb_ptr)
{
#if defined(__NEWLIB__)
    ctx_switch_info.reent = &(new_tcb_ptr->port_priv.reent);
    __DMB();
#endif
    asm_arch_first_thread_restore(new_tcb_ptr);
}

/**
 * We do not perform the context switch directly. Instead we mark the new tcb
 * as should-be-running in ctx_switch_info and trigger a PendSv-interrupt.
 * The pend_sv_handler will be called when all other pending exceptions have
 * returned and perform the actual context switch.
 * This way we do not have to worry if we are being called from task or
 * interrupt context, which would mean messing with either main or thread
 * stack format.
 *
 * One difference to the other architectures is that execution flow will
 * actually continue in the old thread context until interrupts are enabled
 * again. From a thread context this should make no difference, as the context
 * switch will be performed as soon as the execution flow would return to the
 * calling thread. Unless, of course, the thread called atomSched() with
 * disabled interrupts, which it should not do anyways...
 */
void __attribute__((noinline))
arch_context_switch(sugar_tcb_t *old_tcb_ptr __attribute__((unused)), sugar_tcb_t *new_tcb_ptr)
{
    if(RTE_LIKELY(ctx_switch_info.running_tcb != NULL)){
        ctx_switch_info.next_tcb = new_tcb_ptr;
#if defined(__NEWLIB__)
        ctx_switch_info.reent = &(new_tcb_ptr->port_priv.reent);
#endif
        __DMB();

        SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    }
}

/**
 * Initialise a threads stack so it can be scheduled in by
 * archFirstThreadRestore or the pend_sv_handler.
 */
void arch_thread_context_init(sugar_tcb_t *tcb_ptr, void *stack_top,
                            void (*entry_point)(void *), void *user_param)
{
    struct isr_stack *isr_ctx;
    struct task_stack *tsk_ctx;

    /**
     * Do compile time verification for offsets used in _archFirstThreadRestore
     * and pend_sv_handler. If compilation aborts here, you will have to adjust
     * the offsets for struct task_switch_info's members in asm-offsets.h
     */
    OS_ASSERT(offsetof(struct task_switch_info, running_tcb) == ARCH_CTX_RUN_OFFSET);
    OS_ASSERT(offsetof(struct task_switch_info, next_tcb) == ARCH_CTX_NEXT_OFFSET);
#if defined(__NEWLIB__)
    OS_ASSERT(offsetof(struct task_switch_info, reent) == ARCH_CTX_REENT_OFFSET);
#endif
    OS_ASSERT(((uintptr_t)stack_top % ARCH_STACK_ALIGN_SIZE) == 0);

    /**
     * New threads will be scheduled from an exception handler, so we have to
     * set up an exception stack frame as well as task stack frame
     */
    isr_ctx = stack_top - sizeof(struct isr_stack);
    tsk_ctx = stack_top - sizeof(struct isr_stack) - sizeof(struct task_stack);
    OS_LOGI("[%s] tcb_ptr: %p stack_top: %p isr_ctx: %p tsk_ctx: %p entry_point: %p, user_param: 0x%x",
            __func__, tcb_ptr, stack_top, isr_ctx, tsk_ctx, entry_point, user_param);
    OS_LOGI("[%s] isr_ctx->r0: %p isr_ctx->psr: %p tsk_ctx->r4: %p isr_ctx->lr: %p",
            __func__, &isr_ctx->r0, &isr_ctx->psr, &tsk_ctx->r4, &isr_ctx->lr);
    /**
     * We use the exception return mechanism to jump to our thread_shell()
     * function and initialise the PSR to the default value (thumb state
     * flag set and nothing else)
     */
    isr_ctx->psr = 0x01000000;
    isr_ctx->pc  = (uint32_t) sugar_thread_body;

    /* initialise unused registers to silly value */
    isr_ctx->lr  = 0xEEEEEEEE;
    isr_ctx->r12 = 0xCCCCCCCC;
    isr_ctx->r3  = 0x33333333;
    isr_ctx->r2  = 0x22222222;
    isr_ctx->r1  = 0x11111111;
    isr_ctx->r0  = 0x00000000;

    /**
     * We use this special EXC_RETURN code to switch from main stack to our
     * thread stack on exception return
     */
    tsk_ctx->exc_ret = 0xFFFFFFFD;

    /* initialise unused registers to silly value */
    tsk_ctx->r11 = 0xBBBBBBBB;
    tsk_ctx->r10 = 0xAAAAAAAA;
    tsk_ctx->r9  = 0x99999999;
    tsk_ctx->r8  = 0x88888888;
    tsk_ctx->r7  = 0x77777777;
    tsk_ctx->r6  = 0x66666666;
    tsk_ctx->r5  = 0x55555555;
    tsk_ctx->r4  = 0x44444444;

    /**
     * Stack frames have been initialised, save it to the TCB. Also set
     * the thread's real entry point and param, so the thread shell knows
     * what function to call.
     */
    tcb_ptr->stack_ptr = (uintptr_t)tsk_ctx;
    tcb_ptr->entry_point = entry_point;
    tcb_ptr->user_param = user_param;

#if defined(__NEWLIB__)
    /**
     * Initialise thread's reentry context for newlib
     */
    _REENT_INIT_PTR(&(tcb_ptr->port_priv.reent));
#endif
}
