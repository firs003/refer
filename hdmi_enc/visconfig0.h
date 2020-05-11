#ifndef	__VISCONFIG_H__
#define __VISCONFIG_H__

/*******
Copy right @ viscodec http://www.viscodec.com
Version V2.0
Date:2010/03/19
Author: linxj(zerolinxj@gmail.com)
 *******/
/**************** Readme ***************************
��Э����ʹ��4���ֽڱ�ʾ��IP��ַ�����õ���������"192.168.18.32"���ĸ��ֽڱ�ʾ���ֵΪ��0x2012A8C0
���ڸ�ͨ������Э�飬����ר�ŵ�˵���ĵ������в�����֮������鿴����ĵ���
���ĵ����޸ģ��뾡���������¼����ԣ����в����ݣ�������Ŀ��˵��
****************************************************/
/*
 *	USE UDP// UDP PORT
 */
#define CLIENT_UDPPORT			4007	//���������ڽ��չ㲥�����UDP�˿�
#define SERVER_UDPPORT			5007	//PC��(������)���ڽ��ջظ��㲥��Ϣ��UDP�˿�
#define CLIENT_CMDTCPPORT		4010	//���������ڽ�����ͨ�����TCP�����˿�
#define CLIENT_UPDATETCPPORT	8060	//���������ڽ��ճ�����������TCP�����˿�
#define CLIENT_CONNECTTCPPORT	8066	//���������ڽ��ճ������������TCP�����˿�
#define SERIALCONTROLTCPPORT    8067	//���������ڽ��մ������Ĵ��������TCP�����˿�

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
����˵����
VIS_RQ_BROADCAST��VIS_RS_BROADCAST  ֻ���ڹ㲥���ֱ��Ӧ�������˿�CLIENT_UDPPORT��SERVER_UDPPORT
VIS_RQ_GETSTATUS��VIS_RS_SENDSTATUS �����ڹ㲥���ֱ��Ӧ�������˿�CLIENT_UDPPORT��SERVER_UDPPORT
VIS_RQ_SETDEFAULT �����ڹ㲥��ʹ��CLIENT_UDPPORT�˿�
VIS_RQ_SETPROGRAM ʹ��CLIENT_UPDATETCPPORT�˿�ͨ��
VIS_RQ_CONNECTION��VIS_RS_CONNECTIONRETURN ʹ��CLIENT_CONNECTTCPPORT�˿�ͨ��

��VIS_RQ_BROADCAST��VIS_RS_BROADCAST��VIS_RQ_SETPROGRAM��VIS_RQ_CONNECTION��VIS_RS_CONNECTIONRETURN�⣬
�������������ͨ��CLIENT_CMDTCPPORTͨ��
********************************************************************************************************/

//set board informations and program define
#define VIS_NET_BASE  0xE0
#define VIS_RQ_BROADCAST		(VIS_NET_BASE+0x00)		//server��client�㲥�Ի�ȡclient����������Ч�غ�
#define VIS_RS_BROADCAST		(VIS_NET_BASE+0x01) 	//client��server�㲥����Ĳ�������Ч�غ����BoardInfo
#define VIS_RQ_GETBDINFO		(VIS_NET_BASE+0x02)		//server��client�����ȡ������Ϣ������Ч�غ�
#define VIS_RS_SENDBDINFO		(VIS_NET_BASE+0x03)		//client��server��������İ�����Ϣ����Ч�غ����BoardInfo
#define VIS_RQ_SETBDINFO		(VIS_NET_BASE+0x04)		//server����client�޸İ�����Ϣ,tcp��ʽ����Ч�غ����BoardInfo
#define VIS_RQ_GETSTATUS		(VIS_NET_BASE+0x05)		//server��client�����ȡ��������״̬������Ч�غ�
#define VIS_RS_SENDSTATUS		(VIS_NET_BASE+0x06)		//client��server�������������״̬����Ч�غ����VisStatus
#define VIS_RQ_SETPROGRAM		(VIS_NET_BASE+0x07)		//server����client�޸İ��ӵĳ�����Ч�غ����SetProgram
#define VIS_RQ_RESTARTCMD		(VIS_NET_BASE+0x08)		//server����client�������ӻ������Ч�غ����RestartParam
#define VIS_RQ_CHECKONLINE		(VIS_NET_BASE+0x09)		//server��client���ͼ���������������Ϣ������Ч�غ�
#define VIS_RS_RESPONSE			(VIS_NET_BASE+0x0A)		//client��server�������������״̬����Ч�غ����vis_rs_response
#define	VIS_RQ_SETDEFAULT		(VIS_NET_BASE+0x0B)		//server����client�ظ�������Ĭ��ֵ����Ч�غ����vis_rq_setdefault
#define	VIS_RQ_SETNETWORK		(VIS_NET_BASE+0x0C)		//server����client������·���ͻ���ղ�������Ч�غ����NetWorking
#define	VIS_RQ_SETPERIPHERAL	(VIS_NET_BASE+0x0D)		//server����client�޸������������Ч�غ����Peripheral
#define	VIS_RQ_SETVIDEOQUALITY	(VIS_NET_BASE+0x0E)		//server����client�޸���Ƶ���������������������Ч�غ����VideoQuality
#define	VIS_RQ_SETAUDIOVOLUME	(VIS_NET_BASE+0x0F)		//server����client�޸���Ƶ��������������Ч�غ����AudioVolume
#define VIS_RQ_CONFIGBDINFO		(VIS_NET_BASE+0x10)		//server����client�޸İ�����Ϣ,udp��ʽ����Ч�غ����BoardConf
#define VIS_RQ_GETVIDEODIM	    (VIS_NET_BASE+0x11)		//server��client�����ȡ��Ƶ����Ļ��λ�ã���Ч�غ�
#define VIS_RS_SENDVIDEODIM     (VIS_NET_BASE+0x12)		//client��server���͵�ǰ��Ƶ����Ļ��λ�ã���Ч�غ����VideoDimension
#define VIS_RQ_SETVIDEODIM      (VIS_NET_BASE+0x13)		//server����client�޸���Ƶ����Ļ��λ�ã���Ч�غ����VideoDimension
#define	VIS_RQ_CONNECTION		(VIS_NET_BASE+0x18)		//server��client�������ӻ�Ͽ�����Ч�غ����Connect
#define	VIS_RS_CONNECTIONRETURN	(VIS_NET_BASE+0x19)		//client��server�����ӻ�Ͽ����󷢻ط�������Ч�غ����ConnectReturn
#define	VIS_RQ_SETSERIALDATA	(VIS_NET_BASE+0x1A)		//client��server���ʹ���������������ݣ���Ч�غ����SerialControl
#define	VIS_RQ_SETUSERPARAMS	(VIS_NET_BASE+0x1B)		//client��server�����û���������Ч�غ����UserParams
#define	VIS_RQ_SETUSERDATA	    (VIS_NET_BASE+0x1C)		//client��server�����û����ݣ���Ч�غ����UserData
#define	VIS_RQ_GETUSERDATA	    (VIS_NET_BASE+0x1D)		//client��server�������û����ݣ�����Ч�غ�
#define	VIS_RS_SENDUSERDATA	    (VIS_NET_BASE+0x1E)		//server��client�����û����ݣ���Ч�غ����UserData


//////////////////////////////////////////////////////////////////////////

/// size˵��,�ṹ���е�һ��Ԫ��Ϊsize�ģ����size��ʾ����ṹ��Ĵ�С(Byte)�����������ֲ�ͬ�汾�Ľṹ�� ///

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
	char	vis_rq_video_compress;		//��Ƶ�����ʽ����H.264��MPEG4��MPEG2��AVS��WMV�����VIDEO_COMPRESS
	char	vis_rq_video_type;
	char	vis_rq_video_mode;
	char	vis_rq_video_bitrate;			//�Ƿ����CBR������ʹ��VBR
	char	vis_rq_video_quality;			//
	char	vis_rq_video_fps;				//֡��
	short	vis_rq_video_kbps;				//���������С
	int		vis_rq_video_idrinterval;		//I֡���
	int		vis_rq_video_width;				//the video width
	int		vis_rq_video_height;			//the video height
	int		vis_rq_video_enable;			//��Ƶʹ�ܣ�0:�رգ�1:��
}Vis_rq_video_setting,*PVIS_RQ_VIDEO_SETTING,VideoSetting,*pVideoSetting;

typedef struct vis_rq_audio_setting{
	int		size;
	char 	vis_rq_audio_compress;	//�����ʽ����PCM��ADPCM��MP3��AC3��AAC��G721��G729�ȵȣ����AUDIO_COMPRESS
	char 	vis_rq_audio_channel;	//������or˫����
	short	vis_rq_audio_kbps;		//������������ʣ���128(kbps)
	int		vis_rq_audio_sample;	//�����ʣ���44100��ʾ44.1K
	int		vis_rq_audio_enable;	//��Ƶʹ�ܣ�0:�رգ�1:��
}Vis_rq_audio_setting,*PVIS_RQ_AUDIO_SETTING,AudioSetting,*pAudioSetting;

//default type
enum DEFAULT_TYPE{
	DEFAULT_TYPE_NONE=0,		//��Ч
	DEFAULT_TYPE_ALL,			//���е���Ϣ�ָ�������Ĭ��ֵ	
	DEFAULT_TYPE_IPCONFIG,		//IP MAC�Ȼָ�������Ĭ��ֵ	
};
typedef struct vis_rq_setdefault{
	int		size;
	int		DstIP;				//���������IP����ʹ�ù㲥ʱ�������塣IP==0ʱ��Ч��IP==-1��ʾ����IP������
	int		Default_Type;		//�ο�DEFAULT_TYPE
}vis_rq_setdefault,*PVIS_RQ_SETDEFAULT;

//response type
enum RESPONSE_TYPE_UPDATE{
	RESPONSE_TYPE_UPDATE_FAIL=0,	//�Ը��µ�Ӧ�𣬱�ʾ����ʧ��
	RESPONSE_TYPE_UPDATE_OK,		//�Ը��µ�Ӧ�𣬱�ʾ���³ɹ�
	RESPONSE_TYPE_ADDFILE_FAIL,		//�Ը���(����ļ�)��Ӧ�𣬱�ʾ���ʧ��
	RESPONSE_TYPE_ADDFILE_OK,		//�Ը���(����ļ�)��Ӧ�𣬱�ʾ��ӳɹ�
	RESPONSE_TYPE_DELFILE_FAIL,		//�Ը���(ɾ���ļ�)��Ӧ�𣬱�ʾɾ��ʧ��
	RESPONSE_TYPE_DELFILE_OK,		//�Ը���(ɾ���ļ�)��Ӧ�𣬱�ʾɾ���ɹ�	
};
typedef  struct  vis_rs_response
{
	int		size;
    int		vis_con_data_responseType;		//Ӧ����������ͣ���Ӧ��server�˷����������VIS_RQ_SETPROGRAM
	int		vis_con_data_response_result;	//��Բ�ͬ���������ͣ��ظ���ͬ�Ľ��
}vis_rs_response,*PVIS_RS_RESPONSE;

enum VIS_BOARD{
	VIS_BOARD_UNKOWN = 0,	//
	VIS_BOARD_ENC,			//�����
	VIS_BOARD_DEC,			//�����
	VIS_BOARD_VGAENC,		//֧��VGA�ӿڵı����
	VIS_BOARD_VGADEC,		//֧��VGA�ӿڵĽ����
};
enum EXPARAMS{
	EXPARAMS_ENC_MAIN = 0,	//
	EXPARAMS_ENC_BACKUP,	//
};
//
typedef  struct BOARDINFO 
{
	int  size;						//sizeof()
	int  IPAddr;					//��·���IP��ַ 0:DHCP
	int  IPMask; 					//��·�����������
	int  GateIP;					//��·������ص�ַ
	int	 DNS1;  					//��·�����DNS��ַ
	int  DNS2;						//��·��Ĵ�DNS��ַ
	unsigned char MAC[8];			//��·���MAC��ַ
	char	BoardName[32];			//��·�������
	int		Boardtype;				//��·������ͣ��ò����ɰ��Ӿ�����PC�����޸ģ����VIS_BOARD_xxx_xxxx
	short	BoardVersion_Major;		//��·������汾�ţ��ò����ɰ��Ӿ�����PC�����޸�
	short	BoardVersion_Minor;		//��·��Ĵΰ汾�ţ��ò����ɰ��Ӿ�����PC�����޸�
	short	KernelVersion_Major;	//�ں˵����汾�ţ��ò����ɰ��Ӿ�����PC�����޸�
	short	KernelVersion_Minor;	//�ں˵Ĵΰ汾�ţ��ò����ɰ��Ӿ�����PC�����޸�	
	short	AppVersion_Major;		//��������汾�ţ��ò����ɰ��Ӿ�����PC�����޸�
	short	AppVersion_Minor;		//����Ĵΰ汾�ţ��ò����ɰ��Ӿ�����PC�����޸�
	unsigned int BoardSN;			//��·������кţ�Ψһ��ţ��ò����ɰ��Ӿ�����PC�����޸�
	int		ExParams;				//��չ������������������������ݻ������EXPARAMS
} BoardInfo,*pBoardInfo;

typedef struct BOARDCONF
{
    int size;
	int DstIP;		        //���������IP����ʹ�ù㲥ʱ�������塣0ʱ��Ч��-1����IP������, -2��������, -3��������
    BoardInfo bdinfo;
} BoardConf, *pBoardConf;

typedef  struct NETWORKING 
{
	int		size;				//sizeof()
	int		DstUDPIPAddr;		//server��UDP���͵�Ŀ��IP���������鲥��ַ���㲥��ַ��������ַ�������ʹ�ã��������ͨ��ʹ��UDPЭ��ʱ��Ч
	short	DstUDPPORT;			//server��UDP���͵�Ŀ��˿�PORT�������ʹ�ã��������ͨ��ʹ��UDPЭ��ʱ��Ч
	short	sendType;			//server��UDP���͵����͡�0:��������; 1:�鲥����; 3:TCP����; 4:�㲥
	int		UDPMulticastIPAddr;	//client��UDP���յ��鲥IP�������ʹ�ã��������ͨ��ʹ��UDPЭ����ʹ���鲥ʱ��Ч
	short	UDPPORT;			//client��UDP���յĶ˿ںš������ʹ�ã��������ͨ��ʹ��UDPЭ��ʱ��Ч
	short	recvType;		   	//client��UDP���յ����͡�0:��������;1:�鲥����;2:�㲥����;3:TCP����;4:ֱ���������(���ݱ��������);101:��̬�л�;201:����visiondigi����
	int		ServerIPAddr;		//client��TCP���ӵ�server IP�������ʹ�ã��������ͨ��ʹ��TCP����ʱ��Ч
	short	ServerPORT;			//client��TCP���ӵ�server PORT�������ʹ�ã��������ͨ��ʹ��TCP����ʱ��Ч
	short	TCPPORT;			//server��TCP�����Ķ˿ںš������ʹ�ã��������ͨ��ʹ��TCP����ʱ��Ч
}NetWorking,*pNetWorking;

enum VIDEO_TYPE{
	VIDEO_TYPE_UNKNOWN =0,			//�ɼ�����
	VIDEO_TYPE_CAPTURE_CVBS =1,		//�ɼ�CVBS�ź�
	VIDEO_TYPE_CAPTURE_COMPOSITE ,	//�ɼ�COMPOSITE�ź�
	VIDEO_TYPE_CAPTURE_VGA,			//�ɼ�VGA�ź�
	VIDEO_TYPE_CAPTURE_YPbPr ,		//�ɼ�Ypbpr�ź�
	VIDEO_TYPE_CAPTURE_YPbPr50i,	//�ɼ�Ypbpr�ź�
	VIDEO_TYPE_CAPTURE_YPbPr60i,	//�ɼ�Ypbpr�ź�
	VIDEO_TYPE_CAPTURE_YPbPr50p,	//�ɼ�Ypbpr�ź�
	VIDEO_TYPE_CAPTURE_YPbPr60p,	//�ɼ�Ypbpr�ź�
	VIDEO_TYPE_CAPTURE_DVI,			//�ɼ�DVI�ź�
	VIDEO_TYPE_CAPTURE_HDMI,		//�ɼ�HDMI�ź�
	VIDEO_TYPE_DISPLAY_CVBS=0x0081,	//��ʾ���ͣ�CVBS���
	VIDEO_TYPE_DISPLAY_COMPOSITE,	//��Ƶʹ��COMPOSITE�ź����
	VIDEO_TYPE_DISPLAY_VGA,			//��Ƶʹ��VGA�ź����
	VIDEO_TYPE_DISPLAY_YPbPr,		//��Ƶʹ��Ypbpr�ź����
	VIDEO_TYPE_DISPLAY_DVI,			//��Ƶʹ��DVI�ź����
	VIDEO_TYPE_DISPLAY_HDMI,		//��Ƶʹ��HDMI�ź����
	VIDEO_TYPE_DISPLAY_LCD,			//
};

enum AUDIO_TYPE{
	AUDIO_TYPE_MONO =0,			//������
	AUDIO_TYPE_STEREO=0x0080,	//˫���� ������
};
typedef  struct  Peripheral
{
	int		size;
    short	VideoType;			//��Ƶ�ɼ�����ʾ�Ĳ��������ö������VIDEO_TYPE
	short	VideoWidth;			//��Ƶ�ɼ�����ʾ�Ŀ��
	short	VideoHeight;		//��Ƶ�ɼ�����ʾ�ĸ߶�
	short	AudioType;			//��Ƶ�������������ͣ����ö������AUDIO_TYPE
	int		AudioSample;		//��Ƶ����������Ƶ�ʣ���44100��ʾ44.1K	
}Peripheral,*pPeripheral;


typedef  struct  VideoQuality
{
	int		size;
    short	autogain;			//��Ƶ���������Զ����棬�ù����ݱ���
	short	bright;				//��Ƶ�������������ȴ�С(0-100)��  ȡֵ��Χ0-100������Ϊ50
	short	hue;				//��Ƶ����������ɫ�ȴ�С(0-100)��  ȡֵ��Χ0-100������Ϊ50
	short	satution;			//��Ƶ���������ı��Ͷȴ�С(0-100)��ȡֵ��Χ0-100������Ϊ50	
}VideoQuality,*pVideoQuality;

typedef struct VideoDimension {
    int size;
    short width;        //��Ƶ����Ļ�п��
    short height;       //��Ƶ����Ļ�и߶�
    short basex;        //��Ч��Ƶ����ʾ��������x����
    short basey;        //��Ч��Ƶ����ʾ��������y����
    short hp;           //��Ƶ����Ļ�е�����λ��
    short vp;           //��Ƶ����Ļ�е�����λ��
    short hint;         //horizontal interval
    short vint;         //vertical interval
    int reg;   //register address -1��Ч��>0��Ч
    int val;   //register value -1��Ч��>=0��Ч
    int exparams1;      //reserved
    int exparams2;      //reserved
} VideoDimension, *pVideoDimension;

typedef  struct  AudioVolume
{
	int		size;
    short	autogain;			//��Ƶ���������Զ����棬�ù����ݱ���
	short	volume;				//��Ƶ����������������С(0-100)��ȡֵ��Χ0-100������Ϊ50	
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
	RESTART_TYPE_BOARD =0,			//��������
	RESTART_TYPE_PID_ENC,			//�����������
	RESTART_TYPE_PID_DEC,			//�����������
};
typedef  struct  RESTARTPARAM
{
	int		size;
	int		DstIP;			//���������IP����ʹ�ù㲥ʱ�������塣IP==0ʱ��Ч��IP==-1��ʾ����IP������
	int		RestartType;	//���������ͣ���������or����ĳ������
}RestartParam,*pRestartParam;

enum UPDATE_TYPE{
	UPDATE_TYPE_COSTUMER =0,			//�Զ��壬���ļ����ݵ��������о���
	UPDATE_TYPE_DM642BIN,				//���DM642���ӵĳ����ļ�
	UPDATE_TYPE_DM642BOOT,				//���DM642����BOOT�ĳ����ļ���BOOT��bin�Ļ����������һЩ��Ϣ
	UPDATE_TYPE_ADDNEWFILE,				//���linuxϵͳ����µĳ����ļ��������ļ��Ѵ����򲻴������ļ����ο�filename����
	UPDATE_TYPE_ADDFILE,				//���linuxϵͳ��ӳ����ļ��������ļ��Ѵ����򸲸������ļ����ο�filename����
	UPDATE_TYPE_DELFILE,				//���linuxϵͳɾ��ĳ���ļ����ļ����ο�filename����
	UPDATE_TYPE_MASTER=0x100,			//�����̵ĳ����ļ�
	UPDATE_TYPE_SLAVE,					//�ӽ��̵ĳ����ļ�
	UPDATE_TYPE_UPDATE,					//���³�����̵ĳ����ļ�
	UPDATE_TYPE_KERNEL,					//�ں˵ĳ����ļ�
	UPDATE_TYPE_SYSCONFIG,				//ϵͳ���õĳ����ļ�
	UPDATE_TYPE_NETCONFIG,				//�������õĳ����ļ�
	UPDATE_TYPE_CMEMK,					//cmemk.ko�ĳ����ļ�
	UPDATE_TYPE_EDMAK,					//edmak.ko�ĳ����ļ�
	UPDATE_TYPE_IRQK,					//irqk.ko�ĳ����ļ�
	UPDATE_TYPE_MMAPK,					//mmapk.ko�ĳ����ļ�
	UPDATE_TYPE_APP,					//����Ӧ�ó����ѹ�����ļ�
};
typedef  struct  SETPROGRAM
{
	int		size;				//���������Ч�غɳ��ȣ������ļ�������
    int		prgfilelen;			//�����ļ��ĳ���
	short	ver_major;			//�������汾��
	short	ver_minor;			//����ΰ汾��
	int		prgtype;			//�����ļ������ͣ��μ�ö������UPDATE_TYPE
	char	prgfilename[256];	//�����ļ�ʱʹ�õ��ļ���
    //unsigned char prgfilebite[xxxx];		//�����ļ������ݣ���������
}SetProgram,*pSetProgram;

enum CONNECTION_STATUS_TYPE{
	CONNECTION_STATUS_NOERROR,	//���ӳɹ�
	CONNECTION_STATUS_PARAM,	//���ӵĲ�����������
	CONNECTION_STATUS_INVALID,	//��ǰ״̬�����ã���֧�ָ����ӷ�ʽ
	CONNECTION_STATUS_TOOMUCH,	//��ǰ��������������
	CONNECTION_STATUS_NOTINLIST,//��ʾ��Ͽ������Ӳ���������
	CONNECTION_STATUS_UNKNOWN,	//��ʾδ֪�Ĵ���
};
typedef  struct CONNECTION
{
	int		size;			//���������Ч�غɳ���
	int		senddataType;	//���ӳɹ����������ķ��ͷ�ʽ��0:����Ӧ,���ݷ�����Ϣ����  1:TCP 2:UDP
	int		IPaddr;			//���Ӷ˵�IP��ַ��0:����Ӧ
	short	UDPPORT;		//���Ӷ˵�UDP���ն˿�
	short	connectEnable;	//����ʹ�ܣ�1:��ʾ���� 0:��ʾ�Ͽ� -1:��ʾ�Ͽ���������
}Connection, *pConnection;

typedef  struct CONNECTIONRETURN
{
	int		size;			//���������Ч�غɳ���
	int		senddataType;	//���ӳɹ����������ķ��ͷ�ʽ��0:����Ӧ  1:TCP 2:UDP
	int		IPaddr;			//client��IP��ַ��0:����Ӧ��TCP���ӷ�ʽ��Ч
	short	TCPPORT;		//client��TCP�����˿ڣ�TCP���ӷ�ʽ��Ч
	short	connectCounter;	//֧�ֵ����������
	int		connectStatus;	//����״̬�����CONNECTION_STATUS_TYPE
}ConnectionReturn, *pConnectionReturn;

typedef struct USERPARAMS {
    int size;
    int iFrame;         //�Ƿ����I֡��0:��1:��
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
