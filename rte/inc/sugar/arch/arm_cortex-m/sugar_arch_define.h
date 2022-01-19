/**
 * @file sugar_arch_offset.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-01-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __SUGAR_ARCH_OFFSET_H
#define __SUGAR_ARCH_OFFSET_H

#define ARCH_CTX_RUN_OFFSET     0
#define ARCH_CTX_NEXT_OFFSET    4
#define ARCH_CTX_REENT_OFFSET   8

/**
 * Create more readable defines for usable intruction set and FPU
 */
#define ARCH_CORTEX_M           (7U)
#define ARCH_WITH_FPU           1

/* Size of each stack entry / stack alignment size */
#if ARCH_WITH_FPU == 0
#define ARCH_STACK_ALIGN_SIZE    4      /*  (4 bytes on Cortex-M without FPU) */
#else
#define ARCH_STACK_ALIGN_SIZE    8      /*  (8 bytes on Cortex-M with FPU) */
#endif

#endif