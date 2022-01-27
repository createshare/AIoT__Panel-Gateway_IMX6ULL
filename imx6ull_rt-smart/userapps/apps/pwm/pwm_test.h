/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-8      wxy      the first version
 */

#ifndef PWM_TEST_H__
#define PWM_TEST_H__
#include <stdio.h>
#include <stdlib.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>

#define PWM_CMD_ENABLE      (128 + 0)
#define PWM_CMD_DISABLE     (128 + 1)
#define PWM_CMD_SET         (128 + 2)
#define PWM_CMD_GET         (128 + 3)

struct rt_pwm_configuration
{
    rt_uint32_t channel;
    rt_uint32_t period;
    rt_uint32_t pulse;
};

struct rt_device_pwm;
struct rt_pwm_ops
{
    rt_err_t (*control)(struct rt_device_pwm *device, int cmd, void *arg);
};

struct rt_device_pwm
{
    struct rt_device parent;
    const struct rt_pwm_ops *ops;
};

#endif
