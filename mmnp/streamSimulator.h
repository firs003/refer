#ifndef ___STREAMSIMULATOR_H_
#define ___STREAMSIMULATOR_H_

#include "streamSimulatorapi.h"



	typedef struct _sSimulator
	{
		char syn[4];	//SSIM
		void* pvf;		//��Ƶ�ļ����	//��void*����FILE* ��֤linux�ļ�����
		void* paf;		//��Ƶ�ļ����
		int vbuf_start,vbuf_end,vbuf_len,vsrc_len,vframe_num;
		int abuf_start,abuf_end,abuf_len,asrc_len,aframe_num;
		unsigned char videofile_buf[1024*1024*4];
		unsigned char audiofile_buf[1024*256];
		unsigned char nal_buf[1024*1024];		//һ֡��Ƶ
		unsigned char audio_buf[1024*128];		//linxj:һ֡��Ƶ 128k�㹻
		int		videofile_readover;
		int		audiofile_readover;

		int	timestampe_enable;	//ʱ���ģ�� ʹ��
		
		int  video_iEnable;	//�Ƿ�����Ƶ
		int video_timewrite;//�Ƿ��ڻ�ȡ��Ƶ��д��ʱ���
		unsigned int video_timestamp;	//ʱ�����Ϣ����λ����(ms)
		int video_tsenable;	//��Ƶ�ⲿʱ�����Чλ 0:��Ч ʹ��ϵͳʱ��  1:��Ч ʹ��video_timestamp
		int video_encodeType;	//��Ƶ�������� 0:δ֪ H264 = 1 or mpeg4 = 2 //��ǰĬ��ΪH264 //REALDATA_VIDEO ��Ч
		int video_framerate;	//��Ƶ֡��  �ɲ����� ��video_timestampΪ׼
		int video_framegap_us;	//ÿ֡��Ƶ��ȡ��� ,<=0��ʾ����Ҫʱ����
		int video_circle;		//1: ѭ����ȡ��Ƶ
		int video_idr_devide; //1: IDR֡��� 0:IDR֡�����
#ifndef WIN32
		unsigned long long video_gettime;//��һ��getvideo��ʱ��
#else
		unsigned int video_gettime;//��һ��getvideo��ʱ��
#endif
		int	video_width;
		int video_heigth;

		int audio_iEnable;	//�Ƿ�����Ƶ
		int audio_timewrite;//�Ƿ��ڻ�ȡ��Ƶ��д��ʱ���
		unsigned int audio_timestamp;	//ʱ�����Ϣ����λ����(ms)
		int audio_tsenable;	//��Ƶ�ⲿʱ�����Чλ 0:��Ч ʹ��ϵͳʱ��  1:��Ч ʹ��audio_timestamp
		int	audio_bits;		//λ�� 16bits or 8bits	//REALDATA_PCM ��Ч
		int audio_samplerate;	//������ 8000Hz 16000Hz 44100Hz�ȵ�	//REALDATA_PCM ��Ч
		int audio_channels;	//������ ������=1 or ˫����=2	//REALDATA_PCM ��Ч
		int audio_encodeType;	//��Ƶ�������� 0:δ֪ AAC=1 MP3=2 G729=10 G711=11	//REALDATA_AUDIO ��Ч
		int audio_framegap_us;		//ÿ֡��Ƶ��ȡ��� ,<=0��ʾ����Ҫʱ����
		int audio_circle;		//1: ѭ����ȡ��Ƶ
#ifndef WIN32
		unsigned long long audio_gettime;//��һ��getvideo��ʱ��
#else
		unsigned int audio_gettime;//��һ��getvideo��ʱ��
#endif

	}sSimulator;

#endif	//end of ___STREAMSIMULATOR_H_
