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

#define DBG_TAG "DTAG"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static void rt_sysinfo_entry(void* paramter)
{
    struct rt_channel_msg msg_text;
    char* channel_msg;
    int shmid;
    /* create channel called 'sysinfo' to transmitting sys information */
    int ch = rt_channel_open("sysinfo", O_CREAT);
    if(ch == -1)
    {
        LOG_D("Error: rt_channel_open: fail to create the IPC channel for sysinfo!");
        return;
    }
    while (1)
    {
        rt_channel_recv(ch,&msg_text);
        shmid = (int)msg_text.u.d;
        if (shmid < 0 || !(channel_msg = lwp_shminfo(shmid)))
        {
            msg_text.u.d = (void*)-1;
            LOG_D("sysinfo:receive an invalid shared-memory page.");
            rt_channel_reply(ch,&msg_text);
            continue;
        }
        request_type = (int)channel_msg[0];
        switch(request_type)
        {
            case REQ_MEM:
            {
                sysinfo_mem_handle((void *)channel_msg);
                break;
            }
            case REQ_CPUUSAGE:
            {
                sysinfo_cpu_handle((void *)channel_msg);
                break;
            }
            case REQ_SYSNAME:
            {
                sysinfo_sysname_handle((void *)channel_msg);
                break;
            }
            case REQ_BUILDDATE:
            {
                sysinfo_date_handle((void *)channel_msg);
                break;
            }
            case REQ_GCCVERSION:
            {
                sysinfo_gcc_handle((void *)channel_msg);
                break;
            }
            case OPENLED:
            {
                sysinfo_led_control_handle(1);
                break;
            }
            case CLOSELED:
            {
                sysinfo_led_control_handle(0);
                break;
            }
            case REQ_LEDSTATUS:
            {
                sysinfo_led_status_handle((void *)channel_msg);
                break;
            }
            case REQ_DFS:
            {
                if (sysinfo_dfs_handle((void *)channel_msg))
                {
                    LOG_D("sysinfo: dfs handle failed!");
                    lwp_shmdt(channel_msg);
                    lwp_shmrm(shmid);
                    msg_text.u.d = (void*)-1;
                }
                break;
            }
            case REQ_NET:
            {
                if (sysinfo_net_handle((void *)channel_msg))
                {
                    LOG_D("sysinfo: net handle failed!");
                    lwp_shmdt(channel_msg);
                    lwp_shmrm(shmid);
                    msg_text.u.d = (void*)-1;
                }
                break;
            }
            default:
                LOG_D("sysinfo:no this case num!");
                lwp_shmdt(channel_msg);
                lwp_shmrm(shmid);
                msg_text.u.d = (void*)-1;
                break;
        }
        rt_channel_reply(ch,&msg_text);
    }
    rt_channel_close(ch);
}

int rt_sysinfo_init(void)
{
    rt_thread_t tid = rt_thread_create("sysinfo_thread", rt_sysinfo_entry, RT_NULL, LWP_TASK_STACK_SIZE, 25, 300);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_D("rt_sysinfo_init failed!");
    }
    return 0;
}
INIT_APP_EXPORT(rt_sysinfo_init);

