#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "vis_mmnp.h"
#include "vis_mmnp_api.h"
#include "ringbuffer.h"
#include "packet.h"
#include "TSpacket.h"

#define	DEBUG_RING_BUFFER
#define THREAD_SEND_INDEPEND
#define ALIGN 128	//should be pow2

#if 0
//使用0x81000000以后的SDRAM内存，这样可以不时能CE1的cache映射，确保PCI访问的数据一致性
#define	BUFFER_SEND_HANDLE			0x81000000	//发送缓冲句柄所在的地址
#define	BUFFER_SEND_BUFFER_START	0x81001000	//发送缓存保存数据的开始地址
#define	BUFFER_SEND_BUFFER_END		0x81300000	//发送缓存保存数据的终止地址
#define	BUFFER_RECV_HANDLE			0x81300000	//接收缓冲句柄所在的地址
#define	BUFFER_RECV_BUFFER_START	0x81301000	//接收缓存保存数据的开始地址
#define	BUFFER_RECV_BUFFER_END		0x813F0000	//接收缓存保存数据的终止地址

#define	BUFFER_DSPMUTEX_OFFSET			0x003F8000
#define	BUFFER_PCMUTEX_OFFSET			0x003FC000
#endif

//#define FORTEST

/*
vis_ring_buffer_send_init(int id) 初始化发送缓存句柄
int id DSP的id号，0 或者 1 ，区分双路板的dsp标号，单路板id=0即可
*/
void* vis_ring_buffer_send_init(size_t bufsize, enum vis_mmnp_packet_type packet_type, int (*packet_custom)(unsigned char *, struct vis_mmnp_avdata *)) {
	int i, j, err_flag = 0;
	pRingBuffer pbuffer = NULL;
	unsigned char *p;

	pbuffer = (RingBuffer *)calloc(1, sizeof(RingBuffer));
	if (pbuffer == NULL) {
		Vmn_Log_Error("alloc memery for pbuffer handle failed");
		err_flag = 1;
	}

	if (bufsize<=(EXTENDED_LEN*2)) bufsize = (BUFFER_SEND_BUFLEN+EXTENDED_LEN);
	p = (unsigned char *)calloc(1, bufsize);
	if (p == NULL) {
		Vmn_Log_Error("alloc memery for data addr in ringbuffer failed");
		err_flag = 1;
	}
	pbuffer->rbsyn[0] = 'v'; pbuffer->rbsyn[1]= 'i'; pbuffer->rbsyn[2]= 's'; pbuffer->rbsyn[3]= 's'; 
	pbuffer->rbbits  = 0x1e3c5a96;	//固定值
	pbuffer->pbuffer = p;
	//pbuffer->pstart	 = (unsigned char *)((((unsigned int)p-1)&(~(ALIGN-1)))+ALIGN);
	pbuffer->pstart	 = p;
	pbuffer->pextended = pbuffer->pstart+bufsize;
	pbuffer->pend	 = pbuffer->pextended-EXTENDED_LEN;
	
	pbuffer->psend = pbuffer->pstart;
	pbuffer->precv = pbuffer->pstart;
	
	pbuffer->pencode = pbuffer->pstart;
	pbuffer->pdecode = pbuffer->pstart;

	pbuffer->bufferlen = pbuffer->pend - pbuffer->pstart;
	switch (packet_type) {
		case VIS_MMNP_PACKET_TYPE_TS :
		{
			VisTS_params tsparams;
			VisTS_config tsconfig;
			/* Configure TS Params */
#if 0
			memset(&tsparams,0, sizeof(tsparams));
			memset(&tsconfig,0, sizeof(tsconfig));
			tsparams.size = sizeof(tsparams);
			tsconfig.size = sizeof(tsconfig);
			tsparams.config = &tsconfig;
		//	tsparams.channel_id = id;
			//tsconfig.PMT_num
			ret = VisTS_init(&tsparams);
			if (ret == 0) {
				vis_ring_buffer_close(pbuffer);
				return 0;
			}
#else
			memset(&tsparams,0,sizeof(tsparams));
			memset(&tsconfig,0,sizeof(tsconfig));
			tsparams.size = sizeof(tsparams);
			tsconfig.size = sizeof(tsconfig);
			tsparams.config = &tsconfig;
			//set the config
			tsparams.config->PMT_num = 1;
			tsparams.config->PMT_PID[0] = 10;
			
			tsparams.config->PMT_elementary_num[0] = 1;
			tsparams.config->PMT_es_PID[0][0] = 100;
			tsparams.config->PMT_es_streamType[0][0] = STREAM_TYPE_H264;
#if MPEG4
			tsparams.config->PMT_es_streamType[0][0] = STREAM_TYPE_MPEG4;
#endif
#if MPEG2
			tsparams.config->PMT_es_streamType[0][0] = STREAM_TYPE_MPEG2;
#endif

			tsparams.config->PMT_elementary_num[0] = 2;
			tsparams.config->PMT_es_PID[0][1] = 102;
			tsparams.config->PMT_es_streamType[0][1] = STREAM_TYPE_AAC; //sp 02-26-2010

			if (VisTS_init(&tsparams) <= 0) {
				err_flag = 1;
			}	
			pbuffer->packet_callback = packet_av2ts;
			break;
		}
		case VIS_MMNP_PACKET_TYPE_C1 :
			pbuffer->packet_callback = packet_av2c1;
			break;
		case VIS_MMNP_PACKET_TYPE_CUSTOM :
			pbuffer->packet_callback = packet_custom;
			break;
		default :
			pbuffer->packet_callback = packet_nil;
			break;
	}

	/*
	if (sem_init(&pbuffer->sem_vis_ring_buffer_putdata, 0, 1) == -1) {
		Vmn_Log_Error("init putdata semaphore error");
		err_flag = 1;
	}
	if (sem_init(&pbuffer->sem_vis_ring_buffer_sendaddr, 0, 1) == -1) {
		Vmn_Log_Error("init sendaddr semaphore error");
		err_flag = 1;
	}
	*/
	if (pthread_mutex_init(&pbuffer->mutex_vis_ring_buffer_putdata, NULL) != 0) {
		Vmn_Log_Error("init putdata mutex error");
		err_flag = 1;
	}
	if (pthread_mutex_init(&pbuffer->mutex_vis_ring_buffer_sendaddr, NULL) != 0) {
		Vmn_Log_Error("init sendaddr mutex error");
		err_flag = 1;
	}

	for (i=0; i<MAX_THREAD; i++) {
		pbuffer->sendthread[i] = 0;
	}
	for (j=0; j<MAXNUM_BUFFERHEADER; j++) {
		for (i=0; i<MAX_THREAD; i++) {
			pbuffer->vbufheader[j].tcpflag[i] = 0;
			pbuffer->abufheader[j].tcpflag[i] = 0;
		}
	}
#endif

	if (err_flag && pbuffer) {
		if (pbuffer->pbuffer) {
			free(pbuffer->pbuffer);
			pbuffer->pbuffer = NULL;
		}
		free(pbuffer);
		pbuffer = NULL;
	}
	return pbuffer;
}

#if 0
void* vis_ring_buffer_recv_init()
{
	pRingBuffer pbuffer = (void*)BUFFER_RECV_HANDLE;
	int ret;
	int buflen =  BUFFER_RECV_BUFFER_END-BUFFER_RECV_BUFFER_START-EXTENDED_LEN;
	unsigned char *p;

	memset(pbuffer,0,sizeof(RingBuffer));
	
	p=(void*) BUFFER_RECV_BUFFER_START;
	//p=malloc(buflen+EXTENDED_LEN);
	if(p==NULL)
	{
		assert(p);
		return 0;
	}
	pbuffer->rbsyn[0]= 'v'; pbuffer->rbsyn[1]= 'i'; pbuffer->rbsyn[2]= 's'; pbuffer->rbsyn[3]= 'r'; 
	pbuffer->rbbits  = 0x1e3c5a96;	//固定值
	pbuffer->pbuffer=p;	
	pbuffer->pstart=(unsigned char *)((((unsigned int)p-1)&(~(ALIGN-1)))+ALIGN);
	pbuffer->pend=pbuffer->pstart+buflen;
	pbuffer->pextended=pbuffer->pend+EXTENDED_LEN;
	
	pbuffer->precv=pbuffer->pstart;
	pbuffer->psend=pbuffer->pstart;
	
	pbuffer->pencode=pbuffer->pstart;
	pbuffer->pdecode=pbuffer->pstart;


	pbuffer->bufferlen = pbuffer->pend - pbuffer->pstart;

	return pbuffer;
}
#endif

int vis_ring_buffer_close(pRingBuffer pbuffer)
{
	if(pbuffer==NULL)
		return 0;
	//need mutex lock?
	if(pbuffer->pbuffer)
		pthread_mutex_destroy(&pbuffer->mutex_vis_ring_buffer_putdata);
		pthread_mutex_destroy(&pbuffer->mutex_vis_ring_buffer_sendaddr);
		free(pbuffer->pbuffer);
	free(pbuffer);
	return 1;
}

static int vis_ring_buffer_send_reset(pRingBuffer pbuffer)
{
	if(pbuffer==NULL)
		return -1;
	pbuffer->precv=pbuffer->pstart;
	pbuffer->psend=pbuffer->pstart;
	
	pbuffer->pencode=pbuffer->pstart;
	pbuffer->pdecode=pbuffer->pstart;
	pbuffer->data_len = 0;

	memset(pbuffer->vbufheader,0,sizeof(pbuffer->vbufheader));
	memset(pbuffer->abufheader,0,sizeof(pbuffer->abufheader));
	memset(&pbuffer->vsender,0,sizeof(Vis_Buffer_Send));
	memset(&pbuffer->asender,0,sizeof(Vis_Buffer_Send));
	
#ifdef DEBUG_RING_BUFFER
	printf("[sender]ring buffer reset!\n");
#endif
	return 1;
}

/*
static int vis_ring_buffer_recv_reset(pRingBuffer pbuffer)
{
	if(pbuffer==NULL)
		return -1;
	pbuffer->precv=pbuffer->pstart;
	pbuffer->psend=pbuffer->pstart;
	
	pbuffer->pencode=pbuffer->pstart;
	pbuffer->pdecode=pbuffer->pstart;
	pbuffer->data_len = 0;

	memset(pbuffer->bufheader,0,sizeof(pbuffer->bufheader));
	memset(pbuffer->recver,0,sizeof(Vis_Buffer_Recv));
	
#ifdef DEBUG_RING_BUFFER
	printf("[recver]ring buffer reset!\n");
#endif
	return 1;
}
*/
/*
static int vis_ring_buffer_reset(pRingBuffer pbuffer)
{
	if(pbuffer==NULL)
		return -1;
	
	vis_ring_buffer_send_reset(pbuffer);
	vis_ring_buffer_recv_reset(pbuffer);

	return 1;
}
*/

static pBufferHeader vis_ring_buffer_getheader(pRingBuffer pbuffer, int stream)
{
	pBufferHeader p = NULL;

	if(pbuffer == NULL)
		return 0;
	if (is_video(stream))
		p = &pbuffer->vbufheader[pbuffer->vsender.index_buffer];
	else if (is_audio(stream))
		p = &pbuffer->abufheader[pbuffer->asender.index_buffer];
#if 0
#ifdef FORTEST
	if( p->validflag && p->udpflag==0)
	{
		//如何处理异常的问题
		int frame = pbuffer->sender.counter_buffer-pbuffer->sender.counter_tcpsender;
		if(frame>=MAXNUM_BUFFERHEADER/2)
		{
		pbuffer->sender.counter_tcpsender = pbuffer->sender.counter_buffer;
		pbuffer->sender.index_sender = pbuffer->sender.index_buffer;
		memset(pbuffer->bufheader,0,sizeof(pbuffer->bufheader));
		pbuffer->data_len++;
		}//return 0;
	}
#endif
#endif
	return p;
}

static pBufferHeader vis_ring_buffer_getheader_vsender(pRingBuffer pbuffer, int thread)
{
	pBufferHeader p;
	int tmp;

	if (pbuffer == NULL || thread >= MAX_THREAD || thread < 0)
		return NULL;
	p = &pbuffer->vbufheader[pbuffer->vsender.index_sender[thread]];
	//if (p->validflag==0 || p->tcpflag[thread]==1) {
	tmp = pbuffer->vsender.counter_buffer-pbuffer->vsender.counter_tcpsender[thread];
	if (tmp <= 0) {
		//printf("getheader_vsender: vsender.index_sender[%d] = %d\n", thread, pbuffer->vsender.index_sender[thread]);
		return NULL;
	}
	//printf("index_sender[%d] = %d\n", thread, pbuffer->vsender.index_sender[thread]);

	return p;
}

static pBufferHeader vis_ring_buffer_getheader_asender(pRingBuffer pbuffer, int thread)
{
	pBufferHeader p;
	int tmp;

	if (pbuffer == NULL || thread >= MAX_THREAD || thread < 0)
		return NULL;
	p = &pbuffer->abufheader[pbuffer->asender.index_sender[thread]];
	tmp = pbuffer->asender.counter_buffer-pbuffer->asender.counter_tcpsender[thread];
	if (tmp <= 0) {
		//printf("getheader_asender: asender.index_sender[%d] = %d\n", thread, pbuffer->asender.index_sender[thread]);
		return NULL; //is this just ok?
	}

	return p;
}

#if 0
static pBufferHeader vis_ring_buffer_getheader_recv(pRingBuffer pbuffer)
{			
	pBufferHeader p;
	if(pbuffer==NULL)
		return 0;
	p=&pbuffer->bufheader[pbuffer->recver.index_buffer];
	if( p->validflag&&(p->parserflag==0))
		return 0;
	return p;
}

static pBufferHeader vis_ring_buffer_getheader_parser(pRingBuffer pbuffer)//for sender
{
	pBufferHeader p;
	if(pbuffer==NULL)
		return 0;
	p=&pbuffer->bufheader[pbuffer->recver.index_parser];
	if( p->validflag==0||p->parserflag==1)
		return 0;
	return p;
}

static int vis_ring_buffer_send_over(pRingBuffer pbuffer,int len, int thread)
{
	int all;

	if(pbuffer==NULL || len<=0)
		return 0;
	pbuffer->psend += len;
	if(pbuffer->psend >= pbuffer->pend) {
		all = pbuffer->pend - pbuffer->pstart;
		pbuffer->psend -= all;
		//pbuffer->data_len-=all;
	}

	return 1;
}
#endif

#if 0
static int vis_ring_buffer_parse_over(pRingBuffer pbuffer,int len)
{
	if(pbuffer==NULL||len<=0)
		return 0;
	pbuffer->pdecode+=len;
	if(pbuffer->pdecode>=pbuffer->pend)
	{
		int all=pbuffer->bufferlen;
		pbuffer->pdecode-=all;
		pbuffer->data_len-=all;
	}
	
	return 1;
}

int vis_ring_buffer_recv(pRingBuffer pbuffer,unsigned char* buf,int len)
{
	BufferHeader  *pheader,bufferheader;
	if(pbuffer==NULL||buf==NULL||len<=0)
		return 0;
	if(pbuffer->precv-pbuffer->pdecode+pbuffer->data_len+len>pbuffer->bufferlen)
	{
		//overflow 
		return -2;
		//vis_ring_buffer_recv_reset(pbuffer);
	}
	pheader = &bufferheader;
	pheader = vis_ring_buffer_getheader_recv(pbuffer);
	if(pheader==NULL)
	{
		return -1;	//没有空的索引头信息
	}
	memset(pheader,0,sizeof(BufferHeader));
	pheader->pdata 	 =  pbuffer->precv;
	pheader->datalen = len;
	pheader->syn[0]='v';pheader->syn[1]='i';pheader->syn[2]='s';pheader->syn[3]='h';
	pheader->validflag =1;
	memcpy(pheader->pdata,buf,len);
	pbuffer->recver.counter_buffer++;
	pbuffer->recver.index_buffer++;
	if(pbuffer->recver.index_buffer>=MAXNUM_BUFFERHEADER)
	   pbuffer->recver.index_buffer=0;
	//
	pbuffer->precv+=len;
	if(pbuffer->precv>=pbuffer->pend)
	{
		memcpy(pbuffer->pstart,pbuffer->pend,pbuffer->precv-pbuffer->pend);
		pbuffer->data_len+=(pbuffer->pend-pbuffer->pstart);
		pbuffer->precv-=(pbuffer->pend-pbuffer->pstart);
	}
	return 1;
}

void* vis_ring_buffer_parse(pRingBuffer pbuffer,int* len,int* type)
{
	BufferHeader  *pheader,bufferheader;
	InfoHeader   *pinfoheader,infoheader;
	void * retval=0;
	int bufcounter;
	if(pbuffer==NULL||len==NULL||type==NULL)
		return 0;
	*len = 0;
	bufcounter=pbuffer->recver.counter_buffer-pbuffer->recver.counter_parser;
	if(bufcounter>0)
	{
		pheader = vis_ring_buffer_getheader_parser(pbuffer);
		pbuffer->recver.counter_parser++;
		pbuffer->recver.index_parser++;
		if(pbuffer->recver.index_parser>=MAXNUM_BUFFERHEADER)
		   pbuffer->recver.index_parser=0;
		if(pheader&&pheader->syn[0]=='v'&&pheader->syn[1]=='i'&&pheader->syn[2]=='s'&&pheader->syn[3]=='h')
		{
			vis_ring_buffer_parse_over(pbuffer,pheader->datalen);
			pinfoheader = &infoheader;
			memcpy(pinfoheader,pheader->pdata,sizeof(InfoHeader));
			if(pinfoheader->headerlen==sizeof(InfoHeader)&&(pinfoheader->len+pinfoheader->headerlen)==(pheader->datalen))//confirm
			{
				retval = pheader->pdata+sizeof(InfoHeader);
				*len   = pheader->datalen-sizeof(InfoHeader);
				*type  = pinfoheader->type;
			}
		}
		if(retval==NULL)
		{
			//reset//not expect
			return 0;
			//vis_ring_buffer_recv_reset(pbuffer);
		}
	}
	return retval;
}
#endif

int vis_ring_buffer_getthreadid(pRingBuffer pbuffer)
{
	char tmp = -1;
	int i;
	for (i=0; i<MAX_THREAD; i++)
		if (pbuffer->sendthread[i] == 0)
		{
			tmp = i;
		}
	return tmp;
}

static int vis_ring_buffer_getslow(pRingBuffer pbuffer, int stream)
{
	int i, thread = -1;
	unsigned int counter = ~1;

	if (pbuffer->status.num_connect == 1) {
		for (i=0; i<MAX_THREAD; i++) {
			if (pbuffer->sendthread[i] == 1) {
				return i;
			}
		}
	} else if (pbuffer->status.num_connect > 1) {
		if (is_video(stream)) {
			for (i=0; i<MAX_THREAD; i++) {
				if (pbuffer->sendthread[i] == 1) {
					if (counter > pbuffer->vsender.counter_tcpsender[i]) {
						counter = pbuffer->vsender.counter_tcpsender[i];
						thread = i;
					}
				}
			}
			return thread;
		} else if (is_audio(stream)) {
			for (i=0; i<MAX_THREAD; i++) {
				if (pbuffer->sendthread[i] == 1) {
					if (counter > pbuffer->asender.counter_tcpsender[i]) {
						counter = pbuffer->asender.counter_tcpsender[i];
						thread = i;
					}
				}
			}
			return thread;
		}
	}

	return -1;
}

#if 0
static int vis_ring_buffer_getfast(pRingBuffer pbuffer, int stream)
{
	int i, thread;
	unsigned int counter = 0;

	if (pbuffer->status.num_connect == 1) {
		for (i=0; i<MAX_THREAD; i++) {
			if (pbuffer->sendthread[i] == 1) {
				return i;
			}
		}
	} else if (pbuffer->status.num_connect > 1) {
		if (is_video(stream)) {
			for (i=0; i<MAX_THREAD; i++) {
				if (pbuffer->sendthread[i] == 1) {
					if (counter < pbuffer->vsender.counter_tcpsender[i]) {
						counter = pbuffer->vsender.counter_tcpsender[i];
						thread = i;
					}
				}
			}
			return thread;
		} else if (stream == STREAM_TYPE_AAC) {
			for (i=0; i<MAX_THREAD; i++) {
				if (pbuffer->sendthread[i] == 1) {
					if (counter < pbuffer->asender.counter_tcpsender[i]) {
						counter = pbuffer->asender.counter_tcpsender[i];
						thread = i;
					}
				}
			}
			return thread;
		}
	}

	return -1;
}
#endif

int vis_ring_buffer_regthread(pRingBuffer pbuffer, int thread)
{
	char tmp = 0;
	int i;
	//int fast_thread;

	if (pbuffer == NULL || thread >= MAX_THREAD || thread < 0)
		return -1;
	if (pbuffer->status.num_connect == 0) {
		vis_ring_buffer_send_reset(pbuffer);
	} else if (pbuffer->status.num_connect >= 1) {
//        fast_thread = vis_ring_buffer_getfast(pbuffer, STREAM_TYPE_H264);
		pbuffer->vsender.counter_tcpsender[thread] = pbuffer->vsender.counter_buffer;
		pbuffer->asender.counter_tcpsender[thread] = pbuffer->asender.counter_buffer;
		pbuffer->vsender.index_sender[thread] = pbuffer->vsender.index_buffer;
		pbuffer->asender.index_sender[thread] = pbuffer->asender.index_buffer;
	}
	pbuffer->sendthread[thread] = 1;
	for (i=0; i<MAX_THREAD; i++)
		if (pbuffer->sendthread[i] == 1)
			tmp++;
	pbuffer->status.num_connect = tmp;

	return 0;
}

int vis_ring_buffer_unregthread(pRingBuffer pbuffer, int thread)
{
	char tmp = 0;
	int i;

	if (pbuffer == NULL || thread >= MAX_THREAD || thread < 0)
		return -1;
	pbuffer->sendthread[thread] = 0;
	pbuffer->vsender.counter_tcpsender[thread] = 0;
	pbuffer->asender.counter_tcpsender[thread] = 0;
	pbuffer->vsender.index_sender[thread] = 0;
	pbuffer->asender.index_sender[thread] = 0;
	for (i=0; i<MAX_THREAD; i++)
		if (pbuffer->sendthread[i] == 1)
			tmp++;
	pbuffer->status.num_connect = tmp;

	return 0;
}


int vis_ring_buffer_putdata(volatile pRingBuffer pbuffer, struct vis_mmnp_avdata *avdata)
{
	int all, framenum=0, thread;
	BufferHeader *pheader;
	int len, i;

	if(pbuffer==NULL || avdata==NULL || avdata->datalen<=0 || avdata->data==NULL || pbuffer->packet_callback==NULL) {
		Vmn_Log_Warning("pbuffer=%p, avdata=%p, data=%p, datalen=%d, packet_callback=%p\n", pbuffer, avdata, avdata->data, avdata->datalen, pbuffer->packet_callback);
		return -1;  //传入参数有误，返回-1
	}

	pthread_mutex_lock(&pbuffer->mutex_vis_ring_buffer_putdata);

#if 0
#ifndef FORTEST
	all=pbuffer->pend-pbuffer->pstart;
	datalen=pbuffer->pencode-pbuffer->psend+pbuffer->data_len;
	if(len>=all)
		return 0;
	if(datalen+len+len/4>=all)//overflow//防止内存溢出
	{
		return -1;
		//vis_ring_buffer_send_reset(pbuffer);
	}
#endif
#endif

#ifdef THREAD_SEND_INDEPEND
	i = 0;
	do {
	int vframenum = 0;
#endif
	thread = vis_ring_buffer_getslow(pbuffer, avdata->type);		//get the slowest thread
	if (is_video(avdata->type)) {
		vframenum = pbuffer->vsender.counter_buffer - pbuffer->vsender.counter_tcpsender[thread];
		framenum = vframenum;
		//printf("vframenum = %d thread = %d\n", framenum, thread);
		if (pbuffer->vsender.index_buffer!= (pbuffer->vsender.counter_buffer%MAXNUM_BUFFERHEADER)) {
			pbuffer->vsender.index_buffer= pbuffer->vsender.counter_buffer%MAXNUM_BUFFERHEADER;//确保索引的正确性
		}
	} else if (is_audio(avdata->type)) {
		int aframenum = pbuffer->asender.counter_buffer - pbuffer->asender.counter_tcpsender[thread];
		framenum = aframenum;
		//printf("aframenum = %d thread = %d\n", framenum, thread);
		if (pbuffer->asender.index_buffer!= (pbuffer->asender.counter_buffer%MAXNUM_BUFFERHEADER)) {
			pbuffer->asender.index_buffer= pbuffer->asender.counter_buffer%MAXNUM_BUFFERHEADER;//确保索引的正确性
		}
	}
#ifdef THREAD_SEND_INDEPEND
	if (vframenum < 64) {
		break;
	} else {
//		int tmplen;
//		vis_ring_buffer_send_addr(pbuffer, &tmplen, thread);
		printf("<w>ringbuffer.c: in putdata(), thread[%d] send too slow, frame_dropped_num=%d\n", thread, vframenum);
		pbuffer->vsender.counter_tcpsender[thread] = pbuffer->vsender.counter_buffer;
		pbuffer->vsender.index_sender[thread] = pbuffer->vsender.index_buffer;
	}
	} while (1);
#endif

#ifndef THREAD_SEND_INDEPEND 
	if(framenum > MAXNUM_BUFFERHEADER/8) {
		return -2;	//过多的帧数据未发送时，返回-2
	}
#endif
	
	pheader = vis_ring_buffer_getheader(pbuffer, avdata->type);
	if(pheader==NULL) {
		return -2;	//没有空的索引头信息，返回-2
	}
#if 0
	tsparams.length_of_Access_Unit   = params->length_of_Access_Unit;
	tsparams.pBuffer_for_Access_Unit = params->pBuffer_for_Access_Unit;
	tsparams.second       = params->second;
	tsparams.usecond      = params->usecond;
	tsparams.stream_Type  =	params->stream_Type;
	tsparams.flag		  = params->flag;
	tsparams.tspackets 	  = pbuffer->pencode;

	len=0;
	if ((len = VisTS_package(&tsparams)) <= 0) {
		return -3;  //打包失败，返回-3
	}
#else
	if ((len = pbuffer->packet_callback(pbuffer->pencode, avdata)) <= 0) {
		return -3;  //打包失败，返回-3
	}
#endif

	/* Write info to index node */
	memset(pheader,0,sizeof(BufferHeader));
	pheader->pdata =  pbuffer->pencode;		//打包后的TS保存的内存地址
	pheader->syn[0]='v';pheader->syn[1]='i';pheader->syn[2]='s';pheader->syn[3]='h';
	//pheader->bufno = pbuffer->sender.counter_buffer;
	pheader->datalen = len;
	pheader->validflag 	= 1;
	for (i=0; i<MAX_THREAD; i++)
		pheader->tcpflag[i] = 0;
	pheader->sec = avdata->timestamp.sec;
	pheader->usec = avdata->timestamp.usec;

	pbuffer->pencode += len;
	if(pbuffer->pencode >= pbuffer->pend) {
		all = pbuffer->pend - pbuffer->pstart;
		pbuffer->pencode -= all;
//		pbuffer->data_len += all;
	}

	if (is_video(avdata->type)) {
		pbuffer->vsender.counter_buffer++;
		pbuffer->vsender.index_buffer++;
		//printf("video counter_buffer = %d index_buffer = %d\n", pbuffer->vsender.counter_buffer, pbuffer->vsender.index_buffer);
		if(pbuffer->vsender.index_buffer >= MAXNUM_BUFFERHEADER)
			pbuffer->vsender.index_buffer = 0;
	} else if (is_audio(avdata->type)) {
		pbuffer->asender.counter_buffer++;
		pbuffer->asender.index_buffer++;
		//printf("audio counter_buffer = %d index_buffer = %d\n", pbuffer->asender.counter_buffer, pbuffer->asender.index_buffer);
		if(pbuffer->asender.index_buffer >= MAXNUM_BUFFERHEADER)
			pbuffer->asender.index_buffer = 0;
	}
		
	pthread_mutex_unlock(&pbuffer->mutex_vis_ring_buffer_putdata);
	return len;
}

static pBufferHeader vis_ring_buffer_getheader_sender(pRingBuffer pbuffer, int *stream, int thread)
{
	pBufferHeader pvheader, paheader;
	unsigned int vtime = 0, atime = 0;

	if ((pvheader = vis_ring_buffer_getheader_vsender(pbuffer, thread)) != NULL)
		vtime = pvheader->sec*1000 + pvheader->usec/1000;
	if ((paheader = vis_ring_buffer_getheader_asender(pbuffer, thread)) != NULL)
		atime = paheader->sec*1000 + paheader->usec/1000;
	//printf("vtime = %d atime = %d\n", vtime, atime);

	/* 2013-07-26 ls */
	if (paheader != NULL) {
		*stream = VIS_MMNP_DATA_TYPE_AAC;
		return paheader;
	}

	if (paheader!=NULL && pvheader!=NULL) {
		if (atime <= vtime) {
			*stream = VIS_MMNP_DATA_TYPE_AAC;
			return paheader;
		} else {
			*stream = VIS_MMNP_DATA_TYPE_H264;
			return pvheader;
		}
	} else if (paheader!=NULL && pvheader==NULL) {
		*stream = VIS_MMNP_DATA_TYPE_AAC;
		return paheader;
	} else if (paheader==NULL && pvheader!=NULL) {
		*stream = VIS_MMNP_DATA_TYPE_H264;
		return pvheader;
	} else {
		return NULL;
	}
}

unsigned char* vis_ring_buffer_send_addr(pRingBuffer pbuffer, int *len, int *data_type, unsigned int *sec, unsigned int *usec, int thread)
{
	int sendlen, vframenum, aframenum;
	unsigned char *retdata = NULL;
	pBufferHeader  pheader;
	int stream = -1;

	if (pbuffer==NULL || len==NULL || thread>=MAX_THREAD || thread < 0) {
		Vmn_Log_Debug("vis_ring_buffer_send_addr:invalid params, pbuffer=0x%p, len=0x%p, threadid=%d\n", pbuffer, len, thread);
		return NULL;
	}
	*len = 0;
	vframenum = pbuffer->vsender.counter_buffer - pbuffer->vsender.counter_tcpsender[thread];
	aframenum = pbuffer->asender.counter_buffer - pbuffer->asender.counter_tcpsender[thread];
	//printf("vframenum = %d aframenum = %d thread = %d\n", vframenum, aframenum, thread);
	//if(vframenum>0 && aframenum>0) {
	if(vframenum>0 || aframenum>0) {
//		pthread_mutex_lock(&pbuffer->mutex_vis_ring_buffer_sendaddr);
		if ((pheader = vis_ring_buffer_getheader_sender(pbuffer, &stream, thread)) == NULL) {
			printf("ringbuffer.c: getheader_sender return pheader = 0x%p thread = %d\n", pheader, thread);
//			pthread_mutex_unlock(&pbuffer->mutex_vis_ring_buffer_sendaddr);
			return NULL;
		}

		if (is_video(stream)) {
			pbuffer->vsender.counter_tcpsender[thread]++;
			pbuffer->vsender.index_sender[thread]++;
			if(pbuffer->vsender.index_sender[thread] >= MAXNUM_BUFFERHEADER)
				pbuffer->vsender.index_sender[thread] = 0;
		} else if (is_audio(stream)) {
			pbuffer->asender.counter_tcpsender[thread]++;
			pbuffer->asender.index_sender[thread]++;
			if(pbuffer->asender.index_sender[thread] >= MAXNUM_BUFFERHEADER)
				pbuffer->asender.index_sender[thread] = 0;
		}
		if (pheader&&pheader->syn[0]=='v'&&pheader->syn[1]=='i'&&pheader->syn[2]=='s'&&pheader->syn[3]=='h') {
			pheader->tcpflag[thread] = 1;
			retdata = pheader->pdata;
			sendlen = pheader->datalen;
			if (sec) *sec = pheader->sec;
			if (usec) *usec = pheader->usec;
			*len = sendlen;
			//vis_ring_buffer_send_over(pbuffer,sendlen, thread);
		} else {
			//reset//not expect
			vis_ring_buffer_send_reset(pbuffer);
		}
//		pthread_mutex_unlock(&pbuffer->mutex_vis_ring_buffer_sendaddr);
	}
	if (data_type) {
		*data_type = stream;
	}

	return retdata;
}

int vis_ring_buffer_send_status(pRingBuffer pbuffer)
{
	int vthread, athread;

	if(pbuffer==NULL)
		return -1;
	vthread = vis_ring_buffer_getslow(pbuffer, VIS_MMNP_DATA_TYPE_H264);//thread which send video slow
	athread = vis_ring_buffer_getslow(pbuffer, VIS_MMNP_DATA_TYPE_AAC); //thread which send audio slow
//    printf("vthread = %d athread = %d\n", vthread, athread);
	pbuffer->status.num_videoleft = pbuffer->vsender.counter_buffer - pbuffer->vsender.counter_tcpsender[vthread];
	pbuffer->status.num_audioleft = pbuffer->asender.counter_buffer - pbuffer->asender.counter_tcpsender[athread];
	pbuffer->status.num_vbufleft = MAXNUM_BUFFERHEADER - pbuffer->status.num_videoleft;
	pbuffer->status.num_abufleft = MAXNUM_BUFFERHEADER - pbuffer->status.num_audioleft;
	//printf("0 vcounter_tcpsender = %u acounter_tcpsender\n", pbuffer->vsender.counter_tcpsender[0], pbuffer->asender.counter_tcpsender[0]);
	//printf("1 vcounter_tcpsender = %u acounter_tcpsender\n", pbuffer->vsender.counter_tcpsender[1], pbuffer->asender.counter_tcpsender[1]);
//    printf("vindex_sender = %u aindex_sender\n", pbuffer->vsender.index_sender, pbuffer->asender.index_sender[]);

	return 0;
}

#if 0
unsigned char* vis_ring_buffer_pci_paser(pRingBuffer pbuffer,int *len)
{
	unsigned char* buf=0;
	if(pbuffer==NULL||len==NULL)
		return 0;
}

void* vis_ring_buffer_recvfrom(pRingBuffer pbuffer,int* len,int* type)
{
	BufferHeader  *pheader,bufferheader;
	InfoHeader   *pinfoheader,infoheader;
	void * retval=0;
	int bufcounter;
	if(pbuffer==NULL||len==NULL||type==NULL)
		return 0;
	*len = 0;
	bufcounter=pbuffer->recver.counter_buffer-pbuffer->recver.counter_parser;
	if(bufcounter>0)
	{
		pbuffer->recver.index_parser=pbuffer->recver.counter_parser%MAXNUM_BUFFERHEADER;//linxj20100516
		pheader = vis_ring_buffer_getheader_parser(pbuffer);
		pbuffer->recver.counter_parser++;
		pbuffer->recver.index_parser++;
		if(pbuffer->recver.index_parser>=MAXNUM_BUFFERHEADER)
		   pbuffer->recver.index_parser=0;
		if(pheader&&pheader->syn[0]=='v'&&pheader->syn[1]=='i'&&pheader->syn[2]=='s'&&pheader->syn[3]=='h')
		{
			//vis_ring_buffer_parse_over(pbuffer,pheader->datalen);
			if(pheader->validflag==1&&pheader->parserflag==0)
			{
				pinfoheader = &infoheader;
				memcpy(pinfoheader,pheader->pdata,sizeof(InfoHeader));
				if(pinfoheader->headerlen==sizeof(InfoHeader)&&(pinfoheader->len+pinfoheader->headerlen)==(pheader->datalen))//confirm
				{
					retval = pheader->pdata+sizeof(InfoHeader);
					*len   = pinfoheader->len;
					*type  = pinfoheader->type;
				}
			}
			pheader->parserflag=1;
		}
		if(retval==NULL)
		{
			//reset//not expect
			return 0;
			//vis_ring_buffer_recv_reset(pbuffer);
		}
	}
	return retval;
}
#endif
