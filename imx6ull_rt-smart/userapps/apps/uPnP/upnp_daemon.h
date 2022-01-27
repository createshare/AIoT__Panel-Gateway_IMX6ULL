#ifndef SSDP_H__
#define SSDP_H__

/*
 * field:
 * ST  - search target
 * MX -
 * NTS
 * NT
 * USN
*/

#define SSDP_ADDR                       "239.255.255.250"
#define SSDP_PORT				1900
#define SSDP_PORT_STR 			"1900"

#define SSDP_SEARCH_METHOD 		"M-SEARCH"
#define GENA_NOTIFY_METHOD 		"NOTIFY"

#define SSDP_ALIVE_NTS  		"ssdp:alive"
#define SSDP_BYEBYE_NTS 		"ssdp:byebye"

#define SSDP_DEFAULT_MAX_AGE 	1800
#define SSDP_DEFAULT_MX      	3

#define SSDP_ALIVE_MESSAGE                                          \
        "NOTIFY * HTTP/1.1\r\n"                                     \
        "Host: " SSDP_ADDR ":" SSDP_PORT_STR "\r\n"                 \
        "Cache-Control: max-age=%d\r\n"                             \
        "Location: %s\r\n"                                          \
        "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"     \
        "Server: RT-Thread/2.1.0, UPnP 1.0\r\n"                     \
        "X-User-Agent: %s\r\n"                                      \
        "NTS: ssdp:alive\r\n"                                       \
        "NT: %s\r\n"                                                \
        "USN: %s\r\n\r\n"

#define SSDP_BYEBYE_MESSAGE                         \
        "NOTIFY * HTTP/1.1\r\n"                     \
        "Host: " SSDP_ADDR ":" SSDP_PORT_STR "\r\n" \
        "NTS: ssdp:byebye\r\n"                      \
        "NT: %s\r\n"                                \
        "USN: %s\r\n\r\n"

#define SSDP_DISCOVERY_REQUEST                      \
        "M-SEARCH * HTTP/1.1\r\n"                   \
        "Host: " SSDP_ADDR ":" SSDP_PORT_STR "\r\n" \
        "Man: \"ssdp:discover\"\r\n"                \
        "ST: %s\r\n"                                \
        "MX: %d\r\n"                                \
        "User-Agent: %s GSSDP/" VERSION "\r\n\r\n"  \

#define SSDP_DISCOVERY_RESPONSE                     \
        "HTTP/1.1 200 OK\r\n"                       \
        "Location: %s\r\n"                          \
        "%s"                                        \
        "Ext:\r\n"                                  \
        "USN: %s\r\n"                               \
        "Server: %s\r\n"                            \
        "Cache-Control: max-age=%d\r\n"             \
        "ST: %s\r\n"                                \
        "Date: %s\r\n"                              \
        "Content-Length: 0\r\n\r\n"

/***************** UPNP  **************/
#define SSDP_UDP_PORT           1900
typedef struct
{
    char * head;
    char * S;
    char * Host;
    char * ST;
    char * Man;
    char * MX;
}ssdp_netbios_request_t;

int ssdp_init(void);

#endif
