/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-08     WWL     the first version
 */

#ifndef I2C_TEST_H__
#define I2C_TEST_H__

#include <rtthread.h>
#include <rtdevice.h>

#define  RT_TOUCH_CTRL_GET_ID            (0)
#define  RT_TOUCH_CTRL_GET_INFO          (1)
#define  RT_TOUCH_CTRL_SET_X_RANGE       (3)
#define  RT_TOUCH_CTRL_SET_Y_RANGE       (4)

#define RT_TOUCH_EVENT_DOWN              (2)
#define RT_TOUCH_EVENT_MOVE              (3)

#define  RT_TOUCH_CTRL_DISABLE_INT       (6)
#define  RT_TOUCH_CTRL_ENABLE_INT        (7)


struct rt_touch_info
{
    rt_uint8_t     type;
    rt_uint8_t     vendor;
    rt_uint8_t     point_num;
    rt_int32_t     range_x;
    rt_int32_t     range_y;
};

struct rt_touch_data
{
    rt_uint8_t          event;
    rt_uint8_t          track_id;
    rt_uint8_t          width;
    rt_uint16_t         x_coordinate;
    rt_uint16_t         y_coordinate;
    rt_tick_t           timestamp;
};
#endif
