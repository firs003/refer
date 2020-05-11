/*
 * audio.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <xdc/std.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Sound.h>
#include <ti/sdo/dmai/Buffer.h>
#include <ti/sdo/dmai/Loader.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include <ti/sdo/dmai/ce/Aenc1.h>
#include <ti/sdo/dmai/ce/Aenc2.h>

#include "audio.h"
#include "../demo.h"
#include "vis_common.h"

#define BUFSIZE 1152

#define GPIO_DEVICE "/dev/dm365_gpio"
#define GIO_SET_INPUT 3
#define GIO_OUTPUT 4
#define GIO_GET 2

#define ADV7441AUDIO_DEVICE "/proc/adv7441/audio"
static int getHDMIsamplerate()
{
    char audiodevbuf[64] = {0, };
	int samplerate = 0,audio_fd;
    int buflen;
    audio_fd = open(ADV7441AUDIO_DEVICE, O_RDWR);
    if(audio_fd)
    {
        buflen = read(audio_fd,audiodevbuf,sizeof(audiodevbuf));
        if(buflen>2)
        {
            audiodevbuf[buflen] = 0;
            samplerate = atoi(&audiodevbuf[2]);
			//printf("getHDMIsamplerate(),file content:%s,samplerate = %d \n",audiodevbuf,samplerate);
        }
        close(audio_fd);
    }
    return samplerate ;
}

/******************************************************************************
 * audioThrFxn
 ******************************************************************************/
Void *audioThrFxn(Void *arg)
{
    AudioEnv               *envp                = (AudioEnv *) arg;
    VisTS_params 			tsparams 			= (VisTS_params)(envp->tsparams);	//sp 02-09-2010
    Void                   *status              = THREAD_SUCCESS;
    Sound_Attrs             sAttrs              = Sound_Attrs_STEREO_DEFAULT;
    Buffer_Attrs            bAttrs              = Buffer_Attrs_DEFAULT;
    AUDENC1_Params          defaultParams       = Aenc1_Params_DEFAULT;
    AUDENC1_DynamicParams   defaultDynParams    = Aenc1_DynamicParams_DEFAULT;
    Engine_Handle           hEngine             = NULL;
    Sound_Handle            hSound              = NULL;
    Aenc1_Handle            hAe1                = NULL;
    Buffer_Handle           hOutBuf             = NULL;
    Buffer_Handle           hInBuf              = NULL;
    Buffer_Handle           hEncInBuf           = NULL;
    FILE                   *outFile             = NULL;
    AUDENC1_Params         *params;
    AUDENC1_DynamicParams  *dynParams;

    struct timeval      timeNow;
    unsigned char		*dest_ts = NULL;
    unsigned char		*tsdata;
    int                 bytesSent, len;
    int                 gpio_fd,i,audio_fd;
    unsigned int        buf[2] = {0};
    char                audiodevbuf[64];
    int                 fromHDMI = 0;
    int                 hdmiSampleRate = 0,downsample=1;
    int                 readtimes = 0,readfaildtimes=0;
//	int					writefailedtimes=0, ret;

    dest_ts = (unsigned char*)malloc(BUFSIZE);
    if (dest_ts == NULL) {
        ERR("failed to malloc dest_ts buffer!\n");
        exit(-1);
    }

	fromHDMI = envp->soundSource;
	printf("audio.c:fromHDMI=%d\n", fromHDMI);
	gpio_fd = open(GPIO_DEVICE, O_RDWR);
	buf[0] = 30;    //GPIO 30 = high audio from AIC31xx low:ADV7441
	buf[1] = 1;
	buf[1] = 1;     //from aic31xx
	if(fromHDMI==1) 
		buf[1]=0;   //linxj 2012-04-09
	ioctl(gpio_fd, GIO_OUTPUT, buf);

	/* Open the codec engine */
	hEngine = Engine_open(envp->engineName, NULL, NULL);

	if (hEngine == NULL) {
		ERR("Failed to open codec engine %s\n", envp->engineName);
		cleanup(THREAD_FAILURE);
	}

	/* Use supplied params if any, otherwise use defaults */
	params = envp->params ? envp->params : &defaultParams;
	dynParams = envp->dynParams ? envp->dynParams : &defaultDynParams;

	params->bitRate = dynParams->bitRate = envp->soundBitRate;
	if (envp->soundBitRate == 128000) {
		params->bitRate = envp->soundBitRate;
		//dynParams->bitRate = 96000;
	} else {
		params->bitRate = dynParams->bitRate = envp->soundBitRate;
	}
	params->sampleRate = dynParams->sampleRate = envp->sampleRate;
#if 1
	printf("audio.c: params->sampleRate = %d\n", (int)params->sampleRate);
	printf("audio.c: dynParams->sampleRate = %d\n", (int)dynParams->sampleRate);
	printf("audio.c: params->bitRate = %d\n", (int)params->bitRate);
	printf("audio.c: dynParams->bitRate = %d\n", (int)dynParams->bitRate);
#endif 


    /* Signal that initialization is done and wait for other threads */
    Rendezvous_meet(envp->hRendezvousInit);   

    audio_fd = open(ADV7441AUDIO_DEVICE, O_RDWR);
    if(audio_fd)
    {
        int buflen;
        buflen = read(audio_fd,audiodevbuf,sizeof(audiodevbuf));
        if(buflen>0)
        {
            if(fromHDMI==1)
                audiodevbuf[0] = '1';
            else
                audiodevbuf[0] = '0';
            write(audio_fd,audiodevbuf,buflen);
         }else
         {
				ERR("audio.c Failed to read file[%s]\n", ADV7441AUDIO_DEVICE);
         }
        close(audio_fd);
    } else {
		ERR("audio.c Failed to open file[%s]\n", ADV7441AUDIO_DEVICE);
	}

    printf("audio.c : read audio file OK!! \n");
    sleep(1);

	printf("audio.c: Setting info:envp->sampleRate = %d\n", envp->sampleRate);

    printf("audio.c: before main loop! quit_flag = %d \n", gblGetQuit());
    while (!gblGetQuit()) {
        if(hAe1==NULL)
        {
			if(fromHDMI==1)
			{
				hdmiSampleRate = getHDMIsamplerate();
				if(hdmiSampleRate>96000)
				{
					if((hdmiSampleRate%32000)==0)
					{
						envp->sampleRate= 32000;
					}else //if((hdmiSampleRate%32000)==0)
					{
						envp->sampleRate= 44100;
					}					
				}
                else 
                    envp->sampleRate = hdmiSampleRate;
				if(envp->sampleRate==0||hdmiSampleRate==0)
				{
                    printf("audio.c samplerate %d env->samplerate %d,so waiting... \n",hdmiSampleRate,envp->sampleRate);
				    sleep(1);continue;
				}
				downsample = (hdmiSampleRate+envp->sampleRate/4)/envp->sampleRate;
			    if(downsample>1) printf("audio.c: hdmiSamplerate=%d , downsample = %d\n", hdmiSampleRate,downsample);
                
                if(envp->sampleRate>=44100&&envp->soundBitRate<96000)
                    envp->soundBitRate = 96000; //make sure it works well //linxj 2012-05-29
			}
			printf("audio.c: Get new value of envp->sampleRate = %d\n", envp->sampleRate);
            /*
			if (envp->soundBitRate == 128000) {
				params->bitRate = envp->soundBitRate;
				dynParams->bitRate = 96000;
			} else {
				params->bitRate = dynParams->bitRate = envp->soundBitRate;
			}*/
			params->bitRate = dynParams->bitRate = envp->soundBitRate;
			params->sampleRate = dynParams->sampleRate = envp->sampleRate;
#if 0
			printf("audio.c: params->sampleRate = %d\n", params->sampleRate);
			printf("audio.c: dynParams->sampleRate = %d\n", dynParams->sampleRate);
			printf("audio.c: params->bitRate = %d\n", params->bitRate);
			printf("audio.c: dynParams->bitRate = %d\n", dynParams->bitRate);
#endif 
			/* Create the audio encoder */
			hAe1 = Aenc2_create(hEngine, envp->audioEncoder, params, dynParams);
			if (hAe1 == NULL) {
				ERR("Failed to create audio encoder: %s\n", envp->audioEncoder);
				cleanup(THREAD_FAILURE);
			}

			/* Ask the codec how much space it needs for output data */
			hOutBuf = Buffer_create(Aenc1_getOutBufSize(hAe1), &bAttrs);

			/* Ask the codec how much input data it needs */
			hInBuf = Buffer_create(Aenc1_getInBufSize(hAe1), &bAttrs);
			//hInBuf = Buffer_create(Aenc1_getInBufSize(hAe1)*2, &bAttrs);//linxj 2012-04-07
		
			printf("audio.c: Aenc1_getInBufSize(hAe1) = %d\n", (int)Aenc1_getInBufSize(hAe1));
			printf("audio.c: Aenc1_getOutBufSize(hAe1) = %d\n", (int)Aenc1_getOutBufSize(hAe1));

			//if (hInBuf == NULL || hOutBuf == NULL || hEncInBuf == NULL) {
			if (hInBuf == NULL || hOutBuf == NULL) {
				ERR("Failed to allocate audio buffers\n");
				cleanup(THREAD_FAILURE);
			}
        }//end of hAe1

    	if(hSound == NULL)
        {

			/* Set the sample rate for the user interface */
			//gblSetSamplingFrequency(envp->sampleRate);

			/* Create the sound device */
			sAttrs.sampleRate = envp->sampleRate;
			//sAttrs.soundStd = Sound_Std_ALSA;
			//sAttrs.soundInput = envp->soundInput;
			//sAttrs.mode = Sound_Mode_INPUT;
			//sAttrs.leftGain = 160;
			//sAttrs.rightGain = 160;
			sAttrs.leftGain = envp->volume*2;
			sAttrs.rightGain = envp->volume*2;
			//sAttrs.channels = 1; //linxj 2012-04-07
			printf("sAttrs.rightGain = %d\n", (int)sAttrs.rightGain);
			hSound = Sound_create(&sAttrs);
			if (hSound == NULL) {
				ERR("Failed to create audio device\n");
				cleanup(THREAD_FAILURE);
			}
			if(fromHDMI==1)
			{
				system("./mreg -m 0x01d02008 0x02300030 \n"); //SPCR to reset mode
				system("./mreg -m 0x01d0200C 0x80A10040 \n"); //RCR two phase 0-16bit 1-32bit
				system("./mreg -m 0x01d02008 0x02f10031 \n"); //SPCR out of reset mode
			}
        }//end of hSound


        /* Pause processing? */
        //Pause_test(envp->hPauseProcess);
        
//        printf("audio.c: before Sound_read ! quit_flag = %d \n", gblGetQuit());
        /* Read samples from the Sound device */
        if (Sound_read(hSound, hInBuf) < 0) {
            ERR("Failed to read audio buffer\n");
            //cleanup(THREAD_FAILURE);
			readfaildtimes++;
        }else
		{
			readfaildtimes = 0;
		}
		readtimes++;
		if(readtimes>16||readfaildtimes>4)
		{
			if(fromHDMI==1)
			{
				int newsamplerate = getHDMIsamplerate();
				if(newsamplerate != hdmiSampleRate)
				{
					//release all    
					printf("audio.c samplerate change from %d to %d \n",hdmiSampleRate,newsamplerate);
		    		Aenc1_delete(hAe1);
					hAe1 = NULL;
					Buffer_delete(hInBuf);
					Buffer_delete(hOutBuf);
					hInBuf = hOutBuf = NULL;
					Sound_delete(hSound);
					hSound=NULL;
		            readtimes = 0;
		            readfaildtimes = 0;
		            continue ;
				}
			}
			if(readfaildtimes>4)
			{
                printf("audio.c : failed to read buffer too many times!! \n");
            	cleanup(THREAD_FAILURE);
			}
		}
//		printf("audio.c: after Sound_read ! quit_flag = %d \n", gblGetQuit());
#if 0
        if(envp->soundBitRate==128000&&readtimes>14)
        {
            memset(Buffer_getUserPtr(hInBuf),0,Buffer_getNumBytesUsed(hInBuf));
            printf("memset buf=0x%x,len=%d \n",Buffer_getUserPtr(hInBuf),Buffer_getNumBytesUsed(hInBuf));
        }
#endif 
#if 0
        if(writefailedtimes==0)
        {
        if ((ret = Sound_write(hSound, hInBuf)) < 0) {
                    printf("audio.c : Failed to write audio buffer ret = %d ,restart it\n",ret);
                    writefailedtimes++;
                    continue ;
                    Sound_delete(hSound);
					hSound=NULL;
                    if(writefailedtimes>2)
                    {
                printf("audio.c : failed to write buffer too many times!! \n");
            	cleanup(THREAD_FAILURE);
                    }
                    //return -1;
        }
        }
//        continue ;
        {
            //int usebytes=0;
            //usebytes = Buffer_getNumBytesUsed(hInBuf);
            //printf("hInbuf usebytes = %d \n",usebytes);
        }
#endif
        //printf("audio.c: before Aenc2_process! quit_flag = %d \n", gblGetQuit());
        /* Encode the audio buffer */
        if (Aenc2_process(hAe1, hInBuf, hOutBuf) < 0) {
            ERR("Failed to encode audio buffer\n");
            cleanup(THREAD_FAILURE);
        }
        //printf("audio.c: after Aenc2_process! quit_flag = %d \n", gblGetQuit());

#if 0
        /* Write encoded buffer to the speech file */
        if (Buffer_getNumBytesUsed(hOutBuf)) {
            if (fwrite(Buffer_getUserPtr(hOutBuf),
                       Buffer_getNumBytesUsed(hOutBuf), 1, outFile) != 1) {
                ERR("Error writing the encoded data to speech file.\n");
                cleanup(THREAD_FAILURE);
            }
        }
        else {
            printf("Warning, zero bytes audio encoded\n");
        }

#else
        //printf("audio.c: after Sound_read 1! quit_flag = %d \n", gblGetQuit());
		sem_wait(&vis_global.sem_protect);
		gettimeofday(&timeNow, NULL);
		tsparams.second = timeNow.tv_sec;
		tsparams.usecond = timeNow.tv_usec;
		tsparams.pBuffer_for_Access_Unit = (unsigned char*)Buffer_getUserPtr(hOutBuf);
		tsparams.flag = FLAG_SPEECH;
		tsparams.stream_Type = STREAM_TYPE_AAC;
		tsparams.length_of_Access_Unit = Buffer_getNumBytesUsed(hOutBuf);
		tsparams.tspackets = dest_ts;
		len = VisTS_package(&tsparams);
		tsdata = dest_ts;
        //printf("audio.c: after Sound_read 2! quit_flag = %d \n", gblGetQuit());
        do {
            int temp;
            if (len > SEND_LEN) {
                temp = SEND_LEN;
                len -= SEND_LEN;
            } else {
                temp = len;
                len = 0;
            }
            for(i=0; i<MAX_CONNECTION; i++) {
                if (conn_client.flag[i] == 1) {
                    bytesSent = sendto(vis_global.socket, tsdata, temp, 0,
                                       (struct sockaddr*)&conn_client.dest_addr[i],
                                       sizeof(struct sockaddr_in));
//					printf("<debug>:send len = %d\n", bytesSent);
                    if (bytesSent != temp) {
                        ERR("udp send error! bytesSent = %d, %s\n", bytesSent, strerror(errno));
                    }
                }
            }
            tsdata += temp;
        } while (len > 0);

        sem_post(&vis_global.sem_protect);
#endif
    }

cleanup:
    /* Make sure the other threads aren't waiting for us */
    Rendezvous_force(envp->hRendezvousInit);
    Pause_off(envp->hPauseProcess);

    /* Meet up with other threads before cleaning up */
    Rendezvous_meet(envp->hRendezvousCleanup);

    /* Clean up the thread before exiting */
    //ERR("cleanup before hAe1 \n");
    //sleep(2);
	if (dest_ts) {
		free(dest_ts);
		dest_ts = NULL;
	}

    if (hAe1) {
        Aenc1_delete(hAe1);
    }
    //sleep(2);
    //ERR("cleanup after hAe1 \n");

    if (hSound) {
        Sound_delete(hSound);
    }

    if (hInBuf) {
        Buffer_delete(hInBuf);
    }
    if (hEncInBuf) {
        Buffer_delete(hEncInBuf);
    }

    if (hOutBuf) {
        Buffer_delete(hOutBuf);
    }

    //printf("cleanup before hEngine \n");
    //sleep(2);
    if (hEngine) {
        Engine_close(hEngine);
    }
    //sleep(2);
    //ERR("cleanup after hEngine \n");

    if (outFile) {
        fclose(outFile);
    }

    return status;
}
