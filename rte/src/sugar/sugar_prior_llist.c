/**
 * @file sugar_prior_llist.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
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
