#include "vis_mmnp.h"
#include "vis_mmnp_api.h"
#include "packet.h"
#include "TSpacket.h"
//#include "ringbuffer.c"

int packet_nil(unsigned char *dest, struct vis_mmnp_avdata *avdata) {
	memcpy(dest, avdata->data, avdata->datalen);
	return avdata->datalen;
}

int packet_av2ts(unsigned char *dest, struct vis_mmnp_avdata *avdata) {
//	int packetlen = -1;
	VisTS_params tsparams;
	VisTS_config tsconfig;

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

	tsparams.length_of_Access_Unit = avdata->datalen;
	tsparams.pBuffer_for_Access_Unit = avdata->data;
	tsparams.second       = avdata->timestamp.sec;
	tsparams.usecond      = avdata->timestamp.usec;
	tsparams.tspackets 	  = dest;
	switch (avdata->type) {
		case VIS_MMNP_DATA_TYPE_H264 :
		case VIS_MMNP_DATA_TYPE_H264_PFRAME :
		case VIS_MMNP_DATA_TYPE_H264_SPS :
		case VIS_MMNP_DATA_TYPE_H264_PPS :
			tsparams.stream_Type  =	STREAM_TYPE_H264;
			tsparams.flag		  = FLAG_VIDEO;
			break;
		case VIS_MMNP_DATA_TYPE_H264_IDR :
		case VIS_MMNP_DATA_TYPE_H264_IFRAME :
			tsparams.stream_Type  =	STREAM_TYPE_H264;
			tsparams.flag		  = FLAG_VIDEO_IDR;
			break;
		case VIS_MMNP_DATA_TYPE_AAC :
		case VIS_MMNP_DATA_TYPE_AAC_ADTS :
		case VIS_MMNP_DATA_TYPE_AAC_RAW :
			tsparams.stream_Type  =	STREAM_TYPE_AAC;
			tsparams.flag		  = FLAG_SPEECH;
			break;
		case VIS_MMNP_DATA_TYPE_G711 :
			tsparams.stream_Type  =	STREAM_TYPE_G711;
			tsparams.flag		  = FLAG_SPEECH;
			break;
		case VIS_MMNP_DATA_TYPE_G729 :
			tsparams.stream_Type  =	STREAM_TYPE_G729;
			tsparams.flag		  = FLAG_SPEECH;
			break;
		default :
			tsparams.stream_Type  =	STREAM_TYPE_H264;
			tsparams.flag		  = FLAG_VIDEO;
			break;
	}
//	Vmn_Log_Debug("in packet\n");
	return VisTS_package(&tsparams);
}


/**********
I frame takes 16 bytes
00 00 01 FD X1 X2 X3 X4 T1 T2 T3 T4 L1 L2 L3 L4
X1:0, X2:framerate(25 usually), X3:width/8, X4:height/8
T1 T2 T3 T4:timestamp
L1 L2 L3 L4:packet length(not include head)

I frame takes 8 bytes
00 00 01 FC L1 L2 L3 L4
L1 L2 L3 L4:packet length(not include head)

Audio takes 8 bytes
00 00 01 F0 X1 X2 L1 L2
X1:audio compress, g711a:14, aac:31, aac_dual:32, X2:samplerate,16000:4, 8000:2, 32000:7, 44100:8
L1 L2:packet length(not include head)
**********/
#define CUSTOMER1_PACKAGE_LENGTH	(1024*8)
typedef struct msg_head {
	unsigned char	CommandID;				//message type
	unsigned char	Res0;					//res
	unsigned char	Res1;					//tlv:1
	unsigned char	HeadLength_Version;		//head length+version, 0x58, fixxed
	unsigned		ExtLength;				//tlv length, head length is not include
	unsigned char	HeadData[24];			//params in head
} Head;

typedef struct msg_body {
	int		Type;		//tlv type
	int		Length;		//tlv pure length , not include Type and Length
	void	*Value;		//tlv content
} Body;

int packet_av2c1(unsigned char *dest, struct vis_mmnp_avdata *avdata) {
    int dstlen=0,srclen,packagesize,bodylen,frameheadlen,packagenum,packagelen,srccopylen;
    unsigned char *pdst,*psrc;
    unsigned char  framehead[32];   //
	Head    head;
	memset(&head, 0, sizeof(Head));		//ls modify, 2012-07-18
    psrc    = avdata->data;
    srclen  = avdata->datalen;
    pdst    = dest;
    
    if(psrc==0||srclen<=0||pdst==0) {
		printf("[E]customer1_package() params invalid\n");
        return -1;
	}
    head.CommandID = 0xbc;
	head.HeadLength_Version = 0x58;
	memcpy(head.HeadData+16, &(avdata->timestamp.sec), sizeof(avdata->timestamp.sec));	
	memcpy(head.HeadData+16+sizeof(avdata->timestamp.sec), &(avdata->timestamp.usec), sizeof(avdata->timestamp.usec));	//set first 8 bit of HeadData as frame time(sec and usec), ls, 2012-11-22
    if (avdata->type==VIS_MMNP_DATA_TYPE_H264_IDR || avdata->type==VIS_MMNP_DATA_TYPE_H264_IFRAME) {
        int frameid = 0xfd010000;
        frameheadlen = 16;
        memcpy(&framehead[0],&frameid,sizeof(frameid));
        framehead[4] = 0;
        framehead[5] = avdata->media.video.video_framerate;
        framehead[6] = avdata->media.video.video_width/8;
        framehead[7] = avdata->media.video.video_height/8;
        memcpy(framehead+8, &avdata->timestamp.sec, sizeof(avdata->timestamp.sec));
        memcpy(framehead+12, &srclen, sizeof(srclen));
    } else if(avdata->type==VIS_MMNP_DATA_TYPE_H264_PFRAME) {
        int frameid = 0xfc010000;
        frameheadlen = 8;
        memcpy(&framehead[0],&frameid,sizeof(frameid));
        memcpy(&framehead[4],&srclen,sizeof(srclen));
    } else if(is_audio(avdata->type)) {
        int frameid = 0xf0010000;
		unsigned short framelen = srclen;
        frameheadlen = 8;
        memcpy(&framehead[0], &frameid, sizeof(frameid));
		if(avdata->type==VIS_MMNP_DATA_TYPE_G711) {
        	framehead[4] = 14;
			framehead[5] = 2;
		} else if(avdata->type==VIS_MMNP_DATA_TYPE_AAC) {
        	framehead[4] = 32;		//vis audio thread use dual track only
			if(avdata->media.audio.audio_samplerate == 8000)
				framehead[5] = 2;
			else if(avdata->media.audio.audio_samplerate==16000)
				framehead[5] = 4;
			else if(avdata->media.audio.audio_samplerate==32000)
				framehead[5] = 7;
			else if(avdata->media.audio.audio_samplerate==44100)
				framehead[5] = 8;
		}
//		printf("type = %d, samplerate = %d\n", (int)framehead[4], (int)framehead[5]);
        memcpy(&framehead[6],&framelen,sizeof(framelen));
//		printf("audio: %hu\n", framelen);
    } else {
		Vmn_Log_Error("[E]customer1_package() FLAG[%d] invalid", avdata->type);
        return -1;
	}
    packagesize = CUSTOMER1_PACKAGE_LENGTH;
	if(packagesize<32)  packagesize = 0x0fffffff;
    packagenum = 0;
    while(srclen>0)
    {
      if (packagenum==0) {
            //first package need to add fream head 
            packagelen = packagesize;
            if(srclen+frameheadlen+sizeof(Head) < packagelen)
                packagelen = srclen+frameheadlen+sizeof(Head);
            bodylen = packagelen - sizeof(Head);
            head.ExtLength = bodylen;
            memcpy(pdst, &head, sizeof(Head));
            pdst+=sizeof(Head);
            memcpy(pdst,framehead,frameheadlen);
            pdst += frameheadlen;
            srccopylen = bodylen-frameheadlen;
            memcpy(pdst, psrc, srccopylen);
            psrc += srccopylen;
            pdst += srccopylen;
            srclen -= srccopylen;
      } else {
            packagelen = packagesize;
            if(srclen+sizeof(Head)<packagelen)
                packagelen = srclen+sizeof(Head);
            bodylen = packagelen-sizeof(Head);
            head.ExtLength = bodylen;
            memcpy(pdst, &head, sizeof(Head));
            pdst+=sizeof(Head);
            srccopylen=bodylen;
            memcpy(pdst, psrc, srccopylen);
            psrc += srccopylen;
            pdst += srccopylen;
            srclen -= srccopylen;
	}
      packagenum++;
      dstlen += packagelen;
    }
//	printf("dstlen=%d\n", dstlen);
    return dstlen;
}
