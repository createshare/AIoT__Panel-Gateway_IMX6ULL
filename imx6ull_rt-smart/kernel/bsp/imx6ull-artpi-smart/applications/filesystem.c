#include <rtthread.h>

#include <dfs_fs.h>
#include <ioremap.h>
#include <drv_sdio.h>

#define DBG_TAG "app.filesystem"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static void _sdcard_mount(void)
{
    rt_device_t device;

    device = rt_device_find("sd0");
    if (device == NULL)
    {
        mmcsd_wait_cd_changed(0);
        host_change();
        mmcsd_wait_cd_changed(RT_WAITING_FOREVER);
        rt_thread_mdelay(10);
        device = rt_device_find("sd0");
    }
    if (device != RT_NULL)
    {
        if (dfs_mount("sd0", "/mnt", "elm", 0, 0) == RT_EOK)
        {
            LOG_I("sd card mount to '/mnt'");
        }
        else
        {
            LOG_W("sd card mount to '/mnt' failed!");
        }
    }
}

static void _sdcard_unmount(void)
{
    rt_thread_mdelay(200);
    dfs_unmount("/mnt");
    LOG_I("Unmount \"/mnt\"");

    mmcsd_wait_cd_changed(0);
    host_change();
    mmcsd_wait_cd_changed(RT_WAITING_FOREVER);
}

static void sd_mount(void *parameter)
{
    int part_id = 1;
    volatile unsigned int *IN_STATUS;
    IN_STATUS = (volatile unsigned int *)rt_ioremap((void *)0x2190030, 4);
    rt_thread_mdelay(20);
	if (dfs_mount("sd0","/sd","elm",0,(void *)part_id) != 0)	
    {
        LOG_I("sd0 mount success");
    }
    else
    {
        LOG_W("sd0 mount failed!");
    }
    while (1)
    {
        rt_thread_mdelay(200);
        if (((*IN_STATUS >>6) & 0x1) == 1)
        {
            *IN_STATUS = 0x40;
            _sdcard_mount();
        }

        if (((*IN_STATUS >>7) & 0x1) == 1)
        {
            *IN_STATUS = (0x80);
            _sdcard_unmount();
        }
    }
}

int sd_task(void)
{

    rt_thread_t tid;
    tid = rt_thread_create("sd_mount", sd_mount, RT_NULL,
                           2048, RT_THREAD_PRIORITY_MAX - 2, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_E("create sd_mount thread err!");
    }
    return RT_EOK;
}

