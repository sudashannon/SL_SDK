/**
 * @file rte_timer.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-04
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../inc/middle_layer/rte_timer.h"
#include "../../inc/middle_layer/rte_memory.h"
#include "../../inc/middle_layer/rte_log.h"
#include "../../inc/data_structure/ds_vector.h"

#define THIS_MODULE LOG_STR(TIMER)
#define TIMER_LOGF(...) LOG_FATAL(THIS_MODULE, __VA_ARGS__)
#define TIMER_LOGE(...) LOG_ERROR(THIS_MODULE, __VA_ARGS__)
#define TIMER_LOGI(...) LOG_INFO(THIS_MODULE, __VA_ARGS__)
#define TIMER_LOGW(...) LOG_WARN(THIS_MODULE, __VA_ARGS__)
#define TIMER_LOGD(...) LOG_DEBUG(THIS_MODULE, __VA_ARGS__)
#define TIMER_LOGV(...) LOG_VERBOSE(THIS_MODULE, __VA_ARGS__)
#define TIMER_ASSERT(v) LOG_ASSERT(THIS_MODULE, v)
typedef struct
{
    struct {
        uint8_t AREN:1;  		/*!< Auto-reload enabled */
        uint8_t CNTEN:1; 		/*!< Count enabled */
        uint8_t reserved:6;
    } config;
    timer_id_t index;			    /*!< Timer ID */
    timer_group_id_t group_id;	    /*!< Group ID */
    volatile uint32_t ARR;          /*!< Auto reload value */
    volatile uint32_t CNT;          /*!< Counter value, counter counts down */
    rte_callback_f callback;	    /*!< Callback which will be called when timer reaches zero */
    void* parameter;           		/*!< Pointer to user parameters used for callback function */
} timer_t;

typedef struct
{
    uint8_t index;
    ds_vector_t timer_table;
} timer_group_t;

typedef struct
{
    uint8_t if_with_os:1;
    uint8_t reserved:7;
    uint8_t group_capability;
    uint8_t group_count;
    timer_group_t *timer_group;
    volatile uint32_t timer_tick_count;
} timer_handle_t;

static timer_handle_t timer_handle_instance;

static void single_timer_free_cb(void *timer, uint32_t index)
{
    RTE_UNUSED(index);
    TIMER_ASSERT(timer == ds_vector_at(timer_handle_instance.timer_group[((timer_t *)timer)->group_id].timer_table, index));
    rte_get_general_allocator()->free(timer);
}
/**
 * @brief Init the timer module with excepted group count and os configration.
 *
 * @param max_group_num
 * @param if_with_os
 * @return rte_error_t
 */
rte_error_t timer_init(uint8_t max_group_num, bool if_with_os)
{
    timer_handle_instance.timer_group = rte_get_general_allocator()->calloc(
                                        sizeof(timer_group_t) * max_group_num);
    if (!timer_handle_instance.timer_group)
        return RTE_ERR_NO_MEM;
    timer_handle_instance.group_count = 0;
    timer_handle_instance.group_capability = max_group_num;
    timer_handle_instance.timer_tick_count = 0;
    timer_handle_instance.if_with_os = if_with_os;
    return RTE_SUCCESS;
}
/**
 * @brief Add a new timer group into this timer handle.
 *        NOTE: this api isn't thread-safe.
 *              Mutex given to the timer group must be recursive!
 *
 * @param group_id
 * @return rte_error_t
 */
rte_error_t timer_create_group(timer_group_id_t *group_id, rte_mutex_t *mutex)
{
    if (RTE_UNLIKELY(group_id == NULL)) {
        return RTE_ERR_PARAM;
    }
    if (timer_handle_instance.group_count >= timer_handle_instance.group_capability - 1) {
        TIMER_LOGE("no valid space for new timer group!");
        return RTE_ERR_NO_RSRC;
    }
    ds_vector_t temp = NULL;
    vector_configuration_t configuration = VECTOR_CONFIG_INITIALIZER;
    configuration.mutex = mutex;
    configuration.capacity = RTE_TIMER_GROUP_CAPACITY;
    configuration.if_expand = true;
    configuration.free_cb = single_timer_free_cb;
    rte_error_t result = ds_vector_create(&configuration, &temp);
    if (result != RTE_SUCCESS) {
        TIMER_LOGE("create vector failed for new timer group!");
        return RTE_ERR_NO_RSRC;
    }
    timer_handle_instance.timer_group[timer_handle_instance.group_count].index = timer_handle_instance.group_count;
    timer_handle_instance.timer_group[timer_handle_instance.group_count].timer_table = temp;
    *group_id = timer_handle_instance.group_count++;
    return RTE_SUCCESS;
}
/**
 * @brief Deinit the timer module.
 *
 * @return rte_error_t
 */
rte_error_t timer_deinit(void)
{
    for (uint8_t i = 0; i < timer_handle_instance.group_count; i++) {
        TIMER_ASSERT(ds_vector_destroy(timer_handle_instance.timer_group[i].timer_table));
    }
    rte_get_general_allocator()->free(timer_handle_instance.timer_group);
    return RTE_SUCCESS;
}
/**
 * @brief Create a new timer and push it into select timer group.
 *
 * @param group_id
 * @param config
 * @param timer_id
 * @return rte_error_t
 */
rte_error_t timer_create_new(timer_group_id_t group_id, timer_configuration_t *config, timer_id_t *timer_id)
{
    if (RTE_UNLIKELY(group_id > timer_handle_instance.group_count) ||
        RTE_UNLIKELY(config == NULL) ||
        RTE_UNLIKELY(timer_id == NULL)) {
        return RTE_ERR_PARAM;
    }
    TIMER_ASSERT(timer_handle_instance.timer_group[group_id].timer_table);
    timer_t *v = rte_get_general_allocator()->calloc(sizeof(timer_t));
    if (v == NULL)
        return RTE_ERR_NO_MEM;
    ds_vector_lock(timer_handle_instance.timer_group[group_id].timer_table);
    v->index = ds_vector_length(timer_handle_instance.timer_group[group_id].timer_table);
    v->config.AREN = config->if_reload;
    v->config.CNTEN = config->if_run_immediately;
    v->ARR = config->repeat_period_tick;
    v->CNT = config->repeat_period_tick;
    v->callback = config->timer_callback;
    v->parameter = config->parameter;
    v->group_id = group_id;
    rte_error_t result = ds_vector_push(timer_handle_instance.timer_group[group_id].timer_table, v);
    if (result != RTE_SUCCESS) {
        rte_get_general_allocator()->free(v);
    } else {
        *timer_id = v->index;
    }
    ds_vector_unlock(timer_handle_instance.timer_group[group_id].timer_table);
    return result;
}
/**
 * @brief Delete an existed timer in the select timer group.
 *
 * @param group_id
 * @param timer_id
 * @return rte_error_t
 */
rte_error_t timer_delete(timer_group_id_t group_id, timer_id_t timer_id)
{
    if (RTE_UNLIKELY(group_id > timer_handle_instance.group_count)) {
        return RTE_ERR_PARAM;
    }
    TIMER_ASSERT(timer_handle_instance.timer_group[group_id].timer_table);
    return ds_vector_remove_by_index(timer_handle_instance.timer_group[group_id].timer_table, timer_id);
}

inline static void timer_check(timer_t *timer)
{
    /* Check if count is zero */
    if(timer->CNT == 0) {
        /* Call user callback function */
        if(timer->callback)
            timer->callback(timer->parameter);
        /* Set new counter value */
        timer->CNT = timer->ARR;
        /* Disable timer if auto reload feature is not used */
        if (!timer->config.AREN) {
            /* Disable counter */
            timer->config.CNTEN = false;
            timer_delete(timer->group_id, timer->index);
        }
    }
}
/**
 * @brief Poll select timer group.
 *
 * @param group_id
 */
void timer_group_poll(timer_group_id_t group_id)
{
    uint8_t i = 0;
    timer_t *timer = NULL;
    ds_vector_lock(timer_handle_instance.timer_group[group_id].timer_table);
    // Loop through each task in the task table.
    VECTOR_FOR_EACH_SAFELY(i, timer, timer_handle_instance.timer_group[group_id].timer_table) {
        timer_check(timer);
    }
    ds_vector_unlock(timer_handle_instance.timer_group[group_id].timer_table);
}
/**
 * @brief Pause selected timer.
 *
 * @param group_id
 * @param timer_id
 * @return rte_error_t
 */
rte_error_t timer_pause(uint8_t group_id, uint8_t timer_id)
{
    if (RTE_UNLIKELY(group_id > timer_handle_instance.group_count)) {
        return RTE_ERR_PARAM;
    }
    ds_vector_t this_timer_table = timer_handle_instance.timer_group[group_id].timer_table;
    rte_error_t retval = RTE_ERR_UNDEFINE;
    ds_vector_lock(this_timer_table);
    timer_t *timer = ds_vector_at(this_timer_table, timer_id);
    if (timer) {
        timer->config.CNTEN = 0;
        retval = RTE_SUCCESS;
    } else {
        retval = RTE_ERR_PARAM;
    }
    ds_vector_unlock(this_timer_table);
    return retval;
}
/**
 * @brief Resume selected timer.
 *
 * @param group_id
 * @param timer_id
 * @return rte_error_t
 */
rte_error_t timer_resume(uint8_t group_id, uint8_t timer_id)
{
    if (RTE_UNLIKELY(group_id > timer_handle_instance.group_count)) {
        return RTE_ERR_PARAM;
    }
    ds_vector_t this_timer_table = timer_handle_instance.timer_group[group_id].timer_table;
    rte_error_t retval = RTE_ERR_UNDEFINE;
    ds_vector_lock(this_timer_table);
    timer_t *timer = ds_vector_at(this_timer_table, timer_id);
    if (timer) {
        timer->config.CNTEN = 1;
        retval = RTE_SUCCESS;
    } else {
        retval = RTE_ERR_PARAM;
    }
    ds_vector_unlock(this_timer_table);
    return retval;
}
/**
 * @brief Tick handle, should be called in the systick interrupt when without OS
 *        or in the timer thread when with OS.
 *
 */
void timer_tick_handle(uint32_t tick)
{
    // Loop through each group in the group table.
    for(uint8_t i = 0; i < timer_handle_instance.group_count; i++) {
        uint8_t j = 0;
        timer_t *timer = NULL;
        ds_vector_lock(timer_handle_instance.timer_group[i].timer_table);
        // Loop through each task in the task table.
        VECTOR_FOR_EACH_SAFELY(j, timer, timer_handle_instance.timer_group[i].timer_table) {
              /*!< Timer is enabled */
            if (timer->config.CNTEN) {
                /* Decrease counter if needed */
                if (timer->CNT)
                    timer->CNT -= tick;
            }
        }
        ds_vector_unlock(timer_handle_instance.timer_group[i].timer_table);
    }
    if (timer_handle_instance.if_with_os)
        timer_group_poll(0);
    else
        timer_handle_instance.timer_tick_count += tick;
}
/**
 * @brief Return current time in milliseconds
 *
 * @return uint32_t
 */
__attribute__((weak)) uint32_t rte_get_tick(void)
{
    return timer_handle_instance.timer_tick_count;
}
/**
 * @brief Calculate time diff.
 *
 * @param prev_tick
 * @return uint32_t
 */
uint32_t rte_tick_consume(uint32_t prev_tick)
{
    uint32_t act_time = rte_get_tick();
    /*If there is no overflow in sys_time simple subtract*/
    if(act_time >= prev_tick) {
        prev_tick = act_time - prev_tick;
    } else {
        prev_tick = UINT32_MAX - prev_tick + 1;
        prev_tick += act_time;
    }
    return prev_tick;
}
/**
 * @brief Block CPU for the running thread in some certain time.
 *
 * @param delay
 */
__attribute__((weak)) void rte_delay_ms(uint32_t delay)
{
    /* Delay for amount of milliseconds */
    uint32_t tickstart = rte_get_tick();
    /* Count interrupts */
    while ((rte_get_tick() - tickstart) < delay);
}
