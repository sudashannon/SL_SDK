/**
 * @file sugar_kernel.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-01-12
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "sugar_kernel.h"
#include "../../inc/middle_layer/rte_memory.h"
#include "../../inc/middle_layer/rte_log.h"
#include "../../inc/middle_layer/rte_atomic.h"
#include "../../inc/data_structure/ds_rbtree.h"
#include "../../inc/data_structure/ds_vector.h"

#define THIS_MODULE LOG_STR(SUGAR)
#define OS_LOGF(...) LOG_FATAL(THIS_MODULE, __VA_ARGS__)
#define OS_LOGE(...) LOG_ERROR(THIS_MODULE, __VA_ARGS__)
#define OS_LOGI(...) LOG_INFO(THIS_MODULE, __VA_ARGS__)
#define OS_LOGW(...) LOG_WARN(THIS_MODULE, __VA_ARGS__)
#define OS_LOGD(...) LOG_DEBUG(THIS_MODULE, __VA_ARGS__)
#define OS_LOGV(...) LOG_VERBOSE(THIS_MODULE, __VA_ARGS__)
#define OS_ASSERT(v) LOG_ASSERT(THIS_MODULE, v)

/** Bytecode to fill thread stacks with for stack-checking purposes */
#define STACK_CHECK_BYTE    0x5A

MEM_ALIGN_NBYTES (__attribute__((section (".os_memory"))) uint8_t os_buffer[128 * 1024], MEM_BLOCK_ALIGN) = {0};

static sugar_kernel_t sugar_kernel_handle = {0};

/**
 * \b sugar_idle_thread
 *
 * Entry point for idle thread.
 *
 * This thread must always be present, and will be the thread executed when
 * no other threads are ready to run. It must not call any library routines
 * which would cause it to block.
 *
 * @param[in] param Unused (optional thread entry parameter)
 *
 * @return None
 */
__attribute__((weak)) void sugar_idle_thread(void *param)
{
    /* Compiler warning  */
    param = param;
    /* Loop forever */
    while (1) {
        /** \todo Provide user idle hooks*/
    }
}

/**
 * \b sugar_kernel_init
 *
 * Initialise the Sugar Kernel.
 *
 * Must be called before any application code uses the sugar kernel's APIs. No
 * threads are actually started until the application calls sugar_kernel_start().
 *
 * Callers can provide a pointer to some storage for the idle thread stack.
 * If the callers didn't give the idle thread stack, it will use the memory API
 * to allocate from the OS memory bank.
 * The caller is responsible for calculating the appropriate space required
 * for their particular architecture.
 *
 * Applications should use the following initialisation sequence:
 *
 * \li Call sugar_kernel_init() before calling any other sugar kernel's APIs
 * \li Arrange for a timer to call sugar_kernel_tick_handle() periodically
 * \li Create one or more application threads using sugar_thread_create()
 * \li Start the OS using sugar_kernel_start(). At this point the highest
 *     priority application thread created will be started.
 *
 * Interrupts should be disabled until the first thread restore is complete,
 * to avoid any complications due to interrupts occurring while crucial
 * operating system facilities are being initialised. They are normally
 * enabled by the arch_first_thread_restore() routine in the architecture port.
 *
 * @param[in] idle_thread_stack_bottom Ptr to bottom of stack for idle thread
 * @param[in] idle_thread_stack_size Size of idle thread stack in bytes
 * @param[in] idle_thread_stack_check TRUE if stack checking required on idle thread
 *
 * @retval RTE_SUCCESS Success
 * @retval Other Initialisation error
 */
rte_error_t sugar_kernel_init(void *idle_thread_stack_bottom, uint32_t idle_thread_stack_size, bool if_idle_thread_stack_check)
{
    rte_error_t retval = RTE_ERR_UNDEFINE;
    memory_pool(BANK_DEFAULT, NULL, os_buffer, sizeof(os_buffer));
    if (idle_thread_stack_bottom == NULL &&
        idle_thread_stack_size == 0) {
        OS_LOGF("plz input stack size for idle thread!");
        retval = RTE_ERR_PARAM;
        goto end;
    }
    if (idle_thread_stack_bottom == NULL &&
        idle_thread_stack_size) {
        idle_thread_stack_bottom = memory_calloc(BANK_OS, idle_thread_stack_size);
        OS_ASSERT(idle_thread_stack_bottom);
    }
    sugar_kernel_handle.ready_queue = NULL;
    sugar_kernel_handle.curr_tcb_ptr = NULL;
    sugar_kernel_handle.if_started = false;
    /* Create the idle thread */
    sugar_kernel_handle.idle_tcb = sugar_thread_create(
                                    SUGAR_IDLE_THREAD_PRIORITY,
                                    sugar_idle_thread,
                                    NULL,
                                    idle_thread_stack_bottom,
                                    idle_thread_stack_size,
                                    if_idle_thread_stack_check);
end:
    return retval;
}

/**
 * \b sugar_kernel_start
 *
 * Start the highest priority thread running.
 *
 * This function must be called after all OS initialisation is complete, and
 * at least one application thread has been created. It will start executing
 * the highest priority thread created (or first created if multiple threads
 * share the highest priority).
 *
 * Interrupts must still be disabled at this point. They must only be enabled
 * when the first thread is restored and started by the architecture port's
 * arch_first_thread_restore() routine.
 *
 * @return None
 */
void sugar_kernel_start(void)
{
    sugar_tcb_t *new_tcb = NULL;
    /**
     * Enable the OS started flag. This stops routines like atomThreadCreate()
     * attempting to schedule in a newly-created thread until the scheduler is
     * up and running.
     */
    sugar_kernel_handle.if_started = true;
    /**
     * Application calls to atomThreadCreate() should have added at least one
     * thread to the ready queue. Take the highest priority one off and
     * schedule it in. If no threads were created, the OS will simply start
     * the idle thread (the lowest priority allowed to be scheduled is the
     * idle thread's priority, 255).
     */
    new_tcb = sugar_tcb_ready_queue_pop_next();
    if (new_tcb) {
        /* Set the new currently-running thread pointer */
        sugar_kernel_handle.curr_tcb_ptr = new_tcb;
        /* Restore and run the first thread */
        arch_first_thread_restore(new_tcb);
        /* Never returns to here, execution shifts to new thread context */
    } else {
        /* No ready threads were found. sugar_kernel_start() probably was not called */
        OS_LOGE("No valid threads were found when tried to start the kernel!");
    }
}

sugar_tcb_t *sugar_tcb_ready_queue_pop_next(void)
{
    return NULL;
}

sugar_tcb_t *sugar_tcb_ready_queue_pop_priority(uint8_t priority)
{
    return NULL;
}

rte_error_t sugar_tcb_ready_queue_push_priority(sugar_tcb_t *tcb)
{
    return RTE_SUCCESS;
}



/**
 * \b sugar_context_switch
 *
 * This is an internal function not for use by application code.
 *
 * The function is called by the scheduler to perform a context switch.
 * Execution will switch to the new thread's context, therefore the
 * function doesn't actually return until the old thread is scheduled
 * back in.
 *
 * @param[in] old_tcb Pointer to TCB for thread being scheduled out
 * @param[in] new_tcb Pointer to TCB for thread being scheduled in
 *
 * @return None
 */
static void sugar_context_switch(sugar_tcb_t *old_tcb, sugar_tcb_t *new_tcb)
{
    /**
     * The context switch will shift execution to a different thread. The
     * new thread is now ready to run so clear its suspend status in
     * preparation for it waking up.
     */
    new_tcb->if_suspended = false;
    /**
     * Check if the new thread is actually the current one, in which
     * case we don't need to do any context switch. This can happen
     * if a thread goes into suspend but is unsuspended again before
     * it is fully scheduled out.
     */
    if (old_tcb != new_tcb) {
        /* Set the new currently-running thread pointer */
        sugar_kernel_handle.curr_tcb_ptr = new_tcb;
        /* Call the architecture-specific context switch */
        arch_context_switch(old_tcb, new_tcb);
    }
}

/**
 * \b sugar_sched
 *
 * This is an internal function not for use by application code.
 *
 * This is the main scheduler routine. It is called by the various OS
 * library routines to check if any threads should be scheduled in now.
 * If so, the context will be switched from the current thread to the
 * new one.
 *
 * The scheduler is priority-based with round-robin performed on threads
 * with the same priority. Round-robin is only performed on timer ticks
 * however. During reschedules caused by an OS operation (e.g. after
 * giving or taking a semaphore) we only allow the scheduling in of
 * threads with higher priority than current priority. On timer ticks we
 * also allow the scheduling of same-priority threads - in that case we
 * schedule in the head of the ready list for that priority and put the
 * current thread at the tail.
 *
 * @param[in] timer_tick Should be TRUE when called from the system tick
 *
 * @return None
 */
void sugar_sched(bool if_in_tickhandle)
{
    arch_critical_store();
    sugar_tcb_t *new_tcb = NULL;
    int16_t lowest_pri;
    /**
     * Check the OS has actually started. As long as the proper initialisation
     * sequence is followed there should be no calls here until the OS is
     * started, but we check to handle badly-behaved ports.
     */
    if (sugar_kernel_handle.if_started == false) {
        /* Don't schedule anything in until the OS is started */
        return;
    }
    /* Enter critical section */
    arch_critical_enter();
    /**
     * If the current thread is going into suspension or is being
     * terminated (run to completion), then unconditionally dequeue
     * the next thread for execution.
     */
    if ((sugar_kernel_handle.curr_tcb_ptr->if_suspended == true) ||
        (sugar_kernel_handle.curr_tcb_ptr->if_terminated == true)) {
        /**
         * Dequeue the next ready to run thread. There will always be
         * at least the idle thread waiting. Note that this could
         * actually be the suspending thread if it was unsuspended
         * before the scheduler was called.
         */
        new_tcb = sugar_tcb_ready_queue_pop_next();

        /**
         * Don't need to add the current thread to any queue because
         * it was suspended by another OS mechanism and will be
         * sitting on a suspend queue or similar within one of the OS
         * primitive libraries (e.g. semaphore).
         */

        /* Switch to the new thread */
        sugar_context_switch(sugar_kernel_handle.curr_tcb_ptr, new_tcb);
    }

    /**
     * Otherwise the current thread is still ready, but check
     * if any other threads are ready.
     */
    else
    {
        /* Calculate which priority is allowed to be scheduled in */
        if (if_in_tickhandle == true) {
            /* Same priority or higher threads can preempt */
            lowest_pri = (int16_t)sugar_kernel_handle.curr_tcb_ptr->priority;
        } else if (sugar_kernel_handle.curr_tcb_ptr->priority > 0) {
            /* Only higher priority threads can preempt, invalid for 0 (highest) */
            lowest_pri = (int16_t)(sugar_kernel_handle.curr_tcb_ptr->priority - 1);
        } else {
            /**
             * Current priority is already highest (0), don't allow preempt by
             * threads of any priority because this is not a time-slice.
             */
            lowest_pri = -1;
        }

        /* Check if a reschedule is allowed */
        if (lowest_pri >= 0) {
            /* Check for a thread at the given minimum priority level or higher */
            new_tcb = sugar_tcb_ready_queue_pop_priority((uint8_t)lowest_pri);
            /* If a thread was found, schedule it in */
            if (new_tcb) {
                /* Add the current thread to the ready queue */
                (void)sugar_tcb_ready_queue_push_priority(sugar_kernel_handle.curr_tcb_ptr);

                /* Switch to the new thread */
                sugar_context_switch (sugar_kernel_handle.curr_tcb_ptr, new_tcb);
            }
        }
    }
    /* Exit critical section */
    arch_critical_exit();
}

/**
 * \b sugar_context_get_current
 *
 * Get the current thread context.
 *
 * Returns a pointer to the current thread's TCB, or NULL if not in
 * thread-context (in interrupt context).
 *
 * @retval Pointer to current thread's TCB, NULL if in interrupt context
 */
sugar_tcb_t *sugar_context_get_current(void)
{
    /* Return the current thread's TCB or NULL if in interrupt context */
    if (ATOMIC_READ(&sugar_kernel_handle.interrupts_count) == 0) {
        return (sugar_kernel_handle.curr_tcb_ptr);
    } else {
        return (NULL);
    }
}

/**
 * \b sugar_interrupt_enter
 *
 * Interrupt handler entry routine.
 *
 * Must be called at the start of any interrupt handlers that may
 * call an OS primitive and make a thread ready.
 *
 * @return None
 */
void sugar_interrupt_enter(void)
{
    /* Increment the interrupt count */
    ATOMIC_INCREMENT(&sugar_kernel_handle.interrupts_count);
}


/**
 * \b sugar_interrupt_exit
 *
 * Interrupt handler exit routine.
 *
 * Must be called at the end of any interrupt handlers that may
 * call an OS primitive and make a thread ready.
 *
 * This is responsible for calling the scheduler at the end of
 * interrupt handlers to determine whether a new thread has now
 * been made ready and should be scheduled in.
 *
 * @param timer_tick TRUE if this is a timer tick
 *
 * @return None
 */
void sugar_interrupt_exit(uint8_t if_in_tickhandle)
{
    /* Decrement the interrupt count */
    ATOMIC_DECREMENT(&sugar_kernel_handle.interrupts_count);
    /* Call the scheduler */
    sugar_sched(if_in_tickhandle);
}

/**
 * \b sugar_thread_create
 *
 * Creates and starts a new thread.
 *
 * TODO: New tcb structure storage can be obtained
 * from an internal TCB free list to speed up.
 *
 * The function puts the new thread on the ready queue and calls the
 * scheduler. If the priority is higher than the current priority, then the
 * new thread may be scheduled in before the function returns.
 *
 * Optionally prefills the thread stack with a known value to enable stack
 * usage checking (if the SUGAR_ENABLE_STACK_CHECKING macro is defined and
 * stack_check parameter is set to TRUE).
 *
 * @param[in] priority Priority of the thread (0 to 255)
 * @param[in] entry_point Thread entry point
 * @param[in] user_param Parameter passed to thread entry point
 * @param[in] stack_bottom Bottom of the stack area
 * @param[in] stack_size Size of the stack area in bytes
 * @param[in] stack_check TRUE to enable stack checking for this thread
 *
 * @retval Not NULL Success
 */
sugar_tcb_t *sugar_thread_create(uint8_t priority,
                                void (*entry_point)(void *),
                                void *user_param,
                                void *stack_bottom,
                                uint32_t stack_size,
                                bool if_stack_check)
{
    arch_critical_store();
    rte_error_t status = RTE_ERR_UNDEFINE;
    uint8_t *stack_top = NULL;
    sugar_tcb_t *tcb_ptr = NULL;
#if SUGAR_ENABLE_STACK_CHECKING
	int32_t count;
#endif

    if ((entry_point == NULL) ||
        ((stack_bottom == NULL) &&
        (stack_size == 0))) {
        /* Bad parameters */
        OS_LOGF("Wrong parameters when create new thread!");
        return NULL;
    } else {
        if (stack_bottom) {
            tcb_ptr = (sugar_tcb_t *)stack_bottom;
            stack_size = stack_size - sizeof(sugar_tcb_t);
            stack_bottom = (uint8_t *)stack_bottom + sizeof(sugar_tcb_t);
        } else {
            tcb_ptr = memory_calloc(BANK_OS, sizeof(sugar_tcb_t) + stack_size);
            stack_bottom = (uint8_t *)tcb_ptr + sizeof(sugar_tcb_t);
        }
        /* Set up the TCB initial values */
        tcb_ptr->if_suspended = false;
        tcb_ptr->if_terminated = false;
        tcb_ptr->priority = priority;
        /**
         * Store the thread entry point and parameter in the TCB. This may
         * not be necessary for all architecture ports if they put all of
         * this information in the initial thread stack.
         */
        tcb_ptr->entry_point = entry_point;
        tcb_ptr->user_param = user_param;

        /**
         * Calculate a pointer to the topmost stack entry, suitably aligned
         * for the architecture. This may discard the top few bytes if the
         * stack size is not a multiple of the stack entry/alignment size.
         */
        stack_top = (uint8_t *)stack_bottom + (stack_size & ~(ARCH_STACK_ALIGN_SIZE - 1)) - ARCH_STACK_ALIGN_SIZE;

        /**
         * Additional processing only required if stack-checking is
         * enabled. Incurs a slight overhead on each thread creation
         * and uses some additional storage in the TCB, but can be
         * compiled out if not desired.
         */
#if SUGAR_ENABLE_STACK_CHECKING
        /* Set up stack-checking if enabled for this thread */
        if (if_stack_check) {
            /* Store the stack details for use by the stack-check function */
            tcb_ptr->stack_bottom = stack_bottom;
            tcb_ptr->stack_size = stack_size;

            /**
             * Prefill the stack with a known value. This is used later in
             * calls to atomThreadStackCheck() to get an indication of how
             * much stack has been used during runtime.
             */
            count = (int32_t)stack_size;
            while (count > 0) {
                /* Initialise all stack bytes from top down to 0x5A */
                *((uint8_t *)stack_bottom + (count - 1)) = STACK_CHECK_BYTE;
                count--;
            }
        }
#else
        /* Avoid compiler warning due to unused parameter */
        if_stack_check = if_stack_check;
#endif

        /**
         * Call the arch-specific routine to set up the stack. This routine
         * is responsible for creating the context save area necessary for
         * allowing atomThreadSwitch() to schedule it in. The initial
         * archContextSwitch() call when this thread gets scheduled in the
         * first time will then restore the program counter to the thread
         * entry point, and any other necessary register values ready for
         * it to start running.
         */
        arch_thread_context_init(tcb_ptr, stack_top, entry_point, user_param);

        /* Protect access to the OS queue */
        arch_critical_enter();

        /* Put this thread on the ready queue */
        if (sugar_tcb_ready_queue_push_priority (tcb_ptr) != RTE_SUCCESS) {
            /* Exit critical region */
            arch_critical_exit();
            /* Queue-related error */
            return NULL;
        } else {
            /* Exit critical region */
            arch_critical_exit();
            /**
             * If the OS is started and we're in thread context, check if we
             * should be scheduled in now.
             */
            if ((sugar_kernel_handle.if_started == true) &&
                sugar_context_get_current() != NULL) {
                sugar_sched(false);
            }
            /* Success */
        }
    }
    return tcb_ptr;
}

#if SUGAR_ENABLE_STACK_CHECKING
/**
 * \b sugar_thread_check_stack
 *
 * Check the stack usage of a thread.
 *
 * If the SUGAR_ENABLE_STACK_CHECKING macro is defined, thread stacks are filled
 * with a known value before the thread is started. This function can be
 * called at runtime to examine the stack and find the high water mark
 * (the furthest modified byte from the start of the stack).
 *
 * This gives an indication of how much stack the thread has used. It is
 * useful but not absolutely precise because the thread may legitimately
 * have the known value on its stack. The thread's stack pointer may also
 * have strayed outside of the allowable stack area while leaving some of
 * the known-value bytes unmodified. This simple method cannot trap stack
 * usage outside of the thread's allocated stack, for which you could use
 * additional guard areas (still limited in scope) or compiler/CPU/MMU
 * features.
 *
 * The function takes a thread's TCB and returns both the number of stack
 * bytes used, and the free stack bytes.
 *
 * @param[in] tcb_ptr Pointer to the TCB of the thread to stack-check
 * @param[in,out] used_bytes Pointer into which the used byte count is copied
 * @param[in,out] free_bytes Pointer into which the free byte count is copied
 *
 * @retval RTE_SUCCESS Success
 * @retval RTE_ERR_PARAM Bad parameters
 */
rte_error_t sugar_thread_check_stack(sugar_tcb_t *tcb_ptr, uint32_t *used_bytes, uint32_t *free_bytes)
{
    rte_error_t status;
    uint8_t *stack_ptr;
    int i;

    if ((tcb_ptr == NULL) || (used_bytes == NULL) || (free_bytes == NULL))
    {
        /* Bad parameters */
        status = RTE_ERR_PARAM;
    } else {
        /**
         * Starting at the bottom end, count the unmodified areas until a
         * modified byte is found.
         */
        stack_ptr = (uint8_t *)tcb_ptr->stack_bottom;
        for (i = 0; i < tcb_ptr->stack_size; i++) {
            /* Loop until a modified byte is found */
            if (*stack_ptr++ != STACK_CHECK_BYTE) {
                /* Found a modified byte */
                break;
            }
        }
        /* We quit the loop above on the count of the free bytes */
        *free_bytes = (uint32_t)i;
        /* Calculate used bytes using our knowledge of the stack size */
        *used_bytes = tcb_ptr->stack_size - *free_bytes;
        /* No error */
        status = RTE_SUCCESS;
    }
    return (status);
}
#endif /* SUGAR_ENABLE_STACK_CHECKING */
