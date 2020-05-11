#ifndef _VIS_COMMON_H
#define _VIS_COMMON_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define		ADDRSTRLEN	16
#define		SEND_LEN	(TS_LENGTH * TS_NUM)
#define		MAX_CONNECTION 8

#define		PAL_WIDTH	720
#define		PAL_HEIGHT	576
#define		VGA_WIDTH   1024    //shenpei 2009-11-11	
#define		VGA_HEIGHT	768     //shenpei 2009-11-11

#define     BROADCAST   1
#define     MULTICAST   2
#define     UNICAST     4
#define     NOCAST      8

#define NEWCODEC
#ifdef NEWCODEC
#define     IDRHEAD_LEN 102
#else
#define     IDRHEAD_LEN 114
#endif

typedef struct _vis_global_t {
    int			socket;
    char		ipAddr[ADDRSTRLEN];
    int			port;
    struct sockaddr_in	dest_addr;
    sem_t		sem_protect;
} vis_global_t;

typedef struct conn_client {
    int          current;
    int			 socket;
    char		 ipAddr[ADDRSTRLEN];
    int          num_connect;
    int			 port[MAX_CONNECTION];
    struct sockaddr_in	dest_addr[MAX_CONNECTION];
    char         flag[MAX_CONNECTION];
    unsigned int time[MAX_CONNECTION];
    pthread_mutex_t mutex;
} conn_client_t;

typedef struct DynamicParams {
    int port;
    char ip[ADDRSTRLEN];
    int iframe;
} DynamicParams;

extern vis_global_t vis_global;
extern conn_client_t conn_client;

int socketInit(vis_global_t *vis_global);

#endif
