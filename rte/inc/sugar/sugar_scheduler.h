/**
 * @file sugar_scheduler.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-01-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __SUGAR_SCHEDULER_H
#define __SUGAR_SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../middle_layer/rte.h"
#include "sugar_kernel.h"

extern void sugar_thread_body(void);
extern void sugar_scheduler(bool if_in_tickhandle);
extern void sugar_interrupt_enter(void);
extern void sugar_interrupt_exit(uint8_t if_in_tickhandle);
extern rte_error_t sugar_delay_tick(tick_unit_t ticks);

#ifdef __cplusplus
}
#endif

#endif /* __SUGAR_SCHEDULER_H */