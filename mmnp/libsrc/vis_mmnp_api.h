#ifndef __VIS_MMNP_H__
#define __VIS_MMNP_H__

/***********************************************************************
 * Error Code
 ***********************************************************************/
typedef enum vis_mmnp_errno {
	VIS_MMNP_ERROR_UNKNOWN = -1,
	VIS_MMNP_ERROR_OK,
	//TODO
} Vis_Mmnp_Errno;

typedef enum vis_mmnp_loglevel {
	VIS_MMNP_LOGLEVEL_SILENCE,
	VIS_MMNP_LOGLEVEL_CRIT,
	VIS_MMNP_LOGLEVEL_ERROR,
	VIS_MMNP_LOGLEVEL_DEFAULT = VIS_MMNP_LOGLEVEL_ERROR,
	VIS_MMNP_LOGLEVEL_WARNING,
	VIS_MMNP_LOGLEVEL_INFO,
	VIS_MMNP_LOGLEVEL_DEBUG,
	VIS_MMNP_LOGLEVEL_ALL,
	VIS_MMNP_LOGLEVEL_MAX = 32
} Vis_Mmnp_Log_Level;

/***********************************************************************
 * Network Type
 * Send:0x00-0xfe
 * Recv:0x100-
 ***********************************************************************/
typedef enum vis_mmnp_network_type {
	NETWORK_SEND_UNKNOWN = -1,
	NETWORK_SEND_BROADCAST,
	NETWORK_SEND_MULTICAST,
	NETWORK_SEND_UNICAST,
	NETWORK_SEND_VOD,
	NETWORK_SEND_UDP,
	NETWORK_SEND_TCP,
	NETWORK_SEND_C1,
	NETWORK_SEND_RTSP,
	NETWORK_SEND_RTMP,
	NETWORK_SEND_HTTP,
	NETWORK_SEND_ONVIF,
	NETWORK_SEND_VSIP,
	
	NETWORK_RECV_UNKNOWN = 0xff,
	NETWORK_RECV_BROADCAST,
	NETWORK_RECV_MULTICAST,
	NETWORK_RECV_UNICAST,
	NETWORK_RECV_TCP,
	NETWORK_RECV_C1,
	NETWORK_RECV_RTSP,
	NETWORK_RECV_RTMP,
	NETWORK_RECV_HTTP,
	NETWORK_RECV_ONVIF,
	NETWORK_RECV_VSIP,
} Vis_Mmnp_Network_Type;

typedef enum vis_mmnp_data_type {
	VIS_MMNP_DATA_TYPE_UNKOWN = 0,		//unknown
	VIS_MMNP_DATA_TYPE_H264,			//H264 generic
	VIS_MMNP_DATA_TYPE_H264_SPS,		//SPS for H264
	VIS_MMNP_DATA_TYPE_H264_PPS,		//PPS for H264
	VIS_MMNP_DATA_TYPE_H264_IDR,		//H264 IDR frame with SPS and PPS
	VIS_MMNP_DATA_TYPE_H264_IFRAME,		//I Frame
	VIS_MMNP_DATA_TYPE_H264_PFRAME,		//P Frame
	VIS_MMNP_DATA_TYPE_MPEG2,			//MPEG2
	VIS_MMNP_DATA_TYPE_MPEG4,			//MPEG4
	VIS_MMNP_DATA_TYPE_AAC = 0x100,		//AAC generic, usually with ADTS
	VIS_MMNP_DATA_TYPE_AAC_ADTS,		//AAC with ADTS
	VIS_MMNP_DATA_TYPE_AAC_RAW,			//AAC without ADTS
	VIS_MMNP_DATA_TYPE_MP3,				//MP3
	VIS_MMNP_DATA_TYPE_G711,			//G711 speech
	VIS_MMNP_DATA_TYPE_G729,			//G729 speech
	VIS_MMNP_DATA_TYPE_HYBRID = 0x200,	//video and audio
	VIS_MMNP_DATA_TYPE_PCM = 0x300,		//PCM
} Vis_Mmnp_Data_Type;

typedef enum vis_mmnp_packet_type {
	VIS_MMNP_PACKET_TYPE_UNKOWN = -1,
	VIS_MMNP_PACKET_TYPE_AUTO,
	VIS_MMNP_PACKET_TYPE_NIL,		//do nothing
	VIS_MMNP_PACKET_TYPE_TS,		//TS packet
	VIS_MMNP_PACKET_TYPE_C1,		//C1 packet
	VIS_MMNP_PACKET_TYPE_CUSTOM = 0x100,
} Vis_Mmnp_Packet_Type;

/***********************************************************************
 * Audio/Video Data Structure use for mmnp
 * Common:size of self
 *		  media type, audio or video
 *		  payload addr
 *		  payload length
 *		  timestamp mode, 1-from extern user specify, 0-mmnp get itself
 *		  timestamp
 * Special:this is an union
 *		   audio info
 *		   video info
 ***********************************************************************/
typedef struct vis_mmnp_avdata {
	int size;				//size of struct self
	char sync[4];
	enum vis_mmnp_data_type type;	//payload type, audio or video, speciafy union use which member
	unsigned char *data;	//payload addr, input
	unsigned int datalen;	//payload length, input
	unsigned char *encoded;	//encoded addr, output
	unsigned int encodedlen;//encoded length, output
	int tsenable;			//time stamp from params(1) or system call(0)
	struct {
		unsigned int sec;
		unsigned int usec;
	} timestamp;			//time stamp in us
	union {
		struct {
			int video_iEnable;	//REALDATA_VIDEO or REALDATA_YUV
			unsigned short video_width;	//video width	//REALDATA_YUV
			unsigned short video_height;	//video height  //REALDATA_YUV
			int video_compress;	//video compress 0:unknown H264 = 1 or mpeg4 = 2 //Default H264 //REALDATA_VIDEO
			int video_frametype;//frame type 0:unknown IDR=1 I=2 P=3 B=4 //REALDATA_VIDEO
			int video_naltype;	//reference to typedef enum _RTMP_H264NALUTYPE
			int video_framerate;//video frame rate, subject to timestamp when nil
			int video_framepersec;	//video frame period, subject to timestamp when nil
			int video_framegap;	//unused, subject to timestamp when nil
		} video;
		struct {
			int audio_iEnable;	//REALDATA_AUDIO or REALDATA_PCM
			unsigned short audio_bits;		//16bits or 8bits	//REALDATA_PCM
			unsigned short audio_channels;	//channel num mono=1 or dual=2	//REALDATA_PCM
			int audio_compress;	//audio compress 0:unknown AAC=1 MP3=2 G729=10 G711=11	//REALDATA_AUDIO
			int audio_samplerate;	//sample rate 8000Hz 16000Hz 44100Hz	//REALDATA_PCM
			int audio_framerate;//audio frame rate, subject to timestamp when nil
			int audio_framepersec;//video frame period, subject to timestamp when nil
			int audio_framegap;	//unused, subject to timestamp when nil
		} audio;
	} media;
} Vis_Mmnp_AVData, vis_mmnp_avdata_t;

/***********************************************************************
 * Attrs for vis_mmnp_create, contains:
 *		network type
 *		max connection num
 *		ringbuffer size(not use yet)
 *		callback function for getting compressed avdata
 *		callback function for putting compressed avdata
 *		callback function for packetting avdata, if customer want
 ***********************************************************************/
typedef struct vis_mmnp_attrs {
	int size;
	enum vis_mmnp_network_type network_type;
	char url[128];
	char src_ip[16];
	char dst_ip[16];
	unsigned short src_port;
	unsigned short dst_port;
	unsigned short maxConnNum;
	unsigned short data_transmit_mode;	//0 for deviece put data to lib, 1 for lib get data from lib.
	enum vis_mmnp_packet_type packet_type;
	unsigned int maxBufSize;
	//video info
	unsigned short video_enable;
	unsigned short video_compress;
	unsigned short video_width;
	unsigned short video_height;
	unsigned int video_bitrate;
	unsigned short video_fps;
	unsigned short video_iinterval;
	//audio info
	unsigned short audio_enable;
	unsigned short audio_compress;
	unsigned int audio_samplerate;
	unsigned short audio_bits;
	unsigned short audio_channel;
	int (*Get_AVData_callback)(struct vis_mmnp_avdata *avData);	//for send
	int (*Put_AVData_callback)(struct vis_mmnp_avdata *avData);	//for recv
	int (*packet_custom)(unsigned char *dest, struct vis_mmnp_avdata *avdata);
} Vis_Mmnp_Attrs;

typedef struct vis_mmnp_udp_dynamic_params {
	int size;
	char dst_ip[16];
	unsigned short dst_port;
} Vis_Mmnp_UDP_Dynamic_Params;

typedef enum vis_mmnp_opt_type {
	VIS_MMNP_OPT_TYPE_UNKNOWN,
	VIS_MMNP_OPT_TYPE_SET_RINGBUFFER_SIZE,
	VIS_MMNP_OPT_TYPE_GET_LOGLEVEL,
	VIS_MMNP_OPT_TYPE_SET_LOGLEVEL,
	VIS_MMNP_OPT_TYPE_GET_UDP_DYNAMIC_PARAMS,
	VIS_MMNP_OPT_TYPE_SET_UDP_DYNAMIC_PARAMS,
	VIS_MMNP_OPT_TYPE_GET_AUDIO_SAMPLERATE,
	VIS_MMNP_OPT_TYPE_SET_AUDIO_SAMPLERATE,
	VIS_MMNP_OPT_TYPE_GET_CURR_CONN_NUM,
	//TODO
} Vis_Mmnp_Opt_Type;

/***********************************************************************
 * API
 ***********************************************************************/
/* Create mmnp handler */
void *vis_mmnp_create(struct vis_mmnp_attrs *attrs);
/* Create mmnp threads, init, and work */
int vis_mmnp_start(void *visMmnpHandle);
/* Stop mmnp */
int vis_mmnp_stop(void *visMmnpHandle);
/* Close mmnp handler, release */
int vis_mmnp_close(void *visMmnpHandle);
/* Get/Set mmnp opt arguments, ref vis_mmnp_opt_type */
int vis_mmnp_getopt(void *visMmnpHandle, enum vis_mmnp_opt_type optType, void *buf, size_t buflen);
int vis_mmnp_setopt(void *visMmnpHandle, enum vis_mmnp_opt_type optType, void *buf, size_t buflen);
/* Device put compressed avdata to mmnp ringbuffer */
int vis_mmnp_putdata_tolib(void *visMmnpHandle, struct vis_mmnp_avdata *avdata);

#endif	//__VIS_MMNP_H__
