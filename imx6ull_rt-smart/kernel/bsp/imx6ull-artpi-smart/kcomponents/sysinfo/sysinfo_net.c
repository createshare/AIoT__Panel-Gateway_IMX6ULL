/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2021-08-25     Weilin         The first version
 */
#include <rtthread.h>
#include <netdev_ipaddr.h>
#include <netdev.h>
#include "sysinfo.h"

#define RT_NET_MAX 20

struct sysinfo_net
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

int sysinfo_net_handle(void *shm)
{
    struct netdev *netdev = RT_NULL;
    int index;
    struct sysinfo_net sys_net;
    rt_uint16_t len;
    memset(&sys_net, 0, sizeof(struct sysinfo_net));
    netdev = netdev_get_first_by_flags(NETDEV_FLAG_LINK_UP);
    if (!netdev)
    {
        return -1;
    }
    sprintf(sys_net.name, "%s", netdev->name);
    sys_net.mtu = netdev->mtu;
    sys_net.mac_len = netdev->hwaddr_len;
    for (index = 0; index < netdev->hwaddr_len; index++)
    {
        sys_net.mac[index] = netdev->hwaddr[index];
    }
    sys_net.flags = netdev->flags;
    sprintf(sys_net.ip, "%s", inet_ntoa(netdev->ip_addr));
    sprintf(sys_net.gw, "%s", inet_ntoa(netdev->gw));
    sprintf(sys_net.netmask, "%s", inet_ntoa(netdev->netmask));
    sprintf(sys_net.dns_server, "%s", inet_ntoa(netdev->dns_servers[0]));
    len = sizeof(sys_net);
    memcpy(shm+TYPE_LEN, &len, LENGTH_LEN);
    memcpy(shm+TYPE_LEN+LENGTH_LEN, &sys_net, len);
    return 0;
}

