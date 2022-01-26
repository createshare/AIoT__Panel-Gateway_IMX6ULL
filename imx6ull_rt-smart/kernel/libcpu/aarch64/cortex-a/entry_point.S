/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Date           Author       Notes
 * 2020-01-15     bigmagic     the first version
 * 2020-08-10     SummerGift   support clang compiler
 */

#include "rtconfig.h"
.section ".text.entrypoint","ax"
.set EL1_stack,         __el1_stack
.global __start

__start:
    /* read cpu id, stop slave cores */
    mrs     x1, mpidr_el1           /* MPIDR_EL1: Multi-Processor Affinity Register */
    and     x1, x1, #3
    cbz     x1, .L__cpu_0           /* .L prefix is the local label in ELF */

    /* cpu id > 0, stop */
    /* cpu id == 0 will also goto here after returned from entry() if possible */
.L__current_cpu_idle:
    wfe
    b       .L__current_cpu_idle

.L__cpu_0:
    /* set stack before our code, Define stack pointer for current exception level */
    /* ldr	x2, =EL1_stack */
    /* mov	sp, x2         */

    adr     x1, __start

    /* set up EL1 */
    mrs     x0, CurrentEL           /* CurrentEL Register. bit 2, 3. Others reserved */
    and     x0, x0, #12             /* clear reserved bits */

    /* running at EL3? */
    cmp     x0, #12                 /* 1100b. So, EL3 */
    bne     .L__not_in_el3          /* 11?  !EL3 -> 5: */

    /* should never be executed, just for completeness. (EL3) */
    mov     x2, #0x5b1
    msr     scr_el3, x2             /* SCR_ELn  Secure Configuration Register */
    mov     x2, #0x3c9
    msr     spsr_el3, x2            /* SPSR_ELn. Saved Program Status Register. 1111001001 */
    adr     x2, .L__not_in_el3
    msr     elr_el3, x2
    eret                            /* Exception Return: from EL3, continue from .L__not_in_el3 */

.L__not_in_el3:                     /* running at EL2 or EL1 */
    cmp     x0, #4                  /* 0x04  0100 EL1 */
    beq     .L__in_el1              /* EL1 -> 5: */
 
    mrs     x0, hcr_el2
    bic     x0, x0, #0xff
    msr     hcr_el2, x0

    msr     sp_el1, x1              /* in EL2, set sp of EL1 to _start */

    /* enable CNTP for EL1 */
    mrs     x0, cnthctl_el2         /* Counter-timer Hypervisor Control register */
    orr     x0, x0, #3
    msr     cnthctl_el2, x0
    msr     cntvoff_el2, xzr

    /* enable AArch64 in EL1 */
    mov     x0, #(1 << 31)          /* AArch64 */
    orr     x0, x0, #(1 << 1)       /* SWIO hardwired on Pi3 */
    msr     hcr_el2, x0
    mrs     x0, hcr_el2

    /* change execution level to EL1 */
    mov     x2, #0x3c4
    msr     spsr_el2, x2            /* 1111000100 */
    adr     x2, .L__in_el1
    msr     elr_el2, x2

    eret                            /* exception return. from EL2. continue from .L__in_el1 */

.L__in_el1:
    ldr     x9, =PV_OFFSET
    mov     sp, x1                  /* in EL1. Set sp to _start */

    /* Set CPACR_EL1 (Architecture Feature Access Control Register) to avoid trap from SIMD or float point instruction */
    mov     x1, #0x00300000         /* Don't trap any SIMD/FP instructions in both EL0 and EL1 */
    msr     cpacr_el1, x1

    /* clear bss */
    ldr     x1, =__bss_start
    add     x1, x1, x9
    ldr     w2, =__bss_size

.L__clean_bss_loop:
    cbz     w2, .L__jump_to_entry
    str     xzr, [x1], #8
    sub     w2, w2, #8
    cbnz    w2, .L__clean_bss_loop

.L__jump_to_entry:          /* jump to C code, should not return */        

    bl get_free_page
    mov x21, x0
    bl get_free_page
    mov x20, x0

    mov x1, x21
    bl mmu_tcr_init

    mov x0, x20
    mov x1, x21

    msr ttbr0_el1, x0
    msr ttbr1_el1, x1
    dsb sy

    ldr x2, =0x40000000      /* map 1G memory for kernel space */
    ldr x3, =PV_OFFSET
    bl rt_hw_mmu_setup_early

    ldr x30, =after_mmu_enable

    mrs x1, sctlr_el1
    bic x1, x1, #(3 << 3)    /* dis SA, SA0 */
    bic x1, x1, #(1 << 1)    /* dis A */
    orr x1, x1, #(1 << 12)   /* I */
    orr x1, x1, #(1 << 2)    /* C */
    orr x1, x1, #(1 << 0)    /* M */
    msr sctlr_el1, x1        /* enable MMU */

    dsb sy
    isb sy
    ic ialluis               /* Invalidate all instruction caches in Inner Shareable domain to Point of Unification */
    dsb sy
    isb sy
    tlbi vmalle1             /* Invalidate all stage 1 translations used at EL1 with the current VMID */
    dsb sy
    isb sy
    ret

after_mmu_enable:
    mrs x0, tcr_el1          /* disable ttbr0, only using kernel space */
    orr x0, x0, #(1 << 7)
    msr tcr_el1, x0
    msr ttbr0_el1, xzr
    dsb sy

    mov     x0, #1
    msr     spsel, x0
    adr     x1, __start
    mov     sp, x1           /* sp_el1 set to _start */

    b  rtthread_startup
