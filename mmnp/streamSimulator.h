#ifndef ___STREAMSIMULATOR_H_
#define ___STREAMSIMULATOR_H_

#include "streamSimulatorapi.h"



	typedef struct _sSimulator
	{
		char syn[4];	//SSIM
		void* pvf;		//视频文件句柄	//用void*代替FILE* 保证linux的兼容性
		void* paf;		//音频文件句柄
		int vbuf_start,vbuf_end,vbuf_len,vsrc_len,vframe_num;
		int abuf_start,abuf_end,abuf_len,asrc_len,aframe_num;
		unsigned char videofile_buf[1024*1024*4];
		unsigned char audiofile_buf[1024*256];
		unsigned char nal_buf[1024*1024];		//一帧视频
		unsigned char audio_buf[1024*128];		//linxj:一帧音频 128k足够
		int		videofile_readover;
		int		audiofile_readover;

		int	timestampe_enable;	//时间戳模拟 使能
		
		int  video_iEnable;	//是否有视频
		int video_timewrite;//是否在获取视频是写入时间戳
		unsigned int video_timestamp;	//时间戳信息，单位毫秒(ms)
		int video_tsenable;	//视频外部时间戳有效位 0:无效 使用系统时钟  1:有效 使用video_timestamp
		int video_encodeType;	//视频编码类型 0:未知 H264 = 1 or mpeg4 = 2 //当前默认为H264 //REALDATA_VIDEO 有效
		int video_framerate;	//视频帧率  可不输入 以video_timestamp为准
		int video_framegap_us;	//每帧视频获取间隔 ,<=0表示不需要时间间隔
		int video_circle;		//1: 循环获取视频
		int video_idr_devide; //1: IDR帧拆分 0:IDR帧不拆分
#ifndef WIN32
		unsigned long long video_gettime;//上一次getvideo的时间
#else
		unsigned int video_gettime;//上一次getvideo的时间
#endif
		int	video_width;
		int video_heigth;

		int audio_iEnable;	//是否有音频
		int audio_timewrite;//是否在获取音频是写入时间戳
		unsigned int audio_timestamp;	//时间戳信息，单位毫秒(ms)
		int audio_tsenable;	//音频外部时间戳有效位 0:无效 使用系统时钟  1:有效 使用audio_timestamp
		int	audio_bits;		//位宽 16bits or 8bits	//REALDATA_PCM 有效
		int audio_samplerate;	//采样率 8000Hz 16000Hz 44100Hz等等	//REALDATA_PCM 有效
		int audio_channels;	//声道数 单声道=1 or 双声道=2	//REALDATA_PCM 有效
		int audio_encodeType;	//音频编码类型 0:未知 AAC=1 MP3=2 G729=10 G711=11	//REALDATA_AUDIO 有效
		int audio_framegap_us;		//每帧音频获取间隔 ,<=0表示不需要时间间隔
		int audio_circle;		//1: 循环获取音频
#ifndef WIN32
		unsigned long long audio_gettime;//上一次getvideo的时间
#else
		unsigned int audio_gettime;//上一次getvideo的时间
#endif

	}sSimulator;

#endif	//end of ___STREAMSIMULATOR_H_
