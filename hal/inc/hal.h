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

#include <stdio.h>
#include "rte_include.h"

#if RTE_USE_OS
#include <cmsis_os2.h>
#endif

#if __DCACHE_PRESENT
/*
    the SCB_CleanDCache_by_Addr() requires a 32-Byte aligned address
    adjust the address and the D-Cache size to clean accordingly.
*/
#define HAL_RAM_CLEAN_PRE_SEND(p, size)    do{                                            \
                                                RTE_ASSERT(((uint32_t)p % 32) == 0);      \
                                                SCB_CleanDCache_by_Addr((uint32_t *)p, size);         \
                                            }while(0)
/*
    the SCB_InvalidateDCache_by_Addr() requires a 32-Byte aligned address,
    adjust the address and the D-Cache size to invalidate accordingly.
*/
#define HAL_RAM_CLEAN_AFTER_REC(p, size)   do{                                           \
                                                RTE_ASSERT(((uint32_t)p % 32) == 0);     \
                                                SCB_InvalidateDCache_by_Addr((uint32_t *)p, size);   \
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

typedef rte_error_t (*device_read_f)(struct __hal_device *device, uint8_t *dest_buf, uint32_t *buf_size, tick_unit_t timeout_tick);

typedef rte_error_t (*device_write_f)(struct __hal_device *device, uint8_t *src_buf, uint32_t buf_size, tick_unit_t timeout_tick);

typedef void (*hal_device_constructor_t)(void);
typedef void (*hal_device_destructor_t)(void);

/*! @def HAL_CONSTRUCTOR
 * @brief Define a Metal constructor
 *
 * Functions defined with HAL_CONSTRUCTOR will be added to the list of
 * Metal constructors. By default, these functions are called before main by
 * the metal_init() function.
 */
#if defined (__IAR_SYSTEMS_ICC__)
#define HAL_DECLARE_CONSTRUCTOR(function_name)                                       \
    void function_name(void);                                                \
    __root @("hal_constructors")                                             \
        const hal_device_constructor_t hal_constructors_##function_name##_ptr = &function_name;    \
    void function_name(void)
#else
#define HAL_DECLARE_CONSTRUCTOR(function_name)                                       \
    void function_name(void);                                                \
    __attribute__((used)) __attribute__((section("hal_constructors")))       \
        const hal_device_constructor_t hal_constructors_##function_name##_ptr = &function_name;    \
    void function_name(void)

#endif

/*! @def HAL_DESTRUCTOR
 * @brief Define a Metal destructor
 *
 * Functions defined with HAL_DESTRUCTOR will be added to the list of
 * Metal destructors. By default, these functions are called on exit by
 * the metal_fini() function.
 */
#if defined (__IAR_SYSTEMS_ICC__)
#define HAL_DECLARE_DESTRUCTOR(function_name)                                        \
    void function_name(void);                                                \
    __root @("hal_constructors")                                             \
    __attribute__((used)) __attribute__((section("hal_destructors")))        \
        const hal_device_destructor_t hal_destructors_##function_name##_ptr = &function_name;     \
    void function_name(void)
#else
#define HAL_DECLARE_DESTRUCTOR(function_name)                                        \
    void function_name(void);                                                \
    __attribute__((used)) __attribute__((section("hal_destructors")))        \
        const hal_device_destructor_t hal_destructors_##function_name##_ptr = &function_name;     \
    void function_name(void)
#endif
typedef struct __hal_device {
    hal_device_id_t device_id;
    rte_mutex_t mutex;
    device_read_f read;
    device_read_f read_async;
    device_write_f write;
    device_write_f write_async;
#if RTE_USE_OS
    osSemaphoreId_t rx_sema;
    osSemaphoreId_t tx_sema;
#else
    bool if_rx_ready;
    bool if_tx_ready;
#endif
    hal_device_op_callback_f op_callback;
    void *user_arg;
} hal_device_t;

void *hal_get_device_table(void);

hal_device_t *hal_get_device(const char *device_name);

rte_error_t hal_device_read_sync(char *device_name, uint8_t *dest_buf,
                                uint32_t *buf_size, tick_unit_t timeout_tick);

rte_error_t hal_device_write_sync(char *device_name, uint8_t *src_buf,
                                uint32_t buf_size, tick_unit_t timeout_tick);

rte_error_t hal_device_read_async(char *device_name, uint8_t *dest_buf,
                                uint32_t *buf_size, tick_unit_t timeout_tick);

rte_error_t hal_device_write_async(char *device_name, uint8_t *src_buf,
                                uint32_t buf_size, tick_unit_t timeout_tick);

#if RTE_USE_OS

#define hal_device_prepare_wait(device, op)

#define hal_device_wait_rx_ready(device, timeout_tick)                                \
    osSemaphoreAcquire((device)->rx_sema, timeout_tick)

#define hal_device_wait_tx_ready(device, timeout_tick)                                \
    osSemaphoreAcquire((device)->tx_sema, timeout_tick)

#define hal_device_active(device, op)                                               \
    osSemaphoreRelease((device)->op##_sema)

#define hal_device_initialize(device_type, name,                                    \
                                read_f, write_f,                                    \
                                read_async_f, write_async_f, device_ptr)            \
    osMutexAttr_t mutex_attr = {                                                    \
        LOG_STR(name),                                                              \
        osMutexPrioInherit | osMutexRecursive,                                      \
        NULL,                                                                       \
        0U                                                                          \
    };                                                                              \
    device_type##_control_handle[name].device.mutex.mutex = osMutexNew(&mutex_attr);\
    RTE_ASSERT(device_type##_control_handle[name].device.mutex.mutex);              \
    device_type##_control_handle[name].device.mutex.lock = rte_mutex_lock;          \
    device_type##_control_handle[name].device.mutex.unlock = rte_mutex_unlock;      \
    device_type##_control_handle[name].device.mutex.trylock = rte_mutex_trylock;    \
    device_type##_control_handle[name].device.device_id = name;                     \
    device_type##_control_handle[name].device.read = read_f;                        \
    device_type##_control_handle[name].device.write = write_f;                      \
    device_type##_control_handle[name].device.read_async = read_async_f;            \
    device_type##_control_handle[name].device.write_async = write_async_f;          \
    device_type##_control_handle[name].device.rx_sema = osSemaphoreNew(1, 0, NULL); \
    RTE_ASSERT(device_type##_control_handle[name].device.rx_sema);                  \
    device_type##_control_handle[name].device.tx_sema = osSemaphoreNew(1, 0, NULL); \
    RTE_ASSERT(device_type##_control_handle[name].device.tx_sema);                  \
    device_type##_control_handle[name].device.op_callback = NULL;                   \
    device_type##_control_handle[name].device.user_arg = NULL;                      \
    *device_ptr = &device_type##_control_handle[name].device

#else

#define hal_device_prepare_wait(device, op)                                         \
    (device)->if_##op##_ready = false

extern rte_error_t hal_device_wait_rx_ready(hal_device_t *device, tick_unit_t timeout_tick);
extern rte_error_t hal_device_wait_tx_ready(hal_device_t *device, tick_unit_t timeout_tick);

#define hal_device_active(device, op)                                               \
    (device)->if_##op##_ready = true

#define hal_device_initialize(device_type, name,                                    \
                                read_f, write_f,                                    \
                                read_async_f, write_async_f, device_ptr)            \
    device_type##_control_handle[name].device.mutex.mutex = NULL;                   \
    device_type##_control_handle[name].device.mutex.lock = NULL;                    \
    device_type##_control_handle[name].device.mutex.unlock = NULL;                  \
    device_type##_control_handle[name].device.mutex.trylock = NULL;                 \
    device_type##_control_handle[name].device.device_id = name;                     \
    device_type##_control_handle[name].device.read = read_f;                        \
    device_type##_control_handle[name].device.write = write_f;                      \
    device_type##_control_handle[name].device.read_async = read_async_f;            \
    device_type##_control_handle[name].device.write_async = write_async_f;          \
    device_type##_control_handle[name].device.if_rx_ready = false;                  \
    device_type##_control_handle[name].device.if_tx_ready = false;                  \
    device_type##_control_handle[name].device.op_callback = NULL;                   \
    device_type##_control_handle[name].device.user_arg = NULL;                      \
    *device_ptr = &device_type##_control_handle[name].device

#endif

#define hal_device_poll(device_type)    for (device_type##_name_t this_device = 0; this_device < device_type##_N; this_device++)

#define hal_device_register(device_type, user_prefix)                                 \
    hal_device_poll(device_type) {                                                  \
        char device_name[64] = {0};                                                 \
        hal_device_t *device = NULL;                                                \
        snprintf(device_name, sizeof(device_name), "%s_%d",                         \
                    user_prefix ? user_prefix : LOG_STR(device_type), this_device); \
        device_type##_create(this_device, &device);                                 \
        ht_set_if_not_exists(hal_get_device_table(), device_name,                   \
                                strlen(device_name), device,                        \
                                sizeof(hal_device_t *));                            \
    }

#define hal_device_unregister(device_type)                                            \
    hal_device_poll(device_type) {                                                  \
        device_type##_destroy(this_device);                                         \
    }

#endif
