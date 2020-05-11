#ifndef __VIS_MMNP_C1_H__
#define __VIS_MMNP_C1_H__

#include "vis_mmnp.h"
#include "ringbuffer.h"

/* 
 * 2012-07-12
 * ls
 */
#ifndef		__PANTZ_H__
#define		__PANTZ_H__

#define		SERIALDEV				"/dev/tts/1"
#define     PANTZ_PROTOCOL_PD		0x00
#define     PANTZ_PROTOCOL_SONY		0x01
#define     PANTZ_MOTION_UP			0
#define     PANTZ_MOTION_DOWN		1
#define     PANTZ_MOTION_LEFT		2
#define     PANTZ_MOTION_RIGHT		3
#define     PANTZ_MOTION_ZOOMIN		4
#define     PANTZ_MOTION_ZOOMOUT	5
#define     PANTZ_MOTION_FOCUSNEAR	6
#define     PANTZ_MOTION_FOCUSFAR	7
#define     PANTZ_MOTION_IRISOPEN	8
#define     PANTZ_MOTION_IRISCLOSE	9
#define     PANTZ_MOTION_PRESET		10
#define     PANTZ_MOTION_CALL		11
#define     PANTZ_MOTION_CLEAR		12
#define     PANTZ_MOTION_LIGHT		13
#define     PANTZ_MOTION_SCANPRESET	14
#define     PANTZ_MOTION_SCANCIRCLE	16
#define     MAX_PANTZ_CMD_LEN		16
#define     MAX_PANTZ_CMD_COUNT		5

typedef struct pantz_motion {
	int				 cmdlen;
	int				 motion;
	int				 steps;
	unsigned char	*cmd; 
} Pantz_Motion;

typedef struct default_cmd {
	int	cmdlen;
	unsigned char stop[8];
	unsigned char up[8];
	unsigned char down[8];
	unsigned char left[8];
	unsigned char right[8];
	unsigned char zoom_in[8];
	unsigned char zoom_out[8];
	unsigned char focus_near[8];
	unsigned char focus_far[8];
	unsigned char iris_open[8];
	unsigned char iris_close[8];
	unsigned char preset[8];
	unsigned char call[8];
	unsigned char clear[8];
	unsigned char light[8];
	unsigned char scan_preset[8];
	unsigned char scan_circle[8];
} Default_Cmd;

#define	PANTZ_CMD_PD_DEFAULT {\
	{0xff,0x01,0x00,0x08,0x00,0x27,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},\
	{0xff,0x01,0x00,0x10,0x00,0x27,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},\
	{0xff,0x01,0x00,0x04,0x27,0x00,0x2c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},\
	{0xff,0x01,0x00,0x02,0x27,0x00,0x2a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},\
	{0xff,0x01,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}\
}

#endif	//__PANTZ_H__

#define		MAXQ 8
#define		IP_LEN					16
#define     CMD_SENDBUF_LEN			64
#define		CMD_RECVBUF_LEN			96
#define     DATA_SENDBUF_LEN		0x20000
#define     PACKAGE_LEN				(0x2000-0x20)
#define     ACCOUNT					"admin"
#define     PWD						"123456"
#define     MAX_ACCOUNT_LEN			8
#define     MAX_PWD_LEN				8
#define     CHANNEL_COUNT			0x1
#define		SERVER_HEART_TIMEOUT	30

#define     DEV_SYSTEM_INFO_DEV_VER	"VP9516.3.2.T01,Build:2010-1-14"

/* CommandID */
#define     REQ_USER_LOGIN			0x0a
#define     ACK_USER_LOGIN			0xb0
#define     REQ_HEART_BEAT			0xa1
#define     ACK_HEART_BEAT			0xb1
#define     REQ_INFO_SYSTEM			0xa4
#define     ACK_INFO_SYSTEM			0xb4
#define     REQ_SYSTEM_CONFIG		0x0
#define     ACK_REQ_SYSTEM_CONFIG	0x1
#define     ACK_SET_SYSTEM_CONFIG	0x0
#define     REQ_SUB_CONNECTION		0xf1
#define     REQ_VIDEO_START			0x11
#define     REQ_PANTZ_CTRL			0x12
#define     REQ_VIDEO_STOP			0xbc
#define     REQ_DEVICE_CTRL			0x60
#define     ACK_REQ_DEVICE_CTRL		0x60
#if 0
#define     REQ_ALARM_STAT			0x60
#define     ACK_ALARM_STAT			0x60
#define     REQ_CTRL_PANTZ			0x60
#define     REQ_SEND_AUDIO			0x60
#define     ACK_SEND_AUDIO			0x60
#define     REQ_AUDIO_DATA			0x60
#define     REQ_RECD_PLAY			0x60
#define     REQ_PLAY_STOP			0x60
#define     REQ_PLAY_PAUSE			0x60
#define     REQ_CTRL_PLAY			0x60
#endif
#define     REQ_SET_DTIME			0x24
/* End of CommandID */

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

#define     SOCK_TYPE_CMD		100
#define     SOCK_TYPE_DATA		101
#define     SOCK_TYPE_OTHER		102
#define     SOCK_TYPE_READY		103
#define     SOCK_TYPE_RECV		104
#define     SOCK_TYPE_CLOSE		-1
#define     SOCK_TYPE_DATA_0	0
#define     SOCK_TYPE_DATA_1	1
#define     SOCK_TYPE_DATA_2	2
#define     SOCK_TYPE_DATA_3	3
#define     SOCK_TYPE_DATA_4	4
#define     SOCK_TYPE_DATA_5	5
#define     SOCK_TYPE_DATA_6	6
#define     SOCK_TYPE_DATA_7	7
#define     SOCK_TYPE_DATA_8	8
#define     SOCK_TYPE_DATA_9	9
#define     SOCK_TYPE_DATA_10	10
#define     SOCK_TYPE_DATA_11	11
#define     SOCK_TYPE_DATA_12	12
#define     SOCK_TYPE_DATA_13	13
#define     SOCK_TYPE_DATA_14	14
#define     SOCK_TYPE_DATA_15	15


typedef struct c1_env {
	char *url;
	char *ip;
    unsigned short port;
	unsigned short maxConnNum;
	pthread_mutex_t *mutex_initsync;	//create in vmn_monitor(), use in sender thread and vmn_monitor()
	pthread_cond_t *cond_initsync;		//create in vis_mmnp_create(), use in sender thread and vis_mmnp_start()
//	Vis_Mmnp_Handle *handle_ptr;
	volatile int *quit_flag_ptr;
	unsigned int *conn_num_ptr;
	pthread_mutex_t *conn_num_mutex;
	RingBuffer *pbuffer;		//video buffer for rtsp
	RingBuffer *pbuffer_rsz;	//audio buffer for rtsp
} C1Env;

extern void *vmn_c1ThrFxn(void *arg);

#endif /* __VIS_MMNP_C1_H__ */
