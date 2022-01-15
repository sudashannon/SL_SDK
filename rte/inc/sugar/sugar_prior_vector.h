/**
 * @file sugar_prior_vector.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-01-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __SUGAR_PRIOR_VECTOR_H
#define __SUGAR_PRIOR_VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../middle_layer/rte.h"
#include "sugar_kernel.h"

typedef void *sugar_pri_vec_t;

extern sugar_pri_vec_t sugar_prior_vector_create(void);
extern sugar_tcb_t *sugar_prior_vector_pop_highest(sugar_pri_vec_t queue);
extern sugar_tcb_t *sugar_prior_vector_pop_as_priority(sugar_pri_vec_t queue, uint8_t priority);
extern sugar_tcb_t *sugar_prior_vector_pop(sugar_pri_vec_t queue, sugar_tcb_t *tcb_ptr);
extern rte_error_t sugar_prior_vector_push(sugar_pri_vec_t queue, sugar_tcb_t *tcb);

#ifdef __cplusplus
}
#endif

#endif /* __SUGAR_PRIOR_VECTOR_H */
