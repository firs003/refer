/*
 * rtsp.cpp
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#include <sys/syscall.h>
#include "vis_os.h"

#include "rtsp.h"
#include "ringbuffer.h"
#include "vis_mmnp.h"

//Compile opt need to add -I/usr/local/include/BasicUsageEnvironment -I/usr/local/include/groupsock -I/usr/local/include/UsageEnvironment
#include "include/liveMedia.hh"
#include "include/BasicUsageEnvironment.hh"
#include "include/GroupsockHelper.hh"
//#include <../AACAudioMatroskaFileServerMediaSubsession.hh>
//#include "AACStreamerFramer.hh"

//#define USEC 1000000
//#define AVSYNC_AUDIO_LATE_DEBUG
#ifdef AVSYNC_AUDIO_LATE_DEBUG
#define SEC_AUDIO_TO_ADVANCE 1
#define USEC_AUDIO_TO_ADVANCE 100000
#endif

static int firstlink = 0;
//#define SINK_PORT 3030

//#define VIDEO_WIDTH 320
//#define VIDEO_HEIGHT 240
//#define FRAME_PER_SEC 200.0

typedef struct get_stream_env {
	UsageEnvironment *_env;
	int media_type;	//1-video, 2-audio, 0 or 3-both
	RingBuffer *pbuffer_video;
	RingBuffer *pbuffer_audio;
	int ringID_video;
	int ringID_audio;
	unsigned int *conn_num_ptr;
	pthread_mutex_t *conn_num_mutex;
} Get_Stream_Env;

typedef struct common_args {
	RingBuffer *pbuffer_video;
	RingBuffer *pbuffer_audio;
	unsigned int *audio_samplerate_ptr;
	unsigned int audio_samplerate_old;
	volatile int *quit_flag_ptr;
	unsigned int *conn_num_ptr;
	pthread_mutex_t *conn_num_mutex;
} Common_Args;

static pid_t gettid()
{
#ifdef	WIN32
	return 0;
#else
	return syscall(SYS_gettid);
#endif
}

static  void* ptask=0;
static  const int comsume_delay = 2000;   //us //linxj2014-08-07
static void getH264stream(void* ptr)
{
	struct get_stream_env *envp = 0;
	envp = (struct get_stream_env*)ptr;
	if(*envp->conn_num_ptr <= 0)
	{
		int buflen;
		char *buf;
		switch (envp->media_type) {
			case 1 :
			{
				int ringID_video = (envp->ringID_video==-1)? vis_ring_buffer_getthreadid(envp->pbuffer_video) : envp->ringID_video;
				buf = (char *)vis_ring_buffer_send_addr(envp->pbuffer_video, &buflen, NULL, NULL, NULL, ringID_video);
				if (envp->ringID_video == -1) vis_ring_buffer_unregthread(envp->pbuffer_video, ringID_video);
				break;
			}
			case 2 :
			{
				int ringID_audio = (envp->ringID_audio==-1)? vis_ring_buffer_getthreadid(envp->pbuffer_audio) : envp->ringID_audio;
				buf = (char *)vis_ring_buffer_send_addr(envp->pbuffer_audio, &buflen, NULL, NULL, NULL, ringID_audio);
				if (envp->ringID_audio == -1) vis_ring_buffer_unregthread(envp->pbuffer_audio, ringID_audio);
				break;
			}
			default :
			{
				int ringID_video = (envp->ringID_video==-1)? vis_ring_buffer_getthreadid(envp->pbuffer_video) : envp->ringID_video;
				int ringID_audio = (envp->ringID_audio==-1)? vis_ring_buffer_getthreadid(envp->pbuffer_audio) : envp->ringID_audio;
				buf = (char *)vis_ring_buffer_send_addr(envp->pbuffer_video, &buflen, NULL, NULL, NULL, ringID_video);
				buf = (char *)vis_ring_buffer_send_addr(envp->pbuffer_audio, &buflen, NULL, NULL, NULL, ringID_audio);
				if (envp->ringID_audio == -1) vis_ring_buffer_unregthread(envp->pbuffer_audio, ringID_audio);
				if (envp->ringID_video == -1) vis_ring_buffer_unregthread(envp->pbuffer_video, ringID_video);
				break;
			}
		}
		//int getvideoret=getvideobuf(&buf,&buflen);  //comsume the buffer of video //linxj 2012-07-26
		//int getaudioret=getaudiobuf(&buf,&buflen);  //comsume the buffer of video //linxj 2014-08-07
//		buf = (char *)vis_ring_buffer_send_addr(envp->pbuffer_audio, &buflen, NULL, NULL, NULL, envp->ringID_audio);
	}
	ptask = envp->_env->taskScheduler().scheduleDelayedTask(comsume_delay, getH264stream, ptr);
}

#if 0
class VisUsageEnvironment : public BasicUsageEnvironment	//lengshan, 2014-08-27
{
	RingBuffer *pbuffer_video;
	RingBuffer *pbuffer_audio;
	int ringID_video;
	int ringID_audio;

public:
	static VisUsageEnvironment *createNew(TaskScheduler &scheduler, RingBuffer *video_buffer, RingBuffer *audio_buffer)
	{
		return new VisUsageEnvironment(scheduler, video_buffer, audio_buffer);
	}

	static void close(VisUsageEnvironment *visenv)
	{
		delete visenv;
	}

	RingBuffer *getVideoBuffer()
	{
		return pbuffer_video;
	}

	RingBuffer *getAudioBuffer()
	{
		return pbuffer_audio;
	}

	int getVideoBufferID()
	{
		return ringID_video;
	}

	int getAudioBufferID()
	{
		return ringID_audio;
	}

protected:
	VisUsageEnvironment(TaskScheduler &scheduler, RingBuffer *video_buffer, RingBuffer *audio_buffer) : BasicUsageEnvironment(scheduler)
	{
		pbuffer_video = video_buffer;
		pbuffer_audio = audio_buffer;
		ringID_video = vis_ring_buffer_getthreadid(pbuffer_video);
		int reg_ret = vis_ring_buffer_regthread(pbuffer_video, ringID_video);
		Vmn_Log_Debug("regthread for video=%d, pbuffer_video[@%p]\n", reg_ret, pbuffer_video);
		ringID_audio = vis_ring_buffer_getthreadid(pbuffer_audio);
		ret _ret = vis_ring_buffer_regthread(pbuffer_audio, ringID_audio);
		Vmn_Log_Debug("regthread for audio=%d, pbuffer_audio[@%p]\n", reg_ret, pbuffer_audio);
	}

	~VisUsageEnvironment()
	{
		vis_ring_buffer_unregthread(pbuffer_video, ringID_video);
		vis_ring_buffer_unregthread(pbuffer_audio, ringID_audio);
		reclaim();
	}
}
#endif

// using webcam + dm365 codec
class WebcamFrameSource : public FramedSource
{
	double  fps;
	int m_started;
	void *mp_token;
	char* lastframebuf;
	int   lastframebuflen;
	int to_delay ;   //us 
	unsigned int durationus;
	RingBuffer *pbuffer;
	int ringID;
	volatile int *quit_flag_ptr;
	unsigned int *conn_num_ptr;
	pthread_mutex_t *conn_num_mutex;

public:
	WebcamFrameSource (UsageEnvironment &env,double videofps,struct common_args *pcommon)
		: FramedSource(env)
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);

		m_started = 0;
		mp_token = 0;
		lastframebuf=0;
		lastframebuflen=0;
		fps = videofps;
//		durationus = (unsigned int)(1000000.0/fps);
		to_delay = 30000;   //us = 30ms
		pbuffer = pcommon->pbuffer_video;
		ringID = vis_ring_buffer_getthreadid(pbuffer);
		int regret = vis_ring_buffer_regthread(pbuffer, ringID);
		Vmn_Log_Debug("fps=%f, durationus=%u, ret=%d, regist ID=%d to video buffer[@%p]\n", fps, durationus, regret, ringID, pbuffer);
		quit_flag_ptr = pcommon->quit_flag_ptr;
		conn_num_ptr = pcommon->conn_num_ptr;
		conn_num_mutex = pcommon->conn_num_mutex;
		//mp_token = envir().taskScheduler().scheduleDelayedTask(to_delay, getH264data, this);
	}

	~WebcamFrameSource ()
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		int unregret = vis_ring_buffer_unregthread(pbuffer, ringID);
		Vmn_Log_Debug("ret=%d, unregist ID=%d from video buffer[@%p]\n", unregret, ringID, pbuffer);

		if (mp_token) {
			envir().taskScheduler().unscheduleDelayedTask(mp_token);
		}

	}
	int SetFramerate(double videofps)
	{
		fps = videofps;
//		fFrameRate = videofps;
//		durationus = 1000000/fps;
		return 0;
	}

protected:
#if 0
	virtual void doGetNextFrame ()
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		if (m_started)
        {
		    fprintf(stderr, "[%d] %s aha,not ready!!! \n", gettid(), __func__);
            while(m_started)
            {
                sleep(5);
            }
		    fprintf(stderr, "[%d] %s yeah,ready go go go!!! \n", gettid(), __func__);
            return;
        }
		m_started = 1;

		// computing wait time depend on fps
		double delay = 1000.0 / fps;
		int to_delay = delay * 1000;	// us

		fprintf(stderr, "[%d] %s .... calling to_delay= %d (us)\n", gettid(), __func__,to_delay);
		mp_token = envir().taskScheduler().scheduleDelayedTask(to_delay,
				getNextFrame, this);
	}
#else
	virtual void doGetNextFrame ()
	{
		//fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
        int ret;
        ret = getNextFrame1();
        return ;

    }
#endif
	virtual unsigned maxFrameSize() const
	{
		//fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		return 512*1024;
		//return 140*1024;
		return 380*1024;
	}

	virtual void doGetH264data()
	{
		if(*conn_num_ptr <= 0)
		//if(*conn_num_ptr <= 0)
		{
			int buflen;
			char* buf;
			//getvideoret=getvideobuf(&buf,&buflen);  //comsume the buffer of video //linxj 2012-07-26
			buf = (char *)vis_ring_buffer_send_addr(pbuffer, &buflen, NULL, NULL, NULL, ringID);
		}
		fprintf(stderr, "[%d] %s .... calling to_delay= %d (us)\n", gettid(), __func__,to_delay);
		mp_token = envir().taskScheduler().scheduleDelayedTask(to_delay, getH264data, this);
    }
private:
	static void getNextFrame (void *ptr)
	{
		int ret=((WebcamFrameSource*)ptr)->getNextFrame1();
		if(ret!=0)
		{
		}
	}
	static void getH264data (void *ptr)
	{
		((WebcamFrameSource*)ptr)->doGetH264data();
	}

	int getNextFrame1 ()
	{
#if 0
		// capture:
		if (Capture_get(mp_capture, &hCapBuf) < 0) {
			fprintf(stderr, "==== %s: Capture_get\n", __func__);
			m_started = 0;
			return;
		}

		// compress
		if ((Venc1_process(mp_compress, hCapBuf, hDstBuf)) < 0) {
			fprintf(stderr, "==== %s: Venc1_process err\n", __func__);
			m_started = 0;
			return;
		}

		/* TODO: it is important.
		 * 1. save the frame size in variable "fFrameSize"
		 * 2. copy the encoded data to buffer "fTo", something like:
		       memcpy(fTo, "your buffer in hDstBuf", fFrameSize);
		 */
#endif
		//fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		char* buf=0;
		int buflen=0;
		//if(fNumTruncatedBytes==0)
		//lastframebuflen=0;
		if(*quit_flag_ptr)	//TODO
		{
			// The input source has ended:
			handleClosure(this);
			return -10;
		}
		if(lastframebuflen==0)
		{
#if 1
	            if(firstlink>0)
	            {
				//int tmp;
				//for(tmp=0;tmp<4;tmp++)
				//getvideoret=getvideobuf(&buf,&buflen);  //comsume the buffer of video //linxj 2012-07-26
				buf = (char *)vis_ring_buffer_send_addr(pbuffer, &buflen, NULL, (unsigned int *)&fPresentationTime.tv_sec, (unsigned int *)&fPresentationTime.tv_usec, ringID);
				firstlink ++ ;
				if(firstlink>6)
				firstlink = 0;
	            }
#endif
			//getvideoret=getvideobuf(&buf,&buflen);
			fPresentationTime.tv_sec = 0;
			fPresentationTime.tv_usec = 0;
			buf = (char *)vis_ring_buffer_send_addr(pbuffer, &buflen, NULL, (unsigned int *)&fPresentationTime.tv_sec, (unsigned int *)&fPresentationTime.tv_usec, ringID);
			//printf("getvideobuf(): ret=%d buf=0x%x buflen=%d \n",getvideoret,buf,buflen);
			if(buf==0||buflen<=0)
			{
				//fprintf(stderr,"getvideobuf()error:buf[@%p], buflen=%d \n",buf,buflen);
				// computing wait time depend on fps
				int to_delay = 5 * 1000;	// us

				m_started = 1;
				//fprintf(stderr, "[%d] %s .... calling to_delay= %d (us)\n", gettid(), __func__,to_delay);
				mp_token = envir().taskScheduler().scheduleDelayedTask(to_delay,getNextFrame, this);
				return -1;
			}
		} else {
			printf("Use lastfram buf \n");
			buf = lastframebuf;
			buflen=lastframebuflen;
		}

		fFrameSize = buflen;
		if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) gettimeofday(&fPresentationTime, 0);
//		fDurationInMicroseconds = durationus;   //need to check efficient?linxj2014-08-15
		if (fFrameSize > fMaxSize) {
	            printf("frameSize[%d] too large !! \n",fFrameSize);
			fNumTruncatedBytes = fFrameSize - fMaxSize;
			fFrameSize = fMaxSize;
	            lastframebuf=buf+fFrameSize;
	            lastframebuflen=fNumTruncatedBytes;
		} else {
			lastframebuf=0;
			lastframebuflen=0;
			fNumTruncatedBytes = 0;
		}
		memcpy(fTo,buf,fFrameSize);
		m_started = 0;
		// notify
		afterGetting(this);
		//printf("after afterGetting() \n");

		return 0;
	}
};

class VISH264VideoStreamFramer : public H264VideoStreamFramer
{
    //double  fps;

public:
	static VISH264VideoStreamFramer* createNew(UsageEnvironment& env, double videofps=25.0,FramedSource* inputSource=NULL, Boolean includeStartCodeInOutput = False)
	{
		return new VISH264VideoStreamFramer(env, videofps,inputSource, True, includeStartCodeInOutput);
	}

	VISH264VideoStreamFramer(UsageEnvironment &env, double videofps,FramedSource* inputSource, Boolean createParser, Boolean includeStartCodeInOutput)
		: H264VideoStreamFramer(env,inputSource, createParser, includeStartCodeInOutput)
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);

        fFrameRate = videofps;
        //*conn_num_ptr = 0;
        //*conn_num_ptr ++ ;
	}

	~VISH264VideoStreamFramer ()
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
        //*conn_num_ptr = 0;
	}
	int SetFramerate(double videofps)
	{
		fprintf(stderr, "[%d] %s .... calling fps=%f\n", gettid(), __func__,videofps);
		fFrameRate = videofps;
		((WebcamFrameSource*)fInputSource)->SetFramerate(videofps);
		return 0;
	}
};

class WebcamOndemandMediaSubsession : public OnDemandServerMediaSubsession
{
private:
	VISH264VideoStreamFramer *video_source;	// parent of WebcamFrameSource
	char *mp_sdp_line;
	char sdp_lines[512];
	RTPSink *mp_dummy_rtpsink;
	char m_done;
	int videofps;
	struct common_args *common;
	unsigned int *conn_num_ptr;
	pthread_mutex_t *conn_num_mutex;

public:
	static WebcamOndemandMediaSubsession *createNew (UsageEnvironment &env, int fps, struct common_args *pcommon)
	{
		return new WebcamOndemandMediaSubsession(env, fps, pcommon);
	}

    int SetFramerate(int fps)
    {
		fprintf(stderr, "median subsession[%d] %s .... calling fps=%d\n", gettid(), __func__,fps);
        videofps=fps;
        video_source->SetFramerate((double)videofps);
        return 0;
    }
protected:
	WebcamOndemandMediaSubsession (UsageEnvironment &env, int fps, struct common_args *pcommon)
		: OnDemandServerMediaSubsession(env, True) // reuse the first source
	{
		video_source = 0;
		videofps = fps;
		mp_sdp_line = 0;
		common = pcommon;
		conn_num_ptr = common->conn_num_ptr;
		conn_num_mutex = common->conn_num_mutex;
		pthread_mutex_lock(conn_num_mutex);
		*conn_num_ptr = 0;
		pthread_mutex_unlock(conn_num_mutex);
		fprintf(stderr, "[%d] %s .... calling, fps=%d\n", gettid(), __func__, fps);
	}

	~WebcamOndemandMediaSubsession ()
	{
		fprintf(stderr, "~[%d] %s .... calling\n", gettid(), __func__);
		//if (mp_sdp_line) free(mp_sdp_line);
	}
/*
private:
	static void afterPlayingDummy (void *ptr)
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		// ok
		WebcamOndemandMediaSubsession *This = (WebcamOndemandMediaSubsession*)ptr;
		This->m_done = 0xff;
	}

	static void chkForAuxSDPLine (void *ptr)
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		WebcamOndemandMediaSubsession *This = (WebcamOndemandMediaSubsession *)ptr;
		This->chkForAuxSDPLine1();
	}

	void chkForAuxSDPLine1 ()
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		if (mp_dummy_rtpsink->auxSDPLine())
			m_done = 0xff;
		else {
			int delay = 100*1000;	// 100ms
			nextTask() = envir().taskScheduler().scheduleDelayedTask(delay,
					chkForAuxSDPLine, this);
		}
	}
*/

protected:
    virtual void deleteStream(unsigned clientSessionId, void*& streamToken) //linxj 2012-06-09
    {
        //--;
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
        if(*conn_num_ptr > 0)
			pthread_mutex_lock(conn_num_mutex);
            (*conn_num_ptr) --;
			pthread_mutex_unlock(conn_num_mutex);
        OnDemandServerMediaSubsession::deleteStream(clientSessionId, streamToken);
    }
	virtual const char *sdpLines () //linxj 2012-06-09
    {
        //++;
        static unsigned int calltimes=0;
        calltimes++;
		fprintf(stderr, "[%d] %s .... calling times=%d\n", gettid(), __func__,calltimes);
        if((calltimes&1)==0)
        {
			pthread_mutex_lock(conn_num_mutex);
            (*conn_num_ptr) ++;    //call it twice if a session is setup //linxj 2012-06-09
			pthread_mutex_unlock(conn_num_mutex);
            if(*conn_num_ptr==1)
                firstlink = 1;          //first connect 
        }
        return OnDemandServerMediaSubsession::sdpLines();
    }
	virtual const char *getAuxSDPLine (RTPSink *sink, FramedSource *source)
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
        
		//printf("mp_sdp_line=0x%d \n",mp_sdp_line);
        //if (mp_sdp_line) return mp_sdp_line;
        const char* temp;
        temp = sink == NULL ? "" :  sink->auxSDPLine();
        if(temp==0) temp="";
        //sprintf(sdp_lines,"a=framerate:%d.00\r\n%s",videofps,temp);
        sprintf(sdp_lines,"%s",temp);
        mp_sdp_line = sdp_lines;
        printf("mp_sdp_line=%s ==end \n",mp_sdp_line);
#if 0
		mp_dummy_rtpsink = sink;
		mp_dummy_rtpsink->startPlaying(*source, 0, 0);
		//mp_dummy_rtpsink->startPlaying(*source, afterPlayingDummy, this);
		chkForAuxSDPLine(this);
		m_done = 0;
		envir().taskScheduler().doEventLoop(&m_done);
		mp_sdp_line = strdup(mp_dummy_rtpsink->auxSDPLine());
		mp_dummy_rtpsink->stopPlaying();
#endif
		return mp_sdp_line;
	}
protected:
	virtual RTPSink *createNewRTPSink(Groupsock *rtpsock, unsigned char type, FramedSource *source)
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
        OutPacketBuffer::maxSize = 512*1024; //increas to 512k linxj2014-08-08
		return H264VideoRTPSink::createNew(envir(), rtpsock, type);
	}

	virtual FramedSource *createNewStreamSource (unsigned sid, unsigned &bitrate)
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		bitrate = 10000;
		video_source = VISH264VideoStreamFramer::createNew(envir(),videofps, new WebcamFrameSource(envir(),videofps,common));
        return video_source;
		//return H264VideoStreamFramer::createNew(envir(), new WebcamFrameSource(envir()));
	}

};



static unsigned const samplingFrequencyTable[16] = {
  96000, 88200, 64000, 48000,
  44100, 32000, 24000, 22050,
  16000, 12000, 11025, 8000,
  7350, 0, 0, 0
};

class ADTSAudioStreamerSource: public FramedSource
{
    //double  fps;
	int m_started;
	void *mp_token;
	char* lastframebuf;
	int fchannel,fsample;
	unsigned int durationus;
	RingBuffer *pbuffer;
	int ringID;
	volatile int *quit_flag_ptr;
	unsigned int *samplerate_ptr;
	
public:
	ADTSAudioStreamerSource (UsageEnvironment &env,int channel,int sample,struct common_args *pcommon)
		: FramedSource(env)
	{
		fchannel=channel;
		fsample=sample;
		durationus = (1024*1000000)/fsample;
		pbuffer = pcommon->pbuffer_audio;
		ringID = vis_ring_buffer_getthreadid(pbuffer);
		int regret = vis_ring_buffer_regthread(pbuffer, ringID);
		Vmn_Log_Debug("sample=%d, durationus=%u, ret=%d, regist ID=%d to audio buffer[@%p]\n", sample, durationus, regret, ringID, pbuffer);
		quit_flag_ptr = pcommon->quit_flag_ptr;
		samplerate_ptr = pcommon->audio_samplerate_ptr;
		printf("Audio[%d] %s .... calling samplerate=%d, ringID=%d in audio buffer[@%p]\n", gettid(), __func__,sample,ringID,pbuffer);
#if 0
    // Now, having opened the input file, read the fixed header of the first frame,
    // to get the audio stream's parameters:
    unsigned char fixedHeader[4]={0xff,0xf9,0x50,0x80}; // it's actually 3.5 bytes long

    // Check the 'syncword':
    if (!(fixedHeader[0] == 0xFF && (fixedHeader[1]&0xF0) == 0xF0)) {
      env.setResultMsg("Bad 'syncword' at start of ADTS file");
      break;
    }

    // Get and check the 'profile':
    u_int8_t profile = (fixedHeader[2]&0xC0)>>6; // 2 bits //0x50 profile=1 
    if (profile == 3) {
      env.setResultMsg("Bad (reserved) 'profile': 3 in first frame of ADTS file");
      break;
    }

    // Get and check the 'sampling_frequency_index':
    u_int8_t sampling_frequency_index = (fixedHeader[2]&0x3C)>>2; // 4 bits 
	//index 44.1->4  32k->5 16k->8 8k->11
    if (samplingFrequencyTable[sampling_frequency_index] == 0) {
      env.setResultMsg("Bad 'sampling_frequency_index' in first frame of ADTS file");
      break;
    }

    // Get and check the 'channel_configuration':
    u_int8_t channel_configuration
      = ((fixedHeader[2]&0x01)<<2)|((fixedHeader[3]&0xC0)>>6); // 3 bits

  fSamplingFrequency = samplingFrequencyTable[samplingFrequencyIndex];
  fNumChannels = channelConfiguration == 0 ? 2 : channelConfiguration;
  fuSecsPerFrame
    = (1024/*samples-per-frame*/*1000000) / fSamplingFrequency/*samples-per-second*/;

  // Construct the 'AudioSpecificConfig', and from it, the corresponding ASCII string:
  unsigned char audioSpecificConfig[2];
  u_int8_t const audioObjectType = profile + 1;
  audioSpecificConfig[0] = (audioObjectType<<3) | (samplingFrequencyIndex>>1);
  audioSpecificConfig[1] = (samplingFrequencyIndex<<7) | (channelConfiguration<<3);
  sprintf(fConfigStr, "%02X%02x", audioSpecificConfig[0], audioSpecificConfig[1]);
#endif

	}

	~ADTSAudioStreamerSource ()
	{
		fprintf(stderr, "[%d] %s .... calling\n", gettid(), __func__);
		int unregret = vis_ring_buffer_unregthread(pbuffer, ringID);
		Vmn_Log_Debug("ret=%d, unregist ID=%d from audio buffer[@%p]\n", unregret, ringID, pbuffer);
		if (m_started) {
			envir().taskScheduler().unscheduleDelayedTask(mp_token);
		}

	}

	virtual void doGetNextFrame ()
	{
		//fprintf(stderr, "A[%d] %s .... calling\n", gettid(), __func__);
		int ret;
		ret = getNextFrame1();
		return ;
	}

private:
	static void getNextFrame (void *ptr)
	{
		int ret=((ADTSAudioStreamerSource*)ptr)->getNextFrame1();
		if(ret!=0)
		{
		}
	}

	int getNextFrame1 ()
	{
		//fprintf(stderr, "A[%d] %s .... calling\n", gettid(), __func__);
		char* buf=0;
		int buflen=0;
		if(*quit_flag_ptr)	//TODO
		{
			// The input source has ended:
			handleClosure(this);
			return -10;
		}
		if (fsample != (int)*samplerate_ptr) {
			Vmn_Log_Debug("samplerate has been changed (%d -> %d)\n", fsample, (int)*samplerate_ptr);
			fsample = (int)*samplerate_ptr;
			durationus = (1024*1000000)/fsample;
		}
		fPresentationTime.tv_sec = 0;
		fPresentationTime.tv_usec = 0;
		buf = (char *)vis_ring_buffer_send_addr(pbuffer, &buflen, NULL, (unsigned int *)&fPresentationTime.tv_sec, (unsigned int *)&fPresentationTime.tv_usec, ringID);
		// if (buf && buflen) Vmn_Log_Debug("rtsp.cpp: get audio data[@%p], datalen=%d\n", buf, buflen);
        	// printf("Streamer:getaudiobuf(): ret=%d buf=0x%x buflen=%d \n",getaudioret,buf,buflen);
		if(buf==0||buflen<=0)
		{
           		 //fprintf(stderr,"getaudiobuf()error:buf=%p, buflen=%d \n",buf,buflen);
			 // computing wait time depend on fps
			int to_delay = 1 * 1000;	// us
			m_started = 1;
			//fprintf(stderr, "[%d] %s .... calling to_delay= %d (us)\n", gettid(), __func__,to_delay);
			mp_token = envir().taskScheduler().scheduleDelayedTask(to_delay,getNextFrame, this);
			return -1;
		}
		//printf("audiobuf[0] = 0x%x ,audiobuf[1] = 0x%x\n",((int*)buf)[0],((int*)buf)[1]);

		// Begin by reading the 7-byte fixed_variable headers:
		unsigned char* headers=(unsigned char*)buf;

		// Extract important fields from the headers:
		int headerlen=7;
		Boolean protection_absent = headers[1]&0x01;


		// If there's a 'crc_check' field, skip it:
		if (!protection_absent) {
			headerlen+=2;
		}
		buf+=headerlen;
		buflen-=headerlen;
		if(buflen<0) buflen=0;	//
		fFrameSize = buflen;
		if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) gettimeofday(&fPresentationTime, 0);
#ifdef AVSYNC_AUDIO_LATE_DEBUG
		if (fPresentationTime.tv_usec < USEC_AUDIO_TO_ADVANCE) {
			fPresentationTime.tv_usec += 1000000;
			fPresentationTime.tv_sec -= 1;
		}
		fPresentationTime.tv_usec -= USEC_AUDIO_TO_ADVANCE;
		fPresentationTime.tv_sec -= SEC_AUDIO_TO_ADVANCE;
#endif
		if (fFrameSize > fMaxSize) {
			printf("Audio frameSize[%d] too large !! \n",fFrameSize);
			fNumTruncatedBytes = fFrameSize - fMaxSize;
			fFrameSize = fMaxSize;
		}
		else {
			fNumTruncatedBytes = 0;
		}
		memcpy(fTo,buf,fFrameSize);

		fDurationInMicroseconds = durationus;   //need to check efficient?linxj2012-05-18
	//	fDurationInMicroseconds = 0;   //need to check efficient?linxj2012-05-18

		m_started = 0;
		// notify
		afterGetting(this);
		//printf("after afterGetting() \n");
		// Switch to another task, and inform the reader that he has data:
		//nextTask() = envir().taskScheduler().scheduleDelayedTask(0,(TaskFunc*)FramedSource::afterGetting, this);

		return 0;
	}
};
/* RTP:Audio class */

class WebcamOndemandAudioSubsession : public OnDemandServerMediaSubsession
{
	FramedSource *mp_source;	// parent of WebcamFrameSource
	unsigned int  samplerate,channel,uSecsPerFrame;
	struct common_args *common;

public:
	static WebcamOndemandAudioSubsession *createNew (UsageEnvironment &env, int audiosamplerate,int audiochannel,struct common_args *pcommon)
	{
		return new WebcamOndemandAudioSubsession(env,audiosamplerate,audiochannel,pcommon);
	}
protected:
	WebcamOndemandAudioSubsession (UsageEnvironment &env, int audiosamplerate,int audiochannel,struct common_args *pcommon)
		: OnDemandServerMediaSubsession(env, True) // reuse the first source
	{
		fprintf(stderr, "Audio[%d] %s .... calling\n", gettid(), __func__);
		common = pcommon;
		if(common->audio_samplerate_old != *common->audio_samplerate_ptr)
		{
			common->audio_samplerate_old = samplerate = *common->audio_samplerate_ptr;
			printf("[%s]rtsp get new samplerate=%d \n",__func__,samplerate);
		}
		printf("[%s] samplerate=%d ,channels=%d .... calling\n",  __func__,audiosamplerate,audiochannel);
		samplerate = audiosamplerate;
		channel = audiochannel;
		//uSecsPerFrame  = (1024*1000000) / samplerate;

	}

	~WebcamOndemandAudioSubsession ()
	{
		fprintf(stderr, "~A[%d] %s .... calling\n", gettid(), __func__);
	}


protected:

	virtual const char *sdpLines () //linxj 2012-06-09
	{
	//linxj 2014-08-08
		//unsigned int calltimes=0;
		//setSDPLinesFromRTPSink(dummyRTPSink, inputSource, estBitrate);
		if (fSDPLines == NULL)
			printf("[%s]:First time!! \n",__func__);
		else
		{ 
			delete[] fSDPLines;
			fSDPLines = NULL;
			printf("[%s]:Delete fSDPLines \n",__func__);
		}
		return OnDemandServerMediaSubsession::sdpLines();
	}
	virtual RTPSink *createNewRTPSink(Groupsock *rtpsock, unsigned char type, FramedSource *source)
	{
		//fprintf(stderr, "A[%d] %s .... calling\n", gettid(), __func__);
		char configstr[6];
//		OutPacketBuffer::maxSize = 2*1024; //enough?
		OutPacketBuffer::maxSize = 512*1024; //enough?
		// Construct the 'AudioSpecificConfig', and from it, the corresponding ASCII string:
		unsigned char audioSpecificConfig[2];
		unsigned char profile=1;
		unsigned char const audioObjectType = profile + 1;
		unsigned char samplingFrequencyIndex,channelConfiguration=channel;
		if(common->audio_samplerate_old != *common->audio_samplerate_ptr)
		{
			common->audio_samplerate_old = samplerate = *common->audio_samplerate_ptr;
			printf("[%s]rtsp get new samplerate=%d \n",__func__,samplerate);
		}
		printf( "[%d] %s samplerate=%d\n", gettid(), __func__,samplerate);
		
		for(samplingFrequencyIndex=0;samplingFrequencyIndex<sizeof(samplingFrequencyTable)/sizeof(unsigned);samplingFrequencyIndex++)
		{
		    if(samplingFrequencyTable[samplingFrequencyIndex]==samplerate)
		        break ;
		}
		audioSpecificConfig[0] = (audioObjectType<<3) | (samplingFrequencyIndex>>1);
		audioSpecificConfig[1] = (samplingFrequencyIndex<<7) | (channelConfiguration<<3);
		sprintf(configstr, "%02X%02x", audioSpecificConfig[0], audioSpecificConfig[1]);
		//"1210"=44.1k+2channel
		fprintf(stderr, "Audio[%d] %s configstr=%s \n", gettid(), __func__,configstr);
        
		return MPEG4GenericRTPSink::createNew(envir(), rtpsock,
		type,//rtpPayloadTypeIfDynamic,
		samplerate,
		"audio", "AAC-hbr", configstr,  
		channel);
	}

	virtual FramedSource *createNewStreamSource (unsigned sid, unsigned &bitrate)
	{
		bitrate = 192;
		if(common->audio_samplerate_old != *common->audio_samplerate_ptr)
		{
			common->audio_samplerate_old = samplerate = *common->audio_samplerate_ptr;
			printf("[%s]rtsp get new samplerate=%d \n",__func__,samplerate);
		}
		return new ADTSAudioStreamerSource(envir(),channel,samplerate,common);

#if 0
        char* fMode = "AAC-hbr",*fMediumName1="audio";
        Groupsock *fRTPSocket;
        unsigned char fRTPPayloadFormat=0;
        unsigned fRTPTimestampFrequency=44100,fSizelength=1596,fIndexlength=0,fIndexdeltalength=0;
        return AACStreamerFramer::createNew(envir(),new WebcamAudioSource(envir()),channel,samplerate);

        return MPEG1or2AudioStreamFramer::createNew(envir(),new WebcamAudioSource(envir()));
        return new WebcamAudioSource(envir());
        return WebcamAudioSource::createNew(envir(),NULL);
        
        MPEG4GenericRTPSource::create
        return MPEG4GenericRTPSource::createNewStreamSource(sid, bitrate);
        return MPEG4GenericRTPSource::createNew( envir(), fRTPSocket, fRTPPayloadFormat,  
                                               fRTPTimestampFrequency,(const char*) fMediumName1, (const char*)fMode, fSizelength,  
                                                                                        fIndexlength, fIndexdeltalength);
        return AACAudioMatroskaFileServerMediaSubsession::createNewStreamSource(sid, bitrate);
#endif
	}
};





#ifdef  __cplusplus
extern "C"
{
#endif
 WebcamOndemandMediaSubsession* videoSubsession=0;
 int changeFramerate(int fps)
 {
     if(videoSubsession)
     {
         return videoSubsession->SetFramerate(fps);
     }
     return -1;
 }
/******************************************************************************
 * vmn_rtspThrFxn
 ******************************************************************************/
void *vmn_rtspThrFxn(void *arg)
{
	void				*status		= THREAD_SUCCESS;
	RtspEnv				*envp		= (RtspEnv *)arg;
	unsigned short		 rtspport	= 8554;
	struct get_stream_env gsEnv;
//	int ringID_video=-1, ringID_audio=-1;
	UsageEnvironment *_env = 0;
	WebcamFrameSource *webcam_source = 0;
	//ADTSAudioStreamerSource *webcam_audiosource = 0;
	ServerMediaSession *sms = 0;
	TaskScheduler *scheduler = 0;
	RTSPServer *rtspServer = 0;
	struct common_args common;

	printf("%s:threadid=%ld\n",__func__, (long)gettid());
	if (envp==NULL || envp->quit_flag_ptr==NULL || envp->pbuffer_video==NULL || envp->pbuffer_audio==NULL) {
		Vmn_Log_Error("Invalid Params, envp[@%p], quit_flag_ptr[@%p], pbuffer_video[@%p], pbuffer_audio[@%p]", envp, envp->quit_flag_ptr, envp->pbuffer_video, envp->pbuffer_audio);	//not exatly right
		cleanup(THREAD_FAILURE);
	}

	scheduler = BasicTaskScheduler::createNew();
	_env = BasicUsageEnvironment::createNew(*scheduler);
	if(envp->port>1024) rtspport = envp->port;
	rtspServer = RTSPServer::createNew(*_env, rtspport);    //default is 8554
	if (!rtspServer) {
		fprintf(stderr, "ERR: create RTSPServer err\n");
		*_env << "Failed to crate RTSP server "<< _env->getResultMsg() <<"\n";
		cleanup(THREAD_FAILURE);
	}
#ifdef AVSYNC_AUDIO_LATE_DEBUG
	Vmn_Log_Debug("rtsp.cpp: port=%d, audio_advanced=%dms\n", rtspport, SEC_AUDIO_TO_ADVANCE*1000+USEC_AUDIO_TO_ADVANCE/1000);
#endif
//	Vmn_Log_Debug("rtsp.cpp: before add serverMediaSession, video ID[%d] in buffer[@%p], audio ID[%d] in buffer[@%p]\n", ringID_video, envp->pbuffer_video, ringID_audio, envp->pbuffer_audio);

	// add live stream
	//do
	{
		memset(&common, 0, sizeof(common));
		common.pbuffer_video = envp->pbuffer_video;
		common.pbuffer_audio = envp->pbuffer_audio;
		common.audio_samplerate_ptr = envp->audio_samplerate_ptr;
		common.audio_samplerate_old = *common.audio_samplerate_ptr;
		common.quit_flag_ptr = envp->quit_flag_ptr;
		common.conn_num_ptr = envp->conn_num_ptr;
		common.conn_num_mutex = envp->conn_num_mutex;
      	sms = ServerMediaSession::createNew(*_env, "webcam", 0, "Session from /dev/video0"); 
		if(envp->audioEnable)
		{
			printf("add audio subsession!! \n");
			//sms->addSubsession(ADTSAudioFileServerMediaSubsession::createNew(*_env, "test.aac", false));
			sms->addSubsession(WebcamOndemandAudioSubsession::createNew(*_env, *envp->audio_samplerate_ptr, (int)envp->audioChannel, &common));
		}
		if(envp->videoEnable)
		{
			printf("add video subsession!! \n");
			videoSubsession = WebcamOndemandMediaSubsession::createNew(*_env, (int)envp->videofps, &common);
			sms->addSubsession(videoSubsession);
		}
		rtspServer->addServerMediaSession(sms);

		char *url = rtspServer->rtspURL(sms);
		*_env << "using url \"" << url << "\"\n";
		delete [] url;
	} 
	//while (0);

	/* Signal that initialization is done and wait for other threads */
//	Rendezvous_meet(envp->hRendezvousInit);
	printf("rtsp.cpp: before main loop! quit_flag = %d \n", *envp->quit_flag_ptr);
//    if(!*envp->quit_flag_ptr())
	try
	{
		memset(&gsEnv, 0, sizeof(gsEnv));
		gsEnv._env = _env;
		gsEnv.media_type = 0;
		gsEnv.ringID_video = -1;
		gsEnv.ringID_audio = -1;
		gsEnv.pbuffer_video = envp->pbuffer_video;
		gsEnv.pbuffer_audio = envp->pbuffer_audio;
		gsEnv.conn_num_ptr = envp->conn_num_ptr;
		gsEnv.conn_num_mutex = envp->conn_num_mutex;
		ptask = _env->taskScheduler().scheduleDelayedTask(comsume_delay, getH264stream, (void *)&gsEnv);
      	printf("rtsp.cpp: doEventloop! quit_flag = %d \n", *envp->quit_flag_ptr);
	    
      	_env->taskScheduler().doEventLoop((char *)envp->quit_flag_ptr);
      	//_env->taskScheduler().doEventLoop();
    
		Vmn_Log_Debug("rtsp.cpp:Exit event loop... \n");

		if (ptask) {
			_env->taskScheduler().unscheduleDelayedTask(ptask);
		}
	}
	catch(...)//(int throwval)
	{
		int throwval=0;
		printf("[rtsp.cpp]Get throw val=%d \n",throwval);
	}

cleanup:
	/* Make sure the other threads aren't waiting for us */
	if (envp) {
		*envp->quit_flag_ptr = 1;

		/* Meet up with other threads before cleaning up */
		Vmn_Log_Debug("after meet\n");
		if(0)
		{
			int i;
			for(i=0;i<4;i++)
			{
				usleep(250000);
				printf("sleep times=%d \n",i+1);
			}
		}

	    /* Clean up the thread before exiting */
	    if (webcam_source) {
		    delete webcam_source;
	    }
		if(sms)
			printf("sms refrence count =%d \n",sms->referenceCount());
	#if 0
	    if(sms){
			rtspServer->removeServerMediaSession(sms);
	        delete [] sms;
	    }
	    Vmn_Log_Debug("after delete sms\n");

		{
			int i;
			for(i=0;i<4;i++)
			{
				usleep(250000);
				printf("[test2]sleep times=%d \n",i+1);
			}
		}
	    if(rtspServer){
	        //delete rtspServer;
	        //RTSPServer::close(rtspServer);
	        Medium::close(rtspServer);
	    }
	#else
	    if(rtspServer){
	        Medium::close(rtspServer);
	    }
	#endif
	    Vmn_Log_Debug("after close rtspServer\n");

		if(0)
		{
			int i;
			for(i=0;i<4;i++)
			{
				usleep(250000);
				printf("[test3]sleep times=%d \n",i+1);
			}
		}
		if(_env){
	        //delete _env;
	        //VisUsageEnvironment::close(_env);
			int reclaimval=0;
			reclaimval =  _env->reclaim();
			printf("rtsp.cpp _env->reclaimval = %d \n",reclaimval);
		}
		Vmn_Log_Debug("after env->reclaim\n");
		if(0)
		{
			int i;
			for(i=0;i<4;i++)
			{
				usleep(250000);
				printf("[test4]sleep times=%d \n",i+1);
			}
		}

		if(scheduler){
			delete scheduler;
		}

		if(0)
		{
			int i;
			for(i=0;i<6;i++)
			{
				usleep(250000);
				printf("[test5]sleep times=%d \n",i+1);
			}
		}
	} //if(envp)

    return status;
}
#ifdef  __cplusplus
}
#endif
