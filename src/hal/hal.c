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

#include "../../inc/hal/hal.h"

#define HAL_DEVICE_TRY_LOCK(device, timeout_ms, retval)             \
    if (device->mutex) {                                            \
        uint32_t start_tick = rte_get_tick();                       \
        uint32_t consumed_time = rte_tick_consume(start_tick);      \
        RTE_TRYLOCK(device->mutex, timeout_ms, retval);             \
        if (retval != RTE_SUCCESS)                                  \
            return retval;                                          \
        if (consumed_time >= left_time) {                           \
            RTE_UNLOCK(device->mutex);                              \
            return RTE_ERR_TIMEOUT;                                 \
        }                                                           \
        left_time -= consumed_time;                                 \
    }

rte_error_t hal_device_read_sync(hal_device_t *device, uint8_t *dest_buf,
                                uint32_t *buf_size, uint32_t timeout_ms)
{
    if (RTE_UNLIKELY(device == NULL) ||
        RTE_UNLIKELY(dest_buf == NULL) ||
        RTE_UNLIKELY(device->read == NULL)) {
        return RTE_ERR_PARAM;
    }
    rte_error_t retval = RTE_ERR_UNDEFINE;
    uint32_t left_time = timeout_ms;
    HAL_DEVICE_TRY_LOCK(device, left_time, retval);
    retval = device->read(device, dest_buf, buf_size, left_time);
    RTE_UNLOCK(device->mutex);
    return retval;
}

rte_error_t hal_device_write_sync(hal_device_t *device, uint8_t *src_buf,
                                        uint32_t buf_size, uint32_t timeout_ms)
{
    if (RTE_UNLIKELY(device == NULL) ||
        RTE_UNLIKELY(src_buf == NULL) ||
        RTE_UNLIKELY(device->write == NULL)) {
        return RTE_ERR_PARAM;
    }
    rte_error_t retval = RTE_ERR_UNDEFINE;
    uint32_t left_time = timeout_ms;
    HAL_DEVICE_TRY_LOCK(device, left_time, retval);
    retval = device->write(device, src_buf, buf_size, left_time);
    RTE_UNLOCK(device->mutex);
    return retval;
}

rte_error_t hal_device_read_async(hal_device_t *device, uint8_t *dest_buf,
                                        uint32_t *buf_size, uint32_t timeout_ms)
{
    if (RTE_UNLIKELY(device == NULL) ||
        RTE_UNLIKELY(dest_buf == NULL) ||
        RTE_UNLIKELY(device->read_async == NULL)) {
        return RTE_ERR_PARAM;
    }
    rte_error_t retval = RTE_ERR_UNDEFINE;
    uint32_t left_time = timeout_ms;
    HAL_DEVICE_TRY_LOCK(device, left_time, retval);
    retval = device->read_async(device, dest_buf, buf_size, left_time);
    RTE_UNLOCK(device->mutex);
    return retval;
}

rte_error_t hal_device_write_async(hal_device_t *device, uint8_t *src_buf,
                                        uint32_t buf_size, uint32_t timeout_ms)
{
    if (RTE_UNLIKELY(device == NULL) ||
        RTE_UNLIKELY(src_buf == NULL) ||
        RTE_UNLIKELY(device->write_async == NULL)) {
        return RTE_ERR_PARAM;
    }
    rte_error_t retval = RTE_ERR_UNDEFINE;
    uint32_t left_time = timeout_ms;
    HAL_DEVICE_TRY_LOCK(device, left_time, retval);
    retval = device->write_async(device, src_buf, buf_size, left_time);
    RTE_UNLOCK(device->mutex);
    return retval;
}
