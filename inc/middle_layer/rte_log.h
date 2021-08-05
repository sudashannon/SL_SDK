/**
 * @file rte_log.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-07-27
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __RTE_LOG_H
#define __RTE_LOG_H

#include "rte.h"
/**
 * @brief Log Configuration.
 *
 */
#define LOG_MAX_HEAD_LENGTH     128         //
#define LOG_MAX_FILTER_CNT      8           //
/* Define logging levels */
typedef uint8_t log_level_t;
enum {
    LOG_LEVEL_FATAL     =  0,    // A fatal error has occurred: program will exit immediately
    LOG_LEVEL_ERROR     =  1,    // An error has occurred: program may not exit
    LOG_LEVEL_INFO      =  2,    // Necessary information regarding program operation
    LOG_LEVEL_WARN      =  3,    // Any circumstance that may not affect normal operation
    LOG_LEVEL_DEBUG     =  4,    // Standard debug messages
    LOG_LEVEL_VERBOSE   =  5,    // All debug messages
};
typedef uint16_t log_format_t;
enum {
    LOG_FMT_FUNCTION    =  0x0001,    //
    LOG_FMT_LINE        =  0x0002,    //
    LOG_FMT_FILE        =  0x0004,    //
    LOG_FMT_TIME        =  0x0008,    //
    LOG_FMT_DEFAULT     =  0x000B,    //
    LOG_FMT_FULL        =  0x000F,    //
};
typedef uint8_t log_command_t;
enum{
    LOG_CMD_ENABLE      = 0,    //
    LOG_CMD_DISABLE     = 1,    //
    LOG_CMD_LEVEL_UP    = 2,    //
    LOG_CMD_LEVEL_DOWN  = 3,    //
    LOG_CMD_SET_LEVEL   = 4,    //
    LOG_CMD_GET_LEVEL   = 5,    //
    LOG_CMD_SET_FORMAT  = 6,    //
    LOG_CMD_GET_FORMAT  = 7,    //
    LOG_CMD_ADD_FILTER  = 8,    //
    LOG_CMD_RMV_FILTER  = 9,    //
    LOG_CMD_SET_OUTPUT  = 10,   //
};
/* Define colors for printing to terminal */
#define COL_FATAL   "\x1B[31m"  // Red
#define COL_ERROR   "\x1B[91m"  // Light Red
#define COL_INFO    "\x1B[37m"  // White
#define COL_WARN    "\x1B[33m"  // Yellow
#define COL_DEBUG   "\x1B[94m"  // Light Blue
#define COL_VERBOSE "\x1B[36m"  // Cyan
#define CLR_RESET   "\033[0m"
typedef size_t (*log_output_f)(uint8_t *data, size_t length);
typedef uint32_t (*log_get_tick_f)(void);
#define LOG_STR(R)  #R
#if RTE_USE_LOG
#define LOG_FATAL(MODULE, ...) log_out(LOG_LEVEL_FATAL, __FILE__, __func__, __LINE__, MODULE, __VA_ARGS__)
#define LOG_ERROR(MODULE, ...) log_out(LOG_LEVEL_ERROR, __FILE__, __func__, __LINE__, MODULE, __VA_ARGS__)
#define LOG_INFO(MODULE, ...) log_out(LOG_LEVEL_INFO, __FILE__, __func__, __LINE__, MODULE, __VA_ARGS__)
#define LOG_WARN(MODULE, ...) log_out(LOG_LEVEL_WARN, __FILE__, __func__, __LINE__, MODULE, __VA_ARGS__)
#define LOG_DEBUG(MODULE, ...) log_out(LOG_LEVEL_DEBUG, __FILE__, __func__, __LINE__, MODULE, __VA_ARGS__)
#define LOG_VERBOSE(MODULE, ...) log_out(LOG_LEVEL_VERBOSE, __FILE__, __func__, __LINE__, MODULE, __VA_ARGS__)
#else
#define LOG_FATAL(MODULE, ...)
#define LOG_ERROR(MODULE, ...)
#define LOG_INFO(MODULE, ...)
#define LOG_WARN(MODULE, ...)
#define LOG_DEBUG(MODULE, ...)
#define LOG_VERBOSE(MODULE, ...)
#endif
#define LOG_ASSERT(MODULE, v)   do{														        \
                                    if(!(v)) {											        \
                                        LOG_FATAL(MODULE, "assert [%s] fail! at %s %d", LOG_STR(v), __FILE__, __LINE__);     \
                                        while(1);                                               \
                                    }													        \
						        }while(0)

/* For include header in CPP code */
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Init for log system, do not have dynamic memory allocation.
 *
 * @param mutex
 * @param out_func
 * @param get_tick_func
 * @return rte_error_t
 */
extern rte_error_t log_init(rte_mutex_t *mutex, log_output_f out_func, log_get_tick_f get_tick_func);
/**
 * @brief Wrapped log output function.
 *
 * @param level
 * @param file
 * @param function
 * @param line
 * @param module
 * @param msg
 * @param ...
 * @return size_t
 */
extern size_t log_out(log_level_t level, const char *file, const char *function, int line,
                        const char *module, const char *msg, ...);
/**
 * @brief Log control function, provide much function to control log handle.
 *
 * @param command
 * @param param
 * @return rte_error_t
 */
extern rte_error_t log_control(log_command_t command, void *param);
/**
 * @brief Translate timetick in us into general struct tm.
 *
 * @param timer
 * @param tm
 */
extern void log_sec2time(time_t timetick, struct tm *tm);
/**
 * @brief Provide simple snprintf function, do not support '%f'.
 *
 * @param s
 * @param n
 * @param format
 * @param ...
 * @return int
 */
extern int log_snprintf(char * restrict s, size_t n, const char * restrict format, ...);
/**
 * @brief Provide simple sprintf function, do not support '%f'.
 *
 * @param s
 * @param format
 * @param ...
 * @return int
 */
extern int log_sprintf(char * restrict s, const char *format, ...);
/* For include header in CPP code */
#ifdef __cplusplus
}
#endif

#endif
