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
#include <dfs_fs.h>

int sysinfo_dfs_handle(void *shm)
{
    struct statfs buffer;
    int result;
    rt_uint16_t len = sizeof(rt_uint64_t)*3;
    result = dfs_statfs("/", &buffer);
    if (result != 0)
    {
        return -1;
    }
    unsigned long long all_size, free_size, used_size;
    all_size = ((long long)buffer.f_blocks) * ((long long)buffer.f_bsize)/1024LL;
    free_size = ((long long)buffer.f_bfree) * ((long long)buffer.f_bsize)/1024LL;
    used_size = all_size - free_size;
    memcpy((void *)shm+TYPE_LEN, &len, LENGTH_LEN);
    memcpy((void *)shm+TYPE_LEN+LENGTH_LEN, &all_size, sizeof(all_size));
    memcpy((void *)shm+TYPE_LEN+LENGTH_LEN+sizeof(all_size), &free_size, sizeof(free_size));
    memcpy((void *)shm+TYPE_LEN+LENGTH_LEN+sizeof(all_size)+sizeof(free_size), &used_size, sizeof(used_size));
    return 0;
}
