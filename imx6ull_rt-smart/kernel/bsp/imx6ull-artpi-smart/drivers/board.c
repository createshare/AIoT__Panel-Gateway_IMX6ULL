/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-11-20     Bernard    the first version
 * 2018-11-22     Jesven     add rt_hw_spin_lock
 *                           add rt_hw_spin_unlock
 *                           add smp ipi init
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdbg.h>

#include "board.h"

#include <mmu.h>
#ifdef RT_USING_USERSPACE
#include <page.h>
#include <lwp_arch.h>
#endif

extern size_t MMUTable[];
rt_mmu_info mmu_info;

#ifdef RT_USING_USERSPACE
struct mem_desc platform_mem_desc[] = {  /* 100ask_imx6ull ddr 512M */
    {KERNEL_VADDR_START, KERNEL_VADDR_START + 0x1FFFFFFF, KERNEL_VADDR_START + PV_OFFSET, NORMAL_MEM}
};
#else
struct mem_desc platform_mem_desc[] = {
    {0x00000000, 0x80000000, 0x00000000, DEVICE_MEM},
    {0x80000000, 0xFFF00000, 0x80000000, NORMAL_MEM}
};
#endif
const rt_uint32_t platform_mem_desc_size = sizeof(platform_mem_desc)/sizeof(platform_mem_desc[0]);

void idle_wfi(void)
{
    asm volatile ("wfi");
}

/**
 * This function will initialize board
 */

#ifdef RT_USING_USERSPACE
rt_region_t init_page_region = {
    (uint32_t)PAGE_START,
    (uint32_t)PAGE_END,
};
#endif

int board_reboot(int argc, char **argv)
{
    wdog_config_t config;
    SRC_Type *src = (SRC_Type*)g_src_vbase;
    WDOG_Type *wdog = (WDOG_Type*)g_wdog1_vbase;

    LOG_E("resetting ...\n");

    rt_hw_ms_delay(50);

    src->SCR &= ~SRC_SCR_WARM_RESET_ENABLE_MASK;

    CLOCK_EnableClock(kCLOCK_Wdog1);

    WDOG_GetDefaultConfig(&config);
    config.timeoutValue = 0x00u;

    WDOG_Init(wdog, &config);

    while (1)
    {
        //waiting...
    }

    return 0;
}
MSH_CMD_EXPORT_ALIAS(board_reboot, reboot, reboot system);

void rt_hw_board_init(void)
{
#ifdef RT_USING_USERSPACE
    rt_hw_mmu_map_init(&mmu_info, (void*)0xf0000000, 0x10000000, MMUTable, PV_OFFSET);

    rt_page_init(init_page_region);
    rt_hw_mmu_ioremap_init(&mmu_info, (void*)0xf0000000, 0x10000000);

    arch_kuser_init(&mmu_info, (void*)0xffff0000);
#else
    rt_hw_mmu_map_init(&mmu_info, (void*)0x80000000, 0x10000000, MMUTable, 0);
    rt_hw_mmu_ioremap_init(&mmu_info, (void*)0x80000000, 0x10000000);
#endif

    /* initialize hardware interrupt */
    rt_hw_interrupt_init();

    /* initialize system heap */
    rt_system_heap_init(HEAP_BEGIN, HEAP_END);

    SystemAddressMapping();
    SystemClockInit();

    rt_components_board_init();
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);

    rt_thread_idle_sethook(idle_wfi);
}
