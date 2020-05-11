#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/route.h>
#include <stdlib.h>

//#include "main.h"
#include "netconfig.h"
#include "../include/util.h"

#define NET_CONFIG_FILE	"../configFile/viscfgEnc.net"
#define IFNAME_DEFAULT "eth0"

static struct sockaddr_in sa = {
	sin_family:	PF_INET,
	sin_port:	0
};

struct net_config defNetCfg = {
	.ip         = 0xdf12a8c0,
	.mask       = 0xffffff,
	.gateway    = 0x112a8c0,
    .mac        = {0x30,0x0a,0x09,0x11,0xe0,0x44},
#ifdef DHCP
    .name       = "VISCODEC ENC",
	.isdhcp		= 0
#else
    .name       = "VISCODEC ENC"
#endif
};

int net_init(net_config *param)
{
	int n, i, err = 0;
    unsigned char temp = 0;
    FILE *fp;

    if ((fp = fopen(NET_CONFIG_FILE, "rb")) == NULL) {
        perror("fopen netconf file for read err");
		memcpy(param, &defNetCfg, sizeof(struct net_config));
        err = 1;
    }

    if (err == 0) {
        if ((n = fread(param, 1, sizeof(net_config), fp)) <= 0) {
            perror("fread netconf file err");
            err = 1;
        }
        if (n != sizeof(net_config)) {
			if (sizeof(net_config)-4 == n) {
#ifdef DHCP
				printf("master: netconfig load params of early version\n");
				param->isdhcp = (param->ip)?0:1;
				err = 0;
#else
				err = 1;
#endif
			} else {
	            printf("net config file maybe destoryed, use default\n");
		        err = 1;
			}
        }
        fclose(fp);

#ifdef DHCP
		if (param->isdhcp) {
			param->ip = param->mask = param->gateway = 0;
			err = 0;
			return 0;
		}
#endif
        if ((param->ip&param->mask) != (param->gateway&param->mask)) {
            err = 1;
        }
        if (((htonl(param->ip)&0xff000000) == 0) || ((htonl(param->ip)&0xff) == 0xff) || (htonl(param->ip) >= 0xe0000000)) {
            err = 1;
        } else if ((param->mask == 0) || (param->mask == 0xffffffff) || ((htonl(param->mask)&0xff000000) == 0)
                   || ((htonl(param->mask)&0xff0000) == 0) ){//|| ((htonl(param->mask)&0xff00) == 0)) { //linxj2011-06-01
            err = 1;
        } else if (((htonl(param->gateway)&0xff000000) == 0) || ((htonl(param->gateway)&0xff) == 0xff)) {
            err = 1;
        } else if ((param->ip == param->mask) || (param->ip == param->gateway) || (param->mask == param->gateway)) {
            err = 1;
        }
        for (i=0; i<6; i++)
            temp |= param->mac[i];
        if (temp == 0x0) {
            err = 1;
        }
    }

    if (err == 1) {
		memcpy(param, &defNetCfg, sizeof(net_config));
    }

    return 0;
}

int net_modify(net_config *param)
{
    FILE *fp;
	struct ifreq ifr;
	struct rtentry rt;
	int sockfd;
#ifdef DHCP
	if (param->isdhcp) {
		param->ip = param->mask = param->gateway = 0;
	}
#endif
    if ((fp = fopen(NET_CONFIG_FILE, "wb")) == NULL) {
        perror("fopen netconf file for write err");
    }
	if (fwrite(param, 1, sizeof(net_config), fp) <= 0) {
        perror("fwrite netconf file err");
    }
    fclose(fp);
#ifdef DHCP
	if (param->isdhcp) {
		printf("\n\n\n------------------------------------net_cfg.isdhcp = %d\n", param->isdhcp);
		system("udhcpc");
//		if (-1 == net_getstatus(param)) {
//			printf("[E]net_modify get net status error\n");
//			return -1;
//		}
		if (-1 == net_setmac(param)) {
			printf("[E]net_modify set mac error\n");
			return -1;
		}
		printf("ipAddr=0x%x\n", param->ip);
		printf("mask=0x%x\n", param->mask);
		printf("mac=%02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n\n\n", param->mac[0], param->mac[1], param->mac[2], param->mac[3], param->mac[4], param->mac[5]);

		return 0;
	}
#endif

	memset(&ifr, 0, sizeof(struct ifreq));
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket");
		return -1;
	}

	//steven 09-27-09, set macAddr
	strncpy(ifr.ifr_name, IFNAME_DEFAULT, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
		perror("get MAC err\n");
		close(sockfd);
		return -1;
	}

	strncpy(ifr.ifr_name, IFNAME_DEFAULT, IFNAMSIZ);
	memcpy(ifr.ifr_ifru.ifru_hwaddr.sa_data, param->mac, IFHWADDRLEN);
//	print_in_hex(param->mac, IFHWADDRLEN, "Src Mac", NULL);
//	print_in_hex(ifr.ifr_ifru.ifru_hwaddr.sa_data, IFHWADDRLEN, "New Mac", NULL);
	if (ioctl(sockfd, SIOCSIFHWADDR, &ifr) < 0) {
        perror("set macaddr err");
		close(sockfd);
		return -1;
	}

	//steven 09-27-09, set ipaddr 
	sa.sin_addr.s_addr = param->ip;
	strncpy(ifr.ifr_name, IFNAME_DEFAULT, IFNAMSIZ);
	memcpy((char *) &ifr.ifr_addr, (char *) &sa, sizeof(struct sockaddr));
	if (ioctl(sockfd, SIOCSIFADDR, &ifr) < 0) {
		close(sockfd);
		perror("set ipaddr err\n");
		return -1;
	}

	//steven 09-27-09, set mask
	sa.sin_addr.s_addr = param->mask;
	strncpy(ifr.ifr_name, IFNAME_DEFAULT, IFNAMSIZ);
	memcpy((char *) &ifr.ifr_addr, (char *) &sa, sizeof(struct sockaddr));
	if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) < 0) {
		close(sockfd);
        perror("set mask err");
		//return -1;    //sp 12-02-09 cut a bug
	}

	//steven 09-27-09, set gateway Addr
	// Clean out the RTREQ structure.
	memset((char *) &rt, 0, sizeof(struct rtentry));
	// Fill in the other fields.
	rt.rt_flags = (RTF_UP | RTF_GATEWAY);
	rt.rt_dst.sa_family = PF_INET;
	rt.rt_genmask.sa_family = PF_INET;

	sa.sin_addr.s_addr = param->gateway;
	memcpy((char *) &rt.rt_gateway, (char *) &sa, sizeof(struct sockaddr));
	// Tell the kernel to accept this route.
	if (ioctl(sockfd, SIOCADDRT, &rt) < 0) {
		close(sockfd);
        perror("set gateway err");
		//return -1;    //sp 12-02-09 cut a bug
	}
	close(sockfd);

    return 0;
}

int net_getstatus(struct net_config *param) {
	struct ifreq ifr;
	int sockfd;
/*
	struct rtentry rt;
	FILE *fp = NULL;
    if ((fp = fopen(NET_CONFIG_FILE, "wb")) == NULL) {
        perror("fopen netconf file for write err");
    }
	if (fwrite(param, 1, sizeof(net_config), fp) <= 0) {
        perror("fwrite netconf file err");
    }
    fclose(fp);
*/
	memset(&ifr, 0, sizeof(struct ifreq));
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
		return -1;
	}

	//steven 09-27-09, get ipaddr 
	strncpy(ifr.ifr_name, IFNAME_DEFAULT, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
		close(sockfd);
		perror("set ipaddr err\n");
		return -1;
	}
	memcpy((char *)&sa, (char *)&ifr.ifr_addr, sizeof(struct sockaddr));
	param->ip = sa.sin_addr.s_addr;

	//steven 09-27-09, get mask
	strncpy(ifr.ifr_name, IFNAME_DEFAULT, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFNETMASK, &ifr) < 0) {
		close(sockfd);
        perror("set mask err");
		//return -1;    //sp 12-02-09 cut a bug
	}
	memcpy((char *)&sa, (char *)&ifr.ifr_addr, sizeof(struct sockaddr));
	param->mask = sa.sin_addr.s_addr;
#ifdef DHCP
	strncpy(ifr.ifr_name, IFNAME_DEFAULT, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
		perror("get MAC err\n");
		close(sockfd);
		return -1;
	}
	memcpy(param->mac, ifr.ifr_ifru.ifru_hwaddr.sa_data, IFHWADDRLEN);
#else
	strncpy(ifr.ifr_name, IFNAME_DEFAULT, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
		perror("get MAC err\n");
		close(sockfd);
		return -1;
	}
	memcpy(param->mac, ifr.ifr_ifru.ifru_hwaddr.sa_data, IFHWADDRLEN);
#endif
/*	//how to get GATEWAY? ls, 2013-02-25
	//steven 09-27-09, set gateway Addr
	// Clean out the RTREQ structure.
	memset((char *) &rt, 0, sizeof(struct rtentry));
	// Fill in the other fields.
	rt.rt_flags = (RTF_UP | RTF_GATEWAY);
	rt.rt_dst.sa_family = PF_INET;
	rt.rt_genmask.sa_family = PF_INET;

	sa.sin_addr.s_addr = param->gateway;
	memcpy((char *) &rt.rt_gateway, (char *) &sa, sizeof(struct sockaddr));
	// Tell the kernel to accept this route.
	if (ioctl(sockfd, SIOCADDRT, &rt) < 0) {
		close(sockfd);
        perror("set gateway err");
		//return -1;    //sp 12-02-09 cut a bug
	}
*/
	if (ioctl(sockfd, SIOCGIFBRDADDR, &ifr) < 0) {
		perror("get broadcast addr err\n");
		close(sockfd);
		return -1;
	}
	memcpy(&sa, &ifr.ifr_addr, sizeof(struct sockaddr));
	param->gateway = sa.sin_addr.s_addr;
	if ((param->gateway&0xff000000) == 0xff000000) param->gateway &= 0x01ffffff;
	if ((param->gateway&0x00ff0000) == 0x00ff0000) param->gateway &= 0x0101ffff;
	if ((param->gateway&0x0000ff00) == 0x0000ff00) param->gateway &= 0x010101ff;
	if ((param->gateway&0x000000ff) == 0x000000ff) param->gateway &= 0x01010101; 
	printf("broadcast addr is %08x\n", param->gateway);

	close(sockfd);
    return 0;
}

int net_setmac(struct net_config *param) {
	struct ifreq ifr;
	int sockfd;

	memset(&ifr, 0, sizeof(struct ifreq));
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket");
		return -1;
	}

	//steven 09-27-09, set macAddr
	strncpy(ifr.ifr_name, IFNAME_DEFAULT, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
		perror("get MAC err\n");
		close(sockfd);
		return -1;
	}

	strncpy(ifr.ifr_name, IFNAME_DEFAULT, IFNAMSIZ);
	memcpy(ifr.ifr_ifru.ifru_hwaddr.sa_data, param->mac, IFHWADDRLEN);
	if (ioctl(sockfd, SIOCSIFHWADDR, &ifr) < 0) {
        perror("set macaddr err");
		close(sockfd);
		return -1;
	}

	close(sockfd);
	return 0;
}
