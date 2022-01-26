/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2021-07-30     Weilin         The first version
 */

#include "sysinfo.h"

/* put memory informations into buf */
void rt_memory_get(void* buf)
{
    rt_uint32_t total_mem,used_mem,max_mem;
    rt_memory_info(&total_mem,&used_mem,&max_mem);
    size_t total_pages = 0, free_pages = 0;
    rt_page_get_info(&total_pages, &free_pages);
    total_mem = total_mem + total_pages * ARCH_PAGE_SIZE;
    used_mem = used_mem + (total_pages - free_pages) * ARCH_PAGE_SIZE;
    int size = sizeof(rt_uint32_t);
    memcpy(buf, &total_mem, size);
    memcpy(buf+size, &used_mem, size);
    memcpy(buf+size*2, &max_mem, size);
}

void sysinfo_mem_handle(void* shm)
{
    rt_uint16_t len = sizeof(rt_uint32_t)*3;
    unsigned char memory_buf[len];
    rt_memory_get(memory_buf);
    memcpy((void *)shm+1, &len, 2);
    memcpy((void *)shm+3, memory_buf, len);
}
