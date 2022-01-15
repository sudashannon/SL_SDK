/**
 * @file sugar_scheduler.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-01-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "../../inc/sugar/sugar_scheduler.h"
#include "../../inc/sugar/sugar_prior_vector.h"
#include "../../inc/data_structure/ds_vector.h"
#include "../../inc/middle_layer/rte_memory.h"
#include "../../inc/middle_layer/rte_log.h"
#include "../../inc/middle_layer/rte_atomic.h"

/**
 * This function is called when a new thread is scheduled in for the first
 * time. It will simply call the threads entry point function.
 */
void sugar_thread_body(void)
{
    sugar_tcb_t *task_ptr = NULL;

    /**
     * We "return" to here after being scheduled in by the pend_sv_handler.
     * We get a pointer to our TCB from sugar_kernel_get_current_tcb()
     */
    task_ptr = sugar_kernel_get_current_tcb();
    /**
     * Our thread entry point and parameter are stored in the TCB.
     * Call it if it is valid
     */
    if(task_ptr && task_ptr->entry_point){
        task_ptr->entry_point(task_ptr->user_param);
    }
    /**
     * Thread returned or entry point was not valid.
     * Should never happen... Maybe we should switch MCU into debug mode here
     * TODO: add thread exit logic
     */
    while(1);
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
    old_tcb->thread_state = SUGAR_THREAD_READY_STATE;
    new_tcb->thread_state = SUGAR_THREAD_RUNNING_STATE;
    /**
     * Check if the new thread is actually the current one, in which
     * case we don't need to do any context switch. This can happen
     * if a thread goes into suspend but is unsuspended again before
     * it is fully scheduled out.
     */
    if (old_tcb != new_tcb) {
        /* Set the new currently-running thread pointer */
        sugar_kernel_handle.current_tcb = new_tcb;
        /* Call the architecture-specific context switch */
        arch_context_switch(old_tcb, new_tcb);
    }
}

/**
 * \b sugar_scheduler
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
void sugar_scheduler(bool if_in_tickhandle)
{
    arch_critical_store();
    sugar_tcb_t *new_tcb = NULL;
    /* Enter critical section */
    arch_critical_enter();
    /**
     * Check the OS has actually started. As long as the proper initialisation
     * sequence is followed there should be no calls here until the OS is
     * started, but we check to handle badly-behaved ports.
     */
    if (RTE_UNLIKELY(sugar_kernel_handle.if_started == false)) {
        /* Don't schedule anything in until the OS is started */
        goto no_scheduler;
    }
    /**
     * If the current thread is going into suspension or is being
     * terminated (run to completion), then unconditionally dequeue
     * the next thread for execution.
     */
    if ((sugar_kernel_handle.current_tcb->thread_state == SUGAR_THREAD_SUSPEND_STATE)) {
        /**
         * Dequeue the next ready to run thread. There will always be
         * at least the idle thread waiting. Note that this could
         * actually be the suspending thread if it was unsuspended
         * before the scheduler was called.
         */
        new_tcb = sugar_prior_vector_pop_highest(sugar_kernel_handle.ready_queue);
        /**
         * Don't need to add the current thread to any queue because
         * it was suspended by another OS mechanism and will be
         * sitting on a suspend queue or similar within one of the OS
         * primitive libraries (e.g. semaphore).
         */

        /* Switch to the new thread */
        sugar_context_switch(sugar_kernel_handle.current_tcb, new_tcb);
    }

    /**
     * Otherwise the current thread is still ready, but check
     * if any other threads are ready.
     */
    else
    {
        /* Check if a reschedule is allowed */
        new_tcb = sugar_prior_vector_pop_highest(sugar_kernel_handle.ready_queue);
        if (new_tcb != NULL) {
            if (new_tcb != sugar_kernel_handle.current_tcb) {
                if (if_in_tickhandle == true) {
                    /* Same priority or higher threads can preempt */
                    if (new_tcb->priority <= sugar_kernel_handle.current_tcb->priority) {
                        goto preempt;
                    }
                    (void)sugar_prior_vector_push(sugar_kernel_handle.ready_queue, new_tcb);
                    goto no_scheduler;
                } else {
                    /* Only higher priority threads can preempt */
                    if (new_tcb->priority < sugar_kernel_handle.current_tcb->priority) {
                        goto preempt;
                    }
                    (void)sugar_prior_vector_push(sugar_kernel_handle.ready_queue, new_tcb);
                    goto no_scheduler;
                }
            }
            (void)sugar_prior_vector_push(sugar_kernel_handle.ready_queue, new_tcb);
        }
        goto no_scheduler;
preempt:
        /* Add the current thread to the ready queue */
        (void)sugar_prior_vector_push(sugar_kernel_handle.ready_queue, sugar_kernel_handle.current_tcb);
        /* Switch to the new thread */
        sugar_context_switch (sugar_kernel_handle.current_tcb, new_tcb);
    }
no_scheduler:
    /* Exit critical section */
    arch_critical_exit();
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
    sugar_scheduler(if_in_tickhandle);
}

/**
 * \b sugar_tcb_scheduler_timer
 *
 * This is an internal function not for use by application code.
 *
 * Callback for sugar_delay_tick() calls. Wakes up the sleeping threads.
 *
 * @param[in] cb_data Callback parameter (DELAY_TIMER ptr for sleeping thread)
 *
 * @return None
 */
static void sugar_tcb_scheduler_timer(void *user_param)
{
    sugar_tcb_t *tcb_ptr = (sugar_tcb_t *)user_param;
    /* Check parameter is valid */
    if (tcb_ptr) {
        arch_critical_store();
        /* Enter critical region */
        arch_critical_enter();
        tcb_ptr->suspend_timer = NULL;
        /* Put the thread on the ready queue */
        (void)sugar_prior_vector_push(sugar_kernel_handle.ready_queue, tcb_ptr);
        /* Exit critical region */
        arch_critical_exit();
        /**
         * Don't call the scheduler yet. The ISR exit routine will do this
         * in case there are other callbacks to be made, which may also make
         * threads ready.
         */
    }
}

/**
 * \b sugar_delay_tick
 *
 * Suspend a thread for the given number of system ticks.
 *
 * Note that the wakeup time is the number of ticks from the current system
 * tick, therefore, for a one tick delay, the thread may be woken up at any
 * time between the atomTimerDelay() call and the next system tick. For
 * a minimum number of ticks, you should specify minimum number of ticks + 1.
 *
 * NOTE: This function can only be called from thread context.
 *
 * @param[in] ticks Number of system ticks to delay (must be > 0)
 *
 * @retval RTE_SUCCESS Successful delay
 */
rte_error_t sugar_delay_tick(tick_unit_t ticks)
{
    sugar_tcb_t *current_tcb = NULL;
    arch_critical_store();
    rte_error_t status = RTE_ERR_UNDEFINE;

    /* Get the current TCB  */
    current_tcb = sugar_kernel_get_current_tcb();

    /* Parameter check */
    if (ticks == 0) {
        /* Return error */
        status = RTE_ERR_PARAM;
        goto end;
    }

    /* Check we are actually in thread context */
    else if (current_tcb == NULL) {
        /* Not currently in thread context, can't suspend */
        status = RTE_ERR_PARAM;
        goto end;
    }

    /* Otherwise safe to proceed */
    else
    {
        /* Protect the system queues */
        arch_critical_enter();

        /* Set suspended status for the current thread */
        current_tcb->thread_state = SUGAR_THREAD_SUSPEND_STATE;

        /* Register the timer callback */
        /* Fill out the data and the timer callback request structure needed by the callback to wake us up */
        timer_configuration_t config = TIMER_CONFIG_INITIALIZER;
        config.repeat_period_tick = ticks;
        config.timer_callback = sugar_tcb_scheduler_timer;
        config.parameter = current_tcb;
        config.if_reload = false;
        /* Store the timeout callback details, though we don't use it */
        status = timer_create_new(sugar_kernel_handle.timer_group, &config, &(current_tcb->suspend_timer));
        /* Register the callback */
        if (status != RTE_SUCCESS ||
            current_tcb->suspend_timer == NULL) {
            /* Exit critical region */
            arch_critical_exit ();
            /* Timer registration didn't work, won't get a callback */
            status = RTE_ERR_NO_RSRC;
            goto end;
        } else {
            /* Exit critical region */
            arch_critical_exit();
            /* Successful timer registration */
            status = RTE_SUCCESS;
            /* Current thread should now block, schedule in another */
            sugar_scheduler(false);
        }
    }

end:
    return (status);
}
