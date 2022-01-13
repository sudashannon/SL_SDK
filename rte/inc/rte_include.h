#ifndef __RTE_INCLUDE_H
#define __RTE_INCLUDE_H

#include "middle_layer/rte.h"
#include "middle_layer/rte_log.h"
#include "middle_layer/rte_memory.h"
#include "middle_layer/rte_atomic.h"
#include "middle_layer/rte_timer.h"
#include "middle_layer/rte_shell.h"
#include "data_structure/ds_vector.h"
#include "data_structure/ds_ringbuffer.h"
#include "data_structure/ds_hashmap.h"
#include "data_structure/ds_linklist.h"
#include "data_structure/ds_rbtree.h"
#include "image_process/image_process.h"
#include "sugar/sugar_kernel.h"
#include "sugar/sugar_scheduler.h"
#include "sugar/sugar_queue.h"

#define COMMON_MODULE LOG_STR(RTE)
#define RTE_LOGF(...) LOG_FATAL(COMMON_MODULE, __VA_ARGS__)
#define RTE_LOGE(...) LOG_ERROR(COMMON_MODULE, __VA_ARGS__)
#define RTE_LOGI(...) LOG_INFO(COMMON_MODULE, __VA_ARGS__)
#define RTE_LOGW(...) LOG_WARN(COMMON_MODULE, __VA_ARGS__)
#define RTE_LOGD(...) LOG_DEBUG(COMMON_MODULE, __VA_ARGS__)
#define RTE_LOGV(...) LOG_VERBOSE(COMMON_MODULE, __VA_ARGS__)
#define RTE_ASSERT(v) LOG_ASSERT(COMMON_MODULE, v)

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
 * @brief Wrapper for mutex lock, which is adapted for different OS.
 *
 * @param mutex
 * @return rte_error_t
 */
extern rte_error_t rte_mutex_lock(void *mutex);
/**
 * @brief Wrapper for mutex try lock, which is adapted for CMSIS-RTOS2.
 *
 * @param mutex
 * @return rte_error_t
 */
extern rte_error_t rte_mutex_trylock(void *mutex, uint32_t timeout_ms);
/**
 * @brief Wrapper for mutex unlock, which is adapted for different OS.
 *
 * @param mutex
 * @return rte_error_t
 */
extern rte_error_t rte_mutex_unlock(void *mutex);

#endif
