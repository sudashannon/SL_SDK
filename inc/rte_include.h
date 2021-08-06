#ifndef __RTE_INCLUDE_H
#define __RTE_INCLUDE_H

#include "middle_layer/rte.h"
#include "middle_layer/rte_log.h"
#include "middle_layer/rte_memory.h"
#include "middle_layer/rte_atomic.h"
#include "middle_layer/rte_timer.h"
#include "data_structure/ds_vector.h"
#include "data_structure/ds_ringbuffer.h"

#define COMMON_MODULE LOG_STR(RTE)
#define RTE_LOGF(...) LOG_FATAL(COMMON_MODULE, __VA_ARGS__)
#define RTE_LOGE(...) LOG_ERROR(COMMON_MODULE, __VA_ARGS__)
#define RTE_LOGI(...) LOG_INFO(COMMON_MODULE, __VA_ARGS__)
#define RTE_LOGW(...) LOG_WARN(COMMON_MODULE, __VA_ARGS__)
#define RTE_LOGD(...) LOG_DEBUG(COMMON_MODULE, __VA_ARGS__)
#define RTE_LOGV(...) LOG_VERBOSE(COMMON_MODULE, __VA_ARGS__)
#define RTE_ASSERT(v) LOG_ASSERT(COMMON_MODULE, v)

#endif
