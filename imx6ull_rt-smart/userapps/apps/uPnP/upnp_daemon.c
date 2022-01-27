#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <rtthread.h>
#include <pthread.h>
#include <dfs_posix.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <malloc.h>
#include <sys/types.h>
#include <netdb.h>
#include "upnp_daemon.h"

#define SSDP_DEBUG
#ifdef  SSDP_DEBUG
#define DEBUG_PRINTF        rt_kprintf("[SSDP] "); rt_kprintf
#else
#define DEBUG_PRINTF(...)
#endif /* SSDP_DEBUG */

#define SSDP_NOTIFY_SECOND  5
#define SSDP_BUFSZ          1024

#define DEVICE_NAME "e1"

static char ssdp_bufffer[1460];
static char ssdp_response[512];
static int g_sock = -1;
static char IP_ADDR_DST[] = "192.168.xxx.xxx";  
static char IP_HEX_ADDR_DST[] = "C0A80C16";
static char MAC_ADDR_DST[] = "A85E45919293";  
static char MODULE_NAME_DST[] = "art-pi-smart-xxx";

static char IP_ADDR_SRC[] = "xxx.xxx.xxx.xxx";  
static char IP_HEX_ADDR_SRC[] = "xxxxxxxx";
static char MAC_ADDR_SRC[] = "aaaaaaaaaaaa";  
static char MODULE_NAME_SRC[] = "art-pi-smart-xxx";

char * get_uuid(const char *name)
{
    static char uuid_buffer[100];
    struct ifreq ifr;
    struct sockaddr_in *addr;
    char*address;
    uint32_t ip_addr_32;
    u_int8_t hd[6];

    if( strlen(name) >= IFNAMSIZ)
    {
        printf("device name is error.\n");
        return NULL;
    }
    strcpy( ifr.ifr_name, name);
 
    //get inet addr
    if( ioctl(g_sock, SIOCGIFADDR, &ifr) != 0)
    {
        printf("SIOCGIFADDR ioctl error.\n");
        return NULL;
    }
    addr = (struct sockaddr_in *)&(ifr.ifr_addr);

    ip_addr_32 = addr->sin_addr.s_addr;
    address = (char *)&ip_addr_32;
    
    //get HWaddr
    if(ioctl(g_sock, SIOCGIFHWADDR, &ifr) != 0)
    {
        printf("SIOCGIFHWADDR ioctl error.\n");
        return NULL;
    }
    memcpy( hd, ifr.ifr_hwaddr.sa_data, sizeof(hd));

    sprintf(uuid_buffer, "%02X%02X%02X%02X-1234-1234-1234-%02X%02X%02X%02X%02X%02X"
    ,address[0],address[1],address[2],address[3],hd[0], hd[1], hd[2], hd[3], hd[4], hd[5]);

    sprintf(IP_HEX_ADDR_DST, "%02X%02X%02X%02X",address[0],address[1],address[2],address[3]);    
    sprintf(MAC_ADDR_DST, "%02X%02X%02X%02X%02X%02X",hd[0], hd[1], hd[2], hd[3], hd[4], hd[5]);

    return uuid_buffer;
}

char * get_ip_addr(const char *name)
{
    static char ip_buffer[16];
    struct ifreq ifr;
    struct sockaddr_in *addr;
    char *address;
    char *addr_hex;
    if( strlen(name) >= IFNAMSIZ)
    {
        printf("device name is error.\n");
        return NULL;
    }
    strcpy( ifr.ifr_name, name);

    if( ioctl(g_sock, SIOCGIFADDR, &ifr) == -1)
    {
        printf("SIOCGIFADDR ioctl error.\n");
        return NULL;
    }
    
    addr = (struct sockaddr_in *)&(ifr.ifr_addr);
    address = inet_ntoa(addr->sin_addr);  
    addr_hex = (char *)(&addr->sin_addr);  
    sprintf(ip_buffer,address);
    sprintf(IP_ADDR_DST,address);
    sprintf(MODULE_NAME_DST,"art-pi-smart-%d",addr_hex[3]);
    return ip_buffer;
}

void *ssdp_thread_entry(void *parameter)
{
    int sock;
    int bytes;
    int dmr = 0, rootdevice = 0;
    rt_uint32_t addr_len;
    struct sockaddr_in server_addr, client_addr;
    ssdp_netbios_request_t request;
    char *request_buffer;
    char *ch;
    /* create a UDP socket */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        DEBUG_PRINTF("create socket error\n");
        return NULL;
    }

    {
        int optval;
        int res;

        // set SO_REUSEADDR on a socket to true (1):
        optval = 1;

        res = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
        res = res;
    }

    /* initialize server address */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SSDP_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    rt_memset(&(server_addr.sin_zero),0, sizeof(server_addr.sin_zero));

    /* bind socket to server address */
    if (bind(sock,(struct sockaddr *)&server_addr,
             sizeof(struct sockaddr)) == -1)
    {
        DEBUG_PRINTF("Bind error\r\n");
        return NULL;
    }
    DEBUG_PRINTF("UPnP SSDP server listen on port %d...\r\n", SSDP_UDP_PORT);

    addr_len = sizeof(struct sockaddr);

    while(1)
    {
        dmr = 0; rootdevice = 0;
        bytes = recvfrom(sock, ssdp_bufffer, sizeof(ssdp_bufffer)-1, 0,
                         (struct sockaddr *)&client_addr, &addr_len);

        memset(&request, 0, sizeof(request));
        ssdp_bufffer[bytes] = '\0';

        request_buffer = ssdp_bufffer;
        while (*request_buffer)
        {
            if( strncasecmp(request_buffer,"M-SEARCH", sizeof("M-SEARCH")-1) == 0 )
            {
                request.head = request_buffer;
            }
            else if( strncasecmp(request_buffer,"Host:", sizeof("Host:")-1) == 0 )
            {
                request.Host = request_buffer;
            }
            else if( strncasecmp(request_buffer,"ST:", sizeof("ST:")-1) == 0 )
            {
                request.ST = request_buffer;
            }
            else if( strncasecmp(request_buffer,"Man:", sizeof("Man:")-1) == 0 )
            {
                request.Man = request_buffer;
            }
            else if( strncasecmp(request_buffer,"MX:", sizeof("MX:")-1) == 0 )
            {
                request.MX = request_buffer;
            }
            else if( strncasecmp(request_buffer,"\r\n", sizeof("\r\n")-1) == 0 )
            {
                break;
            }

            ch = strstr(request_buffer, "\r\n");

            if (ch == RT_NULL)
            {
                break;
            }
            *ch ++ = '\0';
            *ch ++ = '\0';
            request_buffer = ch;
        }
        if (request.ST == RT_NULL) continue;
        /* rootdevice   */
        {
            char *man_ptr, *st_ptr;

            man_ptr = request.Man;
            man_ptr += 4;
            while (*man_ptr == ' ') man_ptr ++;

            st_ptr = request.ST;
            st_ptr += 3;
            while (*st_ptr == ' ') st_ptr ++;

            // ST: upnp:rootdevice
            if ((strncasecmp(man_ptr, "\"ssdp:discover\"", sizeof("\"ssdp:discover\"")-1) == 0) &&
                (strncasecmp(st_ptr,  "upnp:rootdevice",  sizeof("upnp:rootdevice")-1) == 0))
                rootdevice = 1;

            // ST: all
            if ((strncasecmp(man_ptr, "\"ssdp:discover\"", sizeof("\"ssdp:discover\"")-1) == 0) &&
                (strncasecmp(st_ptr,  "ssdp:all",  sizeof("ssdp:all")-1) == 0))
                rootdevice = 1;
        }
        if ( rootdevice ) //ST:upnp:rootdevice
        {
            uint32_t len;
            uint32_t offset =0;

            DEBUG_PRINTF("\r\n\r\nSSDP SEND Response:\r\n");

            len = sprintf(&ssdp_response[offset], "HTTP/1.1 200 OK\r\n");
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

            len = sprintf(&ssdp_response[offset], "CACHE-CONTROL: max-age=60\r\n");
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

            len = sprintf(&ssdp_response[offset], "EXT:\r\n");
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

            len = sprintf(&ssdp_response[offset], "%s\r\n", request.ST);
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

#ifdef  RT_USING_WEBNET
            //LOCATION: http://192.168.1.1:80/descriptor.xml\r\n
            len = sprintf(&ssdp_response[offset],
                          "LOCATION: http://%s/MediaRenderer/descriptor.xml\r\n",
                          get_ipaddr(RT_NULL));
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;
#endif // RT_USING_WEBNET

            len = sprintf(&ssdp_response[offset], "SERVER: RT-Thread/%ld.%ld.%ld,UPnP/1.0\r\n", RT_VERSION, RT_SUBVERSION, RT_REVISION);
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

            /* generate product SN */
            len = sprintf(&ssdp_response[offset],
                          "USN:uuid:%s::upnp:rootdevice\r\n",
                          get_uuid(DEVICE_NAME));
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

            /* end of response */
            len = sprintf(&ssdp_response[offset], "\r\n");
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

            bytes = sendto(sock, ssdp_response, offset,
                           0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        }
        /* MediaRenderer */
        {
            char *man_ptr, *st_ptr;

            man_ptr = request.Man;
            man_ptr += 4;
            while (*man_ptr == ' ') man_ptr ++;

            st_ptr = request.ST;
            st_ptr += 3;
            while (*st_ptr == ' ') st_ptr ++;

            // device:MediaRenderer
            if( (strncasecmp(man_ptr, "\"ssdp:discover\"", sizeof("\"ssdp:discover\"")-1) == 0) &&
                (strncasecmp(st_ptr,  "urn:schemas-upnp-org:device:MediaRenderer:1",  sizeof("urn:schemas-upnp-org:device:MediaRenderer:1")-1) == 0) )
                    dmr = 1;
        }

        if (dmr)
        {
            uint32_t len;
            uint32_t offset =0;
            DEBUG_PRINTF("\r\n\r\nSSDP SEND Response:\r\n");

            len = sprintf(&ssdp_response[offset], "HTTP/1.1 200 OK\r\n");
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

            len = sprintf(&ssdp_response[offset], "CACHE-CONTROL: max-age=60\r\n");
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

            len = sprintf(&ssdp_response[offset], "EXT:\r\n");
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

            len = sprintf(&ssdp_response[offset], "%s\r\n", request.ST);
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

#ifdef  RT_USING_WEBNET
            //LOCATION: http://192.168.1.1:80/descriptor.xml\r\n
            len = sprintf(&ssdp_response[offset],
                          "LOCATION: http://%s/MediaRenderer/descriptor.xml\r\n",
                          get_ipaddr(RT_NULL));
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;
#endif // RT_USING_WEBNET

            len = sprintf(&ssdp_response[offset], "SERVER: RT-Thread/%ld.%ld.%ld,UPnP/1.0\r\n", RT_VERSION,
                RT_SUBVERSION, RT_REVISION);
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

            /* generate product SN */
            len = sprintf(&ssdp_response[offset],
                          "USN:uuid:%s::urn:schemas-upnp-org:service:MediaRenderer:1\r\n",
                          get_uuid(DEVICE_NAME));
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

            /* end of response */
            len = sprintf(&ssdp_response[offset], "\r\n");
            DEBUG_PRINTF("%s", &ssdp_response[offset]);
            offset += len;

            bytes = sendto(sock, ssdp_response, offset,
                           0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        }

    }
}

#define MAXSIZES 4096
void Substitute(char *pInput, char *pOutput, char *pSrc, char *pDst)  
{  
  char    *pi, *po, *p;  
  int     nSrcLen, nDstLen, nLen;  
  
  // 指向输入字符串的游动指针.  
  pi = pInput;      
  // 指向输出字符串的游动指针.  
  po = pOutput;  
  // 计算被替换串和替换串的长度.  
  nSrcLen = strlen(pSrc);  
  nDstLen = strlen(pDst);  
  
  // 查找pi指向字符串中第一次出现替换串的位置,并返回指针(找不到则返回null).     
  p = strstr(pi, pSrc);  
  if(p)  
  {  
          // 找到.  
      while(p)  
      {  
          //计算被替换串前边字符串的长度.  
          nLen = (int)(p - pi);  
          // 复制到输出字符串.  
          memcpy(po, pi, nLen);  
          memcpy(po + nLen, pDst, nDstLen);  
          // 跳过被替换串.  
          pi = p + nSrcLen;  
          // 调整指向输出串的指针位置.  
          po = po + nLen + nDstLen;  
          // 继续查找.  
          p = strstr(pi, pSrc);  
      }  
      // 复制剩余字符串.  
      strcpy(po, pi);  
  }  
  else  
  {  
      // 没有找到则原样复制.  
      strcpy(po, pi);  
  }  
} 
int create_decriptor_xml()  
{     
    FILE *fpSrc,*fpDes;  
    char filename1[64] = "/root/mould.xml";  
    char filename2[64] = "/root/descriptor.xml";  
    //要求查找的字符串，替换的字符串；  

    char ps[32];
    char pd[32];
    
    char *Src_buf = NULL;  
    char *Des_buf = NULL;  
    char *Temp_buf = NULL; 

    if((fpSrc=fopen(filename1,"r+"))==NULL)  
    {  
        printf("fail to open the file1 !\n");  
        return -1;   
    }  
    if((fpDes=fopen(filename2,"w+"))==NULL)  
    {  
        printf("fail to open the file2 !\n");  
        return -1;  
    }

    Src_buf = malloc(MAXSIZES);  
    Des_buf = malloc(MAXSIZES);  
    Temp_buf = malloc(MAXSIZES); 

    fread(Src_buf,MAXSIZES,1,fpSrc);
    memcpy(Temp_buf,Src_buf,MAXSIZES);

    memset(ps,0,sizeof(MODULE_NAME_SRC));
    memcpy(ps,MODULE_NAME_SRC,sizeof(MODULE_NAME_SRC));
    memset(pd,0,sizeof(MODULE_NAME_DST));
    memcpy(pd,MODULE_NAME_DST,sizeof(MODULE_NAME_DST));
    Substitute(Temp_buf,Des_buf,ps,pd);  
    memcpy(Temp_buf,Des_buf,MAXSIZES);

    memset(ps,0,sizeof(IP_ADDR_SRC));
    memcpy(ps,IP_ADDR_SRC,sizeof(IP_ADDR_SRC));
    memset(pd,0,sizeof(IP_ADDR_DST));
    memcpy(pd,IP_ADDR_DST,sizeof(IP_ADDR_DST));
    Substitute(Temp_buf,Des_buf,ps,pd);  
    memcpy(Temp_buf,Des_buf,MAXSIZES);

    memset(ps,0,sizeof(IP_HEX_ADDR_SRC));
    memcpy(ps,IP_HEX_ADDR_SRC,sizeof(IP_HEX_ADDR_SRC));
    memset(pd,0,sizeof(IP_HEX_ADDR_DST));
    memcpy(pd,IP_HEX_ADDR_DST,sizeof(IP_HEX_ADDR_DST));
    Substitute(Temp_buf,Des_buf,ps,pd);  
    memcpy(Temp_buf,Des_buf,MAXSIZES);
 
    memset(ps,0,sizeof(MAC_ADDR_SRC));
    memcpy(ps,MAC_ADDR_SRC,sizeof(MAC_ADDR_SRC));
    memset(pd,0,sizeof(MAC_ADDR_DST));
    memcpy(pd,MAC_ADDR_DST,sizeof(MAC_ADDR_DST));
    Substitute(Temp_buf,Des_buf,ps,pd);  

    fwrite(Des_buf,strlen(Des_buf),1,fpDes);

    fclose(fpSrc);  
    fclose(fpDes);  

    free(Src_buf);  
    free(Des_buf);  
    free(Temp_buf); 

    return 0;  
} 

int main(int argc,char **argv)
{
    int ret = -1;
    char usn[128];
    char location[64];
    uint8_t *buf = NULL;
    struct sockaddr_in server_addr;
    char *agent = "rt-thread";
    char *nt = "upnp:rootdevice";
    char *usn_fmt = "uuid:%s::upnp:rootdevice";
    char *ip_addr = NULL;
    char *uuip = NULL;
    int length;
    int get_vaild_ip_flag = 0; 

    buf = (uint8_t*) malloc (SSDP_BUFSZ);
    RT_ASSERT(buf != NULL);
    g_sock = socket(AF_INET, SOCK_DGRAM, 0);
    RT_ASSERT(g_sock >= 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SSDP_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SSDP_ADDR);
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    while (1)
    {
        while(!get_vaild_ip_flag)
        {
            ip_addr = get_ip_addr(DEVICE_NAME);
            if(ip_addr == NULL)
            {
                printf("get ip addr failed\n");
                return -1;
            }
            if(strncasecmp(ip_addr,"0.0.0.0",sizeof("0.0.0.0")) == 0)
            {
                sleep(1);
                continue;
            }
            get_vaild_ip_flag = 1;
            uuip = get_uuid(DEVICE_NAME);
            if(uuip == NULL)
            {
                printf("get uuid failed\n");
                return -1;
            }
            ret = create_decriptor_xml();
            if(ret < 0)
            {
                printf("rebuild xml descriptor file failed\n");
                return -1;
            }  
        }

        snprintf(location, sizeof(location) - 1, "http://%s/descriptor.xml", ip_addr);
        snprintf(usn, sizeof(usn) - 1, usn_fmt, uuip);
        length = rt_snprintf((char*)buf, SSDP_BUFSZ - 1, SSDP_ALIVE_MESSAGE,
                                SSDP_DEFAULT_MAX_AGE,
                                location,
                                agent,
                                nt,
                                usn
                            );
        ret = sendto(g_sock, buf, length, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
        if(ret < 0)
        {
            printf("upnp sendto error ret %d\n",ret);
        }
        sleep(SSDP_NOTIFY_SECOND);
    }
    return 0;
}