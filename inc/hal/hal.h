/**
 * @file hal.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-08-07
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __HAL_H
#define __HAL_H

#include "../rte_include.h"
#if __DCACHE_PRESENT
/*
    the SCB_CleanDCache_by_Addr() requires a 32-Byte aligned address
    adjust the address and the D-Cache size to clean accordingly.
*/
#define HAL_RAM_CLEAN_PRE_SEND(p, size)    do{                                                                                              \
                                                uint32_t alignedAddr = (uint32_t)p & (uint32_t)(~0x1F);                                     \
                                                SCB_CleanDCache_by_Addr((uint32_t*)alignedAddr, (int32_t)(size + ((uint32_t)p - alignedAddr)));        \
                                            }while(0)
/*
    the SCB_InvalidateDCache_by_Addr() requires a 32-Byte aligned address,
    adjust the address and the D-Cache size to invalidate accordingly.
*/
#define HAL_RAM_CLEAN_AFTER_REC(p, size)   do{                                                                                              \
                                                uint32_t alignedAddr = (uint32_t)p & (uint32_t)(~0x1F);                                     \
                                                SCB_InvalidateDCache_by_Addr((uint32_t*)alignedAddr, (int32_t)(size + ((uint32_t)p - alignedAddr)));   \
                                            }while(0)
#else
#define HAL_RAM_CLEAN_PRE_SEND(p, size)
#define HAL_RAM_CLEAN_AFTER_REC(p, size)
#endif

typedef uint8_t hal_device_id_t;
typedef uint8_t hal_operation_id_t;
struct __hal_device;
#define HAL_DEVICE_INVALID_OP               (hal_operation_id_t)0
#define HAL_DEVICE_READ_OP                  (hal_operation_id_t)1
#define HAL_DEVICE_READ_ASYNC_OP            (hal_operation_id_t)2
#define HAL_DEVICE_WRITE_OP                 (hal_operation_id_t)3
#define HAL_DEVICE_WRITE_ASYNC_OP           (hal_operation_id_t)5

typedef void (*hal_device_op_callback_f)(struct __hal_device *device, hal_operation_id_t op, void *arg);

typedef rte_error_t (*device_read_f)(struct __hal_device *device, uint8_t *dest_buf, uint32_t *buf_size, uint32_t timeout_ms);

typedef rte_error_t (*device_write_f)(struct __hal_device *device, uint8_t *src_buf, uint32_t buf_size, uint32_t timeout_ms);

typedef struct __hal_device {
    hal_device_id_t device_id;
    device_read_f read;
    device_read_f read_async;
    device_write_f write;
    device_write_f write_async;
    rte_mutex_t *mutex;
    void *user_arg;
    hal_device_op_callback_f op_callback;
} hal_device_t;

rte_error_t hal_device_read_sync(hal_device_t *device, uint8_t *dest_buf,
                                uint32_t *buf_size, uint32_t timeout_ms);

rte_error_t hal_device_write_sync(hal_device_t *device, uint8_t *src_buf,
                                uint32_t buf_size, uint32_t timeout_ms);

rte_error_t hal_device_read_async(hal_device_t *device, uint8_t *dest_buf,
                                uint32_t *buf_size, uint32_t timeout_ms);

rte_error_t hal_device_write_async(hal_device_t *device, uint8_t *src_buf,
                                uint32_t buf_size, uint32_t timeout_ms);

#endif
