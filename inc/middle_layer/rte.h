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

#define RTE_VERSION                 "5.0.1"
#define RTE_USE_LOG                 1
// Recommended when ram is smaller than 16KB, and use
// first-fit method when enabled. If disabled,
// the mempool will use tlsf method */
#define RTE_USE_SIMPLY_MEMPOOL      0
#ifndef RTE_MEMPOOL_SIZE
#define RTE_MEMPOOL_SIZE            64 * 1024 * 1024
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
#endif

#define RTE_TIMER_GROUP_CAPACITY    8

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

/* Typedef for unused */
#ifndef RTE_UNUSED
#define RTE_UNUSED(x)                           (void)x
#endif
/* Typedef for memory align */
#define MEM_BLOCK_ALIGN         (sizeof(void *) * 2)
#define MEM_ALIGN_BYTES (buf)    buf __attribute__((aligned(MEM_BLOCK_ALIGN)))
#define MEM_ALIGN_NBYTES(buf, n) buf __attribute__((aligned(n)))
/* Typedef for array's size */
#define ARRAY_SIZE(arr)         (sizeof(arr) / sizeof((arr)[0]))

#define RTE_LIKELY(x)           __builtin_expect(!!(x), 1)
#define RTE_UNLIKELY(x)         __builtin_expect(!!(x), 0)

#define RTE_CONTAINER_OF(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);        \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define RTE_OFFSET_OF(type, member) offsetof(type, member)

/* Typedef for mutex */
#define RTE_LOCK(v)         do{                                                          \
                                if((v) && ((v)->lock) && ((v)->mutex))                   \
                                    (((v)->lock))((v)->mutex);                           \
                            }while(0)

#define RTE_TRYLOCK(v, t)   (((v)->trylock))((v)->mutex, t)

#define RTE_UNLOCK(v)       do{                                                          \
                                if((v) && ((v)->unlock) && ((v)->mutex))                 \
                                    (((v)->unlock))((v)->mutex);                         \
                            }while(0)

#define RTE_MAX(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define RTE_MIN(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define RTE_DIV(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a / _b) : 0; })
#define RTE_MOD(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a % _b) : 0; })

#define RTE_LOG2_2(x)    (((x) &                0x2ULL) ? ( 2                        ) :             1) // NO ({ ... }) !
#define RTE_LOG2_4(x)    (((x) &                0xCULL) ? ( 2 +  RTE_LOG2_2((x) >>  2)) :  RTE_LOG2_2(x)) // NO ({ ... }) !
#define RTE_LOG2_8(x)    (((x) &               0xF0ULL) ? ( 4 +  RTE_LOG2_4((x) >>  4)) :  RTE_LOG2_4(x)) // NO ({ ... }) !
#define RTE_LOG2_16(x)   (((x) &             0xFF00ULL) ? ( 8 +  RTE_LOG2_8((x) >>  8)) :  RTE_LOG2_8(x)) // NO ({ ... }) !
#define RTE_LOG2_32(x)   (((x) &         0xFFFF0000ULL) ? (16 + RTE_LOG2_16((x) >> 16)) : RTE_LOG2_16(x)) // NO ({ ... }) !
#define RTE_LOG2(x)      (((x) & 0xFFFFFFFF00000000ULL) ? (32 + RTE_LOG2_32((x) >> 32)) : RTE_LOG2_32(x)) // NO ({ ... }) !

#define RTE_MAX(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define RTE_MIN(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define RTE_DIV(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a / _b) : 0; })
#define RTE_MOD(a,b)     ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _b ? (_a % _b) : 0; })

#define INT8_T_BITS     (sizeof(int8_t) * 8)
#define INT8_T_MASK     (INT8_T_BITS - 1)
#define INT8_T_SHIFT    RTE_LOG2(INT8_T_MASK)

#define INT16_T_BITS    (sizeof(int16_t) * 8)
#define INT16_T_MASK    (INT16_T_BITS - 1)
#define INT16_T_SHIFT   RTE_LOG2(INT16_T_MASK)

#define INT32_T_BITS    (sizeof(int32_t) * 8)
#define INT32_T_MASK    (INT32_T_BITS - 1)
#define INT32_T_SHIFT   RTE_LOG2(INT32_T_MASK)

#define INT64_T_BITS    (sizeof(int64_t) * 8)
#define INT64_T_MASK    (INT64_T_BITS - 1)
#define INT64_T_SHIFT   RTE_LOG2(INT64_T_MASK)

#define UINT8_T_BITS    (sizeof(uint8_t) * 8)
#define UINT8_T_MASK    (UINT8_T_BITS - 1)
#define UINT8_T_SHIFT   RTE_LOG2(UINT8_T_MASK)

#define UINT16_T_BITS   (sizeof(uint16_t) * 8)
#define UINT16_T_MASK   (UINT16_T_BITS - 1)
#define UINT16_T_SHIFT  RTE_LOG2(UINT16_T_MASK)

#define UINT32_T_BITS   (sizeof(uint32_t) * 8)
#define UINT32_T_MASK   (UINT32_T_BITS - 1)
#define UINT32_T_SHIFT  RTE_LOG2(UINT32_T_MASK)

#define UINT64_T_BITS   (sizeof(uint64_t) * 8)
#define UINT64_T_MASK   (UINT64_T_BITS - 1)
#define UINT64_T_SHIFT  RTE_LOG2(UINT64_T_MASK)

typedef void (*rte_callback_f)(void *arg);

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

#ifdef __cplusplus
extern "C" {
#endif

typedef void *ds_vector_t;
typedef void *ds_ringbuffer_t;
typedef void *ds_framebuffer_t;
typedef void *ds_burstbuffer_t;
typedef void *ds_image_t;
typedef uint8_t timer_id_t;
typedef uint8_t timer_group_id_t;

typedef struct {
    void *mutex;
    rte_error_t (*lock)(void *mutex);
    rte_error_t (*trylock)(void *mutex, uint32_t timeout_ms);
    rte_error_t (*unlock)(void *mutex);
} rte_mutex_t;

typedef struct {
    uint8_t *buf;
    uint16_t size;
} rte_buffer_t;

typedef struct {
    void* (*malloc)(uint32_t size);
    void* (*calloc)(uint32_t size);
    void* (*realloc)(void *ptr, uint32_t size);
    void (*free)(void *ptr);
} rte_allocator_t;

typedef struct {
    uint8_t source_id;
    uint8_t dest_id;
    uint16_t msg_id;
    uint16_t seq_id;
    uint16_t msg_len;
    uint8_t msg_buf[0];
} rte_msg_t;

/**
 * @brief Get the general rte allocator.
 *
 * @return rte_allocator_t*
 */
extern rte_allocator_t *rte_get_general_allocator(void);

#ifdef __cplusplus
}
#endif

#endif
