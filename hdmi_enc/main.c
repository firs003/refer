/*
 * main.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <strings.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include <xdc/std.h>

#include <ti/sdo/ce/trace/gt.h>
#include <ti/sdo/ce/CERuntime.h>

#include <ti/sdo/dmai/Dmai.h>
#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Sound.h>
#include <ti/sdo/dmai/VideoStd.h>
#include <ti/sdo/dmai/Capture.h>
#include <ti/sdo/dmai/BufferGfx.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include <ti/sdo/fc/rman/rman.h>

#include "demand.h"
#include "video.h"
#include "audio.h"
#include "capture.h"
#include "writer.h"
#include "speech.h"
#include "../demo.h"
#include "vis_common.h"

#define SLAVEVERSION_MAJOR 2	//for HDMI
#define SLAVEVERSION_MINOR 5	//for dapt customer

#define USE_DEMAND
#define USE_DYNAMICTHREAD

/* The levels of initialization */
#define LOGSINITIALIZED         0x1
#define DYNAMICTHREADCREATED    0x2
#define DISPLAYTHREADCREATED    0x20
#define CAPTURETHREADCREATED    0x40
#define WRITERTHREADCREATED     0x80
#define VIDEOTHREADCREATED      0x100
#define AUDIOTHREADCREATED      0x400
#define SPEECHTHREADCREATED		0x400		//ls, 012-07-08
#define DEMANDTHREADCREATED     0x800

/* Thread priorities */
#define DEMAND_THREAD_PRIORITY  sched_get_priority_max(SCHED_FIFO) - 2
#define WRITER_THREAD_PRIORITY  sched_get_priority_max(SCHED_FIFO) - 1
#define SPEECH_THREAD_PRIORITY  sched_get_priority_max(SCHED_FIFO) - 1
#define AUDIO_THREAD_PRIORITY  sched_get_priority_max(SCHED_FIFO) - 1
#define VIDEO_THREAD_PRIORITY   sched_get_priority_max(SCHED_FIFO) - 1
#define CAPTURE_THREAD_PRIORITY sched_get_priority_max(SCHED_FIFO)
#define DYNAMIC_THREAD_PRIORITY  sched_get_priority_max(SCHED_FIFO) - 3

typedef struct Args {
    VideoStd_Type videoStd;
    Char          *videoStdString;
    Sound_Input   soundInput;
    Capture_Input videoInput;
    Char          *speechFile;
    Char          *videoFile;
    Char          *audioFile;
    Codec         *speechEncoder;
    Codec         *audioEncoder;
    Codec         *videoEncoder;
    Int32         imageWidth;
    Int32         imageHeight;
    Int           videoBitRate;
    Int           videoCbr;
    Int           keyboard;
    Int           time;
    Int           osd;
    Int           interface;
	/* xudonghai	10-29-09 */
	char		  ipAddr[ADDRSTRLEN];
	Int           videoFrameRate;
    Int           intraFrameInterval;   //sp 12-10-09
    Int           videoEnable;
    Int           audioEnable;
    Int           audioSampleRate;
    Int           audioBitRate;
    Int           lum;
    Int           volume;
} Args;

typedef struct capture_std {
	char	*stdstr;
	char	*stdaliars;
	int		stdno;
} CaptureStd, *pCaptureStd;

#define DEFAULT_ARGS \
    { VideoStd_XGA, "XGA 60Hz", Sound_Input_LINE, Capture_Input_COMPONENT, \
      NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, -1, FALSE, FALSE, FALSE, FALSE, \
      {FALSE}, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE}

/* Global variable declarations for this application */
GlobalData gbl = GBL_DATA_INIT;
vis_global_t	vis_global;	
short int port = 0;
int encodedFrameType = 1;
unsigned char idrhead[IDRHEAD_LEN];
conn_client_t conn_client;
DynamicParams dynamic_params;

/******************************************************************************
 * Signal handler
 ******************************************************************************/
Void signalHandler(int sig)
{
    signal(SIGINT, SIG_DFL);
    gblSetQuit();
}

/******************************************************************************
 * getCodec
 ******************************************************************************/
static Codec *getCodec(Char *extension, Codec *codecs)
{
    Codec *codec = NULL;
    Int i, j;

    i = 0;
    while (codecs[i].codecName) {
        j = 0;
        while (codecs[i].fileExtensions[j]) {
            if (strcmp(extension, codecs[i].fileExtensions[j]) == 0) {
                codec = &codecs[i];
            }
            j++;
        }
        i++;
    }

    return codec;
}

/******************************************************************************
 * usage
 ******************************************************************************/
static void usage(void)
{
#if 0
    fprintf(stderr, "Usage: encode [options]\n\n"
      "Options:\n"
      "-s | --speechfile       Speech file to record to\n"
      "-v | --videofile        Video file to record to\n"
      "-y | --display_standard Video standard to use for display (see below).\n"
      "-r | --resolution       Video resolution ('width'x'height')\n"
      "                        [video standard default]\n"
      "-b | --videobitrate     Bit rate to encode video at [variable]\n"
      "-x | --svideo           Use s-video instead of composite video \n"
      "                        input [off]\n"
      "-l | --linein           Use linein for encoding sound instead of mic \n"
      "                        [off]\n"
      "-k | --keyboard         Enable keyboard interface [off]\n"
      "-o | --osd              Show demo data on an OSD [off]\n"
      "-i | --interface        Launch the demo interface when exiting [off]\n"
      "-h | --help             Print this message\n\n"
      "Video standards available:\n"
      "\t1\tD1 @ 30 fps (NTSC) [Default]\n"
      "\t2\tD1 @ 25 fps (PAL)\n"
      "\t3\t720P @ 60 fps\n"
      "\t4\t720P @ 50 fps\n"
      "\t5\t1080I @ 30 fps [Not supported for DM365]\n"
      "\t6\t1080I @ 25 fps [Not supported for DM365]\n"
      "You must supply at least a video or a speech file or both\n"
      "with appropriate extensions for the file formats.\n\n");
#endif
    fprintf(stderr, "Usage: encode [options]\n\n"
      "You must supply at least a video or a speech file or both\n"
      "with appropriate extensions for the file formats.\n\n");
}

/******************************************************************************
 * parseArgs
 ******************************************************************************/
static Void parseArgs(Int argc, Char *argv[], Args *argsp)
{
	//xudonghai	10-29-09
    const Char shortOptions[] = "d:p:f:i:s:t:a:v:l:g:r:b:c:w:h:n:e";
	//const Char shortOptions[] = "s:v:y:r:b:xlkt:oih";
    const struct option longOptions[] = {
		//xudonghai	10-29-09
		{"ipaddr", 			 required_argument, NULL, 'd'},
		{"port", 			 required_argument, NULL, 'p'},
		{"framerate", 		 required_argument, NULL, 'f'},
		{"Intraframeinterval", required_argument, NULL, 'i'},
        {"audioenable",      required_argument, NULL, 'a'},
        {"videoenable",      required_argument, NULL, 'v'},
        {"resolution",       required_argument, NULL, 'r'},
        {"videobitrate",     required_argument, NULL, 'b'},
        {"videoratecontrol", required_argument, NULL, 'c'},
        {"audiosamplerate",  required_argument, NULL, 's'},
        {"audiobitrate",     required_argument, NULL, 't'},
        {"videolum",         required_argument, NULL, 'l'},
        {"volume",           required_argument, NULL, 'g'},
        {"videowidth",       required_argument, NULL, 'w'},
        {"videoheight",      required_argument, NULL, 'h'},
        {"netsendtype",      required_argument, NULL, 'n'},
        {"end",              no_argument, NULL, 'e'},
        {0, 0, 0, 0}
    };
    
	/* for new capture input type, ls, 2012-11-19 */
	const CaptureStd stdtab[] = {
		{"VGA_352x288_30Hz",		"CIF",		VideoStd_CIF},           /**< CIF @ 30 frames per second */
		{"VGA_352x240_30Hz",		"SIF_NTSC",	VideoStd_SIF_NTSC},      /**< SIF @ 30 frames per second */
		{"VGA_352x288_25Hz",		"SIF_PAL",	VideoStd_SIF_PAL},       /**< SIF @ 25 frames per second */
		{"VGA_640x480_60Hz",		"VGA",		VideoStd_VGA},           /**< VGA (640x480) @ 60 frames per second */
		{"VGA_720x480_30Hz",		"D1_NTSC",	VideoStd_D1_NTSC},       /**< D1 NTSC @ 30 frames per second */
		{"VGA_720x576_25Hz",		"D1_PAL",	VideoStd_D1_PAL},        /**< D1 PAL @ 25 frames per second */
		{"VGA_720x480_60Hz",		"480P",		VideoStd_480P},          /**< D1 Progressive NTSC @ 60 frames per second */
		{"VGA_720x576_50Hz",		"576P",		VideoStd_576P},          /**< D1 Progressive PAL @ 50 frames per second */
		{"YPbPr_1280x720_60Hz",		"720P60",	VideoStd_720P_60},       /**< 720P @ 60 frames per second, YPbPr */
		{"YPbPr_1280x720_50Hz",		"720P50",	VideoStd_720P_50},       /**< 720P @ 50 frames per second, YPbPr */
		{"YPbPr_1280x720_30Hz",		"720P_30",	VideoStd_720P_30},       /**< 720P @ 30 frames per second, YPbPr */
		{"YPbPr_1920x1080i_30Hz",	"1080I60",	VideoStd_1080I_30},      /**< 1080I @ 30 frames per second, YPbPr */
		{"YPbPr_1920x1080i_25Hz",	"1080I50",	VideoStd_1080I_25},      /**< 1080I @ 25 frames per second, YPbPr */
		{"YPbPr_1920x1080_30Hz",	"1080P_30",	VideoStd_1080P_30},      /**< 1080P @ 30 frames per second, YPbPr */
		{"YPbPr_1920x1080_25Hz",	"1080P_25",	VideoStd_1080P_25},      /**< 1080P @ 25 frames per second, YPbPr */
		{"YPbPr_1920x1080_24Hz",	"1080P_24",	VideoStd_1080P_24},      /**< 1080P @ 24 frames per second, YPbPr */
		{"VGA_800x600_60Hz",		"SVGA",		VideoStd_SVGA},          /**< 800x600 @ 60 frames per second */
		{"VGA_1024x768_60Hz",		"XGA",		VideoStd_XGA},           /**< 1024x768 @ 60 frames per second */    //sp 03-23-2010
		{"VGA_1280x1024_60Hz", 		"SXGA",		VideoStd_SXGA},          /**< 1280x1024 @ 60 frames per second */
		{"VGA_1600x1200_60Hz",		"UXGA",		VideoStd_UXGA},          /**< 1600x1200 @ 60 frames per second */
		{"", "NONE", VideoStd_NONE},
		{"VGA_1280x768_60Hz",	"WXGAI",	VideoStd_WXGAI},         /**< 1280x768 @ 60 frames per second */
		{"VGA_1440x900_60Hz",	"WXGAII",	VideoStd_WXGAII},        /**< 1440x900 @ 60 frames per second */
		{"VGA_1280x800_60Hz",	"WXGAIII",	VideoStd_WXGAIII},       /**< 1280x800 @ 60 frames per second */
		{"VGA_1280x720_60Hz",	"WXGAIV",	VideoStd_WXGAIV},        /**< 1280x720 @ 60 frames per second */
		{"VGA_1280x960_60Hz",	"XVGA",		VideoStd_XVGA},          /**< 1280x960 @ 60 frames per second */
		{"VGA_1360x768_60Hz",	"WXGAV",	VideoStd_WXGAV},         /**< 1360x768 @ 60 frames per second */
		{"VGA_1920x1080_60Hz", "WXGAVI",	VideoStd_WXGAVI},        /**< 1920x1080 @ 60 frames per second */
		{"VGA_1400x1050_60Hz", "SXGAPLUS",	VideoStd_SXGAPLUS},      /**< 1400x1050 @ 60 frames per second */
		{"VGA_1680x1050_60Hz", "WSXGAPLUS",	VideoStd_WSXGAPLUS},     /**< 1680x1050 @ 60 frames per second */
		{"VGA_1920x1080_60Hz", "1080P60",	VideoStd_1080P_60},      /**< 1920x1080 @ 60 frames per second} */
		{"VGA_1920x1080_50Hz", "1080P50",	VideoStd_1080P_50},      /**< 1920x1080 @ 50 frames per second} */
		{"VGA_1600x900_60Hz", "WXGAVII",	VideoStd_WXGAVII},       /**< 1600x900  @ 60 frames per second */    //linxj 2012-02-24
		{"VGA_1920x1200_60Hz", "WUXGA",		VideoStd_WUXGA},         /**< 1920x1200 @ 60 frames per second */
		{"VGA_800x600_75Hz", "SVGA_75",		VideoStd_SVGA_75},       /**< 800x600 @ 75 frames per second */
		{"VGA_1024x768_70Hz", "XGA_70",		VideoStd_XGA_70},        /**< 1024x768 @ 70 frames per second */
		{"VGA_1024x768_75Hz", "XGA_75",		VideoStd_XGA_75},        /**< 1024x768 @ 75 frames per second */
		{"VGA_1280x1024_75Hz", "SXGA_75",	VideoStd_SXGA_75},       /**< 1280x1024 @ 75 frames per second */
		{"", "VGA_CUSTOM", VideoStd_CUSTOM},
		{"HDMI_1280x720_60Hz",	"HDMI720P",		VideoStd_HDMI720P},     /**< HDMI 720P */
		{"HDMI_1920x1080_60Hz", "HDMI1080P",	VideoStd_HDMI1080P},    /**< HDMI 1080P */
		{"HDMI60i_1080_60Hz",	"HDMI1080I",	VideoStd_HDMI1080I},	/**< HDMI 1080I */
		{"VGA_1600x1200_75Hz",	"UXGA_75",		VideoStd_UXGA_75},		/**< 1600x1200 @ 75 frames per second */	//ls 2012-10-19
		{"VGA_1280x768_75Hz",	"WXGAI_75",		VideoStd_WXGAI_75},      /**< 1280x768 @ 75 frames per second */
		{"VGA_1440x900_75Hz",	"WXGAII_75",	VideoStd_WXGAII_75},     /**< 1440x900 @ 75 frames per second */
		{"VGA_1280x800_75Hz",	"WXGAIII_75",	VideoStd_WXGAIII_75},    /**< 1280x800 @ 75 frames per second */
		{"VGA_1280x720_75Hz",	"WXGAIV_75",	VideoStd_WXGAIV_75},     /**< 1280x720 @ 75 frames per second */
		{"VGA_1280x960_75Hz",	"XVGA_75",		VideoStd_XVGA_75},       /**< 1280x960 @ 75 frames per second */
		{"VGA_1375x768_75Hz",	"WXGAV_75",		VideoStd_WXGAV_75},      /**< 1375x768 @ 75 frames per second */
		{"VGA_1920x1080_75Hz",	"WXGAVI_75",	VideoStd_WXGAVI_75},     /**< 1920x1080 @ 75 frames per second */
		{"VGA_1400x1050_75Hz",	"SXGAPLUS_75",	VideoStd_SXGAPLUS_75},   /**< 1400x1050 @ 75 frames per second */
		{"VGA_1680x1050_75Hz",	"WSXGAPLUS_75", VideoStd_WSXGAPLUS_75},  /**< 1680x1050 @ 75 frames per second */
		{"VGA_1600x900_75Hz",	"WXGAVII_75",	VideoStd_WXGAVII_75},    /**< 1600x900  @ 75 frames per second */
		{"VGA_1920x1200_75Hz",	"WUXGA_75",		VideoStd_WUXGA_75},      /**< 1920x1200 @ 75 frames per second */
		{"VGA_800x600_85Hz",	"SVGA_85",		VideoStd_SVGA_85},       /**< 800x600 @ 85 frames per second */		//ls 2012-10-19
		{"VGA_1024x768_85Hz",	"XGA_85",		VideoStd_XGA_85},       /**< 1024x768 @ 85 frames per second */
		{"VGA_1280x1024_85Hz",	"SXGA_85",		VideoStd_SXGA_85},      /**< 1280x1024 @ 85 frames per second */
		{"VGA_1600x1200_85Hz",	"UXGA_85",		VideoStd_UXGA_85},		/**< 1600x1200 @ 85 frames per second */
		{"VGA_1280x768_85Hz",	"WXGAI_85", 	VideoStd_WXGAI_85},     /**< 1280x768 @ 85 frames per second */
		{"VGA_1440x900_85Hz",	"WXGAII_85", 	VideoStd_WXGAII_85},    /**< 1440x900 @ 85 frames per second */
		{"VGA_1280x800_85Hz",	"WXGAIII_85",	VideoStd_WXGAIII_85},   /**< 1280x800 @ 85 frames per second */
		{"VGA_1280x720_85Hz",	"WXGAIV_85",	VideoStd_WXGAIV_85},    /**< 1280x720 @ 85 frames per second */
		{"VGA_1280x960_85Hz",	"XVGA_85",		VideoStd_XVGA_85},      /**< 1280x960 @ 85 frames per second */
		{"VGA_1385x768_85Hz",	"WXGAV_85",		VideoStd_WXGAV_85},     /**< 1385x768 @ 85 frames per second */
		{"VGA_1920x1080_85Hz",	"WXGAVI_85", 	VideoStd_WXGAVI_85},    /**< 1920x1080 @ 85 frames per second */
		{"VGA_1400x1050_85Hz",	"SXGAPLUS_85",	VideoStd_SXGAPLUS_85},  /**< 1400x1050 @ 85 frames per second */
		{"VGA_1680x1050_85Hz",	"WSXGAPLUS_85", VideoStd_WSXGAPLUS_85}, /**< 1680x1050 @ 85 frames per second */
		{"VGA_1600x900_85Hz",	"WXGAVII_85",	VideoStd_WXGAVII_85},   /**< 1600x900  @ 85 frames per second */
		{"VGA_1920x1200_85Hz",	"WUXGA_85",		VideoStd_WUXGA_85},     /**< 1920x1200 @ 85 frames per second */
		{"", "HDMI_AUTO", VideoStd_HDMI_AUTO},			/**< Automatically select standard (if supported) */
		{"", "HDMI_CUSTOM", VideoStd_HDMI_CUSTOM},
		{"HDMI_800x600_60Hz", "HDMI_SVGA_60",		VideoStd_HDMI_SVGA_60},		/**< 800x600   @ 60 frames per second */
		{"HDMI_1024x768_60Hz", "HDMI_XGA_60",		VideoStd_HDMI_XGA_60},		/**< 1024x768  @ 60 frames per second */
		{"HDMI_1280x1024_60Hz", "HDMI_SXGA_60",		VideoStd_HDMI_SXGA_60},		/**< 1280x1024 @ 60 frames per second */
		{"HDMI_1280x720_60Hz", "HDMI_WXGAIV_60",	VideoStd_HDMI_WXGAIV_60},	/**< 1280x720  @ 60 frames per second */	//60HZ
		{"HDMI_1280x768_60Hz", "HDMI_WXGAI_60",		VideoStd_HDMI_WXGAI_60},	/**< 1280x768  @ 60 frames per second */
		{"HDMI_1280x800_60Hz", "HDMI_WXGAIII_60",	VideoStd_HDMI_WXGAIII_60},	/**< 1280x800  @ 60 frames per second */
		{"HDMI_1280x960_60Hz", "HDMI_XVGA_60",		VideoStd_HDMI_XVGA_60},		/**< 1280x960  @ 60 frames per second */
		{"HDMI_1360x768_60Hz", "HDMI_WXGAV_60",		VideoStd_HDMI_WXGAV_60},	/**< 1360x768  @ 60 frames per second */
		{"HDMI_1440x900_60Hz", "HDMI_WXGAII_60",	VideoStd_HDMI_WXGAII_60},	/**< 1440x900  @ 60 frames per second */
		{"HDMI_1400x1050_60Hz","HDMI_SXGAPLUS_60",	VideoStd_HDMI_SXGAPLUS_60},	/**< 1400x1050 @ 60 frames per second */
		{"HDMI_1680x1050_60Hz","HDMI_WSXGAPLUS_60", VideoStd_HDMI_WSXGAPLUS_60},/**< 1680x1050 @ 60 frames per second */
		{"HDMI_1600x900_60Hz", "HDMI_WXGAVII_60",	VideoStd_HDMI_WXGAVII_60},	/**< 1600x900  @ 60 frames per second */
		{"HDMI_1600x1200_60Hz","HDMI_UXGA_60",		VideoStd_HDMI_UXGA_60},		/**< 1600x1200 @ 60 frames per second */
		{"HDMI_1920x1080_60Hz","HDMI_1080P_60",		VideoStd_HDMI_1080P_60},	/**< 1920x1080 @ 60 frames per second */
		{"HDMI_1920x1200_60Hz","HDMI_WUXGA_60",		VideoStd_HDMI_WUXGA_60},	/**< 1920x1200 @ 60 frames per second */
		{"HDMI_1280x720_75Hz", "HDMI_WXGAIV_75",	VideoStd_HDMI_WXGAIV_75},	/**< 1280x720  @ 75 frames per second */	//75HZ
		{"HDMI_1280x768_75Hz", "HDMI_WXGAI_75",		VideoStd_HDMI_WXGAI_75},	/**< 1280x768  @ 75 frames per second */
		{"HDMI_1280x800_75Hz", "HDMI_WXGAIII_75",	VideoStd_HDMI_WXGAIII_75},	/**< 1280x800  @ 75 frames per second */
		{"HDMI_1440x900_75Hz", "HDMI_WXGAII_75",	VideoStd_HDMI_WXGAII_75},	/**< 1440x900  @ 75 frames per second */
		{"HDMI_1400x1050_75Hz","HDMI_SXGAPLUS_75",	VideoStd_HDMI_SXGAPLUS_75},	/**< 1400x1050 @ 75 frames per second */
		{"HDMI_1600x1200_75Hz","HDMI_UXGA_75",		VideoStd_HDMI_UXGA_75},		/**< 1600x1200 @ 75 frames per second */
		{"HDMI_1280x768_85Hz", "HDMI_WXGAI_85",		VideoStd_HDMI_WXGAI_85},		/**< 1280x768  @ 85 frames per second */	//85HZ
		{"HDMI_1280x800_85Hz", "HDMI_WXGAIII_85",	VideoStd_HDMI_WXGAIII_85},	/**< 1280x800  @ 85 frames per second */
		{"HDMI_1280x960_85Hz", "HDMI_XVGA_85", 		VideoStd_HDMI_XVGA_85},		/**< 1280x960  @ 85 frames per second */
		{"HDMI_1440x900_85Hz", "HDMI_WXGAII_85",	VideoStd_HDMI_WXGAII_85},	/**< 1440x900  @ 85 frames per second */
		{"HDMI_1600x1200_85Hz","HDMI_UXGA_85",		VideoStd_HDMI_UXGA_85},		/**< 1600x1200 @ 85 frames per second */
		//add by ls, 2012-12-28
		{"HDMI_720x480i_60Hz","HDMI_480I_60",	VideoStd_HDMI_480I_60},		/**< 720x480I  @ 60 frames per second */
		{"HDMI_720x480p_60Hz","HDMI_480P_60",	VideoStd_HDMI_480P_60},		/**< 720x480P  @ 60 frames per second */
		{"HDMI_720x576i_50Hz","HDMI_576I_50",	VideoStd_HDMI_576I_50},		/**< 720x576I  @ 50 frames per second */
		{"HDMI_720x576p_50Hz","HDMI_576P_50",	VideoStd_HDMI_576P_50},		/**< 720x576P  @ 50 frames per second */
		{"HDMI_1920x1080i_50Hz","HDMI_1080I_50",VideoStd_HDMI_1080I_50},	/**< 1920x1080I@ 50 frames per second */
		{"HDMI_1920x1080i_60Hz","HDMI_1080I_60",VideoStd_HDMI_1080I_60},	/**< 1920x1080I@ 60 frames per second */
		{"HDMI_1920x1080p_50Hz","HDMI_1080P_50",VideoStd_HDMI_1080P_50},	/**< 1920x1080P@ 50 frames per second */	
	
		/* 2012-12-21, ls, DVI Interface */
		{"", "HDVI_AUTO", VideoStd_HDMI_AUTO},			/**< Automatically select standard (if supported) */
		{"HDVI_800x600_60Hz", "HDVI_SVGA_60",		VideoStd_HDMI_SVGA_60},		/**< 800x600   @ 60 frames per second */
		{"HDVI_1024x768_60Hz", "HDVI_XGA_60",		VideoStd_HDMI_XGA_60},		/**< 1024x768  @ 60 frames per second */
		{"HDVI_1280x1024_60Hz", "HDVI_SXGA_60",		VideoStd_HDMI_SXGA_60},		/**< 1280x1024 @ 60 frames per second */
		{"HDVI_1280x720_60Hz", "HDVI_WXGAIV_60",	VideoStd_HDMI_WXGAIV_60},	/**< 1280x720  @ 60 frames per second */	//60HZ
		{"HDVI_1280x768_60Hz", "HDVI_WXGAI_60",		VideoStd_HDMI_WXGAI_60},	/**< 1280x768  @ 60 frames per second */
		{"HDVI_1280x800_60Hz", "HDVI_WXGAIII_60",	VideoStd_HDMI_WXGAIII_60},	/**< 1280x800  @ 60 frames per second */
		{"HDVI_1280x960_60Hz", "HDVI_XVGA_60",		VideoStd_HDMI_XVGA_60},		/**< 1280x960  @ 60 frames per second */
		{"HDVI_1360x768_60Hz", "HDVI_WXGAV_60",		VideoStd_HDMI_WXGAV_60},	/**< 1360x768  @ 60 frames per second */
		{"HDVI_1440x900_60Hz", "HDVI_WXGAII_60",	VideoStd_HDMI_WXGAII_60},	/**< 1440x900  @ 60 frames per second */
		{"HDVI_1400x1050_60Hz","HDVI_SXGAPLUS_60",	VideoStd_HDMI_SXGAPLUS_60},	/**< 1400x1050 @ 60 frames per second */
		{"HDVI_1680x1050_60Hz","HDVI_WSXGAPLUS_60", VideoStd_HDMI_WSXGAPLUS_60},/**< 1680x1050 @ 60 frames per second */
		{"HDVI_1600x900_60Hz", "HDVI_WXGAVII_60",	VideoStd_HDMI_WXGAVII_60},	/**< 1600x900  @ 60 frames per second */
		{"HDVI_1600x1200_60Hz","HDVI_UXGA_60",		VideoStd_HDMI_UXGA_60},		/**< 1600x1200 @ 60 frames per second */
		{"HDVI_1920x1080_60Hz","HDVI_1080P_60",		VideoStd_HDMI_1080P_60},	/**< 1920x1080 @ 60 frames per second */
		{"HDVI_1920x1200_60Hz","HDVI_WUXGA_60",		VideoStd_HDMI_WUXGA_60},	/**< 1920x1200 @ 60 frames per second */
		{"HDVI_1280x720_75Hz", "HDVI_WXGAIV_75",	VideoStd_HDMI_WXGAIV_75},	/**< 1280x720  @ 75 frames per second */	//75HZ
		{"HDVI_1280x768_75Hz", "HDVI_WXGAI_75",		VideoStd_HDMI_WXGAI_75},	/**< 1280x768  @ 75 frames per second */
		{"HDVI_1280x800_75Hz", "HDVI_WXGAIII_75",	VideoStd_HDMI_WXGAIII_75},	/**< 1280x800  @ 75 frames per second */
		{"HDVI_1440x900_75Hz", "HDVI_WXGAII_75",	VideoStd_HDMI_WXGAII_75},	/**< 1440x900  @ 75 frames per second */
		{"HDVI_1400x1050_75Hz","HDVI_SXGAPLUS_75",	VideoStd_HDMI_SXGAPLUS_75},	/**< 1400x1050 @ 75 frames per second */
		{"HDVI_1600x1200_75Hz","HDVI_UXGA_75",		VideoStd_HDMI_UXGA_75},		/**< 1600x1200 @ 75 frames per second */
		{"HDVI_1280x768_85Hz", "HDVI_WXGAI_85",		VideoStd_HDMI_WXGAI_85},	/**< 1280x768  @ 85 frames per second */	//85HZ
		{"HDVI_1280x800_85Hz", "HDVI_WXGAIII_85",	VideoStd_HDMI_WXGAIII_85},	/**< 1280x800  @ 85 frames per second */
		{"HDVI_1280x960_85Hz", "HDVI_XVGA_85", 		VideoStd_HDMI_XVGA_85},		/**< 1280x960  @ 85 frames per second */
		{"HDVI_1440x900_85Hz", "HDVI_WXGAII_85",	VideoStd_HDMI_WXGAII_85},	/**< 1440x900  @ 85 frames per second */
		{"HDVI_1600x1200_85Hz","HDVI_UXGA_85",		VideoStd_HDMI_UXGA_85},		/**< 1600x1200 @ 85 frames per second */
		//add by ls, 2012-12-28
		{"HDVI_720x480i_60Hz","HDVI_480I_60",	VideoStd_HDMI_480I_60},		/**< 720x480I  @ 60 frames per second */
		{"HDVI_720x480p_60Hz","HDVI_480P_60",	VideoStd_HDMI_480P_60},		/**< 720x480P  @ 60 frames per second */
		{"HDVI_720x576i_50Hz","HDVI_576I_50",	VideoStd_HDMI_576I_50},		/**< 720x576I  @ 50 frames per second */
		{"HDVI_720x576p_50Hz","HDVI_576P_50",	VideoStd_HDMI_576P_50},		/**< 720x576P  @ 50 frames per second */
		{"HDVI_1920x1080i_50Hz","HDVI_1080I_50",VideoStd_HDMI_1080I_50},	/**< 1920x1080I@ 50 frames per second */
		{"HDVI_1920x1080i_60Hz","HDVI_1080I_60",VideoStd_HDMI_1080I_60},	/**< 1920x1080I@ 60 frames per second */	
		{"HDVI_1920x1080p_50Hz","HDVI_1080P_50",VideoStd_HDMI_1080P_50}		/**< 1920x1080P@ 50 frames per second */	
	};

    Int     index;
    Int     c;
	//xudonghai	10-29-09
	char buf[32];
	Int i;
	memset(buf, 0, 32);
	memset(argsp->ipAddr, 0, ADDRSTRLEN);
    for (;;) {
        c = getopt_long(argc, argv, shortOptions, longOptions, &index);

        if (c == -1) {
            break;
        }

        switch (c) {
        case 0:
            break;
        case 'd':
            i = strcspn(optarg + 1, " ");
            memcpy(argsp->ipAddr, optarg + 1, i);
            argsp->ipAddr[i]=0; //linxj 2012-04-09 ending 
            printf("main.c: targetip = %s, len = %d\n", argsp->ipAddr, i);
            break;
        case 'p':
            port = atoi(optarg);
            printf("main.c: port = %d\n", port);
            break;
        case 'f':
            argsp->videoFrameRate = atoi(optarg) * 1000;
            printf("main.c: videoFrameRate = %d fps\n", argsp->videoFrameRate/1000);
            break;
        case 'b':
            argsp->videoBitRate = atoi(optarg) * 1000;
            printf("main.c: videoBitRate = %d kbps\n", argsp->videoBitRate/1000);
            break;
        case 'c':
            argsp->videoCbr = atoi(optarg);
            printf("main.c: videoCbr = %d\n", argsp->videoCbr);
            break;
        case 'i':
            argsp->intraFrameInterval = atoi(optarg);
            printf("main.c: intraFrameInterval = %d\n", argsp->intraFrameInterval);
            break;
        case 'a':
            argsp->audioEnable = atoi(optarg);
            printf("main.c: audioEnable = %d\n", argsp->audioEnable);
            break;
        case 'v':
            argsp->videoEnable = atoi(optarg);
            printf("main.c: videoEnable = %d\n", argsp->videoEnable);
            break;
        case 's':
            argsp->audioSampleRate = atoi(optarg);
            printf("main.c: audioSampleRate = %d Hz\n", argsp->audioSampleRate);
            break;
        case 't':
            argsp->audioBitRate = atoi(optarg) * 1000;
            printf("main.c: audioBitRate = %d kbps\n", argsp->audioBitRate/1000);
            break;
        case 'l':
            argsp->lum = atoi(optarg);
            printf("main.c: videolum = %d\n", argsp->lum);
            break;
        case 'g':
            argsp->volume = atoi(optarg);
            printf("main.c: volume = %d\n", argsp->volume);
            break;
        case 'w':
            argsp->imageWidth = atoi(optarg);
            printf("main.c: imageWidth = %d\n", (int)argsp->imageWidth);
            break;
        case 'h':
            argsp->imageHeight = atoi(optarg);
            printf("main.c: imageHeight = %d\n", (int)argsp->imageHeight);
            break;
        case 'r':
            i = strcspn(optarg + 1, " ");
            if(i>sizeof(buf))
            {
                printf("solution error,i=%d \n",i);
            }
            memcpy(buf, optarg + 1, i);
#if 0
            if (strcmp(buf, "SVGA") == 0) {
                argsp->videoStd = VideoStd_SVGA;
                argsp->videoStdString = "SVGA 60Hz";
            } else if (strcmp(buf, "XGA") == 0) {
                argsp->videoStd = VideoStd_XGA;
                argsp->videoStdString = "XGA 60Hz";
            } else if (strcmp(buf, "SXGA") == 0) {
                argsp->videoStd = VideoStd_SXGA;
                argsp->videoStdString = "SXGA 60Hz";
            } else if (strcmp(buf, "WXGAI") == 0) {
                argsp->videoStd = VideoStd_WXGAI;
                argsp->videoStdString = "WXGAI 60Hz";
            } else if (strcmp(buf, "WXGAII") == 0) {
                argsp->videoStd = VideoStd_WXGAII;
                argsp->videoStdString = "WXGAII 60Hz";
            } else if (strcmp(buf, "WXGAIII") == 0) {
                argsp->videoStd = VideoStd_WXGAIII;
                argsp->videoStdString = "WXGAIII 60Hz";
            } else if (strcmp(buf, "WXGAIV") == 0) {
                argsp->videoStd = VideoStd_WXGAIV;
                argsp->videoStdString = "WXGAIV 60Hz";
            } else if (strcmp(buf, "WXGAV") == 0) {
                argsp->videoStd = VideoStd_WXGAV;
                argsp->videoStdString = "WXGAV 60Hz";
            } else if (strcmp(buf, "WXGAVI") == 0) {
                argsp->videoStd = VideoStd_WXGAVI;
                argsp->videoStdString = "WXGAVI 60Hz";
            } else if (strcmp(buf, "WXGAVII") == 0) {
                argsp->videoStd = VideoStd_WXGAVII;
                argsp->videoStdString = "WXGAVII 60Hz";
            } else if (strcmp(buf, "XVGA") == 0) {
                argsp->videoStd = VideoStd_XVGA;
                argsp->videoStdString = "XVGA 60Hz";
            } else if (strcmp(buf, "SXGAPLUS") == 0) {
                argsp->videoStd = VideoStd_SXGAPLUS;
                argsp->videoStdString = "SXGAPLUS 60Hz";
            } else if (strcmp(buf, "WSXGAPLUS") == 0) {
                argsp->videoStd = VideoStd_WSXGAPLUS;
                argsp->videoStdString = "WSXGAPLUS 60Hz";
            } else if (strcmp(buf, "UXGA") == 0) {
                argsp->videoStd = VideoStd_UXGA;
                argsp->videoStdString = "UXGA 60Hz";
            } else if (strcmp(buf, "1080P60") == 0) {
                argsp->videoStd = VideoStd_1080P_60;
                argsp->videoStdString = "1080P 60Hz";
            } else if (strcmp(buf, "1080P50") == 0) {
                argsp->videoStd = VideoStd_1080P_50;
                argsp->videoStdString = "1080P 50Hz";
            } else if (strcmp(buf, "1080I50") == 0) {
                argsp->videoStd = VideoStd_1080I_25;
                argsp->videoStdString = "1080I 50Hz";
            } else if (strcmp(buf, "1080I60") == 0) {
                argsp->videoStd = VideoStd_1080I_30;
                argsp->videoStdString = "1080I 60Hz";
            } else if (strcmp(buf, "720P50") == 0) {
                argsp->videoStd = VideoStd_720P_50;
                argsp->videoStdString = "720P 50Hz";
            } else if (strcmp(buf, "720P60") == 0) {
                argsp->videoStd = VideoStd_720P_60;
                argsp->videoStdString = "720P 60Hz";
            } else if (strcmp(buf, "576P") == 0) {
                argsp->videoStd = VideoStd_576P;
                argsp->videoStdString = "576P";
            } else if (strcmp(buf, "480P") == 0) {
                argsp->videoStd = VideoStd_480P;
                argsp->videoStdString = "480P";
            } else if (strcmp(buf, "1080P30") == 0) {//linxj 2011-12-21
                argsp->videoStd = VideoStd_1080P_30;
                argsp->videoStdString = "1080P 30Hz";
            } else if (strcmp(buf, "1080P25") == 0) {
                argsp->videoStd = VideoStd_1080P_25;
                argsp->videoStdString = "1080P 25Hz";
            } else if (strcmp(buf, "HDMI1080P") == 0) {
                argsp->videoStd = VideoStd_HDMI1080P;
                argsp->videoStdString = "HDMI 1080P";
            } else if (strcmp(buf, "HDMI720P") == 0) {
                argsp->videoStd = VideoStd_HDMI720P;
                argsp->videoStdString = "HDMI 720P";
            } else {
                argsp->videoStd = VideoStd_AUTO;
                argsp->videoStdString = "AUTO DETECT";
            }
#else
//			printf("main.c: in parseArgs buf=%s\n", buf);
			int j = 0;
			for (j=0; j<sizeof(stdtab)/sizeof(CaptureStd); j++) {
				if ((strncmp(buf, stdtab[j].stdstr, strlen(buf))==0)
					||(strncmp(buf, stdtab[j].stdaliars, strlen(buf))==0)) {
					argsp->videoStd = stdtab[j].stdno;
					argsp->videoStdString = stdtab[j].stdaliars;
					break;
				}
			}
			if (j == sizeof(stdtab)/sizeof(CaptureStd)) {
				if (strncmp(buf, "HDMI", 4) == 0) {
					argsp->videoStd = VideoStd_HDMI_AUTO;
					argsp->videoStdString = "HDMI_AUTO DETECT";
				} else if (strncmp(buf, "HDVI", 4) == 0) {
					argsp->videoStd = VideoStd_HDMI_AUTO;
					argsp->videoStdString = "HDVI_AUTO DETECT";
				} else {
					argsp->videoStd = VideoStd_AUTO;
					argsp->videoStdString = "AUTO DETECT";
				}

			}
#endif
            printf("main.c: resolution = %s, len: %d\n", argsp->videoStdString, i);
            break;

        case 'n':
            break;
        default:
            usage();
            //exit(EXIT_FAILURE);
            break;
        }
    }

    argsp->videoEncoder =
           getCodec(".264", engine->videoEncoders);
    argsp->videoFile = "test.264";
    argsp->audioEncoder =
           getCodec(".aac", engine->audioEncoders);
	argsp->audioFile = "test.aac";
}

/******************************************************************************
 * main
 ******************************************************************************/
Int main(Int argc, Char *argv[]) {
//	printf("slave do nothing\n");
//	DBG_STOP(1);
    Args                args                = DEFAULT_ARGS;
    Uns                 initMask            = 0;
    Int                 status              = EXIT_SUCCESS;
    Pause_Attrs         pAttrs              = Pause_Attrs_DEFAULT;
    Rendezvous_Attrs    rzvAttrs            = Rendezvous_Attrs_DEFAULT;
    Fifo_Attrs          fAttrs              = Fifo_Attrs_DEFAULT;
    Rendezvous_Handle   hRendezvousCapStd   = NULL;
    Rendezvous_Handle   hRendezvousInit     = NULL;
    Rendezvous_Handle   hRendezvousWriter   = NULL;
    Rendezvous_Handle   hRendezvousCleanup  = NULL;
    Pause_Handle        hPauseProcess       = NULL;
    struct sched_param  schedParam;
    pthread_t           demandThread;
    pthread_t           captureThread;
    pthread_t           writerThread;
    pthread_t           videoThread;
    pthread_t           dynamicThread;  //linxj 2012-03-13
    //pthread_t           speechThread;
    pthread_t           audioThread;
    DemandEnv           demandEnv;
    DemandEnv           dynamicEnv;//libo 2012-03-24
    CaptureEnv          captureEnv;
    WriterEnv           writerEnv;
    VideoEnv            videoEnv;
    AudioEnv           audioEnv;
    Int                 numThreads;
    pthread_attr_t      attr;
    Void               *ret;
	int 				i;
	int					retval;
	
	VisTS_params tsparams;		//steven 12-23-2009
	VisTS_config tsconfig;		//steven 12-23-2009
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

	retval = VisTS_init(&tsparams);
	if(retval<=0)
	{
		printf("init TS failed!\n");
		cleanup(EXIT_FAILURE);
	}

//	printf("sizeof(Uns) = %d\n", sizeof(Uns));
	printf("\n\n>>>>>>>>>>>>>>>>>>>>>>>>> hdmi_enc %d.%d <<<<<<<<<<<<<<<<<<<<<<<<<<\n", SLAVEVERSION_MAJOR, SLAVEVERSION_MINOR);
//	sleep(5);

    /* Zero out the thread environments */
    Dmai_clear(captureEnv);
    Dmai_clear(writerEnv);
    Dmai_clear(videoEnv);
//    Dmai_clear(speechEnv);
    Dmai_clear(audioEnv);
    Dmai_clear(demandEnv);
    Dmai_clear(dynamicEnv);

    /* Parse the arguments given to the app and set the app environment */
    parseArgs(argc, argv, &args);

    /* Initialize the mutex which protects the global data */
    pthread_mutex_init(&gbl.mutex, NULL);
    pthread_mutex_init(&conn_client.mutex, NULL);

    /* Set the priority of this whole process to max (requires root) */
    setpriority(PRIO_PROCESS, 0, -20);

    /* Initialize Codec Engine runtime */
    CERuntime_init();

    /* Initialize signal handler for SIGINT */
    signal(SIGINT, signalHandler);
    
    /* Initialize Davinci Multimedia Application Interface */
    Dmai_init();

    initMask |= LOGSINITIALIZED;

    /* Create the Pause object */
    hPauseProcess = Pause_create(&pAttrs);

    if (hPauseProcess == NULL) {
        ERR("Failed to create Pause object\n");
        cleanup(EXIT_FAILURE);
    }

    conn_client.num_connect = 0;
    for(i=0; i<MAX_CONNECTION; i++)
        conn_client.flag[i] = 0;
	//memcpy(&vis_global.ipAddr, "192.168.18.255", strlen("192.168.18.255"));
    //vis_global.port = 1234;
	memcpy(&vis_global.ipAddr, args.ipAddr, strlen(args.ipAddr));
	vis_global.port = port;
    vis_global.socket = socketInit(&vis_global);
    if (vis_global.socket < 0) {
		ERR("Failed to create socket fd.\n");
        exit(-1);
    }
    sem_init(&vis_global.sem_protect, 0, 1);

    /* Determine the number of threads needing synchronization */
    numThreads = 1;
    if (args.videoFile) {
        numThreads += 3;
    }
    if (args.audioEnable) {
        numThreads += 1;
    }
#ifdef USE_DEMAND
    numThreads += 1; //for demand thread
#endif
#ifdef USE_DYNAMICTHREAD
    numThreads += 1; //for dynamic thread
#endif

    /* Create the objects which synchronizes the thread init and cleanup */
    hRendezvousCapStd  = Rendezvous_create(2, &rzvAttrs);
    hRendezvousInit = Rendezvous_create(numThreads, &rzvAttrs);
    hRendezvousCleanup = Rendezvous_create(numThreads, &rzvAttrs);
    hRendezvousWriter = Rendezvous_create(2, &rzvAttrs);
	
    if (hRendezvousInit == NULL || 	hRendezvousCapStd  == NULL || 
        hRendezvousCleanup == NULL || hRendezvousWriter == NULL) {
        ERR("Failed to create Rendezvous objects\n");
        cleanup(EXIT_FAILURE);
    }

    /* Initialize the thread attributes */
    if (pthread_attr_init(&attr)) {
        ERR("Failed to initialize thread attrs\n");
        cleanup(EXIT_FAILURE);
    }

    /* Force the thread to use custom scheduling attributes */
    if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) {
        ERR("Failed to set schedule inheritance attribute\n");
        cleanup(EXIT_FAILURE);
    }

    /* Set the thread to be fifo real time scheduled */
    if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) {
        ERR("Failed to set FIFO scheduling policy\n");
        cleanup(EXIT_FAILURE);
    }
    
    /* Create the audio thread if a file name is supplied */
    if (args.audioEnable) {
        /* Set the thread priority */
        schedParam.sched_priority = AUDIO_THREAD_PRIORITY;
        if (pthread_attr_setschedparam(&attr, &schedParam)) {
            ERR("Failed to set scheduler parameters\n");
            cleanup(EXIT_FAILURE);
        }

        /* Create the audio thread */

        audioEnv.soundSource = 0;
        //if(args.videoStd == VideoStd_720P_60||args.videoStd ==VideoStd_1080P_60)
        if(strlen(args.videoStdString)>4
				&& args.videoStdString[0]=='H'
				&& args.videoStdString[1]=='D'
				&& args.videoStdString[2]=='M'
				&& args.videoStdString[3]=='I')
            audioEnv.soundSource =1;

        audioEnv.hRendezvousInit    = hRendezvousInit;
        audioEnv.hRendezvousCleanup = hRendezvousCleanup;
        audioEnv.hPauseProcess      = hPauseProcess;
        audioEnv.engineName         = engine->engineName;
        audioEnv.audioEncoder       = args.audioEncoder->codecName;
        audioEnv.params             = args.audioEncoder->params;
        audioEnv.dynParams          = args.audioEncoder->dynParams;
        audioEnv.audioFile          = args.audioFile;
        audioEnv.soundInput         = args.soundInput;
        audioEnv.soundBitRate       = args.audioBitRate;
        audioEnv.sampleRate         = args.audioSampleRate;
        audioEnv.volume             = args.volume;
        audioEnv.tsparams			= tsparams;	//steven 12-23-2009
        if (pthread_create(&audioThread, &attr, audioThrFxn, &audioEnv)) {
            ERR("Failed to create audio thread\n");
            cleanup(EXIT_FAILURE);
        }

        initMask |= AUDIOTHREADCREATED;
    }
    /* Create the demand thread, sp */
    /* Set the thread priority */
    schedParam.sched_priority = DEMAND_THREAD_PRIORITY;
    if (pthread_attr_setschedparam(&attr, &schedParam))
    {
        ERR("Failed to set scheduler parameters\n");
        cleanup(EXIT_FAILURE);
    }
#ifdef USE_DEMAND
	printf("main.c: before create demand thread\n");
    demandEnv.hRendezvousInit    = hRendezvousInit;
    demandEnv.hRendezvousCleanup = hRendezvousCleanup;
#if 1
    if (pthread_create(&demandThread, &attr, demandThrFxn, &demandEnv)) {
        ERR("Failed to create demand thread\n");
        cleanup(EXIT_FAILURE);
    }
#endif
    initMask |= DEMANDTHREADCREATED;
#endif

#ifdef USE_DYNAMICTHREAD
        dynamicEnv.hRendezvousInit    = hRendezvousInit;
        dynamicEnv.hRendezvousCleanup = hRendezvousCleanup;
        /* Set the dynamic thread priority */
        schedParam.sched_priority = DYNAMIC_THREAD_PRIORITY;
        if (pthread_attr_setschedparam(&attr, &schedParam)) {
            ERR("Failed to set scheduler parameters\n");
            cleanup(EXIT_FAILURE);
        }
        if (pthread_create(&dynamicThread, &attr, dynamicThrFxn, &dynamicEnv)) {
            ERR("Failed to create dynamic thread\n");
            cleanup(EXIT_FAILURE);
        }

        initMask |= DYNAMICTHREADCREATED;
#endif
    //if (args.videoEnable) {
        /* Create the capture fifos */
        captureEnv.hInFifo = Fifo_create(&fAttrs);
        captureEnv.hOutFifo = Fifo_create(&fAttrs);

        if (captureEnv.hInFifo == NULL || captureEnv.hOutFifo == NULL) {
            ERR("Failed to open capture fifos\n");
            cleanup(EXIT_FAILURE);
        }

        /* Set the capture thread priority */
        schedParam.sched_priority = CAPTURE_THREAD_PRIORITY;
        if (pthread_attr_setschedparam(&attr, &schedParam)) {
            ERR("Failed to set scheduler parameters\n");
            cleanup(EXIT_FAILURE);
        }

        /* Create the capture thread */
        captureEnv.hRendezvousInit    = hRendezvousInit;
        captureEnv.hRendezvousCapStd  = hRendezvousCapStd;
        captureEnv.hRendezvousCleanup = hRendezvousCleanup;
        captureEnv.hPauseProcess      = hPauseProcess;
        captureEnv.videoStd           = args.videoStd;
        captureEnv.videoInput         = args.videoInput;
        captureEnv.imageWidth         = args.imageWidth;
        captureEnv.imageHeight        = args.imageHeight;
        captureEnv.videoEnable        = args.videoEnable;
        captureEnv.videoFrameRate     = args.videoFrameRate;
        captureEnv.videoLum           = args.lum;
#if 1
        if (pthread_create(&captureThread, &attr, captureThrFxn, &captureEnv)) {
            ERR("Failed to create capture thread\n");
            cleanup(EXIT_FAILURE);
        }

        initMask |= CAPTURETHREADCREATED;
#endif

        /*
         * Once the capture thread has detected the video standard, make it
         * available to other threads. The capture thread will set the
         * resolution of the buffer to encode in the environment (derived
         * from the video standard if the user hasn't passed a resolution).
         */
        Rendezvous_meet(hRendezvousCapStd);

        printf("main.c:before create write thread\n");
        /* Create the writer fifos */
        writerEnv.hInFifo = Fifo_create(&fAttrs);
        writerEnv.hOutFifo = Fifo_create(&fAttrs);

        if (writerEnv.hInFifo == NULL || writerEnv.hOutFifo == NULL) {
            ERR("Failed to open write fifos\n");
            cleanup(EXIT_FAILURE);
        }

        /* Set the video thread priority */
        schedParam.sched_priority = VIDEO_THREAD_PRIORITY;
        if (pthread_attr_setschedparam(&attr, &schedParam)) {
            ERR("Failed to set scheduler parameters\n");
            cleanup(EXIT_FAILURE);
        }

        /* Create the video thread */
        videoEnv.hRendezvousInit    = hRendezvousInit;
        videoEnv.hRendezvousCleanup = hRendezvousCleanup;
        videoEnv.hRendezvousWriter  = hRendezvousWriter;
        videoEnv.hPauseProcess      = hPauseProcess;
        videoEnv.hCaptureOutFifo    = captureEnv.hOutFifo;
        videoEnv.hCaptureInFifo     = captureEnv.hInFifo;
        videoEnv.hWriterOutFifo     = writerEnv.hOutFifo;
        videoEnv.hWriterInFifo      = writerEnv.hInFifo;
        videoEnv.videoEncoder       = args.videoEncoder->codecName;
        videoEnv.params             = args.videoEncoder->params;
        videoEnv.dynParams          = args.videoEncoder->dynParams;
        videoEnv.videoBitRate       = args.videoBitRate;
        videoEnv.videoCbr           = args.videoCbr;
        videoEnv.intraFrameInterval = args.intraFrameInterval;
        videoEnv.imageWidth         = captureEnv.imageWidth;
        videoEnv.imageHeight        = captureEnv.imageHeight;
        videoEnv.engineName         = engine->engineName;
        videoEnv.videoFrameRate     = args.videoFrameRate;
        videoEnv.videoEnable        = args.videoEnable;
#if 1
        if (pthread_create(&videoThread, &attr, videoThrFxn, &videoEnv)) {
            ERR("Failed to create video thread\n");
            cleanup(EXIT_FAILURE);
        }

        initMask |= VIDEOTHREADCREATED;
#endif

        /*
         * Wait for the codec to be created in the video thread before
         * launching the writer thread (otherwise we don't know which size
         * of buffers to use).
         */
        Rendezvous_meet(hRendezvousWriter);

        /* Set the writer thread priority */
        schedParam.sched_priority = WRITER_THREAD_PRIORITY;
        if (pthread_attr_setschedparam(&attr, &schedParam)) {
            ERR("Failed to set scheduler parameters\n");
            cleanup(EXIT_FAILURE);
        }

        /* Create the writer thread */
        writerEnv.hRendezvousInit    = hRendezvousInit;
        writerEnv.hRendezvousCleanup = hRendezvousCleanup;
        writerEnv.hPauseProcess      = hPauseProcess;
        writerEnv.videoFile          = args.videoFile;
        writerEnv.outBufSize         = videoEnv.outBufSize;
        writerEnv.tsparams			 = tsparams;	//steven 12-23-2009
        writerEnv.videoEnable        = args.videoEnable;
#ifdef SLEEP_CUSTOMER_	//macro in writer.h
		writerEnv.videoFrameRate	 = args.videoFrameRate;
#endif
#if 1
        if (pthread_create(&writerThread, &attr, writerThrFxn, &writerEnv)) {
            ERR("Failed to create writer thread\n");
            cleanup(EXIT_FAILURE);
        }

        initMask |= WRITERTHREADCREATED;
#endif
    //}


//#ifdef USE_DYNAMICTHREAD
#if 0
        dynamicEnv.hRendezvousInit    = hRendezvousInit;
        dynamicEnv.hRendezvousCleanup = hRendezvousCleanup;
        /* Set the dynamic thread priority */
        schedParam.sched_priority = DYNAMIC_THREAD_PRIORITY;
        if (pthread_attr_setschedparam(&attr, &schedParam)) {
            ERR("Failed to set scheduler parameters\n");
            cleanup(EXIT_FAILURE);
        }
        if (pthread_create(&dynamicThread, &attr, dynamicThrFxn, &dynamicEnv)) {
            ERR("Failed to create dynamic thread\n");
            cleanup(EXIT_FAILURE);
        }

        initMask |= DYNAMICTHREADCREATED;
#endif

#if 0
    /* Main thread becomes the control thread */
    ctrlEnv.hRendezvousInit    = hRendezvousInit;
    ctrlEnv.hRendezvousCleanup = hRendezvousCleanup;
    ctrlEnv.hPauseProcess      = hPauseProcess;
    ctrlEnv.keyboard           = args.keyboard;
    ctrlEnv.time               = args.time;
    ctrlEnv.engineName         = engine->engineName;

    ret = ctrlThrFxn(&ctrlEnv);
    if (ret == THREAD_FAILURE) {
        status = EXIT_FAILURE;
    }
#else
    /* Signal that initialization is done and wait for other threads */
    printf("main.c: before meet\n");
    Rendezvous_meet(hRendezvousInit);
    printf("main.c: after meet quit_flag = %d \n", gblGetQuit());

    while (!gblGetQuit()) {
        sleep(1);
    }
#endif

cleanup:
    printf("main.c: cleanup\n");
    gblSetQuit();
    /* Make sure the other threads aren't waiting for init to complete */
    if (hRendezvousCapStd) Rendezvous_force(hRendezvousCapStd);
    if (hRendezvousWriter) Rendezvous_force(hRendezvousWriter);
    if (hRendezvousInit) Rendezvous_force(hRendezvousInit);
    if (hPauseProcess) Pause_off(hPauseProcess);

    /* Meet up with other threads before cleaning up */
    Rendezvous_meet(hRendezvousCleanup);

    /* Wait until the other threads terminate */
#if 0
    if (initMask & SPEECHTHREADCREATED) {
        if (pthread_join(speechThread, &ret) == 0) {
            if (ret == THREAD_FAILURE) {
				printf("main.c:speech thread return EXIT_FAILURE\n");
                status = EXIT_FAILURE;
            }
        }
    }
#endif

#ifdef USE_DYNAMICTHREAD
    if (initMask & DYNAMICTHREADCREATED) {
        if (pthread_join(dynamicThread, &ret) == 0) {
            if (ret == THREAD_FAILURE) {
				printf("main.c: dynamic thread return EXIT_FAILURE\n");
                status = EXIT_FAILURE;
            }
        }
    }
#endif

    if (initMask & VIDEOTHREADCREATED) {
        if (pthread_join(videoThread, &ret) == 0) {
            if (ret == THREAD_FAILURE) {
				printf("main.c: video thread return EXIT_FAILURE\n");
                status = EXIT_FAILURE;
            }
        }
    }

    if (initMask & WRITERTHREADCREATED) {
        if (pthread_join(writerThread, &ret) == 0) {
            if (ret == THREAD_FAILURE) {
				printf("main.c: writer thread return EXIT_FAILURE\n");
                status = EXIT_FAILURE;
            }
        }
    }

    if (writerEnv.hOutFifo) {
        Fifo_delete(writerEnv.hOutFifo);
    }

    if (writerEnv.hInFifo) {
        Fifo_delete(writerEnv.hInFifo);
    }

    if (initMask & CAPTURETHREADCREATED) {
        if (pthread_join(captureThread, &ret) == 0) {
            if (ret == THREAD_FAILURE) {
				printf("main.c: capture thread return EXIT_FAILURE\n");
                status = EXIT_FAILURE;
            }
        }
    }

    if (captureEnv.hOutFifo) {
        Fifo_delete(captureEnv.hOutFifo);
    }

    if (captureEnv.hInFifo) {
        Fifo_delete(captureEnv.hInFifo);
    }

    if (hRendezvousCleanup) {
        Rendezvous_delete(hRendezvousCleanup);
    }

    if (hRendezvousInit) {
        Rendezvous_delete(hRendezvousInit);
    }

    if (hPauseProcess) {
        Pause_delete(hPauseProcess);
    }

    system("sync");
    system("echo 3 > /proc/sys/vm/drop_caches");

    pthread_mutex_destroy(&gbl.mutex);
    pthread_mutex_destroy(&conn_client.mutex);

    printf("!!![exit]:args.interface = %d \n",args.interface);
    if (args.interface) {
        /* Launch the demo selection interface when exiting */
        if (execl("./interface", "interface", "-l 3", (char *) NULL) == -1) {
            status = EXIT_FAILURE;
        }
    }

    exit(status);
}
