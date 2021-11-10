#ifndef __RSIS_TRAP_H
#define __RSIS_TRAP_H

#include "rsis_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TRAP_INTERRUPT_TYPE = 0,
    TRAP_EXCEPTION_TYPE = 1,
} trap_type_t;

/*!
 * @brief Possible mode of interrupts to operate
 */
typedef enum{
    IRQ_DIRECT_MODE = CSR_MTVEC_EXCEPTION_DIRECT,
    IRQ_VECTOR_MODE = CSR_MTVEC_EXCEPTION_VECTORED,
} mirq_mode_t;

__STATIC_FORCEINLINE mirq_mode_t __get_mirq_mode(void)
{
    uintptr_t val = csr_read(CSR_MTVEC_BASE);
    val &= CSR_MTVEC_MODE_FIELD;
    return (mirq_mode_t) val;
}

__STATIC_FORCEINLINE void __set_mirq_handle(mirq_mode_t mode,
                                            void *handle)
{
    uintptr_t trap_entry = (uintptr_t)handle;
    switch (mode) {
        case IRQ_DIRECT_MODE: {
            csr_write(CSR_MTVEC_BASE, (trap_entry) | CSR_MTVEC_EXCEPTION_DIRECT);
            break;
        }
        case IRQ_VECTOR_MODE: {
            csr_write(CSR_MTVEC_BASE, (trap_entry) | CSR_MTVEC_EXCEPTION_VECTORED);
            break;
        }
    }
}

/**
 * \brief   Enable IRQ Interrupts
 * \details Enables IRQ interrupts by setting the MIE-bit in the MSTATUS Register.
 * \remarks
 *          Can only be executed in Privileged modes.
 */
__STATIC_FORCEINLINE void __enable_mirq(void)
{
    csr_set(CSR_MSTATUS_BASE, CSR_MSTATUS_MIE_FIELD);
}

/**
 * \brief   Disable IRQ Interrupts
 * \details Disables IRQ interrupts by clearing the MIE-bit in the MSTATUS Register.
 * \remarks
 *          Can only be executed in Privileged modes.
 */
__STATIC_FORCEINLINE void __disable_mirq(void)
{
    csr_clear(CSR_MSTATUS_BASE, CSR_MSTATUS_MIE_FIELD);
}

/**
 * \brief   Save IRQ Interrupts
 * \details Save IRQ interrupts by reading the MIE-bit in the MSTATUS Register.
 * \remarks
 *          Can only be executed in Privileged modes.
 */
__STATIC_FORCEINLINE irq_flags_t __local_mirq_save(void)
{
	return csr_read_clear(CSR_MSTATUS_BASE, CSR_MSTATUS_MIE_FIELD);
}

/**
 * \brief   Restore IRQ Interrupts
 * \details Restore IRQ interrupts by setting the MIE-bit in the MSTATUS Register.
 * \remarks
 *          Can only be executed in Privileged modes.
 */
__STATIC_FORCEINLINE void __local_mirq_restore(irq_flags_t flags)
{
	csr_set(CSR_MSTATUS_BASE, flags & CSR_MSTATUS_MIE_FIELD);
}

/**
 * \brief      Exception Handler Function Typedef
 * \note
 * This typedef is only used internal in this system_gd32vf103.c file.
 * It is used to do type conversion for registered exception handler before calling it.
 */
typedef void (*trap_handler_func)(rv_csr_t mcause, rv_csr_t sp, void *arg);

void core_trap_regist(trap_type_t type, uint16_t trap_id, trap_handler_func handle, void *arg);

rv_csr_t core_trap_handler(rv_csr_t mcause, rv_csr_t mepc, rv_csr_t sp);

#ifdef __cplusplus
}
#endif

#endif /* __RSIS_TRAP_H */
