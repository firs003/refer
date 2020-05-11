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
//流类型取值 stream type：0x80-0xff 为用户私有的值	
typedef enum stream_type {
	STREAM_TYPE_MPEG2=0x02,
	STREAM_TYPE_MPEG4=0x10,
	STREAM_TYPE_H264=0x1B,
	STREAM_TYPE_MP3=0x03,
	STREAM_TYPE_AAC=0x0f,   //sp 02-26-2010
	STREAM_TYPE_G729 =0x88,	//G729为用户私有的，非通用标准的stream type
	STREAM_TYPE_PCM  =0x89,	//PCM为用户私有的，非通用标准的stream type
	STREAM_TYPE_G711 =0x8A,	//G711为用户私有的，非通用标准的stream type
	STREAM_TYPE_USER1=0x98,	//用户自定义类型1
	STREAM_TYPE_USER2=0x99	//用户自定义类型2
}STREAM_TYPE;

#define CHANNEL_MAXNUM		2		//支持的最大节目数量
#define ELEMENTARY_MAXNUM	3		//每个节目支持的最大数据流数目:视频+音频+自定义数据

typedef struct vists_config {
	int size;
	int PMT_num;								//节目的数量，即节目映射表的个数
	int PMT_PID[CHANNEL_MAXNUM];				//每个节目映射表的PID值 0-8191
	int PMT_elementary_num[CHANNEL_MAXNUM];		//每个节目中ES流的个数，应小于ELEMENTARY_MAXNUM
	int PMT_es_streamType[CHANNEL_MAXNUM][ELEMENTARY_MAXNUM];	//每个节目中每个ES流的stream type(数据流类型)，详见STREAM_TYPE
	int PMT_es_PID[CHANNEL_MAXNUM][ELEMENTARY_MAXNUM];			//每个节目中每个ES流的PID	
}VisTS_config,*pVisTS_config;

typedef struct vists_params {
	int size;									//表示该结构体的大小
	pVisTS_config config;						//配置参数指针
	//input params
	int	  channel_id;							//输入的通道ID号
	unsigned char * pBuffer_for_Access_Unit;	//输入码流的地址
	int   length_of_Access_Unit;				//输入码流的长度
	int   stream_Type;							//输入码流的类型：H264 MPEG4 MP3 G729 自定义数据等,详见STREAM_TYPE
	unsigned int  second;						//输入时间：秒(s)
	unsigned int  usecond;						//输入时间：μ秒(us)
	unsigned char * tspackets;					//TS输出的存储地址 TS address
	stream_flag flag;							//流类型标志,如视频IDR帧
	//output params
	int  OutputTSLength;						//输出的TS码流长度
}VisTS_params;
	
#define TS_LENGTH	188
#define TS_NUM		7
#define RTP_LENGTH	((TS_LENGTH*TS_NUM)+12)

int VisTS_init(VisTS_params* params);			//打包程序初始化

int VisTS_package(VisTS_params* params);		//TS打包


#ifdef __cplusplus
};
#endif

#endif				
