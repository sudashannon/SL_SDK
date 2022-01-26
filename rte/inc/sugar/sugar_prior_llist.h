/**
 * @file sugar_prior_llist.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-01-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __SUGAR_PRIOR_LINKLIST_H
#define __SUGAR_PRIOR_LINKLIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../middle_layer/rte.h"
#include "sugar_kernel.h"

typedef void *sugar_pri_llist_t;

extern sugar_pri_llist_t sugar_prior_llist_create(void);
extern rte_error_t sugar_prior_llist_destroy(sugar_pri_llist_t pri_llist);
extern sugar_tcb_t *sugar_prior_llist_pop_highest(sugar_pri_llist_t pri_llist);
extern sugar_tcb_t *sugar_prior_llist_pop_as_priority(sugar_pri_llist_t pri_llist, uint8_t priority);
extern rte_error_t sugar_prior_llist_remove(sugar_pri_llist_t pri_llist, sugar_tcb_t *select_task);
extern rte_error_t sugar_prior_llist_push(sugar_pri_llist_t pri_llist, sugar_tcb_t *tcb);
extern bool sugar_prior_llist_if_empty(sugar_pri_llist_t pri_llist);
#ifdef __cplusplus
}
#endif

#endif /* __SUGAR_PRIOR_LINKLIST_H */
