/**
 * @file rte.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-07-27
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __RTE_H
#define __RTE_H

/**
 * @brief General include.
 *
 */
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTE_VERSION                 "6.0.0"

#ifndef RTE_USE_EXTERNAL_OS
#define RTE_USE_EXTERNAL_OS         0
#endif

#ifndef RTE_USE_SUGAR_KERNEL
#define RTE_USE_SUGAR_KERNEL        1
#endif

#if RTE_USE_SUGAR_KERNEL
#ifndef RTE_SUGAR_KERNEL_FREQUENCY
#define RTE_SUGAR_KERNEL_FREQUENCY  100
#endif
#endif

#ifndef RTE_USE_LOG
#define RTE_USE_LOG                 1
#endif

// Recommended when ram is smaller than 16KB, and use
// first-fit method when enabled. If disabled,
// the mempool will use tlsf method */
#define RTE_USE_SIMPLY_MEMPOOL      0


#define RTE_MAX_TIMER_GROUP_SIZE    2

#ifndef RTE_TIMER_TICK_UNIT
#define tick_unit_t                 uint32_t
#define TIME_MAX_DELAY              UINT32_MAX
#endif

#ifndef RTE_MEMPOOL_SIZE
#define RTE_MEMPOOL_SIZE            512 * 1024
#endif

#ifndef RTE_MEMPOOL_ENABLE_DEBUG
#define RTE_MEMPOOL_ENABLE_DEBUG    1
#endif

/*
** Detect whether or not we are building for a 32- or 64-bit (LP/LLP)
** architecture. There is no reliable portable method at compile-time.
*/
#if defined (__alpha__) || defined (__ia64__) || defined (__x86_64__) \
	|| defined (_WIN64) || defined (__LP64__) || defined (__LLP64__)
#ifndef RTE_MEMPOOL_USE_64BIT
#define RTE_MEMPOOL_USE_64BIT       1
#endif
#else
#ifndef RTE_MEMPOOL_USE_64BIT
#define RTE_MEMPOOL_USE_64BIT       0
#endif
#endif

#define RTE_TIMER_CAPACITY          256

#ifndef RTE_SHELL_ENABLE
#define RTE_SHELL_ENABLE            1
#endif

#if RTE_USE_SUGAR_KERNEL && RTE_USE_EXTERNAL_OS
#error "You must choose only one kernel for SL_SDK!"
#endif

/**
 * @brief Some general typedef.
 *
 */
/* Typedef for retval */
typedef int8_t  rte_error_t;
#define RTE_SUCCESS                              0
#define RTE_ERR_UNDEFINE                        -1
#define RTE_ERR_TIMEOUT                         -2
#define RTE_ERR_NO_MEM                          -3
#define RTE_ERR_PARAM                           -4
#define RTE_ERR_NO_RSRC                         -5
#define RTE_ERR_MISMATCH                        -6
#define RTE_ERR_BAD_USE                         -7
#define RTE_ERR_TRY_LATER                       -8
#define RTE_ERR_OVERFLOW                        -9

/* Typedef for unused */
#ifndef RTE_UNUSED
#define RTE_UNUSED(x)                           (void)x
#endif
/* Typedef for memory align */
#define MEM_MODIFY_ALIGN_UP(ptr, n)             (((ptr) + ((n) - 1)) & ~((n) - 1))
#define MEM_MODIFY_ALIGN_DOWN(ptr, n)           ((ptr) & ~((n) - 1))
#define MEM_BLOCK_ALIGN                         (sizeof(void *) * 2)
#define MEM_ALIGN_BYTES (buf)                   buf __attribute__((aligned(MEM_BLOCK_ALIGN)))
#define MEM_ALIGN_NBYTES(buf, n)                buf __attribute__((aligned(n)))

/* Typedef for array's size */
#define ARRAY_SIZE(arr)                         (sizeof(arr) / sizeof((arr)[0]))

#define rte_likely(x)                           __builtin_expect(!!(x), 1)
#define rte_unlikely(x)                         __builtin_expect(!!(x), 0)

#define rte_offset_of(type, member)             offsetof(type, member)
#define rte_container_of(ptr, type, member)     ({const typeof( ((type *)0)->member ) *__mptr = (ptr);        \
                                                    (type *)( (char *)__mptr - offsetof(type,member) );})

#define rte_strdup(ptr, str)                    do{                                             \
                                                    if (str) {                                  \
                                                        ptr = rte_calloc(strlen(str) + 1);      \
                                                        memcpy(ptr, str, strlen(str));          \
                                                    }                                           \
                                                } while(0)

#define rte_max(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define rte_min(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

#define rte_cast(t, exp)    ((t)(exp))

typedef void *ds_vector_t;
typedef void *ds_ringbuffer_t;
typedef void *ds_framebuffer_t;
typedef void *ds_burstbuffer_t;
typedef void *ds_hashtable_t;
typedef void *ds_linklist_t;
typedef uint8_t timer_id_t;
typedef uint8_t timer_group_id_t;

typedef struct {
    void *mutex;
    rte_error_t (*lock)(void *mutex);
    rte_error_t (*trylock)(void *mutex, tick_unit_t timeout);
    rte_error_t (*unlock)(void *mutex);
} rte_mutex_t;

typedef struct {
    uint8_t *buf;
    uint16_t size;
} rte_buffer_t;

typedef struct {
    uint8_t source_id;
    uint8_t dest_id;
    uint16_t msg_id;
    uint16_t seq_id;
    uint16_t msg_len;
    uint8_t msg_buf[0];
} rte_msg_t;

static inline uint32_t rte_roundup_pow_of_two(uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

static inline rte_error_t rte_lock(rte_mutex_t *instance)
{
    if(instance && (instance->lock) && (instance->mutex))
        return (instance->lock)(instance->mutex);
    return RTE_ERR_PARAM;
}

static inline rte_error_t rte_try_lock(rte_mutex_t *instance, tick_unit_t timeout)
{
    if(instance && (instance->trylock) && (instance->mutex))
        return (instance->trylock)(instance->mutex, timeout);
    return RTE_ERR_PARAM;
}

static inline rte_error_t rte_unlock(rte_mutex_t *instance)
{
    if(instance && (instance->unlock) && (instance->mutex))
        return (instance->unlock)(instance->mutex);
    return RTE_ERR_PARAM;
}

#ifdef __cplusplus
}
#endif

#endif
