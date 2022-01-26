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
#include "drv_pin.h"

void sysinfo_led_control_handle(int type)
{
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
    if(type)
    {
        rt_pin_write(LED_PIN, PIN_LOW);
    }
    else
    {
        rt_pin_write(LED_PIN, PIN_HIGH);
    }
}

void sysinfo_led_status_handle(void *shm)
{
    rt_uint16_t len = sizeof(rt_uint32_t);
    int led_status = rt_pin_read(LED_PIN);
    memcpy((void *)shm+TYPE_LEN, &len, LENGTH_LEN);
    memcpy((void *)shm+TYPE_LEN+LENGTH_LEN, &led_status, sizeof(led_status));
}
