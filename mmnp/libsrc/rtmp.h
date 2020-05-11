#ifndef __VIS_MMNP_RTMP_H__
#define __VIS_MMNP_RTMP_H__

#include "vis_mmnp.h"
#include "ringbuffer.h"

typedef struct rtmp_env {
	char	*url;
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

	volatile int *quit_flag_ptr;
	RingBuffer *pbuffer;
} RtmpEnv;

extern void *vmn_rtmpThrFxn(void *arg);

#endif //__VIS_MMNP_RTMP_H__
