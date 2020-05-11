/*
 * writer.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/vfs.h>
#include <sys/time.h>
#include <sys/mount.h>

#include <xdc/std.h>
#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/BufTab.h>
#include <ti/sdo/dmai/BufferGfx.h>
#include <ti/sdo/dmai/Rendezvous.h>

#include "writer.h"
#include "../demo.h"
#include "../ctrl.h"
#include "vis_common.h"

#define GPIO_DEVICE "/dev/dm365_gpio"
#define GIO_SET_INPUT 3
#define GIO_OUTPUT 4
#define GIO_GET 2
#define USEC 1000000

/* Number of buffers in writer pipe */
//#define NUM_WRITER_BUFS         9
#define NUM_WRITER_BUFS         6
#define FIFONAME   "./fifo"

//#define SAVE_TSDATA	//linxj 2011-08 for customer
#ifdef SAVE_TSDATA
#define USB_DISK_DEV_PATH	"/dev/sda1"
#define	USB_DISK_MOUNT_PATH	"/var/usbmedia/"
#define SAVE_ENABLE_BUTTON	//for zhuhaiyangheng, ls 2013-03-28
#endif

extern int encodedFrameType;
extern unsigned char idrhead[IDRHEAD_LEN];
extern DynamicParams dynamic_params;
static int getDiskspace(char *dirname)//Mbyte
{
    unsigned int freeMByte=0,blocksize;  
    struct statfs diskInfo;
    
    statfs(dirname,&diskInfo);
    blocksize = diskInfo.f_bsize;// 每个block里面包含的字节数
    //totalsize = blocksize * diskInfo.f_blocks;//总的字节数
    //  printf("TOTAL_SIZE == %lu MB/n",totalsize>>20); // 1024*1024 =1MB  换算成MB单位

    freeMByte = (diskInfo.f_bfree*blocksize)>>20; //再计算下剩余的空间大小
    //printf("DISK_FREE == %d MB \n",freeMByte);
    return freeMByte;
}
int checkDiskspace(char *dirname,int requiresize)//size: MByte
{
    struct dirent *ptr;
    DIR *dir;
    unsigned int freeMByte;//retval=0;
    int  fileid[4]={100000,100000,100000,100000};
    char filename[4][64];
    int  getfile=0;

    freeMByte = getDiskspace(dirname);
    
    dir = opendir(dirname);
    while((ptr=readdir(dir))!=NULL)
    {
        int theid=0,tmp;
        //printf("%s is ready \n",ptr->d_name);
        tmp=sscanf(ptr->d_name,"record_%d.mp4",&theid);
        if(tmp==1)
        {
            //printf("file[%s] is ready ID=%d \n",ptr->d_name,theid);
            if(theid<fileid[0])
            {
                fileid[0]=theid;
                sprintf(filename[0],"/var/usbmedia/%s",ptr->d_name);
                getfile = 1;
            }
        }
    }
    closedir(dir);
    requiresize+=10;    //give more space 10Mbyte
    if(requiresize>=freeMByte)
    {
        if (getfile==1) {
			char cmd[80];
			int tmp;
			sprintf(cmd,"rm %s",filename[0]);
			system(cmd);
			printf("remove file[%s] \n",filename[0]);
			tmp = getDiskspace(dirname);
			printf("size from %d to %d MByte\n",freeMByte,tmp);
			freeMByte = tmp;
        } else {
            printf("Need to get space but no file removed!! \n");
        }
    }
    return freeMByte - requiresize;
}
/******************************************************************************
 * writerThrFxn
 ******************************************************************************/
Void *writerThrFxn(Void *arg)
{
    WriterEnv          *envp            = (WriterEnv *) arg;
    VisTS_params 		tsparams 		= (VisTS_params)(envp->tsparams);
    Void               *status          = THREAD_SUCCESS;
    Buffer_Attrs        bAttrs          = Buffer_Attrs_DEFAULT;
    BufTab_Handle       hBufTab         = NULL;
    Buffer_Handle       hOutBuf;
    Int                 fifoRet;
    Int                 bufIdx;

    struct timeval	timeNow;
    unsigned char   *dest_ts = NULL;
    unsigned char   *tsdata;
    unsigned char   *ptr;
    int			    bytesSent, len, i;
	int             temp, dest_tssize, send_times = 0;
    FILE            *outFile = NULL;
    int             fifofd;

#ifdef SAVE_TSDATA
    char            tsfilename[64];
	char			snfilename[32] = {0, };
	FILE			*idFile=NULL, *tsFile=NULL;
    unsigned int    buf[2]={0},recordSize=0,maxFileSize=0;
	int             gpio_fd,usbonline=0,usbwritable=0,fileID,spaceEnough,ret;
#endif

#ifdef SAVE_ENABLE_BUTTON
	int				buf_saveEnable[2] = {0, 0};		//get gpio addr 87
	int				button_down=0, button_up=0, gpio87_status_pre=1, saveEnable=~0;
#endif

#ifdef SLEEP_CUSTOMER_
	struct timeval tv1={0,0}, tv2={0,0};
	unsigned int	fix_times = 0;
	unsigned int	times = 1000000/envp->videoFrameRate*1000/INTERVAL_PER_SEND;
	printf("writer.c: times = %u\n", times);
	unsigned int	sleep_time = 0;
//	unsigned int	interval_time = 1000000/envp->videoFrameRate*1000;
#endif

    if ((fifofd = open(FIFONAME, O_RDONLY | O_NONBLOCK)) == -1) {
        perror("open fifo");
    }

    /*
     * Create a table of buffers for communicating buffers to
     * and from the video thread.
     */
    hBufTab = BufTab_create(NUM_WRITER_BUFS, envp->outBufSize, &bAttrs);

    if (hBufTab == NULL) {
        ERR("Failed to allocate contiguous buffers\n");
        cleanup(THREAD_FAILURE);
    }
    //printf("write.c: outbufsize = %d\n", (int)envp->outBufSize);

    /* Send all buffers to the video thread to be filled with encoded data */
    for (bufIdx = 0; bufIdx < NUM_WRITER_BUFS; bufIdx++) {
        if (Fifo_put(envp->hOutFifo, BufTab_getBuf(hBufTab, bufIdx)) < 0) {
            ERR("Failed to send buffer to display thread\n");
            cleanup(THREAD_FAILURE);
        }
    }

    dest_ts = malloc(1920 * 1088 * 2);
    //dest_ts = malloc(VGA_WIDTH * VGA_HEIGHT * 2);   //shenpei 2009-11-11
    if (dest_ts == NULL) {
		ERR("failed to malloc dest_ts buffer!\n");
		exit(-1);
    }

    /* Signal that initialization is done and wait for other threads */
    Rendezvous_meet(envp->hRendezvousInit);

#ifdef  SAVE_TSDATA
#if 0
    while (!gblGetQuit()) {
        ret = mount(USB_DISK_DEV_PATH, USB_DISK_MOUNT_PATH, "vfat", 0, NULL);
//      ret = system("mount /dev/sda1 /var/usbmedia/");
        if (0 == ret) {
            printf("writer.c:\"%s to %s mount\" success\n", USB_DISK_DEV_PATH, USB_DISK_MOUNT_PATH);
            break;
        }
        if (EBUSY == errno) {
            printf("writer.c:\"%s to %s\" already mounted\n", USB_DISK_DEV_PATH, USB_DISK_MOUNT_PATH);
            break;
        }
//      perror("<w>writer.c: usb dev offline, waiting...");
        sleep(5);
    }
    usbonline=1;
#endif
#if 1
	sleep(5);
	ret = mount(USB_DISK_DEV_PATH, USB_DISK_MOUNT_PATH, "vfat", 0, NULL);
//	ret = system("mount /dev/sda1 /var/usbmedia/");
	if (0 == ret) {
		printf("writer.c:\"%s to %s mount\" success\n", USB_DISK_DEV_PATH, USB_DISK_MOUNT_PATH);
		usbonline = 1;
	} else {
		if (EBUSY == errno) {
			printf("writer.c:\"%s to %s\" already mounted\n", USB_DISK_DEV_PATH, USB_DISK_MOUNT_PATH);
			usbonline = 1;
		}
		else {
			printf("<w>writer.c:\"%s to %s mount\" failed\n", USB_DISK_DEV_PATH, USB_DISK_MOUNT_PATH);
			usbonline = 0;
		}
	}
#else
    sleep(5);
    temp=system("mount /dev/sda1 /var/usbmedia/ -t vfat");	//ls 2013-04-20
    //temp=system("mount /dev/sdb1 /tmp -t ntfs");
//    sleep(3);
    printf("after mount, return=%x\n",temp);

    temp = (temp&0xff00)>>8;
    //if(temp==0||temp==0x20) usbonline=1;//no error or online already
    if(temp==0) usbonline=1;//no error or online already
#endif
    fileID=0;recordSize=0;maxFileSize=1024*1024*100;	//100M one file
	sprintf(snfilename, "%sSN.txt", USB_DISK_MOUNT_PATH);
    if(usbonline)
    {
        idFile = fopen(snfilename,"r");
        if(idFile) 
        {
            fscanf(idFile,"file id=%d",&fileID);
            fclose(idFile);
        }
    
        idFile = fopen(snfilename,"w");
        if(idFile)
        {
            fprintf(idFile,"file id=%d",fileID+1);
            fclose(idFile);
            usbwritable=1;
        }else
        {
            printf("Can't create id file!!!! \n");
        }
        idFile = NULL;

        if(usbwritable)//判断是否可写，当idFile创建成功时usbwritable=1
        {
			spaceEnough = 0;
			for(temp=0;temp<10;temp++)
			{
				spaceEnough = checkDiskspace(USB_DISK_MOUNT_PATH,maxFileSize>>20); //以MByte为单位,不够的话每次会删除一个文件
				if(spaceEnough>0) break ;
			}

			if(spaceEnough>0)
			{
				memset(tsfilename, 0, sizeof(tsfilename));
				sprintf(tsfilename,"%srecord_%05d.mp4", USB_DISK_MOUNT_PATH, fileID);
				tsFile = fopen(tsfilename, "w");
				printf("write a new file[%s] tsfile=%x \n",tsfilename,(Int)tsFile);
			}else
			{
				printf("Disk space is not enough !!!!!!\n");
			}
        }
    }
    gpio_fd = open(GPIO_DEVICE, O_RDWR);
    buf[0] = 91;    //91:the upper light,yellow one GPIO
    buf[1] = usbonline;
    ioctl(gpio_fd, GIO_OUTPUT, buf);
    buf[1] = 0;
#endif
    printf("write.c: before main loop! quit_flag = %d \n", gblGetQuit());
    while (!gblGetQuit()) {
        if (read(fifofd, &dynamic_params, sizeof(dynamic_params)) == sizeof(dynamic_params)) {
            printf("write.c: dynamic_params.iframe = %d\n", dynamic_params.iframe);
            if (dynamic_params.port != -1) {
                printf("write.c: dynamic_params.ip = %s\n", dynamic_params.ip);
                printf("write.c: dynamic_params.port = %d\n", dynamic_params.port);
                close(vis_global.socket);
                memset(&vis_global.ipAddr, 0,ADDRSTRLEN);
                memcpy(&vis_global.ipAddr, dynamic_params.ip, strlen(dynamic_params.ip));
                vis_global.port = dynamic_params.port;
                vis_global.socket = socketInit(&vis_global);
            }
        }
        if (!envp->videoEnable) {
            sleep(1);
            continue;
        }

        /* Get an encoded buffer from the video thread */
        fifoRet = Fifo_get(envp->hInFifo, &hOutBuf);

        if (fifoRet < 0) {
            ERR("Failed to get buffer from video thread\n");
            cleanup(THREAD_FAILURE);
        }

        /* Did the video thread flush the fifo? */
        if (fifoRet == Dmai_EFLUSH) {
            cleanup(THREAD_SUCCESS);
        }

        /* Store the encoded frame to disk */
        if (Buffer_getNumBytesUsed(hOutBuf)) {
#if 0
            if (fwrite(Buffer_getUserPtr(hOutBuf),
                       Buffer_getNumBytesUsed(hOutBuf), 1, outFile) != 1) {
                ERR("Error writing the encoded data to video file\n");
                cleanup(THREAD_FAILURE);
            }
#else
			sem_wait(&vis_global.sem_protect);
			gettimeofday(&timeNow, NULL);
			tsparams.second = timeNow.tv_sec;
			tsparams.usecond = timeNow.tv_usec;
#if 1
            if (encodedFrameType == 0) {
                encodedFrameType = 1;
                ptr = (unsigned char *)Buffer_getUserPtr(hOutBuf)-sizeof(idrhead);
                memcpy(ptr, idrhead, sizeof(idrhead));
                tsparams.pBuffer_for_Access_Unit = ptr;
			    tsparams.length_of_Access_Unit = Buffer_getNumBytesUsed(hOutBuf)+sizeof(idrhead);
                //tsparams.pBuffer_for_Access_Unit = (unsigned char *)Buffer_getUserPtr(hOutBuf);
			    //tsparams.length_of_Access_Unit = Buffer_getNumBytesUsed(hOutBuf);
			    tsparams.flag = FLAG_VIDEO_IDR;
            } else {
                tsparams.pBuffer_for_Access_Unit = (unsigned char *)Buffer_getUserPtr(hOutBuf);
			    tsparams.length_of_Access_Unit = Buffer_getNumBytesUsed(hOutBuf);
			    tsparams.flag = FLAG_VIDEO;
            }
#endif
			tsparams.stream_Type = STREAM_TYPE_H264;
			tsparams.tspackets = dest_ts;
			len=VisTS_package(&tsparams);
			tsdata = dest_ts;
            dest_tssize = len;
#ifdef SLEEP_CUSTOMER_
			fix_times = ((len/TS_LENGTH/TS_NUM)+1)/times + 1;
//			printf("writer.c: fix_times = %u, len=%d\n", fix_times, len);
#endif
			do {
#if 1	//if not sleep, the client will recv uncatchable, image will be broken
                if (send_times > 100) {
                    usleep(100);
                }
#endif
#ifdef SLEEP_CUSTOMER_
				if (send_times >= fix_times) {
#if 1
					gettimeofday(&tv2, NULL);
					sleep_time = INTERVAL_PER_SEND-((tv2.tv_sec-tv1.tv_sec)*1000000+tv2.tv_usec-tv1.tv_usec);
					if (sleep_time > INTERVAL_PER_SEND) sleep_time = 0;
//					printf("sleep_time = %u\n", sleep_time);
					usleep(sleep_time);
#endif
					send_times = 0;
				}
#endif
				if (len > SEND_LEN) {
				    temp = SEND_LEN;
				    len -= SEND_LEN;
				} else {
				    temp = len;
				    len = 0;
				}
#ifdef SLEEP_CUSTOMER_
				gettimeofday(&tv1, NULL);
#endif
                for(i=0; i<MAX_CONNECTION; i++) {
                    if (conn_client.flag[i] == 1) {
                        bytesSent = sendto(vis_global.socket, tsdata, temp, 0,
                                           (struct sockaddr*)&conn_client.dest_addr[i],
                                           sizeof(struct sockaddr_in));
                        //printf("write.c: bytesSent = %d\n", bytesSent);
                        if (bytesSent != temp) {
                            ERR("udp send error! bytesSent = %d, %s\n", bytesSent, strerror(errno));
                            usleep(100000);
                            break;
                        }
                    }
                }
				tsdata += temp;
                send_times++;
			} while (len > 0);
#ifdef  SAVE_TSDATA
            if(usbonline&&usbwritable)
            {
#ifdef SAVE_ENABLE_BUTTON
				buf_saveEnable[0] = 87;
				ioctl(gpio_fd, GIO_GET, buf_saveEnable);
		//		printf("saveEnable = %d, addr = %d\n", buf_saveEnable[1], buf_saveEnable[0]);
				if (gpio87_status_pre==1 && buf_saveEnable[0]==0) button_down = 1;
				if (gpio87_status_pre==0 && buf_saveEnable[0]==1) button_up   = 1;
				gpio87_status_pre = buf_saveEnable[0];
				if (button_down && button_up) {
					saveEnable = ~saveEnable;
					button_down = button_up = 0;
					printf("saveEnable=%d, tsFile=%p\n", saveEnable, tsFile);
				}
				if (!saveEnable && tsFile) {	//when save enable change to disable
					fclose(tsFile);
					tsFile = NULL;
				}
				if (saveEnable && !tsFile) {	//when save disable change to enable
					idFile = fopen(snfilename,"r");
					if(idFile) 
					{
						fscanf(idFile,"file id=%d",&fileID);
						fclose(idFile);
					}
				
					idFile = fopen(snfilename,"w");
					if(idFile)
					{
						fprintf(idFile,"file id=%d",fileID+1);
						fclose(idFile);
						usbwritable=1;
					}else
					{
						printf("Can't create id file!!!! \n");
					}
					idFile = NULL;

					if(usbwritable)//判断是否可写，当idFile创建成功时usbwritable=1
					{
						spaceEnough = 0;
						for(temp=0;temp<10;temp++)
						{
							spaceEnough = checkDiskspace(USB_DISK_MOUNT_PATH,maxFileSize>>20); //以MByte为单位,不够的话每次会删除一个文件
							if(spaceEnough>0) break ;
						}

						if(spaceEnough>0)
						{
							memset(tsfilename, 0, sizeof(tsfilename));
							sprintf(tsfilename,"%srecord_%05d.mp4", USB_DISK_MOUNT_PATH, fileID);
							tsFile = fopen(tsfilename, "w");
							printf("write a new file[%s] tsfile=%x \n",tsfilename,(Int)tsFile);
						}else
						{
							printf("Disk space is not enough !!!!!!\n");
						}
					}
				}
#endif

//				printf("filesize=%u, max=%u, recordSize>maxFileSize_%d, maxFilesize>0_%d, tsparams.flage==FLAG_VIDEO_IDR_%d\n", recordSize, maxFileSize, recordSize>maxFileSize, maxFileSize>0, tsparams.flag==FLAG_VIDEO_IDR);
				if(recordSize>maxFileSize&&maxFileSize>0&&tsparams.flag==FLAG_VIDEO_IDR)
				{
					if(tsFile) {
						fclose(tsFile);  //close it
					}
					fileID++;
					idFile = fopen(snfilename,"w");
					if(idFile)
					{
						fprintf(idFile,"file id=%d",fileID+1);
						fclose(idFile);
					}else
					{
						printf("crate id file faild!!! \n"); 
						usbwritable = 0;//若写入失败，认为usb不可写
					}
					idFile = NULL;
					spaceEnough = 0;
					for(temp=0;temp<10;temp++)
					{
						spaceEnough = checkDiskspace(USB_DISK_MOUNT_PATH, maxFileSize>>20); //以MByte为单位,不够的话每次会删除一个文件
						if(spaceEnough>0) break ;
					}

					tsFile = NULL;
					if(spaceEnough>0)
					{
						memset(tsfilename, 0, sizeof(tsfilename));
						sprintf(tsfilename,"%srecord_%05d.mp4", USB_DISK_MOUNT_PATH, fileID);
						tsFile = fopen(tsfilename, "w");
						printf("write a new file[%s] tsfile=%x \n",tsfilename,(Int)tsFile);
					}else
					{
						printf("Disk space is not enough !!!!!!!!\n");
					}
					recordSize = 0;
				}
				//printf("tsfile=%x,tssize=%d \n",(Int)tsFile,dest_tssize);
				if(tsFile&&dest_tssize>0)
				{
					int writeval;
					writeval=fwrite(dest_ts,1,dest_tssize,tsFile);
					//printf("writeval=%d \n",writeval);
					if(writeval>0)
						recordSize+=dest_tssize;
					else
						printf("Error:write file return=%d \n",writeval);
					buf[1] = ~buf[1];
					ioctl(gpio_fd, GIO_OUTPUT, buf);
				}
//				printf("recordSize = %u\n", recordSize);
            }//end if(usbonline)
#endif
			sem_post(&vis_global.sem_protect);
//            printf("%d\n", send_times);
            send_times = 0;
#endif
        }
        else {
            printf("Warning, writer received 0 byte encoded frame\n");
        }

        /* Return buffer to capture thread */
        if (Fifo_put(envp->hOutFifo, hOutBuf) < 0) {
            ERR("Failed to send buffer to display thread\n");
            cleanup(THREAD_FAILURE);
        }
    }

cleanup:
    /* Make sure the other threads aren't waiting for us */
    printf("write.c: cleanup \n");
    Rendezvous_force(envp->hRendezvousInit);
    Pause_off(envp->hPauseProcess);
    Fifo_flush(envp->hOutFifo);

    /* Meet up with other threads before cleaning up */
    Rendezvous_meet(envp->hRendezvousCleanup);

    /* Clean up the thread before exiting */
    if (outFile) {
		fclose(outFile);
		outFile = NULL;
    }

#ifdef  SAVE_TSDATA
    if (idFile) {
		fclose(idFile);
		idFile = NULL;
    }

    if (tsFile) {
		fclose(tsFile);
		tsFile = NULL;
    }

    //if(usbonline)
    {
        int retval;
        usleep(2500000);    //2.5s
        retval=system("umount /var/usbmedia/");
        printf("after umount return=%d \n",retval);
    }
#endif

    if (dest_ts) {
		free(dest_ts);
    }

    return status;
}
