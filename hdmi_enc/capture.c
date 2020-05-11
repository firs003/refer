/*
 * capture.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#include <xdc/std.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <arpa/inet.h>
#include <time.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>

#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/BufTab.h>
#include <ti/sdo/dmai/Capture.h>
#include <ti/sdo/dmai/Display.h>
#include <ti/sdo/dmai/VideoStd.h>
#include <ti/sdo/dmai/Framecopy.h>
#include <ti/sdo/dmai/BufferGfx.h>
#include <ti/sdo/dmai/Rendezvous.h>

#include "capture.h"
#include "../demo.h"
#include "visconfig.h"
#include "demand.h"
#include "osd.h"

#define	OSDTEST
#define	AUTO_RESOLUTION_DETECT
//#define	CAPTURE_GET_NONBLOCK
//#define CUSTOM_HILAND//linxj 2011-12-27

/* Buffering for the capture driver */
#define NUM_CAPTURE_BUFS         3

/* Number of buffers in the pipe to the capture thread */
/* Note: It needs to match capture.c pipe size */
#define VIDEO_PIPE_SIZE          3

#define NUM_BUFS (NUM_CAPTURE_BUFS+VIDEO_PIPE_SIZE)

#define GPIO_DEVICE "/dev/dm365_gpio"
#define CAPTUREDIM_FILE "/mnt/apps/configFile/capturedim.cfg"
#define CUSTOMSTD_FILE "/mnt/apps/configFile/customstd.cfg"
#define GIO_SET_INPUT 3
#define GIO_OUTPUT 4
#define GIO_GET 2
#define USEC 1000000

#define	MAXNUM_RESOLUTION  32
typedef struct capturedim{
	int size;
	int num;
	struct {
	int	width;
	int height;
	int hori_start;
	int vert_start;
	}_setting[MAXNUM_RESOLUTION];
}capturedim;

#define REG_MODIFY_FILE "/dev/davinci_reg_modify"

#define	HORI_START 0x01c70418
#define	VERT_START 0x01c70410

#define CAPTURE_SYNC() \
{\
	long sleeptime;\
	struct timeval currtime;\
	gettimeofday(&currtime, NULL);\
	static struct timeval desttime = {0, 0};\
	sleeptime = (long)((desttime.tv_sec-currtime.tv_sec)*USEC + (desttime.tv_usec-currtime.tv_usec));\
	if (sleeptime > 0) usleep(sleeptime);\
	desttime.tv_usec += destval;\
	if (desttime.tv_usec >= USEC) {\
		desttime.tv_sec += 1;\
		desttime.tv_usec -= USEC;\
	}\
}
#if 1
#define MAX_SEC_CAPTURE_UNSYNC	2	//2 seconds
#define MAX_USEC_CAPTURE_UNSYNC	(MAX_SEC_CAPTURE_UNSYNC*USEC)
inline int capture_sync(struct timeval *desttime, long long duaration) {
	long sleeptime;
	struct timeval currtime;
	gettimeofday(&currtime, NULL);
	if (desttime->tv_sec==0 && desttime->tv_usec==0) *desttime=currtime;
//	sleeptime = (long)((desttime->tv_sec-currtime.tv_sec)*USEC + (desttime->tv_usec-currtime.tv_usec));
	sleeptime = ((long)desttime->tv_sec-(long)currtime.tv_sec)*USEC + ((long)desttime->tv_usec-(long)currtime.tv_usec);
	if (sleeptime > MAX_USEC_CAPTURE_UNSYNC) {
		ERR("sleep time is too long, sleeptime=%ldms\n", sleeptime/1000);
		sleeptime = MAX_USEC_CAPTURE_UNSYNC;
	}
	if (sleeptime > 0 && sleeptime < MAX_USEC_CAPTURE_UNSYNC) usleep(sleeptime);
	do {
		desttime->tv_usec += duaration;
		if (desttime->tv_usec >= USEC) {
			desttime->tv_sec += 1;
			desttime->tv_usec -= USEC;
		}
	} while(desttime->tv_sec < currtime.tv_sec || (desttime->tv_sec == currtime.tv_sec && desttime->tv_usec < currtime.tv_usec));
	return 0;
}
#endif

/***********************************************************************************************
 * static functions 
 ***********************************************************************************************/
static int set_customstd(Capture_Handle hCapture);
static int set_capdim(Capture_Handle hCapture);
static int save_capdim(Capture_Handle hCapture, struct capture_video_windows *captureWin);
static int adv7441_vga1080p60_ripple_debug(Capture_Handle hCapture);


typedef struct adv7441cfg_triad {
    char i2c_addr;
    char i2c_reg;
    char i2c_data;
} adv7441cfg_triad_t;

adv7441cfg_triad_t adv7441cfg_VGA[]={
    {0x42, 0x03, 0x0C}, //; Disable TOD
    {0x42, 0x04, 0x4F}, //; Enable TIM_OE
    {0x42, 0x05, 0x02}, //; Prim_Mode =010b for GR
    {0x42, 0x06, 0x0C}, //; VID_STD=1100b for 1024x768 _@ 60
    {0x42, 0x6B, 0xC3}, //; Field Out , =3 
    {0x42, 0x1D, 0x40}, //; Disable TRI_LLC
    {0x42, 0x3C, 0xA8}, //; SOG Sync level for atenuated sync, PLL Qpump to default
    {0x42, 0x47, 0x0A}, //; Enable Automatic PLL_Qpump and VCO Range
    {0x42, 0x68, 0xF0}, //; Auto CSC , YUV Out
    {0x42, 0x7B, 0x1D}, //; Turn off EAV and SAV codes
    {0x42, 0x7C, 0x00}, //; HS VS pority...
    {0x42, 0x7E, 0x47}, //; HS start...
    {0x42, 0x7F, 0xff}, //; VS start...
    {0x42, 0xBA, 0xA0}, //; Enable HDMI and Analog in
    {0x42, 0xF4, 0x26}, //; Max Drive Strength //0x15
    //{0x42, 0xBF, 0x13}, //; ..
};

#if 0
static int get_div(VideoStd_Type videoStd, Int frameRate)
{
    int div = 1;
    int val = frameRate / 1000;

    if (videoStd == VideoStd_SVGA) {
        if (val >= 35 && val <= 60) //43
            div = 2;
        else if (val >= 25 && val < 35) //29
            div = 2;
        else if (val >= 15 && val < 25) //19
            div = 3;
        else if (val >= 12 && val < 15) //14
            div = 4;
        else if (val >= 8 && val < 12) //11
            div = 5;
        else if (val >= 1 && val < 8) //6
            div = 9;
        else div = 1;
    } else if (videoStd == VideoStd_XGA) {
        if (val >= 30 && val <= 60) //32
            div = 1;
        else if (val >= 25 && val < 30) //29
            div = 2;
        else if (val >= 15 && val < 25) //19
            div = 3;
        else if (val >= 13 && val < 15) //14
            div = 4;
        else if (val >= 9 && val < 13) //11
            div = 5;
        else if (val >= 1 && val < 9) //7
            div = 8;
        else div = 1;
    } else if (videoStd == VideoStd_SXGA) {
        if (val >= 17 && val < 60) //20
            div = 1;
        else if (val >= 13 && val < 17) //14
            div = 4;
        else if (val >= 9 && val < 13) //11
            div = 5;
        else if (val >= 1 && val < 9) //6
            div = 9;
        else div = 1;
    }

    return div;
}
#endif

/******************************************************************************
 * captureThrFxn
 ******************************************************************************/
Capture_Handle        hCapture = NULL;  //linxj 2012-03-13
long long destval;	//ls 2013-06-07
Void *captureThrFxn(Void *arg)
{
    CaptureEnv           *envp     = (CaptureEnv *) arg;
    Void                 *status   = THREAD_SUCCESS;
    Capture_Attrs         cAttrs   = Capture_Attrs_DM365_DEFAULT;
    Framecopy_Attrs       fcAttrs  = Framecopy_Attrs_DEFAULT;
    BufferGfx_Attrs       gfxAttrs = BufferGfx_Attrs_DEFAULT;    
    Framecopy_Handle      hFcDisp  = NULL;
    Framecopy_Handle      hFcEnc   = NULL;
    BufTab_Handle         hBufTab  = NULL;    
    Buffer_Handle         hDstBuf, hCapBuf, hBuf;
    BufferGfx_Dimensions  capDim;
    VideoStd_Type         videoStd;
    Int32                 width, height, bufSize;
    Int                   fifoRet;
    ColorSpace_Type       colorSpace =  ColorSpace_YUV420PSEMI;
    Int                   bufIdx;
    Bool                  frameCopy = TRUE;

    int i=0, ret=0;
	struct timeval desttime = {0, 0};
    int gpio_fd;
    unsigned int buf[2] = {0};
	int retval = 0;

#ifdef OSDTEST
	int j;
	struct timeval tvosd = {0, 0};
	struct tm *tmosd = NULL;
	char strtime[64] = {0, };
	unsigned short str[64] = {0, };
//	Vis_OSD_Handle hVisOSD = NULL;
//	Vis_OSD_Attrs osdAttrs = Vis_OSD_Attrs_DEFAULT;
	Vis_OSD_Handle hVisOSD_array[MAX_OSD_NUM] = {NULL, };
	Vis_OSD_Attrs osdAttrs_array[MAX_OSD_NUM];
	Fifo_M2S_Msg fifomsg;
	int fifofd_m2s_capture = open(FIFO_M2S_CAPTURE, O_RDONLY|O_NONBLOCK);
	if (fifofd_m2s_capture == -1) {
		perror("open fifo m2s capture failed");
		cleanup(THREAD_FAILURE);
	}
	memset(&fifomsg, 0, sizeof(fifomsg));
	memset(osdAttrs_array, 0, sizeof(osdAttrs_array));
#endif
	destval = (long long)(USEC*(1000.0/envp->videoFrameRate));
#ifdef AUTO_RESOLUTION_DETECT
	VideoStd_Type videoStd_tmp;
	VideoStd_Type videoStd_old = envp->videoStd;
#endif

#ifdef  CUSTOM_HILAND
    destval = (long long)(envp->videoFrameRate*(USEC/1000.0));
#endif

#if 0
    FILE *outFile = NULL;
    int j = 0;
    outFile = fopen("capture.yuv", "w");
    if (outFile == NULL) {
        ERR("Failed to open %s for writing\n", "capture.yuv");
        cleanup(THREAD_FAILURE);
    }
#endif
	DBG("destval=%lld\n", destval);

	gpio_fd = open(GPIO_DEVICE, O_RDWR);
    ret = Capture_setasHDMI(1); //linxj 2012-05-28
    printf("set as E300 ret=%d!! \n",ret); // support HDMI and VGA


    /* Create capture device driver instance */
    cAttrs.numBufs = NUM_CAPTURE_BUFS;
    cAttrs.videoInput = envp->videoInput;
    cAttrs.videoStd = envp->videoStd;

#if 1
	if (envp->videoLum == 75) {
		switch (cAttrs.videoStd) {
			case VideoStd_SXGA :
				cAttrs.videoStd = VideoStd_SXGA_75;
				break;
			case VideoStd_XGA :
				cAttrs.videoStd = VideoStd_XGA_75;
				break;
			case VideoStd_WXGAI :
				cAttrs.videoStd = VideoStd_WXGAI_75;
				break;
			case VideoStd_WXGAII :
				cAttrs.videoStd = VideoStd_WXGAII_75;
				break;
			case VideoStd_WXGAIII :
				cAttrs.videoStd = VideoStd_WXGAIII_75;
				break;
			case VideoStd_WXGAIV :
				cAttrs.videoStd = VideoStd_WXGAIV_75;
				break;
			case VideoStd_SXGAPLUS :
				cAttrs.videoStd = VideoStd_SXGAPLUS_75;
				break;
			case VideoStd_UXGA :
				cAttrs.videoStd = VideoStd_UXGA_75;
				break;
			default :
				break;
		}
	}
	
	if (envp->videoLum == 85) {
		switch (cAttrs.videoStd) {
			case VideoStd_UXGA :
				cAttrs.videoStd = VideoStd_UXGA_85;
				break;
			case VideoStd_WXGAI :
				cAttrs.videoStd = VideoStd_WXGAI_85;
				break;
			case VideoStd_WXGAII :
				cAttrs.videoStd = VideoStd_WXGAII_85;
				break;
			case VideoStd_WXGAIII :
				cAttrs.videoStd = VideoStd_WXGAIII_85;
				break;
			case VideoStd_XVGA :
				cAttrs.videoStd = VideoStd_XVGA_85;
				break;
			case VideoStd_WXGAVI :
				cAttrs.videoStd = VideoStd_CUSTOM;
			default :
				break;
		}
	}
#endif

    buf[0] = 80;
    buf[1] = 0;
	DBG("1.videoStd = %d\n", envp->videoStd);
	if (cAttrs.videoStd == VideoStd_CUSTOM || cAttrs.videoStd == VideoStd_HDMI_CUSTOM) {
		ret = set_customstd(hCapture);
		printf("capture.c: set_customstd() return %d\n", ret);
	}

    while (!gblGetQuit()) {
        if (Capture_detectVideoStd(NULL, &videoStd, &cAttrs) < 0) {
            ERR("Failed to detect video standard, video input connected?\n");
            cleanup(THREAD_FAILURE);
        }
        if (videoStd == VideoStd_NONE || videoStd == VideoStd_AUTO ||videoStd==VideoStd_HDMI_AUTO) {
            ERR("Detected none video standard, video input connected?\n");
            for (i=0; i<10; i++) {
                ioctl(gpio_fd, GIO_OUTPUT, buf);
                usleep(400000);
                buf[1] = ~buf[1];
            }
            continue;
        }
        buf[1] = 0;
        ioctl(gpio_fd, GIO_OUTPUT, buf);
        break;
    }
	DBG("2.videoStd = %d\n", videoStd);

    cAttrs.videoStd = videoStd;
    if (envp->imageWidth > 0 && envp->imageHeight > 0) {
        if (VideoStd_getResolution(cAttrs.videoStd, &width, &height) < 0) {
            ERR("Failed to calculate resolution of video standard\n");
            cleanup(THREAD_FAILURE);
        }

        if (width < envp->imageWidth && height < envp->imageHeight) {
            ERR("User resolution (%ldx%ld) larger than detected (%ldx%ld)\n",
                envp->imageWidth, envp->imageHeight, width, height);
            //cleanup(THREAD_FAILURE);
        }

        capDim.x          = 0;
        capDim.y          = 0;
        capDim.height     = envp->imageHeight & ~0xf;
        capDim.width      = envp->imageWidth & ~0xf;
        capDim.lineLength = BufferGfx_calcLineLength(width, colorSpace);
    } else {
        /* Calculate the dimensions of a video standard given a color space */
        if (BufferGfx_calcDimensions(videoStd, colorSpace, &capDim) < 0) {
            ERR("Failed to calculate Buffer dimensions\n");
            cleanup(THREAD_FAILURE);
        }

        envp->imageWidth  = capDim.width;
        envp->imageHeight = capDim.height;
    }

    /* If it is not component capture with 720P resolution then use framecopy as
       there is a size mismatch between capture, display and video buffers. */
    if((envp->imageWidth == VideoStd_720P_WIDTH) && 
        (envp->imageHeight == VideoStd_720P_HEIGHT)) {
        //frameCopy = FALSE;
    }

    /* Calculate the dimensions of a video standard given a color space */
#if 0
    capDim.width = 1280;
    capDim.height = 1024;
#endif

    envp->imageWidth  = capDim.width;
    envp->imageHeight = capDim.height;

	printf("capture.c: imageWidth=%d, imageHeight=%d\n", (int)envp->imageWidth, (int)envp->imageHeight);

    if(frameCopy == FALSE) {
        gfxAttrs.dim.height = capDim.height;
        gfxAttrs.dim.width = capDim.width;
        gfxAttrs.dim.lineLength = ((Int32)((BufferGfx_calcLineLength(gfxAttrs.dim.width, colorSpace)+31)/32))*32;
        gfxAttrs.dim.x = 0;
        gfxAttrs.dim.y = 0;
        if (colorSpace ==  ColorSpace_YUV420PSEMI) {
            bufSize = gfxAttrs.dim.lineLength * gfxAttrs.dim.height * 3 / 2;
        } else {
            bufSize = gfxAttrs.dim.lineLength * gfxAttrs.dim.height * 2;
        }

        /* Create a table of buffers to use with the device drivers */
        gfxAttrs.colorSpace = colorSpace;
        hBufTab = BufTab_create(NUM_BUFS, bufSize,
                                BufferGfx_getBufferAttrs(&gfxAttrs));
        if (hBufTab == NULL) {
            ERR("Failed to create buftab\n");
            cleanup(THREAD_FAILURE);
        }
    } else {
        gfxAttrs.dim = capDim;
    }
    /* Update global data for user interface */
    //gblSetImageWidth(envp->imageWidth);
    //gblSetImageHeight(envp->imageHeight);

#ifdef OSDTEST
	ret = Vis_OSD_LoadAttrs(osdAttrs_array, envp->imageWidth);
	if (ret > 0) {
		printf("capture.c: load osd params for osd success\n");
		for (i=0; i<MAX_OSD_NUM; ++i) {
//			printf("osdAttrs[%d].osdParams->enable=%d\n", i, osdAttrs_array[i].osdParams->enable);
			if (osdAttrs_array[i].osdParams && osdAttrs_array[i].osdParams->enable) {
				osdAttrs_array[i].imageWidth = envp->imageWidth;
				osdAttrs_array[i].imageHeight = envp->imageHeight;
				osdAttrs_array[i].colorSpace = colorSpace;
				hVisOSD_array[i] = Vis_OSD_create(osdAttrs_array+i);
				if (hVisOSD_array[i] == NULL) {
					printf("<w>capture.c: create Vis_OSD_create failed\n");
			//		cleanup(THREAD_FAILURE);
				} else {
					printf("capture.c: [%d]font_size=%d\n", i, hVisOSD_array[i]->font_table_size);
			//		for (i=0; i<hVisOSD_array[i]->font_table_size; ++i) {
			//			print_font(hVisOSD_array[i]->pfont_table+i);
			//		}
				}
			}
		}
	} else {
		printf("<w>capture.c:load osd params for osd failed\n");
	}
#endif

    /* Report the video standard and image size back to the main thread */
    Rendezvous_meet(envp->hRendezvousCapStd);

    if(envp->videoStd == VideoStd_720P_60 || videoStd == VideoStd_720P_60) {
        //cAttrs.videoStd = VideoStd_720P_30;
    } else {
        //cAttrs.videoStd = envp->videoStd;    
        cAttrs.videoStd = videoStd;    
    }

    cAttrs.numBufs    = NUM_CAPTURE_BUFS;
    cAttrs.colorSpace = colorSpace;
    cAttrs.captureDimension = &gfxAttrs.dim;
    /* Create the capture device driver instance */
    hCapture = Capture_create(hBufTab, &cAttrs);
    if (hCapture == NULL) {
        ERR("Failed to create capture device\n");
        cleanup(THREAD_FAILURE);
    }
	if (videoStd == VideoStd_1080P_60 || videoStd == VideoStd_WXGAVI) {
		int err = adv7441_vga1080p60_ripple_debug(hCapture);
		if (err) {
			ERR("set adv7441 register for VGA_1080P_60 ripple debug failure");
		} else {
			DBG("set adv7441 register for VGA_1080P_60 ripple debug success");
		}
	}
//	Capture_setLum(hCapture, envp->videoLum);
	retval = set_capdim(hCapture);
	printf("app.capture.c:set_capdim return %d\n", retval);

    //linxj 2012-06-20
#ifdef CAPTURE_GET_NONBLOCK
    if(envp->videoStd == envp->videoStd == VideoStd_AUTO || envp->videoStd == VideoStd_HDMI_AUTO)
    {
        //int fd = hCapture->fd;
        int noblock=1,ret;
//		ret=ioctl(fd,FIONBIO,&noblock);
        ret=Capture_ctl(hCapture,FIONBIO,&noblock);
//		if(ret!=0)
            printf("capture.c:ioctl return = %d \n",ret);
    }
#endif

    if (frameCopy == TRUE) {
        /* Get a buffer from the video thread */
        fifoRet = Fifo_get(envp->hInFifo, &hDstBuf);
        if (fifoRet < 0) {
            ERR("Failed to get buffer from video thread\n");
            cleanup(THREAD_FAILURE);
        }

        /* Did the video thread flush the fifo? */
        if (fifoRet == Dmai_EFLUSH) {
            cleanup(THREAD_SUCCESS);
        }

        /* Create frame copy module for display buffer */
        fcAttrs.accel = TRUE;
#if 0
        hFcDisp = Framecopy_create(&fcAttrs);
        if (hFcDisp == NULL) {
            ERR("Failed to create frame copy job\n");
            cleanup(THREAD_FAILURE);
        }
#endif    
        /* Create frame copy module for encode buffer */
        hFcEnc = Framecopy_create(&fcAttrs);
        if (hFcEnc == NULL) {
            ERR("Failed to create frame copy job\n");
            cleanup(THREAD_FAILURE);
        }

        if (Framecopy_config(hFcEnc,
                             BufTab_getBuf(Capture_getBufTab(hCapture), 0),
                             hDstBuf) < 0) {
            ERR("Failed to configure frame copy job\n");
            cleanup(THREAD_FAILURE);
        }
    } else {
        for (bufIdx = 0; bufIdx < VIDEO_PIPE_SIZE; bufIdx++) {
            /* Queue the video buffers for main thread processing */
            hBuf = BufTab_getFreeBuf(hBufTab);
            if (hBuf == NULL) {
                ERR("Failed to fill video pipeline\n");
                cleanup(THREAD_FAILURE);
            }
            /* Send buffer to video thread for encoding */
            if (Fifo_put(envp->hOutFifo, hBuf) < 0) {
                ERR("Failed to send buffer to display thread\n");
                cleanup(THREAD_FAILURE);
            }
        }
    }

    /* Signal that initialization is done and wait for other threads */
    Rendezvous_meet(envp->hRendezvousInit);

 	printf("capture.c: before main loop! quit_flag = %d \n", gblGetQuit());
    while (!gblGetQuit()) {
        if (!envp->videoEnable) {
            sleep(1);
            continue;
        }

#ifdef OSDTEST
		/* read fifo, any changes? */
		ret=read(fifofd_m2s_capture, &fifomsg, sizeof(fifomsg));
		if (ret == -1) {
			if (EAGAIN != errno) {
				printf("capture: read fifo error, ret=%d, fd=%d:%s\n", ret, fifofd_m2s_capture, strerror(errno));
			}
		} else if (ret == 0) {
			;
		} else {
//			printf("capture.c: sync=%c%c%c%c, type=0x%04x\n", fifomsg.sync[0],fifomsg.sync[1],fifomsg.sync[2],fifomsg.sync[3],fifomsg.type);
			if (fifomsg.sync[0]=='v' && fifomsg.sync[1]=='i' && fifomsg.sync[2]=='s' && fifomsg.sync[3]=='h') {
				switch (fifomsg.type) {
					case FIFO_M2S_CAPTURE_CHANGEOSD :
//						cleanup(THREAD_FAILURE);	//test
						printf("capture.c:recreate hVisOSD\n");
						for (i=0; i<MAX_OSD_NUM; ++i) {
							if (hVisOSD_array[i]) {
								Vis_OSD_close(hVisOSD_array[i]);
								hVisOSD_array[i] = NULL;
							}
						}
						memset(osdAttrs_array, 0, sizeof(osdAttrs_array));
						ret = Vis_OSD_LoadAttrs(osdAttrs_array, envp->imageWidth);
						if (ret > 0) {
							printf("capture.c:load osd params for osd success\n");
							for (i=0; i<MAX_OSD_NUM; ++i) {
								if (osdAttrs_array[i].osdParams && osdAttrs_array[i].osdParams->enable) {
									osdAttrs_array[i].imageWidth = envp->imageWidth;
									osdAttrs_array[i].imageHeight = envp->imageHeight;
									osdAttrs_array[i].colorSpace = colorSpace;
									hVisOSD_array[i] = Vis_OSD_create(osdAttrs_array+i);
									if (hVisOSD_array[i] == NULL) {
										printf("<w>capture.c: create Vis_OSD_create failed\n");
								//		cleanup(THREAD_FAILURE);
									} else {
										printf("capture.c: [%d]font_size=%d\n", i, hVisOSD_array[i]->font_table_size);
								//		for (i=0; i<hVisOSD_array[i]->font_table_size; ++i) {
								//			print_font(hVisOSD_array[i]->pfont_table+i);
								//		}
									}
								}
							}
						} else {
							printf("<w>capture.c:load osd params for osd failed\n");
						}
						break;

					default :
						break;
				}
			} else {
				printf("capture.c:fifomsg sync error,just drop\n");
			}
		}
#endif

        ioctl(gpio_fd, GIO_OUTPUT, buf);
        buf[1] = ~buf[1];
//		CAPTURE_SYNC();
		capture_sync(&desttime, destval);

        /* Capture a frame */
        if (Capture_get(hCapture, &hCapBuf) < 0) 
        {
//			printf("capture.c: Capture_get() in NON_BLOCK mode\n");
#ifdef CAPTURE_GET_NONBLOCK
            int cleanup=1,i;
            //linxj 2012-06-20
            if(envp->videoStd == VideoStd_AUTO || envp->videoStd==VideoStd_HDMI_AUTO)
            {
                VideoStd_Type tempstd;
                for(i=0;i<10;i++)
                {
                    if(gblGetQuit()) {break;}

                    retval = Capture_getQueryedVideoStd(hCapture,&tempstd);
                    if(retval==Dmai_EOK)
                    {
                        printf("capture.c: getQueryedVideoStd std=%d, oldStd=%d, i=%d! \n",tempstd,videoStd,i);
                        if(tempstd!=videoStd) {
//                            reboot_flg = 1;
                            break;
                        }
                    }else
                    {
                        printf("capture.c: getQueryedVideoStd failed! \n");
                        break;
                    }

                    usleep(10000);  //10ms
                    if ((retval = Capture_get(hCapture, &hCapBuf)) < 0) 
                    {
                        printf("capture.c: Cpture_get() return %d\n", retval);
                    }else {
                        cleanup = 0;
                        break;
                    }
                }
            }

            if(cleanup==1)
            {
				ERR("Failed to get capture buffer, non-block_model\n");
				cleanup(THREAD_FAILURE);
            }
#else
			ERR("Failed to get capture buffer, block_model\n");
			cleanup(THREAD_FAILURE);
#endif
		}

#ifdef OSDTEST
		for (i=0; i<MAX_OSD_NUM; ++i) {
			if (hVisOSD_array[i]) {
				switch (i) {
				case 0 :
//					str = hVisOSD_array[i]->osdParams->text;
					memcpy(str, hVisOSD_array[i]->osdParams->text, sizeof(str));
					break;
				case 1 :
					gettimeofday(&tvosd, NULL);
					tmosd = localtime(&tvosd.tv_sec);
					sprintf(strtime, "%d/%02d/%02d %02d:%02d:%02d", tmosd->tm_year+1900,tmosd->tm_mon+1, tmosd->tm_mday, tmosd->tm_hour, tmosd->tm_min, tmosd->tm_sec);
					for (j=0; j<=strlen(strtime); ++j) str[j] = (unsigned short)strtime[j];
					str[j] = 0;
					break;
				}
				if (-1 == OSD_Str2Stream(hVisOSD_array[i], str, (unsigned char*)Buffer_getUserPtr(hCapBuf))) {
					printf("capture.c: input osd string error\n");
		//			cleanup(THREAD_FAILURE);
				}
			}
		}
#endif

#ifdef AUTO_RESOLUTION_DETECT
//		printf("VideoStd=%d, AUTO=%d\n", videoStd, VideoStd_AUTO);
		if (videoStd_old == VideoStd_AUTO || videoStd_old == VideoStd_HDMI_AUTO) {
			int i = 0;
//			printf("getQueryedVideoStd\n");
			for (i=0; i<5; ++i) {
				retval = Capture_getQueryedVideoStd(hCapture, &videoStd_tmp);
				if (retval != Dmai_EOK) {
					printf("capture.c:getQueryedVideoStd failed\n");
					cleanup(THREAD_FAILURE);
				}
				if (VideoStd_SDI_AUTO==videoStd_tmp || VideoStd_AUTO==videoStd_tmp || VideoStd_HDMI_AUTO==videoStd_tmp) {
					continue;
				}
				if (5 == i) {
					printf("[E]capture.c: too many times get AUTO return in Capture_getQueryedVideoStd()\n");
					cleanup(THREAD_FAILURE);
				}
				if (videoStd != videoStd_tmp) {
					printf("capture.c: Resolution changed, std=%d, oldStd=%d\n", videoStd_tmp, videoStd);
					kill(getppid(), SIGUSR1);
					cleanup(THREAD_FAILURE);
				}
			}
		}
#endif
        //printf("capture.c: after capture get\n");

        if (frameCopy == TRUE) {
            /* Copy the captured buffer to the encode buffer */
            if (Framecopy_execute(hFcEnc, hCapBuf, hDstBuf) < 0) {
                ERR("Failed to execute frame copy job\n");
                cleanup(THREAD_FAILURE);
            }
            /* Send buffer to video thread for encoding */
            if (Fifo_put(envp->hOutFifo, hDstBuf) < 0) {
                ERR("Failed to send buffer to display thread\n");
                cleanup(THREAD_FAILURE);
            }            
        } else {
            /* Send buffer to video thread for encoding */
            if (Fifo_put(envp->hOutFifo, hCapBuf) < 0) {
                ERR("Failed to send buffer to display thread\n");
                cleanup(THREAD_FAILURE);
            }
        }
        
        /* Get a buffer from the video thread */
        fifoRet = Fifo_get(envp->hInFifo, &hDstBuf);
        if (fifoRet < 0) {
            ERR("Failed to get buffer from video thread\n");
            cleanup(THREAD_FAILURE);
        }

        /* Did the video thread flush the fifo? */
        if (fifoRet == Dmai_EFLUSH) {
            cleanup(THREAD_SUCCESS);
        }

        /* Incremement statistics for the user interface */
        //gblIncFrames();

#if 0
        if (j < 3) {
            j++;
            if (fwrite(Buffer_getUserPtr(hCapBuf),
                       //1440*900*3/2,
                       Buffer_getNumBytesUsed(hCapBuf), 
                       1, 
                       outFile) != 1) {
                        
                ERR("Error writing the capture data to video file\n");
                cleanup(THREAD_FAILURE);
            }
            printf("capture.c: size = %d\n", Buffer_getNumBytesUsed(hCapBuf));
        }
#endif
        if (frameCopy == TRUE) {
            /* Return the buffer to the capture driver */
            if (Capture_put(hCapture, hCapBuf) < 0) {
                ERR("Failed to put capture buffer\n");
                cleanup(THREAD_FAILURE);
            }
        } else {
            /* Return the buffer to the capture driver */
            if (Capture_put(hCapture, hDstBuf) < 0) {
                ERR("Failed to put capture buffer\n");
                cleanup(THREAD_FAILURE);
            }
        }
#if 0
        if (i == 200) {
            Capture_getQueryedVideoStd(hCapture, &queryVideoStd);
            if (videoStd != queryVideoStd) {
                gblSetQuit();
            }
            i = 0;
        }
        i++;
#endif
    }

cleanup:
    /* Make sure the other threads aren't waiting for us */
    Rendezvous_force(envp->hRendezvousCapStd);
    Rendezvous_force(envp->hRendezvousInit);
    Pause_off(envp->hPauseProcess);
    Fifo_flush(envp->hOutFifo);

    /* Meet up with other threads before cleaning up */
    Rendezvous_meet(envp->hRendezvousCleanup);

    /* Clean up the thread before exiting */
#ifdef OSDTEST
	for (i=0; i<MAX_OSD_NUM; ++i) {
		if (hVisOSD_array[i]) {
		    Vis_OSD_close(hVisOSD_array[i]);
		    hVisOSD_array[i] = NULL;
		}
	}

	if (fifofd_m2s_capture != -1) {
		close(fifofd_m2s_capture);
		fifofd_m2s_capture = -1;
	}
#endif

    if (hFcDisp) {
        Framecopy_delete(hFcDisp);
    }

    if (hFcEnc) {
        Framecopy_delete(hFcEnc);
    }

    if (hCapture) {
        Capture_delete(hCapture);
    }
    
    /* Clean up the thread before exiting */
    if (hBufTab) {
        BufTab_delete(hBufTab);
    }

//    if (reboot_flg) {
//      system("init 6");
//}

    return status;
}

#if 1
//#define CLIENT_DYNAMICTCPPORT  6666
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
    servaddr.sin_port = htons(CLIENT_DYNAMICTCPPORT);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
		close(listenfd);
        listenfd = -1;
        return listenfd;
    }
	if (listen(listenfd, 4) < 0) {
		close(listenfd);
        listenfd = -1;
	}        

    return listenfd;
}

#define SEND_BUFLEN 1024
#define RECV_BUFLEN 1024
static  char	sendbuf[SEND_BUFLEN];
static	char    recvbuf[RECV_BUFLEN];
/******************************************************************************
 * dynamicThrFxn
 ******************************************************************************/
Void *dynamicThrFxn(Void *arg)
{
    DemandEnv           *envp     = (DemandEnv *) arg;
    Void                *status   = THREAD_SUCCESS;
    int		listenfd, connfd = -1;
    I2cData i2c_data;
    int     num=0,i,err;
    struct sockaddr_in cliaddr;
    socklen_t cliaddr_len;
    int		message_len;

    //*
    printf("dynamic:: get in \n");
    listenfd = serv_listen();

    if(listenfd == -1)
    {
        printf("listenfd==-1 \n");
        cleanup(THREAD_FAILURE);
        //goto dynamic_cleanup;
    }
    //*/

    printf("dynamic:: before meet! \n");
    /* Signal that initialization is done and wait for other threads */
    Rendezvous_meet(envp->hRendezvousInit);
    printf("dynamic:: after meet! quit_flag = %d \n", gblGetQuit());

    while (!gblGetQuit()) {
		if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len)) > 0) {
            printf("slave accept return!\n");
			message_len = recv(connfd, recvbuf, sizeof(recvbuf), 0);
			if (message_len > 0) {
				if (recvbuf[0] != 'v' || recvbuf[1] != 'i' || recvbuf[2] != 's') {
				    ERR("bad message, just drop!\n");
                    continue;
				} else {
                    //button_flg = 1;
				    printf("message type=0x%02x len=%d\n", recvbuf[3],message_len);
				    switch (recvbuf[3]) {
                    case VIS_RQ_SETI2CDATA:
                        if(message_len - 4 == sizeof(i2c_data))
                        {
                            unsigned char i2c_addr;
                            memcpy(&i2c_data,&recvbuf[4],sizeof(i2c_data));
                            i2c_addr = i2c_data.i2caddr >> 1;
                            if(i2c_data.size==sizeof(i2c_data)&&i2c_addr&&hCapture)
                            {
                                num = i2c_data.dataLen;
                                i=0;
                                while(num>0)
                                {
                                    err=Capture_seti2cReg(hCapture,i2c_addr,i2c_data.reg[i],i2c_data.val[i]);
                                    if(err)
                                    {
                                        ERR("seti2creg error!");
                                        break ;
                                    }
                                    i++;
                                    num--;
                                }
                            }
                        }
                        break;
						
                    case VIS_RQ_GETI2CDATA:
                        if(message_len - 4 == sizeof(i2c_data))
                        {
                            unsigned char i2c_addr;
                            int i2cval;
                            memcpy(&i2c_data,&recvbuf[4],sizeof(i2c_data));
                            i2c_addr = i2c_data.i2caddr >> 1;
                            if(i2c_data.size==sizeof(i2c_data)&&i2c_addr&&hCapture)
                            {
                                num = i2c_data.dataLen;
                                if(num>=256)
                                {
                                    for(i=0;i<256;i++)
                                        i2c_data.reg[i]=i;
                                    num = 256;
                                }
                                i=0;
                                while(num>0)
                                {
                                    err=Capture_geti2cReg(hCapture,i2c_addr,i2c_data.reg[i],&i2cval);
                                    if(err)
                                    {
                                        ERR("geti2creg error!");
                                        break ;
                                    }
                                    i2c_data.val[i]=i2cval;
                                    i++;
                                    num--;
                                }
                                if(num==0)
                                    i2c_data.type = 1;
                                else
                                    i2c_data.type = 0;
                                //send back
                                sendbuf[0]='v';sendbuf[1]='i';sendbuf[2]='s';sendbuf[3]=VIS_RS_SENDI2CDATA;
                                memcpy(&sendbuf[4],&i2c_data,sizeof(i2c_data));
                                err = send(connfd,sendbuf,message_len,0);
                                if(err!=message_len){
                                    printf("sendto err =%d \n",err);
                                }
                            }
                        }
                        break;

					case VIS_RQ_SETVIDEODIM:
					{
						int ret = 0;
			//			FILE *fp_dimconfig = NULL;
						struct capture_video_windows captureWin;
						memset(&captureWin, 0, sizeof(Capture_VideoWin));
						if ((4+sizeof(Capture_VideoWin)) <= message_len) {
							memcpy(&captureWin, recvbuf+4, sizeof(Capture_VideoWin));
						} else {
							ERR("bad message, just drop, VIS_RQ_SETVIDEODIM");
							break;
						}
						if (hCapture != NULL) {
							if (-1!=captureWin.left && -1!=captureWin.top) {
								ret = Capture_setVideoWindows(hCapture, &captureWin);
								if (ret == -1) {
									printf("capture.c: dynamic set capdim error, return=%d\n", ret);
									break;
								}
								printf("capture.c: dynamic set capdim success\n");
							}
							if (-1 == save_capdim(hCapture, &captureWin)) {
								printf("<w>save_capdim return -1\n");
							}
							/*
							if ((fp_dimconfig=fopen(CAPTUREDIM_FILE, "w")) == NULL) {
								perror("capture.c: open dim file fail");
								break;
							}
							if ((write_len=fwrite(&captureWin, 1, sizeof(Capture_VideoWin), fp_dimconfig)) != sizeof(Capture_VideoWin)) {
								printf("capture.c:write captureWin to capture.ini error, size=%d, write_len=%d\n", sizeof(Capture_VideoWin), write_len);
								fclose(fp_dimconfig);
								fp_dimconfig = NULL;
								break;
							}
							fclose(fp_dimconfig);
							fp_dimconfig = NULL;
							*/
							printf("capture.c: save captureWin params success\n");
						}
						break;
					}

					case VIS_RQ_GETVIDEODIM:
					{
						int ret=0;
						struct capture_video_windows captureWin;
						memset(&captureWin, 0, sizeof(Capture_VideoWin));
						if (hCapture != NULL) {
							ret = Capture_getVideoWindows(hCapture, &captureWin);
							if (ret == -1) {
								printf("capture.c: dynamic get capdim error, return=%d\n", ret);
								break;
							}
							sendbuf[0]='v';sendbuf[1]='i';sendbuf[2]='s';sendbuf[3]=VIS_RS_SENDVIDEODIM;
							memcpy(&sendbuf[4],&captureWin,sizeof(Capture_VideoWin));
							err = send(connfd,sendbuf,sizeof(Capture_VideoWin)+4,0);
							if(err!=(sizeof(Capture_VideoWin)+4)){
								printf("sendto err =%d \n",err);
							}
						}
						break;
					}

					case VIS_RQ_SETCUSTOMSTD:
					{
						int write_len = 0;
						FILE *fp_customstd = NULL;
						Capture_customer_param cparam;
						memset(&cparam, 0, sizeof(Capture_customer_param));
						if ((4+sizeof(Capture_customer_param)) <= message_len) {
							memcpy(&cparam, recvbuf+4, sizeof(Capture_customer_param));
						} else {
							ERR("bad message, just drop, VIS_RQ_SETCUSTOMSTD");
							break;
						}
						if ((fp_customstd=fopen(CUSTOMSTD_FILE, "wb")) == NULL) {
							perror("capture.c: open dim file fail");
							break;
						}
						if ((write_len=fwrite(&cparam, 1, sizeof(Capture_customer_param), fp_customstd)) != sizeof(Capture_customer_param)) {
							printf("capture.c:write cparam to capturedim.cfg error, size=%d, write_len=%d\n", sizeof(Capture_customer_param), write_len);
							fclose(fp_customstd);
							fp_customstd = NULL;
							break;
						}
						printf("capture.c: save cparam params success\n");
						fclose(fp_customstd);
						fp_customstd = NULL;
						cleanup(THREAD_SUCCESS);
						break;
					}
					
					case VIS_RQ_GETCUSTOMSTD:
					{
						int ret=0;
						struct capture_customer_param cparam;
						memset(&cparam, 0, sizeof(Capture_customer_param));
						if (hCapture != NULL) {
							ret = Capture_getCustomerVideoStd(hCapture, &cparam);
							if (ret == -1) {
								printf("capture.c: dynamic get capdim error, return=%d\n", ret);
								break;
							}
							sendbuf[0]='v';sendbuf[1]='i';sendbuf[2]='s';sendbuf[3]=VIS_RS_SENDCUSTOMSTD;
							memcpy(&sendbuf[4],&cparam,sizeof(Capture_customer_param));
							err = send(connfd,sendbuf,sizeof(Capture_customer_param)+4,0);
							if(err!=(sizeof(Capture_customer_param)+4)){
								printf("sendto err =%d \n",err);
							}
						}
						break;
					}

                    default:
                        break;
                    }//end of case
                }//end of if header 
            }
            close(connfd);
        }//end of connected
	}
cleanup:
    printf(" dynamic: clean up!\n");
    Rendezvous_force(envp->hRendezvousInit);
    
    /* Meet up with other threads before cleaning up */
    Rendezvous_meet(envp->hRendezvousCleanup);
	if (listenfd !=-1) {
		close(listenfd);
		listenfd = -1;
	}
	if (connfd != -1) {
		close(connfd);
		connfd = -1;
	}
    return status;
}
#endif

static int set_capdim(Capture_Handle hCapture)
{
	FILE *fp = NULL;
	struct capdim_save_params *params_buf = NULL;
	struct capdim_save_params *params = NULL;
	int ret=0, std;
	struct capture_video_windows videoWin = {0, 0, 0, 0};

	if (!hCapture) {
		printf("<w>set_capdim params invalid\n");
		goto cleanup;
	}
	std = Capture_getVideoStd(hCapture);
	fp = fopen(CAPTUREDIM_FILE, "r");
	if (fp) {
		params_buf = (struct capdim_save_params *)calloc(1, sizeof(struct capdim_save_params));
		if (!params_buf) {
			perror("<w>set_capdim alloc mem for params failed");
			ret = -1;
			goto cleanup;
		}
		
		while (!feof(fp)) {
			ret = fread(params_buf, 1, sizeof(struct capdim_save_params), fp);
			if (-1 == ret) goto cleanup;
			if (params_buf->videoStd == std) {
				params = params_buf;
				break;
			}
		}
	} else {
		perror("<w>set_capdim open capturedim.cfg failed");
		ret = -1;
	}
	if (params) {
//		if (VideoStd_getResolution(Capture_getVideoStd(hCapture), &videoWin.width, &videoWin.height) < 0) {
//			ERR("<w>set_capdim Failed to calculate resolution of video standard\n");
//			ret = -1;
//			goto cleanup;
//		}
		videoWin.top = params->top;
		videoWin.left = params->left;
		videoWin.width = params->width;
		videoWin.height = params->height;
		printf("top=%d, left=%d, width=%d, height=%d\n", videoWin.top, videoWin.left, videoWin.width, videoWin.height);
		ret = Capture_setVideoWindows(hCapture, &videoWin);
		if (-1 == ret) printf("<w>set_capdim Capture_setVideoWindows() failed\n");
	}

cleanup:
	if (fp) {
		fclose(fp);
		fp = NULL;
	}
	if (params_buf) {
		free(params_buf);
		params_buf = NULL;
	}
	return ret;
}

static int save_capdim(Capture_Handle hCapture, struct capture_video_windows *videoWin) {
	FILE *fp = NULL;
	struct capdim_save_params *params_buf = NULL;
	int params_count, i=0, ret=0, save_count=0;

	if (!hCapture || !videoWin) {
		printf("<w>save_capdim params invalid\n");
		return -1;
	}

	fp = fopen(CAPTUREDIM_FILE, "r");
	if (fp) {
		if (!(params_buf = (struct capdim_save_params *)calloc(VideoStd_COUNT, sizeof(struct capdim_save_params)))) {
			perror("<w>save_capdim alloc mem for params failed");
			fclose(fp); fp=NULL;
			return -1;
		}
		params_count = fread(params_buf, sizeof(struct capdim_save_params), VideoStd_COUNT, fp);
		if (-1 == params_count) {
			perror("<w>save_capdim read capturedim.cfg error");
			free(params_buf); params_buf=NULL;
			fclose(fp); fp=NULL;
			return -1;
		}
		fclose(fp); fp=NULL;
		(params_buf+params_count)->videoStd = Capture_getVideoStd(hCapture);
		for (i=0; (params_buf+i)->videoStd!=Capture_getVideoStd(hCapture); ++i);
		(params_buf+i)->top = videoWin->top;
		(params_buf+i)->left = videoWin->left;
		(params_buf+i)->width = videoWin->width;
		(params_buf+i)->height = videoWin->height;
		if (i==params_count) ++params_count;	//new videStd config save in tail
		fp = fopen(CAPTUREDIM_FILE, "w");
		if (!fp) {
			perror("<w>save_capdim open capturedim.cfg failed");
			free(params_buf); params_buf=NULL;
			return -1;
		}
		for (i=0; i<params_count; ++i) {
			if ((0xffff!=(params_buf+i)->left) && (0xffff!=(params_buf+i)->top)) {
				ret = fwrite(params_buf+i, sizeof(struct capdim_save_params), 1, fp);
				if (-1 == ret) {
					perror("<w>save_capdim write params to file error");
					free(params_buf); params_buf=NULL;
					fclose(fp); fp=NULL;
					return -1;
				}
				++save_count;
			}
		}
	} else {
		if (-1==videoWin->top || -1==videoWin->left || -1==videoWin->width || -1==videoWin->height) {
			goto cleanup;
		}
		struct capdim_save_params params = {Capture_getVideoStd(hCapture), videoWin->top, videoWin->left, videoWin->width, videoWin->height};
		fp = fopen(CAPTUREDIM_FILE, "w");
		if (!fp) {
			perror("<w>save_capdim open capturedim.cfg failed");
			return -1;
		}
		fwrite(&params, 1, sizeof(struct capdim_save_params), fp);
		printf("save_capdim create new capturedim.cfg file\n");
		save_count = 1;
	}

	printf("save_capdim success, count=%d\n", save_count);

cleanup:
	if (fp) {
		fclose(fp);
		fp = NULL;
	}
	if (params_buf) {
		free(params_buf);
		params_buf = NULL;
	}
	return 0;
}

static int set_customstd(Capture_Handle hCapture) {
    int read_len = 0, ret = 0;
    FILE *fp_customstd = NULL;
    struct capture_customer_param cparam;
    struct capture_customer_param cparam_default = {
        1024,       //width
        768,        //height
        60,         //framerate
        65000000,   //pixelClock
        1344,       //lineClock
        805,        //lineCounter
        0,          //offset_horizontal
        0,          //offset_vertical
        0,          //type
        0,          //block_length
        0,          //line_vs
        0,          //line_field
        0           //fieldlength
    };

    if ((fp_customstd=fopen(CUSTOMSTD_FILE, "rb")) == NULL) {
        perror("capture.c: in set_customstd() open customstd.ini error");
        ret = -1;
        goto cleanup;
    }

    memset(&cparam, 0, sizeof(Capture_customer_param));
    if ((read_len=fread(&cparam, 1, sizeof(struct capture_customer_param), fp_customstd)) != sizeof(struct capture_customer_param)) {
        printf("capture.c: in set_customstd() read cparam error, size=%d, read_len=%d\n", sizeof(struct capture_customer_param), read_len);
        ret = -1;
        goto cleanup;
    }
    printf("capture.c: %u %u %d %u %hu %hu %hd %hd,  %d %hd %hd %hd %hd\n", cparam.width, cparam.height, cparam.framerate, cparam.pixelClock, cparam.lineClock, cparam.lineCounter, cparam.offset_horizontal, cparam.offset_vertical, cparam.type, cparam.blocklength, cparam.lines_vs, cparam.lines_field, cparam.fieldlength);

    if(Capture_setCustomerVideoStd(NULL, &cparam) < 0) {
        ERR("capture.c: set custom videoStd failed\n");
        ret = -1;
        goto cleanup;
    }

cleanup:
    if (ret) Capture_setCustomerVideoStd(NULL, &cparam_default);
    if (fp_customstd) {
        fclose(fp_customstd);
        fp_customstd = NULL;
    }
    return ret;
}

static int adv7441_vga1080p60_ripple_debug(Capture_Handle hCapture) {
	unsigned char buf[3] = {0x21, 0x6a, 0x50};
	if (hCapture == NULL) {
		return -1;
	}
	return Capture_seti2cReg(hCapture, buf[0], buf[1], buf[2]);
}
