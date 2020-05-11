#ifndef __VIS_MMNP_UDP_H__
#define __VIS_MMNP_UDP_H__

//#include <pthread.h>
#include "vis_mmnp.h"
#include "ringbuffer.h"

typedef struct conn_client {
	int				*type_ptr;	//UDP protocol type, multi broadcast or uni
	unsigned int	 maxConnNum;
	unsigned int *conn_num_ptr;
	char			*url;
	char			*src_ipAddr;
	unsigned short	*src_port_ptr;
	char			*dst_ipAddr;
	unsigned short	*dst_port_ptr;
//	int				 socket;

	unsigned short		*port_table;	//not use
	struct sockaddr_in	*dstaddr_table;
	char				*flag_table;
	unsigned int *time_table;
	pthread_mutex_t mutex;
} conn_client_t;

/* Environment passed when creating the thread */
typedef struct demand_env {
	pthread_mutex_t *mutex_initsync;
	pthread_cond_t *cond_initsync;
//	Vis_Mmnp_Handle *handle_ptr;
	volatile int *quit_flag_ptr;
	conn_client_t	*conn_client;
} DemandEnv;

typedef struct udp_env {
//	char *url;
//	char *ip;
//	unsigned short port;
//	unsigned short maxConnNum;
	pthread_mutex_t *mutex_initsync;
	pthread_cond_t *cond_initsync;
//	Vis_Mmnp_Handle *handle_ptr;
	volatile int *quit_flag_ptr;
	conn_client_t	*conn_client;
	RingBuffer *pbuffer;
//	pthread_t demand_tid;
} UdpEnv;

/* Thread function prototype */
extern void *vmn_udpThrFxn(void *arg);
extern void *vmn_demandThrFxn(void *arg);

#endif	//__VIS_MMNP_UDP_H__
