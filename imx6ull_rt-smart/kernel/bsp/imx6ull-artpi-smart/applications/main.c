/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020/10/7      bernard      the first version
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "msh.h"


int main(void)
{
    rt_kprintf("hello rt-smart!\n");


    msh_exec("/root/bin/uhttpd.elf -f -p 80 -h /root/var/dist &", 70);
    return 0;
}

