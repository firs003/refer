#ifndef	__VISCONFIG_H__
#define __VISCONFIG_H__

/*******
Copy right @ viscodec http://www.viscodec.com
Version V2.0
Date:2010/03/19
Author: linxj(zerolinxj@gmail.com)
 *******/
/**************** Readme ***************************
本协议中使用4个字节表示的IP地址均采用的网络序，如"192.168.18.32"用四个字节表示后的值为：0x2012A8C0
关于该通信配置协议，另有专门的说明文档，若有不明白之处，请查看相关文档。
对文档的修改，请尽量保持向下兼容性，如有不兼容，请在显目处说明
****************************************************/
/*
 *	USE UDP// UDP PORT
 */
#define CLIENT_UDPPORT			4007	//编解码板用于接收广播命令的UDP端口
#define SERVER_UDPPORT			5007	//PC端(服务器)用于接收回复广播信息的UDP端口
#define CLIENT_CMDTCPPORT		4010	//编解码板用于接收普通命令的TCP监听端口
#define CLIENT_UPDATETCPPORT	8060	//编解码板用于接收程序更新命令的TCP监听端口
#define CLIENT_CONNECTTCPPORT	8066	//编解码板用于接收程序连接命令的TCP监听端口
#define SERIALCONTROLTCPPORT    8067	//编解码板用于接收串口明文传输命令的TCP监听端口

//******* sys *************//
//camera type 
#define VIS_RQ_SYS_SETTING				0x01  /* setting  */
//channel
//request
#define VIS_RQ_CH0_BASE	0x10
#define	VIS_RQ_CH0_START			VIS_RQ_CH0_BASE+0x00
#define	VIS_RQ_CH0_STOP				VIS_RQ_CH0_BASE+0x01
#define	VIS_RQ_CH0_VIDEOENABLE		VIS_RQ_CH0_BASE+0x02
#define	VIS_RQ_CH0_VIDEODISABLE		VIS_RQ_CH0_BASE+0x03
#define	VIS_RQ_CH0_VIDEOSETTING		VIS_RQ_CH0_BASE+0x06
#define	VIS_RQ_CH0_VIDEOFRAME		VIS_RQ_CH0_BASE+0x07
#define	VIS_RQ_CH0_VIDEOSECOND		VIS_RQ_CH0_BASE+0x08
#define	VIS_RQ_CH0_VIDEOMORE		VIS_RQ_CH0_BASE+0x09
#define	VIS_RQ_CH0_AUDIOENABLE		VIS_RQ_CH0_BASE+0x12
#define	VIS_RQ_CH0_AUDIODISABLE		VIS_RQ_CH0_BASE+0x13
#define	VIS_RQ_CH0_AUDIOSETTING		VIS_RQ_CH0_BASE+0x16
#define	VIS_RQ_CH0_AUDIOFRAME		VIS_RQ_CH0_BASE+0x17
#define	VIS_RQ_CH0_AUDIOSECOND		VIS_RQ_CH0_BASE+0x18
#define	VIS_RQ_CH0_AUDIOMORE		VIS_RQ_CH0_BASE+0x19
//response
#define VIS_RS_CH0_BASE	0x90
#define VIS_RS_CH0_VIDEOFRAME		VIS_RS_CH0_BASE+0x02
#define VIS_RS_CH0_AUDIOFRAME		VIS_RS_CH0_BASE+0x03
#define VIS_RS_CH0_VIDEOAUDIOSYN	VIS_RS_CH0_BASE+0x04
#define VIS_RS_CH0_VIDEOFRAME_KEY		VIS_RS_CH0_BASE+0x21
#define VIS_RS_CH0_VIDEOFRAME_COM		VIS_RS_CH0_BASE+0x22

//sending type
#define VIS_NETSEND_TCP				0
#define VIS_NETSEND_UDP				1

/********************************************************************************************************
命令说明：
VIS_RQ_BROADCAST、VIS_RS_BROADCAST  只用于广播，分别对应的两个端口CLIENT_UDPPORT、SERVER_UDPPORT
VIS_RQ_GETSTATUS、VIS_RS_SENDSTATUS 可用于广播，分别对应的两个端口CLIENT_UDPPORT、SERVER_UDPPORT
VIS_RQ_SETDEFAULT 可用于广播，使用CLIENT_UDPPORT端口
VIS_RQ_SETPROGRAM 使用CLIENT_UPDATETCPPORT端口通信
VIS_RQ_CONNECTION、VIS_RS_CONNECTIONRETURN 使用CLIENT_CONNECTTCPPORT端口通信

除VIS_RQ_BROADCAST、VIS_RS_BROADCAST、VIS_RQ_SETPROGRAM、VIS_RQ_CONNECTION、VIS_RS_CONNECTIONRETURN外，
其他命令均可用通过CLIENT_CMDTCPPORT通信
********************************************************************************************************/

//set board informations and program define
#define VIS_NET_BASE  0xE0
#define VIS_RQ_BROADCAST		(VIS_NET_BASE+0x00)		//server向client广播以获取client参数，无有效载荷
#define VIS_RS_BROADCAST		(VIS_NET_BASE+0x01) 	//client向server广播自身的参数，有效载荷详见BoardInfo
#define VIS_RQ_GETBDINFO		(VIS_NET_BASE+0x02)		//server向client请求获取板子信息，无有效载荷
#define VIS_RS_SENDBDINFO		(VIS_NET_BASE+0x03)		//client向server发送自身的板子信息，有效载荷详见BoardInfo
#define VIS_RQ_SETBDINFO		(VIS_NET_BASE+0x04)		//server命令client修改板子信息,tcp方式，有效载荷详见BoardInfo
#define VIS_RQ_GETSTATUS		(VIS_NET_BASE+0x05)		//server向client请求获取板子运行状态，无有效载荷
#define VIS_RS_SENDSTATUS		(VIS_NET_BASE+0x06)		//client向server发送自身的运行状态，有效载荷详见VisStatus
#define VIS_RQ_SETPROGRAM		(VIS_NET_BASE+0x07)		//server命令client修改板子的程序，有效载荷详见SetProgram
#define VIS_RQ_RESTARTCMD		(VIS_NET_BASE+0x08)		//server命令client重启板子或程序，有效载荷详见RestartParam
#define VIS_RQ_CHECKONLINE		(VIS_NET_BASE+0x09)		//server向client发送检查板子在线与否的信息，无有效载荷
#define VIS_RS_RESPONSE			(VIS_NET_BASE+0x0A)		//client向server发送自身的运行状态，有效载荷详见vis_rs_response
#define	VIS_RQ_SETDEFAULT		(VIS_NET_BASE+0x0B)		//server命令client回复到出厂默认值，有效载荷详见vis_rq_setdefault
#define	VIS_RQ_SETNETWORK		(VIS_NET_BASE+0x0C)		//server命令client更改网路发送或接收参数，有效载荷详见NetWorking
#define	VIS_RQ_SETPERIPHERAL	(VIS_NET_BASE+0x0D)		//server命令client修改外设参数，有效载荷详见Peripheral
#define	VIS_RQ_SETVIDEOQUALITY	(VIS_NET_BASE+0x0E)		//server命令client修改视频的输入输出质量参数，有效载荷详见VideoQuality
#define	VIS_RQ_SETAUDIOVOLUME	(VIS_NET_BASE+0x0F)		//server命令client修改音频的音量参数，有效载荷详见AudioVolume
#define VIS_RQ_CONFIGBDINFO		(VIS_NET_BASE+0x10)		//server命令client修改板子信息,udp方式，有效载荷详见BoardConf
#define VIS_RQ_GETVIDEODIM	    (VIS_NET_BASE+0x11)		//server向client请求获取视频在屏幕中位置，无效载荷
#define VIS_RS_SENDVIDEODIM     (VIS_NET_BASE+0x12)		//client向server发送当前视频在屏幕中位置，有效载荷详见VideoDimension
#define VIS_RQ_SETVIDEODIM      (VIS_NET_BASE+0x13)		//server命令client修改视频在屏幕中位置，有效载荷详见VideoDimension
#define	VIS_RQ_CONNECTION		(VIS_NET_BASE+0x18)		//server向client请求连接或断开，有效载荷详见Connect
#define	VIS_RS_CONNECTIONRETURN	(VIS_NET_BASE+0x19)		//client向server的连接或断开请求发回反馈，有效载荷详见ConnectReturn
#define	VIS_RQ_SETSERIALDATA	(VIS_NET_BASE+0x1A)		//client向server发送串口输出的明文数据，有效载荷详见SerialControl
#define	VIS_RQ_SETUSERPARAMS	(VIS_NET_BASE+0x1B)		//client向server发送用户参数，有效载荷详见UserParams
#define	VIS_RQ_SETUSERDATA	    (VIS_NET_BASE+0x1C)		//client向server发送用户数据，有效载荷详见UserData
#define	VIS_RQ_GETUSERDATA	    (VIS_NET_BASE+0x1D)		//client向server请求发送用户数据，无有效载荷
#define	VIS_RS_SENDUSERDATA	    (VIS_NET_BASE+0x1E)		//server向client反馈用户数据，有效载荷详见UserData


//////////////////////////////////////////////////////////////////////////

/// size说明,结构体中第一个元素为size的，则该size表示这个结构体的大小(Byte)，可用于区分不同版本的结构体 ///

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
	char	vis_rq_video_compress;		//视频编码格式，如H.264、MPEG4、MPEG2、AVS、WMV，详见VIDEO_COMPRESS
	char	vis_rq_video_type;
	char	vis_rq_video_mode;
	char	vis_rq_video_bitrate;			//是否采用CBR，否则使用VBR
	char	vis_rq_video_quality;			//
	char	vis_rq_video_fps;				//帧率
	short	vis_rq_video_kbps;				//输出码流大小
	int		vis_rq_video_idrinterval;		//I帧间隔
	int		vis_rq_video_width;				//the video width
	int		vis_rq_video_height;			//the video height
	int		vis_rq_video_enable;			//视频使能，0:关闭；1:打开
}Vis_rq_video_setting,*PVIS_RQ_VIDEO_SETTING,VideoSetting,*pVideoSetting;

typedef struct vis_rq_audio_setting{
	int		size;
	char 	vis_rq_audio_compress;	//编码格式，如PCM、ADPCM、MP3、AC3、AAC、G721、G729等等，详见AUDIO_COMPRESS
	char 	vis_rq_audio_channel;	//单声道or双声道
	short	vis_rq_audio_kbps;		//输出码流比特率，如128(kbps)
	int		vis_rq_audio_sample;	//采样率，如44100表示44.1K
	int		vis_rq_audio_enable;	//音频使能，0:关闭；1:打开
}Vis_rq_audio_setting,*PVIS_RQ_AUDIO_SETTING,AudioSetting,*pAudioSetting;

//default type
enum DEFAULT_TYPE{
	DEFAULT_TYPE_NONE=0,		//无效
	DEFAULT_TYPE_ALL,			//所有的信息恢复到出厂默认值	
	DEFAULT_TYPE_IPCONFIG,		//IP MAC等恢复到出厂默认值	
};
typedef struct vis_rq_setdefault{
	int		size;
	int		DstIP;				//接收命令的IP，在使用广播时才有意义。IP==0时无效，IP==-1表示所有IP都接受
	int		Default_Type;		//参看DEFAULT_TYPE
}vis_rq_setdefault,*PVIS_RQ_SETDEFAULT;

//response type
enum RESPONSE_TYPE_UPDATE{
	RESPONSE_TYPE_UPDATE_FAIL=0,	//对更新的应答，表示更新失败
	RESPONSE_TYPE_UPDATE_OK,		//对更新的应答，表示更新成功
	RESPONSE_TYPE_ADDFILE_FAIL,		//对更新(添加文件)的应答，表示添加失败
	RESPONSE_TYPE_ADDFILE_OK,		//对更新(添加文件)的应答，表示添加成功
	RESPONSE_TYPE_DELFILE_FAIL,		//对更新(删除文件)的应答，表示删除失败
	RESPONSE_TYPE_DELFILE_OK,		//对更新(删除文件)的应答，表示删除成功	
};
typedef  struct  vis_rs_response
{
	int		size;
    int		vis_con_data_responseType;		//应答的命令类型，对应于server端发出的命令，如VIS_RQ_SETPROGRAM
	int		vis_con_data_response_result;	//针对不同的命令类型，回复不同的结果
}vis_rs_response,*PVIS_RS_RESPONSE;

enum VIS_BOARD{
	VIS_BOARD_UNKOWN = 0,	//
	VIS_BOARD_ENC,			//编码板
	VIS_BOARD_DEC,			//解码板
	VIS_BOARD_VGAENC,		//支持VGA接口的编码板
	VIS_BOARD_VGADEC,		//支持VGA接口的解码板
};
enum EXPARAMS{
	EXPARAMS_ENC_MAIN = 0,	//
	EXPARAMS_ENC_BACKUP,	//
};
//
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
	int DstIP;		        //接收命令的IP，在使用广播时才有意义。0时无效，-1所有IP都接受, -2编码板接受, -3解码板接受
    BoardInfo bdinfo;
} BoardConf, *pBoardConf;

typedef  struct NETWORKING 
{
	int		size;				//sizeof()
	int		DstUDPIPAddr;		//server端UDP发送的目标IP，可能是组播地址、广播地址、单播地址。编码端使用，当编解码通信使用UDP协议时有效
	short	DstUDPPORT;			//server端UDP发送的目标端口PORT。编码端使用，当编解码通信使用UDP协议时有效
	short	sendType;			//server端UDP发送的类型。0:正常发送; 1:组播发送; 3:TCP发送; 4:点播
	int		UDPMulticastIPAddr;	//client端UDP接收的组播IP。解码端使用，当编解码通信使用UDP协议且使用组播时有效
	short	UDPPORT;			//client端UDP接收的端口号。解码端使用，当编解码通信使用UDP协议时有效
	short	recvType;		   	//client端UDP接收的类型。0:正常接收;1:组播接收;2:点播接收;3:TCP接收;4:直接连编码板(根据编码板配置);101:动态切换;201:接收visiondigi数据
	int		ServerIPAddr;		//client端TCP连接的server IP。解码端使用，当编解码通信使用TCP连接时有效
	short	ServerPORT;			//client端TCP连接的server PORT。解码端使用，当编解码通信使用TCP连接时有效
	short	TCPPORT;			//server端TCP监听的端口号。编码端使用，当编解码通信使用TCP连接时有效
}NetWorking,*pNetWorking;

enum VIDEO_TYPE{
	VIDEO_TYPE_UNKNOWN =0,			//采集类型
	VIDEO_TYPE_CAPTURE_CVBS =1,		//采集CVBS信号
	VIDEO_TYPE_CAPTURE_COMPOSITE ,	//采集COMPOSITE信号
	VIDEO_TYPE_CAPTURE_VGA,			//采集VGA信号
	VIDEO_TYPE_CAPTURE_YPbPr ,		//采集Ypbpr信号
	VIDEO_TYPE_CAPTURE_YPbPr50i,	//采集Ypbpr信号
	VIDEO_TYPE_CAPTURE_YPbPr60i,	//采集Ypbpr信号
	VIDEO_TYPE_CAPTURE_YPbPr50p,	//采集Ypbpr信号
	VIDEO_TYPE_CAPTURE_YPbPr60p,	//采集Ypbpr信号
	VIDEO_TYPE_CAPTURE_DVI,			//采集DVI信号
	VIDEO_TYPE_CAPTURE_HDMI,		//采集HDMI信号
	VIDEO_TYPE_DISPLAY_CVBS=0x0081,	//显示类型，CVBS输出
	VIDEO_TYPE_DISPLAY_COMPOSITE,	//视频使用COMPOSITE信号输出
	VIDEO_TYPE_DISPLAY_VGA,			//视频使用VGA信号输出
	VIDEO_TYPE_DISPLAY_YPbPr,		//视频使用Ypbpr信号输出
	VIDEO_TYPE_DISPLAY_DVI,			//视频使用DVI信号输出
	VIDEO_TYPE_DISPLAY_HDMI,		//视频使用HDMI信号输出
	VIDEO_TYPE_DISPLAY_LCD,			//
};

enum AUDIO_TYPE{
	AUDIO_TYPE_MONO =0,			//单声道
	AUDIO_TYPE_STEREO=0x0080,	//双声道 立体声
};
typedef  struct  Peripheral
{
	int		size;
    short	VideoType;			//视频采集或显示的参数，详见枚举类型VIDEO_TYPE
	short	VideoWidth;			//视频采集或显示的宽度
	short	VideoHeight;		//视频采集或显示的高度
	short	AudioType;			//音频输入或输出的类型，详见枚举类型AUDIO_TYPE
	int		AudioSample;		//音频输入或输出的频率，如44100表示44.1K	
}Peripheral,*pPeripheral;


typedef  struct  VideoQuality
{
	int		size;
    short	autogain;			//视频参数采用自动增益，该功能暂保留
	short	bright;				//视频输入或输出的亮度大小(0-100)，  取值范围0-100，正常为50
	short	hue;				//视频输入或输出的色度大小(0-100)，  取值范围0-100，正常为50
	short	satution;			//视频输入或输出的饱和度大小(0-100)，取值范围0-100，正常为50	
}VideoQuality,*pVideoQuality;

typedef struct VideoDimension {
    int size;
    short width;        //视频在屏幕中宽度
    short height;       //视频在屏幕中高度
    short basex;        //有效视频在显示缓冲区中x坐标
    short basey;        //有效视频在显示缓冲区中y坐标
    short hp;           //视频在屏幕中的左右位置
    short vp;           //视频在屏幕中的上下位置
    short hint;         //horizontal interval
    short vint;         //vertical interval
    int reg;   //register address -1无效，>0有效
    int val;   //register value -1无效，>=0有效
    int exparams1;      //reserved
    int exparams2;      //reserved
} VideoDimension, *pVideoDimension;

typedef  struct  AudioVolume
{
	int		size;
    short	autogain;			//音频音量采用自动增益，该功能暂保留
	short	volume;				//音频输入或输出的音量大小(0-100)，取值范围0-100，正常为50	
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
	RESTART_TYPE_BOARD =0,			//板子重启
	RESTART_TYPE_PID_ENC,			//编码进程重启
	RESTART_TYPE_PID_DEC,			//解码进程重启
};
typedef  struct  RESTARTPARAM
{
	int		size;
	int		DstIP;			//接收命令的IP，在使用广播时才有意义。IP==0时无效，IP==-1表示所有IP都接受
	int		RestartType;	//重启的类型，重启板子or重启某个进程
}RestartParam,*pRestartParam;

enum UPDATE_TYPE{
	UPDATE_TYPE_COSTUMER =0,			//自定义，由文件内容的数据自行决定
	UPDATE_TYPE_DM642BIN,				//针对DM642板子的程序文件
	UPDATE_TYPE_DM642BOOT,				//针对DM642板子BOOT的程序文件，BOOT在bin的基础上添加了一些信息
	UPDATE_TYPE_ADDNEWFILE,				//针对linux系统添加新的程序文件，若该文件已存在则不创建，文件名参考filename参数
	UPDATE_TYPE_ADDFILE,				//针对linux系统添加程序文件，若该文件已存在则覆盖它，文件名参考filename参数
	UPDATE_TYPE_DELFILE,				//针对linux系统删除某个文件，文件名参考filename参数
	UPDATE_TYPE_MASTER=0x100,			//主进程的程序文件
	UPDATE_TYPE_SLAVE,					//子进程的程序文件
	UPDATE_TYPE_UPDATE,					//更新程序进程的程序文件
	UPDATE_TYPE_KERNEL,					//内核的程序文件
	UPDATE_TYPE_SYSCONFIG,				//系统配置的程序文件
	UPDATE_TYPE_NETCONFIG,				//网络配置的程序文件
	UPDATE_TYPE_CMEMK,					//cmemk.ko的程序文件
	UPDATE_TYPE_EDMAK,					//edmak.ko的程序文件
	UPDATE_TYPE_IRQK,					//irqk.ko的程序文件
	UPDATE_TYPE_MMAPK,					//mmapk.ko的程序文件
	UPDATE_TYPE_APP,					//整个应用程序的压缩包文件
};
typedef  struct  SETPROGRAM
{
	int		size;				//该命令的有效载荷长度，包括文件的内容
    int		prgfilelen;			//程序文件的长度
	short	ver_major;			//程序主版本号
	short	ver_minor;			//程序次版本号
	int		prgtype;			//程序文件的类型，参加枚举类型UPDATE_TYPE
	char	prgfilename[256];	//创建文件时使用的文件名
    //unsigned char prgfilebite[xxxx];		//程序文件的内容，长度若干
}SetProgram,*pSetProgram;

enum CONNECTION_STATUS_TYPE{
	CONNECTION_STATUS_NOERROR,	//连接成功
	CONNECTION_STATUS_PARAM,	//连接的参数设置有误
	CONNECTION_STATUS_INVALID,	//当前状态不可用，不支持该连接方式
	CONNECTION_STATUS_TOOMUCH,	//当前的连接数量过多
	CONNECTION_STATUS_NOTINLIST,//表示需断开的连接不在链表中
	CONNECTION_STATUS_UNKNOWN,	//表示未知的错误
};
typedef  struct CONNECTION
{
	int		size;			//该命令的有效载荷长度
	int		senddataType;	//连接成功后数据流的发送方式，0:自适应,根据反馈信息觉得  1:TCP 2:UDP
	int		IPaddr;			//连接端的IP地址，0:自适应
	short	UDPPORT;		//连接端的UDP接收端口
	short	connectEnable;	//连接使能，1:表示连接 0:表示断开 -1:表示断开所有连接
}Connection, *pConnection;

typedef  struct CONNECTIONRETURN
{
	int		size;			//该命令的有效载荷长度
	int		senddataType;	//连接成功后数据流的发送方式，0:自适应  1:TCP 2:UDP
	int		IPaddr;			//client的IP地址，0:自适应，TCP连接方式有效
	short	TCPPORT;		//client的TCP监听端口，TCP连接方式有效
	short	connectCounter;	//支持的最大连接数
	int		connectStatus;	//连接状态，详见CONNECTION_STATUS_TYPE
}ConnectionReturn, *pConnectionReturn;

typedef struct USERPARAMS {
    int size;
    int iFrame;         //是否插入I帧，0:否，1:是
    int exparam1;
    int exparam2;
}UserParams, *pUserParams;

typedef struct USERDATA {
    int size;
    int dataLen;
    unsigned char data[64];
    int exparam1;
    int exparam2;
} UserData, *pUserData;

typedef struct SERIALCONTROL {
    int size;
    int dataLen;
    int dataId;
    int maxdataId;
    unsigned int baudRate;
    unsigned char data[1024];
    int exparam1;
    int exparam2;
} SerialControl, *pSerialControl;

#define VISCONFIG_MAXLENGTH	(sizeof(VisStatus)+sizeof(Vis_con)+16)
#define SERIALCONTROL_MAXLEN (sizeof(SerialControl)+sizeof(Vis_con)+16)

#endif	//end of __VISCONFIG_H__
