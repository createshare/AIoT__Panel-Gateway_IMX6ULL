/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2021-08-25     songchao         The first version
 * 2021-08-30     songchao         The second version
 */

#include <rtthread.h>
#include <rtservice.h>
#include <string.h>
#include <stdio.h>

#include "sysinfo.h"

#define DBG_TAG    "CPUUSAGE"
#define DBG_LVL    DBG_INFO
#include <rtdbg.h>

#ifdef RT_USING_SMP
unsigned int g_switch_point_tick[RT_CPUS_NR] = {0};
rt_uint64_t g_int_duration_tick[RT_CPUS_NR] = {0};
static float g_cpu_usage[RT_CPUS_NR];
#else
unsigned int g_switch_point_tick = 0;
rt_uint64_t g_int_duration_tick = 0;
static float g_cpu_usage;
#endif

unsigned int g_cpu_usage_enable = 0;

static unsigned int g_cpu_usage_start_tick = 0;
static unsigned int g_cpu_usage_end_tick = 0;
static unsigned int show_cpu_usage_flag = 0;

#define LIST_FIND_OBJ_NR 8

typedef struct
{
    rt_list_t *list;
    rt_list_t **array;
    rt_uint8_t type;
    int nr;             /* input: max nr, can't be 0 */
    int nr_out;         /* out: got nr */
} list_get_next_t;

rt_inline void object_split(int len)
{
    while (len--) rt_kprintf("-");
}

static void list_find_init(list_get_next_t *p, rt_uint8_t type, rt_list_t **array, int nr)
{
    struct rt_object_information *info;
    rt_list_t *list;

    info = rt_object_get_information((enum rt_object_class_type)type);
    list = &info->object_list;

    p->list = list;
    p->type = type;
    p->array = array;
    p->nr = nr;
    p->nr_out = 0;
}

static rt_list_t *list_get_next(rt_list_t *current, list_get_next_t *arg)
{
    int first_flag = 0;
    rt_ubase_t level;
    rt_list_t *node, *list;
    rt_list_t **array;
    int nr;

    arg->nr_out = 0;

    if (!arg->nr || !arg->type)
    {
        return (rt_list_t *)RT_NULL;
    }

    list = arg->list;

    if (!current) /* find first */
    {
        node = list;
        first_flag = 1;
    }
    else
    {
        node = current;
    }

    level = rt_hw_interrupt_disable();

    if (!first_flag)
    {
        struct rt_object *obj;
        /* The node in the list? */
        obj = rt_list_entry(node, struct rt_object, list);
        if ((obj->type & ~RT_Object_Class_Static) != arg->type)
        {
            rt_hw_interrupt_enable(level);
            return (rt_list_t *)RT_NULL;
        }
    }

    nr = 0;
    array = arg->array;
    while (1)
    {
        node = node->next;

        if (node == list)
        {
            node = (rt_list_t *)RT_NULL;
            break;
        }
        nr++;
        *array++ = node;
        if (nr == arg->nr)
        {
            break;
        }
    }
    
    rt_hw_interrupt_enable(level);
    arg->nr_out = nr;
    return node;
}

static void cpu_usage_update_switch_and_duration_tick(struct rt_thread * tid)
{
    if (g_cpu_usage_enable)
    {
        unsigned int current_tick = CPU_USAGE_GET_TICK;

        tid->run_tick += (current_tick - SWITCH_POINT_TICK);
        SWITCH_POINT_TICK = current_tick;
    }
}

static void cpu_usage_set_start_tick(void)
{
    unsigned int current_tick = CPU_USAGE_GET_TICK;

    g_cpu_usage_start_tick = current_tick;

#ifdef RT_USING_SMP
    unsigned int i;

    for (i = 0; i < RT_CPUS_NR; i++)
    {
        g_switch_point_tick[i] = current_tick;
    }
#else
    g_switch_point_tick = current_tick;
#endif
}

static void cpu_usage_update_task_duration(void)
{
    rt_thread_t tid = NULL;
    unsigned int current_tick = CPU_USAGE_GET_TICK;

#ifdef RT_USING_SMP
    unsigned int i;

    for (i = 0; i < RT_CPUS_NR; i++)
    {
        tid = rt_cpu_index(i)->current_thread;
        if (tid == NULL)
            RT_ASSERT(0);

        tid->run_tick += (current_tick - g_switch_point_tick[i]);
        g_switch_point_tick[i] = current_tick;
    }
#else
    tid = rt_thread_self();
    if (tid == NULL)
        RT_ASSERT(0);

    tid->run_tick += (current_tick - g_switch_point_tick);
    g_switch_point_tick = current_tick;


#endif
}

void cpu_usage_start(void)
{
    rt_base_t level;
    level = rt_hw_interrupt_disable();

    if (!g_cpu_usage_enable)
    {
        /* enable timer */
        g_cpu_usage_enable = 1;
        cpu_usage_set_start_tick();
    }

    rt_hw_interrupt_enable(level);
}

void cpu_usage_stop(void)
{
    rt_base_t level;
    list_get_next_t find_arg;
    rt_list_t *next = (rt_list_t*)RT_NULL;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];

    level = rt_hw_interrupt_disable();

    /* disable timer */

    /* clear global var */
    g_cpu_usage_enable = 0;
    g_cpu_usage_start_tick = 0;
    g_cpu_usage_end_tick = 0;

#ifdef RT_USING_SMP
    {
        unsigned int i;

        for (i = 0; i < RT_CPUS_NR; i++)
        {
            g_int_duration_tick[i] = 0;
            g_switch_point_tick[i] = 0;
        }
    }
#else
    g_int_duration_tick = 0;
    g_switch_point_tick = 0;
#endif

    /* clear duration tick in thread TCB*/
    list_find_init(&find_arg, RT_Object_Class_Thread, obj_list, sizeof(obj_list)/sizeof(obj_list[0]));
    do
    {
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_thread *thread;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);

                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    continue;
                }

                thread = (struct rt_thread*)obj;

                thread->run_tick = 0;
            }
        }
    }
    while (next != (rt_list_t*)RT_NULL);

    rt_hw_interrupt_enable(level);
}

void cpu_usage_display(void)
{
    rt_uint64_t total_tick;
    rt_base_t level;
    list_get_next_t find_arg;
    rt_list_t *next = (rt_list_t*)RT_NULL;
    rt_list_t *obj_list[LIST_FIND_OBJ_NR];
    int maxlen = RT_NAME_MAX;
    const char *item_title = "THREAD";

    level = rt_hw_interrupt_disable();

    if (!g_cpu_usage_enable)
    {
        rt_hw_interrupt_enable(level);
        return;
    }
    list_find_init(&find_arg, RT_Object_Class_Thread, obj_list, sizeof(obj_list)/sizeof(obj_list[0]));

    g_cpu_usage_end_tick = CPU_USAGE_GET_TICK;
    total_tick = g_cpu_usage_end_tick - g_cpu_usage_start_tick;
    cpu_usage_update_task_duration();
    {
        rt_thread_t idle_handler = RT_NULL;

#ifdef RT_USING_SMP
        int i;
        char thread_name[32] = {0};

        for (i = 0; i < RT_CPUS_NR; i++)
        {
            snprintf(thread_name, sizeof(thread_name), "tidle%d", i);
            idle_handler = rt_thread_find(thread_name);

            g_cpu_usage[i] = 100.0f - ((float)idle_handler->run_tick * 100/(float)total_tick);

        }
#else
        idle_handler = rt_thread_find("tidle0");
        g_cpu_usage = 100.0f - ((float)idle_handler->run_tick * 100/(float)total_tick);      
#endif
    }
    if(show_cpu_usage_flag == 1)
    {
        object_split(maxlen);
        rt_kprintf(     "------------\n");
        rt_kprintf("%-*.s  PRI  %%CPU\n", maxlen, item_title); object_split(maxlen);
        rt_kprintf(     "  ---  ---- \n");
    }
    do
    {
        next = list_get_next(next, &find_arg);
        {
            int i;
            for (i = 0; i < find_arg.nr_out; i++)
            {
                struct rt_object *obj;
                struct rt_thread *thread;

                obj = rt_list_entry(obj_list[i], struct rt_object, list);

                if ((obj->type & ~RT_Object_Class_Static) != find_arg.type)
                {
                    continue;
                }

                thread = (struct rt_thread*)obj;
                {
                    float u;
                    char usage[32] = {0};
                    u = (float)thread->run_tick * 100/(float)total_tick;
                    if(show_cpu_usage_flag == 1)
                    {
                        rt_kprintf("%-*.*s  %3d ", maxlen, RT_NAME_MAX, thread->name, thread->current_priority);
                        snprintf(usage, sizeof(usage), " %3.2f", u);
                        rt_kprintf("%s\n", usage);
                    }

                }
            }
        }
    }while (next != (rt_list_t*)RT_NULL);
    
    rt_hw_interrupt_enable(level);
}

void cpu_usage_irq_enter_hook(void)
{
    rt_base_t level;

    level = rt_hw_interrupt_disable();

    if (g_cpu_usage_enable)
    {
        unsigned int current_tick = CPU_USAGE_GET_TICK;
        rt_thread_t tid = rt_thread_self();

        if (tid == NULL)
        {
            RT_ASSERT(0);
        }    

        /* when interrupt nesting (> 1), SWITCH_POINT_TICK is the tick of interrupt starting already */
        if (rt_interrupt_get_nest() == 1)
        {
            tid->run_tick += (current_tick - SWITCH_POINT_TICK);
            SWITCH_POINT_TICK = current_tick;
        }
    }

    rt_hw_interrupt_enable(level);
}

void cpu_usage_irq_exit_hook(void)
{
    rt_base_t level;

    level = rt_hw_interrupt_disable();

    if (g_cpu_usage_enable)
    {
        unsigned int current_tick = CPU_USAGE_GET_TICK;

        if (rt_interrupt_get_nest() == 1)
        {
            INT_DURATION_TICK += (current_tick - SWITCH_POINT_TICK);
            SWITCH_POINT_TICK = current_tick;
        }
    }

    rt_hw_interrupt_enable(level);
}

static int cpu_usage_init(void)
{
#ifdef RT_USING_HOOK
    rt_scheduler_switch_sethook(cpu_usage_update_switch_and_duration_tick);
    rt_interrupt_enter_sethook(cpu_usage_irq_enter_hook);
    rt_interrupt_leave_sethook(cpu_usage_irq_exit_hook);
#endif
    return 0;
}
INIT_BOARD_EXPORT(cpu_usage_init);

static int cpu_usage_thread_start_flag = 0;
static int cpu_usage_thread_run_count = 0;

static void cpu_usage_detect(void *param)
{
    //rt_base_t level;
    while (1)
    {
        cpu_usage_start();
        rt_thread_delay(1000);
        cpu_usage_display();
        cpu_usage_stop();
        cpu_usage_thread_run_count++;
    }
}

void sysinfo_cpu_handle(void* shm)
{
    rt_base_t level;
    rt_uint16_t cpu_len;
    rt_uint32_t cpu_usage;
    rt_uint32_t cpu_usage_low;

    level = rt_hw_interrupt_disable();
    cpu_usage_thread_run_count = 0;
    rt_hw_interrupt_enable(level);   

    if(cpu_usage_thread_start_flag == 0)
    {
        g_cpu_usage = 0;
        rt_thread_t cpu_usage_tid;
        cpu_usage_tid = rt_thread_create("cpu_usage",
                                cpu_usage_detect,
                                RT_NULL,
                                8192,
                                20,
                                2);
        if (cpu_usage_tid != RT_NULL)
        {
            rt_thread_startup(cpu_usage_tid);
            cpu_usage_thread_start_flag = 1;
        }
        else
        {
            LOG_E("create cpu_usage failed\n");
            return;
        }
        
        while(cpu_usage_thread_run_count == 0)
        {
            rt_thread_delay(100);
        }
    }

    level = rt_hw_interrupt_disable();
    cpu_len = sizeof(cpu_usage) + sizeof(cpu_usage_low);
    cpu_usage = (rt_uint32_t)g_cpu_usage;
    cpu_usage_low = (rt_uint32_t)((g_cpu_usage - cpu_usage) * 100);
    rt_hw_interrupt_enable(level);

    memcpy(shm+TYPE_LEN, (void *)(&cpu_len), sizeof(cpu_len));
    memcpy(shm+TYPE_LEN+LENGTH_LEN, (void *)(&cpu_usage), sizeof(cpu_usage));
    memcpy(shm+TYPE_LEN+LENGTH_LEN+sizeof(cpu_usage), (void *)(&cpu_usage_low), sizeof(cpu_usage_low));
}

static void cpu_usage(int argc, char** argv)
{
    if (argc != 2)
    {
        rt_kprintf("cpu_usage on ----> show cpu usage\n");
        rt_kprintf("cpu_usage off ----> close show cpu usage\n");
        return;
    }
    else
    {
        if (strncmp(argv[1] , "on", 2) == 0)
        {
            show_cpu_usage_flag = 1;
        }
        if (strncmp(argv[1] , "off", 3) == 0)
        {
            show_cpu_usage_flag = 0;
        }
    }
}

MSH_CMD_EXPORT_ALIAS(cpu_usage, cpu_usage,display cpu usage);
