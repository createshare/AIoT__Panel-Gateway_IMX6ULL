
/*
 *
 * (C) 2014 David Lettier.
 * (C) 2018 Armink (armink.ztl@gmail.com)
 *
 * http://www.lettier.com/
 *
 * NTP client.
 *
 * Compiled with gcc version 4.7.2 20121109 (Red Hat 4.7.2-8) (GCC).
 *
 * Tested on Linux 3.8.11-200.fc18.x86_64 #1 SMP Wed May 1 19:44:27 UTC 2013 x86_64 x86_64 x86_64 GNU/Linux.
 * Tested on RT-Thread 3.0.0+
 *
 * To compile: $ gcc main.c -o ntpClient.out
 *
 * Usage: $ ./ntpClient.out
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-11     linzhenxing   first version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <rtdef.h>

#include <rtthread.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>



#ifndef NETUTILS_NTP_TIMEZONE
#define NETUTILS_NTP_TIMEZONE                   8
#endif

#define NTP_TIMESTAMP_DELTA            2208988800ull

#define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6) /* (li   & 11 000 000) >> 6 */
#define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3) /* (vn   & 00 111 000) >> 3 */
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0) /* (mode & 00 000 111) >> 0 */

/* Structure that defines the 48 byte NTP packet protocol */
typedef struct {

    uint8_t li_vn_mode;      /* Eight bits. li, vn, and mode */
                             /* li.   Two bits.   Leap indicator */
                             /* vn.   Three bits. Version number of the protocol */
                             /* mode. Three bits. Client will pick mode 3 for client */

    uint8_t stratum;         /* Eight bits. Stratum level of the local clock */
    uint8_t poll;            /* Eight bits. Maximum interval between successive messages */
    uint8_t precision;       /* Eight bits. Precision of the local clock */

    uint32_t rootDelay;      /* 32 bits. Total round trip delay time */
    uint32_t rootDispersion; /* 32 bits. Max error aloud from primary clock source */
    uint32_t refId;          /* 32 bits. Reference clock identifier */

    uint32_t refTm_s;        /* 32 bits. Reference time-stamp seconds */
    uint32_t refTm_f;        /* 32 bits. Reference time-stamp fraction of a second */

    uint32_t origTm_s;       /* 32 bits. Originate time-stamp seconds */
    uint32_t origTm_f;       /* 32 bits. Originate time-stamp fraction of a second */

    uint32_t rxTm_s;         /* 32 bits. Received time-stamp seconds */
    uint32_t rxTm_f;         /* 32 bits. Received time-stamp fraction of a second */

    uint32_t txTm_s;         /* 32 bits and the most important field the client cares about. Transmit time-stamp seconds */
    uint32_t txTm_f;         /* 32 bits. Transmit time-stamp fraction of a second */

} ntp_packet;                /* Total: 384 bits or 48 bytes */

static ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static int sendto_ntp_server(int sockfd, const char *host_name, struct sockaddr_in *serv_addr)
{
    //struct hostent *server;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    /* NTP UDP port number. */
    int portno = 123;
#if 1
    char server_ip[] = "118.31.15.152";
    int l1;
#else
    server = gethostbyname(host_name);
    if (server == NULL)
    {
        printf("No such host(%s)", host_name);
        return -RT_ERROR;
    }
    else
#endif
    {
        /* Zero out the server address structure. */
        rt_memset((char *)serv_addr, 0, addr_len);

        serv_addr->sin_family = AF_INET;

        /* Convert the port number integer to network big-endian style and save it to the server address structure. */
        serv_addr->sin_port = htons(portno);
#if 1
        l1 = inet_addr(server_ip);
        /* Copy the server's IP address to the server address structure. */
        rt_memcpy(&serv_addr->sin_addr.s_addr, &l1, 4);
#else
        /* Copy the server's IP address to the server address structure. */
        rt_memcpy(&serv_addr->sin_addr.s_addr, (char *) server->h_addr, server->h_length);
#endif
        return sendto(sockfd, (char *) &packet, sizeof(ntp_packet), 0, (const struct sockaddr *)serv_addr, addr_len);
    }
}

/**
 * Get the UTC time from NTP server
 *
 * @param host_name NTP server host name, NULL: will using default host name
 *
 * @note this function is not reentrant
 *
 * @return >0: success, current GMT time
 *         =0: get failed
 */
time_t ntp_get_time(const char *host_name)
{
/* the delay(ms) between two receive */
#define RECV_TIME_DELAY_MS             10
/* NTP receive timeout(S) */
#define NTP_GET_TIMEOUT                5
/* number of NTP servers */
#define NTP_SERVER_NUM                 3

    int sockfd, n, i = 0, server_num = 0;
    struct sockaddr_in serv_addr[NTP_SERVER_NUM];
    rt_tick_t start = 0;
    time_t new_time = 0;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    //const char *const host_name_buf[NTP_SERVER_NUM] = {NTP_HOSTNAME1, NTP_HOSTNAME2, NTP_HOSTNAME3};

    /* Create and zero out the packet. All 48 bytes worth. */
    rt_memset(&packet, 0, sizeof(ntp_packet));

    /* Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.
       Represents 27 in base 10 or 00011011 in base 2. */
    *((char *) &packet + 0) = 0x1b;

    #define NTP_INTERNET           0x02
    #define NTP_INTERNET_BUFF_LEN  18
    #define NTP_INTERNET_MONTH_LEN 4
    #define NTP_INTERNET_DATE_LEN  16
    #ifndef SW_VER_NUM
    #define SW_VER_NUM             0x00000000
    #endif

    /* Create a UDP socket. */
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        rt_kprintf("Create socket failed");
        return 0;
    }

    if (host_name)
    {
        /* access the incoming host_name server */
        if (sendto_ntp_server(sockfd, host_name, serv_addr) >= 0)
        {
            server_num = 1;
        }
    }
    else
    {
        if (sendto_ntp_server(sockfd, host_name, serv_addr) >= 0)
        {
            server_num = 1;
        }

    }

    if (server_num <= 0)
    {
        closesocket(sockfd);
        return 0;
    }

    start = rt_tick_get();
    while (rt_tick_get() <= start + NTP_GET_TIMEOUT * 100)
    {
        for (i = 0; i < server_num; i++)
        {
            /* non-blocking receive the packet back from the server. If n == -1, it failed. */
            n = recvfrom(sockfd, (char *) &packet, sizeof(ntp_packet), MSG_DONTWAIT, (struct sockaddr *)&serv_addr[i], &addr_len);
            if (n <= 0)
            {
                //printf("Reading from server %s error (%d).", inet_ntoa(serv_addr[i].sin_addr.s_addr), n);
            }
            else if (n > 0)
            {
                goto __exit;
            }
        }

        rt_thread_mdelay(RECV_TIME_DELAY_MS);
    }

__exit:

    if (rt_tick_get() <= start + NTP_GET_TIMEOUT * 100)
    {
        /* These two fields contain the time-stamp seconds as the packet left the NTP server.
           The number of seconds correspond to the seconds passed since 1900.
           ntohl() converts the bit/byte order from the network's to host's "endianness". */
        packet.txTm_s = ntohl(packet.txTm_s); // Time-stamp seconds.
        packet.txTm_f = ntohl(packet.txTm_f); // Time-stamp fraction of a second.

        /* Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
           Subtract 70 years worth of seconds from the seconds since 1900.
           This leaves the seconds since the UNIX epoch of 1970.
           (1900)------------------(1970)**************************************(Time Packet Left the Server) */
        new_time = (time_t)(packet.txTm_s - NTP_TIMESTAMP_DELTA);
    }
    else
    {
        rt_kprintf("Receive the socket from server timeout (%dS).", NTP_GET_TIMEOUT);
    }

    closesocket(sockfd);

    return new_time;
}
/**
 * Set system date(time not modify).
 *
 * @param rt_uint32_t year  e.g: 2012.
 * @param rt_uint32_t month e.g: 12 (1~12).
 * @param rt_uint32_t day   e.g: 31.
 *
 * @return rt_err_t if set success, return RT_EOK.
 *
 */
rt_err_t set_date(rt_uint32_t year, rt_uint32_t month, rt_uint32_t day)
{
    time_t now;
    struct tm *p_tm;
    struct tm tm_new;
    rt_device_t device;
    rt_err_t ret = -RT_ERROR;

    /* get current time */
    now = time(RT_NULL);

    /* lock scheduler. */
    rt_enter_critical();
    /* converts calendar time time into local time. */
    p_tm = localtime(&now);
    /* copy the statically located variable */
    memcpy(&tm_new, p_tm, sizeof(struct tm));
    /* unlock scheduler. */
    rt_exit_critical();

    /* update date. */
    tm_new.tm_year = year - 1900;
    tm_new.tm_mon  = month - 1; /* tm_mon: 0~11 */
    tm_new.tm_mday = day;

    /* converts the local time in time to calendar time. */
    now = mktime(&tm_new);

    device = rt_device_find("rtc");
    if (device == RT_NULL)
    {
        return -RT_ERROR;
    }

    /* update to RTC device. */
    ret = rt_device_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &now);

    return ret;
}

/**
 * Set system time(date not modify).
 *
 * @param rt_uint32_t hour   e.g: 0~23.
 * @param rt_uint32_t minute e.g: 0~59.
 * @param rt_uint32_t second e.g: 0~59.
 *
 * @return rt_err_t if set success, return RT_EOK.
 *
 */
rt_err_t set_time(rt_uint32_t hour, rt_uint32_t minute, rt_uint32_t second)
{
    time_t now;
    struct tm *p_tm;
    struct tm tm_new;
    rt_device_t device;
    rt_err_t ret = -RT_ERROR;

    /* get current time */
    now = time(RT_NULL);

    /* lock scheduler. */
    rt_enter_critical();
    /* converts calendar time time into local time. */
    p_tm = localtime(&now);
    /* copy the statically located variable */
    memcpy(&tm_new, p_tm, sizeof(struct tm));
    /* unlock scheduler. */
    rt_exit_critical();

    /* update time. */
    tm_new.tm_hour = hour;
    tm_new.tm_min  = minute;
    tm_new.tm_sec  = second;

    /* converts the local time in time to calendar time. */
    now = mktime(&tm_new);

    device = rt_device_find("rtc");
    if (device == RT_NULL)
    {
        return -RT_ERROR;
    }

    /* update to RTC device. */
    ret = rt_device_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &now);

    return ret;
}
/**
 * Get the local time from NTP server
 *
 * @param host_name NTP server host name, NULL: will using default host name
 *
 * @return >0: success, current local time, offset timezone by NETUTILS_NTP_TIMEZONE
 *         =0: get failed
 */
time_t ntp_get_local_time(const char *host_name)
{
    time_t cur_time = ntp_get_time(host_name);

    if (cur_time)
    {
        /* add the timezone offset for set_time/set_date */
        cur_time += NETUTILS_NTP_TIMEZONE * 3600;
    }

    return cur_time;
}
/**
 * Sync current local time to RTC by NTP
 *
 * @param host_name NTP server host name, NULL: will using default host name
 *
 * @return >0: success
 *         =0: sync failed
 */
time_t ntp_sync_to_rtc(const char *host_name)
{
    time_t cur_time = ntp_get_local_time(host_name);

    if (cur_time)
    {
        struct tm *cur_tm;
        cur_tm = localtime(&cur_time);
        set_time(cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec);
        cur_tm = localtime(&cur_time);
        set_date(cur_tm->tm_year + 1900, cur_tm->tm_mon + 1, cur_tm->tm_mday);

        rt_kprintf("Get local time from NTP server: %s", ctime((const time_t *) &cur_time));

    }

    return cur_time;
}
static void *ntp_sync_thread_entry(void *param)
{
#define RTC_NTP_FIRST_SYNC_DELAY 5
#define RTC_NTP_SYNC_PERIOD 3600
    /* first sync delay for network connect */
    rt_thread_mdelay(RTC_NTP_FIRST_SYNC_DELAY * RT_TICK_PER_SECOND);

    while (1)
    {
        ntp_sync_to_rtc(NULL);
        rt_thread_mdelay(RTC_NTP_SYNC_PERIOD * RT_TICK_PER_SECOND);
    }
    return NULL;
}
int main(int argc, char **argv)
{
    pthread_t ssdp_pid1;
    if (pthread_create(&ssdp_pid1, NULL, ntp_sync_thread_entry, NULL) != 0)
    {
        rt_kprintf("Error rtc notify creating thread\n");
        return -1;
    }

    while(1)
    {
        sleep(3);
    }
    return 0;
}
