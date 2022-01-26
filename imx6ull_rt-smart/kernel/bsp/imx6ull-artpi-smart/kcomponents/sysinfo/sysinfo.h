/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change logs:
 * Date            Autor            Notes
 * 2021-07-30      Weilin           The first version
 * 2021-08-27      Weilin           Add cpuusage
 */

#ifndef SYSINFO_H__
#define SYSINFO_H__
#include <rtthread.h>
#include <rthw.h>
#include <lwp.h>
#include <rtdevice.h>

#include "rthw.h"
#include "drv_timer.h"

#ifdef RT_USING_SMP
extern unsigned int g_switch_point_tick[RT_CPUS_NR];
extern rt_uint64_t g_int_duration_tick[RT_CPUS_NR];
#else
extern unsigned int g_switch_point_tick;
extern rt_uint64_t g_int_duration_tick;
#endif

extern unsigned int g_cpu_usage_enable;

//#define CPU_USAGE_GET_TICK (RT_HW_CPU_USAGE_GET_TICK)

#define CPU_USAGE_GET_TICK rt_tick_get()

#ifdef RT_USING_SMP
#define SWITCH_POINT_TICK g_switch_point_tick[rt_hw_cpu_id()]
#define INT_DURATION_TICK g_int_duration_tick[rt_hw_cpu_id()]
#else
#define SWITCH_POINT_TICK g_switch_point_tick
#define INT_DURATION_TICK g_int_duration_tick
#endif

#ifdef RT_USING_HOOK
extern void rt_interrupt_switch_sethook(void (*hook)(void));
#endif

#define LED_PIN     GET_PIN(5, 3)
#define TYPE_LEN 1
#define LENGTH_LEN 2

enum REQUEST_TYPE
{
    REQ_MEM = 1,
    REQ_CPUUSAGE,
    REQ_SYSNAME,
    REQ_BUILDDATE,
    REQ_GCCVERSION,
    OPENLED,
    CLOSELED,
    REQ_LEDSTATUS,
    REQ_DFS,
    REQ_NET,
} request_type;

void sysinfo_mem_handle(void* shm);
void sysinfo_cpu_handle(void* shm);
void sysinfo_sysname_handle(void *shm);
void sysinfo_date_handle(void *shm);
void sysinfo_gcc_handle(void *shm);
void sysinfo_led_control_handle(int type);
void sysinfo_led_status_handle(void *shm);
int sysinfo_dfs_handle(void* shm);
int sysinfo_net_handle(void* shm);


#endif
