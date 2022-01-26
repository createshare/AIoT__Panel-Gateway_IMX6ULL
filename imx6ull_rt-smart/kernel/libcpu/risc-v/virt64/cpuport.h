/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-10-03     Bernard      The first version
 */

#ifndef CPUPORT_H__
#define CPUPORT_H__

#include <rtconfig.h>

/* bytes of register width  */
#ifdef ARCH_CPU_64BIT
#define STORE                   sd
#define LOAD                    ld
#define REGBYTES                8
#else
// error here, not portable
#endif

/* 33 general register */
#define CTX_GENERAL_REG_NR  33

#ifdef ENABLE_FPU
/* 32 fpu register */
#define CTX_FPU_REG_NR  32
#else
#define CTX_FPU_REG_NR  0
#endif

/* all context registers */
#define CTX_REG_NR  (CTX_GENERAL_REG_NR + CTX_FPU_REG_NR)

#ifndef __ASSEMBLY__
rt_inline void rt_hw_dsb()
{
    asm volatile("fence":::"memory");
}

rt_inline void rt_hw_dmb()
{
    asm volatile("fence":::"memory");
}

rt_inline void rt_hw_isb()
{
    asm volatile("fence.i":::"memory");
}

#endif

#endif
#ifdef RISCV_U_MODE
#define RISCV_USER_ENTRY 0xFFFFFFE000000000ULL
#endif
