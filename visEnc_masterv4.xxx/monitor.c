#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

#include "main.h"
#include "../demo_common.h"
#include "slavestatus.h"
extern SlaveStatus slavestatus;

typedef struct _Info2Master{
    int     size;
    unsigned int     captureframe;   //capture frame number
    unsigned int     encodeframe;    //encode
    short            connectnum;     //connected number
    short            videobitrate;   //
    short            videofps;
    short            videoidrinterval;
}Info2Master;

#define GPIO_DEVICE	"/dev/dm365_gpio"
#define	GIO_SET_INPUT 3
#define GIO_OUTPUT 4
#define WATCHDOG
#define STATIC_CPUMEM

#define INFO2MASTER_NAME "./info2master"
int info2master = 0;
int readsize;
Info2Master readbuf[64];
Info2Master slaveinfo;

char slavestatusbuf[64*sizeof(SlaveStatus)];
char *slavestatusbufp = slavestatusbuf;
int readlen;

extern RunningStatus   devstatus;
static unsigned long GetCpuTotal(unsigned long* totalcpu,unsigned long* usedcpu);
static unsigned long GetMemTotal(unsigned long* totalmem,unsigned long* usedmem);
/******************************************************************************
 * monitorThrFxn
 ******************************************************************************/
void *monitorThrFxn(void *arg)
{
#ifdef WATCHDOG
	int				gpio_fd;	
	unsigned int	buf[2] = { 0 };
	unsigned int info[2];
    struct timeval tv1 = {0, 0}, tv2 = {0, 0};
#endif

#ifdef  STATIC_CPUMEM
    static unsigned int cputotal,cpuused,memtotal,memused;
#endif

    pthread_detach(pthread_self());

    if((info2master=open(INFO2MASTER_NAME,O_RDWR|O_NONBLOCK))==-1)
    {
        perror("open info2master at monitor.c");
    }

#ifdef WATCHDOG
	gpio_fd = open(GPIO_DEVICE, O_RDWR);
	if (gpio_fd == -1) {
        perror("monitor.c open");
	}
	buf[0] = 40;
	buf[1] = 1;
	ioctl(gpio_fd, GIO_OUTPUT, buf);

	buf[0] = 32;
	buf[1] = 1;
	ioctl(gpio_fd, GIO_OUTPUT, buf);	

	buf[0] = 32;
	buf[1] = 0;
	ioctl(gpio_fd, GIO_OUTPUT, buf);
#endif

    while (1) {
#ifdef WATCHDOG
		//feed the watchdog
		buf[0] = 32;
		buf[1] = 1;
		ioctl(gpio_fd, GIO_OUTPUT, buf);

		buf[0] = 32;
		buf[1] = 0;
		ioctl(gpio_fd, GIO_OUTPUT, buf);

        if (child_status == 0) {
            if (tv1.tv_sec == 0)
                gettimeofday(&tv1, NULL);
            gettimeofday(&tv2, NULL);
            if (tv2.tv_sec - tv1.tv_sec >= 10) {
                printf("monitor.c: child is not active\n");
                sleep(10);
            }
        } else if (child_status == 1) {
            tv1.tv_sec = 0;
        }

        //if create slave process failed too many times
        if (restart_times >= 10) {
            printf("monitor.c: child restart too many times\n");
            sleep(10);
        }
#endif

        sleep(1);
#ifdef  STATIC_CPUMEM
        if(GetCpuTotal(&cputotal,&cpuused))
        {
            unsigned int cput,cpuu;
            cput = cputotal - devstatus.cputotal;
            cpuu = cpuused  - devstatus.cpuused;
            if(cput>0&&cpuu>0)
            {
                devstatus.cpupercent = cpuu*1000/cput;
            }
            devstatus.cputotal = cputotal;
            devstatus.cpuused  = cpuused;
        }
        if(GetMemTotal(&memtotal,&memused))
        {
            if(memtotal>0&&memused>0)
            {
                devstatus.mempercent = memused*1000/memtotal;
            }
            devstatus.memtotal = memtotal;
            devstatus.memused  = memused;
        }
        //static info from slave
		readsize = read(info2master, &slaveinfo, sizeof(slaveinfo));
		if (readsize == sizeof(slaveinfo)) {
			devstatus.connecttednum = slaveinfo.connectnum;
//			printf("master: connect_count = %d\n", devstatus.connecttednum);
		} else {
			if (readsize!=0 && errno!=EAGAIN) {
				ERR("<w>read slaveinfo error");
			}
		}

/*
        if((readsize=read(info2master,readbuf,sizeof(readbuf)))>3)
        {
            slaveinfo = readbuf[readsize/sizeof(Info2Master) -1];
            devstatus.captureframe = slaveinfo.captureframe;
            devstatus.videoframe   = slaveinfo.encodeframe;
            devstatus.videobitrate = slaveinfo.videobitrate;
            devstatus.videofps     = slaveinfo.videofps;
            devstatus.videoidrinterval  = slaveinfo.videoidrinterval;
            devstatus.connecttednum   = slaveinfo.connectnum;
        }
		memset(slavestatusbuf,0,sizeof(slavestatusbuf));
		readlen = read(info2master,slavestatusbuf,sizeof(slavestatusbuf));
		slavestatusbufp = slavestatusbuf;
		if(readlen>0){
		  while(readlen >= 2*sizeof(int)){
			memcpy(info, slavestatusbufp, 2*sizeof(int));
			if(
			((info[0]==SlaveStatusType_Capture) && (info[1]==sizeof(CaptureStatus)))||
			((info[0]==SlaveStatusType_Video) && (info[1]==sizeof(VideoStatus)))||
			((info[0]==SlaveStatusType_Audio) && (info[1]==sizeof(AudioStatus)))||
			((info[0]==SlaveStatusType_Resize) && (info[1]==sizeof(ResizeStatus)))||
			((info[0]==SlaveStatusType_Writer) && (info[1]==sizeof(WriterStatus)))||
			((info[0]==SlaveStatusType_RszWriter) && (info[1]==sizeof(RszWriterStatus)))||
			((info[0]==SlaveStatusType_Network) && (info[1]==sizeof(NetworkStatus)))
			){
			switch (info[0]){
				case SlaveStatusType_Capture:
						memcpy(&slavestatus.capture, slavestatusbufp, info[1]);
						printf("read capture\n");
						readlen = readlen - info[1];
						slavestatusbufp = slavestatusbufp + info[1];
						break;
				case	SlaveStatusType_Video:	
						memcpy(&slavestatus.video, slavestatusbufp, info[1]);
						printf("read capture\n");
						readlen = readlen - info[1];
						slavestatusbufp = slavestatusbufp + info[1];
						break;
				case	SlaveStatusType_Audio:
						memcpy(&slavestatus.audio, slavestatusbufp, info[1]);
						printf("read capture\n");
						readlen = readlen - info[1];
						slavestatusbufp = slavestatusbufp + info[1];
						break;
				case	SlaveStatusType_Resize:

						memcpy(&slavestatus.resize, slavestatusbufp, info[1]);
						printf("read capture\n");
						readlen = readlen - info[1];
						slavestatusbufp = slavestatusbufp + info[1];
						break;
				case	SlaveStatusType_Writer:
						memcpy(&slavestatus.writer, slavestatusbufp, info[1]);
						printf("read capture\n");
						readlen = readlen - info[1];
						slavestatusbufp = slavestatusbufp + info[1];
						break;
				case	SlaveStatusType_RszWriter:
						memcpy(&slavestatus.rszwriter, slavestatusbufp, info[1]);
						printf("read capture\n");
						readlen = readlen - info[1];
						slavestatusbufp = slavestatusbufp + info[1];
						break;
				case	SlaveStatusType_Network:
						memcpy(&slavestatus.network, slavestatusbufp, info[1]);
						printf("read capture\n");
						readlen = readlen - info[1];
						slavestatusbufp = slavestatusbufp + info[1];
						break;
				default:
						break;
						}
			}else{
//				printf("slavestatus type size error\n");
				break;
			}
			}
	  }
*/
#endif
    }

    return 0;
}
static unsigned long GetCpuTotal(unsigned long* totalcpu,unsigned long* usedcpu) {
	FILE *fp = fopen("/proc/stat", "r");
	unsigned long total = 0;
	unsigned long cputime[11];
	int i = 0;
	if (NULL == fp) {
		perror("monitor.c:open [stat]file err");
        return 0;
	}
	fscanf(fp, "cpu  %lu", cputime+i);	
	for (i = 1 ; i < 10-1 ; ++i) {
		fscanf(fp, "%lu ", cputime+i);
	}
	fscanf(fp, "%lu\n", cputime+i);
	fclose(fp);

	for (i = 0 ; i < 10 ; ++i) {
//		printf("%lu  ", cputime[i]);
		total += cputime[i];
	}
//	cputime[10] = GetMemTotal();

//	printf("\ntotal = %lu\n", total);
//	printf("mem = %lu\n", cputime[10]);
	*totalcpu = total;
    *usedcpu = total - cputime[3]; //idle cpu 
	return total;
}

static unsigned long GetMemTotal(unsigned long* totalmem,unsigned long* usedmem) {
	FILE *fp_mem = fopen("/proc/meminfo", "r");
	unsigned long memtotal = 0,memfree;
	char memstr[16];
	int i = 0;
	
	if (NULL == fp_mem) {
		perror("monitor.c: open [meminfo]file err");
        return 0;
	}
    memset(memstr, 0, sizeof(memstr));
	while (memstr[i] = getc(fp_mem)) {
		if ('\n' == memstr[i]) {
			break;
		} else if (memstr[i]>='0' && memstr[i]<='9') {
			++i;
		} else {
			continue;
		}
	}
	memstr[i] = '\0';
	memtotal = atol(memstr);
    i=0;
	while (memstr[i] = getc(fp_mem)) {
		if ('\n' == memstr[i]) {
			break;
		} else if (memstr[i]>='0' && memstr[i]<='9') {
			++i;
		} else {
			continue;
		}
	}
	memstr[i] = '\0';
	memfree = atol(memstr);
	fclose(fp_mem);
    *totalmem= memtotal;
    *usedmem = memtotal - memfree;
	return memtotal; 
}
