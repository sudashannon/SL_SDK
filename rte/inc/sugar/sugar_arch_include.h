/**
 * @file sugar_arch_include.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2022-01-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __SUGAR_ARCH_INCLUDED_H
#define __SUGAR_ARCH_INCLUDED_H

#include "../middle_layer/rte.h"

#if RTE_USE_SUGAR_KERNEL
#if defined(__arm__)
    #if ((defined (__ARM_ARCH_7M__      ) && (__ARM_ARCH_7M__      == 1)) || \
        (defined (__ARM_ARCH_7EM__     ) && (__ARM_ARCH_7EM__     == 1)))
        #include "arch/arm_v7m/sugar_arch.h"
    #endif
#else
    #error "Does not support this arch!"
#endif
#endif

#endif /* __SUGAR_ARCH_INCLUDED_H */
