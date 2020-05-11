#include <xdc/std.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <arpa/inet.h>

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
#if 0
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
    Void                 *status   = THREAD_SUCCESS;  
    //Capture_Handle        hCapture = NULL;
    //char	sendbuf[SEND_BUFLEN];
	//char    recvbuf[RECV_BUFLEN];
    struct in_addr	ipaddr;
    //char    *front_progress_argv[FPA_LEN + 1];
    int		listenfd, connfd = -1, filefd = -1;
    I2cData i2c_data;
    int     num=0,i,err;
    struct sockaddr_in cliaddr;
    socklen_t cliaddr_len;
    //int     fifofd;
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

	//while(1)	{
    while (!gblGetQuit()) {
        sleep(1);
        continue ;
#if 0
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
                    //case VIS_RQ_SETI2CDATA:
                    //    break;
                    default:
                        break;
                    }//end of case
                }//end of if header 
            }
            close(connfd);
        }//end of connected
#endif
	}
cleanup:
    printf(" dynamic: clean up!\n");
    Rendezvous_force(envp->hRendezvousInit);
    
    /* Meet up with other threads before cleaning up */
    Rendezvous_meet(envp->hRendezvousCleanup);
	if (listenfd !=-1) {
		close(listenfd);
	} 
    return status;
}

#endif

