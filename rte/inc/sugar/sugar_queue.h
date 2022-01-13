/**
 * @file sugar_queue.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-01-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __SUGAR_QUEUE_H
#define __SUGAR_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../middle_layer/rte.h"
#include "sugar_kernel.h"

typedef void *sugar_queue_t;

extern sugar_queue_t sugar_tcb_queue_init(void);
extern sugar_tcb_t *sugar_tcb_ready_queue_pop_next(sugar_queue_t queue);
extern sugar_tcb_t *sugar_tcb_ready_queue_pop_priority(sugar_queue_t queue, uint8_t priority);
extern rte_error_t sugar_tcb_ready_queue_push_priority(sugar_queue_t queue, sugar_tcb_t *tcb);

#ifdef __cplusplus
}
#endif

#endif /* __SUGAR_QUEUE_H */
