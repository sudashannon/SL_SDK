/**
 * @file sugar_sema.c
 * @author Leon Shan (813475603@qq.com)
 * @brief FIXME: We use link list to store the wating thread and don't care about
 *               the priority which will cause the priority reverse problems.
 *               Make up a new data structure named prior_linklist to fix this problem.
 * @version 1.0.0
 * @date 2022-01-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "../../inc/sugar/sugar_sema.h"
#include "../../inc/sugar/sugar_prior_vector.h"
#include "../../inc/sugar/sugar_scheduler.h"
#include "../../inc/data_structure/ds_linklist.h"
#include "../../inc/middle_layer/rte_memory.h"
#include "../../inc/middle_layer/rte_log.h"
#include "../../inc/middle_layer/rte_atomic.h"

typedef struct sugar_sema {
    linked_list_t *wait_thread_list;
    uint8_t count_value;
    uint8_t max_count;
} sugar_sema_impl_t;

/* Local data types */
typedef struct sem_timer_arg
{
    sugar_tcb_t *tcb_ptr;  /* Thread which is suspended with timeout */
    sugar_sema_impl_t *sem_ptr;  /* Semaphore the thread is suspended on */
    size_t position_in_list;
} sem_timer_arg_t;

/**
 * \b sugar_sema_create
 *
 * Initialises a semaphore object.
 *
 * Must be called before calling any other semaphore library routines on a
 * semaphore. Objects can be deleted later using sugar_sema_destroy().
 *
 * This function can be called from interrupt context.
 *
 * @param[in] initial_count Initial count value
 * @param[in] max_count Max count value
 *
 * @retval sem Pointer to semaphore object
 */
sugar_semaphore_t sugar_sema_create(uint8_t initial_count, uint8_t max_count)
{
    sugar_sema_impl_t *semaphore = memory_calloc(BANK_OS, sizeof(sugar_sema_impl_t));
    if (semaphore == NULL)
        goto end;
    semaphore->count_value = initial_count;
    semaphore->wait_thread_list = list_create(NULL);
    if (max_count)
        semaphore->max_count = max_count;
    else
        semaphore->max_count = UINT8_MAX;
    if (semaphore->wait_thread_list == NULL) {
        memory_free(BANK_OS, semaphore);
        semaphore = NULL;
    }
end:
    return semaphore;
}

/**
 * \b sugar_sema_destroy
 *
 * Deletes a semaphore object.
 *
 * Any threads currently suspended on the semaphore will be woken up with
 * return status RTE_ERR_NO_RSRC. If called at thread context then the
 * scheduler will be called during this function which may schedule in one
 * of the woken threads depending on relative priorities.
 *
 * This function can be called from interrupt context, but loops internally
 * waking up all threads blocking on the semaphore, so the potential
 * execution cycles cannot be determined in advance.
 *
 * @param[in] sem Pointer to semaphore object
 *
 * @retval RTE_SUCCESS Success
 * @retval RTE_ERR_NO_RSRC Problem putting a woken thread on the ready queue
 * @retval RTE_ERR_UNDEFINE Problem cancelling a timeout on a woken thread
 */
rte_error_t sugar_sema_destroy(sugar_semaphore_t sem)
{
    sugar_sema_impl_t *semaphore_impl = (sugar_sema_impl_t *)sem;
    rte_error_t status = RTE_ERR_UNDEFINE;
    sugar_tcb_t *tcb_ptr;
    bool if_woken_thread = false;
    arch_critical_store();
    /* Parameter check */
    if (semaphore_impl == NULL) {
        /* Bad semaphore pointer */
        status = RTE_ERR_PARAM;
        goto end;
    } else {
        /* Default to success status unless errors occur during wakeup */
        status = RTE_SUCCESS;
        /* Wake up all suspended tasks */
        while (true) {
            /* Enter critical region */
            arch_critical_enter();
            /* Check if any threads are suspended, first waits first responses */
            tcb_ptr = list_pop_head_value(semaphore_impl->wait_thread_list);
            /* A thread is suspended on the semaphore */
            if (tcb_ptr) {
                /* Return error status to the waiting thread */
                tcb_ptr->suspend_wake_status = SUGAR_SUSPEND_DELETED_STATE;
                /* Put the thread on the ready queue */
                status = sugar_prior_vector_push(sugar_kernel_handle.ready_queue, tcb_ptr);
                if (status != RTE_SUCCESS) {
                    /* Exit critical region */
                    arch_critical_exit();
                    /* Quit the loop, returning error */
                    break;
                }
                /* If there's a timeout on this suspension, cancel it */
                if (tcb_ptr->suspend_timer != NULL) {
                    /* Cancel the callback */
                    status = timer_delete(sugar_kernel_handle.timer_group, tcb_ptr->suspend_timer->index);
                    if (status != RTE_SUCCESS) {
                        /* Exit critical region */
                        arch_critical_exit ();
                        /* Quit the loop, returning error */
                        break;
                    }
                    /* Flag as no timeout registered */
                    tcb_ptr->suspend_timer = NULL;
                }
                /* Exit critical region */
                arch_critical_exit ();
                /* Request a reschedule */
                if_woken_thread = true;
            } else {/* No more suspended threads */
                /* Exit critical region and quit the loop */
                arch_critical_exit ();
                break;
            }
        }
        /* Release allocated memory */
        list_destroy(semaphore_impl->wait_thread_list);
        memory_free(BANK_OS, semaphore_impl);
        /* Call scheduler if any threads were woken up */
        if (if_woken_thread == true) {
            /**
             * Only call the scheduler if we are in thread context, otherwise
             * it will be called on exiting the ISR by sugar_interrupt_exit().
             */
            if (sugar_kernel_get_current_tcb()) {
                sugar_scheduler(false);
            }
        }
    }
end:
    return (status);
}

/**
 * \b sugar_sema_timer
 *
 * This is an internal function not for use by application code.
 *
 * Timeouts on suspended threads are notified by the timer system through
 * this generic callback. The timer system calls us back with a pointer to
 * the relevant \c sem_timer_arg_t object which is used to retrieve the
 * semaphore details.
 *
 * @param[in] cb_data Pointer to a SEM_TIMER object
 */
static void sugar_sema_timer(void *user_param)
{
    sem_timer_arg_t *timer_data_ptr = NULL;
    arch_critical_store();

    /* Get the SEM_TIMER structure pointer */
    timer_data_ptr = (sem_timer_arg_t *)user_param;

    /* Check parameter is valid */
    if (timer_data_ptr) {
        /* Enter critical region */
        arch_critical_enter();
        /* Set status to indicate to the waiting thread that it timed out */
        timer_data_ptr->tcb_ptr->suspend_wake_status = SUGAR_SUSPEND_TIMEOUT_STATE;
        /* Flag as no timeout registered */
        timer_data_ptr->tcb_ptr->suspend_timer = NULL;
        /* Remove this thread from the semaphore's suspend list */
        OS_ASSERT(list_fetch_value(timer_data_ptr->sem_ptr->wait_thread_list,
                                    timer_data_ptr->position_in_list) == timer_data_ptr->tcb_ptr);
        /* Put the thread on the ready queue */
        (void)sugar_prior_vector_push(sugar_kernel_handle.ready_queue, timer_data_ptr->tcb_ptr);
        /* Exit critical region */
        arch_critical_exit ();
        /**
         * Note that we don't call the scheduler now as it will be called
         * when we exit the ISR by sugar_interrupt_exit().
         */
    }
}


/**
 * \b sugar_sema_acquire
 *
 * Perform a wait operation on a semaphore.
 *
 * This decrements the current count value for the semaphore and returns.
 * If the count value is already zero then the call will block until the
 * count is incremented by another thread, or until the specified \c timeout
 * is reached. Blocking threads will also be woken if the semaphore is
 * deleted by another thread while blocking.
 *
 * Depending on the \c timeout value specified the call will do one of
 * the following if the count value is zero:
 *
 * \c timeout == SUGAR_WAIT_NONE : Return immediately if the count is zero \n
 * \c timeout > 0 : Call will block until non-zero up to the specified timeout \n
 * \c timeout == TIME_MAX_DELAY : Call will block until the count is non-zero \n
 *
 * If the call needs to block and \c timeout is zero, it will block
 * indefinitely until sugar_sema_give() or sugar_sema_destroy() is called on the
 * semaphore.
 *
 * If the call needs to block and \c timeout is non-zero, the call will only
 * block for the specified number of system ticks after which time, if the
 * thread was not already woken, the call will return with \c RTE_ERR_TIMEOUT.
 *
 * If the call would normally block and \c timeout is -1, the call will
 * return immediately with \c RTE_ERR_TRY_LATER.
 *
 * This function can only be called from interrupt context if the \c timeout
 * parameter is -1 (in which case it does not block).
 *
 * @param[in] sem Pointer to semaphore object
 * @param[in] timeout Max system ticks to block (0 = forever)
 *
 * @retval RTE_SUCCESS Success
 * @retval RTE_ERR_TIMEOUT Semaphore timed out before being woken
 * @retval RTE_ERR_TRY_LATER Called with timeout == SUGAR_WAIT_NONE but count is zero
 * @retval RTE_ERR_BAD_USE Not called in thread context and attempted to block
 * @retval RTE_ERR_PARAM Bad parameter
 * @retval Other Problem putting the thread on the suspend queue
 * @retval Other Problem registering the timeout
 * @retval Other Semaphore was deleted while suspended
 */
rte_error_t sugar_sema_acquire(sugar_semaphore_t sem, tick_unit_t timeout)
{
    arch_critical_store();
    rte_error_t status = RTE_ERR_UNDEFINE;
    sem_timer_arg_t timer_arg = {NULL, NULL};
    sugar_tcb_t *curr_tcb_ptr = NULL;
    sugar_sema_impl_t *semaphore_impl = (sugar_sema_impl_t *)sem;

    /* Check parameters */
    if (semaphore_impl == NULL) {
        /* Bad semaphore pointer */
        status = RTE_ERR_PARAM;
        goto end;
    } else {
        /* Protect access to the semaphore object and OS queues */
        arch_critical_enter();
        /* If count is zero, block the calling thread */
        if (semaphore_impl->count_value == 0) {
            /* If called with timeout > 0, we should block */
            if (timeout != SUGAR_WAIT_NONE) {
                /* Count is zero, block the calling thread */

                /* Get the current TCB */
                curr_tcb_ptr = sugar_kernel_get_current_tcb();
                /* Check we are actually in thread context */
                if (curr_tcb_ptr) {
                    /* Add current thread to the suspend list on this semaphore */
                    status = list_push_tail_value(semaphore_impl->wait_thread_list, curr_tcb_ptr);
                    if (status != RTE_SUCCESS) {
                        /* Exit critical region */
                        arch_critical_exit();
                        /* There was an error putting this thread on the suspend list */
                        goto end;
                    } else {
                        /* Set suspended status for the current thread */
                        curr_tcb_ptr->thread_state = SUGAR_THREAD_SUSPEND_STATE;
                        /* Track errors */
                        status = RTE_SUCCESS;
                        /* Register a timer callback if requested */
                        if (timeout != SUGAR_WAIT_FOREVER) {
                            /* Fill out the data needed by the callback to wake us up */
                            timer_arg.tcb_ptr = curr_tcb_ptr;
                            timer_arg.sem_ptr = sem;
                            timer_arg.position_in_list = list_count(semaphore_impl->wait_thread_list) - 1;
                            /* Fill out the timer callback request structure */
                            timer_configuration_t config = TIMER_CONFIG_INITIALIZER;
                            config.repeat_period_tick = timeout;
                            config.timer_callback = sugar_sema_timer;
                            config.parameter = &timer_arg;;
                            config.if_reload = false;
                            /**
                             * Store the timer details in the TCB so that we can
                             * cancel the timer callback if the semaphore is put
                             * before the timeout occurs.
                             */
                            status = timer_create_new(sugar_kernel_handle.timer_group, &config, &(curr_tcb_ptr->suspend_timer));
                            /* Register a callback on timeout */
                            if (status != RTE_SUCCESS) {
                                /* Timer registration failed */
                                /* Clean up and return to the caller */
                                OS_ASSERT(list_pop_tail_value (semaphore_impl->wait_thread_list) == curr_tcb_ptr);
                                curr_tcb_ptr->thread_state = SUGAR_THREAD_RUNNING_STATE;
                                curr_tcb_ptr->suspend_timer = NULL;
                                arch_critical_exit();
                                goto end;
                            }
                        } else {
                            /* Set no timeout requested */
                            /* No need to cancel timeouts on this one */
                            curr_tcb_ptr->suspend_timer = NULL;
                        }

                        /* Exit critical region */
                        arch_critical_exit();
                        /**
                         * Current thread now blocking, schedule in a new
                         * one. We already know we are in thread context
                         * so can call the scheduler from here.
                         */
                        sugar_scheduler(false);

                        /**
                         * Normal sugar_sema_give() wakeups will set SUGAR_SUSPEND_OK_STATE status,
                         * while timeouts will set SUGAR_SUSPEND_TIMEOUT_STATE and semaphore
                         * deletions will set SUGAR_SUSPEND_DELETED_STATE.
                         */
                        status = curr_tcb_ptr->suspend_wake_status;
                        /**
                         * If we have been woken up with SUGAR_SUSPEND_OK_STATE then
                         * another thread incremented the semaphore and
                         * handed control to this thread. In theory the
                         * the posting thread increments the counter and
                         * as soon as this thread wakes up we decrement
                         * the counter here, but to prevent another
                         * thread preempting this thread and decrementing
                         * the semaphore before this section was
                         * scheduled back in, we emulate the increment
                         * and decrement by not incrementing in the
                         * sugar_sema_give() and not decrementing here. The
                         * count remains zero throughout preventing other
                         * threads preempting before we decrement the
                         * count again.
                         */
                        goto end;
                    }
                } else {
                    /* Exit critical region */
                    arch_critical_exit ();
                    /* Not currently in thread context, can't suspend */
                    status = RTE_ERR_BAD_USE;
                    goto end;
                }
            } else {
                /* timeout == NO_WAIT, requested not to block and count is zero */
                arch_critical_exit();
                status = RTE_ERR_TRY_LATER;
                goto end;
            }
        } else {
            /* Count is non-zero, just decrement it and return to calling thread */
            semaphore_impl->count_value--;
            /* Exit critical region */
            arch_critical_exit();
            /* Successful */
            status = RTE_SUCCESS;
            goto end;
        }
    }
end:
    return (status);
}

/**
 * \b sugar_sema_give
 *
 * Perform a put operation on a semaphore.
 *
 * This increments the current count value for the semaphore and returns.
 *
 * If the count value was previously zero and there are threads blocking on the
 * semaphore, the call will wake up the highest priority thread suspended. Only
 * one thread is woken per call to sugar_sema_give(). If multiple threads of the
 * same priority are suspended, they are woken in order of suspension (FIFO).
 *
 * This function can be called from interrupt context.
 *
 * @param[in] sem Pointer to semaphore object
 *
 * @retval RTE_SUCCESS Success
 * @retval RTE_ERR_OVERFLOW The semaphore count would have overflowed (>255)
 * @retval RTE_ERR_PARAM Bad parameter
 * @retval Other Problem putting a woken thread on the ready queue
 *               Problem cancelling a timeout for a woken thread
 */
rte_error_t sugar_sema_give(sugar_semaphore_t sem)
{
    arch_critical_store();
    rte_error_t status = RTE_ERR_UNDEFINE;
    sugar_sema_impl_t *semaphore_impl = (sugar_sema_impl_t *)sem;
    sugar_tcb_t *tcb_ptr = NULL;

    /* Check parameters */
    if (semaphore_impl == NULL) {
        /* Bad semaphore pointer */
        status = RTE_ERR_PARAM;
        goto end;
    } else {
        /* Protect access to the semaphore object and OS queues */
        arch_critical_enter();
        /* If any threads are blocking on the semaphore, wake up one */
        if (list_count(semaphore_impl->wait_thread_list)) {
            /**
             * Threads are woken up in priority order, with a FIFO system
             * used on same priority threads. We always take the head,
             * ordering is taken care of by an ordered list enqueue.
             */
            tcb_ptr = list_pop_head_value(semaphore_impl->wait_thread_list);
            if (sugar_prior_vector_push(sugar_kernel_handle.ready_queue, tcb_ptr) != RTE_SUCCESS) {
                /* Exit critical region */
                arch_critical_exit();
                /* There was a problem putting the thread on the ready queue */
                status = RTE_ERR_UNDEFINE;
                goto end;
            } else {
                /* Set OK status to be returned to the waiting thread */
                tcb_ptr->suspend_wake_status = SUGAR_SUSPEND_OK_STATE;
                /* If there's a timeout on this suspension, cancel it */
                if (tcb_ptr->suspend_timer != NULL) {
                    status = timer_delete(sugar_kernel_handle.timer_group, tcb_ptr->suspend_timer->index);
                    if (status != RTE_SUCCESS) {
                        /* Flag as no timeout registered */
                        tcb_ptr->suspend_timer = NULL;
                        /* Exit critical region */
                        arch_critical_exit();
                        goto end;
                    }
                } else {
                    /* Successful */
                    status = RTE_SUCCESS;
                }
                /* Exit critical region */
                arch_critical_exit ();
                /**
                 * The scheduler may now make a policy decision to thread
                 * switch if we are currently in thread context. If we are
                 * in interrupt context it will be handled by atomIntExit().
                 */
                if (sugar_kernel_get_current_tcb())
                    sugar_scheduler(false);
            }
        } else {
            /* If no threads waiting, just increment the count and return */
            /* Check for count overflow */
            if (semaphore_impl->count_value == semaphore_impl->max_count) {
                /* Don't increment, just return error status */
                status = RTE_ERR_OVERFLOW;
                /* Exit critical region */
                arch_critical_exit();
                goto end;
            } else {
                /* Increment the count and return success */
                semaphore_impl->count_value++;
                status = RTE_SUCCESS;
            }
            /* Exit critical region */
            arch_critical_exit();
        }
    }
end:
    return (status);
}
