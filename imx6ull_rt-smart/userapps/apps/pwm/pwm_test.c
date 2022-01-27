/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2021-08-8      wxy        the first version
 */

#include "pwm_test.h"

int main(int argc, char **argv)
{
    int result = 0;
    int i = 0;
    struct rt_device_pwm *device = RT_NULL;
    device = (struct rt_device_pwm *)rt_device_find("pwm1");
    if (!device)
    {
        result = -RT_EIO;
        return -1;
    }
    struct rt_pwm_configuration configuration = {0};
    configuration.channel = 1;
    configuration.period = 1000000;
    configuration.pulse = 100000;

    /* modify pwm pulse In order to enhance the brightness of LCD */
    while(i <= 6)
    {
        result = rt_device_control(&device->parent, PWM_CMD_SET, &configuration);
        if (result)
        {
            printf("control failed , error is %d\n",result);
            break;
        }
        rt_thread_mdelay(500);
        rt_thread_mdelay(500);
        configuration.pulse += 100000;
        i++;
    }
    /* modify pwm pulse In order to reduce the brightness of LCD */
    while(i > 1)
    {
        result = rt_device_control(&device->parent, PWM_CMD_SET, &configuration);
        if (result)
        {
            printf("control failed , error is %d\n",result);
            break;
        }
        rt_thread_mdelay(500);
        rt_thread_mdelay(500);
        configuration.pulse -= 100000;
        i--;
    }
    return 0;
}
