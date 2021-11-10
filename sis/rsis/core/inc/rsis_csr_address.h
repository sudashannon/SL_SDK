/**
 * @file rsis_csr_address.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 1.0.0   First version, follow the documentation The RISC-V Instruction Set Manual Volume II:
 *                  Privileged Architecture, section 2.2
 * @date 2021-09-25
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef __RSIS_CSR_ADDRESS_H
#define __RSIS_CSR_ADDRESS_H

#ifdef __cplusplus
extern "C" {
#endif

#if __riscv_xlen == 64
#define SLL32                  sllw
#define STORE                  sd
#define LOAD                   ld
#define LWU                    lwu
#define LOG_REGBYTES           3
#else
#define SLL32                  sll
#define STORE                  sw
#define LOAD                   lw
#define LWU                    lw
#define LOG_REGBYTES           2
#endif /* __riscv_xlen */

#define REGBYTES (1 << LOG_REGBYTES)

#if __riscv_flen == 64
#define FPSTORE                fsd
#define FPLOAD                 fld
#define LOG_FPREGBYTES         3
#else
#define FPSTORE                fsw
#define FPLOAD                 flw
#define LOG_FPREGBYTES         2
#endif /* __riscv_flen == 64 */

#define FPREGBYTES              (1 << LOG_FPREGBYTES)

/**
 * \defgroup RSIS_Core_CSR_Registers    Core CSR Registers
 * \ingroup  RSIS_Core
 * \brief    RSIS Core CSR Register Definitions
 * \details
 *
 * The following macros are used for CSR Register Defintions.
 *   @{
 */
/* === Currently allocated RISC-V user-level CSR addresses === */
#define CSR_USTATUS_BASE                     0x0
#define CSR_FFLAGS_BASE                      0x1
#define CSR_FRM_BASE                         0x2
#define CSR_FCSR_BASE                        0x3
#define CSR_UIE_BASE                         0x4
#define CSR_UTVEC_BASE                       0x5
#define CSR_USCRATCH_BASE                    0x040
#define CSR_UEPC_BASE                        0x041
#define CSR_UCAUSE_BASE                      0x042
#define CSR_UTVAL_BASE                       0x043
#define CSR_UIP_BASE                         0x044
#define CSR_CYCLE_BASE                       0xc00
#define CSR_TIME_BASE                        0xc01
#define CSR_INSTRET_BASE                     0xc02
#define CSR_HPMCOUNTER3_BASE                 0xc03
#define CSR_HPMCOUNTER4_BASE                 0xc04
#define CSR_HPMCOUNTER5_BASE                 0xc05
#define CSR_HPMCOUNTER6_BASE                 0xc06
#define CSR_HPMCOUNTER7_BASE                 0xc07
#define CSR_HPMCOUNTER8_BASE                 0xc08
#define CSR_HPMCOUNTER9_BASE                 0xc09
#define CSR_HPMCOUNTER10_BASE                0xc0a
#define CSR_HPMCOUNTER11_BASE                0xc0b
#define CSR_HPMCOUNTER12_BASE                0xc0c
#define CSR_HPMCOUNTER13_BASE                0xc0d
#define CSR_HPMCOUNTER14_BASE                0xc0e
#define CSR_HPMCOUNTER15_BASE                0xc0f
#define CSR_HPMCOUNTER16_BASE                0xc10
#define CSR_HPMCOUNTER17_BASE                0xc11
#define CSR_HPMCOUNTER18_BASE                0xc12
#define CSR_HPMCOUNTER19_BASE                0xc13
#define CSR_HPMCOUNTER20_BASE                0xc14
#define CSR_HPMCOUNTER21_BASE                0xc15
#define CSR_HPMCOUNTER22_BASE                0xc16
#define CSR_HPMCOUNTER23_BASE                0xc17
#define CSR_HPMCOUNTER24_BASE                0xc18
#define CSR_HPMCOUNTER25_BASE                0xc19
#define CSR_HPMCOUNTER26_BASE                0xc1a
#define CSR_HPMCOUNTER27_BASE                0xc1b
#define CSR_HPMCOUNTER28_BASE                0xc1c
#define CSR_HPMCOUNTER29_BASE                0xc1d
#define CSR_HPMCOUNTER30_BASE                0xc1e
#define CSR_HPMCOUNTER31_BASE                0xc1f
#define CSR_CYCLEH_BASE                      0xc80
#define CSR_TIMEH_BASE                       0xc81
#define CSR_INSTRETH_BASE                    0xc82
#define CSR_HPMCOUNTER3H_BASE                0xc83
#define CSR_HPMCOUNTER4H_BASE                0xc84
#define CSR_HPMCOUNTER5H_BASE                0xc85
#define CSR_HPMCOUNTER6H_BASE                0xc86
#define CSR_HPMCOUNTER7H_BASE                0xc87
#define CSR_HPMCOUNTER8H_BASE                0xc88
#define CSR_HPMCOUNTER9H_BASE                0xc89
#define CSR_HPMCOUNTER10H_BASE               0xc8a
#define CSR_HPMCOUNTER11H_BASE               0xc8b
#define CSR_HPMCOUNTER12H_BASE               0xc8c
#define CSR_HPMCOUNTER13H_BASE               0xc8d
#define CSR_HPMCOUNTER14H_BASE               0xc8e
#define CSR_HPMCOUNTER15H_BASE               0xc8f
#define CSR_HPMCOUNTER16H_BASE               0xc90
#define CSR_HPMCOUNTER17H_BASE               0xc91
#define CSR_HPMCOUNTER18H_BASE               0xc92
#define CSR_HPMCOUNTER19H_BASE               0xc93
#define CSR_HPMCOUNTER20H_BASE               0xc94
#define CSR_HPMCOUNTER21H_BASE               0xc95
#define CSR_HPMCOUNTER22H_BASE               0xc96
#define CSR_HPMCOUNTER23H_BASE               0xc97
#define CSR_HPMCOUNTER24H_BASE               0xc98
#define CSR_HPMCOUNTER25H_BASE               0xc99
#define CSR_HPMCOUNTER26H_BASE               0xc9a
#define CSR_HPMCOUNTER27H_BASE               0xc9b
#define CSR_HPMCOUNTER28H_BASE               0xc9c
#define CSR_HPMCOUNTER29H_BASE               0xc9d
#define CSR_HPMCOUNTER30H_BASE               0xc9e
#define CSR_HPMCOUNTER31H_BASE               0xc9f
/* === Currently allocated RISC-V supervisor-level CSR addresses === */
#define CSR_SSTATUS_BASE                     0x100
#define CSR_SEDELEG_BASE                     0x102
#define CSR_SIDELEG_BASE                     0x103
#define CSR_SIE_BASE                         0x104
#define CSR_STVEC_BASE                       0x105
#define CSR_SCOUNTEREN_BASE                  0x106
#define CSR_SSCRATCH_BASE                    0x140
#define CSR_SEPC_BASE                        0x141
#define CSR_SCAUSE_BASE                      0x142
#define CSR_SBADADDR_BASE                    0x143
#define CSR_SIP_BASE                         0x144
#define CSR_SPTBR_BASE                       0x180
/* === Currently allocated RISC-V machine-level CSR addresses === */
#define CSR_MVENDORID_BASE                   0xf11
#define CSR_MARCHID_BASE                     0xf12
#define CSR_MIMPID_BASE                      0xf13
#define CSR_MHARTID_BASE                     0xf14
#define CSR_MSTATUS_BASE                     0x300
#define CSR_MISA_BASE                        0x301
#define CSR_MEDELEG_BASE                     0x302
#define CSR_MIDELEG_BASE                     0x303
#define CSR_MIE_BASE                         0x304
#define CSR_MTVEC_BASE                       0x305
#define CSR_MCOUNTEREN_BASE                  0x306
#define CSR_MSCRATCH_BASE                    0x340
#define CSR_MEPC_BASE                        0x341
#define CSR_MCAUSE_BASE                      0x342
#define CSR_MBADADDR_BASE                    0x343
#define CSR_MTVAL_BASE                       0x343
#define CSR_MIP_BASE                         0x344
#define CSR_PMPCFG0_BASE                     0x3a0
#define CSR_PMPCFG1_BASE                     0x3a1
#define CSR_PMPCFG2_BASE                     0x3a2
#define CSR_PMPCFG3_BASE                     0x3a3
#define CSR_PMPADDR0_BASE                    0x3b0
#define CSR_PMPADDR1_BASE                    0x3b1
#define CSR_PMPADDR2_BASE                    0x3b2
#define CSR_PMPADDR3_BASE                    0x3b3
#define CSR_PMPADDR4_BASE                    0x3b4
#define CSR_PMPADDR5_BASE                    0x3b5
#define CSR_PMPADDR6_BASE                    0x3b6
#define CSR_PMPADDR7_BASE                    0x3b7
#define CSR_PMPADDR8_BASE                    0x3b8
#define CSR_PMPADDR9_BASE                    0x3b9
#define CSR_PMPADDR10_BASE                   0x3ba
#define CSR_PMPADDR11_BASE                   0x3bb
#define CSR_PMPADDR12_BASE                   0x3bc
#define CSR_PMPADDR13_BASE                   0x3bd
#define CSR_PMPADDR14_BASE                   0x3be
#define CSR_PMPADDR15_BASE                   0x3bf
#define CSR_MCYCLE_BASE                      0xb00
#define CSR_MINSTRET_BASE                    0xb02
#define CSR_MHPMCOUNTER3_BASE                0xb03
#define CSR_MHPMCOUNTER4_BASE                0xb04
#define CSR_MHPMCOUNTER5_BASE                0xb05
#define CSR_MHPMCOUNTER6_BASE                0xb06
#define CSR_MHPMCOUNTER7_BASE                0xb07
#define CSR_MHPMCOUNTER8_BASE                0xb08
#define CSR_MHPMCOUNTER9_BASE                0xb09
#define CSR_MHPMCOUNTER10_BASE               0xb0a
#define CSR_MHPMCOUNTER11_BASE               0xb0b
#define CSR_MHPMCOUNTER12_BASE               0xb0c
#define CSR_MHPMCOUNTER13_BASE               0xb0d
#define CSR_MHPMCOUNTER14_BASE               0xb0e
#define CSR_MHPMCOUNTER15_BASE               0xb0f
#define CSR_MHPMCOUNTER16_BASE               0xb10
#define CSR_MHPMCOUNTER17_BASE               0xb11
#define CSR_MHPMCOUNTER18_BASE               0xb12
#define CSR_MHPMCOUNTER19_BASE               0xb13
#define CSR_MHPMCOUNTER20_BASE               0xb14
#define CSR_MHPMCOUNTER21_BASE               0xb15
#define CSR_MHPMCOUNTER22_BASE               0xb16
#define CSR_MHPMCOUNTER23_BASE               0xb17
#define CSR_MHPMCOUNTER24_BASE               0xb18
#define CSR_MHPMCOUNTER25_BASE               0xb19
#define CSR_MHPMCOUNTER26_BASE               0xb1a
#define CSR_MHPMCOUNTER27_BASE               0xb1b
#define CSR_MHPMCOUNTER28_BASE               0xb1c
#define CSR_MHPMCOUNTER29_BASE               0xb1d
#define CSR_MHPMCOUNTER30_BASE               0xb1e
#define CSR_MHPMCOUNTER31_BASE               0xb1f
#define CSR_MCYCLEH_BASE                     0xb80
#define CSR_MINSTRETH_BASE                   0xb82
#define CSR_MHPMCOUNTER3H_BASE               0xb83
#define CSR_MHPMCOUNTER4H_BASE               0xb84
#define CSR_MHPMCOUNTER5H_BASE               0xb85
#define CSR_MHPMCOUNTER6H_BASE               0xb86
#define CSR_MHPMCOUNTER7H_BASE               0xb87
#define CSR_MHPMCOUNTER8H_BASE               0xb88
#define CSR_MHPMCOUNTER9H_BASE               0xb89
#define CSR_MHPMCOUNTER10H_BASE              0xb8a
#define CSR_MHPMCOUNTER11H_BASE              0xb8b
#define CSR_MHPMCOUNTER12H_BASE              0xb8c
#define CSR_MHPMCOUNTER13H_BASE              0xb8d
#define CSR_MHPMCOUNTER14H_BASE              0xb8e
#define CSR_MHPMCOUNTER15H_BASE              0xb8f
#define CSR_MHPMCOUNTER16H_BASE              0xb90
#define CSR_MHPMCOUNTER17H_BASE              0xb91
#define CSR_MHPMCOUNTER18H_BASE              0xb92
#define CSR_MHPMCOUNTER19H_BASE              0xb93
#define CSR_MHPMCOUNTER20H_BASE              0xb94
#define CSR_MHPMCOUNTER21H_BASE              0xb95
#define CSR_MHPMCOUNTER22H_BASE              0xb96
#define CSR_MHPMCOUNTER23H_BASE              0xb97
#define CSR_MHPMCOUNTER24H_BASE              0xb98
#define CSR_MHPMCOUNTER25H_BASE              0xb99
#define CSR_MHPMCOUNTER26H_BASE              0xb9a
#define CSR_MHPMCOUNTER27H_BASE              0xb9b
#define CSR_MHPMCOUNTER28H_BASE              0xb9c
#define CSR_MHPMCOUNTER29H_BASE              0xb9d
#define CSR_MHPMCOUNTER30H_BASE              0xb9e
#define CSR_MHPMCOUNTER31H_BASE              0xb9f
#define CSR_MUCOUNTEREN_BASE                 0x320
#define CSR_MSCOUNTEREN_BASE                 0x321
#define CSR_MHPMEVENT3_BASE                  0x323
#define CSR_MHPMEVENT4_BASE                  0x324
#define CSR_MHPMEVENT5_BASE                  0x325
#define CSR_MHPMEVENT6_BASE                  0x326
#define CSR_MHPMEVENT7_BASE                  0x327
#define CSR_MHPMEVENT8_BASE                  0x328
#define CSR_MHPMEVENT9_BASE                  0x329
#define CSR_MHPMEVENT10_BASE                 0x32a
#define CSR_MHPMEVENT11_BASE                 0x32b
#define CSR_MHPMEVENT12_BASE                 0x32c
#define CSR_MHPMEVENT13_BASE                 0x32d
#define CSR_MHPMEVENT14_BASE                 0x32e
#define CSR_MHPMEVENT15_BASE                 0x32f
#define CSR_MHPMEVENT16_BASE                 0x330
#define CSR_MHPMEVENT17_BASE                 0x331
#define CSR_MHPMEVENT18_BASE                 0x332
#define CSR_MHPMEVENT19_BASE                 0x333
#define CSR_MHPMEVENT20_BASE                 0x334
#define CSR_MHPMEVENT21_BASE                 0x335
#define CSR_MHPMEVENT22_BASE                 0x336
#define CSR_MHPMEVENT23_BASE                 0x337
#define CSR_MHPMEVENT24_BASE                 0x338
#define CSR_MHPMEVENT25_BASE                 0x339
#define CSR_MHPMEVENT26_BASE                 0x33a
#define CSR_MHPMEVENT27_BASE                 0x33b
#define CSR_MHPMEVENT28_BASE                 0x33c
#define CSR_MHPMEVENT29_BASE                 0x33d
#define CSR_MHPMEVENT30_BASE                 0x33e
#define CSR_MHPMEVENT31_BASE                 0x33f
#define CSR_TSELECT_BASE                     0x7a0
#define CSR_TDATA1_BASE                      0x7a1
#define CSR_TDATA2_BASE                      0x7a2
#define CSR_TDATA3_BASE                      0x7a3
#define CSR_DCSR_BASE                        0x7b0
#define CSR_DPC_BASE                         0x7b1
#define CSR_DSCRATCH0_BASE                   0x7b2
#define CSR_DSCRATCH1_BASE                   0x7b3

/** @} */ /** End of Doxygen Group RSIS_Core_CSR_Registers **/

#ifdef __cplusplus
}
#endif
#endif /* __RISCV_ENCODING_H__ */
