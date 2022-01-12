/**
 * @file sugar_kernel.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-01-12
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __SUGAR_KERNEL_H
#define __SUGAR_KERNEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../inc/middle_layer/rte.h"

/* Idle thread priority (lowest) */
#define SUGAR_IDLE_THREAD_PRIORITY    255
#define SUGAR_ENABLE_STACK_CHECKING   1

typedef struct sugar_tcb
{
    /*
     * Thread's current stack pointer. When a thread is scheduled
     * out the architecture port can save its stack pointer here.
     */
    uintptr_t stack_ptr;
    /* Thread's port specific private data */
#if defined (SUGAR_PORT_PRIV_STRUCT)
    SUGAR_PORT_PRIV_STRUCT;
#endif
    /* Thread entry point and parameter */
    void (*entry_point)(void *);
    void *user_param;
    /* Suspension data */
    bool if_suspended;              /* TRUE if task is currently suspended */
    bool if_terminated;             /* TRUE if task is being terminated (run to completion) */
    uint8_t suspend_wake_status;    /* Status returned to woken suspend calls */
    /* Thread priority (0-255) */
    uint8_t priority;
    /* Details used if thread stack-checking is required */
#if SUGAR_ENABLE_STACK_CHECKING
    uintptr_t stack_bottom;         /* Pointer to bottom of stack allocation */
    uint32_t stack_size;            /* Size of stack allocation in bytes */
#endif
} sugar_tcb_t;

typedef struct sugar_kernel {
    /**
     * This is the head of the queue of threads that are ready to run. It is
     * ordered by priority, with the higher priority threads coming first. Where
     * there are multiple threads of the same priority, the TCB (task control
     * block) pointers are FIFO-ordered.
     *
     * Dequeuing the head is a fast operation because the list is ordered.
     * Enqueuing may have to walk up to the end of the list. This means that
     * context-switch times depend on the number of threads on the ready queue,
     * but efficient use is made of available RAM on tiny systems by avoiding
     * priority tables etc. This scheme can be easily swapped out for other
     * scheduler schemes by replacing the TCB enqueue and dequeue functions.
     *
     * Once a thread is scheduled in, it is not present on the ready queue or any
     * other kernel queue while it is running. When scheduled out it will be
     * either placed back on the ready queue (if still ready), or will be suspended
     * on some OS primitive if no longer ready (e.g. on the suspended TCB queue
     * for a semaphore, or in the timer list if suspended on a timer delay).
     */
    void *ready_queue;
    /** This is a pointer to the TCB for the currently-running thread */
    sugar_tcb_t *curr_tcb_ptr;
    /** Storage for the idle thread's TCB */
    sugar_tcb_t *idle_tcb;
    /** Set to TRUE when OS is started and running threads */
    bool if_started;
    /* Number of nested interrupts */
    int32_t interrupts_count;
} sugar_kernel_t;

extern rte_error_t sugar_kernel_init(void *idle_thread_stack_bottom, uint32_t idle_thread_stack_size, bool if_idle_thread_stack_check);
extern void sugar_kernel_start(void);
extern sugar_tcb_t *sugar_tcb_ready_queue_pop_next(void);
extern sugar_tcb_t *sugar_tcb_ready_queue_pop_priority(uint8_t priority);
extern rte_error_t sugar_tcb_ready_queue_push_priority(sugar_tcb_t *tcb);
extern void sugar_sched(bool if_in_tickhandle);
extern sugar_tcb_t *sugar_context_get_current(void);
extern void sugar_interrupt_enter(void);
extern void sugar_interrupt_exit(uint8_t if_in_tickhandle);
extern sugar_tcb_t *sugar_thread_create(uint8_t priority,
                                void (*entry_point)(void *),
                                void *user_param,
                                void *stack_bottom,
                                uint32_t stack_size,
                                bool if_stack_check);
extern rte_error_t sugar_thread_check_stack(sugar_tcb_t *tcb_ptr, uint32_t *used_bytes, uint32_t *free_bytes);
#ifdef __cplusplus
}
#endif

#endif /* __SUGAR_KERNEL_H */
