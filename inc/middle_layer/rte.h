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
#define RTE_USE_LVGL                0
#define RTE_USE_SFUD                0
#define RTE_USE_EASYFLASH           0
#define RTE_USE_LETTER_SHELL        0
#ifndef RTE_MEMPOOL_SIZE
#define RTE_MEMPOOL_SIZE            64 * 1024 * 1024
#endif
#ifndef RTE_MEMPOOL_USE_64BIT
#define RTE_MEMPOOL_USE_64BIT       1
#endif

#define RTE_TIMER_GROUP_CAPACITY    8

/**
 * @brief General include.
 *
 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
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
 * @brief Should be called firstly to ensure dynamic memory can be used.
 *
 */
extern void rte_init(void);
/**
 * @brief Deinit the rte.
 *
 * @return rte_error_t
 */
extern rte_error_t rte_deinit(void);
/**
 * @brief Get the general rte allocator.
 *
 * @return rte_allocator_t*
 */
extern rte_allocator_t *rte_get_general_allocator(void);
/**
 * @brief Wrapper for mutex lock, which is adapted for different OS.
 *
 * @param mutex
 * @return rte_error_t
 */
extern rte_error_t rte_mutex_lock(void *mutex);
/**
 * @brief Wrapper for mutex unlock, which is adapted for different OS.
 *
 * @param mutex
 * @return rte_error_t
 */
extern rte_error_t rte_mutex_unlock(void *mutex);
/**
 * @brief Get the main timer group.
 *
 * @return timer_group_id_t
 */
extern timer_group_id_t rte_get_main_timergroup(void);
/**
 * @brief Push a character into the shell buffer.
 *
 * @param data
 * @return ret_error_t
 */
extern rte_error_t rte_push_character_into_shell(char data);
#ifdef __cplusplus
}
#endif

#endif
