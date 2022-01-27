#include <stdio.h>
#include <string.h>
#include <lwp_shm.h>
#include <rtthread.h>
#include <rthw.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdev/netdev_ipaddr.h>
#include <netdev/netdev.h>

#define RT_NET_MAX 20
#define RT_SIZE_MAX 10
#define MAX_MSG_LEN 4096
#define TYPE_LEN 1
#define LENGTH_LEN 2
#define DATA_LEN 4
#define DFS_DATA_LEN 8

enum REQUEST_TYPE
{
    REQ_MEM=1,
    REQ_CPUUSAGE,
    REQ_SYSNAME,
    REQ_BUILDDATE,
    REQ_GCCVERSION,
    OPENLED,
    CLOSELED,
    REQ_LEDSTATUS,
    REQ_DFS,
    REQ_NET,
} request_type;


struct sys_mem
{
    char total_mem[RT_SIZE_MAX];
    char used_mem[RT_SIZE_MAX];
};

struct sys_dfs
{
    char total_size[RT_SIZE_MAX];
    char used_size[RT_SIZE_MAX];
};

struct sys_cpu
{
    rt_uint32_t cpu_usage;
    rt_uint32_t cpu_usage_low;
};

struct sys_net
{
    char name[RT_NAME_MAX];
    uint16_t mtu;
    uint8_t mac_len;
    uint8_t mac[NETDEV_HWADDR_MAX_LEN];
    uint16_t flags;
    char ip[RT_NET_MAX];
    char gw[RT_NET_MAX];
    char netmask[RT_NET_MAX];
    char dns_server[RT_NET_MAX];
};

rt_inline int prepare_data(void *data, size_t len)
{
    int shmid;
    void *shm_vaddr;

    /* use the current thread ID to label the shared memory */
    size_t key = (size_t) rt_thread_self();

    shmid = lwp_shmget(key, len, 1);    /* create a new shm */
    if (shmid == -1)
    {
        printf("Fail to allocate a shared memory!\n");
        return -1;
    }

    /* get the start address of the shared memory */
    shm_vaddr = lwp_shmat(shmid, NULL);
    if (shm_vaddr == RT_NULL)
    {
        printf("The allocated shared memory doesn't have a valid address!\n");
        lwp_shmrm(shmid);
        return -1;
    }

    /* put the data into the shared memory */
    memcpy(shm_vaddr, data, len);
    lwp_shmdt(shm_vaddr);

    return shmid;
}


int prepare_req(int ch, int type, struct rt_channel_msg *ch_msg, struct rt_channel_msg *ch_msg_ret)
{
    char msg[MAX_MSG_LEN];
    int prepare_num = 10;
    memset(&msg,0,MAX_MSG_LEN);
    /* initialize the message to send */
    ch_msg->type = RT_CHANNEL_RAW;
    msg[0] = type;
    int shmid = prepare_data(msg, MAX_MSG_LEN);
    while(shmid < 0 && prepare_num)
    {
        shmid = prepare_data(msg, MAX_MSG_LEN);
    }
    if (prepare_num == 0)
    {
        printf("fail to prepare the req message.\n");
        return -1;
    }
    ch_msg->u.d = (void *)shmid;
    rt_channel_send_recv(ch, ch_msg, ch_msg_ret);
    return 0;
}

void size_trans(unsigned long long size, char* size_str)
{
    char *unit_str[] = {"KB", "MB", "GB"};
    int unit_index;
    int minor = 0;
    for (unit_index = 0; unit_index < 2; unit_index++)
    {
        if(size < 1024)
        {
            break;
        }

        minor = (size % 1024) * 10 / 1024;
        size = size / 1024;
    }
    sprintf(size_str, "%d.%d%s",(int)size, minor , unit_str[unit_index]);
}

int ret_check(int ret_shmid, void* buffer)
{
    char* ret_msg = NULL;
    rt_uint16_t len;
    int ret_type;
    if (ret_shmid < 0 || !(ret_msg = (char*)lwp_shmat(ret_shmid, NULL)))
    {
        printf("get mem information failed!\n");
        return -1;
    }

    ret_type = ret_msg[0];
    switch(ret_type)
    {
    case REQ_MEM:
    {
        memcpy(&len, ret_msg+TYPE_LEN, LENGTH_LEN);
        if(len == DATA_LEN*3)
        {
            /*get mem informations from ret msg */
            rt_uint32_t total_mem,used_mem,max_mem;
            memcpy(&total_mem, ret_msg+TYPE_LEN+LENGTH_LEN, DATA_LEN);
            memcpy(&used_mem, ret_msg+TYPE_LEN+LENGTH_LEN+DATA_LEN, DATA_LEN);
            memcpy(&max_mem, ret_msg+TYPE_LEN+LENGTH_LEN+DATA_LEN*2, DATA_LEN);
            struct sys_mem *mem_struct = (struct sys_mem *)buffer;
            memset(mem_struct,0,sizeof(struct sys_mem));
            size_trans(total_mem/1024UL,mem_struct->total_mem);
            size_trans(used_mem/1024UL,mem_struct->used_mem);
        }
        else
        {
            printf("REQ_MEM: the length is error!\n");
            goto err;
        }
        break;
    }
    case REQ_CPUUSAGE:
    {
        memcpy(&len, ret_msg+TYPE_LEN, LENGTH_LEN);
        if(len == DATA_LEN*2)
        {
            struct sys_cpu *tmp_cpu = (struct sys_cpu *)buffer;
            memcpy(&(tmp_cpu->cpu_usage), ret_msg+TYPE_LEN+LENGTH_LEN, DATA_LEN);
            memcpy(&(tmp_cpu->cpu_usage_low), ret_msg+TYPE_LEN+LENGTH_LEN+DATA_LEN, DATA_LEN);
        }
        else
        {
            printf("REQ_CPUUSAGE is error!\n");
            goto err;
        }
        break;
    }
    case REQ_SYSNAME:
    {
        memcpy(&len, ret_msg+TYPE_LEN, LENGTH_LEN);
        if(len == DATA_LEN*3)
        {
            rt_uint32_t version,subversion,revision;
            memcpy(&version, ret_msg+TYPE_LEN+LENGTH_LEN, DATA_LEN);
            memcpy(&subversion, ret_msg+TYPE_LEN+LENGTH_LEN+DATA_LEN, DATA_LEN);
            memcpy(&revision, ret_msg+TYPE_LEN+LENGTH_LEN+DATA_LEN*2, DATA_LEN);
            memset(buffer, 0, MAX_MSG_LEN);
            sprintf(buffer,"Thread Smart Operating System %u.%u.%u",version,subversion,revision);
        }
        else
        {
            printf("REQ_SYSNAME: the length is error!\n");
            goto err;
        }
        break;
    }
    case REQ_BUILDDATE:
    {
        memcpy(&len, ret_msg+TYPE_LEN, LENGTH_LEN);
        memset(buffer, 0, MAX_MSG_LEN);
        memcpy(buffer, ret_msg+TYPE_LEN+LENGTH_LEN, len);
        break;
    }
    case REQ_GCCVERSION:
    {
        memcpy(&len, ret_msg+TYPE_LEN, LENGTH_LEN);
        memset(buffer, 0, MAX_MSG_LEN);
        memcpy(buffer, ret_msg+TYPE_LEN+LENGTH_LEN, len);
        break;
    }
    case OPENLED:
    {
        break;
    }
    case CLOSELED:
    {
        break;
    }
    case REQ_LEDSTATUS:
    {
        memcpy(&len, ret_msg+TYPE_LEN, LENGTH_LEN);
        if(len == DATA_LEN)
        {
            int led_status;
            memcpy(&led_status, ret_msg+TYPE_LEN+LENGTH_LEN, DATA_LEN);
            if(led_status)
            {
                lwp_shmdt(ret_msg);
                lwp_shmrm(ret_shmid);
                return 0;
            }
            else
            {
                lwp_shmdt(ret_msg);
                lwp_shmrm(ret_shmid);
                return 1;
            }
        }
        else
        {
            printf("led status length is error!\n");
            goto err;
        }
        break;
    }
    case REQ_DFS:
    {
        memcpy(&len, ret_msg+TYPE_LEN, LENGTH_LEN);
        if(len == DFS_DATA_LEN*3)
        {
            unsigned long long all_size, free_size, used_size;
            memcpy(&all_size, ret_msg+TYPE_LEN+LENGTH_LEN, DFS_DATA_LEN);
            memcpy(&free_size, ret_msg+TYPE_LEN+LENGTH_LEN+DFS_DATA_LEN, DFS_DATA_LEN);
            memcpy(&used_size, ret_msg+TYPE_LEN+LENGTH_LEN+DFS_DATA_LEN*2, DFS_DATA_LEN);
            struct sys_dfs *tmp_dfs = (struct sys_dfs*)buffer;
            memset(tmp_dfs, 0, sizeof(struct sys_dfs));
            size_trans(all_size, tmp_dfs->total_size);
            size_trans(used_size, tmp_dfs->used_size);
        }
        else
        {
            printf("dfs length is error!\n");
            goto err;
        }
        break;
    }
    case REQ_NET:
    {
        memcpy(&len, ret_msg+TYPE_LEN, LENGTH_LEN);
        memcpy(buffer, ret_msg+TYPE_LEN+LENGTH_LEN, len);
        break;
    }
    default:
        printf("this case %u is not my req!\n",ret_msg[0]);
        goto err;
    }
    lwp_shmdt(ret_msg);
    lwp_shmrm(ret_shmid);
    return 0;
err:
    lwp_shmdt(ret_msg);
    lwp_shmrm(ret_shmid);
    return -1;
}


int case_check(int ch, int type, void *buffer)
{ 
    struct rt_channel_msg ch_msg, ch_msg_ret;
    int ret_shmid;
    if (prepare_req(ch, type, &ch_msg, &ch_msg_ret) < 0)
    {
        return -1;
    }
    ret_shmid = (int) ch_msg_ret.u.d;
    return(ret_check(ret_shmid, buffer));
}



int main()
{
    int ch;
    int ret;
    int i = 0;
    struct sys_mem mem_buffer;
    char sys_name_str[MAX_MSG_LEN];
    char build_date[MAX_MSG_LEN];
    char gcc_version[MAX_MSG_LEN];
    struct sys_dfs dfs_buffer;
    struct sys_cpu cpu_buffer;
    struct sys_net net_buffer;
    /* open the IPC channel created by kernel */
    ch = rt_channel_open("sysinfo", 0);
    char *data;
    char cc;
    char restr[256];
    printf("Content-Type:text/html\n\n");

    if (ch == -1)
    {
        printf("Error: rt_channel_open: could not find the \'sysinfo\' channel!\n");
        return -1;
    }
 
    memset(restr, 0, sizeof(restr));
    data = getenv("QUERY_STRING");
    sscanf(data,"type=%c", &cc);
    switch(cc)
    {
    case 'm':  /* memory and space size*/
        ret = case_check(ch, REQ_MEM, (void *)&mem_buffer);
        if (ret < 0)
        {
            printf("case REQ_MEM failed!\n");
            return -1;
        }
        strcat(restr, mem_buffer.used_mem);
        strcat(restr, "&");
        strcat(restr, mem_buffer.total_mem);
        strcat(restr, ";");

        ret = case_check(ch, REQ_DFS, (void *)&dfs_buffer);
        if (ret < 0)
        {
            printf("case REQ_DFS failed!\n");
            return -1;
        }
        strcat(restr, dfs_buffer.used_size);
        strcat(restr, "&");
        strcat(restr, dfs_buffer.total_size);
        strcat(restr, ";");

        ret = case_check(ch, REQ_CPUUSAGE, (void *)&cpu_buffer);
        if (ret < 0)
        {
            printf("case REQ_CPUUSAGE failed!\n");
            return -1;
        }
        sprintf(restr+strlen(restr),"%u.%u", cpu_buffer.cpu_usage, cpu_buffer.cpu_usage_low);
        strcat(restr, ";");

        ret = case_check(ch, REQ_NET, (void *)&net_buffer);
        if (ret < 0)
        {
            printf("case REQ_NET failed!\n");
            return -1;
        }
        if (net_buffer.flags & NETDEV_FLAG_INTERNET_UP)
        {
            strcat(restr, "up");
        }
        else
        {
            strcat(restr, "down");
        }
        printf("%s", restr);
        break;
    case 'i':  /* sysinfo */
        /* system version */
        ret = case_check(ch, REQ_SYSNAME, (void *)sys_name_str);
        if (ret < 0)
        {
            printf("case REQ_SYSNAME failed!\n");
            return -1;
        }
        strcat(restr, sys_name_str);
        strcat(restr, "&");
        /* build date */
        ret = case_check(ch, REQ_BUILDDATE, (void *)build_date);
        if (ret < 0)
        {
            printf("case REQ_BUILDDATE failed!\n");
            return -1;
        }
        strcat(restr, build_date);
        strcat(restr, "&");
        /* gcc version */
        ret = case_check(ch, REQ_GCCVERSION, (void *)gcc_version);
        if (ret < 0)
        {
            printf("case REQ_GCCVERSION failed!\n");
            return -1;
        }
        strcat(restr, gcc_version);	
        strcat(restr, "&");
        /*ifconfig data*/
        ret = case_check(ch, REQ_NET, (void *)&net_buffer);
        if (ret < 0)
        {
            printf("case REQ_NET failed!\n");
            return -1;
        }
        strcat(restr,net_buffer.name);
        strcat(restr, "&");
        for(i=0; i<net_buffer.mac_len; i++)
        {
            sprintf(restr+strlen(restr),"%02x",net_buffer.mac[i]);
            strcat(restr," ");
        }
        strcat(restr, "&");
        strcat(restr,net_buffer.ip);
        strcat(restr, "&");
        strcat(restr,net_buffer.gw);
        strcat(restr, "&");
        strcat(restr,net_buffer.netmask);
        strcat(restr, "&");
        strcat(restr,net_buffer.dns_server);
        printf("%s", restr);
        break;
    case '0':  /* led off */
        case_check(ch, CLOSELED, NULL);
        ret = case_check(ch, REQ_LEDSTATUS, NULL);
        if (ret < 0)
        {
            printf("case REQ_LEDSTATUS failed!\n");
            return -1;
        }
        if (ret)
        {
            printf("on");
        }
        else
        {
            printf("off");
        }
        break;
    case '1':  /* led on */
        case_check(ch, OPENLED, NULL);
        ret = case_check(ch, REQ_LEDSTATUS, NULL);
        if (ret < 0)
        {
            printf("case REQ_LEDSTATUS failed!\n");
            return -1;
        }
        if (ret)
        {
            printf("on");
        }
        else
        {
            printf("off");
        }
        break;
    case '2':  /* get led status */
        ret = case_check(ch, REQ_LEDSTATUS, NULL);
        if (ret < 0)
        {
            printf("case REQ_LEDSTATUS failed!\n");
            return -1;
        }
        if (ret)
        {
            printf("on");
        }
        else
        {
            printf("off");
        }
        break;
    default:
        break;
    }
    rt_channel_close(ch);
    return 0;
}
