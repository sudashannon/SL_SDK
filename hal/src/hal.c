/**
 * @file hal.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-09-11
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "hal.h"
#include "hal_com.h"

#define HAL_DEVICE_TRY_LOCK(device, timeout_tick, retval)           \
        tick_unit_t start_time = rte_get_tick();                    \
        RTE_TRYLOCK(&(device->mutex), timeout_tick, retval);        \
        tick_unit_t consumed_time = rte_time_consume(start_time);   \
        if (retval != RTE_SUCCESS)                                  \
            return retval;                                          \
        if (consumed_time >= left_time) {                           \
            RTE_UNLOCK(&(device->mutex));                           \
            return RTE_ERR_TIMEOUT;                                 \
        }                                                           \
        left_time -= consumed_time

static ds_hashtable_t hal_device_table = NULL;
#if RTE_USE_EXTERNAL_OS
static rte_mutex_t ht_chain_mutex_instance = {NULL};
static rte_mutex_t ht_bucket_mutex_instance = {NULL};
#endif

void *hal_get_device_table(void)
{
    return hal_device_table;
}

rte_error_t hal_init(void)
{
#if RTE_USE_EXTERNAL_OS
    ht_chain_mutex_instance.mutex = (void *)osMutexNew(NULL);
    ht_chain_mutex_instance.lock = rte_mutex_lock;
    ht_chain_mutex_instance.unlock = rte_mutex_unlock;
    ht_chain_mutex_instance.trylock = rte_mutex_trylock;
    ht_bucket_mutex_instance.mutex = (void *)osMutexNew(NULL);
    ht_bucket_mutex_instance.lock = rte_mutex_lock;
    ht_bucket_mutex_instance.unlock = rte_mutex_unlock;
    ht_bucket_mutex_instance.trylock = rte_mutex_trylock;
    hashtable_configuration_t hal_dt_config = {
        .initial_capacity = HASHTABLE_MIN_CAPACITY,
        .chain_mutex = &ht_chain_mutex_instance,
        .bucket_mutex = &ht_bucket_mutex_instance,
        .free_cb = NULL,
    };
#else
    hashtable_configuration_t hal_dt_config = {
        .initial_capacity = HASHTABLE_MIN_CAPACITY,
        .chain_mutex = NULL,
        .bucket_mutex = NULL,
        .free_cb = NULL,
    };
#endif
    ht_create(&hal_dt_config, &hal_device_table);
    hal_device_constructor_t *constructor_func_base = NULL;
    uint16_t constructor_num = 0;
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
    extern const uintptr_t hal_constructors$$Base;
    extern const uintptr_t hal_constructors$$Limit;
    constructor_func_base = (hal_device_constructor_t *)(&hal_constructors$$Base);
    constructor_num = ((hal_device_constructor_t *)(&hal_constructors$$Limit) - (hal_device_constructor_t *)(&hal_constructors$$Base));
#elif defined(__ICCARM__) || defined(__ICCRX__)
    #pragma section="hal_constructors"
    constructor_func_base = (hal_device_constructor_t *)(__section_begin("hal_constructors"));
    constructor_num = ((hal_device_constructor_t *)(__section_end("hal_constructors")) - (hal_device_constructor_t *)(__section_begin("hal_constructors")));
#elif defined(__GNUC__)
    extern const uintptr_t __hal_constructors_start;
    extern const uintptr_t __hal_constructors_end;
    constructor_func_base = (hal_device_constructor_t *)(&__hal_constructors_start);
    constructor_num = ((hal_device_constructor_t *)(&__hal_constructors_end) - (hal_device_constructor_t *)(&__hal_constructors_start));
#endif
    for (uint16_t i = 0; i < constructor_num; i++) {
        hal_device_constructor_t *func = constructor_func_base + i;
        if(func)
            (*func)();
    }
    return RTE_SUCCESS;
}

rte_error_t hal_deinit(void)
{
    ht_destroy(hal_device_table);
#if RTE_USE_EXTERNAL_OS
    osMutexDelete(ht_chain_mutex_instance.mutex);
    osMutexDelete(ht_bucket_mutex_instance.mutex);
#endif
    hal_device_destructor_t *destructor_func_base = NULL;
    uint16_t destructor_num = 0;
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
    extern const uintptr_t hal_destructors$$Base;
    extern const uintptr_t hal_destructors$$Limit;
    destructor_func_base = (hal_device_destructor_t *)(&hal_destructors$$Base);
    destructor_num = ((hal_device_destructor_t *)(&hal_destructors$$Limit) - (hal_device_destructor_t *)(&hal_destructors$$Base));
#elif defined(__ICCARM__) || defined(__ICCRX__)
    #pragma section="hal_destructors"
    destructor_func_base = (hal_device_destructor_t *)(__section_begin("hal_destructors"));
    destructor_num = ((hal_device_destructor_t *)(__section_end("hal_destructors")) - (hal_device_destructor_t *)(__section_begin("hal_destructors")));
#elif defined(__GNUC__)
    extern const uintptr_t __hal_destructors_start;
    extern const uintptr_t __hal_destructors_end;
    destructor_func_base = (hal_device_destructor_t *)(&__hal_destructors_start);
    destructor_num = ((hal_device_destructor_t *)(&__hal_destructors_end) - (hal_device_destructor_t *)(&__hal_destructors_start));
#endif
    for (uint16_t i = 0; i < destructor_num; i++) {
        hal_device_destructor_t *func = destructor_func_base + i;
        if(func)
            (*func)();
    }
    return RTE_SUCCESS;
}

hal_device_t *hal_get_device(const char *device_name)
{
    hal_device_t *retval = NULL;
    retval = ht_get(hal_device_table, (void *)device_name, strlen(device_name), NULL);
    return retval;
}

rte_error_t hal_device_read_sync(char *device_name, uint8_t *dest_buf,
                                uint32_t *buf_size, tick_unit_t timeout_tick)
{
    hal_device_t *device = hal_get_device(device_name);
    if (RTE_UNLIKELY(device == NULL) ||
        RTE_UNLIKELY(dest_buf == NULL) ||
        RTE_UNLIKELY(device->read == NULL)) {
        return RTE_ERR_PARAM;
    }
    rte_error_t retval = RTE_ERR_UNDEFINE;
    tick_unit_t left_time = timeout_tick;
    HAL_DEVICE_TRY_LOCK(device, left_time, retval);
    retval = device->read(device, dest_buf, buf_size, left_time);
    RTE_UNLOCK(&device->mutex);
    return retval;
}

rte_error_t hal_device_write_sync(char *device_name, uint8_t *src_buf,
                                        uint32_t buf_size, tick_unit_t timeout_tick)
{
    hal_device_t *device = hal_get_device(device_name);
    if (RTE_UNLIKELY(device == NULL) ||
        RTE_UNLIKELY(src_buf == NULL) ||
        RTE_UNLIKELY(device->write == NULL)) {
        return RTE_ERR_PARAM;
    }
    rte_error_t retval = RTE_ERR_UNDEFINE;
    tick_unit_t left_time = timeout_tick;
    HAL_DEVICE_TRY_LOCK(device, left_time, retval);
    retval = device->write(device, src_buf, buf_size, left_time);
    RTE_UNLOCK(&device->mutex);
    return retval;
}

rte_error_t hal_device_read_async(char *device_name, uint8_t *dest_buf,
                                        uint32_t *buf_size, tick_unit_t timeout_tick)
{
    hal_device_t *device = hal_get_device(device_name);
    if (RTE_UNLIKELY(device == NULL) ||
        RTE_UNLIKELY(dest_buf == NULL) ||
        RTE_UNLIKELY(device->read_async == NULL)) {
        return RTE_ERR_PARAM;
    }
    rte_error_t retval = RTE_ERR_UNDEFINE;
    tick_unit_t left_time = timeout_tick;
    HAL_DEVICE_TRY_LOCK(device, left_time, retval);
    retval = device->read_async(device, dest_buf, buf_size, left_time);
    RTE_UNLOCK(&device->mutex);
    return retval;
}

rte_error_t hal_device_write_async(char *device_name, uint8_t *src_buf,
                                        uint32_t buf_size, tick_unit_t timeout_tick)
{
    hal_device_t *device = hal_get_device(device_name);
    if (RTE_UNLIKELY(device == NULL) ||
        RTE_UNLIKELY(src_buf == NULL) ||
        RTE_UNLIKELY(device->write_async == NULL)) {
        return RTE_ERR_PARAM;
    }
    rte_error_t retval = RTE_ERR_UNDEFINE;
    tick_unit_t left_time = timeout_tick;
    HAL_DEVICE_TRY_LOCK(device, left_time, retval);
    retval = device->write_async(device, src_buf, buf_size, left_time);
    RTE_UNLOCK(&device->mutex);
    return retval;
}

#if RTE_USE_EXTERNAL_OS == 0 && RTE_USE_SUGAR_KERNEL == 0
rte_error_t hal_device_wait_rx_ready(hal_device_t *device, tick_unit_t timeout_tick)
{
    tick_unit_t start_time = rte_get_tick();
    while (device->if_rx_ready == false) {
        rte_yield();
        if (rte_time_consume(start_time) >= timeout_tick) {
            return RTE_ERR_TIMEOUT;
        }
    }
    return RTE_SUCCESS;
}

rte_error_t hal_device_wait_tx_ready(hal_device_t *device, tick_unit_t timeout_tick)
{
    tick_unit_t start_time = rte_get_tick();
    while (device->if_tx_ready == false) {
        rte_yield();
        if (rte_time_consume(start_time) >= timeout_tick) {
            return RTE_ERR_TIMEOUT;
        }
    }
    return RTE_SUCCESS;
}
#endif
