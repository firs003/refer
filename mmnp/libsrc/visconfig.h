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
#define SERIALCONTROLTCPPORT    8067	//���������ڽ��մ������Ĵ��������TCP�����˿�1
#define CLIENT_DATATCPPORT		8088	
#define SERVER_CONNECTUDPPORT	4040	
#define CLIENT_DYNAMICTCPPORT	8068	//��̬�������в���dynamic�߳�TCP�����˿�
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
#define	VIS_NET_BASE				0xE0
#define	VIS_RQ_BROADCAST			(VIS_NET_BASE+0x00)		//server��client�㲥�Ի�ȡclient����������Ч�غ�
#define	VIS_RS_BROADCAST			(VIS_NET_BASE+0x01) 	//client��server�㲥����Ĳ�������Ч�غ����BoardInfo
#define	VIS_RQ_GETBDINFO			(VIS_NET_BASE+0x02)		//server��client�����ȡ������Ϣ������Ч�غ�
#define	VIS_RS_SENDBDINFO			(VIS_NET_BASE+0x03)		//client��server��������İ�����Ϣ����Ч�غ����BoardInfo
#define	VIS_RQ_SETBDINFO			(VIS_NET_BASE+0x04)		//server����client�޸İ�����Ϣ,tcp��ʽ����Ч�غ����BoardInfo
#define	VIS_RQ_GETSTATUS			(VIS_NET_BASE+0x05)		//server��client�����ȡ��������״̬������Ч�غ�
#define	VIS_RS_SENDSTATUS			(VIS_NET_BASE+0x06)		//client��server�������������״̬����Ч�غ����VisStatus
#define	VIS_RQ_SETPROGRAM			(VIS_NET_BASE+0x07)		//server����client�޸İ��ӵĳ�����Ч�غ����SetProgram
#define	VIS_RQ_RESTARTCMD			(VIS_NET_BASE+0x08)		//server����client�������ӻ������Ч�غ����RestartParam
#define	VIS_RQ_CHECKONLINE			(VIS_NET_BASE+0x09)		//server��client���ͼ���������������Ϣ������Ч�غ�
#define	VIS_RS_RESPONSE				(VIS_NET_BASE+0x0A)		//client��server�������������״̬����Ч�غ����vis_rs_response
#define	VIS_RQ_SETDEFAULT			(VIS_NET_BASE+0x0B)		//server����client�ظ�������Ĭ��ֵ����Ч�غ����vis_rq_setdefault
#define	VIS_RQ_SETNETWORK			(VIS_NET_BASE+0x0C)		//server����client������·���ͻ���ղ�������Ч�غ����NetWorking
#define	VIS_RQ_SETPERIPHERAL		(VIS_NET_BASE+0x0D)		//server����client�޸������������Ч�غ����Peripheral
#define	VIS_RQ_SETVIDEOQUALITY		(VIS_NET_BASE+0x0E)		//server����client�޸���Ƶ���������������������Ч�غ����VideoQuality
#define	VIS_RQ_SETAUDIOVOLUME		(VIS_NET_BASE+0x0F)		//server����client�޸���Ƶ��������������Ч�غ����AudioVolume
#define VIS_RQ_CONFIGBDINFO			(VIS_NET_BASE+0x10)		//server����client�޸İ�����Ϣ,udp��ʽ����Ч�غ����BoardConf
#define VIS_RQ_GETVIDEODIM			(VIS_NET_BASE+0x11)		//server��client�����ȡ��Ƶ����Ļ��λ�ã���Ч�غ�
#define VIS_RS_SENDVIDEODIM			(VIS_NET_BASE+0x12)		//client��server���͵�ǰ��Ƶ����Ļ��λ�ã���Ч�غ����VideoDimension
#define VIS_RQ_SETVIDEODIM			(VIS_NET_BASE+0x13)		//server����client�޸���Ƶ����Ļ��λ�ã���Ч�غ����VideoDimension
#define	VIS_RQ_SETCUSTOMSTD			(VIS_NET_BASE+0x14)		//dialog
#define	VIS_RQ_GETCUSTOMSTD			(VIS_NET_BASE+0x15)
#define VIS_RS_SENDCUSTOMSTD		(VIS_NET_BASE+0x16)
#define	VIS_RQ_CONNECTION			(VIS_NET_BASE+0x18)		//server��client�������ӻ�Ͽ�����Ч�غ����Connect
#define	VIS_RS_CONNECTIONRETURN		(VIS_NET_BASE+0x19)		//client��server�����ӻ�Ͽ����󷢻ط�������Ч�غ����ConnectReturn
#define	VIS_RQ_SETSERIALDATA		(VIS_NET_BASE+0x1A)		//client��server���ʹ���������������ݣ���Ч�غ����SerialControl
#define	VIS_RQ_SETUSERPARAMS		(VIS_NET_BASE+0x1B)		//client��server�����û���������Ч�غ����UserParams
#define	VIS_RQ_SETUSERDATA			(VIS_NET_BASE+0x1C)		//client��server�����û����ݣ���Ч�غ����UserData
#define	VIS_RQ_GETUSERDATA	    	(VIS_NET_BASE+0x1D)		//client��server�������û����ݣ�����Ч�غ�
#define	VIS_RS_SENDUSERDATA			(VIS_NET_BASE+0x1E)		//server��client�����û����ݣ���Ч�غ����UserData
#define VIS_RQ_GETNETWORKINGEXT		(VIS_NET_BASE+0x1F)		//client ��server����������չ���ݣ���Ч�غɼ�NetWorkingExt

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
#define	VIS_RQ_SETI2CDATA			(VIS_DYNAMIC_BASE+0x00)	//client��server��������i2c���ݣ���Ч�غ����I2cData
#define VIS_RQ_GETI2CDATA			(VIS_DYNAMIC_BASE+0x01)	//client��server�����ȡi2c���ݣ�����Ч�غ�
#define	VIS_RS_SENDI2CDATA			(VIS_DYNAMIC_BASE+0x02)	//server��client��������i2c���ݣ���Ч�غ����I2cData
#define	VIS_RQ_GETSERIALDATA		(VIS_DYNAMIC_BASE+0x03)	//client��server�����ȡ�������ݣ�����Ч�غ�
#define	VIS_RS_SENDSERIALDATA		(VIS_DYNAMIC_BASE+0x04)	//server��client��������i2c���ݣ���Ч�غ����SerialControl
#define	VIS_RQ_GETRUNNINGSTATUS		(VIS_DYNAMIC_BASE+0x05)	//client��server�����ȡ����״̬������Ч�غ�
#define	VIS_RS_SENDRUNNINGSTATUS	(VIS_DYNAMIC_BASE+0x06)	//server��client�������Ͱ��ӵ�����״̬����Ч�غ����SDK_RUNNING_STATUS
#define	VIS_RQ_GETSLAVESTATUS		(VIS_DYNAMIC_BASE+0x07)	//Jzf 2012-11-9
#define	VIS_RS_GETSLAVESTATUS		(VIS_DYNAMIC_BASE+0x08)	//jzf 2012-11-9
#define	VIS_RQ_SETSPIDATA			(VIS_DYNAMIC_BASE+0x09)	//client��server��������spi���ݣ���Ч�غ����SpiData, ls 2013-03-12
#define	VIS_RQ_GETSPIDATA			(VIS_DYNAMIC_BASE+0x0a)	//client��server�����ȡspi���ݣ�����Ч�غ�
#define	VIS_RS_SENDSPIDATA			(VIS_DYNAMIC_BASE+0x0b)	//server��client��������spi���ݣ���Ч�غ����SpiData
//////////////////////////////////////////////////////////////////////////

/* size˵��,�ṹ���е�һ��Ԫ��Ϊsize�ģ����size��ʾ����ṹ��Ĵ�С(Byte)�����������ֲ�ͬ�汾�Ľṹ�� */

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
	char	vis_rq_video_compress;		//��Ƶ�����ʽ����H.264��MPEG4��MPEG2��AVS��WMV�����VIDEO_COMPRESS
	char	vis_rq_video_type;
	char	vis_rq_video_mode;
	char	vis_rq_video_bitrate;		//�Ƿ����CBR������ʹ��VBR
	char	vis_rq_video_quality;		//
	char	vis_rq_video_fps;			//֡��
	short	vis_rq_video_kbps;			//���������С, ��λkbps
	int		vis_rq_video_idrinterval;	//I֡���
	int		vis_rq_video_width;			//the video width
	int		vis_rq_video_height;		//the video height
	int		vis_rq_video_enable;		//��Ƶʹ�ܣ�0:�رգ�1:��
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
	DEFAULT_TYPE_NONE=0,	//��Ч
	DEFAULT_TYPE_ALL,		//���е���Ϣ�ָ�������Ĭ��ֵ
	DEFAULT_TYPE_IPCONFIG,	//IP MAC�Ȼָ�������Ĭ��ֵ
};
typedef struct vis_rq_setdefault{
	int		size;
	int		DstIP;				//���������IP����ʹ�ù㲥ʱ�������塣IP==0ʱ��Ч��IP==-1��ʾ����IP������
	int		Default_Type;		//�ο�DEFAULT_TYPE
}vis_rq_setdefault,*PVIS_RQ_SETDEFAULT;

//response type
enum RESPONSE_TYPE_UPDATE{
	RESPONSE_TYPE_UPDATE_FAIL=0,	//�Ը��µ�Ӧ�𣬱�ʾ����ʧ��
	RESPONSE_TYPE_UPDATE_OK,		//�Ը��µ�Ӧ�𣬱�ʾ����ʧ��
	RESPONSE_TYPE_ADDFILE_FAIL,		//�Ը���(����ļ�)��Ӧ�𣬱�ʾ���ʧ��
	RESPONSE_TYPE_ADDFILE_OK,		//�Ը���(����ļ�)��Ӧ�𣬱�ʾ��ӳɹ�
	RESPONSE_TYPE_DELFILE_FAIL,		//�Ը���(ɾ���ļ�)��Ӧ�𣬱�ʾɾ��ʧ��
	RESPONSE_TYPE_DELFILE_OK,		//�Ը���(ɾ���ļ�)��Ӧ�𣬱�ʾɾ���ɹ�	
	RESPONSE_TYPE_SENDTOCLIENT,		//2012.11.2 jiangzhifei send file to client
	RESPONSE_TYPE_GETFILE_FAIL,		//2012.11.2 jiangzhifei get file fail
	RESPONSE_TYPE_GETFILE_OK,		//2012.11.2 jiangzhifei get file ok
};

typedef  struct  vis_rs_response
{
	int	 size;
	int	 vis_con_data_responseType;		//Ӧ����������ͣ���Ӧ��server�˷����������VIS_RQ_SETPROGRAM
	int	 vis_con_data_response_result;	//��Բ�ͬ���������ͣ��ظ���ͬ�Ľ��
}vis_rs_response,*PVIS_RS_RESPONSE;

enum VIS_BOARD{
	VIS_BOARD_UNKOWN = 0,	//
	VIS_BOARD_ENC,			//�����
	VIS_BOARD_DEC,			//�����
	VIS_BOARD_VGAENC,		//֧��VGA�ӿڵı����
	VIS_BOARD_VGADEC,		//֧��VGA�ӿڵĽ����
	VIS_BOARD_HDMIENC,		//֧��HDMI�ӿڵı����
	VIS_BOARD_HDMIDEC,		//֧��HDMI�ӿڵĽ����
	VIS_BOARD_SDIENC,		//֧��SDI�ӿڵı����
	VIS_BOARD_SDIDEC,		//֧��SDI�ӿڵĽ����
	VIS_BOARD_DVIENC,		//֧��DVI�ӿڵı����
	VIS_BOARD_DVIDEC,		//֧��DVI�ӿڵĽ����
	VIS_BOARD_STREAM,		//ͬʱ֧�ֱ����ĵ�·��
	VIS_BOARD_STORE,		//֧�ִ洢�ĵ�·��
};

enum EXPARAMS{
	EXPARAMS_ENC_MAIN = 0,	//
	EXPARAMS_ENC_BACKUP,	//
};

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
	int DstIP;		//���������IP����ʹ�ù㲥ʱ�������塣0ʱ��Ч��-1����IP������, -2��������, -3��������
	BoardInfo bdinfo;
} BoardConf, *pBoardConf;

enum NETSEND_TYPE {		//��·�����ӿ�ͨ��Э��
	NETSEND_TYPE_RES0,	//0�ű���
	NETSEND_TYPE_UDP,	//TS����UDPЭ�飺�����������鲥���㲥���㲥
	NETSEND_TYPE_RES2,	//2�ű���
	NETSEND_TYPE_TCP,	//TS����TCPЭ��
	NETSEND_TYPE_C1,	//��ǬЭ��
	NETSEND_TYPE_RTSP,	//RTSPЭ��
	NETSEND_TYPE_RTMP,	//RTMPЭ��
	NETSEND_TYPE_HTTP,	//HTTPЭ��
	NETSEND_TYPE_ONVIF,	//ONVIFЭ��
	NETSEND_TYPE_VSIP	//VSIPЭ��
};

enum NETRECV_TYPE {		//��·�����ӿ�ͨ��Э��
	NETRECV_TYPE_RES0,	//0�ű���
	NETRECV_TYPE_UDP,	//TS����UDPЭ�飺�����������鲥���㲥���㲥
	NETRECV_TYPE_RES2,	//2�ű���
	NETRECV_TYPE_TCP,	//TS����TCPЭ��
	NETRECV_TYPE_RTSP,	//RTSPЭ��
	NETRECV_TYPE_RES3,	//����
	NETRECV_TYPE_RTMP,	//RTMPЭ��
	NETRECV_TYPE_HTTP,	//HTTPЭ��
	NETRECV_TYPE_ONVIF,	//ONVIFЭ��
	NETRECV_TYPE_VSIP,	//VSIPЭ��
	NETRECV_TYPE_C1=201	//��ǬЭ��
};

typedef  struct NETWORKING 
{
	int				size;				//sizeof()
	int				DstUDPIPAddr;		//server��UDP���͵�Ŀ��IP���������鲥��ַ���㲥��ַ��������ַ�������ʹ�ã��������ͨ��ʹ��UDPЭ��ʱ��Ч
	unsigned short	DstUDPPORT;			//server��UDP���͵�Ŀ��˿�PORT�������ʹ�ã��������ͨ��ʹ��UDPЭ��ʱ��Ч
	short			sendType;			//server��UDP���͵����͡�0:��������; 1:�鲥����; 3:TCP����; 4:�㲥
	int				UDPMulticastIPAddr;	//client��UDP���յ��鲥IP�������ʹ�ã��������ͨ��ʹ��UDPЭ����ʹ���鲥ʱ��Ч
	unsigned short	UDPPORT;			//client��UDP���յĶ˿ںš������ʹ�ã��������ͨ��ʹ��UDPЭ��ʱ��Ч
	short			recvType;			//client��UDP���յ����͡�0:��������;1:�鲥����;2:�㲥����;3:TCP����;4:ֱ���������(���ݱ��������);101:��̬�л�;201:����visiondigi����
	int				ServerIPAddr;		//client��TCP���ӵ�server IP�������ʹ�ã��������ͨ��ʹ��TCP����ʱ��Ч
	unsigned short	ServerPORT;			//client��TCP���ӵ�server PORT�������ʹ�ã��������ͨ��ʹ��TCP����ʱ��Ч
	unsigned short	TCPPORT;			//server��TCP�����Ķ˿ںš������ʹ�ã��������ͨ��ʹ��TCP����ʱ��Ч
}NetWorking,*pNetWorking;

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

enum VIDEO_TYPE{		//�ɼ�����
	VIDEO_TYPE_UNKNOWN =0,			//�ɼ�����
	VIDEO_TYPE_CAPTURE_CVBS =1,		//�ɼ�CVBS�ź�
	VIDEO_TYPE_CAPTURE_COMPOSITE,	//�ɼ�COMPOSITE�ź�
	VIDEO_TYPE_CAPTURE_VGA,			//�ɼ�VGA�ź�
	VIDEO_TYPE_CAPTURE_YPbPr ,		//�ɼ�Ypbpr�ź�
	VIDEO_TYPE_CAPTURE_YPbPr50i,	//�ɼ�Ypbpr�ź� 1080i@50
	VIDEO_TYPE_CAPTURE_YPbPr60i,	//�ɼ�Ypbpr�ź� 1080i@60
	VIDEO_TYPE_CAPTURE_YPbPr50p,	//�ɼ�Ypbpr�ź� 720P@50 or 1080P
	VIDEO_TYPE_CAPTURE_YPbPr60p,	//�ɼ�Ypbpr�ź� 720P@60 or 1080P
	VIDEO_TYPE_CAPTURE_YPbPr25p,	//�ɼ�Ypbpr�ź�720p or 1080P
	VIDEO_TYPE_CAPTURE_YPbPr30p,	//�ɼ�Ypbpr�ź�720p or 1080P
	VIDEO_TYPE_CAPTURE_DVI,			//�ɼ�DVI�ź�
	VIDEO_TYPE_CAPTURE_HDMI,		//�ɼ�HDMI�ź�
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
	VIDEO_TYPE_DISPLAY_CVBS=0x0081,		//��ʾ���ͣ�CVBS���
	VIDEO_TYPE_DISPLAY_COMPOSITE,		//��Ƶʹ��COMPOSITE�ź����
	VIDEO_TYPE_DISPLAY_VGA,				//��Ƶʹ��VGA�ź����
	VIDEO_TYPE_DISPLAY_YPbPr,			//��Ƶʹ��Ypbpr�ź����
	VIDEO_TYPE_DISPLAY_DVI,				//��Ƶʹ��DVI�ź����
	VIDEO_TYPE_DISPLAY_HDMI,			//��Ƶʹ��HDMI�ź����
	VIDEO_TYPE_DISPLAY_LCD,				//��Ƶʹ��LCD�ź����
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
	AUDIO_TYPE_MONO =0,			//������
	AUDIO_TYPE_STEREO=0x0080,	//˫���� ������
	AUDIO_TYPE_HDMI  =0x0100,	//audio from HDMI port, linxj 2012-12-21
	AUDIO_TYPE_SDI   =0x0200,	//audio from SDI port, linxj 2013-03-13
};

typedef  struct  Peripheral
{
	int		size;
    short	VideoType;		//��Ƶ�ɼ�����ʾ�Ĳ��������ö������VIDEO_TYPE
	short	VideoWidth;		//��Ƶ�ɼ�����ʾ�Ŀ��
	short	VideoHeight;	//��Ƶ�ɼ�����ʾ�ĸ߶�
	short	AudioType;		//��Ƶ�������������ͣ����ö������AUDIO_TYPE
	int		AudioSample;	//��Ƶ����������Ƶ�ʣ���44100��ʾ44.1K	
}Peripheral,*pPeripheral;

typedef  struct  VideoQuality
{
	int		size;
    short	autogain;	//��Ƶ���������Զ����棬�ù����ݱ���
	short	bright;		//��Ƶ�������������ȴ�С(0-100)��  ȡֵ��Χ0-100������Ϊ50
	short	hue;		//��Ƶ����������ɫ�ȴ�С(0-100)��  ȡֵ��Χ0-100������Ϊ50
	short	satution;	//��Ƶ���������ı��Ͷȴ�С(0-100)��ȡֵ��Χ0-100������Ϊ50	
}VideoQuality,*pVideoQuality;

typedef struct VideoDimension {
    int size;
    short width;	//��Ƶ����Ļ�п��
    short height;	//��Ƶ����Ļ�и߶�
    short basex;	//��Ч��Ƶ����ʾ��������x����
    short basey;	//��Ч��Ƶ����ʾ��������y����
    short hp;		//��Ƶ����Ļ�е�����λ��
    short vp;		//��Ƶ����Ļ�е�����λ��
    short hint;		//horizontal interval
    short vint;		//vertical interval
    int reg;		//register address -1��Ч��>0��Ч
    int val;		//register value -1��Ч��>=0��Ч
    int exparams1;	//reserved
    int exparams2;	//reserved
} VideoDimension, *pVideoDimension;

typedef  struct  AudioVolume
{
	int		size;
    short	autogain;	//��Ƶ���������Զ����棬�ù����ݱ���
	short	volume;		//��Ƶ����������������С(0-100)��ȡֵ��Χ0-100������Ϊ50
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
	RESTART_TYPE_BOARD =0,	//��������
	RESTART_TYPE_PID_ENC,	//�����������
	RESTART_TYPE_PID_DEC,	//�����������
};
typedef  struct  RESTARTPARAM
{
	int		size;
	int		DstIP;			//���������IP����ʹ�ù㲥ʱ�������塣IP==0ʱ��Ч��IP==-1��ʾ����IP������
	int		RestartType;	//���������ͣ���������or����ĳ������
}RestartParam,*pRestartParam;

enum UPDATE_TYPE{
	UPDATE_TYPE_COSTUMER =0,	//�Զ��壬���ļ����ݵ��������о���
	UPDATE_TYPE_DM642BIN,		//���DM642���ӵĳ����ļ�
	UPDATE_TYPE_DM642BOOT,		//���DM642����BOOT�ĳ����ļ���BOOT��bin�Ļ����������һЩ��Ϣ
	UPDATE_TYPE_ADDNEWFILE,		//���linuxϵͳ����µĳ����ļ��������ļ��Ѵ����򲻴������ļ����ο�filename����
	UPDATE_TYPE_ADDFILE,		//���linuxϵͳ��ӳ����ļ��������ļ��Ѵ����򸲸������ļ����ο�filename����
	UPDATE_TYPE_DELFILE,		//���linuxϵͳɾ��ĳ���ļ����ļ����ο�filename����
	UPDATE_TYPE_GETFILE,		//2012.11.2 jiangzhifei get file from server
	UPDATE_TYPE_SENDTOCLIENT,	//2012.11.2 jiangzhifei send file to client
	UPDATE_TYPE_MASTER=0x100,	//�����̵ĳ����ļ�
	UPDATE_TYPE_SLAVE,			//�ӽ��̵ĳ����ļ�
	UPDATE_TYPE_UPDATE,			//���³�����̵ĳ����ļ�
	UPDATE_TYPE_KERNEL,			//�ں˵ĳ����ļ�
	UPDATE_TYPE_SYSCONFIG,		//ϵͳ���õĳ����ļ�
	UPDATE_TYPE_NETCONFIG,		//�������õĳ����ļ�
	UPDATE_TYPE_CMEMK,			//cmemk.ko�ĳ����ļ�
	UPDATE_TYPE_EDMAK,			//edmak.ko�ĳ����ļ�
	UPDATE_TYPE_IRQK,			//irqk.ko�ĳ����ļ�
	UPDATE_TYPE_MMAPK,			//mmapk.ko�ĳ����ļ�
	UPDATE_TYPE_APP,			//����Ӧ�ó����ѹ�����ļ�
};
typedef  struct  SETPROGRAM
{
	int		size;				//���������Ч�غɳ��ȣ������ļ�������
    int		prgfilelen;			//�����ļ��ĳ���
	short	ver_major;			//�������汾��
	short	ver_minor;			//����ΰ汾��
	int		prgtype;			//�����ļ������ͣ��μ�ö������UPDATE_TYPE
	char	prgfilename[256];	//�����ļ�ʱʹ�õ��ļ���
    //unsigned char prgfilebite[xxxx];	//�����ļ������ݣ���������
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
	int		size;				//���������Ч�غɳ���
	int		senddataType;		//���ӳɹ����������ķ��ͷ�ʽ��0:����Ӧ,���ݷ�����Ϣ����  1:TCP 2:UDP
	int		IPaddr;				//���Ӷ˵�IP��ַ��0:����Ӧ
	unsigned short	UDPPORT;	//���Ӷ˵�UDP���ն˿�
	short	connectEnable;		//����ʹ�ܣ�1:��ʾ���� 0:��ʾ�Ͽ� -1:��ʾ�Ͽ���������
}Connection, *pConnection;

typedef  struct CONNECTIONRETURN
{
	int		size;				//���������Ч�غɳ���
	int		senddataType;		//���ӳɹ����������ķ��ͷ�ʽ��0:����Ӧ  1:TCP 2:UDP
	int		IPaddr;				//client��IP��ַ��0:����Ӧ��TCP���ӷ�ʽ��Ч
	unsigned short	TCPPORT;	//client��TCP�����˿ڣ�TCP���ӷ�ʽ��Ч
	short	connectCounter;		//֧�ֵ����������
	int		connectStatus;		//����״̬�����CONNECTION_STATUS_TYPE
}ConnectionReturn, *pConnectionReturn;

typedef struct USERPARAMS {
    int size;
    int iFrame;		//�Ƿ����I֡��0:��1:��
    int exparam1;
    int exparam2;
}UserParams, *pUserParams;

typedef struct USERDATA {
    int size;
    int dataLen;				//���ݳ���
    unsigned char data[64];		//�Զ�������
    int checked;				//У������0��У�鲻ͨ����1��У��ͨ��
    int exparam2;
} UserData, *pUserData;


//i2c����
typedef struct I2CDATA {
    int size;
    short   i2caddr;            //i2c��ַ����Χ0-255
    short   dataLen;            //���ݳ���0-256
    int     type;               //
    unsigned char reg[256];     //�Ĵ�����ַ
    unsigned char val[256];     //����ֵ
} I2cData, *pI2cData;

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
    unsigned int	baudrate;	//���Ͳ�����
    unsigned char	data[1024];	//��Ҫ���͵���Ч����
    int				exparam1;
    int				exparam2;
} SerialControl, *pSerialControl;

//slave dynamic port
typedef struct DYNAMIC_PARAM {
    int size;
    int videobitrate;			//��Ƶ���� 0:���䣿
    int videofps;				//��Ƶ֡�� 0:����
	int videoidrinterval;		//IDR֡��� 0:����
    int audiobitrate;			//��Ƶ����
    int audiosamplerate;		//��Ƶ������
    int exparam0;
    int exparam1;
    int exparam2;
} DynamicParam, *pDynamicParam;

typedef struct RUNNING_STATUS {
	int				size;
	unsigned int	poweronnum;		//��������
	int				connecttednum;	//�ͻ�������Ӧ������
	char			videoinput;		//��Ƶʹ�� 0:�ر�
	char			audioinput;		//��Ƶʹ�� 0:�ر�
	char			null1;
	char			null2;
	unsigned int	captureframe;		//�ɼ�֡��
	unsigned int	videoframe;			//��Ƶ֡����
	int				videobitrate;		//��Ƶ����
	int				videofps;			//��Ƶ֡��
	int				videoidrinterval;	//IDR֡���
	int				audiobitrate;		//��Ƶ����
	int				audiosamplerate;	//��Ƶ������
	/* ��Դͳ�� */
	unsigned int	cputotal;			//cpu������ʱ�䣬΢��
	unsigned int	cpuused;			//cpu������ʱ��΢��
	unsigned int	cpupercent;			//cpuռ�ðٷֱ�  0-1000
	unsigned int	memtotal;			//�����ڴ�����
	unsigned int	memused;			//�ڴ�ռ��
	unsigned int	mempercent;			//�ڴ�ռ�ðٷֱ�  0-1000
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

#endif	//end of __VISCONFIG_H__
