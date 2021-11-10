#include "../inc/rsis_trap.h"
#include "rte_include.h"

typedef struct {
    trap_handler_func handle;
    void *arg; //
} trap_handle_t;

/**
 * \brief      Store the trap handlers for each trap ID
 * \note
 * - This SystemExceptionHandlers are used to store all the handlers for all
 *   the trap codes sifive u84 core provided.
 */
static trap_handle_t system_trap_handlers[2][1024] = {0};

/**
 * \brief      Common trap handler entry
 * \details
 * This function provided a command entry for NMI and exception. Silicon Vendor could modify
 * this template implementation according to requirement.
 * \remarks
 * - RISCV provided common entry for all types of exception. This is proposed code template
 *   for exception entry function, Silicon Vendor could modify the implementation.
 * - For the core_trap_handler template, we provided exception register function \ref Exception_Register_EXC
 *   which can help developer to register your exception handler for specific exception number.
 */
rv_csr_t core_trap_handler(rv_csr_t mcause, rv_csr_t mepc, rv_csr_t sp)
{
    uint8_t trap_type = (mcause >> 63 == 1) ? TRAP_INTERRUPT_TYPE : TRAP_EXCEPTION_TYPE;
    uint16_t trap_id = (mcause & 0x1FF);
    if (system_trap_handlers[trap_type][trap_id].handle) {
        trap_handler_func pfunc = system_trap_handlers[trap_type][trap_id].handle;
        pfunc(mcause, sp, system_trap_handlers[trap_type][trap_id].arg);
    } else {
        RTE_LOGF("Undefined trap handle for trap type:%d id:%d", trap_type, trap_id);
        if (trap_type == TRAP_EXCEPTION_TYPE) {
            uintptr_t mtval = csr_read(CSR_MTVEC_BASE);
            RTE_LOGI("mtval value 0x%016lx", mtval);
            RTE_LOGI("mepc value 0x%016lx", mepc);
            for(;;);
        }
    }
    return mepc;
}

void core_trap_regist(trap_type_t type, uint16_t trap_id, trap_handler_func handle, void *arg)
{
    system_trap_handlers[type][trap_id].handle = handle;
    system_trap_handlers[type][trap_id].arg = arg;
}
