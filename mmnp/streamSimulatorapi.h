#ifndef ___STREAMSIMULATOR_API_H_
#define ___STREAMSIMULATOR_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef	WIN32
#ifdef LIBVISSTRESM_EXPORTS
#define LIBVIS_STRESM_API  __declspec(dllexport) 
#else	
#define LIBVIS_STRESM_API  __declspec(dllimport)   //VC ��
#endif

#else	//linux
#define LIBVIS_STRESM_API   //linux��
#endif


	enum _simm_error_{
		SSIM_ERROR_NOERROR=0,
		SSIM_ERROR_HANDLE,		//�������
		SSIM_ERROR_INPUT,		//�����������
		SSIM_ERROR_TOOEARLY,	//ʱ��δ��
		SSIM_ERROR_READOVER,	//�ļ�������β
		SSIM_ERROR_UNKNOWN,		//��ȡʧ�ܣ�ԭ��δ֪
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
		unsigned char *p_payload;	//bufferָ��
		int i_payload;				//buffer��С����λ���ֽ�
		int i_type;					//buffer���� ���_ssim_gettype_
		int width;
		int height;
		int frameType;				//֡���� 0:δ֪ IDR֡=1 I֡=2 P֡=3 B֡=4 //REALDATA_VIDEO ��Ч
		unsigned int video_timestamp; //ʱ���
	}video_frame_param;

	typedef struct _audio_frame_param
	{
		int size;	//size of audio_frame_param
		unsigned char *p_payload;	//bufferָ��
		int i_payload;				//buffer��С����λ���ֽ�
		int i_type;					//buffer���� ���_ssim_gettype_
		int samplerate;
		int channels;
		int audiobits;
		unsigned int audio_timestamp;	//ʱ���
	}audio_frame_param;

	typedef struct 
	{	
		int		size;//size of ssim_media_param

		int ts_simulatenable;	//�ڲ�ģ��׼ȷ��ʱ�������ʱ�䲻�����������

		int video_encodeType;	//��Ƶ�������� 0:δ֪ H264 = 1 or mpeg4 = 2 //��ǰĬ��ΪH264 //REALDATA_VIDEO ��Ч
		int video_framerate;	//��Ƶ֡��  �ɲ����� ��video_timestampΪ׼
		int video_idr_devide;	//1: IDR֡���
		int video_circle;		//1: ѭ����ȡ��Ƶ
		int video_framegap_us;	//��֡��Ƶ�ļ�� (us)

		int audio_encodeType;	//��Ƶ�������� 0:δ֪ AAC=1 MP3=2 PCM=3 G729=10 G711=11	
		int	audio_bits;			//λ�� 16bits or 8bits	//REALDATA_PCM ��Ч
		int audio_samplerate;	//������ 8000Hz 16000Hz 44100Hz�ȵ�	//REALDATA_PCM ��Ч
		int audio_channels;		//������ ������=1 or ˫����=2	//REALDATA_PCM ��Ч
		int audio_circle;		//1: ѭ����ȡ��Ƶ
		int audio_framegap_us;	//��֡��Ƶ�ļ�� (us)
	}ssim_media_param;



	LIBVIS_STRESM_API void*	ssim_create(char *pvideo_filename,char *paudio_filename,ssim_media_param *sparam);		//������Ƶ����Ƶ�ļ���
	LIBVIS_STRESM_API int ssim_getvideo( void *handle,int type, video_frame_param* vparam);	//handle��typeΪ���룬paramΪ���
	LIBVIS_STRESM_API int ssim_getaudio( void *handle,int type, audio_frame_param* aparam);	//handle��typeΪ���룬paramΪ���
	LIBVIS_STRESM_API int ssim_close( void *handle);

#ifdef __cplusplus
}
#endif

#endif	//end of ___STREAMSIMULATOR_API_H_
