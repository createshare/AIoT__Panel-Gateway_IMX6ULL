/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#ifndef  CPUPORT_H__
#define  CPUPORT_H__

#include <armv8.h>

rt_inline void rt_hw_isb(void)
{
    asm volatile ("isb":::"memory");
}

rt_inline void rt_hw_dmb(void)
{
    asm volatile ("dmb sy":::"memory");
}

rt_inline void rt_hw_dsb(void)
{
    asm volatile ("dsb sy":::"memory");
}

#endif  /*CPUPORT_H__*/
