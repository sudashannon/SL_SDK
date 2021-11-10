/**
 * @file rsis_compiler.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-09-19
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __RSIS_COMPILER_H
#define __RSIS_COMPILER_H

#if defined (__GNUC__)
    #include "rsis_gcc.h"
#else
    #error Please use correct compiler!
#endif

#endif
