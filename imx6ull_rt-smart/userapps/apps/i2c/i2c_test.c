/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-08     WWL     the first version
 */

#include <stdlib.h>
#include "i2c_test.h"

#define THREAD_PRIORITY   25
#define THREAD_STACK_SIZE 4096
#define THREAD_TIMESLICE  5

#define I2C_NAME "gt911"

static rt_device_t  dev = RT_NULL;
static struct rt_touch_data *read_data;
static struct rt_touch_info info;

static void gt911_touch_test(void)
{
    rt_device_control(dev, RT_TOUCH_CTRL_GET_INFO, &info);

    read_data = (struct rt_touch_data *)rt_malloc(sizeof(struct rt_touch_data) * info.point_num);

    while (1)
    {
        rt_device_control(dev, RT_TOUCH_CTRL_DISABLE_INT, RT_NULL);
        if (rt_device_read(dev, 0, read_data, info.point_num) == info.point_num)
        {
            for (rt_uint8_t i = 0; i < info.point_num; i++)
            {
                if (read_data[i].event == RT_TOUCH_EVENT_DOWN || read_data[i].event == RT_TOUCH_EVENT_MOVE)
                {
                    rt_kprintf("%d %d %d %d %d\n", read_data[i].track_id,
                               read_data[i].x_coordinate,
                               read_data[i].y_coordinate,
                               read_data[i].timestamp,
                               read_data[i].width);
                }
            }
        }
        rt_device_control(dev, RT_TOUCH_CTRL_ENABLE_INT, RT_NULL);
        rt_thread_mdelay(100);
    }
}

void Stop(int signo)
{
    rt_device_close(dev);
    exit(0);
}

/* Test function */
int main()
{
    void *id;
    rt_uint16_t x = 1000;
    rt_uint16_t y = 600;
    dev = rt_device_find(I2C_NAME);
    if (dev == RT_NULL)
    {
        rt_kprintf("can't find device:%s\n", I2C_NAME);
        return -1;
    }

    if (rt_device_open(dev, RT_DEVICE_FLAG_INT_RX) != RT_EOK)
    {
        rt_kprintf("open device failed!");
        return -1;
    }

    id = rt_malloc(sizeof(rt_uint8_t) * 8);
    rt_device_control(dev, RT_TOUCH_CTRL_GET_ID, id);
    rt_uint8_t * read_id = (rt_uint8_t *)id;
    rt_kprintf("id = GT%d%d%d \n", read_id[0] - '0', read_id[1] - '0', read_id[2] - '0');

    rt_device_control(dev, RT_TOUCH_CTRL_SET_X_RANGE, &x);  /* if possible you can set your x y coordinate*/
    rt_device_control(dev, RT_TOUCH_CTRL_SET_Y_RANGE, &y);
    rt_device_control(dev, RT_TOUCH_CTRL_GET_INFO, id);
    rt_kprintf("range_x = %d \n", (*(struct rt_touch_info*)id).range_x);
    rt_kprintf("range_y = %d \n", (*(struct rt_touch_info*)id).range_y);
    rt_kprintf("point_num = %d \n", (*(struct rt_touch_info*)id).point_num);
    signal(SIGINT, Stop);
    gt911_touch_test();
    return 0;
}
