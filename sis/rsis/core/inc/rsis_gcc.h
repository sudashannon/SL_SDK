/**
 * @file rsis_gcc.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-09-19
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __RSIS_GCC_H
#define __RSIS_GCC_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* Fallback for __has_builtin */
#ifndef __has_builtin
    #define __has_builtin(x) (0)
#endif

#ifndef   __ASM
    #define __ASM                                  __asm
#endif

#ifndef   __INLINE
    #define __INLINE                               inline
#endif

#ifndef   __FORCEINLINE
    #define __FORCEINLINE                          __attribute__((always_inline))
#endif

#ifndef   __STATIC_INLINE
    #define __STATIC_INLINE                        static inline
#endif

#ifndef   __STATIC_FORCEINLINE
    #define __STATIC_FORCEINLINE                   __attribute__((always_inline)) static inline
#endif

#ifndef   __NO_RETURN
    #define __NO_RETURN                            __attribute__((__noreturn__))
#endif

#ifndef   __DEPRECATED
    #define __DEPRECATED                           __attribute__((deprecated))
#endif

#ifndef   __USED
    #define __USED                                 __attribute__((used))
#endif

#ifndef   __WEAK
    #define __WEAK                                 __attribute__((weak))
#endif

#ifndef   __PACKED
    #define __PACKED                               __attribute__((packed, aligned(1)))
#endif

#ifndef   __PACKED_STRUCT
    #define __PACKED_STRUCT                        struct __attribute__((packed, aligned(1)))
#endif

#ifndef   __PACKED_UNION
    #define __PACKED_UNION                         union __attribute__((packed, aligned(1)))
#endif

#ifndef   __VECTOR_SIZE
    #define __VECTOR_SIZE(x)                       __attribute__((vector_size(x)))
#endif

#ifndef   __UNALIGNED_UINT16_WRITE
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpacked"
    #pragma GCC diagnostic ignored "-Wattributes"
    /** \brief Packed struct for unaligned uint16_t write access */
    __PACKED_STRUCT T_UINT16_WRITE {
        uint16_t v;
    };
    #pragma GCC diagnostic pop
    /** \brief Pointer for unaligned write of a uint16_t variable. */
    #define __UNALIGNED_UINT16_WRITE(addr, val)    (void)((((struct T_UINT16_WRITE *)(void *)(addr))->v) = (val))
#endif

#ifndef   __UNALIGNED_UINT16_READ
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpacked"
    #pragma GCC diagnostic ignored "-Wattributes"
    /** \brief Packed struct for unaligned uint16_t read access */
    __PACKED_STRUCT T_UINT16_READ {
        uint16_t v;
    };
    #pragma GCC diagnostic pop
    /** \brief Pointer for unaligned read of a uint16_t variable. */
    #define __UNALIGNED_UINT16_READ(addr)          (((const struct T_UINT16_READ *)(const void *)(addr))->v)
#endif

#ifndef   __UNALIGNED_UINT32_WRITE
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpacked"
    #pragma GCC diagnostic ignored "-Wattributes"
    /** \brief Packed struct for unaligned uint32_t write access */
    __PACKED_STRUCT T_UINT32_WRITE {
        uint32_t v;
    };
    #pragma GCC diagnostic pop
    /** \brief Pointer for unaligned write of a uint32_t variable. */
    #define __UNALIGNED_UINT32_WRITE(addr, val)    (void)((((struct T_UINT32_WRITE *)(void *)(addr))->v) = (val))
#endif

#ifndef   __UNALIGNED_UINT32_READ
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpacked"
    #pragma GCC diagnostic ignored "-Wattributes"
    /** \brief Packed struct for unaligned uint32_t read access */
    __PACKED_STRUCT T_UINT32_READ {
        uint32_t v;
    };
    #pragma GCC diagnostic pop
    /** \brief Pointer for unaligned read of a uint32_t variable. */
    #define __UNALIGNED_UINT32_READ(addr)          (((const struct T_UINT32_READ *)(const void *)(addr))->v)
#endif

#ifndef   __ALIGNED
    #define __ALIGNED(x)                           __attribute__((aligned(x)))
#endif

#ifndef   __RESTRICT
    #define __RESTRICT                             __restrict
#endif

#ifndef   __COMPILER_BARRIER
    #define __COMPILER_BARRIER()                   __ASM volatile("":::"memory")
#endif

#ifndef   __USUALLY
    #define __USUALLY(exp)                         __builtin_expect((exp), 1)
#endif

#ifndef   __RARELY
    #define __RARELY(exp)                          __builtin_expect((exp), 0)
#endif

#ifndef   __INTERRUPT
    #define __INTERRUPT                            __attribute__((interrupt))
#endif

/** \brief Defines 'read only' permissions */
#ifdef __cplusplus
    #define   __I     volatile
#else
    #define   __I     volatile const
#endif
/** \brief Defines 'write only' permissions */
#define     __O     volatile
/** \brief Defines 'read / write' permissions */
#define     __IO    volatile

/* following defines should be used for structure members */
/** \brief Defines 'read only' structure member permissions */
#define     __IM     volatile const
/** \brief Defines 'write only' structure member permissions */
#define     __OM     volatile
/** \brief Defines 'read/write' structure member permissions */
#define     __IOM    volatile

/**
 * \brief   Mask and shift a bit field value for use in a register bit range.
 * \details The macro \ref _VAL2FLD uses the #define's _Pos and _Msk of the related bit
 * field to shift bit-field values for assigning to a register.
 *
 * **Example**:
 * \code
 * ECLIC->CFG = _VAL2FLD(CLIC_CLICCFG_NLBIT, 3);
 * \endcode
 * \param[in] field  Name of the register bit field.
 * \param[in] value  Value of the bit field. This parameter is interpreted as an uint32_t type.
 * \return           Masked and shifted value.
 */
#define _VAL2FLD(field, value)    (((uint32_t)(value) << field ## _Pos) & field ## _Msk)

/**
 * \brief   Mask and shift a register value to extract a bit filed value.
 * \details The macro \ref _FLD2VAL uses the #define's _Pos and _Msk of the related bit
 * field to extract the value of a bit field from a register.
 *
 * **Example**:
 * \code
 * nlbits = _FLD2VAL(CLIC_CLICCFG_NLBIT, ECLIC->CFG);
 * \endcode
 * \param[in] field  Name of the register bit field.
 * \param[in] value  Value of register. This parameter is interpreted as an uint32_t type.
 * \return           Masked and shifted bit field value.
 */
#define _FLD2VAL(field, value)    (((uint32_t)(value) & field ## _Msk) >> field ## _Pos)

#ifdef __cplusplus
}
#endif

#endif
