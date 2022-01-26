/**
 * @file sugar_prior_llist.c
 * @author Leon Shan (813475603@qq.com)
 * @brief   FIXME: Don't use the llist in ds_struture to speed up the
 *                  remove progress.
 * @version 1.0.0
 * @date 2022-01-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "../../inc/sugar/sugar_prior_llist.h"
#include "../../inc/middle_layer/rte_memory.h"
#include "../../inc/middle_layer/rte_log.h"
#include "../../inc/middle_layer/rte_atomic.h"
#include "../../inc/data_structure/ds_rbtree.h"
#include "../../inc/data_structure/ds_linklist.h"

typedef struct sugar_pri_llist_rbt_element {
    uint8_t priority;
    linked_list_t *tcb_list;
} sugar_pri_llist_rbt_element_t;

typedef struct sugar_pri_llist {
    rbt_t *tcb_rbt;
    uint8_t highest_priority;
} sugar_pri_llist_impl_t;


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
    list_destroy(element);
}

sugar_pri_llist_t sugar_prior_llist_create(void)
{
    sugar_pri_llist_impl_t *llist = NULL;
    llist = memory_calloc(BANK_OS, sizeof(sugar_pri_llist_impl_t));
    OS_ASSERT(llist);
    llist->tcb_rbt = rbt_create(rbt_cmp_priority, rbt_free_element);
    llist->highest_priority = SUGAR_IDLE_THREAD_PRIORITY;
    OS_ASSERT(llist->tcb_rbt);
    return llist;
}

rte_error_t sugar_prior_llist_destroy(sugar_pri_llist_t pri_llist)
{
    sugar_pri_llist_impl_t *llist_impl = (sugar_pri_llist_impl_t *)pri_llist;
    rbt_destroy(llist_impl->tcb_rbt);
    memory_free(BANK_OS, llist_impl);
    return RTE_SUCCESS;
}

static rbt_walk_return_code_t rbt_walk_update_highest_priority(rbt_t *rbt, void *key, size_t ksize, void *value, void *priv)
{
    sugar_pri_llist_impl_t *llist_impl = (sugar_pri_llist_impl_t *)priv;
    sugar_pri_llist_rbt_element_t *element = (sugar_pri_llist_rbt_element_t *)value;
    if (element->priority <= llist_impl->highest_priority &&
        list_count(element->tcb_list) != 0) {
        llist_impl->highest_priority = element->priority;
    }
    return RBT_WALK_CONTINUE;
}

sugar_tcb_t *sugar_prior_llist_pop_highest(sugar_pri_llist_t pri_llist)
{
    sugar_pri_llist_impl_t *llist_impl = (sugar_pri_llist_impl_t *)pri_llist;
    sugar_pri_llist_rbt_element_t *element = NULL;
    sugar_tcb_t *retval = NULL;
    if (rte_unlikely(llist_impl == NULL)) {
        goto end;
    }
    rbt_find(llist_impl->tcb_rbt, &(llist_impl->highest_priority), sizeof(uint8_t), (void **)&element);
    if (element == NULL) {
        OS_LOGF("Can't find llist with pro %d", llist_impl->highest_priority);
        goto end;
    }
    OS_ASSERT(llist_impl->highest_priority == element->priority);
    retval = list_pop_head_value(element->tcb_list);
    if (retval == NULL) {
        goto end;
    }
    if (list_count(element->tcb_list) == 0 &&
        element->priority != SUGAR_IDLE_THREAD_PRIORITY) {
        llist_impl->highest_priority = SUGAR_IDLE_THREAD_PRIORITY;
        rbt_walk(llist_impl->tcb_rbt, rbt_walk_update_highest_priority, llist_impl);
    }
end:
    return retval;
}

sugar_tcb_t *sugar_prior_llist_pop_as_priority(sugar_pri_llist_t pri_llist, uint8_t priority)
{
    sugar_pri_llist_impl_t *llist_impl = (sugar_pri_llist_impl_t *)pri_llist;
    sugar_pri_llist_rbt_element_t *element = NULL;
    sugar_tcb_t *retval = NULL;
    if (rte_unlikely(llist_impl == NULL)) {
        goto end;
    }
    rbt_find(llist_impl->tcb_rbt, &(priority), sizeof(uint8_t), (void **)&element);
    if (element == NULL) {
        goto end;
    }
    OS_ASSERT(priority == element->priority);
    retval = list_pop_head_value(element->tcb_list);
    if (retval == NULL) {
        goto end;
    }
    if (list_count(element->tcb_list) == 0 &&
        element->priority != SUGAR_IDLE_THREAD_PRIORITY &&
        llist_impl->highest_priority == priority) {
        llist_impl->highest_priority = SUGAR_IDLE_THREAD_PRIORITY;
        rbt_walk(llist_impl->tcb_rbt, rbt_walk_update_highest_priority, llist_impl);
    }
end:
    return retval;
}

static int llist_element_remove_handle(void *item, size_t idx, void *user)
{
    if (item == user) {
        return -2;
    }
    return 1;
}

rte_error_t sugar_prior_llist_remove(sugar_pri_llist_t pri_llist, sugar_tcb_t *select_task)
{
    sugar_pri_llist_impl_t *llist_impl = (sugar_pri_llist_impl_t *)pri_llist;
    sugar_pri_llist_rbt_element_t *element = NULL;
    rte_error_t retval = RTE_ERR_UNDEFINE;
    if (rte_unlikely(llist_impl == NULL)) {
        retval = RTE_ERR_PARAM;
        goto end;
    }
    rbt_find(llist_impl->tcb_rbt, &(select_task->priority), sizeof(uint8_t), (void **)&element);
    if (element == NULL) {
        retval = RTE_ERR_NO_RSRC;
        goto end;
    }
    OS_ASSERT(select_task->priority == element->priority);
    list_foreach_value(element->tcb_list, llist_element_remove_handle, select_task);
    if (list_count(element->tcb_list) == 0 &&
        element->priority != SUGAR_IDLE_THREAD_PRIORITY &&
        llist_impl->highest_priority == select_task->priority) {
        llist_impl->highest_priority = SUGAR_IDLE_THREAD_PRIORITY;
        rbt_walk(llist_impl->tcb_rbt, rbt_walk_update_highest_priority, llist_impl);
    }
    retval = RTE_SUCCESS;
end:
    return retval;
}

rte_error_t sugar_prior_llist_push(sugar_pri_llist_t pri_llist, sugar_tcb_t *tcb)
{
    sugar_pri_llist_impl_t *llist_impl = (sugar_pri_llist_impl_t *)pri_llist;
    sugar_pri_llist_rbt_element_t *element = NULL;
    rte_error_t retval = RTE_ERR_UNDEFINE;
    if (rte_unlikely(llist_impl == NULL) ||
        rte_unlikely(tcb == NULL)) {
        retval = RTE_ERR_PARAM;
        goto end;
    }
    rbt_find(llist_impl->tcb_rbt, &(tcb->priority), sizeof(uint8_t), (void **)&element);
    if (element) {
        OS_ASSERT(element->priority == tcb->priority);
        retval = list_push_tail_value(element->tcb_list, tcb);
        if (retval == RTE_SUCCESS) {
            /* Check if the llist is empty before and update the highest priority */
            if (list_count(element->tcb_list) == 1) {
                if (element->priority <= llist_impl->highest_priority) {
                    llist_impl->highest_priority = element->priority;
                }
            }
        }
    } else {
        element = memory_calloc(BANK_OS, sizeof(sugar_pri_llist_rbt_element_t));
        OS_ASSERT(element);
        element->priority = tcb->priority;
        element->tcb_list = list_create(NULL);
        if (element->tcb_list == NULL) {
            memory_free(BANK_OS, element);
            goto end;
        }
        retval = list_push_tail_value(element->tcb_list, tcb);
        if (retval != RTE_SUCCESS) {
            list_destroy(element->tcb_list);
            memory_free(BANK_OS, element);
            goto end;
        }
        retval = rbt_add(llist_impl->tcb_rbt, &element->priority, sizeof(uint8_t), element);
        if (retval == 0) {
            if (element->priority <= llist_impl->highest_priority) {
                llist_impl->highest_priority = element->priority;
            }
            tcb->thread_state = SUGAR_THREAD_READY_STATE;
        }
    }
end:
    return retval;
}

static rbt_walk_return_code_t rbt_walk_get_llist_count(rbt_t *rbt, void *key, size_t ksize, void *value, void *priv)
{
    sugar_pri_llist_rbt_element_t *element = (sugar_pri_llist_rbt_element_t *)value;
    if (list_count(element->tcb_list) != 0) {
        *((bool *)priv) = false;
        return RBT_WALK_STOP;
    }
    return RBT_WALK_CONTINUE;
}

bool sugar_prior_llist_if_empty(sugar_pri_llist_t pri_llist)
{
    bool retval = true;
    sugar_pri_llist_impl_t *llist_impl = (sugar_pri_llist_impl_t *)pri_llist;
    rbt_walk(llist_impl->tcb_rbt, rbt_walk_get_llist_count, &retval);
    return retval;
}