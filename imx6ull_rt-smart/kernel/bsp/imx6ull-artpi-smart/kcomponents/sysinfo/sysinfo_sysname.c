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

void sysinfo_sysname_handle(void *shm)
{
    rt_uint16_t len = sizeof(rt_uint32_t)*3;
    rt_uint32_t version = RT_VERSION;
    rt_uint32_t subversion = RT_SUBVERSION;
    rt_uint32_t revision = RT_REVISION;
    memcpy((void *)shm+TYPE_LEN, &len, LENGTH_LEN);
    memcpy((void *)shm+TYPE_LEN+LENGTH_LEN, &version, sizeof(version));
    memcpy((void *)shm+TYPE_LEN+LENGTH_LEN+sizeof(version), &subversion, sizeof(subversion));
    memcpy((void *)shm+TYPE_LEN+LENGTH_LEN+sizeof(version)+sizeof(subversion), &revision, sizeof(revision));
}
