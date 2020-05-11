#ifndef ___RTMPSENDAPI_H_
#define ___RTMPSENDAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _RTMP_ERROR{
	RTMP_ERROR_NOERROR=0,			//没有错误
	RTMP_ERROR_INPUTPARAM,			//输入参数不正确
	RTMP_ERROR_HANDLE_ILLEGAL,		//非法的句柄
	RTMP_ERROR_PARAMSIZE,			//输入的参数结构体大小不正确
	RTMP_ERROR_PARAMTYPE,			//输入的参数类型未知
	RTMP_ERROR_CONNECT,			//RTMP连接错误
	RTMP_ERROR_SEND,					//RTMP传送错误
	RTMP_ERROR_WRITEFRAME,		//RTMP准备发送数据时出错
	RTMP_ERROR_MEMORY,				//内存操作失败，如内存分配malloc、内存释放free等等

}RTMP_ERROR;	

typedef enum _RTMP_BUF_TYPE{
	BUF_TYPE_UNKOWN=0,			//未知类型
	BUF_TYPE_H264,				//视频H264，不区分各种NAL
	BUF_TYPE_H264_SPS,			//视频H264的SPS帧
	BUF_TYPE_H264_PPS,			//视频H264的PPS帧
	BUF_TYPE_H264_IDR,			//视频H264的IDR帧，一般是指包含了SPS和PPS的IDR
	BUF_TYPE_H264_IFRAME,		//视频H264的I帧
	BUF_TYPE_H264_PFRAME,		//视频H264的P帧
	BUF_TYPE_AAC,				//音频AAC，一般是指包含ADTS头信息
	BUF_TYPE_AAC_ADTS,			//音频AAC，包含ADTS头信息
	BUF_TYPE_AAC_RAW,			//音频AAC，不包含ADTS头信息
	BUF_TYPE_HYBRID,			//视频+音频
}RTMP_BUF_TYPE;

typedef	struct _rtmp_write_param{
	int		size;			//表示该结构体的大小
	char*	buffer;			//数据缓冲区指针
	int		bufsize;		//帧大小
	int		buftype;		//帧类型参考RTMP_BUF_TYPE;

	int  	video_iEnable;	//该结构体内的信息是否有效 //REALDATA_VIDEO or REALDATA_YUV 有效
	unsigned int video_timestamp;	//时间戳信息，单位毫秒(ms)
	int video_tsenable;	//视频外部时间戳有效位 0:无效 使用系统时钟  1:有效 使用video_timestampe
	int video_width;		//视频宽度	//REALDATA_YUV 有效
	int video_height;		//视频长度  //REALDATA_YUV 有效
	int video_encodeType;	//视频编码类型 0:未知 H264 = 1 or mpeg4 = 2 //当前默认为H264 //REALDATA_VIDEO 有效
	int video_frameType;	//帧类型 0:未知 IDR帧=1 I帧=2 P帧=3 B帧=4 //REALDATA_VIDEO 有效
	int video_naltype;		//参考typedef enum _RTMP_H264NALUTYPE
	int video_framerate;	//视频帧率  可不输入 以video_timestampe为准
	int video_framepersec;//视频每帧持续时间  可不输入 以video_timestampe为准
	int video_framegap;	//未使用  可不输入 以video_timestampe为准

	int audio_iEnable;	//该结构体内的信息是否有效 //REALDATA_AUDIO or REALDATA_PCM 有效
	unsigned int audio_timestamp;	//时间戳信息，单位毫秒(ms)
	int audio_tsenable;	//音频外部时间戳有效位 0:无效 使用系统时钟  1:有效 使用audio_timestampe
	int	audio_bits;		//位宽 16bits or 8bits	//REALDATA_PCM 有效
	int audio_samplerate;	//采样率 8000Hz 16000Hz 44100Hz等等	//REALDATA_PCM 有效
	int audio_channels;	//声道数 单声道=1 or 双声道=2	//REALDATA_PCM 有效
	int audio_encodeType;	//音频编码类型 0:未知 AAC=1 MP3=2 G729=10 G711=11	//REALDATA_AUDIO 有效
	int audio_framerate;	//音频帧率   可不输入 以audio_timestampe为准
	int audio_framepersec;//音频每帧持续时间  可不输入 以audio_timestampe为准
	int audio_framegap;	//未使用  可不输入 以audio_timestampe为准

	unsigned char *pstreambuf;  //待发送数据
	int streamsize; //待发送数据大小

	unsigned char *pheadbuf;
	int headsize;
}rtmp_write_param;

//rtmpaddress为将要连接的RTMP地址 。若rtmpaddress==0，则默认使用“rtmp://127.0.0.1:1935"
void* rtmpsender_create(char *rtmpaddress,int rcvtimeout,int sendtimeout );

//使用RTMP协议传送一帧的音频或视频数据
int rtmpsender_write(void *handle, rtmp_write_param *param);

int rtmpsender_close(void *handle);

int rtmpsender_reconnect(void *handle);

#ifdef __cplusplus
}
#endif

#endif //end of ___RTMPSENAPI_H_
