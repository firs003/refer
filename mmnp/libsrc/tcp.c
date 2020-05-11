#include "vis_mmnp.h"

#include "tcp.h"
//#include "visconfig.h"
//#include "vis_common.h"
#include "ringbuffer.h"

#define SERV_PORT 1230
#define SEND_BY_PACKAGE

#ifdef SEND_BY_PACKAGE
#define PACKAGE_LEN	(188*32)
static size_t send_package(int sockfd, void *buf, size_t len, int flag);
#endif

typedef struct tcp_send_env {
	pthread_t send_tid;
	int index;
	int sock;
//	Vis_Mmnp_Handle *handle_ptr;
	RingBuffer *pbuffer;
	volatile int *quit_flag_ptr;
	pthread_mutex_t *mutex;
	unsigned int *conn_num_ptr;
	pthread_mutex_t *conn_num_mutex;
} Tcp_Send_Env;

void *sendThread(void *arg)
{
	int len = 0;
	unsigned char recvbuf[2], *sendbuf;
	struct tcp_send_env *envp = (struct tcp_send_env *)arg;
	int current = envp->index;
	int sendsock = envp->sock;
	RingBuffer *pbuffer = envp->pbuffer;
	pthread_mutex_t *mutex = envp->mutex;
	//Vmn_Log_Debug("sender.c: conn_server.connfd[%d] = %d, <conn_num=%u>\n", current, sendsock, *envp->conn_num_ptr);
	if(envp==0)
		return 0;
	mutex = envp->mutex;
	if(mutex==0)
		return 0;

	//pthread_mutex_lock(mutex);		//linxj
	current = envp->index;
	sendsock = envp->sock;
	pbuffer = envp->pbuffer;
	//pthread_mutex_unlock(mutex);	//params protection

    pthread_detach(pthread_self());
    vis_ring_buffer_regthread(pbuffer, current);

    while(envp->quit_flag_ptr && !(*envp->quit_flag_ptr)) {
//		Vmn_Log_Debug("\t\t1-1 sender.c:before get_send_addr\n");
		if ((sendbuf = vis_ring_buffer_send_addr(pbuffer, &len, NULL, NULL, NULL, current)) == NULL) {
			int recvlen = -1;
			usleep(5000);		//linxj: usleep(30000);
			recvlen = recv(sendsock, recvbuf, sizeof(recvbuf), MSG_DONTWAIT);
			if (recvlen == 0) {
				Vmn_Log_Info("Client has been closed\n");
				break;
			} else if (recvlen<0 && !IS_SOCK_TIMEOUT()) {
				Vmn_Log_Debug("<w>connfd[%d] recv failed[%d]:%s\n", current, errno, strerror(errno));
				break;
			}
			continue;
		}
//		Vmn_Log_Debug("\t\t1-2 sender.c:after get_send_addr\n");
//		Vmn_Log_Debug("\t\t\t1-3 sender.c:before send\n");
#ifndef SEND_BY_PACKAGE
		if (send(sendsock, sendbuf, len, 0) <= 0) {
			/* Broken pipe error printed if client has been closed */
			Vmn_Log_Error("send");
			break;
		}
#else
		if (send_package(sendsock, sendbuf, len, 0) != len) {
			break;
		}
#endif
//		Vmn_Log_Debug("\t\t\t1-4 sender.c:after send\n");
	}

	pthread_mutex_lock(mutex);
	if (envp->sock != -1) {
		closesocket(envp->sock);
		envp->sock = -1;
		--(*envp->conn_num_ptr);
	}
	pthread_mutex_unlock(mutex);

	vis_ring_buffer_unregthread(pbuffer, current);
	Vmn_Log_Debug("tcp.c: conn_server.connfd[%d] = %d disconnect, <conn_num>=%d\n", current, sendsock, *envp->conn_num_ptr);

    return NULL;
}

/******************************************************************************
 * tcpThrFxn
 ******************************************************************************/
void *vmn_tcpThrFxn(void *arg)
{
//	TcpEnv		*envp           = (TcpEnv *)arg;
	TcpEnv		*envp           = NULL;
	void 		*status         = THREAD_SUCCESS;
	int			listenfd = -1, opt = 1;
	int         i;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len=sizeof(struct sockaddr_in), socklen=sizeof(struct sockaddr_in);
	char str[INET_ADDRSTRLEN] = {0, };
	unsigned short cli_port;
	int buflen;
	struct tcp_send_env *send_env_array  = NULL;		//ls 2013-06-03
	int socktmp = -1;
	int maxq;
	unsigned short port;
	pthread_mutex_t mutex;
	int cleanup_flag = 0;

	envp = (TcpEnv *)calloc(1, sizeof(TcpEnv));
	if (envp == NULL) {
		Vmn_Log_Error("alloc memery for TcpEnv failed");
		envp = (TcpEnv *) arg;			//linxj, crit when arg is NULL, TODO
		pthread_mutex_lock(envp->mutex_initsync);
		pthread_cond_broadcast(envp->cond_initsync);
		pthread_mutex_unlock(envp->mutex_initsync);
		*envp->quit_flag_ptr = 1;
		envp = (TcpEnv *) 0;			//linxj make sure no free @ cleanup
		cleanup(THREAD_FAILURE);
	}
	memcpy(envp, arg, sizeof(TcpEnv));

	if (pthread_mutex_init(&mutex, NULL) != 0) {
		Vmn_Log_Error("init mutex for tcpsender failed");
		pthread_mutex_lock(envp->mutex_initsync);
		pthread_cond_broadcast(envp->cond_initsync);
		pthread_mutex_unlock(envp->mutex_initsync);
		cleanup(THREAD_FAILURE);
	}
	cleanup_flag = 1;

#ifndef	WIN32
	signal(SIGPIPE, SIG_IGN);
#endif

	port = envp->port;
	//if (port<=1024) port = 36511;
	if (port<=1024)  port = 1234; //linxj
//	(envp->port<=1024) ? port=1234: port=envp->port;
//	(envp->maxConnNum>MAXQ||envp->maxConnNum==0) ? maxq=MAXQ: maxq=envp->maxConnNum;
	maxq = MAXQ;
	send_env_array = (struct tcp_send_env *)calloc(maxq, sizeof(struct tcp_send_env));
	if (send_env_array == NULL) {
		Vmn_Log_Error("alloc memery for send_env_array failed");
		pthread_mutex_lock(envp->mutex_initsync);
		pthread_cond_broadcast(envp->cond_initsync);
		pthread_mutex_unlock(envp->mutex_initsync);
		cleanup(THREAD_FAILURE);
	}
	for (i=0; i<maxq; ++i) send_env_array[i].sock = -1;

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

	listen(listenfd, maxq);

	/* Signal that initialization is done and wait for other threads */
	Vmn_Log_Debug("2.listen before cond_broadcast\n");
	pthread_mutex_lock(envp->mutex_initsync);
	pthread_cond_broadcast(envp->cond_initsync);
	pthread_mutex_unlock(envp->mutex_initsync);
	Vmn_Log_Debug("3.listen after cond_broadcast\n");
//	cleanup(THREAD_FAILURE);	//test

	Vmn_Log_Debug("quit_flag[%p]=%d\n", envp->quit_flag_ptr, *envp->quit_flag_ptr);
	Vmn_Log_Info("tcp.c: Accepting connections...\n");
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
		//	Vmn_Log_Debug("Not expect, ret_select=%d, FD_ISSET=%d\n", ret_select, FD_ISSET(listenfd, &read_fds));
			continue;
		}
//		Vmn_Log_Debug("before accept\n");
		socktmp = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
//		Vmn_Log_Debug("accept return %d\n", socktmp);
		if (socktmp != -1) {
			pthread_mutex_lock(&mutex);
			for (i=0; i<maxq; i++) {
				if ((send_env_array+i)->sock == -1)
					break;
			}
			pthread_mutex_unlock(&mutex);

			if (i == maxq) {
				Vmn_Log_Info("tcp.c: already %d connections\n", maxq);
				closesocket(socktmp);
				Sleep(1000);
				continue;
			}
//			setsockopt((send_env_array+i)->sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
			(send_env_array+i)->index = i;
			(send_env_array+i)->sock = socktmp;
//			(send_env_array+i)->handle_ptr = envp->handle_ptr;
			(send_env_array+i)->pbuffer = envp->pbuffer;
			(send_env_array+i)->quit_flag_ptr = envp->quit_flag_ptr;
			(send_env_array+i)->conn_num_ptr = envp->conn_num_ptr;
			(send_env_array+i)->conn_num_mutex = envp->conn_num_mutex;
			(send_env_array+i)->mutex = &mutex;

			if (pthread_create(&(send_env_array+i)->send_tid, NULL, sendThread, (void *)(send_env_array+i))!= 0) {
				Vmn_Log_Error("tcp.c: Failed to create sendThread\n");
					pthread_mutex_lock(&mutex);
					if((send_env_array+i)->sock!=-1) {
						closesocket((send_env_array+i)->sock);
						(send_env_array+i)->sock = -1;
					}
					pthread_mutex_unlock(&mutex);
				continue;
			}
			pthread_mutex_lock(&mutex);
			++(*envp->conn_num_ptr);
			pthread_mutex_unlock(&mutex);
#ifdef	WIN32
			strcpy(str,inet_ntoa(cliaddr.sin_addr));
#else
			inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str));
#endif
			cli_port = ntohs(cliaddr.sin_port);
			Vmn_Log_Info("tcp.c:Received from %s at PORT %d, send_env_array[%d].sock=%d, <conn_num>=%d\n", str, cli_port, i, (send_env_array+i)->sock, *envp->conn_num_ptr);
		} else {
			perror("accept failed");
			if (errno != EAGAIN && errno != 0) {	//linxj
				Vmn_Log_Error("accept failed errno=%d ",errno);
			}
		}
	}

cleanup:
    /* Make sure the other threads aren't waiting for init to complete */
	Vmn_Log_Debug("Listen Thread Cleanup\n");
	if (envp) *envp->quit_flag_ptr = 1;

	if (listenfd != -1) {
		closesocket(listenfd);
		listenfd = -1;
	}

	switch (cleanup_flag) {
		case 1 :
			pthread_mutex_lock(&mutex);
			if (send_env_array) {
				for (i=0; i<maxq; i++) {
					if (send_env_array[i].sock != -1)
						closesocket(send_env_array[i].sock);
				}
				free(send_env_array);
				send_env_array = NULL;
			}
			pthread_mutex_unlock(&mutex);
			pthread_mutex_destroy(&mutex);
		default :
			break;
	}

	if (envp) {
		free(envp);
		envp = NULL;
	}

	Vmn_Log_Debug("Listen Thread Return\n");
    return status;
}

#ifdef SEND_BY_PACKAGE
static size_t send_package(int sockfd, void *buf, size_t len, int flag) {
	/*
	 * len:res length in total
	 * sendlen:length to send a time < 188*50
	 * retlen:length has been send indeed
	 */
	int sendlen = 0, retlen = 0;
	char *sendbuf = (char *)buf;
	
	if (sockfd<=0 || buf==NULL || len==0) {
		Vmn_Log_Debug("tcp.c: send_package params invalid, sockfd=%d, buf=%p, len=%u\n", sockfd, sendbuf, len);
		return -2;
	}

	do {
		sendlen = (len>PACKAGE_LEN) ? PACKAGE_LEN : len;
		retlen = send(sockfd, sendbuf, sendlen, flag);
		if (retlen <= 0) {
			Vmn_Log_Error("send_package failed, retlen=%d", retlen);
			break;
		}
		len -= retlen;
		sendbuf += retlen;
	} while (len>0);

	return (sendbuf-(char *)buf) ;
}
#endif
