#include "vis_mmnp.h"

#include "rtmp.h"
#include "RtmpSendApi.h"

//#define MULTI_RTMP_SENDER

#ifdef MULTI_RTMP_SENDER
static unsigned int sock_count = 0;	//ls 2013-0-17 for test
static sem_t sem_rtmp;
#endif

typedef struct send_thread_env {
	volatile int *quit_flag_ptr;
	RingBuffer *pbuffer;
	int		 thread_index;	//for ringbuffer use
	char	*url;			//for rtmp handle use
	/* video infomation */
	int		 video_width;
	int		 video_height;
	int		 video_compress;
	int		 video_frameType;
	/* audio infomation */
	int		 audio_bits;
	int		 *audio_samplerate_ptr;
	int		 audio_channels;
	int		 audio_compress;
} Send_Thread_Env;

typedef enum
{
	RTMP_LOGCRIT=0,
	RTMP_LOGERROR,
	RTMP_LOGWARNING,
	RTMP_LOGINFO,
	RTMP_LOGDEBUG,
	RTMP_LOGDEBUG2,
	RTMP_LOGALL
} RTMP_LogLevel;
extern void RTMP_LogSetLevel(RTMP_LogLevel lvl);

static void *sendThread(void *arg)
{
	void *status = THREAD_SUCCESS;
	struct send_thread_env *envp = (struct send_thread_env *)arg;
	void *hRtmp = NULL;
	rtmp_write_param writeParams;
	int len = 0, stream_type = 0, ret, index = envp->thread_index;
	unsigned char *sendbuf = NULL;
	struct timeval current = {0, 0}, base = {0, 0};
	RingBuffer *pbuffer = envp->pbuffer;

	Vmn_Log_Debug("url=[%s]\n", envp->url);
	hRtmp = rtmpsender_create(envp->url,0,0);
	if (hRtmp == NULL) {
		Vmn_Log_Error("rtmp handler create failed");
		cleanup(THREAD_FAILURE);
	}
	RTMP_LogSetLevel(RTMP_LOGWARNING);	//do not display DEBUG info

	/* write params init */
	memset(&writeParams, 0, sizeof(rtmp_write_param));
	writeParams.size			 = sizeof(rtmp_write_param);
	writeParams.video_width		 = envp->video_width;
	writeParams.video_height	 = envp->video_height;
	writeParams.video_encodeType = envp->video_compress;
	writeParams.video_frameType  = envp->video_frameType;
	writeParams.audio_bits		 = envp->audio_bits;
	writeParams.audio_samplerate = *envp->audio_samplerate_ptr;	//audio samplerate maybe change when audio input is hdmi
	writeParams.audio_channels	 = envp->audio_channels;
	writeParams.audio_encodeType = envp->audio_compress;
	writeParams.video_tsenable   = 1;
	writeParams.audio_tsenable   = 1;
	printf("video_env, width=%d, height=%d, frameType=%d, compress=%d\n", writeParams.video_width, writeParams.video_height, writeParams.video_frameType, writeParams.video_encodeType);
	printf("audio_env, bits=%d, samplerate=%d, channels=%d, compress=%d\n", writeParams.audio_bits, writeParams.audio_samplerate, writeParams.audio_channels, writeParams.audio_encodeType);

//	printf("rtmp.c: conn_server.connfd[%d] = %d, <sock_count=%d>\n", index, conn_server.connfd[index], sock_count);
#ifdef MULTI_RTMP_SENDER
	pthread_detach(pthread_self());
#endif
	vis_ring_buffer_regthread(pbuffer, index);
	gettimeofday(&base, NULL);
	while(envp->quit_flag_ptr && !(*envp->quit_flag_ptr)) {
		/* Get a avdata package */
		if ((sendbuf = vis_ring_buffer_send_addr(pbuffer, &len, &stream_type, NULL, NULL, index)) == NULL) {
		// if ((sendbuf = vis_ring_buffer_send_addr(pbuffer, &len, &stream_type, &timestamp_sec, &timestamp.usec, NULL, index)) == NULL) {	//TODO, timestamp
			Sleep(10);//linxj2014-03-31
			continue;
		}

		/* send the package thought rtmp stack */
		writeParams.audio_samplerate = *envp->audio_samplerate_ptr;
		writeParams.buffer = (char *)sendbuf;
		writeParams.bufsize = len;
		writeParams.buftype = is_video(stream_type) ? BUF_TYPE_H264 : BUF_TYPE_AAC;	//TODO, maybe other type, like G711, MPEG4, etc.
		writeParams.video_iEnable = is_video(stream_type);
		writeParams.audio_iEnable = is_audio(stream_type);
		gettimeofday(&current, NULL);
		writeParams.video_timestamp = writeParams.audio_timestamp =
//			(current.tv_sec*1000+current.tv_usec/1000) - (base.tv_sec*1000+base.tv_usec/1000);	//maybe overflow
			(current.tv_sec-base.tv_sec)*1000 + ((long long)(current.tv_usec-base.tv_usec))/1000;
		//DBG("timestamp=%u, buffer=%p, len=%d\n", writeParams.video_timestamp, writeParams.buffer, writeParams.bufsize);
		ret = rtmpsender_write(hRtmp, &writeParams);
		if (ret) {
			/* Broken pipe error printed if client has been closed */
			Vmn_Log_Error("send failed, ret = %d", ret);
			Sleep(10);
			continue;
		}
	}

cleanup:
	if (hRtmp) {
		rtmpsender_close(hRtmp);
		hRtmp = NULL;
	}
	vis_ring_buffer_unregthread(pbuffer, index);
#ifdef MULTI_RTMP_SENDER
	sem_wait(&sem_rtmp);
	printf("rtmp.c: conn_server.connfd[%d] = %d disconnect, <sock_count=%d>\n", index, conn_server.connfd[index], sock_count);
	sem_post(&sem_rtmp);
#endif

	return status;
}

/******************************************************************************
 * rtmpThrFxn
 ******************************************************************************/
void *vmn_rtmpThrFxn(void *arg)
{
	RtmpEnv		*envp           = (RtmpEnv *) arg;
	void 		*status         = THREAD_SUCCESS;
//	int         i;
	struct send_thread_env send_env;

#ifndef WIN32
	signal(SIGPIPE, SIG_IGN);
#endif

#ifdef MULTI_RTMP_SENDER
	if (-1 == sem_init(&sem_rtmp, 0, 1)) {
		ERR("sem_init failed");
		cleanup(THREAD_FAILURE);
	}
#endif

	/* Signal that initialization is done and wait for other threads */
	Vmn_Log_Debug("quit_flag[%p]=%d\n", envp->quit_flag_ptr, *envp->quit_flag_ptr);
	Vmn_Log_Info("Before main loop\n");
#ifdef MULTI_RTMP_SENDER
	while (!gblGetQuit()) {		//TODO, MULTI_RTMP_SENDER
		if (pthread_create(&send_thr[i], NULL, sendThread, index_tmp+i) != 0) {
		}
	}
#else
	/* prepare env aguments for sendThread */
	memset(&send_env, 0, sizeof(struct send_thread_env));
	send_env.quit_flag_ptr		= envp->quit_flag_ptr;
	send_env.pbuffer			= envp->pbuffer;
	send_env.thread_index		= 0;
	send_env.url				= envp->url;
	send_env.video_width		= envp->video_width;
	send_env.video_height		= envp->video_height;
	send_env.video_compress		= envp->video_compress;
	send_env.audio_bits			= envp->audio_bits;
	send_env.audio_samplerate_ptr = envp->audio_samplerate_ptr;
	send_env.audio_channels		= envp->audio_channels;
	send_env.audio_compress		= envp->audio_compress;
//	memcpy(((void *)&send_env)+sizeof(int)+sizeof(char *), (void *)envp+2*sizeof(Rendezvous_Handle), sizeof(RtmpEnv)-2*sizeof(Rendezvous_Handle));	//lazy
	/* call sendThread */
	status = sendThread(&send_env);	//rtmp send loop inside
#endif

//cleanup:
	/* Make sure the other threads aren't waiting for init to complete */
	Vmn_Log_Debug("Rtmp Thread Cleanup\n");
	if (envp) *envp->quit_flag_ptr = 1;

	return status;
}
