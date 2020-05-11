#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <netdb.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/route.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <linux/rtc.h>

#include <vis_util/include/util.h>

#include "main.h"
#include "netconfig.h"
#include "refresh.h"
#include "monitor.h"
#include "serial.h"
#include "visconfig.h"
#include "slavestatus.h"
#include "save.h"
#include "../demo_common.h"

#define APPVERSION_MAJOR		1		//E500, SDI_enc
//#define APPVERSION_MAJOR		2		//E300, HDMI_enc
//#define APPVERSION_MAJOR		4		//E200, VGA_enc
//#define APPVERSION_MAJOR		5		//E200, VGA_enc

//#define APPVERSION_MINOR		8		//for all
//#define APPVERSION_MINOR		100		//for develop version
//#define APPVERSION_MINOR		6		//for sdi 1.8 debug
//#define APPVERSION_MINOR		201		//for ?????????
//#define APPVERSION_MINOR		261		//for ?????
//#define APPVERSION_MINOR		308		//for TCP
//#define APPVERSION_MINOR		506		//for zhuhaiyangheng
//#define APPVERSION_MINOR		515		//for customer
//#define APPVERSION_MINOR		9		//for UDP
#define APPVERSION_MINOR      720		//for dapt save system
//#define APPVERSION_MINOR      700		//for E200 c1 system
//#define APPVERSION_MINOR      700		//for E500 c1 system
//#define APPVERSION_MINOR		800		//for E300 c1 protocol
//#define CUSTOM_SCC	//Shanghai Custom College

//#define DYNAMIC_VIDEO_PARAM

#define	MAXNUM_RESOLUTION  32
typedef struct _capturedim{
	int size;
	int num;
	struct {
	int	width;
	int height;
	int hori_start;
	int vert_start;
	}_setting[MAXNUM_RESOLUTION];
}capturedim;

#define CAPTUREDIM_FILE		"../configFile/capture.ini"
#define REG_MODIFY_FILE		"/dev/davinci_reg_modify"
#define	SYSCONF_FILE		"../configFile/viscfgEnc.system"
#define	USERDATA_FILE		"../configFile/userdata.ini"
#define	FIFONAME		    "./fifo"
//#define	VIDEOFIFONAME		"./videofifo"
#define POWERONFILE			"../configFile/poweron.config"
#define POWERONOPT		    "poweron=%d"
#define INFO2MASTER_NAME	"./info2master"
#define OSDCONF_FILE		"/mnt/apps/configFile/osd.config"
#define CUSTOMSTD_FILE		"/mnt/apps/configFile/customstd.cfg"
#ifndef CUSTOM_SCC
#define SLAVE_NAME_DEFAULT	"./visEnc.slave"
#else
#define SLAVE_NAME_DEFAULT	"./visEnc.c1"
#endif
#define SLAVE_NAME_UDP		"./visEnc.slave"
#define SLAVE_NAME_RES2
#define SLAVE_NAME_TCP		"./visEnc.tcp"
#define SLAVE_NAME_C1		"./visEnc.c1"
#define SLAVE_NAME_RTSP		"./visEnc.rtsp"

#define IP_LEN	        16
#define	LINELEN	        32
#define	RESOLUTION_LEN  50
#define	OLD_CONF_LEN	(128+64)
//#define	FPA_LEN		    31
#define CUSTOMSTD_WIDTH_DEFAULT		1024
#define CUSTOMSTD_HEIGHT_DEFAULT	768

#define REFRESH_THREAD_PRIORITY  sched_get_priority_max(SCHED_FIFO) - 2
#define SERIAL_THREAD_PRIORITY  sched_get_priority_max(SCHED_FIFO) - 1
#define MONITOR_THREAD_PRIORITY  sched_get_priority_max(SCHED_FIFO)


#ifndef CUSTOM_SCC
char *default_conf = "visEnc.slave -d 0.0.0.0 -p 0 -f 30 -b 5000 -c 1 -i 100 -s 44100 -t 128 -v 1 -a 1 -l 20 -g 80 -w 0 -h 0 -r XGA -n 1";
char *default_conf_sdi = "visEnc.slave -d 0.0.0.0 -p 0 -f 30 -b 5000 -c 1 -i 100 -s 44100 -t 128 -v 1 -a 1 -l 20 -g 80 -w 0 -h 0 -r SDIV_1280x720p_60Hz -n 1";
char *default_conf_dvi = "visEnc.slave -d 0.0.0.0 -p 0 -f 30 -b 5000 -c 1 -i 100 -s 44100 -t 128 -v 1 -a 0 -l 20 -g 80 -w 0 -h 0 -r HDVI_1280x720p_60Hz -n 1";
#else
char *default_conf = "visEnc.slave -d 0.0.0.0 -p 0 -f 30 -b 5000 -c 1 -i 100 -s 44100 -t 128 -v 1 -a 1 -l 20 -g 80 -w 0 -h 0 -r XGA -n 4";
char *default_conf_sdi = "visEnc.slave -d 0.0.0.0 -p 0 -f 30 -b 5000 -c 1 -i 100 -s 44100 -t 128 -v 1 -a 1 -l 20 -g 80 -w 0 -h 0 -r SDIV_1280x720p_60Hz -n 4";
#endif
unsigned char	g_ip[IP_LEN];
unsigned short  g_port;
unsigned short	g_framerate;
unsigned short	g_kbps;
unsigned short	g_cbr;
unsigned short  g_iinterval;
char            g_resolution[RESOLUTION_LEN];
unsigned short	g_audio_hz;
short	        g_audio_kbps;
unsigned short	g_video;
unsigned short	g_audio;
unsigned short	g_lum;
unsigned short	g_volume;
unsigned short	g_videowidth;
unsigned short	g_videoheight;
unsigned short	g_netsendtype;  //linxj2012-05-17

RunningStatus   devstatus;
SlaveStatus slavestatus;
volatile sig_atomic_t global_flag_e = 0; //sp 12-10-09 cause signal handler modifies it
unsigned short peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
VisStatus visstatus;
net_config	net_cfg;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int child_status;
int restart_times;
int	restart_flg;
int	net_cfg_flg;
static pid_t slave_pid;	//ls, 2013-02-25, make system() function will not cause global_flag_e
int serial_flg;	//ls 2013-04-12, dynamic change serial config

enum front_progress_param_t{
	PROG_NAME,
	TARGET_IP,
	TARGET_IP_VALUE,
	PORT,
	PORT_VALUE,
	FRAMERATE,
	FRAMERATE_VALUE,
	VIDEOBITRATE,
	VIDEOBITRATE_VALUE,
	VIDEOCBR,
	VIDEOCBR_VALUE,
	VIDEOIDR,
	VIDEOIDR_VALUE,
    AUDIOSAMPLERATE,
    AUDIOSAMPLERATE_VALUE,
    AUDIOBITRATE,
    AUDIOBITRATE_VALUE,
    VIDEOENABLE,
    VIDEOENABLE_VALUE,
    AUDIOENABLE,
    AUDIOENABLE_VALUE,
    VIDEOLUM,
    VIDEOLUM_VALUE,
    AUDIOVOLUME,
    AUDIOVOLUME_VALUE,
    VIDEOWIDTH,
    VIDEOWIDTH_VALUE,
    VIDEOHEIGHT,
    VIDEOHEIGHT_VALUE,
	RESOLUTION,
	RESOLUTION_VALUE,
    SENDTYPE,
    SENDTYPE_VALUE,
	NUM_FPP
} front_progress_param;
#define	FPA_LEN		    (NUM_FPP+1)


/*******************************************************************************************************************
 * static functions
 *******************************************************************************************************************/
static unsigned int getDiskTotalSpace(char *dirname);	//Mbyte
static unsigned int getDiskUsageSpace(char *dirname);	//Mbyte
static unsigned int getDiskFreeSpace(char *dirname);	//Mbyte
static unsigned int getTotalDataMBytes(char *dirname, unsigned int *data_file_count);	//Mbyte
static unsigned long get_file_size(const char *path);	//byte
static int mount_point(char *path);
static int set_hw_time(struct tm *tmtime);

/*******************************************************************************************************************
 * extern functions
 *******************************************************************************************************************/
int killname(char *cmd);

#if 0
int system2(const char * cmdstring){
	pid_t pid;
	int status;
	if(cmdstring == NULL){          
		return (1);
    }
    if((pid = fork())<0){
		status = -1;
    }else if(pid == 0){
        execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
        exit(127); //子进程正常执行则不会执行此语句
    }else{
//		sleep(3);
		kill(pid, SIGINT);
		printf("333333333333333333333333333\n");
		while(waitpid(pid, &status, 0) < 0){
			if(errno != EINTR){
				status = -1;
				break;
			}
		}
		printf("444444444444444444444444\n");
	}
	return status;
}
#endif

//JZF 2012.11.12 valum_to_gresolution()
void valum_to_gresolution(Peripheral *peripheral,char *g_resolution){
	char videotype[15];
	int frequency =0;
	int ret = 0;
	switch(peripheral->VideoType){
		case VIDEO_TYPE_CAPTURE_VGA:
			strcpy(videotype, "VGA_");
			frequency = 60;
			ret = 1;
			break;
		case VIDEO_TYPE_CAPTURE_VGA_55:
			strcpy(videotype, "VGA_");
			frequency = 55;
			ret = 1;
			break;
		case VIDEO_TYPE_CAPTURE_VGA_70:
			strcpy(videotype, "VGA_");
			frequency = 70;
			ret = 1;
			break;
		case VIDEO_TYPE_CAPTURE_VGA_72:
			strcpy(videotype, "VGA_");
			frequency = 72;
			ret = 1;
			break;
		case VIDEO_TYPE_CAPTURE_VGA_75:
			strcpy(videotype, "VGA_");
			frequency = 75;
			ret = 1;
			break;
		case VIDEO_TYPE_CAPTURE_VGA_85:
			strcpy(videotype, "VGA_");
			frequency = 85;
			ret = 1;
			break;
		case VIDEO_TYPE_CAPTURE_HDMI:
			strcpy(videotype, "HDMI_");
			frequency = 60;
			ret = 1;
			break;
		case VIDEO_TYPE_CAPTURE_HDMI_55:
			strcpy(videotype, "HDMI_");
			frequency = 55;
			ret = 1;
			break;
		case VIDEO_TYPE_CAPTURE_HDMI_70:
			strcpy(videotype, "HDMI_");
			frequency = 70;
			ret = 1;
			break;
		case VIDEO_TYPE_CAPTURE_HDMI_72:
			strcpy(videotype, "HDMI_");
			frequency = 72;
			ret = 1;
			break;
		case VIDEO_TYPE_CAPTURE_HDMI_75:
			strcpy(videotype, "HDMI_");
			frequency = 75;
			ret = 1;
			break;
		case VIDEO_TYPE_CAPTURE_HDMI_85:
			strcpy(videotype, "HDMI_");
			frequency = 85;
			ret = 1;
			break;
		case VIDEO_TYPE_CAPTURE_HDMI_50i:
			strcpy(videotype,"HDMI_");
			frequency = 50;
			ret = 2;
			break;
		case VIDEO_TYPE_CAPTURE_HDMI_50p:
			strcpy(videotype,"HDMI_");
			frequency = 50;
			ret = 3;
			break;
		case VIDEO_TYPE_CAPTURE_HDMI_60i:
			strcpy(videotype,"HDMI_");
			frequency = 60;
			ret = 2;
			break;
		case VIDEO_TYPE_CAPTURE_SDI_60P:
			strcpy(videotype,"SDIA_");
			frequency = 60;
			ret = 3;
			break;
		case VIDEO_TYPE_CAPTURE_SDI_50P:
			strcpy(videotype,"SDIA_");
			frequency = 50;
			ret = 3;
			break;
		case VIDEO_TYPE_CAPTURE_SDI_30P:
			strcpy(videotype,"SDIA_");
			frequency = 30;
			ret = 3;
			break;
		case VIDEO_TYPE_CAPTURE_SDI_25P:
			strcpy(videotype,"SDIA_");
			frequency = 25;
			ret = 3;
			break;
		case VIDEO_TYPE_CAPTURE_SDI_24P:
			strcpy(videotype,"SDIA_");
			frequency = 24;
			ret = 3;
			break;
		case VIDEO_TYPE_CAPTURE_SDI_60I:
			strcpy(videotype,"SDIA_");
			frequency = 60;
			ret = 2;
			break;
		case VIDEO_TYPE_CAPTURE_SDI_50I:
			strcpy(videotype,"SDIA_");
			frequency = 50;
			ret = 2;
			break;
		default:
			break;
	}
	if (ret ==1)
		sprintf(g_resolution,"%s%dx%d_%dHz",videotype,peripheral->VideoWidth,peripheral->VideoHeight,frequency);
	else if(ret == 2){
		sprintf(g_resolution,"%s%dx%di_%dHz",videotype,peripheral->VideoWidth,peripheral->VideoHeight,frequency);
	}else if (3 == ret) {
		sprintf(g_resolution,"%s%dx%dp_%dHz",videotype,peripheral->VideoWidth,peripheral->VideoHeight,frequency);
	}
		
	if(g_resolution[0]=='H'&&g_resolution[1]=='D'&&g_resolution[2]=='M'&&g_resolution[3]=='I'
		&& peripheral->AudioType!=AUDIO_TYPE_HDMI)
	{//linxj 2012-12-21 //support HDMI and HDVI
		g_resolution[2]='V';
	}
	if(g_resolution[0]=='S'&&g_resolution[1]=='D'&&g_resolution[2]=='I'&&g_resolution[3]=='A'
		&& peripheral->AudioType!=AUDIO_TYPE_SDI)
	{//ls 2013-03-22 //support SDIA and SDIV
		g_resolution[3]='V';
	}
}

//JZF 2012.11.12 gresolution_to_valum()
void gresolution_to_valum(Peripheral *peripheral,char *g_resolution){
		int frequency;
		int hz;
		char iorp;
		int otherfbl;
		if(g_resolution[0]=='V'&&g_resolution[1]=='G'&&g_resolution[2]=='A'&&g_resolution[3]=='_'){
			sscanf(g_resolution,"VGA_%hdx%hd_%dHz",&(peripheral->VideoWidth),&(peripheral->VideoHeight),&frequency);
			if(frequency == 60)
				peripheral->VideoType= VIDEO_TYPE_CAPTURE_VGA;
			if(frequency == 55)
				peripheral->VideoType= VIDEO_TYPE_CAPTURE_VGA_55;
			if(frequency ==70)
				peripheral->VideoType= VIDEO_TYPE_CAPTURE_VGA_70;
			if(frequency == 72)
				peripheral->VideoType= VIDEO_TYPE_CAPTURE_VGA_72;
			if(frequency == 75)
				peripheral->VideoType= VIDEO_TYPE_CAPTURE_VGA_75;
			if(frequency == 85)
				peripheral->VideoType= VIDEO_TYPE_CAPTURE_VGA_85;
		}else if(g_resolution[0]=='H'&&g_resolution[1]=='D'&&(g_resolution[2]=='M'||g_resolution[2]=='V')&&g_resolution[3]=='I'&&g_resolution[4]=='_'){//linxj 2012-12-21 //support HDMI and HDVI
			if ('i' == g_resolution[strlen(g_resolution)-6]){
				sscanf(g_resolution+4,"_%hdx%hdi_%dHz",&(peripheral->VideoWidth),&(peripheral->VideoHeight),&frequency);
				switch (frequency) {
					case 50 :
						peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_50i;
						break;
					case 60 :
						peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_60i;
						break;
					default :
						break;
				}
			} else if ('p' == g_resolution[strlen(g_resolution)-6]){
				sscanf(g_resolution+4,"_%hdx%hdp_%dHz",&(peripheral->VideoWidth),&(peripheral->VideoHeight),&frequency);
				switch (frequency) {
					case 50 :
						peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_50p;
						break;
					case 60 :
						peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_60p;
						break;
					default :
						break;
				}
			} else {
				sscanf(g_resolution+4,"_%hdx%hd_%dHz",&(peripheral->VideoWidth),&(peripheral->VideoHeight),&frequency);
				if(frequency == 60)
					peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI;
				if(frequency == 55)
					peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_55;
				if(frequency == 70)
					peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_70;
				if(frequency == 72)
					peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_72;
				if(frequency == 75)
					peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_75;
				if(frequency == 85)
					peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_85;
			}
		}else if(g_resolution[0]=='S'&&g_resolution[1]=='D'&&g_resolution[2]=='I'&&(g_resolution[3]=='A'||g_resolution[3]=='V')&&g_resolution[4]=='_'){//ls, 2013-03-13 //support SDIA and SDIV
			if ('i' == g_resolution[strlen(g_resolution)-6]){
				sscanf(g_resolution+4,"_%hdx%hdi_%dHz",&(peripheral->VideoWidth),&(peripheral->VideoHeight),&frequency);
				switch (frequency) {
					case 50 :
						peripheral->VideoType= VIDEO_TYPE_CAPTURE_SDI_50I;
						break;
					case 60 :
						peripheral->VideoType= VIDEO_TYPE_CAPTURE_SDI_60I;
						break;
					default :
						break;
				}
			} else if ('p' == g_resolution[strlen(g_resolution)-6]){
				sscanf(g_resolution+4,"_%hdx%hdp_%dHz",&(peripheral->VideoWidth),&(peripheral->VideoHeight),&frequency);
				switch (frequency) {
					case 60 :
						peripheral->VideoType= VIDEO_TYPE_CAPTURE_SDI_60P;
						break;
					case 50 :
						peripheral->VideoType= VIDEO_TYPE_CAPTURE_SDI_50P;
						break;
					case 30 :
						peripheral->VideoType= VIDEO_TYPE_CAPTURE_SDI_30P;
						break;
					case 25 :
						peripheral->VideoType= VIDEO_TYPE_CAPTURE_SDI_25P;
						break;
					case 24 :
						peripheral->VideoType= VIDEO_TYPE_CAPTURE_SDI_24P;
						break;
					default :
						break;
				}
			} else {
				sscanf(g_resolution+4,"_%hdx%hd_%dHz",&(peripheral->VideoWidth),&(peripheral->VideoHeight),&frequency);
				if(frequency == 60)
					peripheral->VideoType= VIDEO_TYPE_CAPTURE_SDI_60P;
				if(frequency == 50)
					peripheral->VideoType= VIDEO_TYPE_CAPTURE_SDI_50P;
				if(frequency == 30)
					peripheral->VideoType= VIDEO_TYPE_CAPTURE_SDI_30P;
				if(frequency == 25)
					peripheral->VideoType= VIDEO_TYPE_CAPTURE_SDI_25P;
				if(frequency == 24)
					peripheral->VideoType= VIDEO_TYPE_CAPTURE_SDI_24P;
			}

		}else{
			//sscanf(g_resolution,"HDMI%d%c_%d_%dHz",&hz,&iorp,&otherfbl,&frequency);
			sscanf(g_resolution+3,"I%d%c_%d_%dHz",&hz,&iorp,&otherfbl,&frequency);	//linxj 2012-12-21 to support HDMI and HDVI
			printf("hz = %d,iorp = %c\n",hz,iorp);
			if(hz == 50 && iorp =='i')
				peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_50i;
			else if(hz == 50 && iorp == 'p')
				peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_50p;
			else if(hz == 60 && iorp == 'i')
				peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_60i;
			else;
			switch (otherfbl){
				case 480:
				peripheral->VideoWidth=720;
				peripheral->VideoHeight=480;
				break;
				case 576:
				peripheral->VideoWidth=720;
				peripheral->VideoHeight=576;
				break;
				case 720:
				peripheral->VideoWidth=1280;
				peripheral->VideoHeight=720;
				break;
				case 1080:
				peripheral->VideoWidth=1920;
				peripheral->VideoHeight=1080;
				break;
				}
			}
	//
	peripheral->AudioType=AUDIO_TYPE_STEREO;
	if(g_resolution[0]=='H'&&g_resolution[1]=='D'&&g_resolution[2]=='M'&&g_resolution[3]=='I')
	{//linxj 2012-12-21 //support HDMI and HDVI
		peripheral->AudioType=AUDIO_TYPE_HDMI;
	}
	if(g_resolution[0]=='S'&&g_resolution[1]=='D'&&g_resolution[2]=='I'&&g_resolution[3]=='A')
	{//linxj 2012-12-21 //support SDIA and SDIV
		peripheral->AudioType=AUDIO_TYPE_SDI;
	}
}
static int read_sysconf(char* buf, int buf_len) {
    int	n, err = 0;
    FILE *fp;

    if ((fp = fopen(SYSCONF_FILE, "r")) == NULL) {
        perror("fopen sysconf file for read");
        err = 1;
    }

    if (err == 0) {
        if ((n = fread(buf, sizeof(char), buf_len, fp)) <= 0) {
            perror("fread sysconfig file");
            err = 1;
        }
        if (n != buf_len && n!=128 ) {  //linxj 2012-04-12 
            ERR("sysconf file maybe destroyed! use default config.\n");
            err = 1;
        }
        fclose(fp);
    }
    if (err == 1) {
//		printf("type=%d, SDI=%d\n", visstatus.bdinfo.Boardtype, VIS_BOARD_SDIENC);
		switch (visstatus.bdinfo.Boardtype) {
			case VIS_BOARD_SDIENC :
				strncpy(buf, default_conf_sdi, strlen(default_conf_sdi));
				break;
			case VIS_BOARD_DVIENC :
				strncpy(buf, default_conf_dvi, strlen(default_conf_dvi));
				break;
			default :
				strncpy(buf, default_conf, strlen(default_conf));
				break;
		}
//		printf("buf = %s\n", buf);
    }

    return 0;
}

static int start_engine(char * argv[]) {
	int ret;
	char *slavename = SLAVE_NAME_DEFAULT;
    pid_t pid;
    pid = fork();
	if (pid < 0) {
		ERR("fork failed!\n");
		return -1;
	} else if (pid == 0) {
		pid = getpid();
		//DBG("I'm child %d! ready for transform!\n", pid);

		switch (g_netsendtype) {
#ifndef CUSTOM_SCC
			case NETSEND_TYPE_TCP:	//tcp
			{
				slavename = SLAVE_NAME_TCP;
				FILE* pf=fopen(slavename,"rb");
				if (pf==NULL) {
					slavename = SLAVE_NAME_DEFAULT;
					printf("no file:./visEnc.tcp\n");
					g_netsendtype = 1;
				} else {
					fclose(pf);
				}
				break;
			}

			case NETSEND_TYPE_RTSP:	//rtsp
			{
				slavename = SLAVE_NAME_RTSP;
				FILE* pf=fopen("/usr/lib/libstdc++.so.6","rb");
				if (pf==NULL) {
					slavename = SLAVE_NAME_DEFAULT;
					printf("no file:/usr/lib/libstdc++.so.6\n");
					g_netsendtype = 1;
				} else {
					fclose(pf);
					pf=fopen(slavename,"rb");
					if(pf==NULL){
						slavename = SLAVE_NAME_DEFAULT;
						printf("no file:./visEnc.rtsp\n");
					}
				}
				break;
			}
			
			case NETSEND_TYPE_C1:	//weiqian
			{
				slavename = SLAVE_NAME_C1;
				FILE* pf=fopen(slavename,"rb");
				if (pf==NULL) {
					slavename = SLAVE_NAME_DEFAULT;
					printf("no file:./visEnc.tcp\n");
					g_netsendtype = 1;
				} else {
					fclose(pf);
				}
				break;
			}

#else
			case NETSEND_TYPE_RES0:
			case NETSEND_TYPE_UDP:	//udp
			{
				slavename = SLAVE_NAME_UDP;
				FILE* pf=fopen(slavename,"rb");
				if (pf==NULL) {
					slavename = SLAVE_NAME_DEFAULT;
					printf("no file:./visEnc.slave\n");
					g_netsendtype = 4;
				} else {
					fclose(pf);
				}
				break;
			}

			case NETSEND_TYPE_TCP:	//udp
			{
				slavename = SLAVE_NAME_TCP;
				FILE* pf=fopen(slavename,"rb");
				if (pf==NULL) {
					slavename = SLAVE_NAME_DEFAULT;
					printf("no file:./visEnc.tcp\n");
					g_netsendtype = 4;
				} else {
					fclose(pf);
				}
				break;
			}

			case NETSEND_TYPE_RTSP:	//rtsp
			{
				slavename = SLAVE_NAME_RTSP;
				FILE* pf=fopen("/usr/lib/libstdc++.so.6","rb");
				if (pf==NULL) {
					slavename = SLAVE_NAME_DEFAULT;
					printf("no file:/usr/lib/libstdc++.so.6\n");
					g_netsendtype = 4;
				} else {
					fclose(pf);
					pf=fopen(slavename,"rb");
					if(pf==NULL){
						slavename = SLAVE_NAME_DEFAULT;
						printf("no file:./visEnc.rtsp\n");
					}
				}
				break;
			}
#endif
		}
		printf("slave_name:%s\n\n", slavename);
		ret= execvp(slavename, argv);
		if(ret==-1)	return -1;
	} else {
		//DBG("fork success!\n");
	}
	return pid;
}

static int init_sysconf(char * dest_argv[], char * src_argv) {
    int pos, i = 0;
    char *src_p = src_argv;

    dest_argv[0] = src_p;
    for(i = 1; i < FPA_LEN; i++) {
		pos = strcspn(src_p, " ");
		if (pos == 0) break;
		src_p += (pos + 1);
		dest_argv[i] = src_p; 
    }
    if (i != FPA_LEN) {
        printf("i = %d\n", i);
        return -1;
    }

    pos = strcspn(dest_argv[TARGET_IP_VALUE], " ");
    memcpy(g_ip, dest_argv[TARGET_IP_VALUE], pos);
    if (pos < IP_LEN)
		g_ip[pos] = '\0';
    g_port = atoi(dest_argv[PORT_VALUE]);
    g_framerate = atoi(dest_argv[FRAMERATE_VALUE]);
    g_kbps = atoi(dest_argv[VIDEOBITRATE_VALUE]);
    g_cbr = atoi(dest_argv[VIDEOCBR_VALUE]);
    g_iinterval = atoi(dest_argv[VIDEOIDR_VALUE]);

    g_audio_hz = atoi(dest_argv[AUDIOSAMPLERATE_VALUE]);
    g_audio_kbps = atoi(dest_argv[AUDIOBITRATE_VALUE]);
    g_video = atoi(dest_argv[VIDEOENABLE_VALUE]);
    g_audio = atoi(dest_argv[AUDIOENABLE_VALUE]);
    g_lum = atoi(dest_argv[VIDEOLUM_VALUE]);
    g_volume = atoi(dest_argv[AUDIOVOLUME_VALUE]);
    g_videowidth = atoi(dest_argv[VIDEOWIDTH_VALUE]);
    g_videoheight = atoi(dest_argv[VIDEOHEIGHT_VALUE]);
    g_netsendtype = atoi(dest_argv[SENDTYPE_VALUE]);

    pos = strcspn(dest_argv[RESOLUTION_VALUE], " ");
    memcpy(g_resolution, dest_argv[RESOLUTION_VALUE], pos);
    if (pos < RESOLUTION_LEN)
        g_resolution[pos] = '\0';

//	DBG("%s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d %d %d %d %s\n", g_ip, g_port, g_framerate, g_kbps, g_cbr, g_iinterval, g_audio_hz, g_audio_kbps, g_video, g_audio, g_lum, g_volume, g_videowidth, g_videoheight, g_resolution);
    return 0;
}

int recreate_fpa(char * dest_argv[], char * old_conf) 
{
    int i;
    int pos;
    char *src_p = old_conf;

    snprintf(old_conf,OLD_CONF_LEN,"visEnc.slave -d %s -p %d -f %d -b %d -c %d -i %d -s %d -t %d -v %d -a %d -l %d -g %d -w %d -h %d -r %s -n %d"
            , g_ip, g_port, g_framerate, g_kbps, g_cbr, g_iinterval, g_audio_hz, g_audio_kbps,
            g_video, g_audio, g_lum, g_volume, g_videowidth, g_videoheight, g_resolution,g_netsendtype);
    DBG("check argv :%s\n", old_conf);

    dest_argv[0] = src_p;
    for(i = 1; i < FPA_LEN; i++) {
        pos = strcspn(src_p, " ");
        if (pos == 0)
            break;
        src_p += (pos + 1);
        dest_argv[i] = src_p;
    }
    if (i != FPA_LEN)
        return -1;
    return 0;
}

static void signalHandler(int signum) {
	switch (signum) {
		case SIGCHLD :
		{
			int	stat_val;
			pid_t child_pid;
			child_pid = wait(&stat_val);
			if (slave_pid == child_pid) {
				global_flag_e = 1;
				child_status = 0;
			}
			break;
		}

		case SIGUSR1 :
		{
			--restart_times;
			printf("master: signal handle SIGUSR1, restart_times=%d\n", restart_times);
			break;
		}
	}
}

static int getPoweronNum()
{
    FILE *fp;
    int num=1;
    fp = fopen(POWERONFILE,"r");
    if(fp)
    {
        fscanf(fp,POWERONOPT,&num);
        fclose(fp);
        num++;  //increase it
    }
    if(num<1) num=1;
    fp = fopen(POWERONFILE,"w");
    if(fp)
    {
        fprintf(fp,POWERONOPT,num);
        fclose(fp);
    }

    return num;
}
void getversion(BoardInfo *boardinfo)
{
    FILE *fp;
    char buf[LINELEN];
    char *ptr;
    int boardtype,version1,version2;

    fp = fopen(KERNALVERSION,"r");
    if(fp)
    {
        //read from /proc/viscodec/version
        //printf("read version from %s \n",KERNALVERSION);
        boardtype=version1=version2=0;
        fscanf(fp,"Version:%d.%d;Boardtype:%d",&version1,&version2,&boardtype);
        if(boardtype==0)
        {
            printf("boardtype error! \n");
            boardtype = VIS_BOARD_ENC;
        }
            boardinfo->Boardtype = boardtype;//=VIS_BOARD_ENC;
            boardinfo->KernelVersion_Major = version1;
            boardinfo->KernelVersion_Minor = version2;
    }else
    {
        //read from /root/version.ini
    fp = fopen(VERSION, "r");
    while (fgets(buf, LINELEN, fp) != NULL) {
        ptr = index(buf, '=');
        *ptr = '\0';
        if (strcmp(buf, "Boardtype") == 0) {
            boardinfo->Boardtype = atoi(++ptr);
        }else if (strcmp(buf, "KernelVersion_Major") == 0) {
            boardinfo->KernelVersion_Major = atoi(++ptr);
        }else if (strcmp(buf, "KernelVersion_Minor") == 0) {
            boardinfo->KernelVersion_Minor = atoi(++ptr);
        }
    }
    }
    if(fp)
    fclose(fp);
}

static int serv_listen(void)
{
    int listenfd, opt = 1;
    struct sockaddr_in servaddr;
    struct timeval tv = {2, 0};

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
    }
    if ( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0 ) {
        perror("setsockopt reuse");
    }    
    if (setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv)) < 0) {
        perror("setsockopt timeout");
    }    

    bzero(&servaddr, sizeof(servaddr));
    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(CLIENT_CMDTCPPORT);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
		close(listenfd);
    }
	if (listen(listenfd, 10) < 0) {
		close(listenfd);
	}        

    return listenfd;
}

void getvisstatus(net_config *net_cfg, VisStatus *visstatus)
{
    Vis_rq_video_setting	video_setting;
    Vis_rq_audio_setting	audio_setting;
    Peripheral      peripheral;
    VideoQuality    video_quality;
    NetWorking		networking;
    BoardInfo		boardinfo;

#if 0	//get net status test
	net_config net_cfg1;
	memset(&net_cfg1, 0, sizeof(net_cfg1));
	if (-1 == net_getstatus(&net_cfg1)) {
		printf("get net status return -1\n");
	}
	printf("ipAddr=0x%x\n", net_cfg1.ip);
	printf("mask=0x%x\n", net_cfg1.mask);
	net_cfg->ip = net_cfg1.ip;
	net_cfg->mask = net_cfg1.mask;
	memcpy(net_cfg->mac, net_cfg1.mac, sizeof(net_cfg->mac));
	printf("mac=%02x %02x %02x %02x %02x %02x\n", net_cfg->mac[0], net_cfg->mac[1], net_cfg->mac[2], net_cfg->mac[3], net_cfg->mac[4], net_cfg->mac[5]);

#endif
#if 0
//	if (net_cfg->isdhcp) {
		if (-1 == net_getstatus(net_cfg)) {
			printf("get net status return -1\n");
		}
//	}
#endif

    memset(&video_setting, 0, sizeof(video_setting));
    memset(&audio_setting, 0, sizeof(audio_setting));
    memset(&peripheral, 0, sizeof(peripheral));
    memset(&video_quality, 0, sizeof(video_quality));
    memset(&networking, 0, sizeof(networking));
    memset(&boardinfo, 0, sizeof(boardinfo));

    boardinfo.IPAddr = net_cfg->ip;
    boardinfo.IPMask = net_cfg->mask;
    boardinfo.GateIP = net_cfg->gateway;
    memcpy(&(boardinfo.MAC), &(net_cfg->mac), 6);
    memcpy(&(boardinfo.BoardName), &(net_cfg->name), 32);
//	printf("IP=0x%08x, mask=0x%08x, Gate=0x%08x\n", boardinfo.IPAddr, boardinfo.IPMask, boardinfo.GateIP);
//	printf("boardinfo.mac=%02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n", boardinfo.MAC[0], boardinfo.MAC[1], boardinfo.MAC[2], boardinfo.MAC[3], boardinfo.MAC[4], boardinfo.MAC[5]);
    getversion(&boardinfo);
    boardinfo.AppVersion_Major = APPVERSION_MAJOR;
    boardinfo.AppVersion_Minor = APPVERSION_MINOR;  //linxj:2011-08-02 
    boardinfo.size = sizeof(boardinfo);
    memcpy(&(visstatus->bdinfo), &boardinfo, sizeof(boardinfo));

    video_setting.vis_rq_video_bitrate = g_cbr;
    video_setting.vis_rq_video_kbps = g_kbps;
    video_setting.vis_rq_video_fps = g_framerate;
    video_setting.vis_rq_video_idrinterval = g_iinterval;
    video_setting.vis_rq_video_enable = g_video;

    peripheral.AudioType = AUDIO_TYPE_STEREO; //linxj2012-12-21
    if (strcmp(g_resolution, "SVGA") == 0) {
        video_setting.vis_rq_video_width = 800;
        video_setting.vis_rq_video_height = 600;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "XGA") == 0) {
        video_setting.vis_rq_video_width = 1024;
        video_setting.vis_rq_video_height = 768;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "SXGA") == 0) {
        video_setting.vis_rq_video_width = 1280;
        video_setting.vis_rq_video_height = 1024;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "WXGAI") == 0) {
        video_setting.vis_rq_video_width = 1280;
        video_setting.vis_rq_video_height = 768;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "WXGAII") == 0) {
        video_setting.vis_rq_video_width = 1440;
        video_setting.vis_rq_video_height = 900;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "WXGAIII") == 0) {
        video_setting.vis_rq_video_width = 1280;
        video_setting.vis_rq_video_height = 800;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "WXGAIV") == 0) {
        video_setting.vis_rq_video_width = 1280;
        video_setting.vis_rq_video_height = 720;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "WXGAV") == 0) {
        video_setting.vis_rq_video_width = 1360;
        video_setting.vis_rq_video_height = 768;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "WXGAVI") == 0) {
        video_setting.vis_rq_video_width = 1920;
        video_setting.vis_rq_video_height = 1080;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "XVGA") == 0) {
        video_setting.vis_rq_video_width = 1280;
        video_setting.vis_rq_video_height = 960;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "SXGAPLUS") == 0) {
        video_setting.vis_rq_video_width = 1400;
        video_setting.vis_rq_video_height = 1050;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "WSXGAPLUS") == 0) {
        video_setting.vis_rq_video_width = 1680;
        video_setting.vis_rq_video_height = 1050;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "UXGA") == 0) {//linxj 2011-11-21
        video_setting.vis_rq_video_width = 1600;
        video_setting.vis_rq_video_height = 1200;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "WXGAVII") == 0) {//linxj 2012-09-14
        video_setting.vis_rq_video_width = 1600;
        video_setting.vis_rq_video_height = 900;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "WUXGA") == 0) {//linxj 2012-09-14
        video_setting.vis_rq_video_width = 1920;
        video_setting.vis_rq_video_height = 1200;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "1080P50") == 0) {
        video_setting.vis_rq_video_width = 1920;
        video_setting.vis_rq_video_height = 1080;
        peripheral_type = VIDEO_TYPE_CAPTURE_YPbPr50p;
    } else if (strcmp(g_resolution, "1080P60") == 0) {
        video_setting.vis_rq_video_width = 1920;
        video_setting.vis_rq_video_height = 1080;
        peripheral_type = VIDEO_TYPE_CAPTURE_YPbPr60p;
    } else if (strcmp(g_resolution, "1080I50") == 0) {
        video_setting.vis_rq_video_width = 1920;
        video_setting.vis_rq_video_height = 1080;
        peripheral_type = VIDEO_TYPE_CAPTURE_YPbPr50i;
    } else if (strcmp(g_resolution, "1080I60") == 0) {
        video_setting.vis_rq_video_width = 1920;
        video_setting.vis_rq_video_height = 1080;
        peripheral_type = VIDEO_TYPE_CAPTURE_YPbPr60i;
    } else if (strcmp(g_resolution, "1080P25") == 0) {//linxj 2011-12-21
        video_setting.vis_rq_video_width = 1920;
        video_setting.vis_rq_video_height = 1080;
        peripheral_type = VIDEO_TYPE_CAPTURE_YPbPr25p;
    } else if (strcmp(g_resolution, "1080P30") == 0) {
        video_setting.vis_rq_video_width = 1920;
        video_setting.vis_rq_video_height = 1080;
        peripheral_type = VIDEO_TYPE_CAPTURE_YPbPr30p;
    } else if (strcmp(g_resolution, "720P50") == 0) {
        video_setting.vis_rq_video_width = 1280;
        video_setting.vis_rq_video_height = 720;
        peripheral_type = VIDEO_TYPE_CAPTURE_YPbPr50p;
    } else if (strcmp(g_resolution, "720P60") == 0) {
        video_setting.vis_rq_video_width = 1280;
        video_setting.vis_rq_video_height = 720;
        peripheral_type = VIDEO_TYPE_CAPTURE_YPbPr60p;
    } else if (strcmp(g_resolution, "576P") == 0) {
        video_setting.vis_rq_video_width = 720;
        video_setting.vis_rq_video_height = 576;
        peripheral_type = VIDEO_TYPE_CAPTURE_YPbPr50p;
    } else if (strcmp(g_resolution, "480P") == 0) {
        video_setting.vis_rq_video_width = 720;
        video_setting.vis_rq_video_height = 480;
        peripheral_type = VIDEO_TYPE_CAPTURE_YPbPr60p;
    } else if (strcmp(g_resolution, "HDMI1080P") == 0) {//linxj 2012-05-28
        video_setting.vis_rq_video_width = 1920;
        video_setting.vis_rq_video_height = 1080;
        peripheral_type = VIDEO_TYPE_CAPTURE_HDMI;
    } else if (strcmp(g_resolution, "HDMI720P") == 0) {//linxj 2012-05-28
        video_setting.vis_rq_video_width = 1280;
        video_setting.vis_rq_video_height = 720;
        peripheral_type = VIDEO_TYPE_CAPTURE_HDMI;
	} else if (strcmp(g_resolution, "VGA_CUSTOM") == 0 || strcmp(g_resolution, "HDMI_CUSTOM") == 0) {//ls 2012-10-24
		//TODO
		int err = 0;
		Capture_customer_param cparam;
		FILE *fp_custom = fopen(CUSTOMSTD_FILE, "rb");
		if (fp_custom) {
			memset(&cparam, 0, sizeof(cparam));
			if (sizeof(Capture_customer_param) != fread(&cparam, 1, sizeof(cparam), fp_custom)) {
				printf("<w>master: read customstd.cfg error\n");
				err = 1;
			}
		} else {
			perror("<w>master: open customstd.cfg failed");
			err = 1;
		}
		if (!err) {
			video_setting.vis_rq_video_width = cparam.width;
			video_setting.vis_rq_video_height = cparam.height;
		} else {
			video_setting.vis_rq_video_width = CUSTOMSTD_WIDTH_DEFAULT;
			video_setting.vis_rq_video_height = CUSTOMSTD_HEIGHT_DEFAULT;
		}
		peripheral_type = (strncmp(g_resolution, "HDMI_CUSTOM", strlen("HDMI_CUSTOM")))?VIDEO_TYPE_CAPTURE_VGA_CUSTOMER:VIDEO_TYPE_CAPTURE_HDMI_CUSTOMER;
		if (fp_custom) {
			fclose(fp_custom);
			fp_custom = NULL;
		}
    } else if (strcmp(g_resolution, "AUTO") == 0) {
        video_setting.vis_rq_video_width = 0;
        video_setting.vis_rq_video_height = 0;
        peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
    } else if (strcmp(g_resolution, "HDMI_AUTO") == 0) { //linxj2012-12-21
        video_setting.vis_rq_video_width = 0;
        video_setting.vis_rq_video_height = 0;
        peripheral_type = VIDEO_TYPE_CAPTURE_HDMI;
		peripheral.AudioType = AUDIO_TYPE_HDMI;
    } else if (strcmp(g_resolution, "HDVI_AUTO") == 0) { //linxj2012-12-21
        video_setting.vis_rq_video_width = 0;
        video_setting.vis_rq_video_height = 0;
        peripheral_type = VIDEO_TYPE_CAPTURE_HDMI;
    }   
    else{
		gresolution_to_valum(&peripheral,g_resolution);
		video_setting.vis_rq_video_width = peripheral.VideoWidth;
		video_setting.vis_rq_video_height = peripheral.VideoHeight;
		peripheral_type = peripheral.VideoType;
	}
    peripheral.VideoWidth = video_setting.vis_rq_video_width;
    peripheral.VideoHeight = video_setting.vis_rq_video_height;
    //if (video_setting.vis_rq_video_width != 0 && video_setting.vis_rq_video_height != 0) //linxj 2011-12-20 when AUTO
    {
        //if( (g_videowidth != 0 && g_videoheight != 0)
        //  && (g_videowidth != video_setting.vis_rq_video_width || g_videoheight != video_setting.vis_rq_video_height) )
        //linxj 2011-12-21
        {
            video_setting.vis_rq_video_width = g_videowidth;
            video_setting.vis_rq_video_height = g_videoheight;
        }
    }
    video_setting.size = sizeof(video_setting);
    memcpy(&(visstatus->video), &video_setting, sizeof(video_setting));

    networking.DstUDPPORT = g_port;
    networking.UDPPORT = 0;
    networking.DstUDPIPAddr = inet_addr((char *)g_ip);
    networking.ServerIPAddr = htonl(INADDR_ANY);
    networking.sendType     = g_netsendtype;
    networking.size = sizeof(networking);
    memcpy(&(visstatus->network), &networking, sizeof(networking));

    audio_setting.vis_rq_audio_channel = 2;
    audio_setting.vis_rq_audio_kbps = g_audio_kbps;
    audio_setting.vis_rq_audio_sample = g_audio_hz;
    audio_setting.vis_rq_audio_enable = g_audio;
    audio_setting.size = sizeof(audio_setting);
    memcpy(&(visstatus->audio), &audio_setting, sizeof(audio_setting));

    peripheral.VideoType = peripheral_type;
    //peripheral.AudioType = AUDIO_TYPE_STEREO; //linxj2012-12-21 //Should set it before gresolution_to_valum()
    peripheral.AudioSample = g_audio_hz;
    peripheral.size = sizeof(peripheral);
    memcpy(&(visstatus->periph), &peripheral, sizeof(peripheral));

    video_quality.bright = g_lum;
    video_quality.size = sizeof(video_quality);
    memcpy(&(visstatus->videoparam), &video_quality, sizeof(video_quality));

    visstatus->size = sizeof(*visstatus);
}

#define	HORI_START 0x01c70418
#define	VERT_START 0x01c70410

int get_videodim(VideoDimension *videodim)
{
    int fd = -1;
    unsigned int tmp[2] = {0};

    if ((fd = open(REG_MODIFY_FILE, O_RDONLY)) == -1) {
        perror("open REG_MODIFY_FILE");
        return -1;
    }

    tmp[0] = HORI_START;
    read(fd, tmp, sizeof(tmp));
    videodim->basex = tmp[1];
    tmp[0] = VERT_START;
    read(fd, tmp, sizeof(tmp));
    videodim->basey = tmp[1];

/*
    tmp[0] = HINTVL;
    read(fd, tmp, sizeof(tmp));
    videodim->hint = tmp[1];
    tmp[0] = VINTVL;
    read(fd, tmp, sizeof(tmp));
    videodim->vint = tmp[1];
*/

    if (videodim->reg != -1 && videodim->val == -1) {
        tmp[0] = videodim->reg;
        read(fd, tmp, sizeof(tmp));
        //videodim->reg = -1;
        videodim->val = tmp[1];
    }

    close(fd);

    return 0;
}

int set_videodim(VideoDimension *videodim)
{
    int fd = -1;
	FILE *dimfp  = NULL;
    unsigned int tmp[2] = {0};

    if ((fd = open(REG_MODIFY_FILE, O_WRONLY)) == -1) {
        perror("open REG_MODIFY_FILE");
        return -1;
    }
	if(videodim->width>0 && videodim->height>0 )
	{
		if (videodim->basex != -1) {
			tmp[0] = HORI_START;
		    tmp[1] = videodim->basex;
		    write(fd, tmp, sizeof(tmp));
		}
		if (videodim->basey != -1) {
			tmp[0] = VERT_START;
		    tmp[1] = videodim->basey;
		    write(fd, tmp, sizeof(tmp));
		}
    }

    if (videodim->reg > 0 && videodim->val != -1) {
        tmp[0] = videodim->reg;
        tmp[1] = videodim->val;
        write(fd, tmp, sizeof(tmp));
    }

	if(videodim->width>0 && videodim->height>0 )
	{
		capturedim cap;
		int i,index,find; 
		memset(&cap,0,sizeof(cap));
		if ((dimfp = fopen(CAPTUREDIM_FILE, "r")) == 0) {
		    perror("open CAPTUREDIM_FILE for read");
		}else
		{
			fread(&cap,1,sizeof(cap),dimfp);
		    fclose(dimfp);
		}

		index=-1;		
		find = 0;
		for(i=0;i<MAXNUM_RESOLUTION;i++)
		{
			if(cap._setting[i].width==videodim->width && cap._setting[i].height==videodim->height)
			{
				index = i;
				find = 1;
				break;
			}
		}
		if(find==0)
		{
			for(i=0;i<MAXNUM_RESOLUTION;i++)
			{
				if(cap._setting[i].width==0 && cap._setting[i].height==0)
				{
					index = i;
					break;
				}
			}
		}
		if(index!=-1)
		{
			cap._setting[index].width = videodim->width;
			cap._setting[index].height= videodim->height;
			cap._setting[index].hori_start = videodim->basex;
			cap._setting[index].vert_start = videodim->basey;
			//save it when changed
			if ((dimfp = fopen(CAPTUREDIM_FILE, "w")) == 0) {
				perror("open CAPTUREDIM_FILE for write");
			} else {
				if (fwrite( &cap,1, sizeof(cap),dimfp) == -1) {
				    perror("write CAPTUREDIM_FILE");
				}
				fclose(dimfp);
			}
		}

	}

    close(fd);
    return 0;
}

/*******************************************************************************************************************
 * main
 *******************************************************************************************************************/
int main(int argc, char *argv[]) 
{
    FILE	*sysconf_fp = NULL;
    char	old_conf[OLD_CONF_LEN];
    char    *front_progress_argv[FPA_LEN + 1];
    int		listenfd, connfd = -1, filefd = -1;
    struct sockaddr_in cliaddr;
    socklen_t cliaddr_len = sizeof(struct sockaddr_in);
    int     fifofd;
    int		message_len;
    int     button_flg = 1;
    int     dynamic_flg = 0;
    int    videotype_flag = 0;
    char	sendbuf[SEND_BUFLEN];
	char    recvbuf[RECV_BUFLEN + 200];
    char	*ip;
    struct in_addr	ipaddr;
    struct sigaction	sigAction;
    struct sched_param  schedParam;
    pthread_t refreshThread;
    pthread_t monitorThread;
    pthread_t serialThread;
    pthread_attr_t attr;
	int ret = 0;

    Vis_rq_video_setting	video_setting;
    Vis_rq_audio_setting	audio_setting;
    Peripheral      peripheral;
    VideoQuality    video_quality;
    AudioVolume     audio_volume;
    NetWorking		networking;
    Vis_con			vis_con;
    BoardInfo		boardinfo;
    UserParams      user_params;
    UserData        user_data;
    DynamicParams   dynamic_params;
    DynamicParam    video_dynamicparams;
    VideoDimension  videodimension;
	EncodeOSD encodeosd;
	struct ifreq;

	Fifo_M2S_Msg fifomsg;
	fifomsg.sync[0] = 'v';
	fifomsg.sync[1] = 'i';
	fifomsg.sync[2] = 's';
	fifomsg.sync[3] = 'h';

	if (access(FIFODIR, 0)) {
		perror("<w>FIFODIR is not access");
		if (mkdir(FIFODIR, 0666)) {
			perror("[E]mkdir for FIFODIR failed");
		}
	}
	if (mkfifo(FIFO_M2S_MAIN, 0666) == -1) {
		if (errno != EEXIST) perror("mkfifo of main failed");
	}
	if (mkfifo(FIFO_M2S_CAPTURE, 0666) == -1) {
		if (errno != EEXIST) perror("mkfifo of capture failed");
	}
 	if (mkfifo(FIFO_M2S_VIDEO, 0666) == -1) {
		if (errno != EEXIST) perror("mkfifo of video failed");
	}
	if (mkfifo(FIFO_M2S_RESIZE, 0666) == -1) {
		if (errno != EEXIST) perror("mkfifo of resize failed");
	}
	if (mkfifo(FIFO_M2S_WRITER, 0666) == -1) {
		if (errno != EEXIST) perror("mkfifo of writer failed");
	}
	if (mkfifo(FIFO_M2S_AUDIO, 0666) == -1) {
		if (errno != EEXIST) perror("mkfifo of audio failed");
	}
	if (mkfifo(FIFO_M2S_SPEECH, 0666) == -1) {
		if (errno != EEXIST) perror("mkfifo of speech failed");
	}
	if (mkfifo(FIFO_M2S_UDP, 0666) == -1) {
		if (errno != EEXIST) perror("mkfifo of udp failed");
	}
	if (mkfifo(FIFO_M2S_TCP, 0666) == -1) {
		if (errno != EEXIST) perror("mkfifo of tcp failed");
	}
	if (mkfifo(FIFO_M2S_DEMAND, 0666) == -1) {
		if (errno != EEXIST) perror("mkfifo of demand failed");
	}
	if (mkfifo(FIFO_M2S_SAVE, 0666) == -1) {
		if (errno != EEXIST) perror("mkfifo of save failed");
	}

	memset (&slavestatus,8,sizeof(slavestatus));

    /* Set the priority of this whole process to max (requires root) */
    setpriority(PRIO_PROCESS, 0, -20);

    if (mkfifo(FIFONAME, 0644) == -1) {
        if (errno != EEXIST) perror("mkfifo of net");
    }
    if ((fifofd = open(FIFONAME, O_RDWR | O_NONBLOCK)) == -1) {
        perror("open fifo");
    }

//    if (mkfifo(VIDEOFIFONAME, 0644) == -1) {
//        perror("mkfifo of video");
//    }
//    if ((videofifofd = open(FIFO_M2S_VIDEO, O_RDWR | O_NONBLOCK)) == -1) {
//        perror("open videofifo");
//    }

    if (mkfifo(INFO2MASTER_NAME, 0644) == -1) {
        if (errno != EEXIST) perror("mkfifo of info2master");
    }

    if(net_init(&net_cfg) != 0) {
        ERR("net_init() failed");
    }
    net_modify(&net_cfg);
    boardinfo.IPAddr = net_cfg.ip;
    boardinfo.IPMask = net_cfg.mask;
    boardinfo.GateIP = net_cfg.gateway;
    memcpy(&(boardinfo.MAC), &(net_cfg.mac), 6);
    memcpy(&(boardinfo.BoardName), &(net_cfg.name), 32);

    /* insure a child cleanner */
    sigAction.sa_handler = signalHandler;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    sigaction(SIGCHLD, &sigAction, NULL);
    sigaction(SIGUSR1, &sigAction, NULL);

	getvisstatus(&net_cfg, &visstatus);		//ls 2013-04-22
//	getversion(&visstatus.bdinfo);		//ls 2013-04-22
    read_sysconf(old_conf, OLD_CONF_LEN);
    if (init_sysconf(front_progress_argv, old_conf) != 0) {
        ERR("init_sysconf() failed, use default param");
        (VIS_BOARD_SDIENC!=visstatus.bdinfo.Boardtype)?strcpy(old_conf, default_conf):strcpy(old_conf, default_conf_sdi);
        init_sysconf(front_progress_argv, old_conf);
        front_progress_argv[FPA_LEN] = NULL;
    }
    front_progress_argv[FPA_LEN] = NULL;

    //init running status;
    memset(&devstatus,0,sizeof(devstatus));
    devstatus.size = sizeof(devstatus);
    devstatus.poweronnum = getPoweronNum();
    devstatus.videobitrate= g_kbps;
    devstatus.videofps  = g_framerate;
    devstatus.videoidrinterval = g_iinterval;
    devstatus.audiobitrate = g_audio_kbps;
    devstatus.audiosamplerate = g_audio_hz;

    listenfd = serv_listen();

    /* Initialize the thread attributes */
    if (pthread_attr_init(&attr)) {
        ERR("Failed to initialize thread attrs");
    }

    /* Force the thread to use custom scheduling attributes */
    if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) {
        ERR("Failed to set schedule inheritance attribute");
    }

    /* Set the thread to be fifo real time scheduled */
    if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) {
        ERR("Failed to set FIFO scheduling policy");
    }

    schedParam.sched_priority = REFRESH_THREAD_PRIORITY;
    if (pthread_attr_setschedparam(&attr, &schedParam))
    {
        ERR("Failed to set scheduler parameters");
    }
    pthread_create(&refreshThread, NULL, refreshThrFxn, NULL);

    schedParam.sched_priority = MONITOR_THREAD_PRIORITY;
    if (pthread_attr_setschedparam(&attr, &schedParam))
    {
        ERR("Failed to set scheduler parameters\n");
    }
    pthread_create(&monitorThread, &attr, monitorThrFxn, NULL);

    schedParam.sched_priority = SERIAL_THREAD_PRIORITY;
    if (pthread_attr_setschedparam(&attr, &schedParam))
    {
        ERR("Failed to set scheduler parameters\n");
    }
    pthread_create(&serialThread, &attr, serialThrFxn, NULL);

    printf("Master:start child : %s \n",old_conf);
    if ((slave_pid = start_engine(front_progress_argv)) != -1) {
        child_status = 1;
    }

    while(1) {
		if (restart_flg) {
		    if (-1 == recreate_fpa(front_progress_argv, old_conf)) {
                ERR("recreate_fpa failed\n");
            }
	
		    if ((sysconf_fp = fopen(SYSCONF_FILE, "w")) == NULL) {
                perror("fopen sysconf file for write");
		    }
		    if (fwrite(old_conf, sizeof(char), OLD_CONF_LEN, sysconf_fp) <= 0) {
                perror("fwrite sysconf file");
	    	}
		    fclose(sysconf_fp);

		    if (net_cfg_flg) {
//				printf("net_cfg.isdhcp = %d\n", net_cfg.isdhcp);	//test
		        if (net_modify(&net_cfg) != 0) {
				    ERR("net_modify() failed\n");
		        }
		        net_cfg_flg = 0;
		    }

            if (button_flg == 0) {
                if (dynamic_flg == 0) {
                    if (global_flag_e == 0) {
                        kill(slave_pid, SIGINT);
                        child_status = 0;
                    }
                    while (global_flag_e != 1) {
                        printf("waiting ........global_flag_e = %d\n", global_flag_e);
                        sleep(1);
                    }
                    global_flag_e = 0;
            
                    printf("Master:restart child : %s \n",old_conf);
                    if ((slave_pid = start_engine(front_progress_argv)) != -1) {
                        child_status = 1;
                    }
                    restart_flg = 0;
                    button_flg = 1;
                } else {
                    dynamic_flg = 0;
                    restart_flg = 0;
                }
            }
		}
#if 1
        if (global_flag_e == 1) {
            sleep(1);
            global_flag_e = 0;
            if ((slave_pid = start_engine(front_progress_argv)) != -1) {
                child_status = 1;
                restart_times++;
            }
        }
#endif

        if (connfd != -1) {
            close(connfd);
            connfd = -1;
        }
		memset(&cliaddr, 0 , sizeof(cliaddr));
//		cliaddr_len = sizeof(cliaddr);
		if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len)) > 0) {
//			printf("master accept return, client_ip=%08x, client_port=%hu, cliaddr_len=%d\n", cliaddr.sin_addr, cliaddr.sin_port, cliaddr_len);
//			printf("master accept returd\n");
			message_len = recv(connfd, recvbuf, sizeof(recvbuf), 0);
//			printf("master after recv()\n");
			if (message_len > 0) {
				if (recvbuf[0] != 'v' || recvbuf[1] != 'i' || recvbuf[2] != 's') {
				    ERR("bad message, just drop!\n");
                    continue;
				} else {
                    button_flg = 1;
				    //DBG("message type! %x \n", recvbuf[3]);
				    switch (recvbuf[3]) {
                    case VIS_RQ_SETDEFAULT:
                        system("flash_eraseall /dev/mtd4; reboot -f \n");
                        break;
                    case VIS_RQ_CH0_VIDEOENABLE:
                        g_video = 1;
                        break;
                    case VIS_RQ_CH0_VIDEODISABLE:
                        g_video = 0;
                        break;
                    case VIS_RQ_CH0_AUDIOENABLE:
                        g_audio = 1;
                        break;
                    case VIS_RQ_CH0_AUDIODISABLE:
                        g_audio = 0;
                        break;

			        case VIS_RQ_SETVIDEOQUALITY:
			            if ((4 + sizeof(VideoQuality)) <= message_len) {
			                memcpy(&video_quality, recvbuf + 4, sizeof(VideoQuality));
			            } else {
			                ERR("bad message, just drop VIS_RQ_SETVIDEOQUALITY:!\n");
			            }
                        if (video_quality.bright >= 0 && video_quality.bright <= 100)
                            g_lum = video_quality.bright;
                        else g_lum = 80;

			            restart_flg = 1;
			            printf("global_flag_e:%d\n", global_flag_e);
                        break;

			        case VIS_RQ_SETAUDIOVOLUME:
			        {
						int fd_fifo_volume = 0;
			            if ((4 + sizeof(AudioVolume)) <= message_len) {
			                memcpy(&audio_volume, recvbuf + 4, sizeof(AudioVolume));
			            } else {
			                ERR("bad message, just drop VIS_RQ_SETAUDIOVOLUME:!\n");
			            }
                        if (audio_volume.volume >= 0 && audio_volume.volume <= 200)
                            g_volume = audio_volume.volume;
                        else g_volume = 160;
						if ((fd_fifo_volume = open(FIFO_VOLUME, O_WRONLY)) < 0) {
							perror("master open volume fifo error");
							break;
						}
						printf("apps.main.c:write fifo volume = %d\n", g_volume);
						if (write(fd_fifo_volume, &g_volume, sizeof(g_volume)) != sizeof(g_volume)) {
							perror("write volume to fifo error");
							close(fd_fifo_volume);
							break;
						}
						printf("master volume has been set to %d\n", (int)g_volume);

						close(fd_fifo_volume);
						
//			            restart_flg = 1;
			            printf("global_flag_e:%d\n", global_flag_e);
                        break;
                    }

			        case VIS_RQ_CH0_AUDIOSETTING:
			            if ((4 + sizeof(Vis_rq_audio_setting)) <= message_len) {
			                memcpy(&audio_setting, recvbuf + 4, sizeof(Vis_rq_audio_setting));
			            } else {
			                ERR("bad message, just drop VIS_RQ_CH0_AUDIOSETTING:!\n");
			            }

                        if (audio_setting.vis_rq_audio_kbps <= 288
                            && audio_setting.vis_rq_audio_kbps >= 8) {
                            g_audio_kbps = audio_setting.vis_rq_audio_kbps;
                        } else {
                            g_audio_kbps = 128;
                        }

                        switch (audio_setting.vis_rq_audio_sample) {
                        case 48000:
                        case 44100:
                        case 32000:
                        case 24000:
                        case 16000:
                        case 12000:
                        case 8000:
                            g_audio_hz = audio_setting.vis_rq_audio_sample;
                            break;
                        default:
                            g_audio_hz = 44100;
                            break;
                        }
                        g_audio = audio_setting.vis_rq_audio_enable;

			            restart_flg = 1;
			            printf("global_flag_e:%d\n", global_flag_e);
			            break;

			        case VIS_RQ_CH0_VIDEOSETTING:
					{
#ifdef DYNAMIC_VIDEO_PARAM
						DynamicParam dynParams;
						int err_flag = 0, writelen = 0;
						int fifofd_m2s_video = open(FIFO_M2S_VIDEO, O_WRONLY);
						if (fifofd_m2s_video < 0) {
							ERR("open fifofd_m2s_video failed");
							err_flag = 1;
						}
#endif
			            if ((4 + sizeof(Vis_rq_video_setting)) <= message_len) {
			                memcpy(&video_setting, recvbuf + 4, sizeof(Vis_rq_video_setting));
			            } else {
			                ERR("bad message, just drop VIS_RQ_CH0_VIDEOSETTING:!\n");
			            }
						printf("master: VIDEO SETTING\n");
			    
			            g_cbr = video_setting.vis_rq_video_bitrate;
                        if (video_setting.vis_rq_video_kbps > 10000)
                            g_kbps = 5000;
                        else if (video_setting.vis_rq_video_kbps <= 0)
                            g_kbps = 500;
                        else
                            g_kbps = video_setting.vis_rq_video_kbps;
			            
                        if ((video_setting.vis_rq_video_fps <= 30 && video_setting.vis_rq_video_fps > 0)
							|| (video_setting.vis_rq_video_fps>100 && video_setting.vis_rq_video_fps<200))
                            g_framerate = video_setting.vis_rq_video_fps;
                        else
                            g_framerate = 30;
			            g_iinterval = video_setting.vis_rq_video_idrinterval;
			            g_video = video_setting.vis_rq_video_enable;
                        g_videowidth = (video_setting.vis_rq_video_width + 0xf) & ~0xf;
                        g_videoheight = (video_setting.vis_rq_video_height + 0xf) & ~0xf;
			            printf("master: videowidth = %d, videoheight = %d, fps=%hu, bps=%hu, internal=%hu, cbr=%hu\n", video_setting.vis_rq_video_width, video_setting.vis_rq_video_height, g_framerate, g_kbps, g_iinterval, g_cbr);
#ifdef DYNAMIC_VIDEO_PARAM
						memset(&dynParams, 0, sizeof(DynamicParam));
						dynParams.size = sizeof(DynamicParam);
						dynParams.videobitrate = g_kbps;
						dynParams.videofps = g_framerate;
						dynParams.videoidrinterval = g_iinterval;
						if (fifofd_m2s_video > 0) {
							writelen = write(fifofd_m2s_video, &dynParams, sizeof(DynamicParam));
							if (writelen != sizeof(DynamicParam)) {
								ERR("write to slave thread video failed, sizeof(DynamicParam)=%d, writelen=%d", sizeof(DynamicParam), writelen);
								err_flag = 1;
							} else {
								printf("write to slave thread video success, sizeof(DynamicParam)=%d, writelen=%d\n", sizeof(DynamicParam), writelen);
							}
						}
						if (err_flag) {
							printf("restart 1\n");
							restart_flg = 1;
							printf("global_flag_e:%d\n", global_flag_e);
						}
						close(fifofd_m2s_video);
#else
						printf("restart 2\n");
			            restart_flg = 1;
			            printf("global_flag_e:%d\n", global_flag_e);
#endif
			            break;
					}

                    case VIS_RQ_SETPERIPHERAL:
                        if ((4 + sizeof(Peripheral)) <= message_len) {
			                memcpy(&peripheral, recvbuf + 4, sizeof(Peripheral));
			            } else {
			                ERR("bad message, just drop VIS_RQ_SETPERIPHERAL:!\n");
			            }
						printf("video_type = %d, vga_custom = %d, hdmi_custom = %d\n", peripheral.VideoType, VIDEO_TYPE_CAPTURE_VGA_CUSTOMER, VIDEO_TYPE_CAPTURE_HDMI_CUSTOMER);
						printf("video_width = %d\n", peripheral.VideoWidth);
						printf("video_height = %d\n", peripheral.VideoHeight);
                        peripheral_type = peripheral.VideoType;
						if(peripheral.VideoType== VIDEO_TYPE_CAPTURE_VGA ){
							if (peripheral.VideoWidth == 800 && peripheral.VideoHeight == 600) {
								memcpy(g_resolution, "SVGA", 4);
								g_resolution[4] = '\0';
							} else if (peripheral.VideoWidth == 1024 && peripheral.VideoHeight == 768) {
								memcpy(g_resolution, "XGA", 3);
								g_resolution[3] = '\0';
							} else if (peripheral.VideoWidth == 1280 && peripheral.VideoHeight == 1024) {
								memcpy(g_resolution, "SXGA", 4);
								g_resolution[4] = '\0';
							} else if (peripheral.VideoWidth == 1280 && peripheral.VideoHeight == 768) {
								memcpy(g_resolution, "WXGAI", 5);
								g_resolution[5] = '\0';
							} else if (peripheral.VideoWidth == 1440 && peripheral.VideoHeight == 900) {
								memcpy(g_resolution, "WXGAII", 6);
								g_resolution[6] = '\0';
							} else if (peripheral.VideoWidth == 1280 && peripheral.VideoHeight == 800) {
								memcpy(g_resolution, "WXGAIII", 7);
								g_resolution[7] = '\0';
							/*//linxj 2011-11-22
							} else if (peripheral.VideoWidth == 1280 && peripheral.VideoHeight == 720) {
								memcpy(g_resolution, "WXGAIV", 6);
								g_resolution[6] = '\0';
							*/
							} else if (peripheral.VideoWidth == 1360 && peripheral.VideoHeight == 768) { //not support yet
								memcpy(g_resolution, "WXGAV", 5);
								g_resolution[5] = '\0';
							/*//linxj 2011-11-22
							} else if (peripheral.VideoWidth == 1920 && peripheral.VideoHeight == 1080) {
								memcpy(g_resolution, "WXGAVI", 6);
								g_resolution[6] = '\0';
								*/
							} else if (peripheral.VideoWidth == 1280 && peripheral.VideoHeight == 960) {
								memcpy(g_resolution, "XVGA", 4);
								g_resolution[4] = '\0';
							} else if (peripheral.VideoWidth == 1400 && peripheral.VideoHeight == 1050) {
								memcpy(g_resolution, "SXGAPLUS", 8);
								g_resolution[8] = '\0';
							} else if (peripheral.VideoWidth == 1680 && peripheral.VideoHeight == 1050) {
								memcpy(g_resolution, "WSXGAPLUS", 9);
								g_resolution[9] = '\0';
							} else if (peripheral.VideoWidth == 1600 && peripheral.VideoHeight == 1200) {//linxj 2011-11-21
								memcpy(g_resolution, "UXGA", 4);
								g_resolution[4] = '\0';
							} else if (peripheral.VideoWidth == 1600 && peripheral.VideoHeight == 900) {//linxj 2012-09-14
								strcpy(g_resolution, "WXGAVII");
							} else if (peripheral.VideoWidth == 1920 && peripheral.VideoHeight == 1200) {//linxj 2012-09-14
								strcpy(g_resolution, "WUXGA");
							} else if (peripheral.VideoWidth == 1920 && peripheral.VideoHeight == 1080) { 
									//linxj 2011-11-22
									//memcpy(g_resolution, "1080I50", 7);
									//g_resolution[7] = '\0';
									memcpy(g_resolution, "WXGAVI", 6);
									g_resolution[6] = '\0';
									peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
							} else if (peripheral.VideoWidth == 1280 && peripheral.VideoHeight == 720) {
									//linxj 2011-11-22
									//memcpy(g_resolution, "720P50", 6);
									//g_resolution[6] = '\0';
									memcpy(g_resolution, "WXGAIV", 6);
									g_resolution[6] = '\0';
									peripheral_type = VIDEO_TYPE_CAPTURE_VGA;
							} else if (peripheral.VideoWidth == 720 && peripheral.VideoHeight == 576) {
								memcpy(g_resolution, "576P", 4);
								g_resolution[4] = '\0';
							} else if (peripheral.VideoWidth == 720 && peripheral.VideoHeight == 480) {
								memcpy(g_resolution, "480P", 4);
								g_resolution[4] = '\0';
							}else if(peripheral.VideoWidth <= 0 || peripheral.VideoHeight <= 0){
								memcpy(g_resolution, "AUTO", 4);
								g_resolution[4] = '\0';
							}else{
								videotype_flag = 1;
							}
						} else if(peripheral.VideoType==VIDEO_TYPE_CAPTURE_YPbPr50p){
							if (peripheral.VideoWidth == 1920 && peripheral.VideoHeight == 1080) {
									memcpy(g_resolution, "1080P50", 7);
									g_resolution[6] = '\0';
							} else if (peripheral.VideoWidth == 1280 && peripheral.VideoHeight == 720){
									memcpy(g_resolution, "720P50", 6);
									g_resolution[6] = '\0';
							}else{
									videotype_flag = 1;
							}
						} else if(peripheral.VideoType==VIDEO_TYPE_CAPTURE_YPbPr60p){	
							if (peripheral.VideoWidth == 1920 && peripheral.VideoHeight == 1080){
									memcpy(g_resolution, "1080P60", 7);
									g_resolution[7] = '\0';
							}else if (peripheral.VideoWidth == 1280 && peripheral.VideoHeight == 720) {
									memcpy(g_resolution, "720P60", 6);
									g_resolution[6] = '\0';
							}else{
								videotype_flag = 1;
							}
						} else if(peripheral.VideoType==VIDEO_TYPE_CAPTURE_YPbPr50i){
							if (peripheral.VideoWidth == 1920 && peripheral.VideoHeight == 1080){
								memcpy(g_resolution, "1080I50", 7);
										g_resolution[7] = '\0';
							}else{
								videotype_flag = 1;
							}
						} else if(peripheral.VideoType==VIDEO_TYPE_CAPTURE_YPbPr60i){
							if (peripheral.VideoWidth == 1920 && peripheral.VideoHeight == 1080) {
										memcpy(g_resolution, "1080I60", 7);
										g_resolution[7] = '\0';
							}else{
									videotype_flag = 1;
							} 
						} else if (peripheral.VideoType == VIDEO_TYPE_CAPTURE_YPbPr30p){
							if (peripheral.VideoWidth == 1920 && peripheral.VideoHeight == 1080) {//linxj 2011-12-21
								memcpy(g_resolution, "1080P30", 7);
								g_resolution[7] = '\0';
							} else {
								 videotype_flag = 1;
							}
						} else if (peripheral.VideoType == VIDEO_TYPE_CAPTURE_YPbPr25p) {
							if (peripheral.VideoWidth == 1920 && peripheral.VideoHeight == 1080){
								memcpy(g_resolution, "1080P25", 7);
								g_resolution[7] = '\0';
							} else{
									videotype_flag = 1;
							}
						} else if (peripheral.VideoType == VIDEO_TYPE_CAPTURE_HDMI){
							if (peripheral.VideoWidth == 1280 && peripheral.VideoHeight == 720){//linxj 2012-05-28
								strcpy(g_resolution, "HDMI720P");
							}else if (peripheral.VideoWidth <=0 && peripheral.VideoHeight <= 0){//linxj 2012-12-21
								strcpy(g_resolution, "HDMI_AUTO");
							if(peripheral.AudioType != AUDIO_TYPE_HDMI)
								strcpy(g_resolution, "HDVI_AUTO");
							} else{
								videotype_flag = 1;
							}
						} else if (peripheral.VideoType == VIDEO_TYPE_CAPTURE_VGA_CUSTOMER) {
							strcpy(g_resolution, "VGA_CUSTOM");
						} else if (peripheral.VideoType == VIDEO_TYPE_CAPTURE_HDMI_CUSTOMER) {
							strcpy(g_resolution, "HDMI_CUSTOM");
						}else{
							videotype_flag = 1;
						}
						if (APPVERSION_MAJOR>=1 && APPVERSION_MINOR>=5) {
							videotype_flag = 1;
						}
						if (videotype_flag == 1){
							valum_to_gresolution(&peripheral,g_resolution);
							videotype_flag = 0;
						}
			            printf("[vis_daemon] get resolution %s\n", g_resolution);

			            restart_flg = 1;
			            printf("global_flag_e:%d\n", global_flag_e);
                        break;

			        case VIS_RQ_SETNETWORK:
			            if ((4 + sizeof(NetWorking)) <= message_len) {
			                memcpy(&networking, recvbuf + 4, sizeof(NetWorking));
			            } else {
			                ERR("bad message, just drop VIS_RQ_SETNETWORK:!\n");
			            }
			            ipaddr.s_addr = networking.DstUDPIPAddr;
			            ip = inet_ntoa(ipaddr);
			            printf("receive ip = %s, ipaddr = 0x%x\n", ip, ipaddr.s_addr);
                        printf("sendType = %d\n", networking.sendType);
			            memset(g_ip, 0, IP_LEN);
			            memcpy(g_ip, ip, strlen(ip));

						printf("g_netsendtype=%d, networking.sendtype=%d\n", g_netsendtype, networking.sendType);
						if (g_netsendtype != networking.sendType) { //ls 2013-04-07
							restart_flg = 1;
							g_netsendtype = networking.sendType;//linxj2012-05-17
						}
                        if ((networking.DstUDPIPAddr != 0) && (networking.DstUDPPORT == 0))
                            g_port = 1234;
                        else
                            g_port = networking.DstUDPPORT;
			            if ((networking.sendType == 3) && (networking.DstUDPPORT == 0))
                            g_port = 1234;
			            if (networking.sendType == 3||networking.sendType == 4 || networking.sendType == 5) {//tcp or c1 or rtsp mode
			                restart_flg = 1;
			                printf("global_flag_e:%d\n", global_flag_e);
                            break;
                        }

						if (0 == restart_flg) {			//ls 2013-04-07
							memset(dynamic_params.ip, 0, IP_LEN);
							memcpy(dynamic_params.ip, g_ip, sizeof(g_ip));
							dynamic_params.port = g_port;
							dynamic_params.iframe = 0;
							write(fifofd, &dynamic_params, sizeof(dynamic_params));
							printf("dynamic_params.ip = %s\n", dynamic_params.ip);
							printf("dynamic_params.port = %d\n", dynamic_params.port);

							//DBG("[vis_daemon] VIS_RQ_SETNETWORK \n");
							restart_flg = 1;
							dynamic_flg = 1;
						}
			            break;
			
			        case VIS_RQ_RESTARTCMD:
			            system("/sbin/reboot -f \n");
			            break;
			
			        case VIS_RQ_SETBDINFO:
			            if ((4 + sizeof(BoardInfo)) <= message_len) {
			                memcpy(&boardinfo, recvbuf + 4, sizeof(BoardInfo));
			            } else {
			                ERR("bad message, just drop VIS_RQ_SETBDINFO:!\n");
			    		}
//						printf("IP=0x%08x, mask=0x%08x, Gate=0x%08x\n", boardinfo.IPAddr, boardinfo.IPMask, boardinfo.GateIP);
						printf("Mac from Dialog: %02x %02x %02x %02x %02x %02x\n", boardinfo.MAC[0], boardinfo.MAC[1], boardinfo.MAC[2], boardinfo.MAC[3], boardinfo.MAC[4], boardinfo.MAC[5]);
						//IPMask, IPAddr, GateIP, all 0, then dhcp
						if (0==boardinfo.IPMask && 0==boardinfo.IPAddr && 0==boardinfo.GateIP) {
#ifdef DHCP
							struct timeval tv;
							printf("master:dhcp\n");
							net_cfg.isdhcp = 1;
//							system("udhcpc\n");
							memcpy(net_cfg.mac, boardinfo.MAC, sizeof(net_cfg.mac));
							net_cfg.mac[0] = 0x32;
							if (0 == gettimeofday(&tv, NULL)) {
								net_cfg.mac[1] = ((unsigned int)tv.tv_usec)&0xff;
							} else {
								perror("<w>master: set mac from time error");
							}
							printf("master: net_cfg.mac=%02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n", net_cfg.mac[0], net_cfg.mac[1], net_cfg.mac[2], net_cfg.mac[3], net_cfg.mac[4], net_cfg.mac[5]);
//							sleep(5);
#else
								printf("<w>:master:board info network params invalid\n");
								break;
#endif
						} else {
							if (0==boardinfo.IPMask || 0==boardinfo.IPAddr || 0==boardinfo.GateIP || (boardinfo.IPMask&boardinfo.IPAddr)!=(boardinfo.IPMask&boardinfo.GateIP)) {
								printf("<w>:master:board info network params invalid\n");
								break;
							} 
#ifdef DHCP
							if (net_cfg.isdhcp) {
								if (-1 == killname("udhcpc")) {
									printf("<w>master: kill udhcpc failed\n");
								}
							}
							net_cfg.isdhcp = 0;
#endif
							if ((boardinfo.IPMask&boardinfo.IPAddr) == (boardinfo.IPMask&boardinfo.GateIP)) {
								net_cfg.ip = boardinfo.IPAddr;
								net_cfg.mask = boardinfo.IPMask;
								net_cfg.gateway = boardinfo.GateIP;
								memcpy(&(net_cfg.mac), &(boardinfo.MAC), 6);
								if (net_cfg.mac[0] % 2) net_cfg.mac[0] = 0x30;
//								net_cfg.mac[0] = 0x30;
							}
						}
                        memcpy(&(net_cfg.name), &(boardinfo.BoardName), 32);
			            //DBG("VIS_RQ_SETBDINFO restart_flg is set... ... ...!\n");
			            net_cfg_flg = 1;
			            restart_flg = 1;
			            break;

                    case VIS_RQ_SETUSERPARAMS:
			            if ((4 + sizeof(UserParams)) <= message_len) {
			                memcpy(&user_params, recvbuf + 4, sizeof(UserParams));
			            } else {
			                ERR("bad message, just drop VIS_RQ_SETUSERPARAMS:!\n");
			            }
                        memset(dynamic_params.ip, 0, IP_LEN);
                        dynamic_params.port = -1;
                        dynamic_params.iframe = user_params.iFrame;
                        write(fifofd, &dynamic_params, sizeof(dynamic_params));
                        printf("dynamic_params.iframe = %d\n", dynamic_params.iframe);

			            //DBG("[vis_daemon] VIS_RQ_SETUSERPARAMS \n");
			            restart_flg = 1;
			            dynamic_flg = 1;
			            break;

                    case VIS_RQ_SETUSERDATA:
			            if ((4 + sizeof(UserData)) <= message_len) {
			                memcpy(&user_data, recvbuf + 4, sizeof(UserData));
			            } else {
			                ERR("bad message, just drop VIS_RQ_SETUSERDATA:!\n");
			            }
                        if ((filefd = open(USERDATA_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
                            perror("open USERDATA_FILE to write");
                        } else {
                            write(filefd, user_data.data, user_data.dataLen);
                            close(filefd);
                        }

			            //DBG("[vis_daemon] VIS_RQ_SETUSERDATA \n");
			            break;

			        case VIS_RQ_GETUSERDATA:
			            vis_con.vis_con_syn0='v';
			            vis_con.vis_con_syn1='i';
			            vis_con.vis_con_syn2='s';
			            vis_con.vis_con_cmd = VIS_RS_SENDUSERDATA;
			            memset(sendbuf, 0, SEND_BUFLEN);
			            memcpy(sendbuf, &vis_con, sizeof(vis_con));

			            memset(&user_data, 0, sizeof(user_data));
                        if ((filefd = open(USERDATA_FILE, O_RDONLY)) == -1) {
                            perror("open USERDATA_FILE to read");
                        } else {
                            user_data.dataLen = read(filefd, user_data.data, sizeof(user_data.data));
                            close(filefd);
                        }
                        user_data.size = sizeof(user_data);
			            memcpy(sendbuf + sizeof(vis_con), &user_data, sizeof(user_data));

			            message_len = send(connfd, sendbuf, sizeof(user_data) + sizeof(vis_con), 0);
			            if (-1 == message_len) {
			                perror("send failed");
			            }
         
			            //DBG("[VIS_RQ_GETUSERDATA] message_len = %d, userdata = %d!\n",message_len, sizeof(user_data.dataLen));
			            break;

			        case VIS_RQ_GETSTATUS:
			            vis_con.vis_con_syn0='v';
			            vis_con.vis_con_syn1='i';
			            vis_con.vis_con_syn2='s';
			            vis_con.vis_con_cmd = VIS_RS_SENDSTATUS;
			            memset(sendbuf, 0, SEND_BUFLEN);
			            memcpy(sendbuf, &vis_con, sizeof(vis_con));

                        pthread_mutex_lock(&mutex);
                        getvisstatus(&net_cfg, &visstatus);
                        pthread_mutex_unlock(&mutex);
			            memcpy(sendbuf + sizeof(vis_con), &visstatus, sizeof(visstatus));
			                    
//						printf("master: type=%d, WxH=%dx%d, VGA_CUSTOM=%d, HDMI_CUSTOM=%d\n", visstatus.periph.VideoType, visstatus.periph.VideoWidth, visstatus.periph.VideoHeight, VIDEO_TYPE_CAPTURE_VGA_CUSTOMER, VIDEO_TYPE_CAPTURE_HDMI_CUSTOMER);
			            message_len = send(connfd, sendbuf, sizeof(visstatus) + sizeof(vis_con), 0);
			            if (-1 == message_len) {
			                perror("send failed");
			            }
         
			            //DBG("[VIS_RQ_GETSTATUS] message_len = %d, visstatus = %d!\n",message_len, sizeof(visstatus));
			            break;

			        case VIS_RQ_CH0_DYNAMICPARAM:
					{
						int fifofd_m2s_video = open(FIFO_M2S_VIDEO, O_WRONLY);
						if (-1 == fifofd_m2s_video) {
							ERR("open fifo_m2s_video failed\n");
							break;
						}
                        //DynamicParam
			            if ((4 + sizeof(DynamicParam)) <= message_len) {
			                memcpy(&video_dynamicparams, recvbuf + 4, sizeof(DynamicParam));
                            if(video_dynamicparams.size==sizeof(DynamicParam))
                            {
                                printf("get video dynamic setting: bitrate=%d,fps=%d idr=%d! \n",
                                video_dynamicparams.videobitrate,video_dynamicparams.videofps,video_dynamicparams.videoidrinterval);
								fifomsg.type = FIFO_M2S_VIDEO_DYNAMICPARAMS;
								if (sizeof(fifomsg) != write(fifofd_m2s_video, &fifomsg, sizeof(fifomsg))) {
									ERR("write fifomsg to videoThr error\n");
									close(fifofd_m2s_video);
									break;
								}
                                if (sizeof(video_dynamicparams) != write(fifofd_m2s_video, &video_dynamicparams, sizeof(video_dynamicparams))) {
									ERR("write fifomsg to videoThr error\n");
									close(fifofd_m2s_video);
									break;
								}
								//TODO, change viscfgenc.system
                            }
			            } else {
			                ERR("bad message, just drop VIS_RQ_SETAUDIOVOLUME:!\n");
			            }
						close(fifofd_m2s_video);
                        break;
					}
                    case VIS_RQ_GETRUNNINGSTATUS:
			            vis_con.vis_con_syn0='v';
			            vis_con.vis_con_syn1='i';
			            vis_con.vis_con_syn2='s';
			            vis_con.vis_con_cmd = VIS_RS_SENDRUNNINGSTATUS;
			            memset(sendbuf, 0, SEND_BUFLEN);
			            memcpy(sendbuf, &vis_con, sizeof(vis_con));

			            memcpy(sendbuf + sizeof(vis_con), &devstatus, sizeof(devstatus));
			                    
			            message_len = send(connfd, sendbuf, sizeof(devstatus) + sizeof(vis_con), 0);
			            if (-1 == message_len) {
			                perror("send failed");
			            }
         
			            //DBG("[VIS_RQ_GETRUNNINGSTATUS] message_len = %d, running status = %d!\n",message_len, sizeof(devstatus));
			            break;
			
                    case VIS_RQ_SETVIDEODIM:
			            if ((4 + sizeof(VideoDimension)) <= message_len) {
			                memcpy(&videodimension, recvbuf + 4, sizeof(VideoDimension));
			            } else {
			                ERR("bad message, just drop VIS_RQ_SETVIDEODIM:!\n");
                            break;
			            }
#if 0
                        printf("message_len = %d sizeof(VideoDimension) = %d\n", message_len, sizeof(VideoDimension));
                        printf("hp = 0x%x\n", videodimension.hp);
                        printf("vp = 0x%x\n", videodimension.vp);
                        printf("hint = 0x%x\n", videodimension.hint);
                        printf("vint = 0x%x\n", videodimension.vint);
                        printf("reg = 0x%x\n", videodimension.reg);
                        printf("val = 0x%x\n", videodimension.val);
#endif
                        if (videodimension.reg != -1 && videodimension.val == -1)
                            break;
                        set_videodim(&videodimension);
			            break;

                    case VIS_RQ_GETVIDEODIM:
							vis_con.vis_con_syn0='v';
			            vis_con.vis_con_syn1='i';
			            vis_con.vis_con_syn2='s';
			            vis_con.vis_con_cmd = VIS_RS_SENDVIDEODIM;
			            memset(sendbuf, 0, SEND_BUFLEN);
			            memcpy(sendbuf, &vis_con, sizeof(vis_con));

                        //memset(&videodimension, 0, sizeof(videodimension));
                        get_videodim(&videodimension);
                        videodimension.size = sizeof(videodimension);
			            memcpy(sendbuf + sizeof(vis_con), &videodimension, sizeof(videodimension));
			                    
#if 0
                        printf("send size = %d\n", sizeof(VideoDimension) + sizeof(vis_con));
                        printf("hp = 0x%x\n", videodimension.hp);
                        printf("vp = 0x%x\n", videodimension.vp);
                        printf("hint = 0x%x\n", videodimension.hint);
                        printf("vint = 0x%x\n", videodimension.vint);
                        printf("reg = 0x%x\n", videodimension.reg);
                        printf("val = 0x%x\n", videodimension.val);
#endif
			            message_len = send(connfd, sendbuf, sizeof(videodimension) + sizeof(vis_con), 0);
			            if (-1 == message_len) {
			                perror("send failed:");
			            }
			            break;
			case VIS_RQ_GETSLAVESTATUS:
				printf("send slavestatus\n");
				vis_con.vis_con_syn0='v';
				vis_con.vis_con_syn1='i';
				vis_con.vis_con_syn2='s';
				vis_con.vis_con_cmd = VIS_RS_GETSLAVESTATUS;
				memset(sendbuf, 0, SEND_BUFLEN);
				memcpy(sendbuf, &vis_con, sizeof(vis_con));
			
				memcpy(sendbuf + sizeof(vis_con), &slavestatus, sizeof(slavestatus));
						
				message_len = send(connfd, sendbuf, sizeof(slavestatus) + sizeof(vis_con), 0);
				if (-1 == message_len) {
					perror("send failed");
				}
				break ;
			case VIS_RQ_SETBOARDTIME:
			{
				BoardTime boardtime;
				struct tm tm_time;
				time_t time_t_time;
				printf("master: SETTIME 0\n");
				if ((4 + sizeof(boardtime)) <= message_len) {
					memcpy(&boardtime, recvbuf + 4, sizeof(boardtime));
				} else {
					ERR("bad message, just drop VIS_RQ_SETBOARDTIME:!\n");
					break;
				}
				if (connfd != -1) {
					close(connfd);
					connfd = -1;
				}
				printf("master: SETTIME 1\n");
	
				tm_time.tm_year	= 	boardtime.year;
				tm_time.tm_mon	=	boardtime.month;
				tm_time.tm_mday	=	boardtime.mday;
				tm_time.tm_yday	=	boardtime.yday;
				tm_time.tm_wday	=	boardtime.wday;
				tm_time.tm_hour	=	boardtime.hour;
				tm_time.tm_min	=	boardtime.min;
				tm_time.tm_sec	=	boardtime.sec;
				tm_time.tm_isdst =  boardtime.isdst;
				printf("master: SETTIME 2\n");
				time_t_time  =  mktime(&tm_time);
				stime(&time_t_time);
				printf("master: SETTIME 3\n");
//				system("hwclock -w");	//linxj2012-12-21, set to RTC
				if (-1 == set_hw_time(&tm_time)) {
					printf("<w>master: set hwclock failed!\n");
				} else {
					printf("master: set hwclock success!\n");
//					system("hwclock -r");
				} 
				printf("master: SETTIME 4\n");
				break;
			}
			case VIS_RQ_GETBOARDTIME:
			{
				BoardTime boardtime;
				time_t timenow;
				struct tm *timenow_tm;
				printf("get board time\n");				
				time(&timenow);
				timenow_tm = localtime(&timenow);
				
				boardtime.year  =  timenow_tm->tm_year;
				boardtime.month =  timenow_tm->tm_mon;
				boardtime.mday  =  timenow_tm->tm_mday;
				boardtime.yday  =  timenow_tm->tm_yday;
				boardtime.wday  =  timenow_tm->tm_wday;
				boardtime.hour  =  timenow_tm->tm_hour;
				boardtime.min   =  timenow_tm->tm_min;
				boardtime.sec   =  timenow_tm->tm_sec;
				boardtime.isdst =  timenow_tm->tm_isdst;
				printf("send board time\n");
				vis_con.vis_con_syn0='v';
				vis_con.vis_con_syn1='i';
				vis_con.vis_con_syn2='s';
				vis_con.vis_con_cmd = VIS_RS_SENDBOARDTIME;
				memset(sendbuf, 0, SEND_BUFLEN);
				memcpy(sendbuf, &vis_con, sizeof(vis_con));
				memcpy(sendbuf + sizeof(vis_con), &boardtime, sizeof(boardtime));
				message_len = send(connfd, sendbuf, sizeof(boardtime) + sizeof(vis_con), 0);
				if (-1 == message_len) {
					perror("send failed");
				}
				break;
			}
			case VIS_RQ_SETOSD:
			{
				int fifofd_m2s_capture	= 0;
				if ((4 + sizeof(encodeosd)) <= message_len) {
					memcpy(&encodeosd, recvbuf + 4, sizeof(encodeosd));
				} else {
//					printf("message_len = %d\n",message_len);
//					printf("4 + sizeof(encodeosd) = %d\n",4 + sizeof(encodeosd));
					ERR("bad message, just drop VIS_RQ_SETOSD:!\n");
					break;
				}
							
				if ((sysconf_fp = fopen(OSDCONF_FILE,"w")) == NULL) {
					perror("fopen osdcof file error");
					break;
				}
				if (fwrite(&encodeosd,1,sizeof(encodeosd),sysconf_fp) <= 0) {
					fclose(sysconf_fp);
					perror("fwrite osdcof file error");
				}			
				fclose(sysconf_fp);
				fifofd_m2s_capture = open(FIFO_M2S_CAPTURE, O_WRONLY);
				if (fifofd_m2s_capture == -1) {
					perror("open fifo_m2s_capture failed");
					break;
				}
				fifomsg.sync[0]='v';
				fifomsg.sync[1]='i';
				fifomsg.sync[2]='s';
				fifomsg.sync[3]='h';
				fifomsg.type = FIFO_M2S_CAPTURE_CHANGEOSD;
				ret = write(fifofd_m2s_capture, &fifomsg, sizeof(fifomsg));
//				printf("master:sync=%c%c%c%c, type=%d, ret=%d\n", fifomsg.sync[0],fifomsg.sync[1],fifomsg.sync[2],fifomsg.sync[3],fifomsg.type, ret);
				if (ret != sizeof(fifomsg)) {
					perror("master:send osd change to capture error");
					break;
				}
				close(fifofd_m2s_capture);fifofd_m2s_capture=-1;
				printf("set osd success\n");
				break;
			}
			case VIS_RQ_GETOSD:
				printf("get osd\n");
				if ((sysconf_fp = fopen(OSDCONF_FILE,"r"))) {
					if (fread(&encodeosd,1,sizeof(encodeosd),sysconf_fp) <= 0) {
						perror("fread osdcof file error");
						fclose(sysconf_fp);
						memset(&encodeosd, 0, sizeof(encodeosd));
					}
				} else {
					perror("fopen osdcof file error");
					memset(&encodeosd, 0, sizeof(encodeosd));
				}
				vis_con.vis_con_syn0='v';
				vis_con.vis_con_syn1='i';
				vis_con.vis_con_syn2='s';
				vis_con.vis_con_cmd = VIS_RS_SENDOSD;
				memset(sendbuf, 0, SEND_BUFLEN);
				memcpy(sendbuf, &vis_con, sizeof(vis_con));
				memcpy(sendbuf + sizeof(vis_con), &encodeosd, sizeof(encodeosd));
				message_len = send(connfd, sendbuf, sizeof(encodeosd) + sizeof(vis_con), 0);
//				printf("message_len = %d\n",message_len);
				if (-1 == message_len){
					perror("send failed");
					break;
				}
				break;
			case VIS_RQ_SETSAVESET:
			{
				Saveset saveset;
#ifdef REAL_TIME_SAVE_SET
				int fifofd_m2s_save = -1;
#endif
				int copylen;
				FILE *fp_saveconfig = NULL;
				printf("master:begin to set saveopt\n");
				memset(&saveset, 0, sizeof(saveset));
				if (message_len-4!=sizeof(saveset) && message_len!=sizeof(saveset) && message_len-8!=sizeof(saveset)) {
					printf("<w>master:recv_len error, len=%d, sizeof(saveset)=%d\n", message_len-4, sizeof(saveset));
					break;
				}
				copylen = (message_len-4>=sizeof(Saveset))?sizeof(saveset):(message_len-4);
				memcpy(&saveset, recvbuf+4, copylen);
				if (saveset.max_file_size == 0) {
					fp_saveconfig = fopen("/mnt/apps/configFile/save.config", "r");
					if (fp_saveconfig) {
					if (fscanf(fp_saveconfig, "MAX_FILE_SIZE:%uMB\n", &saveset.max_file_size) <= 0) {
						ERR("fscanf for max_file_size failed");
						saveset.max_file_size = MAX_FILE_SIZE_MB_DEFAULT;
						saveset.res_dev_size = RES_DEV_SIZE_MB_DEFAULT;
						saveset.max_data_size = MAX_DATA_SIZE_MB_DEFAULT;
						saveset.frames_per_save = FRAMES_PER_SAVE_DEFAULT;
						saveset.save_enable = SAVE_ENABLE_DEFAULT;
					}
					} else {
						ERR("open save.config failed");
						saveset.max_file_size = MAX_FILE_SIZE_MB_DEFAULT;
						saveset.res_dev_size = RES_DEV_SIZE_MB_DEFAULT;
						saveset.max_data_size = MAX_DATA_SIZE_MB_DEFAULT;
						saveset.frames_per_save = FRAMES_PER_SAVE_DEFAULT;
						saveset.save_enable = SAVE_ENABLE_DEFAULT;
					}
				}
				fp_saveconfig = fopen("/mnt/apps/configFile/save.config", "w");
				if (NULL == fp_saveconfig) {
					perror("<w>master:open save.config failed");
					saveset.max_file_size = MAX_FILE_SIZE_MB_DEFAULT;
					saveset.res_dev_size = RES_DEV_SIZE_MB_DEFAULT;
					saveset.max_data_size = MAX_DATA_SIZE_MB_DEFAULT;
					saveset.frames_per_save = FRAMES_PER_SAVE_DEFAULT;
					saveset.save_enable = SAVE_ENABLE_DEFAULT;
				}
				printf("MAX_FILE_SIZE:%uMB\n", saveset.max_file_size);
				printf("RES_DEV_SIZE:%uMB\n", saveset.res_dev_size);
				printf("MAX_DATA_SIZE:%uMB\n", saveset.max_data_size);
				printf("FRAMES_PER_SAVE:%u\n", saveset.frames_per_save);
				fprintf(fp_saveconfig, "MAX_FILE_SIZE:%uMB\n", saveset.max_file_size);
				fprintf(fp_saveconfig, "RES_DEV_SIZE:%uMB\n", saveset.res_dev_size);
				fprintf(fp_saveconfig, "MAX_DATA_SIZE:%uMB\n", saveset.max_data_size);
				fprintf(fp_saveconfig, "FRAMES_PER_SAVE:%u\n", saveset.frames_per_save);
				switch (message_len-4-sizeof(saveset)) {
					case 0 :
						printf("SAVE_ENABLE:%d\n", saveset.save_enable);
						fprintf(fp_saveconfig, "SAVE_ENABLE:%d\n", saveset.save_enable);
						break;
					case -4 :
						printf("<w>master: SAVE_ENABLE:%d, SAVESET req use old params\n", SAVE_ENABLE_DEFAULT);
						fprintf(fp_saveconfig, "SAVE_ENABLE:%d\n", SAVE_ENABLE_DEFAULT);
						break;
					case 4 :
						printf("<w>master: SAVESET req use new params, drop\n");
						break;
					default :
						printf("<w>master: SAVESET req params invalid, drop\n");
						break;
				}
				fclose(fp_saveconfig);
				fp_saveconfig = NULL;
#ifdef REAL_TIME_SAVE_SET
				fifomsg.sync[0] = 'v';
				fifomsg.sync[1] = 'i';
				fifomsg.sync[2] = 's';
				fifomsg.sync[3] = 'h';
				fifomsg.type = FIFO_M2S_SAVE_SAVESETCHANGE;
				fifofd_m2s_save = open(FIFO_M2S_SAVE, O_WRONLY);
//				fifofd_m2s_save = open(FIFO_M2S_SAVE, O_WRONLY|O_NONBLOCK);
				if (fifofd_m2s_save == -1) {
					perror("open fifo_m2s_save failed");
					break;
				}
				//DBG("before fifo write\n");
				ret = write(fifofd_m2s_save, &fifomsg, sizeof(fifomsg));
				//DBG("after fifo write, ret=%d\n", ret);
				printf("master:sync=%c%c%c%c, type=0x%04x, ret=%d\n", fifomsg.sync[0],fifomsg.sync[1],fifomsg.sync[2],fifomsg.sync[3],fifomsg.type, ret);
				if (ret != sizeof(fifomsg)) {
					perror("master:send saveset change to save error");
					close(fifofd_m2s_save);
					fifofd_m2s_save = -1;
					break;
				}
				close(fifofd_m2s_save);
				fifofd_m2s_save = -1;
#else
				restart_flg = 1;	//board need to reboot as the realtime fifo read in save_thread has been removed
#endif
				printf("set saveopt success\n");
				break;
			}
			case VIS_RQ_GETSAVESET:
			{
				FILE *fp_saveconfig = NULL;
				Store_Dev_Info devinfo;
//				fp_saveconfig = fopen("/mnt/apps/configFile/save.config", "r");
//				if (NULL == fp_saveconfig) {
//					perror("<w>master:open save.config failed");
//					break;
//				}
				memset(&devinfo, 0, sizeof(devinfo));
				if ((fp_saveconfig=fopen("/mnt/apps/configFile/save.config", "r"))) {
					fscanf(fp_saveconfig, "MAX_FILE_SIZE:%uMB\n", &devinfo.max_file_size);
					fscanf(fp_saveconfig, "RES_DEV_SIZE:%uMB\n", &devinfo.res_dev_size);
					fscanf(fp_saveconfig, "MAX_DATA_SIZE:%uMB\n", &devinfo.max_data_size);
					fscanf(fp_saveconfig, "FRAMES_PER_SAVE:%u\n", &devinfo.frames_per_save);
					ret = fscanf(fp_saveconfig, "SAVE_ENABLE:%d\n", &devinfo.save_enable);
					if (ret <= 0) {
						printf("master: load old saveopt params, save_enable=1 default\n");
						devinfo.save_enable = 1;
					}
					fclose(fp_saveconfig);
					fp_saveconfig = NULL;
					if (mount_point("/var/usbmedia")) {
						devinfo.total_dev_size = getDiskTotalSpace("/var/usbmedia/channel_1/");
						devinfo.usage_dev_size = getDiskUsageSpace("/var/usbmedia/channel_1/");
						devinfo.free_dev_size = getDiskFreeSpace("/var/usbmedia/channel_1/");
						devinfo.data_dev_size = getTotalDataMBytes("/var/usbmedia/channel_1/", &devinfo.file_count);
					}
//					devinfo.file_count = 0;
				} else {
					printf("<w>master: open save config file failed\n");
//					devinfo.save_enable = 1;
					devinfo.max_file_size = MAX_FILE_SIZE_MB_DEFAULT;
					devinfo.res_dev_size = RES_DEV_SIZE_MB_DEFAULT;
					devinfo.max_data_size = MAX_DATA_SIZE_MB_DEFAULT;
					devinfo.frames_per_save = FRAMES_PER_SAVE_DEFAULT;
				}
//				printf("master: save_enable=%d, max_file_size=%uMB, res_dev_size=%uMB, max_data_size=%uMB, total_dev_size=%uMB, usage_dev_size=%uMB, free_dev_size=%uMB, data_dev_size=%uMB\n", devinfo.save_enable, devinfo.max_file_size, devinfo.res_dev_size, devinfo.max_data_size, devinfo.total_dev_size, devinfo.usage_dev_size, devinfo.free_dev_size, devinfo.data_dev_size);
				printf("master: save_enable=%d, max_file_size=%uMB, res_dev_size=%uMB, max_data_size=%uMB, total_dev_size=%uMB, usage_dev_size=%uMB, free_dev_size=%uMB, data_dev_size=%uMB\n", devinfo.save_enable, devinfo.max_file_size, devinfo.res_dev_size, devinfo.max_data_size, devinfo.total_dev_size, devinfo.usage_dev_size, devinfo.free_dev_size, devinfo.data_dev_size);
				vis_con.vis_con_syn0='v';
				vis_con.vis_con_syn1='i';
				vis_con.vis_con_syn2='s';
				vis_con.vis_con_cmd = VIS_RS_SENDSAVESET;
				memset(sendbuf, 0, SEND_BUFLEN);
				memcpy(sendbuf, &vis_con, sizeof(vis_con));
				memcpy(sendbuf + sizeof(vis_con), &devinfo, sizeof(devinfo));
				message_len = send(connfd, sendbuf, sizeof(devinfo) + sizeof(vis_con), 0);
				printf("message_len = %d\n",message_len);
				if (-1 == message_len){
					perror("send failed");
					break;
				}
				break;
			}
			case VIS_RQ_SETSERIALCONFIG :
			{
				struct serial_config scfg;
				FILE *fp = NULL;
				memset(&scfg, 0, sizeof(struct serial_config));
				if ((4 + sizeof(scfg)) <= message_len) {
					memcpy(&scfg, recvbuf + 4, sizeof(scfg));
				} else {
					ERR("bad message, just drop VIS_RQ_SETSERIAL!message_len=%d, size=%d\n", message_len, sizeof(scfg)+4);
					break;
				}
				fp = fopen(SERIAL_CONFIG_FILE, "w");
				if (!fp) {
					perror("<w>master: set_serial open config file failed");
					break;
				}
				if (sizeof(struct serial_config) != fwrite(&scfg, 1, sizeof(scfg), fp)) {
					perror("<w>master: set_serial write to config error");
					fclose(fp);fp=NULL;
					break;
				}
				fclose(fp);fp=NULL;
				serial_flg = 1;
				printf("master: set_serial write to config success, protocol=%d, baudrate=%u\n", scfg.protocol, scfg.const_baudrate);
				break;
			}
			case VIS_RQ_GETSERIALCONFIG :
			{
				struct serial_config scfg;
				FILE *fp = NULL;
				int readlen;
				memset(&scfg, 0, sizeof(struct serial_config));
				fp = fopen(SERIAL_CONFIG_FILE, "r");
				if (fp) {
					if (sizeof(struct serial_config) != (readlen=fread(&scfg, 1, sizeof(scfg), fp))) {
						ERR("read from config error, readlen=%d", readlen);
						fclose(fp);fp=NULL;
					} else {
						fclose(fp);fp=NULL;
						printf("master: get_serial read from config success\n");
					}
				} else {
					perror("<w>master: get_serial open config file failed");
				}
				vis_con.vis_con_syn0='v';
				vis_con.vis_con_syn1='i';
				vis_con.vis_con_syn2='s';
				vis_con.vis_con_cmd = VIS_RS_SENDSERIALCONFIG;
				memset(sendbuf, 0, SEND_BUFLEN);
				memcpy(sendbuf, &vis_con, sizeof(vis_con));
				memcpy(sendbuf + sizeof(vis_con), &scfg, sizeof(scfg));
				message_len = send(connfd, sendbuf, sizeof(scfg) + sizeof(vis_con), 0);
				printf("message_len = %d\n",message_len);
				if (-1 == message_len){
					perror("send failed");
					break;
				}
				printf("master: get_serial read from config success, protocol=%d, baudrate=%u\n", scfg.protocol, scfg.const_baudrate);
				break;
			}
			default:
				//DBG("message type! %x \n", recvbuf[3]);
				break;
				}
			}
			} else {
//				printf("tcp recv error \n");
                
			}
			if (-1 != connfd) {
				close(connfd);
				connfd = -1;
			}
		} else {
            //printf("accept time out\n");
            button_flg = 0;
		}
   }

    if (listenfd >= 0)
		close(listenfd);

    return 0;
}

static unsigned int getDiskTotalSpace(char *dirname)//Mbyte
{
    unsigned long long blocksize=0,totalsize=0;
    struct statfs diskInfo;
    
    statfs(dirname,&diskInfo);
    blocksize = diskInfo.f_bsize;		//每个block里面包含的字节数
    totalsize = (blocksize * diskInfo.f_blocks)>>20;	//总的字节数
//	printf("TOTAL_SIZE == %lu MB\n",totalsize>>20);		// 1024*1024 =1MB  换算成MB单位
    return (unsigned int)totalsize;
}

static unsigned int getDiskUsageSpace(char *dirname)//Mbyte
{
    unsigned long long freeMByte=0,blocksize=0,totalsize=0, usagesize=0;
    struct statfs diskInfo;
    
    statfs(dirname,&diskInfo);
    blocksize = diskInfo.f_bsize;		//每个block里面包含的字节数
    totalsize = (blocksize * diskInfo.f_blocks)>>20;	//总的字节数
    freeMByte = (diskInfo.f_bfree*blocksize)>>20;		//再计算下剩余的空间大小
    //printf("DISK_FREE == %d MB \n",freeMByte);
	usagesize = totalsize-freeMByte;
    return (unsigned int)usagesize;
}

static unsigned int getDiskFreeSpace(char *dirname)//Mbyte
{
    unsigned long long freeMByte=0,blocksize=0,totalsize=0;
    struct statfs diskInfo;
    
    statfs(dirname,&diskInfo);
    blocksize = diskInfo.f_bsize;		//每个block里面包含的字节数
    totalsize = blocksize * diskInfo.f_blocks;	//总的字节数
//	printf("TOTAL_SIZE == %lu MB\n",totalsize>>20);		// 1024*1024 =1MB  换算成MB单位

    freeMByte = (diskInfo.f_bfree*blocksize)>>20;		//再计算下剩余的空间大小
    //printf("DISK_FREE == %d MB \n",freeMByte);
    return (unsigned int)freeMByte;
}

static unsigned int getTotalDataMBytes(char *dirname, unsigned int *data_file_count) {
	struct dirent *pdir = NULL;
	DIR *dir = NULL;
	unsigned long long totalDataMBytes = 0;
	char filename[64] = {0, };
	int index = 0;

	dir = opendir(dirname);
	while ((pdir=readdir(dir)) != NULL) {
		++index;
		strncpy(filename, dirname, strlen(dirname)+1);
		strncat(filename, pdir->d_name, strlen(pdir->d_name));
		totalDataMBytes += (get_file_size(filename)>>10);
	}
	closedir(dir);
	*data_file_count = index-3;		//except "filelist" "." & ".." 3 files
	return (unsigned int)((totalDataMBytes>>10)+1);
}

static unsigned long get_file_size(const char *path) {
	unsigned long filesize = -1;
	struct stat statbuff;
	if(stat(path, &statbuff) < 0){
		return filesize;  
	}else{
		filesize = statbuff.st_size;
	}
	return filesize;
}

int killname(char *cmd) {
	DIR *procdir = NULL;
	struct dirent *dp = NULL;
	FILE *fp = NULL;
	char buf[256] = {0, };
	pid_t pid = -1;
	int count = 0;

	if (NULL == cmd) {
		printf("[E]killname param invalid, cmd=%p\n", cmd);
		return -1;
	}

	procdir = opendir("/proc/");
	while(1) {
		dp = readdir(procdir);
		if (NULL == dp) {
			break;
		}
//		printf("%s\n", dp->d_name);
		if (dp->d_name[0] <= '9' && dp->d_name[0]>='0') {
			sprintf(buf, "/proc/%s/cmdline", dp->d_name);
			fp = fopen(buf, "rb");
			if (NULL == fp) {
				printf("<w>open [c]%s failed!:%s\n", buf, strerror(errno));
				continue;
			}
			fread(buf, 1, sizeof(buf), fp);
			fclose(fp);
			fp = NULL;
			if (0 == strncmp(cmd, buf, strlen(cmd))) {
				sprintf(buf, "/proc/%s/stat", dp->d_name);
				fp = fopen(buf, "r");
				if (NULL == fp) {
					printf("<w>open [s]%s failed!:%s\n", buf, strerror(errno));
					continue;
				}
				fscanf(fp, "%d ", &pid);
				if (-1 == kill(pid, SIGTERM)) {
					printf("<w>kill pid[%d] failed!:%s\n", pid, strerror(errno));
					continue;
				}
				fclose(fp);
				fp = NULL;
				printf("kill pid[%d] success!\n", pid);
				++count;
			}
		}
	}

//cleanup:
	if (procdir) {
		closedir(procdir);
		procdir = NULL;
	}
	return count;
}

static int dostat(char *path, struct stat *st, int do_lstat, int quiet)
{
	int		n;

	if (do_lstat)
		n = lstat(path, st);
	else
		n = stat(path, st);

	if (n != 0) {
		if (!quiet)
			fprintf(stderr, "mountpoint: %s: %s\n", path,
				strerror(errno));
		return -1;
	}
	return 0;
}

static int mount_point(char *path) {
	struct	stat	st, st2;
	char	buf[256];
	int		quiet = 1;
	int		showdev = 0;
	int		r;

	if (dostat(path, &st, 1, quiet) < 0)
		return 1;

	if (!S_ISDIR(st.st_mode)) {
		if (!quiet)
			fprintf(stderr, "mountpoint: %s: not a directory\n", path);
		return 1;
	}

	memset(buf, 0, sizeof(buf));
	strncpy(buf, path, sizeof(buf) - 4);
	strcat(buf, "/..");
	if (dostat(buf, &st2, 0, quiet) < 0)
		return 1;

	r = (st.st_dev != st2.st_dev) ||
	    (st.st_dev == st2.st_dev && st.st_ino == st2.st_ino);

	if (!quiet && !showdev)
		printf("%s is %sa mountpoint\n", path, r ? "" : "not ");
	if (showdev)
		printf("%u:%u\n", major(st.st_dev), minor(st.st_dev));

	return r;
}

static int set_hw_time(struct tm *tmtime) {
	int ret = 0, fd = -1;
	struct rtc_time rtctime;

	memset(&rtctime, 0, sizeof(rtctime));
	rtctime.tm_sec = tmtime->tm_sec;
	rtctime.tm_min = tmtime->tm_min;
	rtctime.tm_hour = tmtime->tm_hour;
	rtctime.tm_mday = tmtime->tm_mday;
	rtctime.tm_mon = tmtime->tm_mon;
	rtctime.tm_year = tmtime->tm_year;
	
	fd = open("/dev/rtc0", O_RDONLY);
	if (fd == -1) {
		ERR("open rtc0 device failed");
		ret = -1;
		goto cleanup;
	}
#if 0
	ret = ioctl(fd, RTC_RD_TIME, &rtctime);
	printf("ret=%d, rtctime=%04d.%02d.%02d-%02d:%02d:%02d\n", ret, rtctime.tm_year, rtctime.tm_mon, rtctime.tm_mday, rtctime.tm_hour, rtctime.tm_min, rtctime.tm_sec);
	perror("after get rtc time");
#endif

	ret = ioctl(fd, RTC_SET_TIME, &rtctime);
	if (ret == -1) {
		ERR("set rtc time failed");
		ret = -1;
		goto cleanup;
	}

cleanup:
	if (fd != -1) close(fd);
	return 0;
}
