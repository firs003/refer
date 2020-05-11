/*
 * rtsp.h
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#ifndef __VIS_MMNP_RTSP_H__
#define __VIS_MMNP_RTSP_H__

#ifdef  __cplusplus
extern "C"
{
#endif

#include "ringbuffer.h"

/* Environment passed when creating the thread */
typedef struct rtsp_env {
	unsigned short videoEnable;
	unsigned short audioEnable;
	unsigned short videofps;
	unsigned short audioChannel;
	unsigned int *audio_samplerate_ptr;
	char *url;
	char *ip;
	unsigned short port;
	unsigned short maxConnNum;
//	pthread_mutex_t *mutex_initsync;	//create in vmn_monitor(), use in sender thread and vmn_monitor()
//	pthread_cond_t *cond_initsync;		//create in vis_mmnp_create(), use in sender thread and vis_mmnp_start()
//	Vis_Mmnp_Handle *handle_ptr;
	volatile int *quit_flag_ptr;
	unsigned int *conn_num_ptr;
	pthread_mutex_t *conn_num_mutex;
	RingBuffer *pbuffer_video;
	RingBuffer *pbuffer_audio;
} RtspEnv;

/* Thread function prototype */
void *vmn_rtspThrFxn(void *arg);

#ifdef  __cplusplus
}
#endif

#endif /* __VIS_MMNP_RTSP_H__ */
