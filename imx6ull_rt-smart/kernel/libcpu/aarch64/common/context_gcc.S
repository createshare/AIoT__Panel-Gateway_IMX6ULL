/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-18     Jesven       the first version
 */

#include "rtconfig.h"

#include "asm-fpu.h"

  /*
 *enable gtimer
 */
.globl rt_hw_gtimer_enable
rt_hw_gtimer_enable:
    MOV X0,#1
    MSR CNTP_CTL_EL0,X0
    RET

/*
 *set gtimer CNTP_TVAL_EL0 value
 */
.globl rt_hw_set_gtimer_val
rt_hw_set_gtimer_val:
    MSR CNTP_TVAL_EL0,X0
    RET

/*
 *get gtimer CNTP_TVAL_EL0 value
 */
.globl rt_hw_get_gtimer_val
rt_hw_get_gtimer_val:
    MRS X0,CNTP_TVAL_EL0
    RET


.globl rt_hw_get_cntpct_val
rt_hw_get_cntpct_val:
    MRS X0, CNTPCT_EL0
    RET

/*
 *get gtimer frq value
 */
.globl rt_hw_get_gtimer_frq
rt_hw_get_gtimer_frq:
    MRS X0,CNTFRQ_EL0
    RET

.macro SAVE_CONTEXT
    /* Save the entire context. */
    SAVE_FPU SP
    STP     X0, X1, [SP, #-0x10]!
    STP     X2, X3, [SP, #-0x10]!
    STP     X4, X5, [SP, #-0x10]!
    STP     X6, X7, [SP, #-0x10]!
    STP     X8, X9, [SP, #-0x10]!
    STP     X10, X11, [SP, #-0x10]!
    STP     X12, X13, [SP, #-0x10]!
    STP     X14, X15, [SP, #-0x10]!
    STP     X16, X17, [SP, #-0x10]!
    STP     X18, X19, [SP, #-0x10]!
    STP     X20, X21, [SP, #-0x10]!
    STP     X22, X23, [SP, #-0x10]!
    STP     X24, X25, [SP, #-0x10]!
    STP     X26, X27, [SP, #-0x10]!
    STP     X28, X29, [SP, #-0x10]!
    MRS     X28, FPCR
    MRS     X29, FPSR
    STP     X28, X29, [SP, #-0x10]!
    MRS     X29, SP_EL0
    STP     X29, X30, [SP, #-0x10]!

    MRS     X3, SPSR_EL1
    MRS     X2, ELR_EL1

    STP     X2, X3, [SP, #-0x10]!

    MOV     X0, SP   /* Move SP into X0 for saving. */
.endm

.macro SAVE_CONTEXT_FROM_EL1
    /* Save the entire context. */
    SAVE_FPU SP
    STP     X0, X1, [SP, #-0x10]!
    STP     X2, X3, [SP, #-0x10]!
    STP     X4, X5, [SP, #-0x10]!
    STP     X6, X7, [SP, #-0x10]!
    STP     X8, X9, [SP, #-0x10]!
    STP     X10, X11, [SP, #-0x10]!
    STP     X12, X13, [SP, #-0x10]!
    STP     X14, X15, [SP, #-0x10]!
    STP     X16, X17, [SP, #-0x10]!
    STP     X18, X19, [SP, #-0x10]!
    STP     X20, X21, [SP, #-0x10]!
    STP     X22, X23, [SP, #-0x10]!
    STP     X24, X25, [SP, #-0x10]!
    STP     X26, X27, [SP, #-0x10]!
    STP     X28, X29, [SP, #-0x10]!
    MRS     X28, FPCR
    MRS     X29, FPSR
    STP     X28, X29, [SP, #-0x10]!
    MRS     X29, SP_EL0
    STP     X29, X30, [SP, #-0x10]!

    MOV     X3, #((3 << 6) | 0x4 | 0x1)  /* el1h, disable interrupt */
    MOV     X2, X30

    STP     X2, X3, [SP, #-0x10]!
.endm

.macro RESTORE_CONTEXT
    /* Set the SP to point to the stack of the task being restored. */
    MOV     SP, X0

    BL      lwp_check_exit

    BL      rt_thread_self
    MOV     X19, X0
    BL      lwp_mmu_switch
    MOV     X0, X19
    BL      lwp_user_setting_restore

    LDP     X2, X3, [SP], #0x10  /* SPSR and ELR. */

    TST     X3, #0x1f
    MSR     SPSR_EL1, X3
    MSR     ELR_EL1, X2

    LDP     X29, X30, [SP], #0x10
    MSR     SP_EL0, X29
    LDP     X28, X29, [SP], #0x10
    MSR     FPCR, X28
    MSR     FPSR, X29
    LDP     X28, X29, [SP], #0x10
    LDP     X26, X27, [SP], #0x10
    LDP     X24, X25, [SP], #0x10
    LDP     X22, X23, [SP], #0x10
    LDP     X20, X21, [SP], #0x10
    LDP     X18, X19, [SP], #0x10
    LDP     X16, X17, [SP], #0x10
    LDP     X14, X15, [SP], #0x10
    LDP     X12, X13, [SP], #0x10
    LDP     X10, X11, [SP], #0x10
    LDP     X8, X9, [SP], #0x10
    LDP     X6, X7, [SP], #0x10
    LDP     X4, X5, [SP], #0x10
    LDP     X2, X3, [SP], #0x10
    LDP     X0, X1, [SP], #0x10
    RESTORE_FPU SP

    BEQ     ret_to_user

    ERET
.endm

.macro RESTORE_CONTEXT_WITHOUT_MMU_SWITCH
    /* the SP is already ok */
    BL      lwp_check_exit

    LDP     X2, X3, [SP], #0x10  /* SPSR and ELR. */

    TST     X3, #0x1f
    MSR     SPSR_EL1, X3
    MSR     ELR_EL1, X2

    LDP     X29, X30, [SP], #0x10
    MSR     SP_EL0, X29
    LDP     X28, X29, [SP], #0x10
    MSR     FPCR, X28
    MSR     FPSR, X29
    LDP     X28, X29, [SP], #0x10
    LDP     X26, X27, [SP], #0x10
    LDP     X24, X25, [SP], #0x10
    LDP     X22, X23, [SP], #0x10
    LDP     X20, X21, [SP], #0x10
    LDP     X18, X19, [SP], #0x10
    LDP     X16, X17, [SP], #0x10
    LDP     X14, X15, [SP], #0x10
    LDP     X12, X13, [SP], #0x10
    LDP     X10, X11, [SP], #0x10
    LDP     X8, X9, [SP], #0x10
    LDP     X6, X7, [SP], #0x10
    LDP     X4, X5, [SP], #0x10
    LDP     X2, X3, [SP], #0x10
    LDP     X0, X1, [SP], #0x10
    RESTORE_FPU SP

    BEQ     ret_to_user

    ERET
.endm

.text
/*
 * rt_base_t rt_hw_interrupt_disable();
 */
.globl rt_hw_interrupt_disable
rt_hw_interrupt_disable:
    MRS     X0, DAIF
    MSR     DAIFSet, #3
    DSB     SY
    RET

/*
 * void rt_hw_interrupt_enable(rt_base_t level);
 */
.globl rt_hw_interrupt_enable
rt_hw_interrupt_enable:
    DSB     SY
    AND     X0, X0, #0xc0
    MRS     X1, DAIF
    BIC     X1, X1, #0xc0
    ORR     X0, X0, X1
    MSR     DAIF, X0
    RET

/*
 * void rt_hw_context_switch_to(rt_ubase_t to);
 * X0 --> to sp
 */
.globl rt_hw_context_switch_to
rt_hw_context_switch_to:
    LDR     X0, [X0]
    RESTORE_CONTEXT

.text
/*
 * void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to);
 * X0 --> from sp
 * X1 --> to sp
 * X2 --> to thread
 */
.globl rt_hw_context_switch
rt_hw_context_switch:

    SAVE_CONTEXT_FROM_EL1

    MOV    X2, SP
    STR    X2, [X0]            // store sp in preempted tasks TCB
    LDR    X0, [X1]            // get new task stack pointer

    RESTORE_CONTEXT

/*
 * void rt_hw_context_switch_interrupt(rt_ubase_t from, rt_ubase_t to, rt_thread_t from_thread, rt_thread_t to_thread);
 */
.globl rt_thread_switch_interrupt_flag
.globl rt_interrupt_from_thread
.globl rt_interrupt_to_thread
.globl rt_hw_context_switch_interrupt
rt_hw_context_switch_interrupt:
    ADR     X6, rt_thread_switch_interrupt_flag
    LDR     X7, [X6]
    CMP     X7, #1
    B.EQ     _reswitch
    ADR     X4, rt_interrupt_from_thread   // set rt_interrupt_from_thread
    STR     X0, [X4]
    MOV     X7, #1              // set rt_thread_switch_interrupt_flag to 1
    STR     X7, [X6]
    STP     X1, X30, [SP, #-0x10]!
    MOV     X0, X2
    BL      lwp_user_setting_save
    LDP     X1, X30, [SP], #0x10
_reswitch:
    ADR     X6, rt_interrupt_to_thread     // set rt_interrupt_to_thread
    STR     X1, [X6]
    RET

.text

// -- Exception handlers ----------------------------------

    .align  8
.globl vector_fiq
vector_fiq:
    SAVE_CONTEXT
    STP     X0, X1, [SP, #-0x10]!
    BL      rt_hw_trap_fiq
    LDP     X0, X1, [SP], #0x10
    RESTORE_CONTEXT

.globl      rt_interrupt_enter
.globl      rt_interrupt_leave
.globl      rt_thread_switch_interrupt_flag
.globl      rt_interrupt_from_thread
.globl      rt_interrupt_to_thread


// -------------------------------------------------------------------

    .align  8
.globl vector_irq
vector_irq:
    SAVE_CONTEXT
    STP     X0, X1, [SP, #-0x10]!   /* X0 is thread sp */

    BL      rt_interrupt_enter
    BL      rt_hw_trap_irq
    BL      rt_interrupt_leave

    LDP     X0, X1, [SP], #0x10

    // if rt_thread_switch_interrupt_flag set, jump to
    // rt_hw_context_switch_interrupt_do and don't return
    ADR     X1, rt_thread_switch_interrupt_flag
    LDR     X2, [X1]
    CMP     X2, #1
    B.NE    vector_irq_exit

    MOV     X2,  #0         // clear flag
    STR     X2,  [X1]

    ADR     X3,  rt_interrupt_from_thread
    LDR     X4,  [X3]
    STR     x0,  [X4]       // store sp in preempted tasks's TCB

    ADR     x3,  rt_interrupt_to_thread
    LDR     X4,  [X3]
    LDR     x0,  [X4]       // get new task's stack pointer

    RESTORE_CONTEXT

vector_irq_exit:
    MOV     SP, X0
    RESTORE_CONTEXT_WITHOUT_MMU_SWITCH

// -------------------------------------------------

    .globl  vector_exception
vector_exception:
    SAVE_CONTEXT
    STP     X0, X1, [SP, #-0x10]!
    BL      rt_hw_trap_exception
    LDP     X0, X1, [SP], #0x10
    MOV     SP, X0
    RESTORE_CONTEXT_WITHOUT_MMU_SWITCH

    .globl  vector_serror
vector_serror:
    SAVE_CONTEXT
    STP     X0, X1, [SP, #-0x10]!
    BL      rt_hw_trap_serror
    b .

.global switch_mmu
switch_mmu:
    MSR TTBR0_EL1, X0
    MRS X1, TCR_EL1
    CMP X0, XZR
    ORR X1, X1, #(1 << 7)
    BEQ 1f
    BIC X1, X1, #(1 << 7)
1:
    MSR TCR_EL1, X1
    DSB SY
    ISB
    TLBI VMALLE1
    DSB SY
    ISB
    IC IALLUIS
    DSB SY
    ISB
    RET

.global mmu_table_get
mmu_table_get:
    MRS X0, TTBR0_EL1
    RET

