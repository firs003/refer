/********************************************************************
* tspacket.h: 
*********************************************************************
*	Copyright (C) 2005 viscodec (http://www.viscodec.com)
*
*	Version:	3.0
*	Authors:	linxj	<zerolinxj@hotmail.com>
*	Tel:		(+86) 21-64950313
*
********************************************************************/

#ifndef	_TSPACKET_H_
#define _TSPACKET_H_

#ifdef __cplusplus
extern "C"
{
#endif
	
typedef enum stream_flag {
		FLAG_SPEECH=1,
		FLAG_VIDEO=2,
		FLAG_VIDEO_IDR=4,
		FLAG_KEY=8
}stream_flag;
//������ȡֵ stream type��0x80-0xff Ϊ�û�˽�е�ֵ	
typedef enum stream_type {
	STREAM_TYPE_MPEG2=0x02,
	STREAM_TYPE_MPEG4=0x10,
	STREAM_TYPE_H264=0x1B,
	STREAM_TYPE_MP3=0x03,
	STREAM_TYPE_AAC=0x0f,   //sp 02-26-2010
	STREAM_TYPE_G729 =0x88,	//G729Ϊ�û�˽�еģ���ͨ�ñ�׼��stream type
	STREAM_TYPE_PCM  =0x89,	//PCMΪ�û�˽�еģ���ͨ�ñ�׼��stream type
	STREAM_TYPE_G711 =0x8A,	//G711Ϊ�û�˽�еģ���ͨ�ñ�׼��stream type
	STREAM_TYPE_USER1=0x98,	//�û��Զ�������1
	STREAM_TYPE_USER2=0x99	//�û��Զ�������2
}STREAM_TYPE;

#define CHANNEL_MAXNUM		2		//֧�ֵ�����Ŀ����
#define ELEMENTARY_MAXNUM	3		//ÿ����Ŀ֧�ֵ������������Ŀ:��Ƶ+��Ƶ+�Զ�������

typedef struct vists_config {
	int size;
	int PMT_num;								//��Ŀ������������Ŀӳ���ĸ���
	int PMT_PID[CHANNEL_MAXNUM];				//ÿ����Ŀӳ����PIDֵ 0-8191
	int PMT_elementary_num[CHANNEL_MAXNUM];		//ÿ����Ŀ��ES���ĸ�����ӦС��ELEMENTARY_MAXNUM
	int PMT_es_streamType[CHANNEL_MAXNUM][ELEMENTARY_MAXNUM];	//ÿ����Ŀ��ÿ��ES����stream type(����������)�����STREAM_TYPE
	int PMT_es_PID[CHANNEL_MAXNUM][ELEMENTARY_MAXNUM];			//ÿ����Ŀ��ÿ��ES����PID	
}VisTS_config,*pVisTS_config;

typedef struct vists_params {
	int size;									//��ʾ�ýṹ��Ĵ�С
	pVisTS_config config;						//���ò���ָ��
	//input params
	int	  channel_id;							//�����ͨ��ID��
	unsigned char * pBuffer_for_Access_Unit;	//���������ĵ�ַ
	int   length_of_Access_Unit;				//���������ĳ���
	int   stream_Type;							//�������������ͣ�H264 MPEG4 MP3 G729 �Զ������ݵ�,���STREAM_TYPE
	unsigned int  second;						//����ʱ�䣺��(s)
	unsigned int  usecond;						//����ʱ�䣺����(us)
	unsigned char * tspackets;					//TS����Ĵ洢��ַ TS address
	stream_flag flag;							//�����ͱ�־,����ƵIDR֡
	//output params
	int  OutputTSLength;						//�����TS��������
}VisTS_params;
	
#define TS_LENGTH	188
#define TS_NUM		7
#define RTP_LENGTH	((TS_LENGTH*TS_NUM)+12)

int VisTS_init(VisTS_params* params);			//��������ʼ��

int VisTS_package(VisTS_params* params);		//TS���


#ifdef __cplusplus
};
#endif

#endif				
