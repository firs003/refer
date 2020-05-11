#ifndef __NETCONFIG_H__
#define __NETCONFIG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define DHCP

typedef struct net_config {
    int ip;
    int mask;
    int gateway;
    unsigned char mac[6];
    unsigned char name[32];
#ifdef DHCP
	int	isdhcp;
#endif
}net_config;

/*
 * FUNCTION : read config file to fill the args and enable the network use the args
 * ARGS : "net_config *" use for return value
 * RETURN : "-1" is error, "0" is ok
 */
int net_init(net_config *);

/*
 * FUNCTION : use the args to change the network then save it in config file
 * ARGS : "net_config *" use for deliver the value
 * RETURN : "-1" is error, "0" is ok
 */
int net_modify(net_config *param);
int net_getstatus(net_config *param);
int net_setmac(net_config *param);

#ifdef __cplusplus
};
#endif

#endif // __NETCONFIG_H__
