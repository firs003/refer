/***************************************************************
 * name:		visconfig.h
 * use for all vis projects, such as slave master and dialog
 * Author:
 * ver:	
 * time:		2012-11-07	
 * modify:
 * 	ls, 2012-11-27, add line 578-664, edit 136-140
 * 	ls, 2012-11-26, line 118-154
 * 	ls,jzf
 ***************************************************************/

#ifndef	__VISCONFIG_H__
#define __VISCONFIG_H__

/*******
Copy right @ viscodec http://www.viscodec.com
Version V2.0
Date:2010/03/19
Author: linxj(zerolinxj@gmail.com)
 *******/
/**************** Readme ***************************
??4???IP?õ"192.168.18.32"??????0x2012A8C0
????????????
????¼?????
****************************************************/
/*
 *	USE UDP// UDP PORT
 */
#define CLIENT_UDPPORT			4007	//???UDP?
#define SERVER_UDPPORT			5007	//PC()?????UDP?
#define CLIENT_CMDTCPPORT		4010	//??TCP?
#define CLIENT_UPDATETCPPORT	8060	//??TCP?
#define CLIENT_CONNECTTCPPORT	8066	//??TCP?
#define SERIALCONTROLTCPPORT    8067	//???TCP?
#define CLIENT_DATATCPPORT		8088	//?TCP?õ??????????õ
#define SERVER_CONNECTUDPPORT	4040	//PC()????UDP????
#define CLIENT_DYNAMICTCPPORT	8068	//???TCP?
#define FIFO_VOLUME				"/mnt/apps/fifo_volume"

//******* sys *************//
//camera type 
#define VIS_RQ_SYS_SETTING				0x01  /* setting  */
//channel
//request
#define	VIS_RQ_CH0_BASE				0x10
#define	VIS_RQ_CH0_START			VIS_RQ_CH0_BASE+0x00
#define	VIS_RQ_CH0_STOP				VIS_RQ_CH0_BASE+0x01
#define	VIS_RQ_CH0_VIDEOENABLE		VIS_RQ_CH0_BASE+0x02
#define	VIS_RQ_CH0_VIDEODISABLE		VIS_RQ_CH0_BASE+0x03
#define	VIS_RQ_CH0_VIDEOSETTING		VIS_RQ_CH0_BASE+0x06
#define	VIS_RQ_CH0_VIDEOFRAME		VIS_RQ_CH0_BASE+0x07
#define	VIS_RQ_CH0_VIDEOSECOND		VIS_RQ_CH0_BASE+0x08
#define	VIS_RQ_CH0_VIDEOMORE		VIS_RQ_CH0_BASE+0x09
#define	VIS_RQ_CH0_DYNAMICPARAM		VIS_RQ_CH0_BASE+0x10		//linxj 2012-05-16 //ö??? ??DynamicParam
#define	VIS_RQ_CH0_AUDIOENABLE		VIS_RQ_CH0_BASE+0x12
#define	VIS_RQ_CH0_AUDIODISABLE		VIS_RQ_CH0_BASE+0x13
#define	VIS_RQ_CH0_AUDIOSETTING		VIS_RQ_CH0_BASE+0x16
#define	VIS_RQ_CH0_AUDIOFRAME		VIS_RQ_CH0_BASE+0x17
#define	VIS_RQ_CH0_AUDIOSECOND		VIS_RQ_CH0_BASE+0x18
#define	VIS_RQ_CH0_AUDIOMORE		VIS_RQ_CH0_BASE+0x19
//response
#define VIS_RS_CH0_BASE				0x90
#define VIS_RS_CH0_VIDEOFRAME		VIS_RS_CH0_BASE+0x02
#define VIS_RS_CH0_AUDIOFRAME		VIS_RS_CH0_BASE+0x03
#define VIS_RS_CH0_VIDEOAUDIOSYN	VIS_RS_CH0_BASE+0x04
#define VIS_RS_CH0_VIDEOFRAME_KEY	VIS_RS_CH0_BASE+0x21
#define VIS_RS_CH0_VIDEOFRAME_COM	VIS_RS_CH0_BASE+0x22

//sending type
#define VIS_NETSEND_TCP				0
#define VIS_NETSEND_UDP				1

/********************************************************************************************************
?
VIS_RQ_BROADCASTVIS_RS_BROADCAST  ??????CLIENT_UDPPORTSERVER_UDPPORT
VIS_RQ_GETSTATUSVIS_RS_SENDSTATUS ?????CLIENT_UDPPORTSERVER_UDPPORT
VIS_RQ_SETDEFAULT ???CLIENT_UDPPORT?
VIS_RQ_SETPROGRAM ?CLIENT_UPDATETCPPORT??
VIS_RQ_CONNECTIONVIS_RS_CONNECTIONRETURN ?CLIENT_CONNECTTCPPORT??

VIS_RQ_BROADCASTVIS_RS_BROADCASTVIS_RQ_SETPROGRAMVIS_RQ_CONNECTIONVIS_RS_CONNECTIONRETURN?
?CLIENT_CMDTCPPORT?
********************************************************************************************************/

//set board informations and program define
#define VIS_NET_BASE  0xE0
#define VIS_RQ_BROADCAST		(VIS_NET_BASE+0x00)		//serverclient???client??
#define VIS_RS_BROADCAST		(VIS_NET_BASE+0x01) 	//clientserver????BoardInfo
#define VIS_RQ_GETBDINFO		(VIS_NET_BASE+0x02)		//serverclient????
#define VIS_RS_SENDBDINFO		(VIS_NET_BASE+0x03)		//clientserver????BoardInfo
#define VIS_RQ_SETBDINFO		(VIS_NET_BASE+0x04)		//serverclient???,tcp???BoardInfo
#define VIS_RQ_GETSTATUS		(VIS_NET_BASE+0x05)		//serverclient?????
#define VIS_RS_SENDSTATUS		(VIS_NET_BASE+0x06)		//clientserver????VisStatus
#define VIS_RQ_SETPROGRAM		(VIS_NET_BASE+0x07)		//serverclient??????SetProgram
#define VIS_RQ_RESTARTCMD		(VIS_NET_BASE+0x08)		//serverclient???RestartParam
#define VIS_RQ_CHECKONLINE		(VIS_NET_BASE+0x09)		//serverclient????
#define VIS_RS_RESPONSE			(VIS_NET_BASE+0x0A)		//clientserver????vis_rs_response
#define	VIS_RQ_SETDEFAULT		(VIS_NET_BASE+0x0B)		//serverclient?????vis_rq_setdefault
#define	VIS_RQ_SETNETWORK		(VIS_NET_BASE+0x0C)		//serverclient·????NetWorking
#define	VIS_RQ_SETPERIPHERAL	(VIS_NET_BASE+0x0D)		//serverclient???Peripheral
#define	VIS_RQ_SETVIDEOQUALITY	(VIS_NET_BASE+0x0E)		//serverclient????VideoQuality
#define	VIS_RQ_SETAUDIOVOLUME	(VIS_NET_BASE+0x0F)		//serverclient????AudioVolume
#define VIS_RQ_CONFIGBDINFO		(VIS_NET_BASE+0x10)		//serverclient???,udp???BoardConf
#define VIS_RQ_GETVIDEODIM	    (VIS_NET_BASE+0x11)		//serverclient????ã??
#define VIS_RS_SENDVIDEODIM     (VIS_NET_BASE+0x12)		//clientserver?????ã??VideoDimension
#define VIS_RQ_SETVIDEODIM      (VIS_NET_BASE+0x13)		//serverclient????ã??VideoDimension
#define VIS_RQ_SETCUSTOMSTD		(VIS_NET_BASE+0x14)		//dialogû??
#define	VIS_RQ_GETCUSTOMSTD		(VIS_NET_BASE+0x15)
#define VIS_RS_SENDCUSTOMSTD	(VIS_NET_BASE+0x16)
#define	VIS_RQ_CONNECTION		(VIS_NET_BASE+0x18)		//serverclient????Connect
#define	VIS_RS_CONNECTIONRETURN	(VIS_NET_BASE+0x19)		//clientserver???????ConnectReturn
#define	VIS_RQ_SETSERIALDATA	(VIS_NET_BASE+0x1A)		//clientserver????SerialControl
#define	VIS_RQ_SETUSERPARAMS	(VIS_NET_BASE+0x1B)		//clientserverû??UserParams
#define	VIS_RQ_SETUSERDATA	    (VIS_NET_BASE+0x1C)		//clientserverû???UserData
#define	VIS_RQ_GETUSERDATA	    (VIS_NET_BASE+0x1D)		//clientserverû???
#define	VIS_RS_SENDUSERDATA	    (VIS_NET_BASE+0x1E)		//serverclientû???UserData

/* Some information for dapt, ls, 2012-11-26 */
#define VIS_DAPTINFO_BASE		0x30
#define	VIS_RQ_SETBOARDTIME		(VIS_DAPTINFO_BASE+0x00)		//E300 on board time
#define	VIS_RQ_GETBOARDTIME		(VIS_DAPTINFO_BASE+0x01)
#define	VIS_RS_SENDBOARDTIME	(VIS_DAPTINFO_BASE+0x02)
#define	VIS_RQ_SETOSD			(VIS_DAPTINFO_BASE+0x03)		//Encoder osd module as a whole
#define	VIS_RQ_GETOSD			(VIS_DAPTINFO_BASE+0x04)
#define	VIS_RS_SENDOSD			(VIS_DAPTINFO_BASE+0x05)
#define	VIS_RQ_SETCUSTOMTEXT	(VIS_DAPTINFO_BASE+0x06)		//text defined by customer themselves
#define	VIS_RQ_GETCUSTOMTEXT	(VIS_DAPTINFO_BASE+0x07)
#define	VIS_RS_SENDCUSTOMTEXT	(VIS_DAPTINFO_BASE+0x08)		//0x7E
#define	VIS_RQ_SETSAVESET		(VIS_DAPTINFO_BASE+0x09)		//JZF 13.1.23
#define	VIS_RQ_GETSAVESET		(VIS_DAPTINFO_BASE+0x0A)
#define	VIS_RS_SENDSAVESET		(VIS_DAPTINFO_BASE+0x0B)

/* Account & Password for tcp connection check, ls, 2012-11-26 */
#define VIS_RQ_SETTCPACCOUNT	(VIS_DAPTINFO_BASE+0x09)
#define VIS_RQ_GETTCPACCOUNT	(VIS_DAPTINFO_BASE+0x0A)
#define VIS_RS_SENDTCPACCOUNT	(VIS_DAPTINFO_BASE+0x0B)

#define VIS_DYNAMIC_BASE  0xC0
#define VIS_RQ_SETI2CDATA       (VIS_DYNAMIC_BASE+0x00)     //clientserveri2c????I2cData
#define VIS_RQ_GETI2CDATA       (VIS_DYNAMIC_BASE+0x01)     //clientserver?i2c????I2cData
#define VIS_RS_SENDI2CDATA      (VIS_DYNAMIC_BASE+0x02)     //serverclienti2c????I2cData
#define	VIS_RQ_GETSERIALDATA	(VIS_DYNAMIC_BASE+0x03)		//clientserver???????
#define	VIS_RS_SENDSERIALDATA	(VIS_DYNAMIC_BASE+0x04)		//serverclient????SerialControl
#define	VIS_RQ_GETRUNNINGSTATUS	(VIS_DYNAMIC_BASE+0x05)		//clientserver?????
#define	VIS_RS_SENDRUNNINGSTATUS	(VIS_DYNAMIC_BASE+0x06)	//serverclient?????SDK_RUNNING_STATUS
#define	VIS_RQ_GETSLAVESTATUS	(VIS_DYNAMIC_BASE+0x07)		//Jzf 2012-11-9
#define	VIS_RS_GETSLAVESTATUS	(VIS_DYNAMIC_BASE+0x08)		//jzf 2012-11-9
#define VIS_RQ_SETSPIDATA       (VIS_DYNAMIC_BASE+0x09)     //ls 2013-03-12
#define VIS_RQ_GETSPIDATA       (VIS_DYNAMIC_BASE+0x0a)     //ls 2013-03-12
#define VIS_RS_SENDSPIDATA      (VIS_DYNAMIC_BASE+0x0b)     //ls 2013-03-12
//////////////////////////////////////////////////////////////////////////

/// size?,?????size?size????(Byte)????? ///

typedef	struct vis_con{
	char	vis_con_syn0;			//synchronization first byte
	char	vis_con_syn1;			//synchronization second byte
	char	vis_con_syn2;			//synchronization third byte
	unsigned char vis_con_cmd;		//command
}Vis_con,*PVIS_CON;

typedef struct vis_rq_sys_setting{
	int		size;
	char	vis_rq_send_type;
	char	vis_rq_videoenc_type;
	char	vis_rq_videoenc_fps;
	char	vis_rq_audioenc_type;
	int		vis_rq_audioenc_samplerate;
	unsigned int	vis_rq_send_dstip;		//dst ip address for sending
	unsigned int	vis_rq_send_dstport;	//dst port for sending
}Vis_rq_sys_setting,*PVIS_RQ_SYS_SETTING;

enum VIDEO_COMPRESS{
	VIDEO_COMPRESS_MPEG2=0,	//mpeg2
	VIDEO_COMPRESS_MPEG4,	//
	VIDEO_COMPRESS_H264,	//
	VIDEO_COMPRESS_WMV,	//
	VIDEO_COMPRESS_AVS,	//
};

enum AUDIO_COMPRESS{
	AUDIO_COMPRESS_PCM=0,	//pcm
	AUDIO_COMPRESS_ADPCM,	//
	AUDIO_COMPRESS_AC3,	//
	AUDIO_COMPRESS_AAC,	//
	AUDIO_COMPRESS_G721,	//
	AUDIO_COMPRESS_G729,	//
	AUDIO_COMPRESS_MP3,	//
	AUDIO_COMPRESS_WMA,	//
};

typedef struct vis_rq_video_setting{
	int		size;
	char	vis_rq_video_compress;		//??H.264MPEG4MPEG2AVSWMVVIDEO_COMPRESS
	char	vis_rq_video_type;
	char	vis_rq_video_mode;
	char	vis_rq_video_bitrate;			//?CBR?VBR
	char	vis_rq_video_quality;			//
	char	vis_rq_video_fps;				//?
	short	vis_rq_video_kbps;				//?
	int		vis_rq_video_idrinterval;		//I?
	int		vis_rq_video_width;				//the video width
	int		vis_rq_video_height;			//the video height
	int		vis_rq_video_enable;			//???0:??1:
}Vis_rq_video_setting,*PVIS_RQ_VIDEO_SETTING,VideoSetting,*pVideoSetting;

typedef struct vis_rq_audio_setting{
	int		size;
	char 	vis_rq_audio_compress;	//?PCMADPCMMP3AC3AACG721G729??AUDIO_COMPRESS
	char 	vis_rq_audio_channel;	//or?
	short	vis_rq_audio_kbps;		//?128(kbps)
	int		vis_rq_audio_sample;	//?44100?44.1K
	int		vis_rq_audio_enable;	//???0:??1:
}Vis_rq_audio_setting,*PVIS_RQ_AUDIO_SETTING,AudioSetting,*pAudioSetting;

//default type
enum DEFAULT_TYPE{
	DEFAULT_TYPE_NONE=0,		//?
	DEFAULT_TYPE_ALL,			//?????	
	DEFAULT_TYPE_IPCONFIG,		//IP MAC????	
};
typedef struct vis_rq_setdefault{
	int		size;
	int		DstIP;				//IP?ù???IP==0??IP==-1?IP
	int		Default_Type;		//?DEFAULT_TYPE
}vis_rq_setdefault,*PVIS_RQ_SETDEFAULT;

//response type
enum RESPONSE_TYPE_UPDATE{
	RESPONSE_TYPE_UPDATE_FAIL=0,	//?µ?????
	RESPONSE_TYPE_UPDATE_OK,		//?µ????³?
	RESPONSE_TYPE_ADDFILE_FAIL,		//?(?)?????
	RESPONSE_TYPE_ADDFILE_OK,		//?(?)??????
	RESPONSE_TYPE_DELFILE_FAIL,		//?(??)??????
	RESPONSE_TYPE_DELFILE_OK,		//?(??)??????
	RESPONSE_TYPE_SENDTOCLIENT,		//2012.11.2 jiangzhifei send file to client
	RESPONSE_TYPE_GETFILE_FAIL,		//2012.11.2 jiangzhifei get file fail
	RESPONSE_TYPE_GETFILE_OK,		//2012.11.2 jiangzhifei get file ok
};
typedef  struct  vis_rs_response
{
	int		size;
    int		vis_con_data_responseType;		//???server??VIS_RQ_SETPROGRAM
	int		vis_con_data_response_result;	//??????
}vis_rs_response,*PVIS_RS_RESPONSE;

enum VIS_BOARD{
	VIS_BOARD_UNKOWN = 0,	//
	VIS_BOARD_ENC,			//
	VIS_BOARD_DEC,			//
	VIS_BOARD_VGAENC,		//?VGA???
	VIS_BOARD_VGADEC,		//?VGA???
	VIS_BOARD_HDMIENC,		//?HDMI???
	VIS_BOARD_HDMIDEC,		//?HDMI???
	VIS_BOARD_SDIENC,		//?SDI???
	VIS_BOARD_SDIDEC,		//?SDI???
};
enum EXPARAMS{
	EXPARAMS_ENC_MAIN = 0,	//
	EXPARAMS_ENC_BACKUP,	//
};
//
typedef  struct BOARDINFO 
{
	int  size;						//sizeof()
	int  IPAddr;					//·IP? 0:DHCP
	int  IPMask; 					//·
	int  GateIP;					//·??
	int	 DNS1;  					//·DNS?
	int  DNS2;						//·?DNS?
	unsigned char MAC[8];			//·MAC?
	char	BoardName[32];			//·
	int		Boardtype;				//·?ò??PC??VIS_BOARD_xxx_xxxx
	short	BoardVersion_Major;		//·??ò??PC?
	short	BoardVersion_Minor;		//·????ò??PC?
	short	KernelVersion_Major;	//????ò??PC?
	short	KernelVersion_Minor;	//??????ò??PC?	
	short	AppVersion_Major;		//??ò??PC?
	short	AppVersion_Minor;		//????ò??PC?
	unsigned int BoardSN;			//·?????ò??PC?
	int		ExParams;				//??EXPARAMS
} BoardInfo,*pBoardInfo;

typedef struct BOARDCONF
{
    int size;
	int DstIP;		        //IP?ù???0??-1IP, -2, -3
    BoardInfo bdinfo;
} BoardConf, *pBoardConf;

enum NETSEND_TYPE {
	NETSEND_TYPE_RES0,
	NETSEND_TYPE_UDP,
	NETSEND_TYPE_RES2,
	NETSEND_TYPE_TCP,
	NETSEND_TYPE_C1,
	NETSEND_TYPE_RTSP
};

typedef  struct NETWORKING 
{
	int		size;				//sizeof()
	int		DstUDPIPAddr;		//serverUDP??IP??????ã??UDP???
	unsigned short	DstUDPPORT;			//serverUDP???PORT?ã??UDP???
	short	sendType;			//serverUDP??0:; 1:?; 3:TCP; 4:?
	int		UDPMulticastIPAddr;	//clientUDP??IP?ã??UDP?????
	unsigned short	UDPPORT;			//clientUDP??????ã??UDP???
	short	recvType;		   	//clientUDP??0:;1:?;2:?;3:TCP;4:?(?);101:??;201:visiondigi
	int		ServerIPAddr;		//clientTCP?server IP?ã??TCP??
	unsigned short	ServerPORT;			//clientTCP?server PORT?ã??TCP??
	unsigned short	TCPPORT;			//serverTCP?????ã??TCP??
}NetWorking,*pNetWorking;

enum VIDEO_TYPE{
	VIDEO_TYPE_UNKNOWN =0,			//?
	VIDEO_TYPE_CAPTURE_CVBS =1,		//?CVBS?
	VIDEO_TYPE_CAPTURE_COMPOSITE ,	//?COMPOSITE?
	VIDEO_TYPE_CAPTURE_VGA,			//?VGA?
	VIDEO_TYPE_CAPTURE_YPbPr ,		//?Ypbpr?
	VIDEO_TYPE_CAPTURE_YPbPr50i,	//?Ypbpr? 1080i@50
	VIDEO_TYPE_CAPTURE_YPbPr60i,	//?Ypbpr? 1080i@60
	VIDEO_TYPE_CAPTURE_YPbPr50p,	//?Ypbpr? 720P@50 or 1080P
	VIDEO_TYPE_CAPTURE_YPbPr60p,	//?Ypbpr? 720P@60 or 1080P
	VIDEO_TYPE_CAPTURE_YPbPr25p,	//?Ypbpr?720p or 1080P
	VIDEO_TYPE_CAPTURE_YPbPr30p,	//?Ypbpr?720p or 1080P
	VIDEO_TYPE_CAPTURE_DVI,			//?DVI?
	VIDEO_TYPE_CAPTURE_HDMI,		//?HDMI?
	VIDEO_TYPE_CAPTURE_HDMI_60p = VIDEO_TYPE_CAPTURE_HDMI,	//ls, 2012-11-15, deff i & p for hdmi
	VIDEO_TYPE_CAPTURE_VGA_55,		//ls, 2012-11-05, for the next 5 lines
	VIDEO_TYPE_CAPTURE_VGA_70,
	VIDEO_TYPE_CAPTURE_VGA_72,
	VIDEO_TYPE_CAPTURE_VGA_75,
	VIDEO_TYPE_CAPTURE_VGA_85,
	VIDEO_TYPE_CAPTURE_HDMI_55,		//ls, 2012-11-05, for the next 5 lines
	VIDEO_TYPE_CAPTURE_HDMI_70,
	VIDEO_TYPE_CAPTURE_HDMI_72,
	VIDEO_TYPE_CAPTURE_HDMI_75,
	VIDEO_TYPE_CAPTURE_HDMI_85,
	VIDEO_TYPE_CAPTURE_HDMI_50i,	//ls, 2012-11-15, deff i & p for hdmi
	VIDEO_TYPE_CAPTURE_HDMI_50p,
	VIDEO_TYPE_CAPTURE_HDMI_60i,
	VIDEO_TYPE_CAPTURE_SDI_60P,		//linxj 2013-03-13 for SDI encoder
	VIDEO_TYPE_CAPTURE_SDI_50P,
	VIDEO_TYPE_CAPTURE_SDI_30P,
	VIDEO_TYPE_CAPTURE_SDI_25P,
	VIDEO_TYPE_CAPTURE_SDI_24P,
	VIDEO_TYPE_CAPTURE_SDI_60I,
	VIDEO_TYPE_CAPTURE_SDI_50I,
	VIDEO_TYPE_CAPTURE_VGA_CUSTOMER,		//linxj 2013-03-22 for E200
	VIDEO_TYPE_CAPTURE_HDMI_CUSTOMER,		//linxj 2013-03-22 for E200	 
	VIDEO_TYPE_DISPLAY_CVBS=0x0081,	//??CVBS
	VIDEO_TYPE_DISPLAY_COMPOSITE,	//??COMPOSITE?
	VIDEO_TYPE_DISPLAY_VGA,			//??VGA?
	VIDEO_TYPE_DISPLAY_YPbPr,		//??Ypbpr?
	VIDEO_TYPE_DISPLAY_DVI,			//??DVI?
	VIDEO_TYPE_DISPLAY_HDMI,		//??HDMI?
	VIDEO_TYPE_DISPLAY_LCD,			//

	
};

enum AUDIO_TYPE{
	AUDIO_TYPE_MONO =0,			//
	AUDIO_TYPE_STEREO=0x0080,	//? 
	AUDIO_TYPE_HDMI  =0x0100,	//audio from HDMI port;//linxj2012-12-21
	AUDIO_TYPE_SDI   =0x0200,	//audio from SDI port;//linxj2013-03-13
};
typedef  struct  Peripheral
{
	int		size;
    short	VideoType;			//????öVIDEO_TYPE
	short	VideoWidth;			//????
	short	VideoHeight;		//?????
	short	AudioType;			//??öAUDIO_TYPE
	int		AudioSample;		//???44100?44.1K	
}Peripheral,*pPeripheral;


typedef  struct  VideoQuality
{
	int		size;
    short	autogain;			//???ù?
	short	bright;				//???(0-100)  ???0-100?50
	short	hue;				//????(0-100)  ???0-100?50
	short	satution;			//?????(0-100)???0-100?50	
}VideoQuality,*pVideoQuality;

typedef struct VideoDimension {
    int size;
    short width;        //???
    short height;       //????
    short basex;        //???x
    short basey;        //???y
    short hp;           //????
    short vp;           //????
    short hint;         //horizontal interval
    short vint;         //vertical interval
    int reg;   //register address -1?>0?
    int val;   //register value -1?>=0?
    int exparams1;      //reserved
    int exparams2;      //reserved
} VideoDimension, *pVideoDimension;

typedef  struct  AudioVolume
{
	int		size;
    short	autogain;			//???ù?
	short	volume;				//??(0-100)???0-100?50	
}AudioVolume,*pAudioVolume;

typedef  struct  VISSTATUS
{
	int		size;
	BoardInfo bdinfo;
	NetWorking network;
	Peripheral periph;
	VideoSetting video;
	AudioSetting audio;
	VideoQuality videoparam;
	AudioVolume  audioparam;
}VisStatus,*pVisStatus;

enum RESTART_TYPE{
	RESTART_TYPE_BOARD =0,			//
	RESTART_TYPE_PID_ENC,			//
	RESTART_TYPE_PID_DEC,			//
};
typedef  struct  RESTARTPARAM
{
	int		size;
	int		DstIP;			//IP?ù???IP==0??IP==-1?IP
	int		RestartType;	//?or?
}RestartParam,*pRestartParam;

enum UPDATE_TYPE{
	UPDATE_TYPE_COSTUMER =0,			//?????
	UPDATE_TYPE_DM642BIN,				//DM642???
	UPDATE_TYPE_DM642BOOT,				//DM642BOOT??BOOTbin????
	UPDATE_TYPE_ADDNEWFILE,				//linux??µ????????filename
	UPDATE_TYPE_ADDFILE,				//linux??????????filename
	UPDATE_TYPE_DELFILE,				//linux???????filename
	UPDATE_TYPE_GETFILE,				//2012.11.2 jiangzhifei get file from server
	UPDATE_TYPE_SENDTOCLIENT,			//2012.11.2 jiangzhifei send file to client
	UPDATE_TYPE_MASTER=0x100,			//???
	UPDATE_TYPE_SLAVE,					//????
	UPDATE_TYPE_UPDATE,					//³???
	UPDATE_TYPE_KERNEL,					//????
	UPDATE_TYPE_SYSCONFIG,				//??õ??
	UPDATE_TYPE_NETCONFIG,				//õ??
	UPDATE_TYPE_CMEMK,					//cmemk.ko??
	UPDATE_TYPE_EDMAK,					//edmak.ko??
	UPDATE_TYPE_IRQK,					//irqk.ko??
	UPDATE_TYPE_MMAPK,					//mmapk.ko??
	UPDATE_TYPE_APP,					//?ó??
};
typedef  struct  SETPROGRAM
{
	int		size;				//?????
    int		prgfilelen;			//??
	short	ver_major;			//?
	short	ver_minor;			//??
	int		prgtype;			//???öUPDATE_TYPE
	char	prgfilename[256];	//???õ?
    //unsigned char prgfilebite[xxxx];		//??
}SetProgram,*pSetProgram;

enum CONNECTION_STATUS_TYPE{
	CONNECTION_STATUS_NOERROR,	//??
	CONNECTION_STATUS_PARAM,	//??
	CONNECTION_STATUS_INVALID,	//???ã????
	CONNECTION_STATUS_TOOMUCH,	//?
	CONNECTION_STATUS_NOTINLIST,//???
	CONNECTION_STATUS_UNKNOWN,	//????
};
typedef  struct CONNECTION
{
	int		size;			//???
	int		senddataType;	//?????0:?,??  1:TCP 2:UDP
	int		IPaddr;			//??IP?0:?
	unsigned short	UDPPORT;		//??UDP??
	short	connectEnable;	//??1:? 0:?? -1:??
}Connection, *pConnection;

typedef  struct CONNECTIONRETURN
{
	int		size;			//???
	int		senddataType;	//?????0:?  1:TCP 2:UDP
	int		IPaddr;			//clientIP?0:?TCP???
	unsigned short	TCPPORT;		//clientTCP??TCP???
	short	connectCounter;	//??
	int		connectStatus;	//??CONNECTION_STATUS_TYPE
}ConnectionReturn, *pConnectionReturn;

typedef struct USERPARAMS {
    int size;
    int iFrame;					//?I?0:1:
    int exparam1;
    int exparam2;
}UserParams, *pUserParams;

typedef struct USERDATA {
    int size;
    int dataLen;				//?
    unsigned char data[64];		//?
    int checked;				//?0???1??
    int exparam2;
} UserData, *pUserData;

//i2c??
typedef struct I2CDATA {
    int size;
    short   i2caddr;            //i2c?0-255
    short   dataLen;            //??0-256
    int     type;               //???1???   
    unsigned char reg[256];     //??
    unsigned char val[256];     //?
} I2cData, *pI2cData;

typedef struct spi_data {
    int size;
    short   spiaddr;            //invalid in spi
    short   dataLen;            //data len，0-128
    int     type;               //valid in return, 1 for valid data   
    unsigned short reg[128];     //register addr
    unsigned short val[128];     //register value
} SpiData, *pSpiData;

//?????õ?
typedef struct SERIALCONTROL {
    int		size;
    int		dataLen;				//?? 0:???????		//???
    short	comno;					//?????????
	unsigned short	tcpport;		//tcpport???tcpport0:????
    int				null;			//?
    unsigned int	baudRate;		// 11520 ;0:????
    unsigned char	data[1024];		//?											//??
    int				exparam1;
    int				exparam2;
} SerialControl, *pSerialControl;

//?õ?
typedef struct DYNAMIC_PARAM {
    int size;
    int videobitrate;				//? 0:??
    int videofps;					//?? 0:??
	int videoidrinterval;			//?IDR0:??
    int audiobitrate;				//???
    int audiosamplerate;			//???
    int exparam0;
    int exparam1;
    int exparam2;
} DynamicParam, *pDynamicParam;

typedef struct RUNNING_STATUS {
    int		size;
	unsigned int	poweronnum;		//??
	int		connecttednum;			//?
	char	videoinput;				//??? 0:?
	char	audioinput;				//??? 0:?
	char	null1;
	char	null2;
	unsigned int captureframe;		//???
	unsigned int videoframe;		//??
    int videobitrate;				//? 
    int videofps;					//?? 
	int videoidrinterval;			//?IDR
    int audiobitrate;				//?
    int audiosamplerate;			//?
	//??
	unsigned int	cputotal;		//cpu??
	unsigned int	cpuused;		//cpu?õ?
	unsigned int	cpupercent;		//?cpu?	????  ?0-1000
	unsigned int	memtotal;		//???
	unsigned int	memused;		//???õ??
	unsigned int	mempercent;		//???	????  ?0-1000
    int exparam0;
    int exparam1;
    int exparam2;
} RunningStatus, *pRunningStatus;

#define VISCONFIG_MAXLENGTH	(sizeof(VisStatus)+sizeof(Vis_con)+16)
#define SERIALCONTROL_MAXLEN (sizeof(SerialControl)+sizeof(Vis_con)+16)

#define __VGADIALOG_H__
#ifdef __VGADIALOG_H__
typedef struct {
	int top;
	int left;
	int width;
	int height;
}Capture_VideoWin;

typedef struct {
	/* for write */
	unsigned int	width;
	unsigned int	height;
	int 			framerate;
	unsigned int	pixelClock;
	unsigned short	lineClock;
	unsigned short	lineCounter;
	short			offset_horizontal;
	short			offset_vertical;
	
	/* for read */
	int				type;
    unsigned short  blocklength;	// 8 lines
    unsigned short  lines_vs;		//lines of VS 
    unsigned short  lines_field;	//lines per field
    unsigned short  fieldlength;	//  1/256 field
}Capture_customer_param;

typedef enum TcpAccountOpt {
	VIS_RQ_TCPACCOUNT_ADD,
	VIS_RQ_TCPACCOUNT_DEL,
	VIS_RQ_TCPACCOUNT_EDIT,
	VIS_RQ_TCPACCOUNT_CHECK,
	VIS_RS_TCPACCOUNT_SUCCESS,
	VIS_RS_TCPACCOUNT_EXIST,
	VIS_RS_TCPACCOUNT_NOACCOUNT,
	VIS_RS_TCPACCOUNT_MISMATCH,
	VIS_RS_TCPACCOUNT_LOGIN,
	VIS_RS_TCPACCOUNT_LOCK,
	VIS_RS_TCPACCOUNT_BLACKLIST
}TcpAccountOpt;

typedef struct account_info {
	TcpAccountOpt	opt;
	char			org_account[32];
	char			org_password[32];
	char			account[32];
	char			password[32];
} AccountInfo, *pAccountInfo;

typedef struct board_time {
	int year;
	int month;
	int mday;	//day of month
	int yday;	//day of year
	int wday;	//day of week
	int hour;
	int min;
	int sec;
	int usec;
	int isdst;	//daylight saving time
} BoardTime, *pBoardTime;

typedef enum Color {
	RED,
	ORANGE,
	YELLOW,
	GREEN,
	BLUE,
	CYAN,
	PURPLE,
	WHITE,
	BLACK
	//TODO
}Color;

typedef enum OSDAutoTextType {
	AUTOTEXT_UNABLE = -1,
	BOARD_NAME,
	CHANNEL_NAME,
	BOARD_TIME,
	//TODO
	BOARD_TIME_CHN_12H = 100,
	BOARD_TIME_CHN_24H,
	BOARD_TIME_ENG_12H,
	BOARD_TIME_ENG_24H
	//TODO
}OSDAutoTextType;

typedef enum OSDopt {
	VIS_RQ_OSD_SETOSD,
	VIS_RQ_OSD_SETTEXT,
	VIS_RQ_OSD_SETLAYOUT
	//TODO
}OSDopt;

typedef enum osd_type {
	OSD_TYPE_CONTENT,
	OSD_TYPE_TIME
}OSDType;

typedef struct osd_param {
	OSDType			type;
	int				enable;		//display osd? 0 or 1, -1 for unchange
	OSDAutoTextType	auto_type;
	int				xpos;		//position
	int				ypos;
	unsigned short	text[64];	//osd content, customers define
	unsigned int	text_len;	//text char counts
	int				text_tran;	//osd transparency, 0~100 percent
	Color			text_color;	//text color
	int				text_size;	//zoom, percent
	int				bg_tran;	//background transparency, 0~100 percent
	Color			bg_color;	//osd background color
	int				bg_width;
	int				bg_height;
} OSDParam, *pOSDParam;

typedef struct encode_osd {
	OSDopt		opt;
	int			channel;	//channel index, valid when multichannels
	OSDParam	content;
	OSDParam	time;
} EncodeOSD, *pEncodeOSD;

typedef struct save_set {
//	int			 save_enable;
	unsigned int max_file_size;
	unsigned int res_dev_size;
	unsigned int max_data_size;
	unsigned int frames_per_save;
} Saveset, *pSaveset;

typedef struct strorage_dev_info {	//size measure in MB
//	int			 save_enable;
	unsigned int max_file_size;		//single file max size
	unsigned int res_dev_size;		//min free size must keep on dev
	unsigned int max_data_size;		//all data file max size
	unsigned int frames_per_save;	
	unsigned int total_dev_size;	//dev total size
	unsigned int usage_dev_size;	//dev usage size
	unsigned int free_dev_size;		//dev free size
	unsigned int data_dev_size;		//dev data size
	unsigned int file_count;		//data file count
} Store_Dev_Info, *pStore_Dev_Info;

#endif

/********************************************************************************************** 
 * fifo, used between master and slave
 **********************************************************************************************/
#define		FIFODIR				"/mnt/apps/dm365_encode/fifodir/"
#define		FIFO_M2S_MAIN		"/mnt/apps/dm365_encode/fifodir/fifo_m2s_main"
#define		FIFO_M2S_CAPTURE	"/mnt/apps/dm365_encode/fifodir/fifo_m2s_capture"
#define		FIFO_M2S_VIDEO		"/mnt/apps/dm365_encode/fifodir/fifo_m2s_video"
#define		FIFO_M2S_RESIZE		"/mnt/apps/dm365_encode/fifodir/fifo_m2s_resize"
#define		FIFO_M2S_WRITER		"/mnt/apps/dm365_encode/fifodir/fifo_m2s_writer"
#define		FIFO_M2S_AUDIO		"/mnt/apps/dm365_encode/fifodir/fifo_m2s_audio"
#define		FIFO_M2S_SPEECH		"/mnt/apps/dm365_encode/fifodir/fifo_m2s_speech"
#define		FIFO_M2S_UDP		"/mnt/apps/dm365_encode/fifodir/fifo_m2s_udp"
#define		FIFO_M2S_TCP		"/mnt/apps/dm365_encode/fifodir/fifo_m2s_tcp"
#define		FIFO_M2S_DEMAND		"/mnt/apps/dm365_encode/fifodir/fifo_m2s_demand"
#define		FIFO_M2S_SAVE		"/mnt/apps/dm365_encode/fifodir/fifo_m2s_save"

typedef enum fifo_m2s_type {
	FIFO_M2S_FLUSH = -1,
	//main thread
	FIFO_M2S_MAIN_ = 0x0000,
	//capture thread
	FIFO_M2S_CAPTURE_CHANGEOSD = 0x0100,
	//video thread
	//resize thread
	//writer thread
	//audio thread
	//speech thread
	//udp thread
	//tcp thread
	//demand thread
	//save thread
	FIFO_M2S_SAVE_SAVESETCHANGE = 0x0A00
}Fifo_M2S_Type;

typedef struct fifo_m2s_message {
	unsigned char sync[4];
	int type;
} Fifo_M2S_Msg;

#endif	//end of __VISCONFIG_H__
