/***************************************************************
 * name:visconfig.h
 * use for all vis projects, such as slave master and dialog
 * Author:
 * ver:	
 * time:2012-11-07
 * modify:
 *		ls 2012-11-27, add line 578-664, edit 136-140
 *	 	ls 2012-11-26, line 118-154
 *		ls,jzf
 *		ls 2013-04-07, chs version
 *		ls 2013-12-27, 264-267, add 4 types of VIS_BOARD_TYPE
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
本协议中使用4个字节表示的IP地址均采用的网络序，�?192.168.18.32"用四个字节表示后的值为�?x2012A8C0
关于该通信配置协议，另有专门的说明文档，若有不明白之处，请查看相关文档�?对文档的修改，请尽量保持向下兼容性，如有不兼容，请在显目处说�?****************************************************/
/*
 *	USE UDP// UDP PORT
 */
#define CLIENT_UDPPORT			4007	//编解码板用于接收广播命令的UDP端口
#define SERVER_UDPPORT			5007	//PC�?服务�?用于接收回复广播信息的UDP端口
#define CLIENT_CMDTCPPORT		4010	//编解码板用于接收普通命令的TCP监听端口
#define CLIENT_UPDATETCPPORT	8060	//编解码板用于接收程序更新命令的TCP监听端口
#define CLIENT_CONNECTTCPPORT	8066	//编解码板用于接收程序连接命令的TCP监听端口
#define SERIALCONTROLTCPPORT    8067	//编解码板用于接收串口明文传输命令的TCP监听端口1
#define CLIENT_DATATCPPORT		8088	
#define SERVER_CONNECTUDPPORT	4040	
#define CLIENT_DYNAMICTCPPORT	8068	//动态调整运行参数dynamic线程TCP监听端口
#define FIFO_VOLUME				"/mnt/apps/fifo_volume"

//******* sys *************//
//camera type 
#define VIS_RQ_SYS_SETTING			0x01  /* setting  */
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
#define	VIS_RQ_CH0_DYNAMICPARAM		VIS_RQ_CH0_BASE+0x10		//linxj 2012-05-16 //DynamicParam
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
命令说明�?VIS_RQ_BROADCAST、VIS_RS_BROADCAST  只用于广播，分别对应的两个端口CLIENT_UDPPORT、SERVER_UDPPORT
VIS_RQ_GETSTATUS、VIS_RS_SENDSTATUS 可用于广播，分别对应的两个端口CLIENT_UDPPORT、SERVER_UDPPORT
VIS_RQ_SETDEFAULT 可用于广播，使用CLIENT_UDPPORT端口
VIS_RQ_SETPROGRAM 使用CLIENT_UPDATETCPPORT端口通信
VIS_RQ_CONNECTION、VIS_RS_CONNECTIONRETURN 使用CLIENT_CONNECTTCPPORT端口通信

除VIS_RQ_BROADCAST、VIS_RS_BROADCAST、VIS_RQ_SETPROGRAM、VIS_RQ_CONNECTION、VIS_RS_CONNECTIONRETURN外，
其他命令均可用通过CLIENT_CMDTCPPORT通信
********************************************************************************************************/

//set board informations and program define
#define	VIS_NET_BASE				0xE0
#define	VIS_RQ_BROADCAST			(VIS_NET_BASE+0x00)		//server向client广播以获取client参数，无有效载荷
#define	VIS_RS_BROADCAST			(VIS_NET_BASE+0x01) 	//client向server广播自身的参数，有效载荷详见BoardInfo
#define	VIS_RQ_GETBDINFO			(VIS_NET_BASE+0x02)		//server向client请求获取板子信息，无有效载荷
#define	VIS_RS_SENDBDINFO			(VIS_NET_BASE+0x03)		//client向server发送自身的板子信息，有效载荷详见BoardInfo
#define	VIS_RQ_SETBDINFO			(VIS_NET_BASE+0x04)		//server命令client修改板子信息,tcp方式，有效载荷详见BoardInfo
#define	VIS_RQ_GETSTATUS			(VIS_NET_BASE+0x05)		//server向client请求获取板子运行状态，无有效载�?#define	VIS_RS_SENDSTATUS			(VIS_NET_BASE+0x06)		//client向server发送自身的运行状态，有效载荷详见VisStatus
#define	VIS_RQ_SETPROGRAM			(VIS_NET_BASE+0x07)		//server命令client修改板子的程序，有效载荷详见SetProgram
#define	VIS_RQ_RESTARTCMD			(VIS_NET_BASE+0x08)		//server命令client重启板子或程序，有效载荷详见RestartParam
#define	VIS_RQ_CHECKONLINE			(VIS_NET_BASE+0x09)		//server向client发送检查板子在线与否的信息，无有效载荷
#define	VIS_RS_RESPONSE				(VIS_NET_BASE+0x0A)		//client向server发送自身的运行状态，有效载荷详见vis_rs_response
#define	VIS_RQ_SETDEFAULT			(VIS_NET_BASE+0x0B)		//server命令client回复到出厂默认值，有效载荷详见vis_rq_setdefault
#define	VIS_RQ_SETNETWORK			(VIS_NET_BASE+0x0C)		//server命令client更改网路发送或接收参数，有效载荷详见NetWorking
#define	VIS_RQ_SETPERIPHERAL		(VIS_NET_BASE+0x0D)		//server命令client修改外设参数，有效载荷详见Peripheral
#define	VIS_RQ_SETVIDEOQUALITY		(VIS_NET_BASE+0x0E)		//server命令client修改视频的输入输出质量参数，有效载荷详见VideoQuality
#define	VIS_RQ_SETAUDIOVOLUME		(VIS_NET_BASE+0x0F)		//server命令client修改音频的音量参数，有效载荷详见AudioVolume
#define VIS_RQ_CONFIGBDINFO			(VIS_NET_BASE+0x10)		//server命令client修改板子信息,udp方式，有效载荷详见BoardConf
#define VIS_RQ_GETVIDEODIM			(VIS_NET_BASE+0x11)		//server向client请求获取视频在屏幕中位置，无效载�?#define VIS_RS_SENDVIDEODIM			(VIS_NET_BASE+0x12)		//client向server发送当前视频在屏幕中位置，有效载荷详见VideoDimension
#define VIS_RQ_SETVIDEODIM			(VIS_NET_BASE+0x13)		//server命令client修改视频在屏幕中位置，有效载荷详见VideoDimension
#define	VIS_RQ_SETCUSTOMSTD			(VIS_NET_BASE+0x14)		//dialog
#define	VIS_RQ_GETCUSTOMSTD			(VIS_NET_BASE+0x15)
#define VIS_RS_SENDCUSTOMSTD		(VIS_NET_BASE+0x16)
#define	VIS_RQ_CONNECTION			(VIS_NET_BASE+0x18)		//server向client请求连接或断开，有效载荷详见Connect
#define	VIS_RS_CONNECTIONRETURN		(VIS_NET_BASE+0x19)		//client向server的连接或断开请求发回反馈，有效载荷详见ConnectReturn
#define	VIS_RQ_SETSERIALDATA		(VIS_NET_BASE+0x1A)		//client向server发送串口输出的明文数据，有效载荷详见SerialControl
#define	VIS_RQ_SETUSERPARAMS		(VIS_NET_BASE+0x1B)		//client向server发送用户参数，有效载荷详见UserParams
#define	VIS_RQ_SETUSERDATA			(VIS_NET_BASE+0x1C)		//client向server发送用户数据，有效载荷详见UserData
#define	VIS_RQ_GETUSERDATA	    	(VIS_NET_BASE+0x1D)		//client向server请求发送用户数据，无有效载�?#define	VIS_RS_SENDUSERDATA			(VIS_NET_BASE+0x1E)		//server向client反馈用户数据，有效载荷详见UserData
#define VIS_RQ_GETNETWORKINGEXT		(VIS_NET_BASE+0x1F)		//client 向server请求网络扩展数据，有效载荷见NetWorkingExt

/* Some information for dapt, ls, 2012-11-26 */
#define VIS_DAPTINFO_BASE			0x30
#define	VIS_RQ_SETBOARDTIME			(VIS_DAPTINFO_BASE+0x00)		//E300 on board time
#define	VIS_RQ_GETBOARDTIME			(VIS_DAPTINFO_BASE+0x01)
#define	VIS_RS_SENDBOARDTIME		(VIS_DAPTINFO_BASE+0x02)
#define	VIS_RQ_SETOSD				(VIS_DAPTINFO_BASE+0x03)		//Encoder osd module
#define	VIS_RQ_GETOSD				(VIS_DAPTINFO_BASE+0x04)
#define	VIS_RS_SENDOSD				(VIS_DAPTINFO_BASE+0x05)
#define	VIS_RQ_SETCUSTOMTEXT		(VIS_DAPTINFO_BASE+0x06)		//text defined by customer themselves
#define	VIS_RQ_GETCUSTOMTEXT		(VIS_DAPTINFO_BASE+0x07)
#define	VIS_RS_SENDCUSTOMTEXT		(VIS_DAPTINFO_BASE+0x08)		//0x7E
#define	VIS_RQ_SETSAVESET			(VIS_DAPTINFO_BASE+0x09)		//JZF 13.1.23
#define	VIS_RQ_GETSAVESET			(VIS_DAPTINFO_BASE+0x0A)
#define	VIS_RS_SENDSAVESET			(VIS_DAPTINFO_BASE+0x0B)
#define	VIS_RQ_SETSERIALCONFIG		(VIS_DAPTINFO_BASE+0x0C)		//ls 2013-04-12
#define	VIS_RQ_GETSERIALCONFIG		(VIS_DAPTINFO_BASE+0x0D)
#define	VIS_RS_SENDSERIALCONFIG		(VIS_DAPTINFO_BASE+0x0E)

/* Account & Password for tcp connection check, ls, 2012-11-26 */
#define VIS_RQ_SETTCPACCOUNT	(VIS_DAPTINFO_BASE+0x09)
#define VIS_RQ_GETTCPACCOUNT	(VIS_DAPTINFO_BASE+0x0A)
#define VIS_RS_SENDTCPACCOUNT	(VIS_DAPTINFO_BASE+0x0B)

#define VIS_DYNAMIC_BASE 			0xC0
#define	VIS_RQ_SETI2CDATA			(VIS_DYNAMIC_BASE+0x00)	//client向server请求设置i2c数据，有效载荷详见I2cData
#define VIS_RQ_GETI2CDATA			(VIS_DYNAMIC_BASE+0x01)	//client向server请求获取i2c数据，无有效载荷
#define	VIS_RS_SENDI2CDATA			(VIS_DYNAMIC_BASE+0x02)	//server向client反馈发送i2c数据，有效载荷详见I2cData
#define	VIS_RQ_GETSERIALDATA		(VIS_DYNAMIC_BASE+0x03)	//client向server请求获取串口数据，无有效载荷
#define	VIS_RS_SENDSERIALDATA		(VIS_DYNAMIC_BASE+0x04)	//server向client反馈发送i2c数据，有效载荷详见SerialControl
#define	VIS_RQ_GETRUNNINGSTATUS		(VIS_DYNAMIC_BASE+0x05)	//client向server请求获取板子状态，无有效载�?#define	VIS_RS_SENDRUNNINGSTATUS	(VIS_DYNAMIC_BASE+0x06)	//server向client反馈发送板子的运行状态，有效载荷详见SDK_RUNNING_STATUS
#define	VIS_RQ_GETSLAVESTATUS		(VIS_DYNAMIC_BASE+0x07)	//Jzf 2012-11-9
#define	VIS_RS_GETSLAVESTATUS		(VIS_DYNAMIC_BASE+0x08)	//jzf 2012-11-9
#define	VIS_RQ_SETSPIDATA			(VIS_DYNAMIC_BASE+0x09)	//client向server请求设置spi数据，有效载荷详见SpiData, ls 2013-03-12
#define	VIS_RQ_GETSPIDATA			(VIS_DYNAMIC_BASE+0x0a)	//client向server请求获取spi数据，无有效载荷
#define	VIS_RS_SENDSPIDATA			(VIS_DYNAMIC_BASE+0x0b)	//server向client反馈发送spi数据，有效载荷详见SpiData
//////////////////////////////////////////////////////////////////////////

/* size说明,结构体中第一个元素为size的，则该size表示这个结构体的大小(Byte)，可用于区分不同版本的结构体 */

typedef	struct vis_con{
	char	vis_con_syn0;		//synchronization first byte
	char	vis_con_syn1;		//synchronization second byte
	char	vis_con_syn2;		//synchronization third byte
	unsigned char vis_con_cmd;	//command
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
	VIDEO_COMPRESS_WMV,		//
	VIDEO_COMPRESS_AVS,		//
};

enum AUDIO_COMPRESS{
	AUDIO_COMPRESS_PCM=0,	//pcm
	AUDIO_COMPRESS_ADPCM,	//
	AUDIO_COMPRESS_AC3,		//
	AUDIO_COMPRESS_AAC,		//
	AUDIO_COMPRESS_G721,	//
	AUDIO_COMPRESS_G729,	//
	AUDIO_COMPRESS_MP3,		//
	AUDIO_COMPRESS_WMA,		//
};

typedef struct vis_rq_video_setting{
	int		size;
	char	vis_rq_video_compress;		//视频编码格式，如H.264、MPEG4、MPEG2、AVS、WMV，详见VIDEO_COMPRESS
	char	vis_rq_video_type;
	char	vis_rq_video_mode;
	char	vis_rq_video_bitrate;		//是否采用CBR，否则使用VBR
	char	vis_rq_video_quality;		//
	char	vis_rq_video_fps;			//帧率
	short	vis_rq_video_kbps;			//输出码流大小, 单位kbps
	int		vis_rq_video_idrinterval;	//I帧间�?	int		vis_rq_video_width;			//the video width
	int		vis_rq_video_height;		//the video height
	int		vis_rq_video_enable;		//视频使能�?:关闭�?:打开
}Vis_rq_video_setting,*PVIS_RQ_VIDEO_SETTING,VideoSetting,*pVideoSetting;

typedef struct vis_rq_audio_setting{
	int		size;
	char 	vis_rq_audio_compress;	//编码格式，如PCM、ADPCM、MP3、AC3、AAC、G721、G729等等，详见AUDIO_COMPRESS
	char 	vis_rq_audio_channel;	//单声道or双声�?	short	vis_rq_audio_kbps;		//输出码流比特率，�?28(kbps)
	int		vis_rq_audio_sample;	//采样率，�?4100表示44.1K
	int		vis_rq_audio_enable;	//音频使能�?:关闭�?:打开
}Vis_rq_audio_setting,*PVIS_RQ_AUDIO_SETTING,AudioSetting,*pAudioSetting;

//default type
enum DEFAULT_TYPE{
	DEFAULT_TYPE_NONE=0,	//无效
	DEFAULT_TYPE_ALL,		//所有的信息恢复到出厂默认�?	
	DEFAULT_TYPE_IPCONFIG,	//IP MAC等恢复到出厂默认�?
};
typedef struct vis_rq_setdefault{
	int		size;
	int		DstIP;				//接收命令的IP，在使用广播时才有意义。IP==0时无效，IP==-1表示所有IP都接�?	int		Default_Type;		//参看DEFAULT_TYPE
}vis_rq_setdefault,*PVIS_RQ_SETDEFAULT;

//response type
enum RESPONSE_TYPE_UPDATE{
	RESPONSE_TYPE_UPDATE_FAIL=0,	//对更新的应答，表示更新失�?	RESPONSE_TYPE_UPDATE_OK,		//对更新的应答，表示更新失�?	RESPONSE_TYPE_ADDFILE_FAIL,		//对更�?添加文件)的应答，表示添加失败
	RESPONSE_TYPE_ADDFILE_OK,		//对更�?添加文件)的应答，表示添加成功
	RESPONSE_TYPE_DELFILE_FAIL,		//对更�?删除文件)的应答，表示删除失败
	RESPONSE_TYPE_DELFILE_OK,		//对更�?删除文件)的应答，表示删除成功	
	RESPONSE_TYPE_SENDTOCLIENT,		//2012.11.2 jiangzhifei send file to client
	RESPONSE_TYPE_GETFILE_FAIL,		//2012.11.2 jiangzhifei get file fail
	RESPONSE_TYPE_GETFILE_OK,		//2012.11.2 jiangzhifei get file ok
};

typedef  struct  vis_rs_response
{
	int	 size;
	int	 vis_con_data_responseType;		//应答的命令类型，对应于server端发出的命令，如VIS_RQ_SETPROGRAM
	int	 vis_con_data_response_result;	//针对不同的命令类型，回复不同的结�?
}vis_rs_response,*PVIS_RS_RESPONSE;

enum VIS_BOARD{
	VIS_BOARD_UNKOWN = 0,	//
	VIS_BOARD_ENC,			//编码�?	VIS_BOARD_DEC,			//解码�?	VIS_BOARD_VGAENC,		//支持VGA接口的编码板
	VIS_BOARD_VGADEC,		//支持VGA接口的解码板
	VIS_BOARD_HDMIENC,		//支持HDMI接口的编码板
	VIS_BOARD_HDMIDEC,		//支持HDMI接口的解码板
	VIS_BOARD_SDIENC,		//支持SDI接口的编码板
	VIS_BOARD_SDIDEC,		//支持SDI接口的解码板
	VIS_BOARD_DVIENC,		//支持DVI接口的编码板
	VIS_BOARD_DVIDEC,		//支持DVI接口的解码板
	VIS_BOARD_STREAM,		//同时支持编解码的电路�?	VIS_BOARD_STORE,		//支持存储的电路板
};

enum EXPARAMS{
	EXPARAMS_ENC_MAIN = 0,	//
	EXPARAMS_ENC_BACKUP,	//
};

typedef  struct BOARDINFO 
{
	int  size;						//sizeof()
	int  IPAddr;					//电路板的IP地址 0:DHCP
	int  IPMask; 					//电路板的子网掩码
	int  GateIP;					//电路板的网关地址
	int	 DNS1;  					//电路板的主DNS地址
	int  DNS2;						//电路板的次DNS地址
	unsigned char MAC[8];			//电路板的MAC地址
	char	BoardName[32];			//电路板的名字
	int		Boardtype;				//电路板的类型，该参数由板子决定，PC不可修改，详见VIS_BOARD_xxx_xxxx
	short	BoardVersion_Major;		//电路板的主版本号，该参数由板子决定，PC不可修改
	short	BoardVersion_Minor;		//电路板的次版本号，该参数由板子决定，PC不可修改
	short	KernelVersion_Major;	//内核的主版本号，该参数由板子决定，PC不可修改
	short	KernelVersion_Minor;	//内核的次版本号，该参数由板子决定，PC不可修改	
	short	AppVersion_Major;		//程序的主版本号，该参数由板子决定，PC不可修改
	short	AppVersion_Minor;		//程序的次版本号，该参数由板子决定，PC不可修改
	unsigned int BoardSN;			//电路板的序列号，唯一编号，该参数由板子决定，PC不可修改
	int		ExParams;				//扩展参数，如编码器分主机、备份机，详见EXPARAMS
} BoardInfo,*pBoardInfo;

typedef struct BOARDCONF
{
	int size;
	int DstIP;		//接收命令的IP，在使用广播时才有意义�?时无效，-1所有IP都接�? -2编码板接�? -3解码板接�?	BoardInfo bdinfo;
} BoardConf, *pBoardConf;

enum NETSEND_TYPE {		//电路板对外接口通信协议
	NETSEND_TYPE_RES0,	//0号保�?	NETSEND_TYPE_UDP,	//TS流，UDP协议：包括单播、组播、广播、点�?	NETSEND_TYPE_RES2,	//2号保�?	NETSEND_TYPE_TCP,	//TS流，TCP协议
	NETSEND_TYPE_C1,	//威乾协议
	NETSEND_TYPE_RTSP,	//RTSP协议
	NETSEND_TYPE_RTMP,	//RTMP协议
	NETSEND_TYPE_HTTP,	//HTTP协议
	NETSEND_TYPE_ONVIF,	//ONVIF协议
	NETSEND_TYPE_VSIP	//VSIP协议
};

enum NETRECV_TYPE {		//电路板对外接口通信协议
	NETRECV_TYPE_RES0,	//0号保�?	NETRECV_TYPE_UDP,	//TS流，UDP协议：包括单播、组播、广播、点�?	NETRECV_TYPE_RES2,	//2号保�?	NETRECV_TYPE_TCP,	//TS流，TCP协议
	NETRECV_TYPE_RTSP,	//RTSP协议
	NETRECV_TYPE_RES3,	//保留
	NETRECV_TYPE_RTMP,	//RTMP协议
	NETRECV_TYPE_HTTP,	//HTTP协议
	NETRECV_TYPE_ONVIF,	//ONVIF协议
	NETRECV_TYPE_VSIP,	//VSIP协议
	NETRECV_TYPE_C1=201	//威乾协议
};

typedef  struct NETWORKING 
{
	int				size;				//sizeof()
	int				DstUDPIPAddr;		//server端UDP发送的目标IP，可能是组播地址、广播地址、单播地址。编码端使用，当编解码通信使用UDP协议时有�?	unsigned short	DstUDPPORT;			//server端UDP发送的目标端口PORT。编码端使用，当编解码通信使用UDP协议时有�?	short			sendType;			//server端UDP发送的类型�?:正常发�? 1:组播发�? 3:TCP发�? 4:点播
	int				UDPMulticastIPAddr;	//client端UDP接收的组播IP。解码端使用，当编解码通信使用UDP协议且使用组播时有效
	unsigned short	UDPPORT;			//client端UDP接收的端口号。解码端使用，当编解码通信使用UDP协议时有�?	short			recvType;			//client端UDP接收的类型�?:正常接收;1:组播接收;2:点播接收;3:TCP接收;4:直接连编码板(根据编码板配�?;101:动态切�?201:接收visiondigi数据
	int				ServerIPAddr;		//client端TCP连接的server IP。解码端使用，当编解码通信使用TCP连接时有�?	unsigned short	ServerPORT;			//client端TCP连接的server PORT。解码端使用，当编解码通信使用TCP连接时有�?	unsigned short	TCPPORT;			//server端TCP监听的端口号。编码端使用，当编解码通信使用TCP连接时有�?}NetWorking,*pNetWorking;

enum str_type {
	STR_TYPE_UNKNOW = -1,
	STR_TYPE_DEFAULT,
	STR_TYPE_URL,
	STR_TYPE_ACCOUNT,
	STR_TYPE_PASSWORD
};

typedef struct NETWORKING_EXT {
	int size;
	int strType;
	char str1[128];		//rtsp URL
	char str2[32];		//ACCOUNT
	char str3[32];		//PASSWORD
} NetWorkingExt, *pNetWorkingExt;

typedef struct NETWORKING_NEW {
	struct NETWORKING		net_ori;
	struct NETWORKING_EXT	net_ext;
}NetWorkingNew, *pNetWorkingNew;

enum VIDEO_TYPE{		//采集类型
	VIDEO_TYPE_UNKNOWN =0,			//采集类型
	VIDEO_TYPE_CAPTURE_CVBS =1,		//采集CVBS信号
	VIDEO_TYPE_CAPTURE_COMPOSITE,	//采集COMPOSITE信号
	VIDEO_TYPE_CAPTURE_VGA,			//采集VGA信号
	VIDEO_TYPE_CAPTURE_YPbPr ,		//采集Ypbpr信号
	VIDEO_TYPE_CAPTURE_YPbPr50i,	//采集Ypbpr信号 1080i@50
	VIDEO_TYPE_CAPTURE_YPbPr60i,	//采集Ypbpr信号 1080i@60
	VIDEO_TYPE_CAPTURE_YPbPr50p,	//采集Ypbpr信号 720P@50 or 1080P
	VIDEO_TYPE_CAPTURE_YPbPr60p,	//采集Ypbpr信号 720P@60 or 1080P
	VIDEO_TYPE_CAPTURE_YPbPr25p,	//采集Ypbpr信号720p or 1080P
	VIDEO_TYPE_CAPTURE_YPbPr30p,	//采集Ypbpr信号720p or 1080P
	VIDEO_TYPE_CAPTURE_DVI,			//采集DVI信号
	VIDEO_TYPE_CAPTURE_HDMI,		//采集HDMI信号
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
	VIDEO_TYPE_CAPTURE_VGA_CUSTOMER,	//linxj 2013-03-22 for E200
	VIDEO_TYPE_CAPTURE_HDMI_CUSTOMER,	//linxj 2013-03-22 for E200	 
	VIDEO_TYPE_DISPLAY_CVBS=0x0081,		//显示类型，CVBS输出
	VIDEO_TYPE_DISPLAY_COMPOSITE,		//视频使用COMPOSITE信号输出
	VIDEO_TYPE_DISPLAY_VGA,				//视频使用VGA信号输出
	VIDEO_TYPE_DISPLAY_YPbPr,			//视频使用Ypbpr信号输出
	VIDEO_TYPE_DISPLAY_DVI,				//视频使用DVI信号输出
	VIDEO_TYPE_DISPLAY_HDMI,			//视频使用HDMI信号输出
	VIDEO_TYPE_DISPLAY_LCD,				//视频使用LCD信号输出
	VIDEO_TYPE_DISPLAY_HDMI_55,			//ls, 2012-08-27, for hdmi_dec on pc
	VIDEO_TYPE_DISPLAY_HDMI_70,
	VIDEO_TYPE_DISPLAY_HDMI_72,
	VIDEO_TYPE_DISPLAY_HDMI_75,
	VIDEO_TYPE_DISPLAY_HDMI_85,
	VIDEO_TYPE_DISPLAY_HDMI_50i,
	VIDEO_TYPE_DISPLAY_HDMI_50p,
	VIDEO_TYPE_DISPLAY_HDMI_60i,
};

enum AUDIO_TYPE{
	AUDIO_TYPE_MONO =0,			//单声�?	AUDIO_TYPE_STEREO=0x0080,	//双声�?立体�?	AUDIO_TYPE_HDMI  =0x0100,	//audio from HDMI port, linxj 2012-12-21
	AUDIO_TYPE_SDI   =0x0200,	//audio from SDI port, linxj 2013-03-13
};

typedef  struct  Peripheral
{
	int		size;
    short	VideoType;		//视频采集或显示的参数，详见枚举类型VIDEO_TYPE
	short	VideoWidth;		//视频采集或显示的宽度
	short	VideoHeight;	//视频采集或显示的高度
	short	AudioType;		//音频输入或输出的类型，详见枚举类型AUDIO_TYPE
	int		AudioSample;	//音频输入或输出的频率，如44100表示44.1K	
}Peripheral,*pPeripheral;

typedef  struct  VideoQuality
{
	int		size;
    short	autogain;	//视频参数采用自动增益，该功能暂保�?	short	bright;		//视频输入或输出的亮度大小(0-100)�? 取值范�?-100，正常为50
	short	hue;		//视频输入或输出的色度大小(0-100)�? 取值范�?-100，正常为50
	short	satution;	//视频输入或输出的饱和度大�?0-100)，取值范�?-100，正常为50	
}VideoQuality,*pVideoQuality;

typedef struct VideoDimension {
    int size;
    short width;	//视频在屏幕中宽度
    short height;	//视频在屏幕中高度
    short basex;	//有效视频在显示缓冲区中x坐标
    short basey;	//有效视频在显示缓冲区中y坐标
    short hp;		//视频在屏幕中的左右位�?    short vp;		//视频在屏幕中的上下位�?    short hint;		//horizontal interval
    short vint;		//vertical interval
    int reg;		//register address -1无效�?0有效
    int val;		//register value -1无效�?=0有效
    int exparams1;	//reserved
    int exparams2;	//reserved
} VideoDimension, *pVideoDimension;

typedef  struct  AudioVolume
{
	int		size;
    short	autogain;	//音频音量采用自动增益，该功能暂保�?	short	volume;		//音频输入或输出的音量大小(0-100)，取值范�?-100，正常为50
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
	RESTART_TYPE_BOARD =0,	//板子重启
	RESTART_TYPE_PID_ENC,	//编码进程重启
	RESTART_TYPE_PID_DEC,	//解码进程重启
};
typedef  struct  RESTARTPARAM
{
	int		size;
	int		DstIP;			//接收命令的IP，在使用广播时才有意义。IP==0时无效，IP==-1表示所有IP都接�?	int		RestartType;	//重启的类型，重启板子or重启某个进程
}RestartParam,*pRestartParam;

enum UPDATE_TYPE{
	UPDATE_TYPE_COSTUMER =0,	//自定义，由文件内容的数据自行决定
	UPDATE_TYPE_DM642BIN,		//针对DM642板子的程序文�?	UPDATE_TYPE_DM642BOOT,		//针对DM642板子BOOT的程序文件，BOOT在bin的基础上添加了一些信�?	UPDATE_TYPE_ADDNEWFILE,		//针对linux系统添加新的程序文件，若该文件已存在则不创建，文件名参考filename参数
	UPDATE_TYPE_ADDFILE,		//针对linux系统添加程序文件，若该文件已存在则覆盖它，文件名参考filename参数
	UPDATE_TYPE_DELFILE,		//针对linux系统删除某个文件，文件名参考filename参数
	UPDATE_TYPE_GETFILE,		//2012.11.2 jiangzhifei get file from server
	UPDATE_TYPE_SENDTOCLIENT,	//2012.11.2 jiangzhifei send file to client
	UPDATE_TYPE_MASTER=0x100,	//主进程的程序文件
	UPDATE_TYPE_SLAVE,			//子进程的程序文件
	UPDATE_TYPE_UPDATE,			//更新程序进程的程序文�?	UPDATE_TYPE_KERNEL,			//内核的程序文�?	UPDATE_TYPE_SYSCONFIG,		//系统配置的程序文�?	UPDATE_TYPE_NETCONFIG,		//网络配置的程序文�?	UPDATE_TYPE_CMEMK,			//cmemk.ko的程序文�?	UPDATE_TYPE_EDMAK,			//edmak.ko的程序文�?	UPDATE_TYPE_IRQK,			//irqk.ko的程序文�?	UPDATE_TYPE_MMAPK,			//mmapk.ko的程序文�?	UPDATE_TYPE_APP,			//整个应用程序的压缩包文件
};
typedef  struct  SETPROGRAM
{
	int		size;				//该命令的有效载荷长度，包括文件的内容
    int		prgfilelen;			//程序文件的长�?	short	ver_major;			//程序主版本号
	short	ver_minor;			//程序次版本号
	int		prgtype;			//程序文件的类型，参加枚举类型UPDATE_TYPE
	char	prgfilename[256];	//创建文件时使用的文件�?    //unsigned char prgfilebite[xxxx];	//程序文件的内容，长度若干
}SetProgram,*pSetProgram;

enum CONNECTION_STATUS_TYPE{
	CONNECTION_STATUS_NOERROR,	//连接成功
	CONNECTION_STATUS_PARAM,	//连接的参数设置有�?	CONNECTION_STATUS_INVALID,	//当前状态不可用，不支持该连接方�?	CONNECTION_STATUS_TOOMUCH,	//当前的连接数量过�?	CONNECTION_STATUS_NOTINLIST,//表示需断开的连接不在链表中
	CONNECTION_STATUS_UNKNOWN,	//表示未知的错�?};
typedef  struct CONNECTION
{
	int		size;				//该命令的有效载荷长度
	int		senddataType;		//连接成功后数据流的发送方式，0:自适应,根据反馈信息觉得  1:TCP 2:UDP
	int		IPaddr;				//连接端的IP地址�?:自适应
	unsigned short	UDPPORT;	//连接端的UDP接收端口
	short	connectEnable;		//连接使能�?:表示连接 0:表示断开 -1:表示断开所有连�?}Connection, *pConnection;

typedef  struct CONNECTIONRETURN
{
	int		size;				//该命令的有效载荷长度
	int		senddataType;		//连接成功后数据流的发送方式，0:自适应  1:TCP 2:UDP
	int		IPaddr;				//client的IP地址�?:自适应，TCP连接方式有效
	unsigned short	TCPPORT;	//client的TCP监听端口，TCP连接方式有效
	short	connectCounter;		//支持的最大连接数
	int		connectStatus;		//连接状态，详见CONNECTION_STATUS_TYPE
}ConnectionReturn, *pConnectionReturn;

typedef struct USERPARAMS {
    int size;
    int iFrame;		//是否插入I帧，0:否，1:�?    int exparam1;
    int exparam2;
}UserParams, *pUserParams;

typedef struct USERDATA {
    int size;
    int dataLen;				//数据长度
    unsigned char data[64];		//自定义数�?    int checked;				//校验结果�?：校验不通过�?：校验通过
    int exparam2;
} UserData, *pUserData;


//i2c数据
typedef struct I2CDATA {
    int size;
    short   i2caddr;            //i2c地址，范�?-255
    short   dataLen;            //数据长度0-256
    int     type;               //
    unsigned char reg[256];     //寄存器地址
    unsigned char val[256];     //数据�?} I2cData, *pI2cData;

typedef struct spi_data {
    int size;
    short   spiaddr;            //invalid in spi
    short   dataLen;            //data len, 0-128
    int     type;               //valid in return, 1 for valid data   
    unsigned short reg[128];    //register addr
    unsigned short val[128];    //register value
} SpiData, *pSpiData;

//master serial port
typedef struct SERIALCONTROL {
    int				size;
    int				dataLen;	//
    short			comno;		//
	unsigned short	tcpport;	//
    int				null;		//
    unsigned int	baudrate;	//发送波特率
    unsigned char	data[1024];	//需要发送的有效数据
    int				exparam1;
    int				exparam2;
} SerialControl, *pSerialControl;

//slave dynamic port
typedef struct DYNAMIC_PARAM {
    int size;
    int videobitrate;			//视频码率 0:不变�?    int videofps;				//视频帧率 0:不变
	int videoidrinterval;		//IDR帧间�?0:不变
    int audiobitrate;			//音频码率
    int audiosamplerate;		//音频采样�?    int exparam0;
    int exparam1;
    int exparam2;
} DynamicParam, *pDynamicParam;

typedef struct RUNNING_STATUS {
	int				size;
	unsigned int	poweronnum;		//重启次数
	int				connecttednum;	//客户服务相应连接�?	char			videoinput;		//视频使能 0:关闭
	char			audioinput;		//音频使能 0:关闭
	char			null1;
	char			null2;
	unsigned int	captureframe;		//采集帧数
	unsigned int	videoframe;			//视频帧数�?	int				videobitrate;		//视频码率
	int				videofps;			//视频帧率
	int				videoidrinterval;	//IDR帧间�?	int				audiobitrate;		//音频码率
	int				audiosamplerate;	//音频采样�?	/* 资源统计 */
	unsigned int	cputotal;			//cpu运行总时间，微秒
	unsigned int	cpuused;			//cpu工作计时，微�?	unsigned int	cpupercent;			//cpu占用百分�? 0-1000
	unsigned int	memtotal;			//虚拟内存总量
	unsigned int	memused;			//内存占用
	unsigned int	mempercent;			//内存占用百分�? 0-1000
    int 			exparam0;
    int			exparam1;
    int			exparam2;
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
    unsigned short  fieldlength;	// 1/256 field
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
	int mday;		//day of month
	int yday;		//day of year
	int wday;		//day of week
	int hour;
	int min;
	int sec;
	int usec;
	int isdst;		//daylight saving time
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
	int				enable;			//display osd? 0 or 1, -1 for unchange
	OSDAutoTextType	auto_type;
	int				xpos;			//position
	int				ypos;
	unsigned short	text[64];		//osd content, customers define
	unsigned int	text_len;		//text char counts
	int				text_tran;		//osd transparency, 0~100 percent
	Color			text_color;		//text color
	int				text_size;		//zoom, percent
	int				bg_tran;		//background transparency, 0~100 percent
	Color			bg_color;		//osd background color
	int				bg_width;
	int				bg_height;
} OSDParam, *pOSDParam;

typedef struct encode_osd {
	OSDopt		opt;
	int			channel;	//channel index, valid when multichannels
	OSDParam	content;
	OSDParam	time;
} EncodeOSD, *pEncodeOSD;

#define SAVE_ENABLE_DEFAULT	0
typedef struct save_set {
	unsigned int max_file_size;
	unsigned int res_dev_size;
	unsigned int max_data_size;
	unsigned int frames_per_save;
	int			 save_enable;
} Saveset, *pSaveset;

typedef struct strorage_dev_info {	//size measure in MB
	unsigned int max_file_size;		//single file max size
	unsigned int res_dev_size;		//min free size must keep on dev
	unsigned int max_data_size;		//all data file max size
	unsigned int frames_per_save;	
	unsigned int total_dev_size;	//dev total size
	unsigned int usage_dev_size;	//dev usage size
	unsigned int free_dev_size;		//dev free size
	unsigned int data_dev_size;		//dev data size
	unsigned int file_count;		//data file count
	int			 save_enable;
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
	FIFO_M2S_VIDEO_DYNAMICPARAMS = 0x200,
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

typedef struct serial_config {
	int protocol;
	unsigned int const_baudrate;
	int ip;		//dst pc ip
	int port;	//dst pc port
	int res[4];	//res
} Serial_Config, *pSerial_Conifg;

/* Master to Slave Params var 2, use configure file */
#define SLAVE_CONFIGURE_FILE1 "/mnt/apps/configFile/viscfgEnc.system"
#define SLAVE_CONFIGURE_FILE2 "/mnt/apps/configFile/viscfgEnc.system2"
#define VISCODEC_CONFIGURE_FILE_SYNC "<Viscodec Configure File Ver 2.0>"

#endif	//end of __VISCONFIG_H__
