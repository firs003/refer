#include <xdc/std.h>
#include "vis_common.h"
#include "../demo.h"

extern conn_client_t conn_clinet;

int socketInit(vis_global_t *vis_global)
{
    int fd;
    int opt = 1;
    in_addr_t addr;
    int buflen;
    socklen_t socklen;

    fd = socket(AF_INET, SOCK_DGRAM,0);
    if (fd < 0)
            return fd;
    socklen = sizeof(buflen);
    getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buflen, &socklen);
    buflen *= 4;    //sp 01-06-10 double rcvbuf of udp
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buflen, sizeof(buflen));
    socklen = sizeof(buflen);
    getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buflen, &socklen);
    printf("udp socket buflen = %d\n", buflen);

    addr = inet_addr(vis_global->ipAddr);

    vis_global->dest_addr.sin_family = AF_INET;
    vis_global->dest_addr.sin_port = htons(vis_global->port);
    //if (strcmp(vis_global->ipAddr, "192.168.18.255") == 0) {
    if (((htonl(addr)&0xff) == 0xff) &&
        ((htonl(addr)&0xff0000ff) > 0xef0000ff || (htonl(addr)&0xff0000ff) < 0xe00000ff)) {
            setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt));
            conn_client.current = BROADCAST;
            printf("broadcast ip addr select : %s\n", vis_global->ipAddr);
    } else if (htonl(addr) >= 0xe0000000 && htonl(addr) <= 0xefffffff) {
        conn_client.current = MULTICAST;
#if 0
        u_char      flag;
        socklen_t   len;
        u_char loop = 0;
        u_char ttl = 20;

        len = sizeof(flag);
        if (getsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, &len) < 0)
            return(-1);
        printf("loop = %d\n", flag);
        setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
        len = sizeof(flag);
        if (getsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, &len) < 0)
            return(-1);
        printf("loop = %d\n", flag);

        len = sizeof(flag);
        if (getsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &flag, &len) < 0)
            return(-1);
        printf("ttl = %d\n", flag);
        len = sizeof(flag);
        setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
        if (getsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &flag, &len) < 0)
            return(-1);
        printf("ttl = %d\n", flag);
#else
        u_char loop = 0;
        u_char ttl = 20;
        setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
        setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
#endif

        printf("multicast ip addr select : %s\n", vis_global->ipAddr);
    } else if (htonl(addr) == 0x0) {
        conn_client.current = NOCAST;
        printf("nocast ip addr select : %s\n", vis_global->ipAddr);
    } else {
        conn_client.current = UNICAST;
        printf("unicast ip addr select : %s\n", vis_global->ipAddr);
    }
    if (inet_pton(AF_INET, vis_global->ipAddr, &(vis_global->dest_addr.sin_addr)) <= 0) {
        ERR("self ip address error!\n");
        exit(0);
    }
    if (conn_client.current != NOCAST) {
        conn_client.num_connect = 1;
        conn_client.flag[conn_client.num_connect-1] = 1;
        conn_client.dest_addr[conn_client.num_connect-1] = vis_global->dest_addr;
    }

    return fd;
}
