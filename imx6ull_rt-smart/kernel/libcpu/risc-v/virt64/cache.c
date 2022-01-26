/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-01-29     lizhirui     first version
 */

#include <rthw.h>
#include <rtdef.h>
#include <board.h>
#include <riscv.h>

rt_inline rt_uint32_t rt_cpu_icache_line_size()
{
    return 0;
}

rt_inline rt_uint32_t rt_cpu_dcache_line_size()
{
    return 0;
}

void rt_hw_cpu_icache_invalidate(void *addr,int size)
{

}

void rt_hw_cpu_dcache_invalidate(void *addr,int size)
{

}

void rt_hw_cpu_dcache_clean(void *addr,int size)
{

}

void rt_hw_cpu_icache_ops(int ops,void *addr,int size)
{
    if(ops == RT_HW_CACHE_INVALIDATE)
    {
        rt_hw_cpu_icache_invalidate(addr,size);
    }
}

void rt_hw_cpu_dcache_ops(int ops,void *addr,int size)
{
    if(ops == RT_HW_CACHE_FLUSH)
    {
        rt_hw_cpu_dcache_clean(addr,size);
    }
    else
    {
        rt_hw_cpu_dcache_invalidate(addr,size);
    }
}

void rt_hw_cpu_dcache_flush_all()
{

}

void rt_hw_cpu_icache_invalidate_all()
{

}

rt_base_t rt_hw_cpu_icache_status()
{
    return 0;
}

rt_base_t rt_hw_cpu_dcache_status()
{
    return 0;
}

int sys_cacheflush(void *addr, int size, int cache)
{
    return 0;
}
