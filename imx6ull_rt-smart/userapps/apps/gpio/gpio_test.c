/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change logs:
 * Date            Autor            Notes
 * 2021-08-08      WWL           The first version
 */

#include "gpio_test.h"

#define LED_PIN  GET_PIN(5,3)

int main()
{
    rt_device_t pin_dev;
    struct rt_device_pin_mode pin_mode;
    struct rt_device_pin_status pin_status;

    pin_dev = rt_device_find("pin");
    if (pin_dev == RT_NULL)
    {
        rt_kprintf("not find pin device!\n");
        return -1;
    }
    rt_device_open(pin_dev, RT_DEVICE_OFLAG_RDWR);
    pin_mode.pin = LED_PIN;
    pin_mode.mode = 0;

    rt_device_control(pin_dev, 0 ,(void*)&pin_mode);
    pin_status.pin = LED_PIN;

    while(1)
    {
        pin_status.status = 0;
        rt_device_write(pin_dev, 0 ,(void*)&pin_status, sizeof(pin_status));
        rt_thread_mdelay(200);
        pin_status.status = 1;
        rt_device_write(pin_dev, 0 ,(void*)&pin_status, sizeof(pin_status));
        rt_thread_mdelay(200);
    }
    rt_device_close(pin_dev);
    return 0;
}

