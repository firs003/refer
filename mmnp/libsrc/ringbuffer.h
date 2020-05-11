#ifndef	__VIS_RING_BUFFER_H__
#define __VIS_RING_BUFFER_H__

#ifdef __cplusplus
extern "C"
{
#endif

//for encode
#include "vis_mmnp.h"
//extern struct vis_mmnp_avdata;
//extern enum vis_mmnp_packet_type;

#define BUFFER_LENGTH	(1024*1024+1024*512)
#define	BUFFER_SEND_BUFLEN	(0x4c0000)	//发送缓区大小
#define FRAME_MAX		    (0x40000)
#define EXTENDED_LEN	FRAME_MAX	//to make sure the decode buffer is continuous
#define MAX_RESERVE		(FRAME_MAX/4)
#define MAXNUM_BUFFERHEADER 128
#define MAX_THREAD      8

typedef struct buffer_header {
	char  	       syn[4];		//4个字节的同步信号	"vish"
	unsigned char *pdata;		//有效负载的指针
	unsigned int   datalen;		//有效负载的长度
	unsigned int   bufno;   	//缓冲的编号
	unsigned int   sec;
	unsigned int   usec;
	unsigned char  tcpflag[MAX_THREAD];     //表示是否已完成TCP发送 0:no 1:yes
	unsigned char  validflag;		        //该缓冲数据是否有效 0:no 1:yes
} BufferHeader, *pBufferHeader;

enum infotype {
	INFO_TYPE_AUDIO_G729=0,
	INFO_TYPE_AUDIO_PCM,
	INFO_TYPE_AUDIO_MP3,
	INFO_TYPE_USER,
	INFO_TYPE_SETTING
};

//接收数据包时使用
typedef	struct info_packet_header {
	int headerlen;
	int len;
	int type;	//see INFO_TYPE
} InfoHeader;

typedef struct vis_buffer_send {
	void* mybase;
	int   size;
	unsigned int   counter_buffer;		            //发送缓冲的帧数，一直累加
	unsigned int   counter_tcpsender[MAX_THREAD];	// tcp发送的帧数，一直累加
	int  index_buffer;		        //for putdata, 0-127
	int  index_sender[MAX_THREAD];	//threads' sender index, 0-127
} Vis_Buffer_Send;

typedef struct vis_buffer_recv {
	void* mybase;
	int   size;
	//int   recvcounter;
	unsigned int   counter_buffer;		//接收缓冲的帧数
	unsigned int   counter_parser;		//已处理的缓冲帧数
	int  index_buffer;		//for recv
	int  index_parser;		//for parse
	//unsigned char* recvbuf;
	//unsigned char* parserbuf;
} Vis_Buffer_Recv;

typedef struct vis_buffer_status {
    char num_videoleft; //未发送的video帧
    char num_audioleft; //未发送的audio帧
    char num_vbufleft;  //可用video buffer个数, for encode
    char num_abufleft;  //可用audio buffer个数, for encode
    char num_connect;   //TCP连接个数
} Vis_Buffer_Status;

typedef struct ring_buffer {
	char	   	  rbsyn[4];		//ringBuffer4个字节的同步信号 "viss"(发送)或"visr"(接受)
	int 		  rbbits;		//ringBuffer确认比特位，固定为0x1e3c5a96,PCI主机可通过此来判断该ringBuffer是否已经初始化	
	unsigned char *pbuffer;
	unsigned char *pstart;
	unsigned char *pend;
	unsigned char *pextended;
	int            bufferlen;
	int            data_len;
	//for encode and sender
	unsigned char *pencode;
	unsigned char *psend;
	//for recv and decoder;
	unsigned char *precv;
	unsigned char *pdecode;
	int (*packet_callback)(unsigned char *, struct vis_mmnp_avdata *);
//	int packet_callback();	//use in vis_ringbuffer_putdate(), when divide frame into different packet for different protocol

	Vis_Buffer_Send vsender;
	Vis_Buffer_Send asender;
	Vis_Buffer_Recv recver;
	BufferHeader    vbufheader[MAXNUM_BUFFERHEADER];
	BufferHeader    abufheader[MAXNUM_BUFFERHEADER];
    char            sendthread[MAX_THREAD];
    Vis_Buffer_Status status;
//	sem_t sem_vis_ring_buffer_putdata;
//	sem_t sem_vis_ring_buffer_sendaddr;
	pthread_mutex_t mutex_vis_ring_buffer_putdata;
	pthread_mutex_t mutex_vis_ring_buffer_sendaddr;
} RingBuffer, *pRingBuffer;

//extern pRingBuffer pbuffer;

//API
void* vis_ring_buffer_send_init(size_t bufsize, enum vis_mmnp_packet_type packet_type, int (*packet_custom)(unsigned char *, struct vis_mmnp_avdata *));
void* vis_ring_buffer_recv_init();
int vis_ring_buffer_close(pRingBuffer pbuffer);
int vis_ring_buffer_getthreadid(pRingBuffer pbuffer);

//
int vis_ring_buffer_recv(pRingBuffer pbuffer,unsigned char* buf,int len);	//recv数据接收时调用
void* vis_ring_buffer_parse(pRingBuffer pbuffer,int* len,int* type);		//parse处理已接收的数据

//给编码和发送使用的
int vis_ring_buffer_putdata(pRingBuffer pbuffer,struct vis_mmnp_avdata *avdata);	//编码后的数据放入缓冲区  	
unsigned char* vis_ring_buffer_send_addr(pRingBuffer pbuffer,int *len, int *data_type, unsigned int *sec, unsigned int *usec, int thread);	//获取需要发送的地址和长度	//UDP发送时使用//PCI发送的话，应该有PCI主机来做类似操作 
int vis_ring_buffer_send_status(pRingBuffer pbuffer);	//获取发送缓存的状态，以便做相应处理
void* vis_ring_buffer_recvfrom(pRingBuffer pbuffer,int* len,int* type);
int vis_ring_buffer_regthread(pRingBuffer pbuffer, int index);
int vis_ring_buffer_unregthread(pRingBuffer pbuffer, int index);


#ifdef __cplusplus
}
#endif

#endif //end of __VIS_RING_BUFFER_H__
