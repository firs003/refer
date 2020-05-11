#include "vis_mmnp.h"

#include "c1.h"
#include "ringbuffer.h"

typedef struct common_args Common_Args;

typedef struct c1_service_env {
	struct common_args *common;
	int index;
	int heart;	//-1 means this is not a cmdsock, heart beat is invalid
	unsigned int board_id;
	pthread_t service_tid;
	unsigned int channel_server;	//lower 16 bits is channel enable, higher 16 bit is stream_type
	unsigned int channel_client;	//begin with 1
	SOCKET cmdsock;
	SOCKET datasock;
	SOCKET othersock;
} C1_Service_Env;

struct common_args {
	unsigned int *global_board_index;
	struct c1_service_env *env_array;		//table start
//	Vis_Mmnp_Handle *handle_ptr;
	RingBuffer *pbuffer;
	RingBuffer *pbuffer_rsz;
	volatile int *quit_flag_ptr;
	pthread_mutex_t *mutex;
//	pthread_cond_t *cond;
	unsigned int *conn_num_ptr;
	pthread_mutex_t *conn_num_mutex;
	int *max_conn_num;
};


/******************************************************************************
 * Static Functions Declaration
 ******************************************************************************/
static void *serviceThrFxn(void *arg);
static void *senddata(void *arg);
//static int serial_init(int baudrate);
static int AckUserLogin(SOCKET cmdsock, unsigned char *recvbuf, Head *phead, unsigned int board_id);
static int AckSystemInfo(SOCKET cmdsock, Head *phead);


/******************************************************************************
 * c1ThrFxn
 ******************************************************************************/
void *vmn_c1ThrFxn(void *arg)
{
    C1Env		*envp           = NULL;
    void 		*status         = THREAD_SUCCESS;
    SOCKET		listenfd = -1, socktmp = -1;
    int			opt = 1, maxq, i;
    socklen_t	cliaddr_len=sizeof(struct sockaddr_in), socklen=sizeof(struct sockaddr_in);
    struct sockaddr_in servaddr, cliaddr;
    int			buflen;
    struct c1_service_env *service_env_array  = NULL;		//lengshan 2013-06-03
	unsigned short port;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int cleanup_flag = 0;
	unsigned int global_board_index = 0;	//ls 2013-0-17 for test
	char str[INET_ADDRSTRLEN] = {0, };
	struct common_args common;
	
#if 0
	/* Init Serial Port for Pantz Control */
	if (-1 == serial_init(2400)) {
        cleanup(THREAD_FAILURE);
	}
	printf("<debug>:init serial port success\n");
#endif
    
    envp = (C1Env *)calloc(1, sizeof(C1Env));
	if (envp == NULL) {
		Vmn_Log_Error("alloc memery for C1Env failed");
		envp = (C1Env *) arg;			//linxj, crit when arg is NULL, TODO
		pthread_mutex_lock(envp->mutex_initsync);
		pthread_cond_broadcast(envp->cond_initsync);
		pthread_mutex_unlock(envp->mutex_initsync);
		*envp->quit_flag_ptr = 1;
		envp = (C1Env *) 0;			//linxj make sure no free @ cleanup
		cleanup(THREAD_FAILURE);
	}
	memcpy(envp, arg, sizeof(C1Env));
	cleanup_flag = 1;
	
	if (pthread_mutex_init(&mutex, NULL) != 0) {
		Vmn_Log_Error("init mutex for c1sender failed");
		pthread_mutex_lock(envp->mutex_initsync);
		pthread_cond_broadcast(envp->cond_initsync);
		pthread_mutex_unlock(envp->mutex_initsync);
		cleanup(THREAD_FAILURE);
	}
	cleanup_flag = 2;
	if (pthread_cond_init(&cond, NULL) != 0) {
		Vmn_Log_Error("init mutex for c1sender failed");
		pthread_mutex_lock(envp->mutex_initsync);
		pthread_cond_broadcast(envp->cond_initsync);
		pthread_mutex_unlock(envp->mutex_initsync);
		cleanup(THREAD_FAILURE);
	}
	cleanup_flag = 3;

#ifndef	WIN32
    signal(SIGPIPE, SIG_IGN);
#endif

	port = envp->port;
	//if (port<=1024) port = 36511;
	if (port<=1024)  port = 1234; //linxj
//	(envp->port<=1024) ? port=1234: port=envp->port;
//	(envp->maxConnNum>MAXQ||envp->maxConnNum==0) ? maxq=MAXQ: maxq=envp->maxConnNum;
	maxq = MAXQ;
	service_env_array = (struct c1_service_env *)calloc(maxq*3, sizeof(struct c1_service_env));	//2 more mem is fit for last datasock and othersock
	if (service_env_array == NULL) {
		Vmn_Log_Error("alloc memery for service_env_array failed");
		pthread_mutex_lock(envp->mutex_initsync);
		pthread_cond_broadcast(envp->cond_initsync);
		pthread_mutex_unlock(envp->mutex_initsync);
		cleanup(THREAD_FAILURE);
	}
	cleanup_flag = 4;
	for (i=0; i<maxq; ++i) {
		service_env_array[i].cmdsock = -1;
		service_env_array[i].datasock = -1;
		service_env_array[i].othersock = -1;
	}
	
	memset(&common, 0, sizeof(struct common_args));
	common.global_board_index = &global_board_index;
	common.env_array = service_env_array;
//	common.handle_ptr = envp->handle_ptr;
	common.pbuffer = envp->pbuffer;
	common.quit_flag_ptr = envp->quit_flag_ptr;
	common.mutex = &mutex;
	common.conn_num_ptr = envp->conn_num_ptr;
	common.conn_num_mutex = envp->conn_num_mutex;
	common.max_conn_num = &maxq;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        Vmn_Log_Error("socket");
		pthread_mutex_lock(envp->mutex_initsync);
		pthread_cond_broadcast(envp->cond_initsync);
		pthread_mutex_unlock(envp->mutex_initsync);
        cleanup(THREAD_FAILURE);
    }
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
//	setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
//	setsockopt(listenfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    socklen = sizeof(buflen);
    getsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, &buflen, &socklen);
    buflen *= 2;    //sp 01-06-10 double sndbuf of tcp 
    setsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, &buflen, sizeof(buflen));
    socklen = sizeof(buflen);
    getsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, &buflen, &socklen);
    Vmn_Log_Info("buflen=%d, port=%hu\n", buflen, port);

	memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        Vmn_Log_Error("bind failed");
		pthread_mutex_lock(envp->mutex_initsync);
		pthread_cond_broadcast(envp->cond_initsync);
		pthread_mutex_unlock(envp->mutex_initsync);
        cleanup(THREAD_FAILURE);
    }

    listen(listenfd, maxq*3);	//connection contains cmdsock datasock and othersock, maxq should be 3 times

    /* Signal that initialization is done and wait for other threads */
	Vmn_Log_Debug("2.listen before cond_broadcast\n");
	pthread_mutex_lock(envp->mutex_initsync);
	pthread_cond_broadcast(envp->cond_initsync);
	pthread_mutex_unlock(envp->mutex_initsync);
	Vmn_Log_Debug("3.listen after cond_broadcast\n");

	Vmn_Log_Debug("quit_flag[%p]=%d\n", envp->quit_flag_ptr, *envp->quit_flag_ptr);
    Vmn_Log_Info("c1.c: Accepting connections...\n");
    while(envp->quit_flag_ptr && !(*envp->quit_flag_ptr)) {
		fd_set read_fds;
		int ret_select = -1;
		struct timeval tv = {2, 0};
		/* select will change fds_set and tv params
		 * when select return timeout, tv=0, we must reset fd_sets and timeout; */
		FD_ZERO(&read_fds);
		FD_SET(listenfd, &read_fds);
		ret_select = select(listenfd+1, &read_fds, NULL, NULL, &tv);
		if (ret_select!=1 || !FD_ISSET(listenfd, &read_fds)) {
//			Vmn_Log_Debug("Not expect, ret_select=%d, FD_ISSET=%d\n", ret_select, FD_ISSET(listenfd, &read_fds));
			continue;
		}
//		Vmn_Log_Debug("before accept\n");
		socktmp = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
//		Vmn_Log_Debug("accept return %d\n", socktmp);
		if (socktmp != -1) {	//SOCKET in windows is unsigned, >=0 forever
			pthread_mutex_lock(&mutex);
			for (i=0; i<maxq*3; i++) {	//TODO, new cmd sock is unexcept
				if ((service_env_array+i)->cmdsock == -1)
					break;
			}

			if (i == maxq*3) {
				Vmn_Log_Warning("c1.c: already %d connections\n", maxq);
				closesocket(socktmp);
				Sleep(1000);
				continue;
			}
#ifdef	WIN32
            strcpy(str,inet_ntoa(cliaddr.sin_addr));
#else
            inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str));
#endif
			
//			setsockopt((service_env_array+i)->sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#if 0
			(service_env_array+i)->global_board_index = &global_board_index;
			(service_env_array+i)->env_array = service_env_array;
//			(service_env_array+i)->handle_ptr = envp->handle_ptr;
			(service_env_array+i)->pbuffer = envp->pbuffer;
			(service_env_array+i)->pbuffer = envp->pbuffer_rsz;
			(service_env_array+i)->quit_flag_ptr = envp->quit_flag_ptr;
			(service_env_array+i)->mutex = &mutex;
//			(service_env_array+i)->cond = &cond;
			(service_env_array+i)->conn_num_ptr = envp->conn_num_ptr;
			(service_env_array+i)->max_conn_num = maxq;
#endif
			(service_env_array+i)->common = &common;
			(service_env_array+i)->index = i;
			(service_env_array+i)->cmdsock = socktmp;
//			(service_env_array+i)->board_id = ;
//			(service_env_array+i)->service_tid = ;
//			(service_env_array+i)->sendsock = ;
            if (pthread_create(&(service_env_array+i)->service_tid, NULL, serviceThrFxn, (void *)(service_env_array+i)) != 0) {
                Vmn_Log_Error("c1.c: Failed to create service thread");
				pthread_mutex_lock(&mutex);
				if((service_env_array+i)->cmdsock != -1) {
					closesocket((service_env_array+i)->cmdsock);
					(service_env_array+i)->cmdsock = -1;
					socktmp = -1;
				}
				pthread_mutex_unlock(&mutex);
                continue;
            }
			pthread_mutex_lock(envp->conn_num_mutex);
			++(*envp->conn_num_ptr);
			pthread_mutex_unlock(envp->conn_num_mutex);
			Vmn_Log_Info("c1.c:Received from %s:%hu, service_env_array[%d].cmdsock=%d, <conn_num=%u>\n", str, ntohs(cliaddr.sin_port), i, (service_env_array+i)->cmdsock, *envp->conn_num_ptr);
			pthread_mutex_unlock(&mutex);

		} else {
			perror("accept failed");
			if (errno != EAGAIN && errno != 0) {	//linxj
				Vmn_Log_Error("accept failed errno=%d ",errno);
			}
		}
	}

cleanup:
	Vmn_Log_Debug("C1 Listen Thread Cleanup\n");
	if (envp) *envp->quit_flag_ptr = 1;
	
	if (listenfd != -1) {
		closesocket(listenfd);
		listenfd = -1;
	}
	
	//TODO, close serial fd
	switch (cleanup_flag) {
		case 4 :
#if 0
			pthread_mutex_lock(&mutex);
			if (service_env_array) {
				for (i=0; i<maxq; i++) {
					if (service_env_array[i].sock != -1) {
						printf("info   :socket close=%d, c1 module exit...OK!!\n", (service_env_array+i)->cmdsock);
						closesocket((service_env_array+i)->cmdsock);
					}
				}
				free(service_env_array);
				service_env_array = NULL;
			}
			pthread_mutex_unlock(&mutex);
#endif
			if (service_env_array) {
				free(service_env_array);
				service_env_array = NULL;
			}
		case 3 :
			pthread_cond_destroy(&cond);
		case 2 :
			pthread_mutex_destroy(&mutex);
		case 1 :
			if (envp) {
				free(envp);
				envp = NULL;
			}
		default :
			break;
	}

	Vmn_Log_Debug("Listen Thread Return\n");
    return status;
}


static void *serviceThrFxn(void *arg) {
	struct c1_service_env *envp = (struct c1_service_env *)arg;
	void *status = THREAD_SUCCESS;
	int index = -1;
//	Sockmng* psockmanager = &conn_server.sockmanager[index];
	unsigned char recvbuf[CMD_RECVBUF_LEN*4]={0x00, };		//bigger is better
	Head head,sendhead;
	int i=0, recvlen, sock_type=0;
	struct timeval tv; /* timeval and timeout stuff added by davekw7x */
//    int timeouts = 0;

	pthread_detach(pthread_self());
	if (envp==NULL || envp->cmdsock<=0 || envp->common==NULL) {
		Vmn_Log_Warning("c1.c:serviceThrFxn() params invalid\n");
		cleanup(THREAD_FAILURE);
	}
	index = envp->index;

#ifdef WIN32
	//lengshan, 2014-08-03
	i = 1000;
	if (setsockopt(envp->cmdsock, SOL_SOCKET, SO_RCVTIMEO, (char *)&i,  sizeof(tv)))
	{
		perror("c1.c:setsockopt rcvtimeout");
	}
	i = 3000;
	if (setsockopt(envp->cmdsock, SOL_SOCKET, SO_SNDTIMEO, (char *)&i,  sizeof(tv)))
	{
		perror("c1.c:setsockopt sndtimeout");
	}
#else
    //linxj 2012-08-26
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if (setsockopt(envp->cmdsock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,  sizeof(tv)))
	{
		perror("c1.c:setsockopt rcvtimeout");
	}
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	if (setsockopt(envp->cmdsock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,  sizeof(tv)))
	{
		perror("c1.c:setsockopt sndtimeout");
	}
#endif
	
	while (envp->common->quit_flag_ptr && !(*envp->common->quit_flag_ptr)) {
		/* Heart beat check, thread will return when it's timeout */
		if (envp->heart > SERVER_HEART_TIMEOUT) {
			printf("warning:connection[index=%d, sock=%d] heart rate time out\n", index, envp->cmdsock);
			break;
		}
		recvlen = recv(envp->cmdsock, recvbuf, sizeof(recvbuf), 0);	//recv() will return in 1s timeout
		if (recvlen < 0) 
		{
			//recv() return timeout:continue
			if (IS_SOCK_TIMEOUT())
			{
				if (envp->heart >= 0){
					(envp->heart)++; //linxj 2012-08-26
//					printf("heart beat:%d boardID=%d \n",envp->heart,envp->board_id);
				}
//				usleep(10000);
				continue;
			}
			printf("[error]:sock=%d, recv failed! lasterror=%d\n", envp->cmdsock, GET_LAST_SOCK_ERROR());
			break;
		}
		if (recvlen==0)
		{
			Vmn_Log_Warning("warning:socket of the client has been closed, error=%d\n", GET_LAST_SOCK_ERROR());
			break;
		}
		if (recvlen < sizeof(head))
		{
			printf("[error]:recvlen=%d too short! \n",recvlen);
			continue;
		}
        if(recvlen == sizeof(recvbuf))
        {
			printf("warning: recv bufsize is too small??\n");	
        }
		if (REQ_HEART_BEAT != recvbuf[0]) {
//			printf("success:recv from sock = %d, recvlen = %d\n", envp->cmdsock, recvlen);
			printf("\n");
		}
		memset(&head, 0, sizeof (head));
		memcpy((void *) &head, recvbuf, sizeof (head));
		if (REQ_HEART_BEAT != head.CommandID) {
			printf("info   :index=%d, [CommandID=0x%02x ], TLV=0x%02x , ExtLength=%d,version=0x%02x \n",index, head.CommandID, head.Res1, head.ExtLength,head.HeadLength_Version);
		} else {
			//printf("info   :recv form index=%d, sock=%d, heart rate\n", index, envp->cmdsock);    //for debug
		}
		
		if (envp->index > *envp->common->max_conn_num && head.CommandID != REQ_SUB_CONNECTION) {	//lengshan, 2014-07-26
			Vmn_Log_Warning("New request while there is still %u connection, just drop!\n", *envp->common->max_conn_num);
			break;
		}

		switch (head.CommandID) {
			/* Login check, cmdsock */
			case REQ_USER_LOGIN :
			{
				int ret = 0;
//				printall(recvbuf, sizeof(Head));
				printf ("info   :REQ_USER_LOGIN = 0x%02x \n", REQ_USER_LOGIN);
				++(*envp->common->global_board_index);	//board_id is globally unique
				envp->board_id = *envp->common->global_board_index;
				ret = AckUserLogin(envp->cmdsock, recvbuf, &head, envp->board_id);
				if (ret) {
					printf("warning:client login failed, ret=%d", ret);
					cleanup(THREAD_FAILURE);
				}
				break;
			}

			/* Client request new sub connection for data, datacmd */
			case REQ_SUB_CONNECTION : 
			{
				unsigned int current_id;
				sock_type = head.HeadData[4];
				memcpy(&current_id, head.HeadData, sizeof(unsigned int));
				envp->heart = -1;	//disable heart beat for this connection
				if (current_id == 0) {
					perror("[error]:boardID of REQ_SUB_CONNECTION");
					cleanup(THREAD_FAILURE);
				}
				for (i=0 ; i<*(envp->common->max_conn_num); ++i) {
					if ((current_id == (envp->common->env_array+i)->board_id)
						&&((envp->common->env_array+i)->board_id > 0)){
						break;
					}
				}
				if (i >= *envp->common->max_conn_num) {
					perror("[error]:mismatch sock_cmd");
					cleanup(THREAD_FAILURE);
				}
				index = i;	//make sure if REQ_VIDEO_START is sent by datasock, index is correct
				
				if (1 == head.HeadData[4]) {
					(envp->common->env_array+index)->datasock = envp->cmdsock;
					(envp->common->env_array+index)->channel_client = head.HeadData[5];	//Client request master stream or sub stream
					printf("info   :channel_client = %u, channel_type = %hhu\n", (envp->common->env_array+index)->channel_client, head.HeadData[4]);	//test
					printf("info   :c1.c, go to data send function,index=%d\n",index);
					senddata((void *)(envp->common->env_array+index));
				} else if(2 == head.HeadData[4]) {
					(envp->common->env_array+index)->othersock = envp->cmdsock;
					/* FIXME, how to handle other connection? */
//					while (1)	sleep(15);
					Vmn_Log_Debug("info   :c1.c, othercmd[%d], just wait\n", index);
					while ((envp->common->env_array+index)->cmdsock != -1) Sleep(1000);
					cleanup(THREAD_SUCCESS);
				} else {
					perror("[error]:type of REQ_SUB_CONNECTION");
					cleanup(THREAD_FAILURE);
				}

				break;
			}

			/* Client request video data, cmdsock */
			case REQ_VIDEO_START :
				for (i=0; i<=*envp->common->max_conn_num; ++i) {	//<=?
					/* Select channel */
					switch (head.HeadData[i]) {
						case 0 :
							(envp->common->env_array+index)->channel_server &= ((~(1 << i)) | 0xffff0000);
							break;
						case 1 : 
							(envp->common->env_array+index)->channel_server |= (1 << i);
							break;
						case 2 :break;
						default :
							perror("[error]:channel_server enable recv from REQ_VIDEO_START");
							cleanup(THREAD_FAILURE);
					}
					/* Select stream type(host or sub) */
					switch (recvbuf[sizeof(head)+i]) {
						case 0 :	//master
							(envp->common->env_array+index)->channel_server &= ((~(1 << (i+16))) | 0x0000ffff);
							break;
						case 1 :	//sub
							(envp->common->env_array+index)->channel_server |= (1 << (i+16));
							printf("c1.c:socket[%d] set stream bit\n", index);
							break;
						default :
							perror("[error]:Data stream type recv from REQ_VIDEO_START");
							cleanup(THREAD_FAILURE);
							break;
					}
				}

				printf("info   :index=%d :set channel bits=0x%x\n",index, (envp->common->env_array+index)->channel_server);//test
				break;

			/* Heart Beat, cmdsock */
			case REQ_HEART_BEAT :
//				printf("info   :index=%d : heart beat,HeadData[0]=0x%02x !!\n",index,head.HeadData[0]);
				envp->heart = 0;
                memset(&sendhead,0,sizeof(sendhead));
                sendhead.CommandID = ACK_HEART_BEAT;
				sendhead.HeadLength_Version = 0x58;
				sendhead.HeadData[5] = 0xff;
                if (send(envp->cmdsock,&sendhead,sizeof(sendhead),0) <= 0) {
						perror("[error]:send heart ack failed!");
						cleanup(THREAD_FAILURE);
                }
				break;

			case REQ_INFO_SYSTEM :
				printf("info   :index=%d :REQ CommandID=0x%02x , HeadData[0]=0x%02x %02x %02x %02x ,HeadData[4]=0x%02x %02x %02x %02x , HeadData[8]=0x%02x %02x %02x %02x \n", 
						index, 
						head.CommandID, 
						head.HeadData[0],
						head.HeadData[1],
						head.HeadData[2],
						head.HeadData[3],
						head.HeadData[4],
						head.HeadData[5],
						head.HeadData[6],
						head.HeadData[7],
						head.HeadData[8],
						head.HeadData[9],
						head.HeadData[10],
						head.HeadData[11]);
				if (-1 == AckSystemInfo(envp->cmdsock, &head)) {
					cleanup(THREAD_FAILURE);
				}
				break;

			case 0x68 :
				break;
			case 0xa0 :
				break;
			case REQ_PANTZ_CTRL :
				printf("info   :index=%d :REQ CommandID=0x%02x , HeadData[0]=0x%02x %02x %02x %02x , HeadData[4]=0x%02x %02x %02x %02x , HeadData[8]=0x%02x %02x %02x %02x \n",
						index, 
						head.CommandID, 
						head.HeadData[0],
						head.HeadData[1],
						head.HeadData[2],
						head.HeadData[3],
						head.HeadData[4],
						head.HeadData[5],
						head.HeadData[6],
						head.HeadData[7],
						head.HeadData[8],
						head.HeadData[9],
						head.HeadData[10],
						head.HeadData[11]);
//				if (-1 == AckPantzCtrl(&head)) {
//					cleanup(THREAD_FAILURE);
//				}
				break;

			default :
				perror("warning:invalid command");
				break;
		}
	}

cleanup:
	if (envp->cmdsock != -1) {
		pthread_mutex_lock(envp->common->mutex);
		switch (sock_type) {
			case 0 :
				Vmn_Log_Debug("cmd_connection:");
				break;
			case 1 :
				Vmn_Log_Debug("\tdata_connection:");
				break;
			case 2 :
				Vmn_Log_Debug("\tother_connection:");
				break;
			default :
				Vmn_Log_Debug("\tinvalid_connection:");
				break;
		}
		pthread_mutex_lock(envp->common->conn_num_mutex);
		--(*envp->common->conn_num_ptr);
		pthread_mutex_unlock(envp->common->conn_num_mutex);
		printf("socket[%d] close=%d, <conn_num_ptr>=%d ... OK!!\n", index, envp->cmdsock, *envp->common->conn_num_ptr);
		closesocket(envp->cmdsock);	//in env for datasend connection, cmdsock is datasock
		envp->cmdsock = -1;
		envp->heart = 0;	//lengshan, 2014-11-18, make sure heartbeat is clear for the next new connection
		pthread_mutex_unlock(envp->common->mutex);
	}
//	printf("info   :pthread exit, index=%d... OK!!\n", index);
	return status;
}


 /******************************************************************************
 * Return Value:
 * 0:Success
 * 1:Account/Password mismatch
 * 2:Account is NOT exist
 * 3:Account is checkin already
 * 4:Locked
 * 5:Account is in blacklist
 ******************************************************************************/
static int LoginCheck(unsigned char *buf, Head *phead) {
	if (0 == phead->ExtLength) {
		//Account is NOT exist
		if (0 != strncmp((char *)phead->HeadData, ACCOUNT, MAX_ACCOUNT_LEN)) {
			return 2;
		}
		//Account/Password mismatch
		else {
			if (0 != strncmp((char *)phead->HeadData+8, PWD, MAX_PWD_LEN)) {
				return 1;
			}
		//Success
			else {
				return 0;
			}
		}
		
	} else {
		//Account is NOT exist
		if (0 != strncmp((char *)(buf+sizeof(Head)), ACCOUNT, strlen(ACCOUNT))) {
			return 2;
		}
		//Account/Password mismatch
		else {
			if (0 != strncmp((char*)(buf+sizeof(Head)+strlen(ACCOUNT)+2), PWD, strlen(PWD))) {
				return 1;
			}
		//Success
			else {
				return 0;
			}
		}
	}
}


static int AckUserLogin(SOCKET cmdsock, unsigned char *recvbuf, Head *phead, unsigned int board_id) {
	Head head = *phead;
//	unsigned char sendbuf[sizeof(Head)+(8+16)+(8+48)+(8+48)+(8+32)+(8+4)];
	unsigned char sendbuf[32+12];
	int ret = 0;

	printf("info   :%s\t%s\n", &head.HeadData[0], &head.HeadData[8]);		//test
//	printf("before data ACK_USER_LOGIN\n");	//test
//	if (0 == strncmp (head.HeadData, ACCOUNT, MAX_ACCOUNT_LEN)
//	  && 0 == strncmp (head.HeadData + 8, PWD, MAX_PWD_LEN)) {
	printf("info   :%s\t%s\n", &head.HeadData[24], &head.HeadData[24+7]);		//NVR test
	ret = LoginCheck(recvbuf, &head);
	printf("info   :Login check = %d\n", ret);

	memset (&head, 0, sizeof(head));
	head.CommandID		=	ACK_USER_LOGIN;
	head.Res1			=	0x00;
	head.HeadLength_Version = 0x58;
	head.ExtLength		=	12;
	if (0 == ret){
//		printf("info   :in if account and password match\n");
		head.HeadData[0]	=	0x0;			//check success
		head.HeadData[2]	=	CHANNEL_COUNT;	//channel count
//		head.HeadData[2]	=	0x01;
		head.HeadData[3]	=	0x09;			//video compress H264=9
		head.HeadData[4]	=	0x0b;			//device type
//		head.HeadData[5]	=	0xff;
		memcpy(head.HeadData+8, &board_id, sizeof(unsigned int));	//client board id
//		head.HeadData[8]	=	0x01;
		head.HeadData[16]	=	0x06;
		head.HeadData[18]	=	0xf9;
		head.HeadData[21]	=	0x01;
		head.HeadData[22]	=	0x64;			//fixxed
		head.HeadData[23]	=	0x02;			//fixxed
	} else {
		head.HeadData[0]	=	0x01;			//check failure
	}
	memset(sendbuf, 0, sizeof (sendbuf));
	memcpy((void *)sendbuf, (void *)&head, sizeof (head));
	sendbuf[sizeof(Head)+0] = 0x01;
	sendbuf[sizeof(Head)+4] = 0x04;
	sendbuf[sizeof(Head)+8] = 0x66;

	//      else if (Account is NOT exist) {}
	//      else if (Account/Password mismatch) {}
	//      else if (Account has been checkin already) {}
//	printf("before send ACK_USER_LOGIN\n");

/* Send back checkout resault */
//	print_in_hex(sendbuf, 32, "Login ACK:", NULL);
	if (send (cmdsock, sendbuf, sizeof (sendbuf), 0) <= 0) {
		perror ("[error]:send ACK_USER_LOGIN");
		ret = -1;
	}
//	printf("after send ACK_USER_LOGIN\n");

	return ret;
}


static int AckSystemInfo(SOCKET cmdsock, Head *phead) {
	unsigned char sendbuf[] = {
		0xB4,0x00,0x00,0x58,0xF8,0x00,0x00,0x00,0x0A,0x00
		,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
		,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0xFF,0x54,0xFA,0x7F,0xBA,0x00,0x00
		,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x08,0x00
		,0x00,0x00,0x03,0x00,0x00,0x00,0x02,0x00,0x10,0x00,0x40,0x1F,0x00,0x00,0xBC,0xC9
		,0x26,0x00,0xBC,0xC9,0x26,0x00,0x04,0x00,0x00,0x00,0x6C,0xDA,0x29,0x00,0x44,0xDA
		,0x24,0x00,0x57,0x00,0x00,0x00,0x6C,0xFA,0x7F,0xBA,0x28,0xE3,0x24,0x00,0x00,0x00
		,0x00,0x00,0x6C,0xFA,0x7F,0xBA,0x7C,0xFA,0x7F,0xBA,0x28,0xE3,0x24,0x00,0x00,0x00
		,0x00,0x00,0x7C,0xFA,0x7F,0xBA,0x28,0xE3,0x24,0x00,0x00,0x00,0x00,0x00,0x04,0x00
		,0x10,0x00,0x40,0x1F,0x00,0x00,0xBC,0xC9,0x26,0x00,0xBC,0xC9,0x26,0x00,0x04,0x00
		,0x00,0x00,0x6C,0xDA,0x29,0x00,0x44,0xDA,0x24,0x00,0x57,0x00,0x00,0x00,0x6C,0xFA
		,0x7F,0xBA,0x28,0xE3,0x24,0x00,0x00,0x00,0x00,0x00,0x6C,0xFA,0x7F,0xBA,0x7C,0xFA
		,0x7F,0xBA,0x28,0xE3,0x24,0x00,0x00,0x00,0x00,0x00,0x7C,0xFA,0x7F,0xBA,0x28,0xE3
		,0x7F,0xBA,0x28,0xE3,0x24,0x00,0x00,0x00,0x00,0x00,0x7C,0xFA,0x7F,0xBA,0x28,0xE3
		,0x24,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x08,0x00,0x40,0x1F,0x00,0x00,0xBC,0xC9
		,0x26,0x00,0xBC,0xC9,0x26,0x00,0x04,0x00,0x00,0x00,0x6C,0xDA,0x29,0x00,0x44,0xDA
		,0x24,0x00,0x57,0x00,0x00,0x00,0x6C,0xFA,0x7F,0xBA,0x28,0xE3,0x24,0x00,0x00,0x00
		,0x00,0x00,0x6C,0xFA,0x7F,0xBA,0x7C,0xFA,0x7F,0xBA,0x28,0xE3,0x24,0x00,0x00,0x00
	};
	if (REQ_INFO_SYSTEM != phead->CommandID) {
		perror("[error]:mismatch Command ID in REQ_INFO_SYSTEM");
		return -1;
	}
	if (0x1a != phead->HeadData[0]) {
		printf("warning:mismatch SubID[%d] in REQ_INFO_SYSTEM\n",phead->HeadData[0]);
//		return -1;
	}
	if ((send(cmdsock, sendbuf, sizeof(sendbuf), 0)) <= 0) {
		perror("[error]:send ACK_INFO_SYSTEM failed");
		return -1;
	}

	return 0;
}


//-1-Crit; 0-Not Enable; 1-Master; 2-Sub
static int getchannelStatus(struct c1_service_env *envp) {
	int ret = 0;
	if(envp==NULL || envp->cmdsock<0)
		return -1;

	if ((envp->channel_server) & (1 << (envp->channel_client-1))) {
		if ((envp->channel_server) & (1 << (envp->channel_client-1+16))) {
			ret = 2;
		} else {
			ret = 1;
		}
	}
	//else ret is 0, not change;
	return ret;
}


static void *senddata(void *arg) {
	struct c1_service_env *envp = (struct c1_service_env *)arg;
	void *status = THREAD_SUCCESS;
    int ringID=0, len = 0, recvlen;
    unsigned char recvbuf[32], *sendbuf;
	int channelStatus = -1;
	unsigned int channel_tmp = envp->channel_client;
	pRingBuffer pbuffer_tmp = NULL;
    if(envp==NULL || envp->common==NULL || envp->cmdsock==-1 || envp->datasock==-1) {
		Vmn_Log_Warning("c1:senddata invalid params\n");
        cleanup(THREAD_FAILURE);
	}
    
	/* Get channel status which will be changed by REQ_VIDEO_START in cmd socket */
	while (envp->cmdsock!=-1 && envp->common->quit_flag_ptr && !(*envp->common->quit_flag_ptr)) {
		while (envp->cmdsock!=-1 && envp->common->quit_flag_ptr && !(*envp->common->quit_flag_ptr)) {
			channelStatus = getchannelStatus(envp);
			if(channelStatus !=0) break;
			Sleep(10);		//10ms
		}
		if (-1 == channelStatus) {
			perror("[error]: get channel status error");
			cleanup(THREAD_FAILURE);
		}
		/* master or sub */
		switch (channelStatus) {
			case 1 :
				printf("c1.c:data stream type video\n");
				pbuffer_tmp = envp->common->pbuffer;
				break;
			case 2 :
				printf("c1.c:data stream type resize\n");
				pbuffer_tmp = envp->common->pbuffer_rsz;
				break;
			case 0 :
				perror("warning:client has close data channel");
				cleanup(THREAD_SUCCESS);
			default :
				perror("[error]:bad channel status");
				cleanup(THREAD_FAILURE);
		}

		ringID = vis_ring_buffer_getthreadid(pbuffer_tmp);
		if(ringID<0){
			perror("c1.c:ringbuffer get thread id failed");
			cleanup(THREAD_FAILURE);
		}
		printf("c1.c:ringbuffer thread id = %d\n", ringID);
		vis_ring_buffer_regthread(pbuffer_tmp, ringID);
		printf("c1.c:after regthread\n");

		while(envp->cmdsock!=-1 && envp->common->quit_flag_ptr && !(*envp->common->quit_flag_ptr)) {
			if (channel_tmp ^ (envp->channel_client)) {
				printf("c1.c:channel_temp = %x\n", channel_tmp);
				printf("c1.c:channel = %x\n", envp->channel_client);
				printf("c1.c: Channel status has been changed\n");
				channel_tmp = envp->channel_client;
				Sleep(10);
				break;
			}
			if ((sendbuf = vis_ring_buffer_send_addr(pbuffer_tmp, &len, NULL, NULL, NULL, ringID)) == NULL) {
				//printf("c1.c: current = %d no data\n", current);
				Sleep(10);
				recvlen = recv(envp->datasock, recvbuf, sizeof(recvbuf), MSG_DONTWAIT);
				if (recvlen == 0) {
					Vmn_Log_Info("Client has been closed\n");
					cleanup(THREAD_FAILURE);
				} else if (recvlen<0 && !IS_SOCK_TIMEOUT()) {
					Vmn_Log_Error("<w>connfd[%d] recv failed, %d\n", envp->index, GET_LAST_SOCK_ERROR());
					cleanup(THREAD_FAILURE);
				}
				continue;
			}
			if (send(envp->datasock, sendbuf, len, 0) <= 0) {
				/* Broken pipe error printed if client has been closed */
				Vmn_Log_Error("send");
				break;
			}
		}
		vis_ring_buffer_unregthread(pbuffer_tmp, ringID);
		ringID = -1;
	}
	
cleanup:
	if (ringID != -1) {
		vis_ring_buffer_unregthread(pbuffer_tmp, ringID);
		ringID = -1;
	}
	printf("c1.c:senddataFunc() exit,sock=%d\n", envp->datasock);
	envp->channel_server &= ((~(1 << (envp->channel_client-1))) | 0xffff0000);	//maybe not nessery
/*
	closesocket(envp->datasock);     //closesocket in senddataFnc() or recvThread()
	envp->datasock = -1;
	pthread_mutex_lock(envp->common->conn_num_mutex);
	--conn_num_ptr;
	pthread_mutex_unlock(envp->common->conn_num_mutex);
	printf("info   :conn_num_ptr = %d\n", envp->common->conn_num_ptr);
*/ 
    return status;
}
