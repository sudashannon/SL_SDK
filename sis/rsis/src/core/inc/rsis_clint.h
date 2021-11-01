#ifndef __RSIS_CLINT_H
#define __RSIS_CLINT_H

#include "rsis_core.h"

/*!
 * @file     rsis_clint.h
 * @brief    CLINT feature API header file for risc-v Core
 */
/*
 * CLINT Feature Configuration Macro:
 * 1. CLINT_PRESENT:  Define whether Clint is present or not.
 *   * 0: Not present
 *   * 1: Present
 * 2. CLINT_BASEADDR:  Define the base address of the clint.
 */
#ifdef __cplusplus
extern "C" {
#endif

#if defined(CLINT_PRESENT) && (CLINT_PRESENT == 1)

#ifndef CLINT_BASE
/* Base address of CLINT(CLINT_BASE) should be defined in <Device.h> */
#error "CLINT_BASE is not defined, please check!"
#endif

#ifndef MAX_HARTS
/* Hart counts of device(MAX_HARTS) should be defined in <Device.h> */
#error "MAX_HARTS is not defined, please check!"
#endif

/**
 * \brief  Structure type to access the CLINT.
 * \details
 * Structure definition to access the CLINT.
 * \remarks
 */
typedef struct {
    __IOM uint32_t reserved_0[0x1000];       /*!< Offset: 0x000  -Reserved section with msip for each hart */
    __IOM uint64_t reserved_1[0xFFF];        /*!< Offset: 0x4000 -Reserved section with mtimecmp for each hart */
    __IOM uint64_t mtime;                    /*!< Offset: 0xBFF8 mtime timer register */
    __IOM uint64_t reserved_2;               /*!< Offset: 0xC000 -Reserved section */
} core_clint_reg_t;

/**
 * \brief  Structure type to access the msip for each hart.
 * \details
 * \remarks
 */
typedef struct {
    __IOM uint32_t msip_n[MAX_HARTS];        /*!< Offset: 0x000  -Msip for each hart */
} core_clint_msip_reg_t;

/**
 * \brief  Structure type to access the mtimecmp for each hart.
 * \details
 * \remarks
 */
typedef struct {
    __IOM uint64_t mtimecmp_n[MAX_HARTS];    /*!< Offset: 0x000  -Mtimecmp for each hart */
} core_clint_mtimecmp_reg_t;

#define CLINT               ((core_clint_reg_t *) CLINT_BASE)
#define MSIP(n)             ((core_clint_msip_reg_t *)((core_clint_reg_t *) CLINT_BASE)->reserved_0)->msip_n[n]
#define MTIMECMP(n)         ((core_clint_mtimecmp_reg_t *)((core_clint_reg_t *) CLINT_BASE)->reserved_1)->mtimecmp_n[n]

/**
 * \brief  Get system timer tick value
 * \details
 * This function get the system timer current value in MTIME register.
 * \return  current value(64bit) of system timer MTIME register.
 * \remarks
 * - Load value is 64bits wide.
 * - \ref SysTimer_SetLoadValue
 */
__STATIC_FORCEINLINE uint64_t systime_get_tick(void)
{
    return REG64(&CLINT->mtime);
}

/**
 * \brief  Set system timer compare value
 * \details
 * This function set the system timer compare value in MTIMECMP register.
 * \param [in]  value   value to set system timer MTIMECMP register.
 * \remarks
 * - Load value is 64bits wide.
 * - \ref systimer_get_cmpvalue
 */
__STATIC_FORCEINLINE void systimer_set_cmpvalue(uint8_t hart_id, uint64_t value)
{
    REG64(&MTIMECMP(hart_id)) = value;
}

/**
 * \brief  Get system timer compare value
 * \details
 * This function get the system timer current compare value in MTIMECMP register.
 * \return  current value(64bit) of system timer MTIMECMP register.
 * \remarks
 * - Load value is 64bits wide.
 * - \ref systimer_set_cmpvalue
 */
__STATIC_FORCEINLINE uint64_t systimer_get_cmpvalue(uint8_t hart_id)
{
    return REG64(&MTIMECMP(hart_id));
}

/**
 * \brief  Enable a systimer interrupt
 * \details
 * This function enables the systimer interrupt.
 * \sa
 * - \ref systimer_stop
 */
__STATIC_FORCEINLINE void systimer_start(void)
{
    csr_set(CSR_MIE_BASE, BIT(7));
}

/**
 * \brief  Disable a systimer interrupt
 * \details
 * This function disables the systimer interrupt.
 * \sa
 * - \ref systimer_stop
 */
__STATIC_FORCEINLINE void systimer_stop(void)
{
    csr_clear(CSR_MIE_BASE, BIT(7));
}

/**
 * \brief  Clean a systimer interrupt
 * \details
 * This function clean the systimer interrupt.
 * \sa
 * - \ref systimer_stop
 */
__STATIC_FORCEINLINE void systimer_clean(void)
{
    csr_clear(CSR_MIP_BASE, BIT(7));
}

#endif /* defined(CLINT_PRESENT) && (CLINT_PRESENT == 1) */

#ifdef __cplusplus
}
#endif

#endif
