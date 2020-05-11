#ifndef __MAIN_H__
#define __MAIN_H__

#include "visconfig.h"
#include "netconfig.h"
//#include "../demo.h"

#define	KERNALVERSION	"/proc/viscodec/version"
#define	VERSION	        "/root/version.ini"
#define	RECV_BUFLEN	    VISCONFIG_MAXLENGTH
#define	SEND_BUFLEN	    VISCONFIG_MAXLENGTH

typedef struct DynamicParams {
    int port;
    unsigned char ip[16];
    int iframe;
} DynamicParams;

extern VisStatus visstatus;
extern net_config net_cfg;
extern pthread_mutex_t mutex;
extern int child_status;
extern int restart_times;
extern int restart_flg;
extern int net_cfg_flg;

void getvisstatus(net_config *, VisStatus *);
void getversion(BoardInfo *);

/* Enables or disables debug output */
#if 0
#ifdef __DEBUG
#define DBG(fmt, args...) printf("Debug: " fmt, ## args)
#else
#define DBG(fmt, args...)
#endif

#define ERR(fmt, args...) fprintf(stderr, "Error: " fmt, ## args)
#endif

#endif  //end of __MAIN_H__

