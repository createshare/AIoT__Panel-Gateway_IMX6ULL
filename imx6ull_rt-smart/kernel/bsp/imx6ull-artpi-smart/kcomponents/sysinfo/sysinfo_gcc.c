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

void sysinfo_gcc_handle(void *shm)
{
    rt_uint16_t len = strlen(GCC_VERSION_STR);
    memcpy((void *)shm+TYPE_LEN, &len, LENGTH_LEN);
    memcpy((void *)shm+TYPE_LEN+LENGTH_LEN, &(GCC_VERSION_STR), len);
}
