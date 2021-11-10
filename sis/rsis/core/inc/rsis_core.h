/**
 * @file rsis_core.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0
 * @date 2021-09-19
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __RSIS_CORE_H
#define __RSIS_CORE_H

#include "rsis_csr_address.h"
#include "rsis_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __riscv_xlen == 64
typedef uint64_t               rv_csr_t;
typedef uint64_t			   irq_flags_t;
#else
typedef uint32_t               rv_csr_t;
typedef uint32_t			   irq_flags_t;
#endif /* __riscv_xlen */

#define __STR(s)                #s
#define STRINGIFY(s)            __STR(s)

/* Macros for Bit Operations */
#if __riscv_xlen == 32
#define BITMASK_MAX                         0xFFFFFFFFUL
#define BITOFS_MAX                          31
#else
#define BITMASK_MAX                         0xFFFFFFFFFFFFFFFFULL
#define BITOFS_MAX                          63
#endif

// BIT/BITS only support bit mask for __riscv_xlen
// For RISC-V 32 bit, it support mask 32 bit wide
// For RISC-V 64 bit, it support mask 64 bit wide
#define BIT(ofs)                            (0x1UL << (ofs))
#define BITS(start, end)                    ((BITMASK_MAX) << (start) & (BITMASK_MAX) >> (BITOFS_MAX - (end)))
#define GET_BIT(regval, bitofs)             (((regval) >> (bitofs)) & 0x1)
#define SET_BIT(regval, bitofs)             ((regval) |= BIT(bitofs))
#define CLR_BIT(regval, bitofs)             ((regval) &= (~BIT(bitofs)))
#define FLIP_BIT(regval, bitofs)            ((regval) ^= BIT(bitofs))
#define WRITE_BIT(regval, bitofs, val)      CLR_BIT(regval, bitofs); ((regval) |= ((val) << bitofs) & BIT(bitofs))
#define CHECK_BIT(regval, bitofs)           (!!((regval) & (0x1UL<<(bitofs))))
#define GET_BITS(regval, start, end)        (((regval) & BITS((start), (end))) >> (start))
#define SET_BITS(regval, start, end)        ((regval) |= BITS((start), (end)))
#define CLR_BITS(regval, start, end)        ((regval) &= (~BITS((start), (end))))
#define FLIP_BITS(regval, start, end)       ((regval) ^= BITS((start), (end)))
#define WRITE_BITS(regval, start, end, val) CLR_BITS(regval, start, end); ((regval) |= ((val) << start) & BITS((start), (end)))
#define CHECK_BITS_ALL(regval, start, end)  (!((~(regval)) & BITS((start), (end))))
#define CHECK_BITS_ANY(regval, start, end)  ((regval) & BITS((start), (end)))

#define BITMASK_SET(regval, mask)           ((regval) |= (mask))
#define BITMASK_CLR(regval, mask)           ((regval) &= (~(mask)))
#define BITMASK_FLIP(regval, mask)          ((regval) ^= (mask))
#define BITMASK_CHECK_ALL(regval, mask)     (!((~(regval)) & (mask)))
#define BITMASK_CHECK_ANY(regval, mask)     ((regval) & (mask))

#define OFFSET_OF(type, member)             ((size_t) &((type *)0)->member)
#define CONTAINER_OF(ptr, type, member)     ({                                                          \
                                                const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
                                                (type *)( (char *)__mptr - offsetof(type,member) );     \
                                            })

/**
 * \brief  Union type to access MISA CSR register.
 */
typedef union {
    struct {
        __IOM rv_csr_t a:1;                           /*!< bit:     0  Atomic extension */
        __IOM rv_csr_t b:1;                           /*!< bit:     1  Tentatively reserved for Bit-Manipulation extension */
        __IOM rv_csr_t c:1;                           /*!< bit:     2  Compressed extension */
        __IOM rv_csr_t d:1;                           /*!< bit:     3  Double-precision floating-point extension */
        __IOM rv_csr_t e:1;                           /*!< bit:     4  RV32E base ISA */
        __IOM rv_csr_t f:1;                           /*!< bit:     5  Single-precision floating-point extension */
        __IOM rv_csr_t g:1;                           /*!< bit:     6  Additional standard extensions present */
        __IOM rv_csr_t h:1;                           /*!< bit:     7  Hypervisor extension */
        __IOM rv_csr_t i:1;                           /*!< bit:     8  RV32I/64I/128I base ISA */
        __IOM rv_csr_t j:1;                           /*!< bit:     9  Tentatively reserved for Dynamically Translated Languages extension */
        __IOM rv_csr_t _reserved1:1;                  /*!< bit:     10 Reserved  */
        __IOM rv_csr_t l:1;                           /*!< bit:     11 Tentatively reserved for Decimal Floating-Point extension  */
        __IOM rv_csr_t m:1;                           /*!< bit:     12 Integer Multiply/Divide extension */
        __IOM rv_csr_t n:1;                           /*!< bit:     13 User-level interrupts supported  */
        __IOM rv_csr_t _reserved2:1;                  /*!< bit:     14 Reserved  */
        __IOM rv_csr_t p:1;                           /*!< bit:     15 Tentatively reserved for Packed-SIMD extension  */
        __IOM rv_csr_t q:1;                           /*!< bit:     16 Quad-precision floating-point extension  */
        __IOM rv_csr_t _resreved3:1;                  /*!< bit:     17 Reserved  */
        __IOM rv_csr_t s:1;                           /*!< bit:     18 Supervisor mode implemented  */
        __IOM rv_csr_t t:1;                           /*!< bit:     19 Tentatively reserved for Transactional Memory extension  */
        __IOM rv_csr_t u:1;                           /*!< bit:     20 User mode implemented  */
        __IOM rv_csr_t v:1;                           /*!< bit:     21 Tentatively reserved for Vector extension  */
        __IOM rv_csr_t _reserved4:1;                  /*!< bit:     22 Reserved  */
        __IOM rv_csr_t x:1;                           /*!< bit:     23 Non-standard extensions present  */
        __IOM rv_csr_t _reserved5:1;                  /*!< bit:     24 Reserved  */
        __IOM rv_csr_t _reserved6:1;                  /*!< bit:     25 Reserved  */
        __IOM rv_csr_t _reserved7:__riscv_xlen - 28;  /*!< bit:     26..MXLEN-28 Reserved  */
        __IOM rv_csr_t mxl:2;                         /*!< bit:     MXLEN-2..MXLEN-1 Machine XLEN  */
    } b;                                        /*!< Structure used for bit  access */
    rv_csr_t d;                                 /*!< Type      used for csr data access */
} csr_misa_reg_t;

/** CSR MISA register */
/* MXL field */
#define CSR_MISA_MACHINE_XLEN_FIELD              BITS(__riscv_xlen - 2, __riscv_xlen - 1)
/* MXL field option */
#define CSR_MISA_MXLEN_INVALID                  ((rv_csr_t)(0x0 << (__riscv_xlen - 2)))
#define CSR_MISA_MXLEN_32BIT                    ((rv_csr_t)(0x1 << (__riscv_xlen - 2)))
#define CSR_MISA_MXLEN_64BIT                    ((rv_csr_t)(0x2 << (__riscv_xlen - 2)))
#define CSR_MISA_MXLEN_128BIT                   ((rv_csr_t)(0x3 << (__riscv_xlen - 2)))

#define CSR_MISA_REG                            ((csr_misa_reg_t *)CSR_MISA_BASE)

/**
 * \brief  Union type to access MVENDORID CSR register.
 */
typedef union {
    struct {
        __IM uint32_t offset:7;                      /*!< bit:     0..6  offset */
        __IM uint32_t bank:25;                       /*!< bit:     7..31 bank */
    } b;                                        /*!< Structure used for bit  access */
    uint32_t d;                                 /*!< Type      used for csr data access */
} csr_mvendorid_reg_t;

#define CSR_MVENDORID_REG                        ((__IM csr_mvendorid_reg_t *)CSR_MVENDORID_BASE)

#define CSR_MARCHID_REG                          ((__IM rv_csr_t *)CSR_MARCHID_BASE)

#define CSR_MIMPID_REG                           ((__IM rv_csr_t *)CSR_MIMPID_BASE)

#define CSR_MHARTID_REG                          ((__IM rv_csr_t *)CSR_MHARTID_BASE)

/**
 * \brief  Union type to access MSTATUS CSR register.
 */
typedef union {
    struct {
        __IOM rv_csr_t uie:1;                         /*!< bit:     0  User mode interrupt enable flag */
        __IOM rv_csr_t sie:1;                         /*!< bit:     1  Supervisor mode interrupt enable flag */
        __IM  rv_csr_t _reserved1:1;                  /*!< bit:     2  Reserved */
        __IOM rv_csr_t mie:1;                         /*!< bit:     3  Machine mode interrupt enable flag */
        __IOM rv_csr_t upie:1;                        /*!< bit:     4  User Previous mode interrupt enable flag */
        __IOM rv_csr_t spie:1;                        /*!< bit:     5  Supervisor Previous mode interrupt enable flag */
        __IM  rv_csr_t _reserved2:1;                  /*!< bit:     6  Reserved */
        __IOM rv_csr_t mpie:1;                        /*!< bit:     7  mirror of MIE flag */
        __IOM rv_csr_t spp:1;                         /*!< bit:     8  Supervisor Previous Privilege Mode */
        __IM  rv_csr_t _reserved3:2;                  /*!< bit:     9..10  Reserved */
        __IOM rv_csr_t mpp:2;                         /*!< bit:     11..12 mirror of Privilege Mode */
        __IOM rv_csr_t fs:2;                          /*!< bit:     13..14 FS status flag */
        __IOM rv_csr_t xs:2;                          /*!< bit:     15..16 XS status flag */
        __IOM rv_csr_t mprv:1;                        /*!< bit:     17 Modify PRiVilege */
        __IOM rv_csr_t sum:1;                         /*!< bit:     18 permit Supervisor User Memory access */
        __IOM rv_csr_t mxr:1;                         /*!< bit:     19 Make eXecutable Readable */
        __IOM rv_csr_t tvm:1;                         /*!< bit:     20 Trap Virtual Memory */
        __IOM rv_csr_t tw:1;                          /*!< bit:     21 Timeout Wait */
        __IOM rv_csr_t tsr:1;                         /*!< bit:     22 Trap SRET */
#if __riscv_xlen == 32
        __IM  rv_csr_t _reserved4:__riscv_xlen - 24;  /*!< bit:     23..XLEN-2 */
#else
        __IM  rv_csr_t _reserved4:9;                  /*!< bit:     23..31 */
        __IOM rv_csr_t uxl:2;                         /*!< bit:     32..33 Base ISA Control for u-mode */
        __IOM rv_csr_t sxl:2;                         /*!< bit:     34..36 Base ISA Control for s-mode */
        __IM  rv_csr_t _reserved5:__riscv_xlen - 37;  /*!< bit:     36..XLEN-2 */
#endif
        __IOM rv_csr_t sd:1;                          /*!< bit:     XLEN-1 Dirty status for XS or FS */
    } b;                                        /*!< Structure used for bit  access */
    rv_csr_t d;                                 /*!< Type      used for csr data access */
} csr_mstatus_reg_t;

#define CSR_MSTATUS_REG                         ((csr_mstatus_reg_t *)CSR_MSTATUS_BASE)

/** CSR MSTATUS register */
/* MIE field */
#define CSR_MSTATUS_MIE_FIELD                   BIT(3)

/**
 * \brief  Union type to access MTVEC CSR register.
 */
typedef union {
    struct {
        __IOM rv_csr_t mode:2;                        /*!< bit:     0..2   interrupt mode control */
        __IOM rv_csr_t base:__riscv_xlen - 2;         /*!< bit:     2..XLEN-1   a vector base address */
    } b;                                        /*!< Structure used for bit  access */
    rv_csr_t d;                                 /*!< Type      used for csr data access */
} csr_mtvec_reg_t;

#define CSR_MTVEC_REG                           ((csr_mtvec_reg_t *)CSR_MTVEC_BASE)

/** CSR MTVEC register */
/* MODE field */
#define CSR_MTVEC_MODE_FIELD                    BITS(0, 1)
/* MODE field option */
#define CSR_MTVEC_EXCEPTION_DIRECT              ((rv_csr_t)(0x0 << 0))
#define CSR_MTVEC_EXCEPTION_VECTORED            ((rv_csr_t)(0x1 << 0))

/**
 * \brief  Union type to access MIP CSR register.
 */
typedef union {
    struct {
        __IOM rv_csr_t usip:1;                        /*!< bit:     0   lower-privilege software interrupts */
        __IOM rv_csr_t ssip:1;                        /*!< bit:     1   lower-privilege software interrupts */
        __IM  rv_csr_t _reserved0:1;                  /*!< bit:     2   Reserved */
        __IOM rv_csr_t msip:1;                        /*!< bit:     3   lower-privilege software interrupts */
        __IOM rv_csr_t utip:1;                        /*!< bit:     4   timer interrupts */
        __IOM rv_csr_t stip:1;                        /*!< bit:     5   timer interrupts */
        rv_csr_t _reserved1:1;                  /*!< bit:     6   Reserved */
        __IM  rv_csr_t mtip:1;                        /*!< bit:     7   timer interrupts */
        __IOM rv_csr_t ueip:1;                        /*!< bit:     8   external interrupts */
        __IOM rv_csr_t seip:1;                        /*!< bit:     9   external interrupts */
        __IM  rv_csr_t _reserved2:1;                  /*!< bit:     10  Reserved */
        __IM  rv_csr_t meip:1;                        /*!< bit:     11  external interrupts */
        __IM  rv_csr_t _reserved3:__riscv_xlen - 12;  /*!< bit:     12..XLEN-1   Reserved */
    } b;                                        /*!< Structure used for bit  access */
    rv_csr_t d;                                 /*!< Type      used for csr data access */
} csr_mip_reg_t;

#define CSR_MIP_REG                             ((csr_mip_reg_t *)CSR_MIP_BASE)

/**
 * \brief  Union type to access MIE CSR register.
 */
typedef union {
    struct {
        __IOM rv_csr_t usie:1;                        /*!< bit:     0   lower-privilege software interrupts */
        __IOM rv_csr_t ssie:1;                        /*!< bit:     1   lower-privilege software interrupts */
        __IM  rv_csr_t _reserved0:1;                  /*!< bit:     2   Reserved */
        __IOM rv_csr_t msie:1;                        /*!< bit:     3   lower-privilege software interrupts */
        __IOM rv_csr_t utie:1;                        /*!< bit:     4   timer interrupts */
        __IOM rv_csr_t stie:1;                        /*!< bit:     5   timer interrupts */
        rv_csr_t _reserved1:1;                  /*!< bit:     6   Reserved */
        __IM  rv_csr_t mtie:1;                        /*!< bit:     7   timer interrupts */
        __IOM rv_csr_t ueie:1;                        /*!< bit:     8   external interrupts */
        __IOM rv_csr_t seie:1;                        /*!< bit:     9   external interrupts */
        __IM  rv_csr_t _reserved2:1;                  /*!< bit:     10  Reserved */
        __IM  rv_csr_t meie:1;                        /*!< bit:     11  external interrupts */
        __IM  rv_csr_t _reserved3:__riscv_xlen - 12;  /*!< bit:     12..XLEN-1   Reserved */
    } b;                                        /*!< Structure used for bit  access */
    rv_csr_t d;                                 /*!< Type      used for csr data access */
} csr_mie_reg_t;

#define CSR_MIE_REG                             ((csr_mie_reg_t *)CSR_MIE_BASE)

/**
 * \brief  Union type to access MCAUSE CSR register.
 */
typedef union {
    struct {
        __IOM rv_csr_t exccode:__riscv_xlen-1;        /*!< bit:     0..XLEN-2  exception or interrupt code */
        __IOM rv_csr_t interrupt:1;                   /*!< bit:     XLEN-1  trap type. 0 means exception and 1 means interrupt */
    } b;                                        /*!< Structure used for bit  access */
    rv_csr_t d;                                 /*!< Type      used for csr data access */
} csr_mcause_reg_t;

#define CSR_MCAUSE_REG                          ((csr_mcause_reg_t *)CSR_MCAUSE_BASE)

/** CSR MCAUSE register */
/* EXECODE field */
#define CSR_MCAUSE_EXECODE_FIELD                BITS(0, __riscv_xlen - 1)
/* EXECODE field option */
#define CSR_MCAUSE_USER_SW_INT                  ((rv_csr_t)(0x0 << 0))
#define CSR_MCAUSE_SUPER_SW_INT                 ((rv_csr_t)(0x1 << 0))
#define CSR_MCAUSE_MACHINE_SW_INT               ((rv_csr_t)(0x3 << 0))
#define CSR_MCAUSE_USER_TIMER_INT               ((rv_csr_t)(0x4 << 0))
#define CSR_MCAUSE_SUPER_TIMER_INT              ((rv_csr_t)(0x5 << 0))
#define CSR_MCAUSE_MACHINE_TIMER_INT            ((rv_csr_t)(0x7 << 0))
#define CSR_MCAUSE_USER_EXTERNAL_INT            ((rv_csr_t)(0x8 << 0))
#define CSR_MCAUSE_SUPER_EXTERNAL_INT           ((rv_csr_t)(0x9 << 0))
#define CSR_MCAUSE_MACHINE_EXTERNAL_INT         ((rv_csr_t)(0x11 << 0))
#define CSR_MCAUSE_INS_ADD_MISALIGNED           ((rv_csr_t)(0x0 << 0))
#define CSR_MCAUSE_INS_ACCESS_FAULT             ((rv_csr_t)(0x1 << 0))
#define CSR_MCAUSE_ILLEGAL_INS                  ((rv_csr_t)(0x2 << 0))
#define CSR_MCAUSE_BREAKPOINT                   ((rv_csr_t)(0x3 << 0))
#define CSR_MCAUSE_LOAD_ADD_MISALIGNED          ((rv_csr_t)(0x4 << 0))
#define CSR_MCAUSE_LOAD_ACCESS_FAULT            ((rv_csr_t)(0x5 << 0))
#define CSR_MCAUSE_STORE_AMO_ADD_MISALIGNED     ((rv_csr_t)(0x6 << 0))
#define CSR_MCAUSE_STORE_AMO_ACCESS_FAULT       ((rv_csr_t)(0x7 << 0))
#define CSR_MCAUSE_ENV_CALL_FROM_USER           ((rv_csr_t)(0x8 << 0))
#define CSR_MCAUSE_ENV_CALL_FROM_SUPER          ((rv_csr_t)(0x9 << 0))
#define CSR_MCAUSE_ENV_CALL_FROM_MACHINE        ((rv_csr_t)(0x11 << 0))
#define CSR_MCAUSE_INS_PAGE_FAULT               ((rv_csr_t)(0x12 << 0))
#define CSR_MCAUSE_LOAD_PAGE_FAULT              ((rv_csr_t)(0x13 << 0))
#define CSR_MCAUSE_STORE_AMO_PAGE_FAULT         ((rv_csr_t)(0x15 << 0))

/**
 * \brief CSR operation Macro for csrrw instruction.
 * \details
 * Read the content of csr register to __v,
 * then write content of val into csr register, then return __v
 * \param csr   CSR macro definition defined in
 *              \ref NMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   value to store into the CSR register
 * \return the CSR register value before written
 */
#define csr_swap(csr, val)                                          \
({                                                                  \
    unsigned long __v = (unsigned long)(val);                       \
    __ASM __volatile__ ("csrrw %0, " STRINGIFY(csr) ", %1"          \
                    : "=r" (__v) : "rK" (__v)                       \
                    : "memory");                                    \
    __v;                                                            \
})

/**
 * \brief CSR operation Macro for csrr instruction.
 * \details
 * Read the content of csr register to __v and return it
 * \param csr   CSR macro definition defined in
 *              \ref NMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \return the CSR register value
 */
#define csr_read(csr)                                               \
({                                                                  \
    register unsigned long __v;                                     \
    __ASM __volatile__ ("csrr %0, " STRINGIFY(csr)                  \
                    : "=r" (__v) :                                  \
                    : "memory");                                    \
    __v;                                                            \
})

/**
 * \brief CSR operation Macro for csrw instruction.
 * \details
 * Write the content of val to csr register
 * \param csr   CSR macro definition defined in
 *              \ref NMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   value to store into the CSR register
 */
#define csr_write(csr, val)                                         \
({                                                                  \
    unsigned long __v = (unsigned long)(val);                       \
    __ASM __volatile__ ("csrw " STRINGIFY(csr) ", %0"               \
                    : : "rK" (__v)                                  \
                    : "memory");                                    \
})

/**
 * \brief CSR operation Macro for csrrs instruction.
 * \details
 * Read the content of csr register to __v,
 * then set csr register to be __v | val, then return __v
 * \param csr   CSR macro definition defined in
 *              \ref NMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   Mask value to be used wih csrrs instruction
 * \return the CSR register value before written
 */
#define csr_read_set(csr, val)                                      \
({                                                                  \
    unsigned long __v = (unsigned long)(val);                       \
    __ASM __volatile__ ("csrrs %0, " STRINGIFY(csr) ", %1"          \
                    : "=r" (__v) : "rK" (__v)                       \
                    : "memory");                                    \
    __v;                                                            \
})

/**
 * \brief CSR operation Macro for csrs instruction.
 * \details
 * Set csr register to be csr_content | val
 * \param csr   CSR macro definition defined in
 *              \ref NMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   Mask value to be used wih csrs instruction
 */
#define csr_set(csr, val)                                           \
({                                                                  \
    unsigned long __v = (unsigned long)(val);                       \
    __ASM __volatile__ ("csrs " STRINGIFY(csr) ", %0"               \
                    : : "rK" (__v)                                  \
                    : "memory");                                    \
})

/**
 * \brief CSR operation Macro for csrrc instruction.
 * \details
 * Read the content of csr register to __v,
 * then set csr register to be __v & ~val, then return __v
 * \param csr   CSR macro definition defined in
 *              \ref NMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   Mask value to be used wih csrrc instruction
 * \return the CSR register value before written
 */
#define csr_read_clear(csr, val)                                    \
({                                                                  \
    unsigned long __v = (unsigned long)(val);                       \
    __ASM __volatile__ ("csrrc %0, " STRINGIFY(csr) ", %1"          \
                    : "=r" (__v) : "rK" (__v)                       \
                    : "memory");                                    \
    __v;                                                            \
})

/**
 * \brief CSR operation Macro for csrc instruction.
 * \details
 * Set csr register to be csr_content & ~val
 * \param csr   CSR macro definition defined in
 *              \ref NMSIS_Core_CSR_Registers, eg. \ref CSR_MSTATUS
 * \param val   Mask value to be used wih csrc instruction
 */
#define csr_clear(csr, val)                                         \
({                                                                  \
    unsigned long __v = (unsigned long)(val);                       \
    __ASM __volatile__ ("csrc " STRINGIFY(csr) ", %0"               \
                    : : "rK" (__v)                                  \
                    : "memory");                                    \
})

/**
 * \brief   Read whole 64 bits value of mcycle counter
 * \details This function will read the whole 64 bits of MCYCLE register
 * \return  The whole 64 bits value of MCYCLE
 * \remarks It will work for both RV32 and RV64 to get full 64bits value of MCYCLE
 */
__STATIC_FORCEINLINE uint64_t __get_rv_cycle(void)
{
#if __riscv_xlen == 32
    volatile uint32_t high0, low, high;
    uint64_t full;

    high0 = csr_read(CSR_MCYCLEH_BASE);
    low = csr_read(CSR_MCYCLE_BASE);
    high = csr_read(CSR_MCYCLEH_BASE);
    if (high0 != high) {
        low = csr_read(CSR_MCYCLE_BASE);
    }
    full = (((uint64_t)high) << 32) | low;
    return full;
#elif __riscv_xlen == 64
    return (uint64_t)csr_read(CSR_MCYCLE_BASE);
#else
    return (uint64_t)csr_read(CSR_MCYCLE_BASE);
#endif
}

/**
 * \brief   Read whole 64 bits value of machine instruction-retired counter
 * \details This function will read the whole 64 bits of MINSTRET register
 * \return  The whole 64 bits value of MINSTRET
 * \remarks It will work for both RV32 and RV64 to get full 64bits value of MINSTRET
 */
__STATIC_FORCEINLINE uint64_t __get_rv_instret(void)
{
#if __riscv_xlen == 32
    volatile uint32_t high0, low, high;
    uint64_t full;

    high0 = csr_read(CSR_MINSTRETH_BASE);
    low = csr_read(CSR_MINSTRET_BASE);
    high = csr_read(CSR_MINSTRETH_BASE);
    if (high0 != high) {
        low = csr_read(CSR_MINSTRET_BASE);
    }
    full = (((uint64_t)high) << 32) | low;
    return full;
#elif __riscv_xlen == 64
    return (uint64_t)csr_read(CSR_MINSTRET_BASE);
#else
    return (uint64_t)csr_read(CSR_MINSTRET_BASE);
#endif
}

/**
 * \brief   Read whole 64 bits value of real-time clock
 * \details This function will read the whole 64 bits of TIME register
 * \return  The whole 64 bits value of TIME CSR
 * \remarks It will work for both RV32 and RV64 to get full 64bits value of TIME
 * \attention only available when user mode available
 */
__STATIC_FORCEINLINE uint64_t __get_rv_time(void)
{
#if __riscv_xlen == 32
    volatile uint32_t high0, low, high;
    uint64_t full;

    high0 = csr_read(CSR_TIMEH_BASE);
    low = csr_read(CSR_TIME_BASE);
    high = csr_read(CSR_TIMEH_BASE);
    if (high0 != high) {
        low = csr_read(CSR_TIME_BASE);
    }
    full = (((uint64_t)high) << 32) | low;
    return full;
#elif __riscv_xlen == 64
    return (uint64_t)csr_read(CSR_TIME_BASE);
#else
    return (uint64_t)csr_read(CSR_TIME_BASE);
#endif
}

/**
 * \defgroup NMSIS_Core_CPU_Intrinsic   Intrinsic Functions for CPU Intructions
 * \ingroup  NMSIS_Core
 * \brief    Functions that generate RISC-V CPU instructions.
 * \details
 *
 * The following functions generate specified RISC-V instructions that cannot be directly accessed by compiler.
 *   @{
 */

/**
 * \brief   NOP Instruction
 * \details
 * No Operation does nothing.
 * This instruction can be used for code alignment purposes.
 */
__STATIC_FORCEINLINE void __NOP(void)
{
    __ASM volatile("nop");
}

/**
 * \brief   Wait For Interrupt
 * \details
 * Wait For Interrupt is is executed using CSR_WFE.WFE=0 and WFI instruction.
 * It will suspends execution until interrupt, NMI or Debug happened.
 * When Core is waked up by interrupt, if
 * 1. mstatus.MIE == 1(interrupt enabled), Core will enter ISR code
 * 2. mstatus.MIE == 0(interrupt disabled), Core will resume previous execution
 */
__STATIC_FORCEINLINE void __WFI(void)
{
    __ASM volatile("wfi");
}

/**
 * \brief   Wait For Event
 * \details
 * Wait For Event is executed using CSR_WFE.WFE=1 and WFI instruction.
 * It will suspends execution until event, NMI or Debug happened.
 * When Core is waked up, Core will resume previous execution
 */
__STATIC_FORCEINLINE void __WFE(void)
{
    __ASM volatile("wfi");
}

/**
 * \brief   Breakpoint Instruction
 * \details
 * Causes the processor to enter Debug state.
 * Debug tools can use this to investigate system state
 * when the instruction at a particular address is reached.
 */
__STATIC_FORCEINLINE void __EBREAK(void)
{
    __ASM volatile("ebreak");
}

/**
 * \brief   Environment Call Instruction
 * \details
 * The ECALL instruction is used to make a service request to
 * the execution environment.
 */
__STATIC_FORCEINLINE void __ECALL(void)
{
    __ASM volatile("ecall");
}

/* Read and write memory barrier */
#define mb()		__ASM __volatile__ ("fence iorw, iorw" : : : "memory");
/* Read memory barrier */
#define rmb()		__ASM __volatile__ ("fence ir, ir" : : : "memory");
/* Write memory barrier */
#define wmb()		__ASM __volatile__ ("fence ow, ow" : : : "memory");

/* SMP read and write memory barrier */
#define smp_mb()	__ASM __volatile__ ("fence rw, rw" : : : "memory");
/* SMP read memory barrier */
#define smp_rmb()	__ASM __volatile__ ("fence r, r" : : : "memory");
/* SMP write memory barrier */
#define smp_wmb()	__ASM __volatile__ ("fence w, w" : : : "memory");

typedef struct {
	volatile int counter;
} __atomic_t;

#ifdef __riscv_atomic
__STATIC_FORCEINLINE void __atomic_add(__atomic_t * a, int v)
{
	__asm__ __volatile__ (
		"amoadd.w zero, %1, %0"
		: "+A" (a->counter)
		: "r" (v)
		: "memory");
}

__STATIC_FORCEINLINE int __atomic_add_return(__atomic_t * a, int v)
{
	int ret;
	__asm__ __volatile__ (
		"amoadd.w.aqrl %1, %2, %0"
		: "+A" (a->counter), "=r" (ret)
		: "r" (v)
		: "memory");
	return ret + v;
}

__STATIC_FORCEINLINE void __atomic_sub(__atomic_t * a, int v)
{
	__asm__ __volatile__ (
		"amoadd.w zero, %1, %0"
		: "+A" (a->counter)
		: "r" (-v)
		: "memory");
}

__STATIC_FORCEINLINE int __atomic_sub_return(__atomic_t * a, int v)
{
	int ret;
	__asm__ __volatile__ (
		"amoadd.w.aqrl %1, %2, %0"
		: "+A" (a->counter), "=r" (ret)
		: "r" (-v)
		: "memory");
	return ret - v;
}

#define __cmpxchg(ptr, old, new, size)						\
({															\
	__typeof__(ptr) __ptr = (ptr);							\
	__typeof__(*(ptr)) __old = (old);						\
	__typeof__(*(ptr)) __new = (new);						\
	__typeof__(*(ptr)) __ret;								\
	register unsigned int __rc;								\
	switch (size) {											\
	case 4:													\
		__asm__ __volatile__ (								\
			"0:	lr.w %0, %2\n"								\
			"bne  %0, %z3, 1f\n"							\
			"sc.w.rl %1, %z4, %2\n"							\
			"bnez %1, 0b\n"									\
			"fence rw, rw\n"								\
			"1:\n"											\
			: "=&r" (__ret), "=&r" (__rc), "+A" (*__ptr)	\
			: "rJ" (__old), "rJ" (__new)					\
			: "memory");									\
		break;												\
	case 8:													\
		__asm__ __volatile__ (								\
			"0:	lr.d %0, %2\n"								\
			"bne %0, %z3, 1f\n"								\
			"sc.d.rl %1, %z4, %2\n"							\
			"bnez %1, 0b\n"									\
			"fence rw, rw\n"								\
			"1:\n"											\
			: "=&r" (__ret), "=&r" (__rc), "+A" (*__ptr)	\
			: "rJ" (__old), "rJ" (__new)					\
			: "memory");									\
		break;												\
	default:												\
		break;												\
	}														\
	__ret;													\
})

#define cmpxchg(ptr, o, n)									\
({															\
	__typeof__(*(ptr)) _o_ = (o);							\
	__typeof__(*(ptr)) _n_ = (n);							\
	(__typeof__(*(ptr))) __cmpxchg((ptr),					\
			_o_, _n_, sizeof(*(ptr)));						\
})

__STATIC_FORCEINLINE int __atomic_cmp_exchange(__atomic_t * a, int o, int n)
{
	return cmpxchg(&a->counter, o, n);
}
#else
__STATIC_FORCEINLINE void __atomic_add(__atomic_t * a, int v)
{
	mb();
	a->counter += v;
	mb();
}

__STATIC_FORCEINLINE int __atomic_add_return(__atomic_t * a, int v)
{
	mb();
	a->counter += v;
	mb();
	return a->counter;
}

__STATIC_FORCEINLINE void __atomic_sub(__atomic_t * a, int v)
{
	mb();
	a->counter -= v;
	mb();
}

__STATIC_FORCEINLINE int __atomic_sub_return(__atomic_t * a, int v)
{
	mb();
	a->counter -= v;
	mb();
	return a->counter;
}

__STATIC_FORCEINLINE int __atomic_cmp_exchange(__atomic_t * a, int o, int n)
{
	volatile int v;

	mb();
	v = a->counter;
	if(v == o)
		a->counter = n;
	mb();
	return v;
}
#endif

#define __atomic_set(a, v)			    do { ((a)->counter) = (v); smp_wmb(); } while(0)
#define __atomic_get(a)				    ({ int __v; __v = (a)->counter; smp_rmb(); __v; })
#define __atomic_inc(a)				    (__atomic__add(a, 1))
#define __atomic_dec(a)				    (__atomic__sub(a, 1))
#define __atomic_inc_return(a)		    (__atomic__add_return(a, 1))
#define __atomic_dec_return(a)		    (__atomic__sub_return(a, 1))
#define __atomic_inc_and_test(a)		(__atomic__add_return(a, 1) == 0)
#define __atomic_dec_and_test(a)		(__atomic__sub_return(a, 1) == 0)
#define __atomic_add_negative(a, v)	    (__atomic__add_return(a, v) < 0)
#define __atomic_sub_and_test(a, v)	    (__atomic__sub_return(a, v) == 0)
#define __atomic_cmpxchg(a, o, n)		(__atomic__cmp_exchange(a, o, n))

/* Macros for memory access operations */
#define _REG8P(p, i)                        ((volatile uint8_t *) ((uintptr_t)((p) + (i))))
#define _REG16P(p, i)                       ((volatile uint16_t *) ((uintptr_t)((p) + (i))))
#define _REG32P(p, i)                       ((volatile uint32_t *) ((uintptr_t)((p) + (i))))
#define _REG64P(p, i)                       ((volatile uint64_t *) ((uintptr_t)((p) + (i))))
#define _REG8(p, i)                         (*(_REG8P(p, i)))
#define _REG16(p, i)                        (*(_REG16P(p, i)))
#define _REG32(p, i)                        (*(_REG32P(p, i)))
#define _REG64(p, i)                        (*(_REG64P(p, i)))
#define REG8(addr)                          _REG8((addr), 0)
#define REG16(addr)                         _REG16((addr), 0)
#define REG32(addr)                         _REG32((addr), 0)
#define REG64(addr)                         _REG64((addr), 0)

/* Macros for address type convert and access operations */
#define ADDR16(addr)                        ((uint16_t)(uintptr_t)(addr))
#define ADDR32(addr)                        ((uint32_t)(uintptr_t)(addr))
#define ADDR64(addr)                        ((uint64_t)(uintptr_t)(addr))
#define ADDR8P(addr)                        ((uint8_t *)(uintptr_t)(addr))
#define ADDR16P(addr)                       ((uint16_t *)(uintptr_t)(addr))
#define ADDR32P(addr)                       ((uint32_t *)(uintptr_t)(addr))
#define ADDR64P(addr)                       ((uint64_t *)(uintptr_t)(addr))


#ifdef __cplusplus
}
#endif

#endif // __RSIS_CORE_H
