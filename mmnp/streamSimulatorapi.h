#ifndef ___STREAMSIMULATOR_API_H_
#define ___STREAMSIMULATOR_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef	WIN32
#ifdef LIBVISSTRESM_EXPORTS
#define LIBVIS_STRESM_API  __declspec(dllexport) 
#else	
#define LIBVIS_STRESM_API  __declspec(dllimport)   //VC 用
#endif

#else	//linux
#define LIBVIS_STRESM_API   //linux用
#endif


	enum _simm_error_{
		SSIM_ERROR_NOERROR=0,
		SSIM_ERROR_HANDLE,		//句柄有误
		SSIM_ERROR_INPUT,		//输入参数有误
		SSIM_ERROR_TOOEARLY,	//时间未到
		SSIM_ERROR_READOVER,	//文件读到结尾
		SSIM_ERROR_UNKNOWN,		//获取失败，原因未知
	};

	enum _ssim_gettype_{
		SSIM_TYPE_NEXTFRAME=0,
		SSIM_TYPE_SLICE=1,
		SSIM_TYPE_DPA =2,
		SSIM_TYPE_DPB =3,
		SSIM_TYPE_DPC =4,
		SSIM_TYPE_IDR =5,
		SSIM_TYPE_SEI =6,
		SSIM_TYPE_SPS =7,
		SSIM_TYPE_PPS=8,
		SSIM_TYPE_AUD =9,
		SSIM_TYPE_EOSEQ =10,
		SSIM_TYPE_EOSTREAM =11,
		SSIM_TYPE_FILL =12,
		SSIM_TYPE_AAC=13,
		SSIM_TYPE_MP3,
		SSIM_TYPE_PCM,
	};


	typedef struct _video_frame_param
	{
		int size;	//size of video_frame_param
		unsigned char *p_payload;	//buffer指针
		int i_payload;				//buffer大小，单位：字节
		int i_type;					//buffer类型 详见_ssim_gettype_
		int width;
		int height;
		int frameType;				//帧类型 0:未知 IDR帧=1 I帧=2 P帧=3 B帧=4 //REALDATA_VIDEO 有效
		unsigned int video_timestamp; //时间戳
	}video_frame_param;

	typedef struct _audio_frame_param
	{
		int size;	//size of audio_frame_param
		unsigned char *p_payload;	//buffer指针
		int i_payload;				//buffer大小，单位：字节
		int i_type;					//buffer类型 详见_ssim_gettype_
		int samplerate;
		int channels;
		int audiobits;
		unsigned int audio_timestamp;	//时间戳
	}audio_frame_param;

	typedef struct 
	{	
		int		size;//size of ssim_media_param

		int ts_simulatenable;	//内部模拟准确的时间戳，即时间不到不输出数据

		int video_encodeType;	//视频编码类型 0:未知 H264 = 1 or mpeg4 = 2 //当前默认为H264 //REALDATA_VIDEO 有效
		int video_framerate;	//视频帧率  可不输入 以video_timestamp为准
		int video_idr_devide;	//1: IDR帧拆分
		int video_circle;		//1: 循环获取视频
		int video_framegap_us;	//两帧视频的间隔 (us)

		int audio_encodeType;	//音频编码类型 0:未知 AAC=1 MP3=2 PCM=3 G729=10 G711=11	
		int	audio_bits;			//位宽 16bits or 8bits	//REALDATA_PCM 有效
		int audio_samplerate;	//采样率 8000Hz 16000Hz 44100Hz等等	//REALDATA_PCM 有效
		int audio_channels;		//声道数 单声道=1 or 双声道=2	//REALDATA_PCM 有效
		int audio_circle;		//1: 循环获取音频
		int audio_framegap_us;	//两帧视频的间隔 (us)
	}ssim_media_param;



	LIBVIS_STRESM_API void*	ssim_create(char *pvideo_filename,char *paudio_filename,ssim_media_param *sparam);		//输入视频和音频文件名
	LIBVIS_STRESM_API int ssim_getvideo( void *handle,int type, video_frame_param* vparam);	//handle和type为输入，param为输出
	LIBVIS_STRESM_API int ssim_getaudio( void *handle,int type, audio_frame_param* aparam);	//handle和type为输入，param为输出
	LIBVIS_STRESM_API int ssim_close( void *handle);

#ifdef __cplusplus
}
#endif

#endif	//end of ___STREAMSIMULATOR_API_H_
