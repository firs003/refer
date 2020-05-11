/*************************************************************
 * Head file for slave process send its status back to master
 * write: ls, 2012-11-05
 * ver:
 * modify:
 *************************************************************/

#define __SLAVE_STATUS__
#ifdef __SLAVE_STATUS__
enum  SlaveStatusType{
	SlaveStatusType_Capture,
	SlaveStatusType_Video,
	SlaveStatusType_Audio,
	SlaveStatusType_Resize,
	SlaveStatusType_Writer,
	SlaveStatusType_RszWriter,
	SlaveStatusType_Network,
};

typedef struct capture_status {
	int 	type;
	int		size;
	char	capture_compress;		//ÊÓÆµ±àÂë¸ñÊ½£¬ÈçH.264¡¢MPEG4¡¢MPEG2¡¢AVS¡¢WMV£¬Ïê¼ûcapture_COMPRESS
	char	capture_type;
	char	capture_mode;
	char	capture_bitrate;			//ÊÇ·ñ²ÉÓÃCBR£¬·ñÔòÊ¹ÓÃVBR
	char	capture_quality;			//
	char	capture_fps;				//Ö¡ÂÊ
	short	capture_kbps;				//Êä³öÂëÁ÷´óÐ¡
	int		capture_idrinterval;		//IÖ¡¼ä¸ô
	int		capture_width;				//the capture width
	int		capture_height;			//the capture height
	int		capture_enable;			//ÊÓÆµÊ¹ÄÜ£¬0:¹Ø±Õ£»1:´ò¿ª
} CaptureStatus, *pCaptureStatus;

typedef struct video_status {
	int		type;
	int		size;
	char	video_compress;		//ÊÓÆµ±àÂë¸ñÊ½£¬ÈçH.264¡¢MPEG4¡¢MPEG2¡¢AVS¡¢WMV£¬Ïê¼ûVIDEO_COMPRESS
	char	video_type;
	char	video_mode;
	char	video_bitrate;			//ÊÇ·ñ²ÉÓÃCBR£¬·ñÔòÊ¹ÓÃVBR
	char	video_quality;			//
	char	video_fps;				//Ö¡ÂÊ
	short	video_kbps;				//Êä³öÂëÁ÷´óÐ¡
	int		video_idrinterval;		//IÖ¡¼ä¸ô
	int		video_width;				//the video width
	int		video_height;			//the video height
	int		video_enable;			//ÊÓÆµÊ¹ÄÜ£¬0:¹Ø±Õ£»1:´ò¿ª
} VideoStatus, *pVideoStatus;

typedef struct audio_status {
	int		type;
	int		size;
	char 	vis_rq_audio_compress;	//±àÂë¸ñÊ½£¬ÈçPCM¡¢ADPCM¡¢MP3¡¢AC3¡¢AAC¡¢G721¡¢G729µÈµÈ£¬Ïê¼ûAUDIO_COMPRESS
	char 	vis_rq_audio_channel;	//µ¥ÉùµÀorË«ÉùµÀ
	short	vis_rq_audio_kbps;		//Êä³öÂëÁ÷±ÈÌØÂÊ£¬Èç128(kbps)
	int		vis_rq_audio_sample;	//²ÉÑùÂÊ£¬Èç44100±íÊ¾44.1K
	int		vis_rq_audio_enable;	//ÒôÆµÊ¹ÄÜ£¬0:¹Ø±Õ£»1:´ò¿ª
} AudioStatus, *pAudioStatus;

typedef struct resize_status {
	int		type;
	int		size;
	char	resize_compress;		//ÊÓÆµ±àÂë¸ñÊ½£¬ÈçH.264¡¢MPEG4¡¢MPEG2¡¢AVS¡¢WMV£¬Ïê¼ûresize_COMPRESS
	char	resize_type;
	char	resize_mode;
	char	resize_bitrate;			//ÊÇ·ñ²ÉÓÃCBR£¬·ñÔòÊ¹ÓÃVBR
	char	resize_quality;			//
	char	resize_fps;				//Ö¡ÂÊ
	short	resize_kbps;				//Êä³öÂëÁ÷´óÐ¡
	int		resize_idrinterval;		//IÖ¡¼ä¸ô
	int		resize_width;				//the resize width
	int		resize_height;			//the resize height
	int		resize_enable;			//ÊÓÆµÊ¹ÄÜ£¬0:¹Ø±Õ£»1:´ò¿ª
} ResizeStatus, *pResizeStatus;

typedef struct writer_status {
	int				type;
	int				size;
} WriterStatus, *pWriterStatus;

typedef struct rszwriter_status {
	int 			type;
	int				size;
} RszWriterStatus, *pRszWriterStatus;

typedef struct network_status {
	int				type;
	int				size;				//sizeof()
	int				DstUDPIPAddr;		//server¶ËUDP·¢ËÍµÄÄ¿±êIP£¬¿ÉÄÜÊÇ×é²¥µØÖ·¡¢¹ã²¥µØÖ·¡¢µ¥²¥µØÖ·¡£±àÂë¶ËÊ¹ÓÃ£¬µ±±à½âÂëÍ¨ÐÅÊ¹ÓÃUDPÐ­ÒéÊ±ÓÐÐ§
	unsigned short	DstUDPPort;			//server¶ËUDP·¢ËÍµÄÄ¿±ê¶Ë¿ÚPORT¡£±àÂë¶ËÊ¹ÓÃ£¬µ±±à½âÂëÍ¨ÐÅÊ¹ÓÃUDPÐ­ÒéÊ±ÓÐÐ§
	short			sendType;			//server¶ËUDP·¢ËÍµÄÀàÐÍ¡£0:Õý³£·¢ËÍ; 1:×é²¥·¢ËÍ; 3:TCP·¢ËÍ; 4:µã²¥
	int				UDPMulticastIPAddr;	//client¶ËUDP½ÓÊÕµÄ×é²¥IP¡£½âÂë¶ËÊ¹ÓÃ£¬µ±±à½âÂëÍ¨ÐÅÊ¹ÓÃUDPÐ­ÒéÇÒÊ¹ÓÃ×é²¥Ê±ÓÐÐ§
	unsigned short	UDPPort;			//client¶ËUDP½ÓÊÕµÄ¶Ë¿ÚºÅ¡£½âÂë¶ËÊ¹ÓÃ£¬µ±±à½âÂëÍ¨ÐÅÊ¹ÓÃUDPÐ­ÒéÊ±ÓÐÐ§
	short			recvType;		   	//client¶ËUDP½ÓÊÕµÄÀàÐÍ¡£0:Õý³£½ÓÊÕ;1:×é²¥½ÓÊÕ;2:µã²¥½ÓÊÕ;3:TCP½ÓÊÕ;4:Ö±½ÓÁ¬±àÂë°å(¸ù¾Ý±àÂë°åÅäÖÃ);101:¶¯Ì¬ÇÐ»»;201:½ÓÊÕvisiondigiÊý¾Ý
	int				ServerIPAddr;		//client¶ËTCPÁ¬½ÓµÄserver IP¡£½âÂë¶ËÊ¹ÓÃ£¬µ±±à½âÂëÍ¨ÐÅÊ¹ÓÃTCPÁ¬½ÓÊ±ÓÐÐ§
	unsigned short	ServerPort;			//client¶ËTCPÁ¬½ÓµÄserver PORT¡£½âÂë¶ËÊ¹ÓÃ£¬µ±±à½âÂëÍ¨ÐÅÊ¹ÓÃTCPÁ¬½ÓÊ±ÓÐÐ§
	unsigned short	TCPPort;			//server¶ËTCP¼àÌýµÄ¶Ë¿ÚºÅ¡£±àÂë¶ËÊ¹ÓÃ£¬µ±±à½âÂëÍ¨ÐÅÊ¹ÓÃTCPÁ¬½ÓÊ±ÓÐÐ§
} NetworkStatus, *pNetworkStatus;

typedef struct slavestatus {
	CaptureStatus	capture;
	VideoStatus		video;
	AudioStatus		audio;
	ResizeStatus	resize;
	WriterStatus	writer;
	RszWriterStatus	rszwriter;
	NetworkStatus	network;
} SlaveStatus, *pSlaveStatus;
#endif
