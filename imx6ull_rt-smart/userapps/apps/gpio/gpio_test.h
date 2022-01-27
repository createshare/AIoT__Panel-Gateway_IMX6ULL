/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change logs:
 * Date            Autor            Notes
 * 2021-08-08      WWL           The first version
 */

#ifndef GPIO_TEST_H__
#define GPIO_TEST_H__

#include <rtthread.h>
#include <rtdevice.h>

#define GET_PIN(PORTx, PIN)  (32 * (PORTx - 1) + (PIN & 31))

struct rt_device_pin_mode
{
    rt_uint16_t pin;
    rt_uint16_t mode;
};

struct rt_device_pin_status
{
    rt_uint16_t pin;
    rt_uint16_t status;
};

#endif
