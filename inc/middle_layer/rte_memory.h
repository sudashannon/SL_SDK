/**
 * @file rte_memory.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-07-27
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __RTE_MEMPOOL_H
#define __RTE_MEMPOOL_H

#include "rte.h"
/**
 * @brief This enum defines how many bank the RTE_Memory will handle.
 *
 */
typedef int8_t mem_bank_t;
enum {
    BANK_INVALID = -1,
    BANK_DEFAULT = 0,
    BANK_DMA = 1,
    BANK_FB = 2,
    BANK_MATH = 3,
    BANK_CNT,
};

/* For include header in CPP code */
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Initialize a memory bank as a memory pool.
 *
 * @param bank
 * @param mutex
 * @param mem_pool
 * @param mem_pool_size
 */
void memory_pool(mem_bank_t bank, rte_mutex_t *mutex, void *mem_pool, size_t mem_pool_size);
/**
 * @brief Alloc a size of memory stack.
 *
 * @param bank
 * @param size
 * @return void*
 */
void* memory_alloc(mem_bank_t bank, size_t size);
/**
 * @brief Alloc a size of memory stack and set it to zero.
 *
 * @param bank
 * @param size
 * @return void*
 */
void *memory_calloc(mem_bank_t bank, size_t size);
/**
 * @brief Alloc a size of aligned memory stack.
 *
 * @param bank
 * @param align
 * @param size
 * @return void*
 */
void* memory_alloc_align(mem_bank_t bank, size_t align, size_t size);
/**
 * @brief Free a allocated memory stack.
 *
 * @param bank
 * @param ptr
 */
void memory_free(mem_bank_t bank,void* ptr);
/**
 * @brief Realloc a malloced buffer.
 *
 * @param bank
 * @param ptr
 * @param size
 * @return void*
 */
void* memory_realloc(mem_bank_t bank, void* ptr, size_t size);
/**
 * @brief Get a malloced buffer's size.
 *
 * @param ptr
 * @return size_t
 */
size_t memory_sizeof_p(void *ptr);

/**
 * @brief Get a free size of a memory bank.
 *
 * @param bank
 * @return size_t
 */
size_t memory_sizeof_free(mem_bank_t bank);
/**
 * @brief Get a max size of a memory bank.
 *
 * @param bank
 * @return size_t
 */
size_t memory_sizeof_max(mem_bank_t bank);
/**
 * @brief Malloc a max free size of a memory bank.
 *
 * @param bank
 * @param size
 * @return void*
 */
void *memory_alloc_max(mem_bank_t bank,size_t *size);
/**
 * @brief Demon a bank of memory stack.
 *
 * @param ptr
 * @return size_t
 */
void memory_demon(mem_bank_t bank);

/* For include header in CPP code */
#ifdef __cplusplus
}
#endif

#endif
