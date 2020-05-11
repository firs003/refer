#ifndef __VIS_MMNP_TCP_H__
#define __VIS_MMNP_TCP_H__

#include "vis_mmnp.h"
#include "ringbuffer.h"

#define MAXQ 8

typedef struct TcpEnv {
	char *url;
	char *ip;
    unsigned short port;
	unsigned short maxConnNum;
	pthread_mutex_t *mutex_initsync;	//create in vmn_monitor(), use in sender thread and vmn_monitor()
	pthread_cond_t *cond_initsync;		//create in vis_mmnp_create(), use in sender thread and vis_mmnp_start()
//	Vis_Mmnp_Handle *handle_ptr;
	volatile int *quit_flag_ptr;
	unsigned int *conn_num_ptr;
	pthread_mutex_t *conn_num_mutex;
	RingBuffer *pbuffer;
} TcpEnv;

extern void *vmn_tcpThrFxn(void *arg);

#endif /* __VIS_MMNP_TCP_H__ */
