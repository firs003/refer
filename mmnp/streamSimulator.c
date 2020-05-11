#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include "streamSimulator.h"

#ifndef WIN32
#include <unistd.h>
//#include <sys/times.h>
#include <sys/time.h>
struct timeval tvbase = {0, 0};
#else
static unsigned int clk_tck=0;
#endif

static unsigned long long _sim_gettimer()
{
#ifdef WIN32
	//return timeGetTime();
	return clock();
#else
#if 0
	struct tms t;
	if (!clk_tck) clk_tck = sysconf(_SC_CLK_TCK);
//	printf("sizeof(click_t=%d), clk_tck=%d\n", sizeof(t.tms_utime), clk_tck);
	return times(&t) * 1000 / clk_tck; //time()*1000 maybe overflow
#endif
	struct timeval tv;
	if (gettimeofday(&tv, NULL)) {
		return 0;
	}
	if (tvbase.tv_sec==0 && tvbase.tv_usec==0) tvbase = tv;
	return (unsigned long long)(((long long)(tv.tv_sec-tvbase.tv_sec))*1000
			+ ((long long)(tv.tv_usec-tvbase.tv_usec))/1000);
#endif
} 

static unsigned const samplingFrequencyTable[16] = {96000, 88200, 64000, 48000,44100, 32000, 24000, 22050,16000, 12000, 11025, 8000,7350, 0, 0, 0};


static int eg_read_ue(unsigned char*bs, int* poffset)
{
	int m, info,offsetByte,offsetBits;
	unsigned int data,tmp1,tmp2,tmp3,tmp4,tmp5,data1;
	int offset,consumedBits;

	offset=*poffset;

	offsetByte = offset/8;
	offsetBits = offset%8;
	tmp1 = bs[offsetByte]<<offsetBits;
	tmp2 = bs[offsetByte+1]<<offsetBits;
	tmp3 = bs[offsetByte+2]<<offsetBits;
	tmp4 = bs[offsetByte+3]<<offsetBits;
	tmp5 = bs[offsetByte+4]>>(8-offsetBits);
	data = (tmp1<<24)|(tmp2<<16)|(tmp3<<8)|(tmp4)|(tmp5);
	//m=eg_show_firstbit1(bs);
	//info = BitstreamGetBits(bs, 2*m+1);
	m=0;
	data1=data;
	while((data1&0x80000000)==0)
	{
		m++;
		data1<<=1;
		if(m>=16) break;
	}
	info = data>>(32-(2*m+1));
	consumedBits = (2*m+1);
	*poffset = offset+consumedBits;
	return  info - 1;

}



static void scaling_list(unsigned char*bs, int* poffset,int scalsize)
{
	int lastscale,nextscale,deltascale,j;
	unsigned char buf[64];
	int offset,consumedBits;

	offset=*poffset;

	lastscale=nextscale=8;
	for(j=0;j<scalsize;j++)
	{
		if(nextscale)
		{
			int tmpdata = eg_read_ue(bs,&offset);
			deltascale = (tmpdata & 1) ? (tmpdata + 1) >> 1 : -(tmpdata >> 1);
			//deltascale= eg_read_se(v->bs);//eg_read_direct(v->bs, 1);
			nextscale = (lastscale+deltascale+256)%256;
		}
		buf[j] = (nextscale==0)?lastscale:nextscale;
		lastscale = buf[j];
	}
	*poffset = offset;
	/*
	if(scalsize==16)
	buf[0] = 0;
	else
	buf[0] = 1;
	*/
}



static int eg_read_direct(unsigned char*bs, int* poffset)
{
	int m, info,offsetByte,offsetBits;
	unsigned int data,tmp1;
	int offset,consumedBits;

	offset=*poffset;

	offsetByte = offset/8;
	offsetBits = offset%8;
	tmp1 = bs[offsetByte]>>(7-offsetBits);
	data = tmp1&1;
	*poffset = offset+1;
	return data;
}


static void getWxHbyIDR(unsigned char* pBuffer,int iBufSize,int* width,int* height)
{
	unsigned char* bs;
	int i,edged_width,edged_height,offset,consumedBits,tmp,nal_unit_type,profile_idc;

	bs = pBuffer;
	nal_unit_type = bs[4]&0x1f;
	if(nal_unit_type==0x07)	//sps
	{
		//v->ss.profile_idc = eg_read_direct(v->bs, 8);
		profile_idc = bs[5];
		//eg_read_skip(v->bs, 8);
		//v->ss.level_idc = eg_read_direct(v->bs, 8);
		offset = (5+3)*8;
		//v->ss.seq_id = eg_read_ue(v->bs);
		//assert(v->ss.seq_id == 0);
		//tmp = eg_read_ue(bs,offset,&consumedBits);
		//offset+=consumedBits;
		tmp = eg_read_ue(bs,&offset);

		//add by linxj 2011-08-04
		if(profile_idc==100||profile_idc==110||profile_idc==122||profile_idc==144)
		{
			int chroma_format_idc,seq_scaling_matrix_present_flag,tmp;
			//chroma_format_idc=eg_read_ue(v->bs);
			chroma_format_idc=eg_read_ue(bs,&offset);
			if(chroma_format_idc==3)
			{
				//tmp=eg_read_direct(v->bs, 1);	
				offset+=1;
			}
			//tmp=eg_read_ue(v->bs);
			tmp=eg_read_ue(bs,&offset);
			//tmp=eg_read_ue(v->bs);
			tmp=eg_read_ue(bs,&offset);
			//tmp=eg_read_direct(v->bs, 1);
			offset+=1;
			//seq_scaling_matrix_present_flag=eg_read_direct(v->bs, 1);
			seq_scaling_matrix_present_flag=eg_read_direct(bs, &offset);
			if(seq_scaling_matrix_present_flag)
			{
				for(i=0;i<8;i++)
				{
					//tmp=eg_read_direct(v->bs, 1);
					tmp=eg_read_direct(bs, &offset);
					if(tmp==1)
					{
						if(i<6)
							scaling_list(bs, &offset,16);
						else
							scaling_list(bs, &offset,64);
					}
				}
			}
		}

		//v->ss.log2_max_frame_num_minus4 = eg_read_ue(v->bs);
		//tmp = eg_read_ue(bs,offset,&consumedBits);
		//offset+=consumedBits;
		tmp = eg_read_ue(bs,&offset);
		//v->ss.pic_order_cnt_type = eg_read_ue(v->bs);
		//tmp = eg_read_ue(bs,offset,&consumedBits);
		//offset+=consumedBits;
		tmp = eg_read_ue(bs,&offset);
		//assert(v->ss.pic_order_cnt_type == 0);
		//if (v->ss.pic_order_cnt_type == 0)				//linxj: if pic_order_cnt_type=1,there is something error 2005/11/8
		if(tmp==0)
		{
			//v->ss.max_pic_order = eg_read_ue(v->bs);
			//tmp = eg_read_ue(bs,offset,&consumedBits);
			//offset+=consumedBits;
			tmp = eg_read_ue(bs,&offset);
		}else if(tmp==1)
		{
			//
			offset+=1;//
		}

		//v->ss.num_ref_frames = eg_read_ue(v->bs);
		//tmp = eg_read_ue(bs,offset,&consumedBits);
		//offset+=consumedBits;
		tmp = eg_read_ue(bs,&offset);
		//if(v->ss.num_ref_frames>1)
		if(tmp>1)
		{
			//printf("num_ref_frame=%d \n",v->ss.num_ref_frames);
			//v->vis_error|=VIS264DEC_ERROR_MULTIREF;	//ref frame >1
			//	return 0;
		}
		//eg_read_skip(v->bs, 1);								//linxj: gaps_in_frame_num_value_allowed_flag 2005/11/8
		offset+=1;
		//v->ss.pic_width_in_mbs_minus1 = eg_read_ue(v->bs);
		//edged_width = eg_read_ue(bs,offset,&consumedBits);
		//offset+=consumedBits;
		edged_width = eg_read_ue(bs,&offset);
		//v->ss.pic_height_in_mbs_minus1 = eg_read_ue(v->bs);
		//edged_height = eg_read_ue(bs,offset,&consumedBits);
		//offset+=consumedBits;
		edged_height = eg_read_ue(bs,&offset);
		*width  = (edged_width+1)*16;
		*height = (edged_height+1)*16;
	}
}


static int getAudioParam(unsigned char* pBuffer,int iBufSize,audio_frame_param *aparam)
{
	// Now, having opened the input file, read the fixed header of the first frame,
	// to get the audio stream's parameters:
	unsigned char fixedHeader[4]; // it's actually 3.5 bytes long
	unsigned char variableHeader[3];

	unsigned char sampling_frequency_index,channel_configuration;
	int length;

	if(pBuffer==0||iBufSize<4)
		return -1;
	memcpy(fixedHeader,pBuffer,sizeof(fixedHeader));
	memcpy(variableHeader,pBuffer+4*sizeof(unsigned char),sizeof(variableHeader));
	// Check the 'syncword':
	if (!(fixedHeader[0] == 0xFF && (fixedHeader[1]&0xF0) == 0xF0)) {
		//env.setResultMsg("Bad 'syncword' at start of ADTS file");
		return -1;
	}
	/* 
	// Get and check the 'profile':
	u_int8_t profile = (fixedHeader[2]&0xC0)>>6; // 2 bits
	if (profile == 3) {
	env.setResultMsg("Bad (reserved) 'profile': 3 in first frame of ADTS file");
	break;
	}
	*/

	// Get and check the 'sampling_frequency_index':
	length=((fixedHeader[3]&0x03)<<11)|(variableHeader[0]<<3)|((variableHeader[1]&0xe0)>>5);
	sampling_frequency_index = (fixedHeader[2]&0x3C)>>2; // 4 bits
	if (samplingFrequencyTable[sampling_frequency_index] == 0) {
		//env.setResultMsg("Bad 'sampling_frequency_index' in first frame of ADTS file");
		//break;
		return -1;
	}

	// Get and check the 'channel_configuration':
	channel_configuration  = ((fixedHeader[2]&0x01)<<2)|((fixedHeader[3]&0xC0)>>6); // 3 bits

	aparam->samplerate = samplingFrequencyTable[sampling_frequency_index];
	aparam->channels   = channel_configuration == 0 ? 2 : channel_configuration;
	aparam->audiobits  = 16;
	return length;
}

static int read_naldata(unsigned char *pbuf,int *buf_start,int *buf_end,unsigned char *nalbuf,int *len,int devide)  //     
{
	int b_copy=0,b_getend=0,start=*buf_start,end=*buf_end;
	unsigned char  data;
	unsigned char *psrc=pbuf+start;
	unsigned char *pend=pbuf+end;
	unsigned char *pdst=nalbuf;
	unsigned int  shift=-1;
	int type;
	shift=-1;
	b_copy=0;
	while(psrc<=pend)
	{
		data=*psrc++;
		shift=(shift<<8)+data;
		if(shift==0x00000001)
		{
			b_copy=1;
			break;
		}
	}

	if(b_copy)
	{
		*pdst++=0;
		*pdst++=0;
		*pdst++=0;
		type = psrc[0]&0x1f;
		*pdst++=1;
		while(psrc<=pend)
		{
			data=*psrc++;
			*pdst++=data;
			shift=(shift<<8)+data;
			if(shift==0x00000001)
			{
				/*int type;
				type = psrc[0]&0x1f;*/
				if(devide==0)//设置为不分割IDR帧
				{
					if((type==7)||(type==8)||(type==6))//if((type==NALU_TYPE_PPS)||(type==NALU_TYPE_SPS)||(type==NALU_TYPE_SEI))
					{
						type = psrc[0]&0x1f;
						//	if(pdst-nalbuf<256)  
						continue;	//make sure sps and pps connect to I slice
					}
				}

				psrc-=4;
				pdst-=4;
				b_getend=1;
				break;
			}
		}
	}
	(*len)=pdst-nalbuf;
	*buf_start=psrc-pbuf;

	return b_getend;
}

static int read_aacdata(unsigned char *pbuf,int *buf_start,int *buf_end,audio_frame_param *aparam)  //     
{
	int b_copy=0,b_getend=0,start=*buf_start,end=*buf_end;
	unsigned char  data;
	unsigned char *psrc=pbuf+start;
	unsigned char *pend=pbuf+end;
	unsigned char *pdst=aparam->p_payload;
	unsigned short  shift=-1;
	int packlen=0;
	unsigned char *prepos;
	unsigned char *pos;
	shift=-1;
	b_copy=0;
	while(psrc<=pend)
	{
		data=*psrc++;
		shift=(shift<<8)+data;
		if(shift==0xfff9)
		{
			b_copy=1;
			prepos=psrc-2;
			break;
		}
	}

	if(b_copy)
	{
		*pdst++=0xff;
		*pdst++=0xf9;
		while(psrc<=pend)
		{
			data=*psrc++;
			*pdst++=data;
			shift=(shift<<8)+data;
			if(shift==0xfff9)
			{
				pos=psrc-2;
				packlen=getAudioParam(prepos,pos-prepos,aparam);
				if(packlen!=(pos-prepos))
					continue;
				//	if(pdst-nalbuf<128)  continue;	//make sure sps and pps connect to I slice
				psrc-=2;
				pdst-=2;
				b_getend=1;
				break;
			}
		}
	}
	aparam->i_payload=packlen;
	*buf_start=psrc-pbuf;

	return b_getend;
}
/////////////////////


static int checkHandle(sSimulator* pssim)
{
	if(pssim==0)
		return 0;
	if(pssim->syn[0]!='S'||pssim->syn[1]!='S'||pssim->syn[2]!='I'||pssim->syn[3]!='M')
		return 0;
	return 1;
}

//////////////////////// API ////////////
//输入视频和音频文件名
void*	ssim_create(char *pvideo_filename,char *paudio_filename,ssim_media_param *sparam )
{
	FILE* fp;
	sSimulator* pssim=0;
	int error=0;

	pssim = malloc(sizeof(sSimulator));
	if(pssim==0)
	{
		return 0;
	}
	memset(pssim,0,sizeof(*pssim));
	pssim->syn[0]='S';
	pssim->syn[1]='S';
	pssim->syn[2]='I';
	pssim->syn[3]='M';

	pssim->timestampe_enable = sparam->ts_simulatenable;

	if(pvideo_filename)
	{
		fp = fopen(pvideo_filename, "rb");
		if(fp == 0)
		{
			error=1;
			printf("Failed to open %s\n", pvideo_filename);
		}else
		{
			pssim->pvf = fp;
			pssim->video_framerate=sparam->video_framerate;
			if(sparam->video_framegap_us>0)
			pssim->video_framegap_us=sparam->video_framegap_us;
			pssim->video_idr_devide=sparam->video_idr_devide;
			pssim->video_circle=sparam->video_circle;
			pssim->video_encodeType=sparam->video_encodeType;
			pssim->video_iEnable=1;
		}
	}

	if(error==0)
	{
		if(paudio_filename)
		{
			fp = fopen(paudio_filename, "rb");
			if(fp == 0)
			{
				printf("Failed to open audio file :%s\n", paudio_filename);
				error=2;
			}
			else
			{
				pssim->paf = fp;
				pssim->audio_iEnable=1;
				pssim->audio_framegap_us=sparam->audio_framegap_us;
				pssim->audio_circle=sparam->audio_circle;
				pssim->audio_encodeType=sparam->audio_encodeType;
				pssim->audio_channels=sparam->audio_channels;
				pssim->audio_samplerate=sparam->audio_samplerate;
				pssim->audio_bits=sparam->audio_bits;

			}
		}
	}

	if(error)
	{
		if(fp)
			fclose(fp);
		if(pssim)
			free(pssim);

		pssim = 0;
	}

	return pssim;
}

//handle和type为输入，param为输出
int		ssim_getvideo( void *handle,int type, video_frame_param* vparam)//return=0获取成功，-1失败，1时间未到跳过
{
	int ret=0,tmp,nallen;
	sSimulator* pssim=(sSimulator*)handle;
	unsigned char frameType=0,i_type;

	if(checkHandle(pssim)!=1)
	{
		ret = SSIM_ERROR_HANDLE;
		return ret;
	}

	if(vparam==0)
	{
		ret = SSIM_ERROR_INPUT;
		return ret;
	}

	if(vparam->size!=sizeof(*vparam))
	{
		ret = SSIM_ERROR_INPUT;
		return ret;
	}

	nallen = 0;
	if(pssim->timestampe_enable&&pssim->video_framegap_us>0)
	{
		//需要对时间进行判断
#ifndef WIN32
		unsigned long long timenow=_sim_gettimer();
		long long time_gone;
#else
		unsigned long long timenow=_sim_gettimer();
		int time_gone;
#endif
		time_gone = timenow - pssim->video_gettime;
//		printf("\t1.0 simulator.c:time_gone=%lld, time_now=%llu, time_lastget=%llu\n", time_gone, timenow, pssim->video_gettime);
		if((pssim->video_gettime)&&(time_gone<(pssim->video_framegap_us/1000))) {//距上一帧时间间隔未到
//			printf("\t1.1 simulator.c:time_gone=%lld, time_now=%llu, time_lastget=%llu\n", time_gone, timenow, pssim->video_gettime);
			return SSIM_ERROR_TOOEARLY;
		}

		pssim->video_gettime=timenow;//该帧获取时间
	}else
	{
		pssim->video_gettime+=(pssim->video_framegap_us/1000);//该帧获取时间
	}

	if(pssim->pvf)
	{
		if(pssim->vbuf_end-pssim->vbuf_start<sizeof(pssim->videofile_buf)/2)
		{
			if(pssim->vbuf_start)
			{
				int len=pssim->vbuf_end-pssim->vbuf_start;
				memcpy(pssim->videofile_buf,pssim->videofile_buf+pssim->vbuf_start,len);
				pssim->vbuf_start=0;
				pssim->vbuf_end=len;
			}
			if(pssim->videofile_readover!=1)
			{
				pssim->vbuf_len=fread(pssim->videofile_buf+pssim->vbuf_end,1,sizeof(pssim->videofile_buf)/2,pssim->pvf);
				if(pssim->vbuf_len>0) 
					pssim->vbuf_end+=pssim->vbuf_len;
				if(pssim->vbuf_len<sizeof(pssim->videofile_buf)/2)
				{
					if(pssim->video_circle==1)
						fseek(pssim->pvf,0,SEEK_SET);	//seek to start 读完了
					else
						pssim->videofile_readover=1;
				}
			}
		}

		tmp = read_naldata(pssim->videofile_buf,&(pssim->vbuf_start),&(pssim->vbuf_end),pssim->nal_buf,&nallen,pssim->video_idr_devide);
		
		//根据pssim->nal_buf此位置读数据

		if(tmp==1 && nallen>5)
		{
			frameType = 0 ;
			i_type = (pssim->nal_buf[4]&0x1f);	//add by linxj 2011-08-04
			if(i_type==0x05)	//NAL_IDR
				frameType = 1;	//IDR
			if(i_type==0x07||i_type==0x08)	//NAL_SPS  NAL_PPS
				frameType = 1;	//IDR
			else if(i_type==0x01||i_type==0x02||i_type==0x03||i_type==0x04)
			{
#if 0
				//NAL_NOPART
				unsigned char tmp = (pssim->nal_buf[5]&0xf0);
				if(tmp==0x80||tmp==0x60)		//0x88
					frameType = 2;	//I帧
				else if(tmp==0x90||(tmp&0xC0)==0xC0)	//0x9A 0x9B
					frameType = 3;	//P帧 
#endif
				int offset,first_mb,slice_type;
				offset=5*8;
				first_mb=eg_read_ue(pssim->nal_buf,&offset);
				slice_type=eg_read_ue(pssim->nal_buf,&offset);
				if(slice_type==0||slice_type==3||slice_type==5||slice_type==8)
					frameType = 3;	//P帧 
				else if(slice_type==1||slice_type==6)	
					frameType = 4;	//B帧 
				else if(slice_type==2||slice_type==4||slice_type==7||slice_type==9)
					frameType = 2;	//I帧 
			}
			if(i_type==7)//SSIM_TYPE_SPS
				getWxHbyIDR(pssim->nal_buf,nallen, &(pssim->video_width), &(pssim->video_heigth));
		}

		if(tmp!=1)
		{
			ret = SSIM_ERROR_UNKNOWN;
			if(pssim->videofile_readover==1)
				ret = SSIM_ERROR_READOVER;
		}

	}
	if(ret==0)
	{
		//成功时进行赋值
		vparam->p_payload=pssim->nal_buf;
		vparam->i_payload=nallen;
		vparam->i_type = i_type;
		vparam->frameType = frameType;
		vparam->width=pssim->video_width;
		vparam->height=pssim->video_heigth;

		//if(pssim->video_timewrite)//写入时间戳
			vparam->video_timestamp=pssim->video_gettime;
	}else
	{
		vparam->p_payload=0;
		vparam->i_payload=0;
	}
	return ret;

}

int		ssim_getaudio( void *handle,int type, audio_frame_param *aparam)//return=0获取成功，-1失败，1时间未到跳过
{
	int ret=0,tmp;
	sSimulator* pssim=(sSimulator*)handle;	
	aparam->p_payload=pssim->audio_buf;
	

	if(checkHandle(pssim)!=1)
	{
		ret = SSIM_ERROR_HANDLE;
		return ret;
	}

	if(aparam==0)
	{
		ret = SSIM_ERROR_INPUT;
		return ret;
	}

	if(aparam->size!=sizeof(*aparam))
	{
		ret = SSIM_ERROR_INPUT;
		return ret;
	}

	if(pssim->timestampe_enable&&pssim->audio_framegap_us>0)
	{
		//需要对时间进行判断
#ifndef WIN32
		unsigned long long timenow=_sim_gettimer();
		long long time_gone;
#else
		unsigned long long timenow=_sim_gettimer();
		int time_gone;
#endif
		time_gone = timenow-pssim->audio_gettime;

		if((pssim->audio_gettime)&&(time_gone<(pssim->audio_framegap_us/1000)))//距上一帧时间间隔未到
			return SSIM_ERROR_TOOEARLY;
		
		pssim->audio_gettime=timenow;	//该帧获取时间
	}else
	{
		pssim->audio_gettime+=(pssim->audio_framegap_us/1000);	//设置该帧时间
	}



	if(pssim->paf)
	{
		if(pssim->abuf_end-pssim->abuf_start<sizeof(pssim->audiofile_buf)/2)
		{
			if(pssim->abuf_start)
			{
				int len=pssim->abuf_end-pssim->abuf_start;
				memcpy(pssim->audiofile_buf,pssim->audiofile_buf+pssim->abuf_start,len);
				pssim->abuf_start=0;
				pssim->abuf_end=len;
			}
			if(pssim->audiofile_readover!=1)
			{
			pssim->abuf_len=fread(pssim->audiofile_buf+pssim->abuf_end,1,sizeof(pssim->audiofile_buf)/2,pssim->paf);
			if(pssim->abuf_len>0) 
				pssim->abuf_end+=pssim->abuf_len;
			if(pssim->abuf_len<sizeof(pssim->audiofile_buf)/2)
			{

				if(pssim->audio_circle==1)
					fseek(pssim->paf,0,SEEK_SET);	//seek to start 读完了
				else
					pssim->audiofile_readover=1;
			}
			}
		}
		tmp = read_aacdata(pssim->audiofile_buf,&(pssim->abuf_start),&(pssim->abuf_end),aparam);

		if(aparam->p_payload==0||aparam->i_payload<=0)
		{
			ret = SSIM_ERROR_UNKNOWN;
			
			if(pssim->videofile_readover==1)
				ret = SSIM_ERROR_READOVER;
		}

		aparam->i_type = SSIM_TYPE_AAC;	//linxj 需要改进
	}

	//if(pssim->audio_timewrite)//写入时间戳
		aparam->audio_timestamp=pssim->audio_gettime;


	return ret;

}


int		ssim_close( void *handle)
{
	int ret=0;
	sSimulator* pssim=(sSimulator*)handle;

	if(checkHandle(pssim)!=1)
	{
		ret = SSIM_ERROR_HANDLE;
		return ret;
	}
	if(pssim->paf)
		fclose(pssim->paf);


	if(pssim->pvf)
		fclose(pssim->pvf);

	memset(pssim,0,sizeof(*pssim));
	free(pssim);

	return ret;

}
