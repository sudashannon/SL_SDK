/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief configuration file
 */

#ifndef _FDB_CFG_H_
#define _FDB_CFG_H_

#include "rte_include.h"

/* using KVDB feature */
#define FDB_USING_KVDB

#ifdef FDB_USING_KVDB
/* Auto update KV to latest default when current KVDB version number is changed. @see fdb_kvdb.ver_num */
/* #define FDB_KV_AUTO_UPDATE */
#endif

/* using TSDB (Time series database) feature */
//#define FDB_USING_TSDB

/* Using FAL storage mode or file storage mode using posix and lib */
#define FDB_USING_FAL_MODE

#if defined(FDB_USING_FILE_LIBC_MODE) || defined(FDB_USING_FILE_POSIX_MODE)
#define FDB_USING_FILE_MODE
#endif

#ifdef FDB_USING_FAL_MODE
/* the flash write granularity, unit: bit
 * only support 1(nor flash)/ 8(stm32f2/f4)/ 32(stm32f1) */
 /* @note you must define it for a value */
#define FDB_WRITE_GRAN                      1
#endif

/* print debug information */
#define FDB_DEBUG_ENABLE

/* software version number */
#define FDB_SW_VERSION                 "1.1.1"
#define FDB_SW_VERSION_NUM             0x10101

/* the KV max name length must less then it */
#ifndef FDB_KV_NAME_MAX
#define FDB_KV_NAME_MAX                64
#endif

/* the KV cache table size, it will improve KV search speed when using cache */
#ifndef FDB_KV_CACHE_TABLE_SIZE
#define FDB_KV_CACHE_TABLE_SIZE        64
#endif

/* the sector cache table size, it will improve KV save speed when using cache */
#ifndef FDB_SECTOR_CACHE_TABLE_SIZE
#define FDB_SECTOR_CACHE_TABLE_SIZE    4
#endif

#if (FDB_KV_CACHE_TABLE_SIZE > 0) && (FDB_SECTOR_CACHE_TABLE_SIZE > 0)
#define FDB_KV_USING_CACHE
#endif

/* assert for developer. */
#define FDB_ASSERT                      RTE_ASSERT
/* debug level log */
#define FDB_DEBUG(...)                  log_out(LOG_LEVEL_DEBUG, __FILE__, __func__, __LINE__, "FDB", __VA_ARGS__)
/* info level log */
#define FDB_INFO(...)                   log_out(LOG_LEVEL_INFO, __FILE__, __func__, __LINE__, "FDB", __VA_ARGS__)

#define FDB_PRINT                       shell_printf

#endif /* _FDB_CFG_H_ */
