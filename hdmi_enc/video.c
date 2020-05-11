/*
 * video.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <xdc/std.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/BufTab.h>
#include <ti/sdo/dmai/VideoStd.h>
#include <ti/sdo/dmai/BufferGfx.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include <ti/sdo/dmai/ce/Venc1.h>

#include "video.h"
#include "../demo.h"
#include "vis_common.h"
#include "visconfig.h"

#ifndef YUV_420SP
#define YUV_420SP 256
#endif 

//#define VIDEO_CODEC_FORCE_IDR_FRAME


extern int encodedFrameType;
extern unsigned char idrhead[IDRHEAD_LEN];
extern DynamicParams dynamic_params;
DynamicParam video_dynamicparams;
extern long long destval;
/* Number of buffers in the pipe to the capture thread */
/* Note: It needs to match capture.c pipe size */
#define VIDEO_PIPE_SIZE           3

/******************************************************************************
 * videoThrFxn
 ******************************************************************************/
Void *videoThrFxn(Void *arg)
{
    VideoEnv               *envp                = (VideoEnv *) arg;
    Void                   *status              = THREAD_SUCCESS;
    VIDENC1_Params          defaultParams       = Venc1_Params_DEFAULT;
    VIDENC1_DynamicParams   defaultDynParams    = Venc1_DynamicParams_DEFAULT;
    BufferGfx_Attrs         gfxAttrs            = BufferGfx_Attrs_DEFAULT;
    Venc1_Handle            hVe1                = NULL;
    Engine_Handle           hEngine             = NULL;
    BufTab_Handle           hBufTab             = NULL;
    Buffer_Handle           hCapBuf, hDstBuf;
    VIDENC1_Params         *params;
    VIDENC1_DynamicParams  *dynParams;
    Int                     fifoRet;
    Int                     bufIdx;
    ColorSpace_Type         colorSpace =  ColorSpace_YUV420PSEMI;
    Bool                    localBufferAlloc = TRUE;
#ifdef VIDEO_CODEC_FORCE_IDR_FRAME
    UInt                    frameCnt = 0;
    VIDENC1_Status          encStatus;//libo
    VIDENC1_Handle	        h_venc1;
    /* Set video encoder dynamic parameters */
    encStatus.size = sizeof(VIDENC1_Status);
    encStatus.data.buf = NULL;
#endif
#ifndef VIDEO_CODEC_FORCE_IDR_FRAME
    short first = 1;
    char *ptr;
#endif
    int ret;
	int fifofd_m2s_video;
	unsigned char fifobuf[sizeof(Fifo_M2S_Msg)+sizeof(DynamicParams)] = {0, };
#if 0
	//steven 11-18-09
    FILE *outFile = NULL;
    int saveOnce = 45;
    char fname[64] = {0};
    int i = 0, j = 1;
    struct timeval tv1, tv2;
#endif

    if ((fifofd_m2s_video = open(FIFO_M2S_VIDEO, O_RDWR | O_NONBLOCK)) == -1) {
        perror("open videofifo");
    }
   	
    /* Open the codec engine */
    hEngine = Engine_open(envp->engineName, NULL, NULL);

    if (hEngine == NULL) {
        ERR("Failed to open codec engine %s\n", envp->engineName);
        cleanup(THREAD_FAILURE);
    }

    /* In case of 720P resolution the video buffer will be allocated
       by capture thread. */
    if((envp->imageWidth == VideoStd_720P_WIDTH) && 
        (envp->imageHeight == VideoStd_720P_HEIGHT)) {
        //localBufferAlloc = FALSE;
    } 

    /* Use supplied params if any, otherwise use defaults */
    params = envp->params ? envp->params : &defaultParams;
    dynParams = envp->dynParams ? envp->dynParams : &defaultDynParams;
#if 0
    printf("video.c: envp->imageWidth = %d\n", (int)envp->imageWidth);		//steven 11-12-09
    printf("video.c: envp->imageHeight = %d\n", (int)envp->imageHeight);		//steven 11-12-09
    printf("video.c: envp->videoBitRate = %d\n", envp->videoBitRate);
    printf("video.c: envp->videoFrameRate = %d\n", envp->videoFrameRate);
#endif
    /* Set up codec parameters */
    params->maxWidth          = envp->imageWidth;
    params->maxHeight         = envp->imageHeight;
    params->encodingPreset    = XDM_HIGH_SPEED;
    if (colorSpace ==  ColorSpace_YUV420PSEMI) { 
        params->inputChromaFormat = XDM_YUV_420SP;
    } else {
        params->inputChromaFormat = XDM_YUV_422ILE;
    }
    params->reconChromaFormat = XDM_YUV_420SP;

    /* Set up codec parameters depending on bit rate */
    //if (envp->videoBitRate <= 0) {
    if (envp->videoCbr == 0) {
        /* Variable bit rate */
        params->rateControlPreset = IVIDEO_NONE;
        //params->rateControlPreset = IVIDEO_STORAGE;

        /*
         * If variable bit rate use a bogus bit rate value (> 0)
         * since it will be ignored.
         */
        params->maxBitRate        = 0;
        dynParams->targetBitRate  = envp->videoBitRate;
    } else if (envp->videoCbr == 2){
        /* Constrained Variable bit rate */
        params->rateControlPreset = IVIDEO_USER_DEFINED;
        params->maxBitRate  = envp->videoBitRate+100000;
        dynParams->targetBitRate  = envp->videoBitRate;
    } else {
        /* Constant bit rate */
        params->rateControlPreset = IVIDEO_STORAGE;
        //params->rateControlPreset = IVIDEO_LOW_DELAY;
        params->maxBitRate  = envp->videoBitRate;
        dynParams->targetBitRate  = envp->videoBitRate;
    }

    dynParams->inputWidth      = params->maxWidth;
    dynParams->inputHeight     = params->maxHeight;
    dynParams->refFrameRate    = envp->videoFrameRate;
    dynParams->targetFrameRate = envp->videoFrameRate;
    dynParams->interFrameInterval = 0;
	dynParams->intraFrameInterval = envp->intraFrameInterval;
	//dynParams->intraFrameInterval = 0;

    printf("video.c: params->maxWidth&Height = %dx%d\n", (int)params->maxWidth,(int)params->maxHeight); //linxj 2012-02-27
    /* Create the video encoder */
    hVe1 = Venc1_create(hEngine, envp->videoEncoder, params, dynParams);
    if (hVe1 == NULL) {
        ERR("Failed to create video encoder: %s\n", envp->videoEncoder);
        cleanup(THREAD_FAILURE);
    }

    /* Store the output buffer size in the environment */
    envp->outBufSize = Venc1_getOutBufSize(hVe1);

    printf("video.c: envp->outBufSize = %d\n", (int)envp->outBufSize); //linxj 2012-02-27

    /* Signal that the codec is created and output buffer size available */
    Rendezvous_meet(envp->hRendezvousWriter);

    if (localBufferAlloc == TRUE) {
        gfxAttrs.colorSpace = colorSpace;
        gfxAttrs.dim.width  = envp->imageWidth;
        gfxAttrs.dim.height = envp->imageHeight;
        gfxAttrs.dim.lineLength = BufferGfx_calcLineLength(gfxAttrs.dim.width,
                                                           gfxAttrs.colorSpace);
        /*
         * Ask the codec how much input data it needs and create a table of
         * buffers with this size.
         */
        hBufTab = BufTab_create(VIDEO_PIPE_SIZE, Venc1_getInBufSize(hVe1),
                                BufferGfx_getBufferAttrs(&gfxAttrs));

        if (hBufTab == NULL) {
            ERR("Failed to allocate contiguous buffers\n");
            cleanup(THREAD_FAILURE);
        }

        /* Send buffers to the capture thread to be ready for main loop */
        for (bufIdx = 0; bufIdx < VIDEO_PIPE_SIZE; bufIdx++) {
            if (Fifo_put(envp->hCaptureInFifo,
                         BufTab_getBuf(hBufTab, bufIdx)) < 0) {
                ERR("Failed to send buffer to display thread\n");
                cleanup(THREAD_FAILURE);
            }
        }
    } else {
        /* Send buffers to the capture thread to be ready for main loop */
        for (bufIdx = 0; bufIdx < VIDEO_PIPE_SIZE; bufIdx++) {
            if ((fifoRet = Fifo_get(envp->hCaptureOutFifo, &hCapBuf)) < 0) {
                ERR("Failed to get buffer from capture thread\n");
                cleanup(THREAD_FAILURE);
            }
            if (fifoRet < 0) {
                ERR("Failed to get buffer from video thread\n");
                cleanup(THREAD_FAILURE);
            }

            /* Did the capture thread flush the fifo? */
            if (fifoRet == Dmai_EFLUSH) {
                cleanup(THREAD_SUCCESS);
            }
            /* Return buffer to capture thread */
            if (Fifo_put(envp->hCaptureInFifo, hCapBuf) < 0) {
                ERR("Failed to send buffer to display thread\n");
                cleanup(THREAD_FAILURE);
            }
        }
    }
#ifdef VIDEO_CODEC_FORCE_IDR_FRAME
	h_venc1 = Venc1_getVisaHandle(hVe1);
	dynParams->generateHeader  = XDM_ENCODE_AU;
	DBG("prepare dynamic params ready.\n");
#endif

    /* Signal that initialization is done and wait for other threads */
    Rendezvous_meet(envp->hRendezvousInit);
    
    printf("video.c: before main loop! quit_flag = %d \n", gblGetQuit());
#if 0
		h_venc1 = Venc1_getVisaHandle(hVe1);
		dynParams->generateHeader  = XDM_ENCODE_AU;
		dynParams->forceFrame      = IVIDEO_IDR_FRAME;
//		dynParams->forceFrame      = IVIDEO_P_FRAME;
		VIDENC1_control(h_venc1, XDM_SETPARAMS, dynParams, &encStatus);
#endif
    while (!gblGetQuit()) {
    
        if (!envp->videoEnable) {
            sleep(1);
            continue;
        }
#if 1
        //linxj 2012-05-15
        if (read(fifofd_m2s_video, fifobuf, sizeof(fifobuf)) > 0) {
			Fifo_M2S_Msg *pfifomsg = (Fifo_M2S_Msg *)fifobuf;
			switch (pfifomsg->type) {
				case FIFO_M2S_VIDEO_DYNAMICPARAMS:
				{
					unsigned int lastbitrate,lastfps,lastidrinterval;
					lastbitrate = dynParams->targetBitRate;
					lastfps = dynParams->refFrameRate;
					lastidrinterval = dynParams->intraFrameInterval;
					memcpy(&video_dynamicparams, fifobuf+sizeof(Fifo_M2S_Msg), sizeof(video_dynamicparams));
					if (video_dynamicparams.videobitrate>0) {
						printf("video.c: video_dynamicparams.videobitrate = %d\n", video_dynamicparams.videobitrate);
						dynParams->targetBitRate  = video_dynamicparams.videobitrate*1000;
					}
					if (video_dynamicparams.videofps>0) {
						printf("video.c: video_dynamicparams.videofps = %d\n", video_dynamicparams.videofps);
						dynParams->refFrameRate    =  dynParams->targetFrameRate = video_dynamicparams.videofps*1000;
					}
					if (video_dynamicparams.videoidrinterval>0) {
						printf("video.c: video_dynamicparams.videoidrinterval = %d\n", video_dynamicparams.videoidrinterval);
						dynParams->intraFrameInterval = video_dynamicparams.videoidrinterval;
					}
					printf("video.c: do video dynamic control \n");
					ret=Venc1_dyncontrol(hVe1,dynParams);
					if(ret!=0) 
					{
						printf("video.c:dynamic setting failed! \n");
						dynParams->targetBitRate = lastbitrate;
						dynParams->refFrameRate = lastfps;
						dynParams->intraFrameInterval=lastidrinterval;
					}
					if(ret==0&&video_dynamicparams.videofps>0)
					//if(video_dynamicparams.videofps>0)
					{
						//fps is change
						float tmp;
						envp->videoFrameRate = video_dynamicparams.videofps;
						tmp = envp->videoFrameRate/1000;
						tmp = 1/tmp;
						destval = tmp * 1000000;
//						if(changeFramerate((int) video_dynamicparams.videofps)!=0) printf("video.c: change fps failed! \n");
					}
				   
		//            slaveinfo.videobitrate = dynParams->targetBitRate/1000;
		//            slaveinfo.videofps = dynParams->refFrameRate/1000;
		//            slaveinfo.videoidrinterval = dynParams->intraFrameInterval;
				}	//case FIFO_M2S_VIDEO_DYNAMICPARAMS
			}	//switch
        }
#endif

        /* Get a buffer to encode from the capture thread */
        fifoRet = Fifo_get(envp->hCaptureOutFifo, &hCapBuf);
        if (fifoRet < 0) {
            ERR("Failed to get buffer from capture thread\n");
            cleanup(THREAD_FAILURE);
        }

        /* Did the capture thread flush the fifo? */
        if (fifoRet == Dmai_EFLUSH) {
            cleanup(THREAD_SUCCESS);
        }

        /* Get a buffer to encode to from the writer thread */
        fifoRet = Fifo_get(envp->hWriterOutFifo, &hDstBuf);
        if (fifoRet < 0) {
            ERR("Failed to get buffer from writer thread\n");
            cleanup(THREAD_FAILURE);
        } 
        /* Did the writer thread flush the fifo? */
        if (fifoRet == Dmai_EFLUSH) {
            cleanup(THREAD_SUCCESS);
        }

        /* Make sure the whole buffer is used for input */
        BufferGfx_resetDimensions(hCapBuf);
#ifdef VIDEO_CODEC_FORCE_IDR_FRAME
		frameCnt ? (dynParams->forceFrame=IVIDEO_P_FRAME) : (dynParams->forceFrame=IVIDEO_IDR_FRAME);
		VIDENC1_control(h_venc1, XDM_SETPARAMS, dynParams, &encStatus);
		++frameCnt;
		if (frameCnt>=envp->intraFrameInterval) frameCnt=0;
#endif
        /* encode the video buffer */	
        if ((ret = Venc1_process(hVe1, hCapBuf, hDstBuf)) < 0) {
            ERR("Failed to encode video buffer ret = %d\n", ret);
            cleanup(THREAD_FAILURE);
        }
//		print_in_hex(Buffer_getUserPtr(hDstBuf), 144, "AU_IDR", NULL);
#ifndef VIDEO_CODEC_FORCE_IDR_FRAME
        if (first == 1) {
            memcpy(idrhead, Buffer_getUserPtr(hDstBuf), sizeof(idrhead));
            first = 0;
            ptr = (char *)Buffer_getUserPtr(hDstBuf);
        }
        if (BufferGfx_getFrameType(hCapBuf) == 0) {
            encodedFrameType = 0;
        }
#endif

#if 0
		//steven 11-17-09
		if ( saveOnce > 0 ) {
			
			if (encodedFrameType == 0) {
				sprintf(fname, "video_frames/I-video-%d", i);
				encodedFrameType = 1;
				printf("video.c: it is I frame\n");
			} else {
				sprintf(fname, "video_frames/video-%d", i);
				printf("video.c: it is P frame\n");
			}
		#if 1	//steven 11-18-09
			if ((outFile = fopen(fname, "wb")) == NULL) {
				perror("open video file");
				exit(-1);
			}
		#endif	//steven 11-18-09
			
			//printf("video.c: bufNumBytes = %d\n", Buffer_getNumBytesUsed(hDstBuf));
	        if (fwrite(Buffer_getUserPtr(hDstBuf),
	                   120, //Buffer_getNumBytesUsed(hDstBuf),
	                   1, 
	                   outFile) != 1) {
	                   	
	            ERR("Error writing the capture data to video file\n");
	            cleanup(THREAD_FAILURE);
	        }		
	        fclose(outFile);
	        //printf("capture.c: capture and save one frame of video data\n");
	        saveOnce--;
	        i++;
		}
#endif

        /* Send encoded buffer to writer thread for filesystem output */
        if (Fifo_put(envp->hWriterInFifo, hDstBuf) < 0) {
            ERR("Failed to send buffer to display thread\n");
            cleanup(THREAD_FAILURE);
        }

        /* Return buffer to capture thread */
        if (Fifo_put(envp->hCaptureInFifo, hCapBuf) < 0) {
            ERR("Failed to send buffer to display thread\n");
            cleanup(THREAD_FAILURE);
        }

#if 0
        frameCnt++;
		h_venc1 = Venc1_getVisaHandle(hVe1);
		if (frameCnt % envp->intraFrameInterval == 0) {				//libo
		    dynParams->generateHeader  = XDM_GENERATE_HEADER;
		    dynParams->forceFrame      = IVIDEO_NA_FRAME;
		    VIDENC1_control(h_venc1, XDM_SETPARAMS, dynParams, &encStatus);
		    //printf("XDM_GENERATE_HEADER ...\n");
		} else if (frameCnt % envp->intraFrameInterval == 1) {
		    dynParams->generateHeader  = XDM_ENCODE_AU;
	        dynParams->forceFrame      = IVIDEO_IDR_FRAME;
		    VIDENC1_control(h_venc1, XDM_SETPARAMS, dynParams, &encStatus);
		    //printf("XDM_IDR_FRAME ...\n");
		} else if (frameCnt % envp->intraFrameInterval == 2) {
		    dynParams->generateHeader  = XDM_ENCODE_AU;
	        dynParams->forceFrame      = IVIDEO_NA_FRAME;
		    VIDENC1_control(h_venc1, XDM_SETPARAMS, dynParams, &encStatus);
		    //printf("XDM_NA_FRAME ...\n");
		}
#else
//#ifndef VIDEO_CODEC_FORCE_IDR_FRAME
#if 0
        if (dynamic_params.iframe == 1) {
            h_venc1 = Venc1_getVisaHandle(hVe1);
            if (frameCnt == 0) {
                dynParams->generateHeader  = XDM_GENERATE_HEADER;
                dynParams->forceFrame      = IVIDEO_NA_FRAME;
                VIDENC1_control(h_venc1, XDM_SETPARAMS, dynParams, &encStatus);
                printf("XDM_GENERATE_HEADER ...\n");
            } else if (frameCnt == 1) {
                dynParams->generateHeader  = XDM_ENCODE_AU;
                dynParams->forceFrame      = IVIDEO_IDR_FRAME;
                VIDENC1_control(h_venc1, XDM_SETPARAMS, dynParams, &encStatus);
                printf("XDM_IDR_FRAME ...\n");
            } else if (frameCnt == 2) {
                dynParams->generateHeader  = XDM_ENCODE_AU;
                dynParams->forceFrame      = IVIDEO_NA_FRAME;
                VIDENC1_control(h_venc1, XDM_SETPARAMS, dynParams, &encStatus);
                printf("XDM_NA_FRAME ...\n");
            }

            frameCnt++;
            if (frameCnt > 2) {
                dynamic_params.iframe = 0;
                frameCnt = 0;
            }
        }
#endif
#endif
    }

cleanup:
    /* Make sure the other threads aren't waiting for us */
    Rendezvous_force(envp->hRendezvousInit);
    Rendezvous_force(envp->hRendezvousWriter);
    Pause_off(envp->hPauseProcess);
    Fifo_flush(envp->hWriterInFifo);
    Fifo_flush(envp->hCaptureInFifo);

    /* Make sure the other threads aren't waiting for init to complete */
    Rendezvous_meet(envp->hRendezvousCleanup);

    /* Clean up the thread before exiting */
    if (hBufTab) {
        BufTab_delete(hBufTab);
    }

    if (hVe1) {
        Venc1_delete(hVe1);
    }

    if (hEngine) {
        Engine_close(hEngine);
    }

    return status;
}
