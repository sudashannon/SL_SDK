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

#include "../middle_layer/rte.h"
#include "../middle_layer/rte_timer.h"
#include "../data_structure/ds_rbtree.h"
#include "sugar_arch_include.h"

/* Idle thread priority (lowest) */
#define SUGAR_IDLE_THREAD_PRIORITY    255
#define SUGAR_ENABLE_STACK_CHECKING   0
#define SUGAR_TCB_FIFO_CAPABILITY     64
#define SUGAR_TIMER_GROUP             0

#define THIS_MODULE LOG_STR(SUGAR)
#define OS_LOGF(...) LOG_FATAL(THIS_MODULE, __VA_ARGS__)
#define OS_LOGE(...) LOG_ERROR(THIS_MODULE, __VA_ARGS__)
#define OS_LOGI(...) LOG_INFO(THIS_MODULE, __VA_ARGS__)
#define OS_LOGW(...) LOG_WARN(THIS_MODULE, __VA_ARGS__)
#define OS_LOGD(...) LOG_DEBUG(THIS_MODULE, __VA_ARGS__)
#define OS_LOGV(...) LOG_VERBOSE(THIS_MODULE, __VA_ARGS__)
#define OS_ASSERT(v) LOG_ASSERT(THIS_MODULE, v)

#define SUGAR_STACK_ALIGN(p, a)     (typeof(p))((typeof(a))(p) & ~((a) - 1))

typedef struct sugar_tcb
{
    /*
     * Thread's current stack pointer. When a thread is scheduled
     * out the architecture port can save its stack pointer here.
     */
    uintptr_t stack_ptr;
    /* Thread's port specific private data */
#if defined (ARCH_PRIV_STRUCT_DEFINE)
    ARCH_PRIV_STRUCT_DEFINE;
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
    timer_id_t timer_id;
} sugar_tcb_t;

typedef struct sugar_queue {
    rbt_t *tcb_rbt;
    uint8_t highest_priority;
} sugar_queue_impl_t;

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
    sugar_queue_impl_t *ready_queue;
    /** This is a pointer to the TCB for the currently-running thread */
    sugar_tcb_t *current_tcb;
    /** Storage for the idle thread's TCB */
    sugar_tcb_t *idle_tcb;
    /** Set to TRUE when OS is started and running threads */
    bool if_started;
    /* Number of nested interrupts */
    int32_t interrupts_count;
    timer_group_id_t timer_group;
} sugar_kernel_t;

extern sugar_kernel_t sugar_kernel_handle;

extern void arch_first_thread_restore(sugar_tcb_t *new_tcb_ptr);
extern void arch_context_switch(sugar_tcb_t *old_tcb_ptr, sugar_tcb_t *new_tcb_ptr);
extern void arch_thread_context_init(sugar_tcb_t *tcb_ptr, void *stack_top,
                            void (*entry_point)(void *), void *user_param);

extern rte_error_t sugar_kernel_init(void *idle_thread_stack_bottom, uint32_t idle_thread_stack_size, bool if_idle_thread_stack_check);
extern void sugar_kernel_start(void);
extern sugar_tcb_t *sugar_kernel_get_current_tcb(void);

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