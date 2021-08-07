/**
 * @file rte_timer.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-04
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __RTE_TIMER_H
#define __RTE_TIMER_H

#include "rte.h"

typedef struct {
    uint8_t if_reload:1;
    uint8_t if_run_immediately:1;
    uint8_t reserved:6;
    uint32_t repeat_period_tick;
    void (*timer_callback)(void *);
    void *parameter;
} timer_configuration_t;

#define TIMER_CONFIG_INITIALIZER {                         \
    .if_reload = 1,                                        \
    .if_run_immediately = 1,                               \
    .repeat_period_tick = 100,                             \
    .timer_callback = NULL,                                \
    .parameter = NULL,                                     \
}

/**
 * @brief Init the timer module with excepted group count and os configration.
 *
 * @param max_group_num
 * @param if_with_os
 * @return rte_error_t
 */
extern rte_error_t timer_init(uint8_t max_group_num, bool if_with_os);
/**
 * @brief Add a new timer group into this timer handle.
 *        NOTE: this api isn't thread-safe.
 *              Mutex given to the timer group must be recursive!
 *
 * @param group_id
 * @return rte_error_t
 */
extern rte_error_t timer_create_group(timer_group_id_t *group_id, rte_mutex_t *mutex);
/**
 * @brief Deinit the timer module.
 *
 * @return rte_error_t
 */
extern rte_error_t timer_deinit(void);
/**
 * @brief Create a new timer and push it into select timer group.
 *
 * @param group_id
 * @param config
 * @param timer_id
 * @return rte_error_t
 */
extern rte_error_t timer_create_new(timer_group_id_t group_id, timer_configuration_t *config, timer_id_t *timer_id);
/**
 * @brief Delete an existed timer in the select timer group.
 *
 * @param group_id
 * @param timer_id
 * @return rte_error_t
 */
extern rte_error_t timer_delete(timer_group_id_t group_id, timer_id_t timer_id);
/**
 * @brief Poll select timer group.
 *
 * @param group_id
 */
extern void timer_group_poll(timer_group_id_t group_id);
/**
 * @brief Pause selected timer.
 *
 * @param group_id
 * @param timer_id
 * @return rte_error_t
 */
extern rte_error_t timer_pause(uint8_t group_id, uint8_t timer_id);
/**
 * @brief Resume selected timer.
 *
 * @param group_id
 * @param timer_id
 * @return rte_error_t
 */
extern rte_error_t timer_resume(uint8_t group_id, uint8_t timer_id);
/**
 * @brief Tick handle, should be called in the systick interrupt when without OS
 *        or in the timer thread when with OS.
 *
 */
extern void timer_tick_handle(void);
/**
 * @brief Return current time in milliseconds
 *
 * @return uint32_t
 */
extern uint32_t rte_get_tick(void);
/**
 * @brief Calculate time diff.
 *
 * @param prev_tick
 * @return uint32_t
 */
extern uint32_t rte_tick_consume(uint32_t prev_tick);
/**
 * @brief Block CPU for the running thread in some certain time.
 *
 * @param delay
 */
extern void rte_delay_ms(uint32_t delay);

#endif
