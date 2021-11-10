#include "../inc/rsis_csr_address.h"

/**
 * \brief  Exception/NMI Entry
 * \details
 * This function provide common entry functions for exception/nmi.
 * \remarks
 * This function provide a default exception/nmi entry.
 * ABI defined caller save register and some CSR registers
 * to be saved before enter interrupt handler and be restored before return.
 */
.section .text.trap
.align 8
.global exc_entry
exc_entry:
    addi sp, sp, -32*REGBYTES

    STORE x1, 1*REGBYTES(sp)
    STORE x2, 2*REGBYTES(sp)
    STORE x3, 3*REGBYTES(sp)
    STORE x4, 4*REGBYTES(sp)
    STORE x5, 5*REGBYTES(sp)
    STORE x6, 6*REGBYTES(sp)
    STORE x7, 7*REGBYTES(sp)
    STORE x8, 8*REGBYTES(sp)
    STORE x9, 9*REGBYTES(sp)
    STORE x10, 10*REGBYTES(sp)
    STORE x11, 11*REGBYTES(sp)
    STORE x12, 12*REGBYTES(sp)
    STORE x13, 13*REGBYTES(sp)
    STORE x14, 14*REGBYTES(sp)
    STORE x15, 15*REGBYTES(sp)
#ifndef RV32E
    STORE x16, 16*REGBYTES(sp)
    STORE x17, 17*REGBYTES(sp)
    STORE x18, 18*REGBYTES(sp)
    STORE x19, 19*REGBYTES(sp)
    STORE x20, 20*REGBYTES(sp)
    STORE x21, 21*REGBYTES(sp)
    STORE x22, 22*REGBYTES(sp)
    STORE x23, 23*REGBYTES(sp)
    STORE x24, 24*REGBYTES(sp)
    STORE x25, 25*REGBYTES(sp)
    STORE x26, 26*REGBYTES(sp)
    STORE x27, 27*REGBYTES(sp)
    STORE x28, 28*REGBYTES(sp)
    STORE x29, 29*REGBYTES(sp)
    STORE x30, 30*REGBYTES(sp)
    STORE x31, 31*REGBYTES(sp)
#endif
    /*
     * Set the exception handler function arguments
     * argument 1: mcause value
     * argument 2: mepc value
     * argument 3: current stack point(SP) value
     */
    csrr a0, mcause
    csrr a1, mepc
    mv a2, sp
    call core_trap_handler
    csrw mepc, a0
    # Remain in M-mode after mret
    li t0, 0x00001800
    csrs mstatus, t0

    LOAD x1, 1*REGBYTES(sp)
    LOAD x2, 2*REGBYTES(sp)
    LOAD x3, 3*REGBYTES(sp)
    LOAD x4, 4*REGBYTES(sp)
    LOAD x5, 5*REGBYTES(sp)
    LOAD x6, 6*REGBYTES(sp)
    LOAD x7, 7*REGBYTES(sp)
    LOAD x8, 8*REGBYTES(sp)
    LOAD x9, 9*REGBYTES(sp)
    LOAD x10, 10*REGBYTES(sp)
    LOAD x11, 11*REGBYTES(sp)
    LOAD x12, 12*REGBYTES(sp)
    LOAD x13, 13*REGBYTES(sp)
    LOAD x14, 14*REGBYTES(sp)
    LOAD x15, 15*REGBYTES(sp)
#ifndef RV32E
    LOAD x16, 16*REGBYTES(sp)
    LOAD x17, 17*REGBYTES(sp)
    LOAD x18, 18*REGBYTES(sp)
    LOAD x19, 19*REGBYTES(sp)
    LOAD x20, 20*REGBYTES(sp)
    LOAD x21, 21*REGBYTES(sp)
    LOAD x22, 22*REGBYTES(sp)
    LOAD x23, 23*REGBYTES(sp)
    LOAD x24, 24*REGBYTES(sp)
    LOAD x25, 25*REGBYTES(sp)
    LOAD x26, 26*REGBYTES(sp)
    LOAD x27, 27*REGBYTES(sp)
    LOAD x28, 28*REGBYTES(sp)
    LOAD x29, 29*REGBYTES(sp)
    LOAD x30, 30*REGBYTES(sp)
    LOAD x31, 31*REGBYTES(sp)
#endif
    addi sp, sp, 32*REGBYTES
    /* Return to regular code */
    mret

/*
 * For sanity's sake we set up an early trap vector that just does nothing.
 * If you end up here then there's a bug in the early boot code somewhere.
 */
.section .text.init.trap
.global early_trap_entry
.align 8
early_trap_entry:
    .cfi_startproc
    csrr t0, mcause
    csrr t1, mepc
    csrr t2, mtval
    j early_trap_entry
    .cfi_endproc
