#include "encoding.h"

#define LREG ld
#define SREG sd
#define LFREG flw
#define SFREG fsw
#define REGBYTES 8
#define STKSHIFT 15

/* crt0.S: Entry point for RISC-V METAL programs. */

.section .text.start, "ax", @progbits
.global _start
.type   _start, @function

  /* _start is defined by the METAL to have been called with the following
   * arguments:
   *   a0: the hart ID of the currently executing hart.  Harts can start at
   *       any arbitrary point, it's the C library's job to ensure the code is
   *       safe.
   *   a1: set to 0.
   */
_start:
j 1f
.word 0xdeadbeef
.align 3
.global g_wake_up
g_wake_up:
    .dword 1
    .dword 0
1:
  /* Before doing anything we must initialize the global pointer, as we cannot
   * safely perform any access that may be relaxed without GP being set.  This
   * is done with relaxation disabled to avoid relaxing the address calculation
   * to just "addi gp, gp, 0". */
.option push
.option norelax
#ifdef __riscv_cmodel_compact
1:
  auipc t0, %pcrel_hi(__global_pointer__)
  addi t0, t0, %pcrel_lo(1b)
  ld   gp, 0(t0)
  add  gp, gp, t0
#else
  la gp, __global_pointer$
#endif
.option pop

  li  x1, 0
  li  x2, 0
  li  x3, 0
  li  x4, 0
  li  x5, 0
  li  x6, 0
  li  x7, 0
  li  x8, 0
  li  x9, 0
  li  x10,0
  li  x11,0
  li  x12,0
  li  x13,0
  li  x14,0
  li  x15,0
  li  x16,0
  li  x17,0
  li  x18,0
  li  x19,0
  li  x20,0
  li  x21,0
  li  x22,0
  li  x23,0
  li  x24,0
  li  x25,0
  li  x26,0
  li  x27,0
  li  x28,0
  li  x29,0
  li  x30,0
  li  x31,0

  li t0, MSTATUS_FS
  csrs mstatus, t0

  fssr    x0
  fmv.w.x f0, x0
  fmv.w.x f1, x0
  fmv.w.x f2, x0
  fmv.w.x f3, x0
  fmv.w.x f4, x0
  fmv.w.x f5, x0
  fmv.w.x f6, x0
  fmv.w.x f7, x0
  fmv.w.x f8, x0
  fmv.w.x f9, x0
  fmv.w.x f10,x0
  fmv.w.x f11,x0
  fmv.w.x f12,x0
  fmv.w.x f13,x0
  fmv.w.x f14,x0
  fmv.w.x f15,x0
  fmv.w.x f16,x0
  fmv.w.x f17,x0
  fmv.w.x f18,x0
  fmv.w.x f19,x0
  fmv.w.x f20,x0
  fmv.w.x f21,x0
  fmv.w.x f22,x0
  fmv.w.x f23,x0
  fmv.w.x f24,x0
  fmv.w.x f25,x0
  fmv.w.x f26,x0
  fmv.w.x f27,x0
  fmv.w.x f28,x0
  fmv.w.x f29,x0
  fmv.w.x f30,x0
  fmv.w.x f31,x0

  /* Set up a simple trap entry to catch anything that goes wrong early in
    * the boot process. */
  la t0, early_trap_entry
  csrw mtvec, t0

  /* There may be pre-initialization routines inside the MBI code that run in
    * C, so here we set up a C environment. First we set up a stack pointer,
    * which is left as a weak reference in order to allow initialization
    * routines that do not need a stack to be set up to transparently be
    * called. */
  la  tp, _end + 63
  and tp, tp, -64
  csrr a0, mhartid

  add sp, a0, 1
  sll sp, sp, STKSHIFT
  add sp, sp, tp

  /* Disable and clear all interrupt sources */
  li   a3, -1
  csrc mie, a3
  csrc mip, a3

  /* The delegation CSRs exist if user mode interrupts (N extension) or
   * supervisor mode (S extension) are supported */
  csrr a5, misa
  lui  a4, 0x42
  and  a4, a4, a5
  beqz a4, 1f
  csrc mideleg, a3
  csrc medeleg, a3
1:

  /* The satp CSR exists if supervisor mode (S extension) is supported */
  lui  a4, 0x40
  and  a4, a4, a5
  beqz a4, 1f
  csrc satp, a3
1:

  /* Check RISC-V isa and enable FS bits if Floating Point architecture. */
  li   a4, 0x10028
  and  a5, a5, a4
  beqz a5, 1f
  csrr a5, mstatus
  lui  a4, 0x2
  or   a5, a5, a4
  csrw mstatus, a5
  csrwi fcsr, 0
1:

  /* Check for vector extension support and enable it if found.
   * Omit if toolchain doesn't support the vector extension. */
#ifdef __riscv_v
  csrr a5, misa
  li a4, 0x200000
  and a5, a5, a4
  beqz a5, 1f
  csrr a5, mstatus
  ori a5, a5, 0x200
  csrw mstatus, a5
  vsetivli x0, 0, e8, m1, ta, ma
  csrwi vcsr, 0
1:
#endif

  la t0, trap_entry
  csrw mtvec, t0

  /* Stack pointer is expected to be initialized before _init_bsp *
   * When error happens, the _init_bsp will call exit to handle libc's cleanup routines.
   * Under normal contains this shouldn't even get called, but I'm still not using a tail call here
   * because returning to the METAL is the right thing to do in pathological
   * situations. */
  li a1, 0
  j _init_bsp

  ret

#ifdef __riscv_cmodel_compact
/* Compact stub.  */
  .section .text.__global_pointer__, "aMG",@progbits, 8, __global_pointer__, comdat
  .align 3
  .global __global_pointer__
  .type   __global_pointer__, object
__global_pointer__:
  .quad   __global_pointer$ -.
#endif

/*
 * For sanity's sake we set up an early trap vector that just does nothing.
 * If you end up here then there's a bug in the early boot code somewhere.
 */
.section .text.init.trap
.global early_trap_entry
.type early_trap_entry, @function
.align 8
early_trap_entry:
  csrr t0, mcause
  csrr t1, mepc
  csrr t2, mtval
  j early_trap_entry

.globl trap_entry
.type trap_entry, @function
.align 2
trap_entry:
  addi sp, sp, -64*REGBYTES

  SREG x1, 1*REGBYTES(sp)
  SREG x2, 2*REGBYTES(sp)
  SREG x3, 3*REGBYTES(sp)
  SREG x4, 4*REGBYTES(sp)
  SREG x5, 5*REGBYTES(sp)
  SREG x6, 6*REGBYTES(sp)
  SREG x7, 7*REGBYTES(sp)
  SREG x8, 8*REGBYTES(sp)
  SREG x9, 9*REGBYTES(sp)
  SREG x10, 10*REGBYTES(sp)
  SREG x11, 11*REGBYTES(sp)
  SREG x12, 12*REGBYTES(sp)
  SREG x13, 13*REGBYTES(sp)
  SREG x14, 14*REGBYTES(sp)
  SREG x15, 15*REGBYTES(sp)
  SREG x16, 16*REGBYTES(sp)
  SREG x17, 17*REGBYTES(sp)
  SREG x18, 18*REGBYTES(sp)
  SREG x19, 19*REGBYTES(sp)
  SREG x20, 20*REGBYTES(sp)
  SREG x21, 21*REGBYTES(sp)
  SREG x22, 22*REGBYTES(sp)
  SREG x23, 23*REGBYTES(sp)
  SREG x24, 24*REGBYTES(sp)
  SREG x25, 25*REGBYTES(sp)
  SREG x26, 26*REGBYTES(sp)
  SREG x27, 27*REGBYTES(sp)
  SREG x28, 28*REGBYTES(sp)
  SREG x29, 29*REGBYTES(sp)
  SREG x30, 30*REGBYTES(sp)
  SREG x31, 31*REGBYTES(sp)

  SFREG f0, ( 0  + 32)*REGBYTES(sp)
  SFREG f1, ( 1  + 32)*REGBYTES(sp)
  SFREG f2, ( 2  + 32)*REGBYTES(sp)
  SFREG f3, ( 3  + 32)*REGBYTES(sp)
  SFREG f4, ( 4  + 32)*REGBYTES(sp)
  SFREG f5, ( 5  + 32)*REGBYTES(sp)
  SFREG f6, ( 6  + 32)*REGBYTES(sp)
  SFREG f7, ( 7  + 32)*REGBYTES(sp)
  SFREG f8, ( 8  + 32)*REGBYTES(sp)
  SFREG f9, ( 9  + 32)*REGBYTES(sp)
  SFREG f10,( 10 + 32)*REGBYTES(sp)
  SFREG f11,( 11 + 32)*REGBYTES(sp)
  SFREG f12,( 12 + 32)*REGBYTES(sp)
  SFREG f13,( 13 + 32)*REGBYTES(sp)
  SFREG f14,( 14 + 32)*REGBYTES(sp)
  SFREG f15,( 15 + 32)*REGBYTES(sp)
  SFREG f16,( 16 + 32)*REGBYTES(sp)
  SFREG f17,( 17 + 32)*REGBYTES(sp)
  SFREG f18,( 18 + 32)*REGBYTES(sp)
  SFREG f19,( 19 + 32)*REGBYTES(sp)
  SFREG f20,( 20 + 32)*REGBYTES(sp)
  SFREG f21,( 21 + 32)*REGBYTES(sp)
  SFREG f22,( 22 + 32)*REGBYTES(sp)
  SFREG f23,( 23 + 32)*REGBYTES(sp)
  SFREG f24,( 24 + 32)*REGBYTES(sp)
  SFREG f25,( 25 + 32)*REGBYTES(sp)
  SFREG f26,( 26 + 32)*REGBYTES(sp)
  SFREG f27,( 27 + 32)*REGBYTES(sp)
  SFREG f28,( 28 + 32)*REGBYTES(sp)
  SFREG f29,( 29 + 32)*REGBYTES(sp)
  SFREG f30,( 30 + 32)*REGBYTES(sp)
  SFREG f31,( 31 + 32)*REGBYTES(sp)

  csrr a0, mcause
  csrr a1, mepc
  mv a2, sp
  add a3, sp, 32*REGBYTES
  bgez a0, .handle_syscall
.handle_irq:
  jal handle_irq
  j .restore
.handle_syscall:
  jal handle_syscall
.restore:
  csrw mepc, a0
  LREG x1, 1*REGBYTES(sp)
  LREG x2, 2*REGBYTES(sp)
  LREG x3, 3*REGBYTES(sp)
  LREG x4, 4*REGBYTES(sp)
  LREG x5, 5*REGBYTES(sp)
  LREG x6, 6*REGBYTES(sp)
  LREG x7, 7*REGBYTES(sp)
  LREG x8, 8*REGBYTES(sp)
  LREG x9, 9*REGBYTES(sp)
  LREG x10, 10*REGBYTES(sp)
  LREG x11, 11*REGBYTES(sp)
  LREG x12, 12*REGBYTES(sp)
  LREG x13, 13*REGBYTES(sp)
  LREG x14, 14*REGBYTES(sp)
  LREG x15, 15*REGBYTES(sp)
  LREG x16, 16*REGBYTES(sp)
  LREG x17, 17*REGBYTES(sp)
  LREG x18, 18*REGBYTES(sp)
  LREG x19, 19*REGBYTES(sp)
  LREG x20, 20*REGBYTES(sp)
  LREG x21, 21*REGBYTES(sp)
  LREG x22, 22*REGBYTES(sp)
  LREG x23, 23*REGBYTES(sp)
  LREG x24, 24*REGBYTES(sp)
  LREG x25, 25*REGBYTES(sp)
  LREG x26, 26*REGBYTES(sp)
  LREG x27, 27*REGBYTES(sp)
  LREG x28, 28*REGBYTES(sp)
  LREG x29, 29*REGBYTES(sp)
  LREG x30, 30*REGBYTES(sp)
  LREG x31, 31*REGBYTES(sp)

  LFREG f0, ( 0  + 32)*REGBYTES(sp)
  LFREG f1, ( 1  + 32)*REGBYTES(sp)
  LFREG f2, ( 2  + 32)*REGBYTES(sp)
  LFREG f3, ( 3  + 32)*REGBYTES(sp)
  LFREG f4, ( 4  + 32)*REGBYTES(sp)
  LFREG f5, ( 5  + 32)*REGBYTES(sp)
  LFREG f6, ( 6  + 32)*REGBYTES(sp)
  LFREG f7, ( 7  + 32)*REGBYTES(sp)
  LFREG f8, ( 8  + 32)*REGBYTES(sp)
  LFREG f9, ( 9  + 32)*REGBYTES(sp)
  LFREG f10,( 10 + 32)*REGBYTES(sp)
  LFREG f11,( 11 + 32)*REGBYTES(sp)
  LFREG f12,( 12 + 32)*REGBYTES(sp)
  LFREG f13,( 13 + 32)*REGBYTES(sp)
  LFREG f14,( 14 + 32)*REGBYTES(sp)
  LFREG f15,( 15 + 32)*REGBYTES(sp)
  LFREG f16,( 16 + 32)*REGBYTES(sp)
  LFREG f17,( 17 + 32)*REGBYTES(sp)
  LFREG f18,( 18 + 32)*REGBYTES(sp)
  LFREG f19,( 19 + 32)*REGBYTES(sp)
  LFREG f20,( 20 + 32)*REGBYTES(sp)
  LFREG f21,( 21 + 32)*REGBYTES(sp)
  LFREG f22,( 22 + 32)*REGBYTES(sp)
  LFREG f23,( 23 + 32)*REGBYTES(sp)
  LFREG f24,( 24 + 32)*REGBYTES(sp)
  LFREG f25,( 25 + 32)*REGBYTES(sp)
  LFREG f26,( 26 + 32)*REGBYTES(sp)
  LFREG f27,( 27 + 32)*REGBYTES(sp)
  LFREG f28,( 28 + 32)*REGBYTES(sp)
  LFREG f29,( 29 + 32)*REGBYTES(sp)
  LFREG f30,( 30 + 32)*REGBYTES(sp)
  LFREG f31,( 31 + 32)*REGBYTES(sp)

  addi sp, sp, 64*REGBYTES
  mret

.section ".tdata.begin"
.globl _tdata_begin
_tdata_begin:

.section ".tdata.end"
.globl _tdata_end
_tdata_end:

.section ".tbss.end"
.globl _tbss_end
_tbss_end:
