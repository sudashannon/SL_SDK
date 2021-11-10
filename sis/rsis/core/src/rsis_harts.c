/* Copyright 2019 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include "../inc/rsis_include.h"
/*
 * core_hart_synchronize() is called by crt0.S to cause normal hart to wait for
 * boot hart to finish copying the datat section, zeroing the BSS, and running
 * the libc contstructors.
 */
__attribute__((section(".init"))) void core_hart_synchronize() {
#if MAX_HARTS > 1
    int hart = csr_read(CSR_MHARTID_BASE);
    extern char __boot_hart;
    uintptr_t boot_hart_id = (uintptr_t)&__boot_hart;
    /* Get the base address of the MSIP registers for selected hart */
    uintptr_t clint_base = (uintptr_t)CLINT_BASE;

    /* Disable machine interrupts as a precaution */
    __disable_mirq();


    if (hart == boot_hart_id) {
        /* Hart 0 waits for all harts to set their MSIP bit */
        for (int i = 1; i < MAX_HARTS; i++) {
            if (i != boot_hart_id)
                while (_REG32(clint_base, i * sizeof(uint32_t)) == 0);
        }

        /* Hart 0 clears everyone's MSIP bit */
        for (int i = 1; i < MAX_HARTS; i++) {
            if (i != boot_hart_id)
                _REG32(clint_base, i * sizeof(uint32_t)) = 0;
        }
    } else {
        /* Other harts set their MSIP bit to indicate they're ready */
        _REG32(clint_base, hart * sizeof(uint32_t)) = 1;
        __asm__ volatile("fence w,rw");
        /* Wait for hart 0 to clear the MSIP bit */
        while (_REG32(clint_base, hart * sizeof(uint32_t)) == 1);
    }

#endif /* MAX_HARTS > 1 */
}
