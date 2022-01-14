/**
 * @file sugar_sema.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-01-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __SUGAR_SEMA_H
#define __SUGAR_SEMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../middle_layer/rte.h"
#include "sugar_kernel.h"

typedef void *sugar_semaphore_t;

extern sugar_semaphore_t sugar_sema_create(uint8_t initial_count, uint8_t max_count);
extern rte_error_t sugar_sema_destroy(sugar_semaphore_t sem);
extern rte_error_t sugar_sema_acquire(sugar_semaphore_t sem, tick_unit_t timeout);
extern rte_error_t sugar_sema_give(sugar_semaphore_t sem);

#ifdef __cplusplus
}
#endif

#endif // __SUGAR_SEMA_H
