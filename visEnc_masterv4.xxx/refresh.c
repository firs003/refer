#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "main.h"
#include "../demo_common.h"

/*******************************************************************************************************************
 * extern functions
 *******************************************************************************************************************/
extern int killname(char *cmd);

static int udp_init(void)
{
    int sockfd;
    struct sockaddr_in addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("refresh socket");
    }
    
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(CLIENT_UDPPORT);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof addr) == -1) {
        perror("refresh bind");
		close(sockfd);
    }

    return sockfd;
}

static int bcst_init(void)
{
    int sockfd, opt = 1;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("refresh socket");
    }
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt));

    return sockfd;
}

static int getlocalip(void)
{ 
    int sockfd;  
    struct ifreq ifreq;  
    struct in_addr inaddr;
    int *addr;

    if(-1 == (sockfd = socket(PF_INET, SOCK_STREAM, 0)))  
        return 0;

    bzero(&ifreq, sizeof(struct ifreq));
    strncpy(ifreq.ifr_name, "eth0", IFNAMSIZ);
    ioctl(sockfd, SIOCGIFADDR, &ifreq);
    close(sockfd);
    memcpy(&inaddr, &((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr, sizeof(struct in_addr));
    addr = (int*)&inaddr;

    return *addr;
}

void *refreshThrFxn(void *arg)
{

    int 	udp_sockfd;
    int		bcst_sockfd;
    struct sockaddr_in remote_addr;
    socklen_t	remote_addr_len = sizeof(struct sockaddr);
    struct sockaddr_in bcst_addr;
    int		ret;
    char	sendbuf[SEND_BUFLEN];
	char    recvbuf[RECV_BUFLEN];
    int     addr;
	
    vis_rq_setdefault       setdefault;
    RestartParam    restartparam;
    Vis_con			vis_con = {0};
    BoardConf       boardconf;

    pthread_detach(pthread_self());

	udp_sockfd = udp_init();
    bcst_sockfd = bcst_init();

    while (1) {
        ret = recvfrom(udp_sockfd, recvbuf, RECV_BUFLEN, 0, 
                                (struct sockaddr *)&remote_addr, &remote_addr_len);
		printf("refresh.c: recvfrom [ip=%08x, port=%hu]\n", (unsigned int)remote_addr.sin_addr.s_addr, ntohs(remote_addr.sin_port));
		printf("refresh.c: recvfrom [ip=%08x, port=%hu]\n", (unsigned int)remote_addr.sin_addr.s_addr, remote_addr.sin_port);
        if (ret != -1 ) {
            if (recvbuf[0] != 'v' || recvbuf[1] != 'i' || recvbuf[2] != 's') {
                ERR("bad message, just drop! vis ... ... ... ...\n");	
                continue;
            }
            memset(&bcst_addr, 0, sizeof(struct sockaddr_in));
            bcst_addr.sin_family = AF_INET;
            bcst_addr.sin_addr.s_addr = INADDR_BROADCAST;
            bcst_addr.sin_port = htons(SERVER_UDPPORT);

            if (recvbuf[3] == VIS_RQ_BROADCAST ) {
                printf("VIS_RQ_BROADCAST,message_len:%d\n", ret);
                if (4 <= ret) {
                    vis_con.vis_con_syn0='v';
                    vis_con.vis_con_syn1='i';
                    vis_con.vis_con_syn2='s';
                    vis_con.vis_con_cmd = VIS_RS_BROADCAST;
                    memset(sendbuf, 0, SEND_BUFLEN);
                    memcpy(sendbuf, &vis_con, sizeof(vis_con));

                    pthread_mutex_lock(&mutex);
                    getvisstatus(&net_cfg, &visstatus);
                    pthread_mutex_unlock(&mutex);
                    memcpy(sendbuf + sizeof(vis_con), &visstatus.bdinfo, sizeof(visstatus.bdinfo));

                    ret = sendto(bcst_sockfd, sendbuf, sizeof(visstatus.bdinfo)+sizeof(vis_con), 0,
                        (struct sockaddr *)&bcst_addr, sizeof(struct sockaddr));
                    if (-1 == ret) {
                        perror("sendto");
                    }		
                }
            } else if (recvbuf[3] == VIS_RQ_RESTARTCMD ) {
                printf("VIS_RQ_RESTARTCMD,message_len:%d\n", ret);
                if ((4 + sizeof(RestartParam)) <= ret) {
                    memcpy(&restartparam, recvbuf + 4, sizeof(RestartParam));
                } else {
                    ERR("bad message, just drop VIS_RQ_RESTARTCMD:!\n");
                    break;
                }
                addr = getlocalip();
                printf("localaddr = 0x%x, destip = 0x%x\n", addr, restartparam.DstIP);
                if (restartparam.DstIP == 0xffffffff || restartparam.DstIP == 0xfffffffe || restartparam.DstIP == addr) {
                    system("/sbin/reboot -f \n");
                }
            } else if (recvbuf[3] == VIS_RQ_SETDEFAULT ) {
                printf("VIS_RQ_SETDEFAULT,message_len:%d\n", ret);
                if ((4 + sizeof(vis_rq_setdefault)) <= ret) {
                    memcpy(&setdefault, recvbuf + 4, sizeof(vis_rq_setdefault));
                } else {
                    ERR("bad message, just drop VIS_RQ_SETDEFAULT:!\n");
                    break;
                }
                addr = htonl(getlocalip());
                if (setdefault.DstIP == 0xffffffff || setdefault.DstIP == addr) {
                    system("flash_eraseall /dev/mtd4; reboot -f \n");
                }
            } else if (recvbuf[3] == VIS_RQ_CONFIGBDINFO) {
                if ((4 + sizeof(BoardConf)) <= ret) {
                    memcpy(&boardconf, recvbuf + 4, sizeof(BoardConf));
                } else {
                    ERR("bad message, just drop VIS_RQ_SETBDINFO:!\n");
                    break;
                }
                printf("ret = %d, sizeof(boardconf) = %d\n", ret, sizeof(boardconf));
                printf("recvbuf[3] = 0x%x\n", recvbuf[3]);
                printf("boardconf.DstIP = 0x%x\n", boardconf.DstIP);
                printf("boardconf.bdinfo.IPAddr = 0x%x\n", boardconf.bdinfo.IPAddr);
                printf("boardconf.bdinfo.IPMask = 0x%x\n", boardconf.bdinfo.IPMask);
                printf("boardconf.bdinfo.GateIP = 0x%x\n", boardconf.bdinfo.GateIP);
                int i;
                printf("mac: ");
                for (i=0;i<6;i++)
                    printf("%02hhx ", boardconf.bdinfo.MAC[i]);
                printf("\n");
                addr = getlocalip();
                printf("localaddr = 0x%x\n", addr);
                if (boardconf.DstIP != addr) {
                    printf("not this board\n");
                    continue;
                }
#if 1

				printf("refresh.c:IP=0x%08x, mask=0x%08x, Gate=0x%08x\n", boardconf.bdinfo.IPAddr, boardconf.bdinfo.IPMask, boardconf.bdinfo.GateIP);
				//IPMask, IPAddr, GateIP, all 0, then dhcp
				if (0==boardconf.bdinfo.IPMask && 0==boardconf.bdinfo.IPAddr && 0==boardconf.bdinfo.GateIP) {
#ifdef DHCP
							struct timeval tv;
							printf("master:dhcp\n");
							net_cfg.isdhcp = 1;
//							system("udhcpc\n");
							memcpy(net_cfg.mac, boardconf.bdinfo.MAC, sizeof(net_cfg.mac));
							net_cfg.mac[0] = 0x32;
							if (0 == gettimeofday(&tv, NULL)) {
								net_cfg.mac[1] = ((unsigned int)tv.tv_usec)&0xff;
							} else {
								perror("<w>master: set mac from time error");
							}
							printf("master: net_cfg.mac=%02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n", net_cfg.mac[0], net_cfg.mac[1], net_cfg.mac[2], net_cfg.mac[3], net_cfg.mac[4], net_cfg.mac[5]);
//							sleep(5);
#else
						printf("<w>:master:board info network params invalid\n");
						break;
#endif
				} else {
					if (0==boardconf.bdinfo.IPMask || 0==boardconf.bdinfo.IPAddr || 0==boardconf.bdinfo.GateIP || (boardconf.bdinfo.IPMask&boardconf.bdinfo.IPAddr)!=(boardconf.bdinfo.IPMask&boardconf.bdinfo.GateIP)) {
						printf("<w>:master:board info network params invalid\n");
						break;
					} 
#ifdef DHCP
					if (net_cfg.isdhcp) {
						if (-1 == killname("udhcpc")) {
							printf("<w>master: kill udhcpc failed\n");
						}
					}
					net_cfg.isdhcp = 0;
#endif
					if ((boardconf.bdinfo.IPMask&boardconf.bdinfo.IPAddr) == (boardconf.bdinfo.IPMask&boardconf.bdinfo.GateIP)) {
						net_cfg.ip = boardconf.bdinfo.IPAddr;
						net_cfg.mask = boardconf.bdinfo.IPMask;
						net_cfg.gateway = boardconf.bdinfo.GateIP;
						memcpy(&(net_cfg.mac), &(boardconf.bdinfo.MAC), 6);
						net_cfg.mac[0] = 0x30;
					}
				}
				memcpy(&(net_cfg.name), &(boardconf.bdinfo.BoardName), 32);
				DBG("VIS_RQ_SETBDINFO restart_flg is set... ... ...!\n");
				net_cfg_flg = 1;
				restart_flg = 1;


#else


                if ((boardconf.bdinfo.IPMask&boardconf.bdinfo.IPAddr) == (boardconf.bdinfo.IPMask&boardconf.bdinfo.GateIP)) {
                    net_cfg.ip = boardconf.bdinfo.IPAddr;
                    net_cfg.mask = boardconf.bdinfo.IPMask;
                    net_cfg.gateway = boardconf.bdinfo.GateIP;
                    memcpy(&(net_cfg.mac), &(boardconf.bdinfo.MAC), 6);
                    net_cfg.mac[0] = 0x30;
                }
                memcpy(&(net_cfg.name), &(boardconf.bdinfo.BoardName), 32);
                DBG("VIS_RQ_CONFIGBDINFO restart_flg is set... ... ...!\n");
                net_cfg_flg = 1;
                restart_flg = 1;
#endif
            } else if (recvbuf[3] == VIS_RQ_GETSTATUS ) {
                printf("VIS_RQ_GETSTATUS,message_len:%d\n", ret);
                if (4 <= ret) {
                    vis_con.vis_con_syn0='v';
                    vis_con.vis_con_syn1='i';
                    vis_con.vis_con_syn2='s';
                    vis_con.vis_con_cmd = VIS_RS_SENDSTATUS;
                    memset(sendbuf, 0, SEND_BUFLEN);
                    memcpy(sendbuf, &vis_con, sizeof(vis_con));

                    pthread_mutex_lock(&mutex);
                    getvisstatus(&net_cfg, &visstatus);
                    pthread_mutex_unlock(&mutex);
                    memcpy(sendbuf + sizeof(vis_con), &visstatus, sizeof(visstatus));

                    ret = sendto(bcst_sockfd, sendbuf, sizeof(visstatus) + sizeof(vis_con), 0,
                        (struct sockaddr *)&bcst_addr, sizeof(struct sockaddr));
                    if (-1 == ret) {
                        perror("sendto");
                    }
                }
            }
        }
    }

    return 0;
}
