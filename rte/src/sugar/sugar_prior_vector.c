/**
 * @file sugar_prior_vector.c
 * @author Leon Shan (813475603@qq.com)
 * @brief Use rbtree to marked the priority, vector to be as a fifo.
 * @version 1.0.0
 * @date 2022-01-13
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "../../inc/sugar/sugar_prior_vector.h"
#include "../../inc/middle_layer/rte_memory.h"
#include "../../inc/middle_layer/rte_log.h"
#include "../../inc/middle_layer/rte_atomic.h"
#include "../../inc/data_structure/ds_rbtree.h"
#include "../../inc/data_structure/ds_vector.h"

typedef struct sugar_pri_vec_rbt_element {
    uint8_t priority;
    ds_vector_t tcb_array;
} sugar_pri_vec_rbt_element_t;

typedef struct sugar_pri_vec {
    rbt_t *tcb_rbt;
    uint8_t highest_priority;
} sugar_pri_vec_impl_t;

/**
 * @brief 8bit unsigned integers comparator
 */
static int rbt_cmp_priority(void *k1, size_t k1size,
                            void *k2, size_t k2size)
{
    RBT_CMP_KEYS_TYPE(uint8_t, k1, k1size, k2, k2size);
}

static void rbt_free_element(void *element)
{
    OS_ASSERT(ds_vector_destroy(element) == RTE_SUCCESS);
}

static rbt_walk_return_code_t rbt_walk_update_highest_priority(rbt_t *rbt, void *key, size_t ksize, void *value, void *priv)
{
    sugar_pri_vec_impl_t *queue_impl = (sugar_pri_vec_impl_t *)priv;
    sugar_pri_vec_rbt_element_t *element = (sugar_pri_vec_rbt_element_t *)value;
    if (element->priority <= queue_impl->highest_priority &&
        ds_vector_length(element->tcb_array) != 0) {
        queue_impl->highest_priority = element->priority;
    }
    return RBT_WALK_CONTINUE;
}

sugar_pri_vec_t sugar_prior_vector_create(void)
{
    sugar_pri_vec_impl_t *queue = NULL;
    queue = memory_calloc(BANK_OS, sizeof(sugar_pri_vec_impl_t));
    OS_ASSERT(queue);
    queue->tcb_rbt = rbt_create(rbt_cmp_priority, rbt_free_element);
    queue->highest_priority = SUGAR_IDLE_THREAD_PRIORITY;
    OS_ASSERT(queue->tcb_rbt);
    return queue;
}

/**
 * \b sugar_prior_vector_pop_highest
 *
 * This is an internal function not for use by application code.
 *
 * Dequeues the highest priority TCB on the queue pointed to by
 * \c pri_vec.
 *
 * The TCB will be removed from the queue. Same priority TCBs are dequeued in
 * FIFO order.
 *
 * \b NOTE: Assumes that the caller is already in a critical section.
 *
 * @param[in,out] pri_vec Pointer to TCB queue head pointer
 *
 * @return Pointer to highest priority TCB on queue, or NULL if queue empty
 */
sugar_tcb_t *sugar_prior_vector_pop_highest(sugar_pri_vec_t pri_vec)
{
    sugar_pri_vec_impl_t *queue_impl = (sugar_pri_vec_impl_t *)pri_vec;
    sugar_pri_vec_rbt_element_t *element = NULL;
    sugar_tcb_t *retval = NULL;
    if (rte_unlikely(queue_impl == NULL)) {
        goto end;
    }
    rbt_find(queue_impl->tcb_rbt, &(queue_impl->highest_priority), sizeof(uint8_t), (void **)&element);
    if (element == NULL) {
        OS_LOGF("Can't find vector with pro %d", queue_impl->highest_priority);
        goto end;
    }
    OS_ASSERT(queue_impl->highest_priority == element->priority);
    ds_vector_pop(element->tcb_array, (void *)&retval);
    if (retval == NULL) {
        goto end;
    }
    if (ds_vector_length(element->tcb_array) == 0 &&
        element->priority != SUGAR_IDLE_THREAD_PRIORITY) {
        queue_impl->highest_priority = SUGAR_IDLE_THREAD_PRIORITY;
        rbt_walk(queue_impl->tcb_rbt, rbt_walk_update_highest_priority, queue_impl);
    }
end:
    return retval;
}

/**
 * \b sugar_prior_vector_pop_as_priority
 *
 * This is an internal function not for use by application code.
 *
 * Dequeues the first TCB of the given priority or higher, from the queue
 * pointed to by \c pri_vec. Because the queue is ordered high priority
 * first, we only ever dequeue the list head, if any. If the list head is
 * lower priority than we wish to dequeue, then all following ones will also
 * be lower priority and hence are not parsed.
 *
 * The TCB will be removed from the queue. Same priority TCBs will be dequeued
 * in FIFO order.
 *
 * \b NOTE: Assumes that the caller is already in a critical section.
 *
 * @param[in,out] pri_vec Pointer to TCB queue head pointer
 * @param[in] priority Minimum priority to qualify for dequeue
 *
 * @return Pointer to the dequeued TCB, or NULL if none found within priority
 */
sugar_tcb_t *sugar_prior_vector_pop_as_priority(sugar_pri_vec_t pri_vec, uint8_t priority)
{
    sugar_pri_vec_rbt_element_t *element = NULL;
    sugar_pri_vec_impl_t *queue_impl = (sugar_pri_vec_impl_t *)pri_vec;
    sugar_tcb_t *retval = NULL;
    if (rte_unlikely(queue_impl == NULL)) {
        goto end;
    }
    rbt_find(queue_impl->tcb_rbt, &(priority), sizeof(uint8_t), (void **)&element);
    if (element == NULL) {
        goto end;
    }
    OS_ASSERT(priority == element->priority);
    ds_vector_pop(element->tcb_array, (void *)&retval);
    if (retval == NULL) {
        goto end;
    }
    if (ds_vector_length(element->tcb_array) == 0 &&
        element->priority != SUGAR_IDLE_THREAD_PRIORITY &&
        queue_impl->highest_priority == priority) {
        queue_impl->highest_priority = SUGAR_IDLE_THREAD_PRIORITY;
        rbt_walk(queue_impl->tcb_rbt, rbt_walk_update_highest_priority, queue_impl);
    }
end:
    return retval;
}

/**
 * \b sugar_prior_vector_pop
 *
 * This is an internal function not for use by application code.
 *
 * Dequeues a particular TCB from the queue pointed to by \c pri_vec.
 *
 * The TCB will be removed from the queue.
 *
 * \b NOTE: Assumes that the caller is already in a critical section.
 *
 * @param[in,out] queue Pointer to TCB queue pointer
 * @param[in] tcb_ptr Pointer to TCB to dequeue
 *
 * @return Pointer to the dequeued TCB, or NULL if entry wasn't found
 */
sugar_tcb_t *sugar_prior_vector_pop(sugar_pri_vec_t pri_vec, sugar_tcb_t *tcb_ptr)
{
    sugar_pri_vec_rbt_element_t *element = NULL;
    sugar_pri_vec_impl_t *queue_impl = (sugar_pri_vec_impl_t *)pri_vec;
    sugar_tcb_t *retval = NULL;
    rte_error_t result = RTE_ERR_UNDEFINE;
    if (rte_unlikely(queue_impl == NULL)) {
        goto end;
    }
    rbt_find(queue_impl->tcb_rbt, &(tcb_ptr->priority), sizeof(uint8_t), (void **)&element);
    if (element == NULL) {
        goto end;
    }
    OS_ASSERT(tcb_ptr->priority == element->priority);
    result = ds_vector_remove_by_pointer(element->tcb_array, tcb_ptr);
    if (result != RTE_SUCCESS) {
        goto end;
    }
    if (ds_vector_length(element->tcb_array) == 0 &&
        element->priority != SUGAR_IDLE_THREAD_PRIORITY &&
        queue_impl->highest_priority == tcb_ptr->priority) {
        queue_impl->highest_priority = SUGAR_IDLE_THREAD_PRIORITY;
        rbt_walk(queue_impl->tcb_rbt, rbt_walk_update_highest_priority, queue_impl);
    }
    retval = tcb_ptr;
end:
    return retval;
}

/**
 * \b sugar_prior_vector_push
 *
 * This is an internal function not for use by application code.
 *
 * Enqueues the TCB \c tcb on the sugar_pri_vec pointed to by \c queue.
 * TCBs are placed on the rbtree in priority order. If there are existing TCBs
 * at the same priority as the TCB to be enqueued, the enqueued TCB will be
 * placed at the end of the same-priority TCBs vector. Calls to sugar_prior_vector_pop_as_priority()
 * will dequeue same-priority TCBs in FIFO order.
 *
 * \b NOTE: Assumes that the caller is already in a critical section.
 *
 * @param[in,out] queue Pointer to TCB queue pointer
 * @param[in] tcb Pointer to TCB to enqueue
 *
 * @retval RTE_SUCCESS Success
 * @retval Other Bad parameters and so on
 */
rte_error_t sugar_prior_vector_push(sugar_pri_vec_t pri_vec, sugar_tcb_t *tcb)
{
    sugar_pri_vec_rbt_element_t *element = NULL;
    sugar_pri_vec_impl_t *queue_impl = (sugar_pri_vec_impl_t *)pri_vec;
    rte_error_t retval = RTE_ERR_UNDEFINE;
    if (rte_unlikely(queue_impl == NULL) ||
        rte_unlikely(tcb == NULL)) {
        retval = RTE_ERR_PARAM;
        goto end;
    }
    rbt_find(queue_impl->tcb_rbt, &(tcb->priority), sizeof(uint8_t), (void **)&element);
    if (element) {
        OS_ASSERT(element->priority == tcb->priority);
        retval = ds_vector_push(element->tcb_array, tcb);
        if (retval == RTE_SUCCESS) {
            /* Check if the vector is empty before and update the highest priority */
            if (ds_vector_length(element->tcb_array) == 1) {
                if (element->priority <= queue_impl->highest_priority) {
                    queue_impl->highest_priority = element->priority;
                }
            }
        }
    } else {
        element = memory_calloc(BANK_OS, sizeof(sugar_pri_vec_rbt_element_t));
        OS_ASSERT(element);
        vector_configuration_t configuration = VECTOR_CONFIG_INITIALIZER;
        configuration.capacity = SUGAR_TCB_FIFO_CAPABILITY;
        element->priority = tcb->priority;
        retval = ds_vector_create(&configuration, &(element->tcb_array));
        if (retval != RTE_SUCCESS) {
            memory_free(BANK_OS, element);
            goto end;
        }
        retval = ds_vector_push(element->tcb_array, tcb);
        if (retval != RTE_SUCCESS) {
            ds_vector_destroy(element->tcb_array);
            memory_free(BANK_OS, element);
            goto end;
        }
        retval = rbt_add(queue_impl->tcb_rbt, &element->priority, sizeof(uint8_t), element);
        if (retval == 0) {
            if (element->priority <= queue_impl->highest_priority) {
                queue_impl->highest_priority = element->priority;
            }
            tcb->thread_state = SUGAR_THREAD_READY_STATE;
        }
    }
end:
    return retval;
}
