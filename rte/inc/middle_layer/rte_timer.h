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

typedef void (*timer_callback_f)(void *arg);

typedef struct {
    uint8_t if_reload:1;
    uint8_t if_run_immediately:1;
    uint8_t reserved:6;
    tick_unit_t repeat_period_tick;
    timer_callback_f timer_callback;
    void *parameter;
} timer_configuration_t;

#define TIMER_CONFIG_INITIALIZER {                         \
    .if_reload = 1,                                        \
    .if_run_immediately = 1,                               \
    .repeat_period_tick = 100,                               \
    .timer_callback = NULL,                                \
    .parameter = NULL,                                     \
}

typedef struct
{
    struct {
        uint8_t AREN:1;  		/*!< Auto-reload enabled */
        uint8_t CNTEN:1; 		/*!< Count enabled */
        uint8_t reserved:6;
    } config;
    timer_id_t index;			    /*!< Timer ID */
    timer_group_id_t group_id;	    /*!< Group ID */
    volatile tick_unit_t ARR;       /*!< Auto reload value */
    volatile tick_unit_t CNT;       /*!< Counter value, counter counts down */
    timer_callback_f callback;	    /*!< Callback which will be called when timer reaches zero */
    void* parameter;           		/*!< Pointer to user parameters used for callback function */
} timer_impl_t;

/**
 * @brief Init the timer module with excepted group count and os configration.
 *
 * @param max_group_num
 * @return rte_error_t
 */
extern rte_error_t timer_init(uint8_t max_group_num);
/**
 * @brief Add a new timer group into this timer handle.
 *        NOTE: this api isn't thread-safe.
 *              Mutex given to the timer group must be recursive!
 *
 * @param group_id
 * @param mutex
 * @param capacity
 * @return rte_error_t
 */
extern rte_error_t timer_create_group(timer_group_id_t *group_id, rte_mutex_t *mutex, uint32_t capacity);
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
 * @param timer
 * @return rte_error_t
 */
extern rte_error_t timer_create_new(timer_group_id_t group_id, timer_configuration_t *config, timer_impl_t **timer);
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
 * @brief Return current time in tick count.
 *
 * @return tick_unit_t
 */
extern tick_unit_t rte_get_tick(void);
/**
 * @brief Calculate time diff.
 *
 * @param prev_tick
 * @return tick_unit_t
 */
extern tick_unit_t rte_time_consume(tick_unit_t prev_tick);
/**
 * @brief Block CPU for the running thread in some certain time.
 *
 * @param delay_tick
 */
extern void rte_block_tick(tick_unit_t delay_tick);
/**
 * @brief Yield the CPU for the running thread.
 *
 * @param void
 */
extern void rte_yield(void);
/**
 * @brief Demon a selected timer group.
 *
 * @param group_id
 * @return void
 */
extern void timer_group_demon(timer_group_id_t group_id);
#endif
