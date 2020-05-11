#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "streamSimulatorapi.h"


#include "./libsrc/vis_mmnp_api.h"

//#define MULTITEST 1//多路测试标志
#define	FILE_READLOOP

#define TEST_SRC_PORT 16511
#define TEST_DST_PORT 16522

//测试类型选择
//#define TEST_TCP
#define TEST_C1
//#define TEST_RTSP
//#define TEST_BROADCAST
//#define TEST_MULTICAST
//#define TEST_UNICAST
//#define TEST_VOD

#ifdef WIN32
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"../pthreadVC2.lib")
#pragma comment(lib,"../tspacket.lib")
//#pragma comment (linker,"/NODEFAULTLIB:libc.lib")
//#pragma comment (linker,"/NODEFAULTLIB:MSVCRT.lib")
#pragma comment (linker,"/NODEFAULTLIB:LIBCMT.lib")
#if 0
#pragma comment(lib,"librtmpsender.lib")
#endif

#else //linux
#define Sleep(ms) usleep((ms)*1000)
#endif


typedef struct input_param {
	char audiofile[128];
	char videofile[128];
	char flvfile[128];
	char url[128];
}input_param;

static int parseArgs(int argc, char* argv[], input_param *param);

volatile int is_done = 0;  

/**
  handle the ctrl+c event.
*/
#ifdef WIN32
//#define WINAPI      __stdcall
#include <winsock2.h>
//#define WINAPI      
int WINAPI Handler( DWORD ctrl_event )
{
	switch( ctrl_event )
	{
	case CTRL_CLOSE_EVENT:
	case CTRL_C_EVENT:
		{
			/* shutdown the server */
			is_done = 1;
			printf(  "Get ctrl+C event handler.\n" );
		}

		return 1;
	}

	return 0;
}

#define WS_VERSION_CHOICE1 0x202/*MAKEWORD(2,2)*/
#define WS_VERSION_CHOICE2 0x101/*MAKEWORD(1,1)*/
int initializeWinsockIfNecessary(void) 
{
/* We need to call an initialization routine before
* we can do anything with winsock.  
	*/
	static int _haveInitializedWinsock = 0;
	WSADATA	wsadata;
	
	if (!_haveInitializedWinsock) 
	{
		if ((WSAStartup(WS_VERSION_CHOICE1, &wsadata) != 0)
			&& ((WSAStartup(WS_VERSION_CHOICE2, &wsadata)) != 0)) 
		{
			return 0; /* error in initialization */
		}
		if ((wsadata.wVersion != WS_VERSION_CHOICE1)
			&& (wsadata.wVersion != WS_VERSION_CHOICE2)) 
		{
			WSACleanup();
			return 0; /* desired Winsock version was not available */
		}
		_haveInitializedWinsock = 1;
	}
	
	return 1;
}
#else
#include   <unistd.h>
#define	Sleep(ms) usleep((ms)*1000)

#include <signal.h>
void signal_handle(int signo) {
	switch (signo) {
		case SIGINT :
			/* shutdown the server */
			is_done = 1;
			printf(  "Get ctrl+C event handler.\n" );
			break;
		default :
			break;
	}
}
int initializeWinsockIfNecessary(void) 
{
	return 0;
}
#endif

int main(int argc, char* argv[])
{
	int     sleepcount;
	int		exitflag=0;
	int		tmp;
	void*	psim, *pvismmnp;
	ssim_media_param sparam;
	unsigned int framecount;

	struct vis_mmnp_attrs vmnattrs;
#ifdef MULTITEST
	void*	prtmp1;
#endif
	int		havevideo,haveaudio;
	
	input_param config;
	video_frame_param vparam;
	audio_frame_param aparam;
	struct vis_mmnp_avdata avdata;
	const int videoframegap_ms=40,audioframegap_ms=24;
	
#ifdef WIN32	//若有ctrl + C 的处理函数
	if( !SetConsoleCtrlHandler( Handler, TRUE ) )
	{
		fprintf( stderr, "error setting event handler.\n" );
		return -1;
	}
#else
	signal(SIGINT, signal_handle);
#endif

	initializeWinsockIfNecessary();

	memset(&config,0,sizeof(config));
	memset(&vparam,0,sizeof(vparam));
	memset(&aparam,0,sizeof(aparam));
	memset(&vmnattrs,0,sizeof(vmnattrs));
	memset(&avdata,0,sizeof(avdata));
	
	if(parseArgs(argc,argv,&config)!=0)
		return 0;
	if(strlen(config.videofile) <=0 && strlen(config.audiofile)<=0)
	{
		strcpy(config.videofile,"../test.264") ;
		printf("[default]: Use default video file =%s \n",config.videofile);
	}
	if(strlen(config.url) <=0)
	{
		//strcpy(config.url,"rtmp://127.0.0.1:1935/live/test") ;
		//strcpy(config.url,"rtmp://192.168.18.162:1935/live/test") ;
		strcpy(config.url,"rtmp://192.168.18.168:1935/live/test") ;
		printf("[default]: Use default url =%s \n",config.url);
	}
	
	havevideo=haveaudio=0;
	if(config.videofile[0]!=0)
	{
		havevideo =1;
	}
	if(config.audiofile[0]!=0)
	{
		haveaudio =1;
	}

	exitflag=0;


	memset(&sparam,0,sizeof(sparam));
	sparam.size=sizeof(sparam);
#ifdef	FILE_READLOOP
	sparam.video_circle=1;
	sparam.audio_circle=1;
#endif

	sparam.video_framegap_us=videoframegap_ms*1000;
	sparam.audio_framegap_us=audioframegap_ms*1000;
	//sparam.video_idr_devide=1;
	sparam.ts_simulatenable=1;

	psim=ssim_create(havevideo?config.videofile:0,haveaudio?config.audiofile:0,&sparam);
	if(!psim) {
		printf("simulator create failed \n");
		exitflag=1;
	}

	vmnattrs.size = sizeof(struct vis_mmnp_attrs);
#ifdef TEST_TCP
	/* TCP test */
	vmnattrs.network_type = NETWORK_SEND_TCP;
	vmnattrs.src_port = TEST_SRC_PORT;	//1234 is to small for pc
#endif
#ifdef TEST_C1
	/* C1 test */
	vmnattrs.network_type = NETWORK_SEND_C1;
	vmnattrs.src_port = TEST_SRC_PORT;	//1234 is to small for pc
#endif
#ifdef TEST_RTSP
	/* RTSP test */
	vmnattrs.network_type = NETWORK_SEND_RTSP;
	vmnattrs.src_port = 0;
	vmnattrs.video_enable = 1;
	vmnattrs.audio_enable = 0;
	vmnattrs.video_fps = 25;
#endif
#ifdef TEST_BROADCAST
	/* UNICAST test */
	vmnattrs.network_type = NETWORK_SEND_BROADCAST;
	vmnattrs.src_port = TEST_SRC_PORT;	//1234 is to small for pc
	strncpy(vmnattrs.dst_ip, "192.168.18.255", strlen("192.168.18.255"));
	vmnattrs.dst_port = TEST_DST_PORT;
#endif
#ifdef TEST_UNICAST
	/* UNICAST test */
	vmnattrs.network_type = NETWORK_SEND_UNICAST;
	vmnattrs.src_port = TEST_SRC_PORT;	//1234 is to small for pc
	strncpy(vmnattrs.dst_ip, "127.0.0.1", strlen("127.0.0.1"));
	vmnattrs.dst_port = TEST_DST_PORT;
#endif
#ifdef TEST_VOD
	/* VOD test */
	vmnattrs.network_type = NETWORK_SEND_VOD;
	vmnattrs.src_port = 0;	//1234 is to small for pc
	strncpy(vmnattrs.dst_ip, "0.0.0.0", strlen("0.0.0.0"));
	vmnattrs.dst_port = TEST_DST_PORT;
#endif
//	vmnattrs.url = config.url;
	strncpy(vmnattrs.url, config.url, strlen(config.url));
	vmnattrs.data_transmit_mode = 0;	//deviece put data to lib positive
	pvismmnp = vis_mmnp_create(&vmnattrs);
	if(!pvismmnp) {
		printf("vis_mmnp handler create failed \n");
		exitflag=1;
	}
	tmp=vis_mmnp_start(pvismmnp);
	if(tmp)
	{
		printf("vis_mmnp_start failed \n");
		exitflag=1;
	}

#ifdef MULTITEST
	prtmp1=rtmpsender_create("rtmp://127.0.0.1:1935/live/test1");
	if(!prtmp1)
		exitflag=1;
#endif

	sleepcount=0;
//	avdata.size = sizeof(avdata);
//	avdata.media.video.video_width = ;
//	avdata.media.video.video_height = ;
//	avdata.media.video.video_compress = ;
//	avdata.media.video.video_framerate = ;
//	avdata.media.audio.audio_bits = ;
//	avdata.media.audio.audio_channels = ;
//	avdata.media.audio.audio_compress = ;
//	avdata.media.audio.audio_samplerate = ;
//	avdata.media.audio.audio_framerate = ;
	framecount = 0;
#if 0	//test
	if(pvismmnp) {
		vis_mmnp_stop(pvismmnp);
		printf("after stop\n");
		vis_mmnp_close(pvismmnp);
		pvismmnp = NULL;
		printf("after close\n");
	}
#endif
//	int i_count = 0;
	while(exitflag==0 && is_done==0)
	{
		int videotimeout=1,audiotimeout=1;
		if(sparam.ts_simulatenable==0)
		{
			//if no timestamp, use sleepcount to make sure order of video and video
			sleepcount++;
			if(sleepcount==100000)
				sleepcount=0;
		}
		//if((havevideo)&&(sleepcount%40==0))
		if(havevideo)
		{
			if(sparam.ts_simulatenable==0)
			{
				printf("to get video when sleepcount= %d \n",sleepcount); //for debug info
			}
			videotimeout=0;
			memset(&vparam,0,sizeof(vparam));	//linxj2014-04-18
			vparam.size=sizeof(vparam);
//			printf("0-1 main.c:before get_video, frame_count=%u\n", framecount);
			tmp = ssim_getvideo(psim,0,&vparam);
			if(tmp==0)
			{
//				printf("0-2 main.c:after get_video, tmp=%d\n", tmp);
				if(vparam.p_payload==0||vparam.i_payload<0)
				{
					exitflag = 3;
					printf("ssim_getvideo have no video data \n");
					break;
				}
				memset(&avdata,0,sizeof(avdata));
				avdata.size = sizeof(avdata);
#if 0
				if (vparam.i_type==1) {
					++i_count;
					printf("\t");
				} else {
					i_count=0;
				}
				printf("%d.i_type=%d, len=%d\n", i_count, vparam.i_type, vparam.i_payload);
#endif
				avdata.type	= (vparam.i_type==7)?VIS_MMNP_DATA_TYPE_H264_IDR:VIS_MMNP_DATA_TYPE_H264_PFRAME;
				avdata.data		= vparam.p_payload;
				avdata.datalen	= vparam.i_payload;
//				avdata.media.video.video_iEnable = 1;
				avdata.media.video.video_frametype= vparam.i_type;	//可能需要转换	

#ifdef WIN32
				//vparam.video_timestamp=clock();
#endif
				avdata.timestamp.sec = vparam.video_timestamp/1000;
				avdata.timestamp.usec= (vparam.video_timestamp%1000)*1000;
//				printf("2.video timestampe=%d.%03d\n", avdata.timestamp.sec,avdata.timestamp.usec*1000);
//				printf("2.video timestampe=%ds %dus\n", avdata.timestamp.sec,avdata.timestamp.usec);
//				printf("2.before vis_mmnp_putdata_tolib(), len=%d\n", avdata.datalen);
//				printf("\t0-3 main.c:before video put\n");
				tmp = vis_mmnp_putdata_tolib(pvismmnp, &avdata);
//				printf("0-4 main.c:after video put, ret=%d, frame_count=%u\n", tmp, framecount);
#ifdef MULTITEST
				tmp=rtmpsender_write(prtmp1,&avdata);
#endif

				if(tmp < 0)
				{
					exitflag = 2;
					printf("vis_mmnp_putdata_tolib video failed =%d\n",tmp);
				}
//				printf("2.before vis_mmnp_putdata_tolib(), len=%d\n", avdata.datalen);
			}else if(tmp==SSIM_ERROR_TOOEARLY)//时间未到
			{
				//添加处理
				//sleep or not
				videotimeout=1;  
			}
			else
			{
				exitflag = 2;
				printf("ssim_getvideo failed =%d\n",tmp);
			}
		}//	end of if(havevideo)	
		
		if((haveaudio)&&(sleepcount%24==0))
		//if((haveaudio)
		{
			if(sparam.ts_simulatenable==0)
			{
				printf("to get audio when sleepcount= %d \n",sleepcount);	//for debug info
			}
			memset(&aparam,0,sizeof(aparam));	//linxj2014-04-18
			aparam.size=sizeof(aparam);
			tmp = ssim_getaudio(psim,0,&aparam);
			if(tmp==0)
			{
				if(aparam.p_payload==0||aparam.i_payload<0)
				{
					exitflag = 3;
					printf("ssim_getaudio have no audio data\n");
					break;
				}
				memset(&avdata,0,sizeof(avdata));
				avdata.size = sizeof(avdata);
				avdata.type	= VIS_MMNP_DATA_TYPE_AAC;
				avdata.data		= aparam.p_payload;
				avdata.datalen	= aparam.i_payload;
//				avdata.media.audio.audio_iEnable = 1;
				avdata.media.audio.audio_bits=aparam.audiobits;
				avdata.media.audio.audio_samplerate=aparam.samplerate;
				avdata.media.audio.audio_channels=aparam.channels;
#ifdef WIN32
				//avdata.audio_timestampe=clock();
#endif
				avdata.timestamp.sec = aparam.audio_timestamp/1000;
				avdata.timestamp.usec= (aparam.audio_timestamp%1000)*1000;
//				printf("3. audio timestampe=%ds %dus\n", avdata.timestamp.sec,avdata.timestamp.usec);
				//printf("3. avdata.media.audio.audio_samplerate=%d\n", avdata.media.audio.audio_samplerate);
//				printf("3.before vis_mmnp_putdata_tolib(), len=%d\n", avdata.datalen);
				printf("2.1.audio before put\n");
				tmp = vis_mmnp_putdata_tolib(pvismmnp, &avdata);
				printf("2.2.audio before put\n");
				if(tmp < 0)
				{
					exitflag = 2;
					printf("vis_mmnp_putdata_tolib audio failed =%d\n",tmp);
				}
			//	avdata.audio_timestampe=avdata.audio_timestampe+0x18;
			}else if(tmp==SSIM_ERROR_TOOEARLY)//时间未到
			{
				//添加处理
				//sleep or not
				audiotimeout=1;  
			}
			else
			{
				exitflag = 2;
				printf("ssim_getaudio failed =%d\n",tmp);
			}
		}//	end of if(haveaudio)

		if(sparam.ts_simulatenable==1)
		{
			if(videotimeout&&audiotimeout)
				Sleep(1);		//Sleep 1ms //if no videodata and no audiodata
		}
		++framecount;
//		printf("frame_count=%u\n", framecount);
	}//while(exitflag==0)

	if(psim)
		ssim_close(psim);

	if(pvismmnp) {
		vis_mmnp_stop(pvismmnp);
		vis_mmnp_close(pvismmnp);
	}
		
#ifdef MULTITEST
	if(prtmp1)
		rtmpsender_close(prtmp1);
#endif

	printf("All done, framecount=%u!\n", framecount);
	return 0;
}


/******************************************************************************
* usage
******************************************************************************/
static void usage(void)
{
    printf("Usage: client [options]\n\n"
		"Options:\n"
		"-v       Specify the video file name.\n"
		"-a       Specify the audio file name.\n"
		"-f       Specify the flv file name\n"
		"-r       Specify the rtmp address\n"
		"-h       Print this message\n\n"
        "This program is u-pdated on 2014-02-24 by linxj.\n"
		"\n\n");
}
#define ERR printf
#define DBG printf
static int parseArgs(int argc, char* argv[], input_param *param)
{
	char c;
	int  index;
	int  status = 0;

	for (index=1;index<argc;index++)
	{
		//c = getopt_long(argc, argv, shortOp, longOp, &index);
		if(argv[index][0]!='-')
		{
			continue ;
			//break ;
		}
		c = (int) argv[index][1];
		index++;
		switch (c) {
		case 0: 
			break;
		case 'a':
			strcpy(param->audiofile,argv[index]);
			DBG("Set audio file :\t%s \n", param->audiofile);
			break;
		case 'v':
			strcpy(param->videofile,argv[index]);
			DBG("Set video file :\t%s \n", param->videofile);
			break;
		case 'f':
			strcpy(param->flvfile,argv[index]);
			DBG("Set flv file :\t%s \n", param->flvfile);
			break;
		case 'r':
			//strcpy(param->rtmpaddress,argv[index]);
			//DBG("Set rtmp address :\t%s \n", param->rtmpaddress);
			break;
		case 'h':
			usage();
			exit(0);
		default:
			usage();
			exit(0);
		}
	}

	
	return status;
}
