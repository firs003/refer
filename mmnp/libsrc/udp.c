#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#endif

#include "vis_mmnp.h"
#include "udp.h"
#include "ringbuffer.h"
#include "visconfig.h"


#define MAX_BUFLEN 64
#define SERV_PORT  CLIENT_CONNECTTCPPORT
#define MAXQ 8
#define SENDTO_BY_PACKAGE
#define TS_LENGTH 188
#define TS_NUM 7
#define SENDTO_PACKAGE_LEN (TS_LENGTH*TS_NUM)


static SOCKET socketInit(conn_client_t *conn_client);
static int connection_return(const conn_client_t *conn_client, int num, int connfd, Connection *connection);


/******************************************************************************
 * demandThrFxn
 ******************************************************************************/
void *vmn_demandThrFxn(void *arg)
{
//	DemandEnv	*envp            = (DemandEnv *) arg;
	DemandEnv	*envp            = NULL;
	void 		*status          = THREAD_SUCCESS;
	SOCKET		 listenfd, connfd = 0;
	int			 opt = 1, result, i = 0, maxq = 0;
	unsigned char recvbuf[MAX_BUFLEN];
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len = sizeof(cliaddr);
	unsigned int sec = 0;
	conn_client_t *conn_client = NULL;
	Connection          connection;

	envp = (DemandEnv *)calloc(1, sizeof(DemandEnv));
	if (envp == NULL) {
		Vmn_Log_Error("alloc memery for DemandEnv failed");
		envp = (DemandEnv *) arg;			//linxj, crit when arg is NULL, TODO
		pthread_mutex_lock(envp->mutex_initsync);
		pthread_cond_broadcast(envp->cond_initsync);
		pthread_mutex_unlock(envp->mutex_initsync);
		envp = (DemandEnv *) 0;			//linxj make sure no free @ cleanup
		cleanup(THREAD_FAILURE);
	}
	memcpy(envp, arg, sizeof(DemandEnv));
	conn_client = envp->conn_client;
	maxq = conn_client->maxConnNum;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		cleanup(THREAD_FAILURE);
	}
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
//	servaddr.sin_port = (*conn_client->src_port_ptr<=1024) ? htons(SERV_PORT) : htons(*conn_client->src_port_ptr);
	servaddr.sin_port = htons(SERV_PORT);
	Vmn_Log_Debug("demand.c:server_port=%hu\n", ntohs(servaddr.sin_port));

	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		perror("bind");
		cleanup(THREAD_FAILURE);
	}

	listen(listenfd, maxq);

	/* Signal that initialization is done and wait for other threads */
	Vmn_Log_Debug("2.demand before cond_broadcast\n");
	pthread_mutex_lock(envp->mutex_initsync);
	pthread_cond_broadcast(envp->cond_initsync);
	pthread_mutex_unlock(envp->mutex_initsync);
	Vmn_Log_Debug("3.demand after cond_broadcast\n");
	Vmn_Log_Debug("demand.c: after meet quit_flag = %d \n", envp->quit_flag_ptr && !(*envp->quit_flag_ptr));

	while (envp->quit_flag_ptr && !(*envp->quit_flag_ptr)) {
		fd_set read_fds;
		int ret_select = -1;
		struct timeval tv = {2, 0};

		if (*conn_client->type_ptr == NETWORK_SEND_VOD) {
			for (i=0; i<conn_client->maxConnNum; i++) {
				if (conn_client->flag_table[i] == 1) {
					time_t curr_time = time(&curr_time);
					sec = curr_time - conn_client->time_table[i];
					if (sec > 60) {
						pthread_mutex_lock(&conn_client->mutex);
						(*conn_client->conn_num_ptr)--;
						conn_client->flag_table[i] = 0;
						conn_client->time_table[i] = 0;
						pthread_mutex_unlock(&conn_client->mutex);
						Vmn_Log_Warning("ip = 0x%08x port = %hu, time out\n", (unsigned int)conn_client->dstaddr_table[i].sin_addr.s_addr, ntohs(conn_client->dstaddr_table[i].sin_port));
					}
				}
			}
		}
				
		/* select will change fds_set and tv params
		 * when select return timeout, tv=0, we must reset fd_sets and timeout; */
		FD_ZERO(&read_fds);
		FD_SET(listenfd, &read_fds);
		ret_select = select(listenfd+1, &read_fds, NULL, NULL, &tv);
		if (ret_select!=1 || !FD_ISSET(listenfd, &read_fds)) {
		//	Vmn_Log_Debug("Not expect, ret_select=%d, FD_ISSET=%d\n", ret_select, FD_ISSET(listenfd, &read_fds));
			continue;
		}
		//Vmn_Log_Debug("before accept\n");
		connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
		if (connfd == -1) {
			continue;
		}
		//printf("1.demand.c: connfd=%d, client:ipaddr=0x%08x, port=%d\n", connfd, cliaddr.sin_addr.s_addr, cliaddr.sin_port);
		if ((result = recv(connfd, recvbuf, sizeof(recvbuf), 0)) == -1) {
				Vmn_Log_Warning("receive error, continue\n");
				closesocket(connfd);
				continue;
		}
		if (recvbuf[0] != 'v' || recvbuf[1] != 'i' || recvbuf[2] != 's') {
				printf("bad sync, just drop!\n");
				closesocket(connfd);
				continue;
		}
		//Vmn_Log_Debug("%d\n", recvbuf[3]==VIS_RQ_CONNECTION);	//if recvbuf is declared in char, recvbuf[3] whill != VIS_RQ_CONNECTION!!!
		if (recvbuf[3] == VIS_RQ_CONNECTION ) {
			if ((4 + sizeof(Connection)) <= result) {
				memcpy(&connection, recvbuf + 4, sizeof(Connection));
			} else {
				printf("bad message, just drop VIS_RQ_CONNECTION!\n");
				closesocket(connfd);
				continue;
			}
			if (*conn_client->type_ptr == NETWORK_SEND_VOD) {
				i=-1;   //nothing match
				if (connection.connectEnable && connection.UDPPORT!=0) {
					/* Already connected, refresh the timestamp */
					for (i=0; i<conn_client->maxConnNum; i++) {
						if ( conn_client->flag_table[i] == 1 && conn_client->dstaddr_table[i].sin_port == htons(connection.UDPPORT)
								&& memcmp(&conn_client->dstaddr_table[i].sin_addr, &cliaddr.sin_addr, sizeof(cliaddr.sin_addr)) == 0) {
								time_t curr_time = time(&curr_time);
								//gettimeofday(&tv, NULL);
								conn_client->time_table[i] = curr_time;
								//printf("ip = 0x%x, port = %d already in send queue\n", cliaddr.sin_addr, connection.UDPPORT);
								break;
						}
					}
					if (i < conn_client->maxConnNum) {
						connection_return(conn_client, i, connfd, &connection);
						closesocket(connfd);
						continue;
					}
					/* New connection request */
					for (i=0; i<conn_client->maxConnNum; i++) {
						if (conn_client->flag_table[i] == 0) {
							time_t curr_time = 0;
							pthread_mutex_lock(&conn_client->mutex);
							(*conn_client->conn_num_ptr)++;
							conn_client->dstaddr_table[i] = cliaddr;
							conn_client->dstaddr_table[i].sin_family = AF_INET;
							conn_client->dstaddr_table[i].sin_port = htons(connection.UDPPORT);
							conn_client->flag_table[i] = 1;
							curr_time = time(&curr_time);
							//gettimeofday(&tv, NULL);
							conn_client->time_table[i] = curr_time;
							pthread_mutex_unlock(&conn_client->mutex);
							printf("new connect:ip = 0x%x, port = %hu\n", (unsigned int)cliaddr.sin_addr.s_addr, connection.UDPPORT);
							break;
						}
					}
				} else if (connection.connectEnable==0 && connection.UDPPORT!=0) {
					printf("connectDisable\n");
					for (i=0; i<conn_client->maxConnNum; i++) {
						if (conn_client->flag_table[i]==1 && conn_client->dstaddr_table[i].sin_port == htons(connection.UDPPORT)
							&& memcmp(&conn_client->dstaddr_table[i].sin_addr, &cliaddr.sin_addr, sizeof(cliaddr.sin_addr)) == 0) {
							pthread_mutex_lock(&conn_client->mutex);
							(*conn_client->conn_num_ptr)--;
							conn_client->flag_table[i] = 0;
							conn_client->time_table[i] = 0;
							pthread_mutex_unlock(&conn_client->mutex);
							printf("ip = 0x%x, port = %hu\n", (unsigned int)cliaddr.sin_addr.s_addr, connection.UDPPORT);
							break;
						}
					}
				}
			}

			connection_return(envp->conn_client, i, connfd, &connection);
			closesocket(connfd);
		}
	}

cleanup:
	/* Make sure the other threads aren't waiting for init to complete */
	Vmn_Log_Debug("Demand Thread Cleanup\n");
	*envp->quit_flag_ptr = 1;

	if (connfd != -1) {
		closesocket(connfd);
	}
	if (listenfd != -1) {
		closesocket(listenfd);
	}
	if (envp) {
		free(envp);
	}
	return status;
}

/******************************************************************************
 * udpThrFxn
 ******************************************************************************/
void *vmn_udpThrFxn(void *arg)
{
	UdpEnv	*envp	= NULL;
	void	*status	= THREAD_SUCCESS;
	conn_client_t *conn_client = NULL;
	SOCKET	 sendsock = -1;
	RingBuffer *pbuffer = NULL;
	int buffer_index = 0, len=0, temp=0, maxq=0, i;
	unsigned char *psend = NULL, *sendbuf = NULL;
	unsigned int send_times = 0;
	char old_ip[16] = {0, };
	unsigned short old_port = 0;

	envp = (UdpEnv *)calloc(1, sizeof(UdpEnv));
	if (envp == NULL) {
		Vmn_Log_Error("alloc memery for UdpEnv failed");
		envp = (UdpEnv *) arg;			//linxj, crit when arg is NULL, TODO
		pthread_mutex_lock(envp->mutex_initsync);
		pthread_cond_broadcast(envp->cond_initsync);
		pthread_mutex_unlock(envp->mutex_initsync);
		envp = (UdpEnv *) 0;			//linxj make sure no free @ cleanup
		cleanup(THREAD_FAILURE);
	}
	memcpy(envp, arg, sizeof(UdpEnv));
	conn_client = envp->conn_client;
	pbuffer = envp->pbuffer;
	
	//(envp->maxConnNum>MAXQ||envp->maxConnNum==0) ? maxq=MAXQ: maxq=envp->maxConnNum;
	maxq = MAXQ;
	
	vis_ring_buffer_regthread(pbuffer, buffer_index);
	
	Vmn_Log_Debug("2.udp before cond_broadcast\n");
	pthread_mutex_lock(envp->mutex_initsync);
	pthread_cond_broadcast(envp->cond_initsync);
	pthread_mutex_unlock(envp->mutex_initsync);
	Vmn_Log_Debug("3.udp after cond_broadcast\n");
//	cleanup(THREAD_FAILURE);

	Vmn_Log_Info("udp.c: before main loop!\n");
	while (envp->quit_flag_ptr && !(*envp->quit_flag_ptr)) {
		/* Dynamic Change UDP send type */
		if (strncmp(old_ip, conn_client->dst_ipAddr, 16) || old_port!=*conn_client->dst_port_ptr) {
			Vmn_Log_Debug("Change dst_addr from [%s:%hu] to [%s:%hu]\n", old_ip, old_port, conn_client->dst_ipAddr, *conn_client->dst_port_ptr);
			//if not memset, flag and dstaddr will be kicked util time out, when broadcast change to vod
			memset(conn_client->port_table, 0, conn_client->maxConnNum*sizeof(unsigned short));
			memset(conn_client->dstaddr_table, 0, conn_client->maxConnNum*sizeof(struct sockaddr_in));
			memset(conn_client->flag_table, 0, conn_client->maxConnNum*sizeof(char));
			memset(conn_client->time_table, 0, conn_client->maxConnNum*sizeof(unsigned int));
			if (sendsock != -1) {	//make sure old socket has been closed, lengshan 2016-01-22
				closesocket(sendsock);
				sendsock = -1;
			}
			sendsock = socketInit(conn_client);
			if (socket < 0) {
				Vmn_Log_Error("init socket conn_client.socket failed");
				cleanup(THREAD_FAILURE);
			}
			memset(old_ip, 0, sizeof(old_ip));
			strncpy(old_ip, conn_client->dst_ipAddr, strlen(conn_client->dst_ipAddr));
			old_port = *conn_client->dst_port_ptr;
		}
		/* Get an encoded buffer from ringbuffer */
		if ((sendbuf = vis_ring_buffer_send_addr(pbuffer, &len, NULL, NULL, NULL, buffer_index)) == NULL) {
			Sleep(5);		//linxj: usleep(30000);
			continue;
		}
				
		/* Sendto */
		if (len > 0) {
		psend = sendbuf;
#ifndef SENDTO_BY_PACKAGE
			for(i=0; i<MAX_CONNECTION; i++) {
				if (conn_client->flag_table[i] == 1) {
					int bytesSent = sendto(sendsock, psend, len, 0,
										 (struct sockaddr*)&conn_client->dstaddr_table[i],
										 sizeof(struct sockaddr_in));
					//printf("write.c: bytesSent = %d\n", bytesSent);
					if (bytesSent != temp) {
						Vmn_Log_Error("udp send error! bytesSent=%d:%s", bytesSent, strerror(errno));
						Sleep(100);
						break;
					}
				}
			}
#else
			do {
				//if not sleep, the client will recv uncatchable, image will be broken
				if (send_times > 100) {
					usleep(100);
				}
				if (len > SENDTO_PACKAGE_LEN) {
					temp = SENDTO_PACKAGE_LEN;
					len -= SENDTO_PACKAGE_LEN;
				} else {
					temp = len;
					len = 0;
				}
				for(i=0; i<maxq; i++) {
					if (conn_client->flag_table[i] == 1) {
						int bytesSent = sendto(sendsock, psend, temp, 0,
						(struct sockaddr*)&conn_client->dstaddr_table[i],
						sizeof(struct sockaddr_in));
						//printf("write.c: bytesSent = %d\n", bytesSent);
						if (bytesSent != temp) {
							Vmn_Log_Error("udp send error! bytesSent = %d, %s\n", bytesSent, strerror(errno));
							Sleep(100);
							break;
						}
					}
				}
				psend += temp;
				send_times++;
			} while (len > 0);
			send_times = 0;
		}
	}
#endif

cleanup:
		/* Make sure the other threads aren't waiting for init to complete */
	Vmn_Log_Debug("UDP Thread Cleanup\n");
	*envp->quit_flag_ptr = 1;
	
		/* Wait for demand thread cleanup and quit */
#if 0
	if (pthread_join(envp->demand_tid, &demand_ret) == 0) {
		Vmn_Log_Debug("udp.c:Wait for Demand Thread Cleanup - Success, ret=%d\n", (int)demand_ret);
	} else {
		Vmn_Log_Debug("udp.c:Wait for Demand Thread Cleanup - Failure, ret=%d\n", (int)demand_ret);
	}
#endif

	/* if udp thread init success, release conn_client's ptr members HERE, which alloc in monitor() fuction,
	 * else relese them in monitor() fuction */
	if (envp) {
		vis_ring_buffer_unregthread(pbuffer, buffer_index);
		if (sendsock != -1) {
			closesocket(sendsock);
			sendsock = -1;
		}
		free(envp);
		envp = NULL;
	}
	
	return status;
}

static SOCKET socketInit(conn_client_t *conn_client) {
	SOCKET fd;
	int opt = 1;
	in_addr_t addr;	//temp to compare ip with 0xffffffff, such as
	int buflen;
	socklen_t socklen;
	struct sockaddr_in dst_addr;

	fd = socket(AF_INET, SOCK_DGRAM,0);
	if (fd < 0) {
		return fd;
	}
	socklen = sizeof(buflen);
	getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buflen, &socklen);
	buflen *= 4;    //sp 01-06-10 double rcvbuf of udp
	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buflen, sizeof(buflen));
	socklen = sizeof(buflen);
	getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buflen, &socklen);
	printf("udp socket buflen = %d\n", buflen);

	addr = inet_addr(conn_client->dst_ipAddr);

	dst_addr.sin_family = AF_INET;
	dst_addr.sin_port = htons(*conn_client->dst_port_ptr);
	
	/* Get network type from IPAddr and set some socket option if nessery */
	/* Broadcast */
	if (((htonl(addr)&0xff) == 0xff) &&
		((htonl(addr)&0xff0000ff) > 0xef0000ff || (htonl(addr)&0xff0000ff) < 0xe00000ff)) {
		setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt));
		*conn_client->type_ptr = NETWORK_SEND_BROADCAST;
		printf("broadcast ip addr select : %s\n", conn_client->dst_ipAddr);
	/* Multicast */
	} else if (htonl(addr) >= 0xe0000000 && htonl(addr) <= 0xefffffff) {
		unsigned char loop=0,ttl=20;
		*conn_client->type_ptr = NETWORK_SEND_MULTICAST;
		setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
		setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
		printf("multicast ip addr select : %s\n", conn_client->dst_ipAddr);
	/* Video On Demand */
	} else if (htonl(addr) == 0x0) {
		*conn_client->type_ptr = NETWORK_SEND_VOD;
		printf("nocast ip addr select : %s\n", conn_client->dst_ipAddr);
	/* Unicast */
	} else {
		*conn_client->type_ptr = NETWORK_SEND_UNICAST;
		printf("unicast ip addr select : %s\n", conn_client->dst_ipAddr);
	}
		
	//if (inet_pton(AF_INET, conn_client->dst_ipAddr, &(dst_addr.sin_addr)) <= 0) {
	//	Vmn_Log_Error("self ip address error!");
	//	closesocket(fd);
	//	fd = -1;
	//}
	dst_addr.sin_addr.s_addr = inet_addr(conn_client->dst_ipAddr);
	if (*conn_client->type_ptr != NETWORK_SEND_VOD) {
		(*conn_client->conn_num_ptr) = 1;
		conn_client->flag_table[0] = 1;
		conn_client->dstaddr_table[0] = dst_addr;
		//conn_client->flag_table[(*conn_client->conn_num_ptr)-1] = 1;
		//conn_client->dstaddr_table[(*conn_client->conn_num_ptr)-1] = dst_addr;
	}

	return fd;
}

//linxj2011-08-22 UDPPORT should be unsigned short
static int connection_return(const conn_client_t *conn_client, int num, int connfd, Connection *connection)
{
	char sendbuf[MAX_BUFLEN];

	Vis_con             vis_con;
	ConnectionReturn    connection_ret;

	vis_con.vis_con_syn0='v';
	vis_con.vis_con_syn1='i';
	vis_con.vis_con_syn2='s';
	vis_con.vis_con_cmd = VIS_RS_CONNECTIONRETURN;
	memset(sendbuf, 0, MAX_BUFLEN);
	memcpy(sendbuf, &vis_con, sizeof(vis_con));
	connection_ret.size = sizeof(ConnectionReturn);
	connection_ret.connectCounter = conn_client->maxConnNum;
	if (*conn_client->type_ptr == NETWORK_SEND_VOD) {
		if (connection->UDPPORT !=0 ) {
			connection_ret.connectStatus = CONNECTION_STATUS_NOERROR;
			connection_ret.senddataType = 2; //udp type
		} 
		
		if (num < 0 || connection->UDPPORT==0) {
			connection_ret.connectStatus = CONNECTION_STATUS_PARAM;
			printf("illegal param! port=%d\n",connection->UDPPORT);
		}

		if (num == conn_client->maxConnNum) {
			if (connection->connectEnable!=1) {
				connection_ret.connectStatus = CONNECTION_STATUS_NOTINLIST;
				printf("disconnect device not in the list\n");
			} else {
				connection_ret.connectStatus = CONNECTION_STATUS_TOOMUCH;
				printf("more than eight connections\n");
			}
		}
	} else {
		connection_ret.connectStatus = CONNECTION_STATUS_INVALID;
	}
	memcpy(sendbuf + sizeof(vis_con), &connection_ret, sizeof(ConnectionReturn));
	if (send(connfd, sendbuf, sizeof(ConnectionReturn)+sizeof(vis_con), 0) == -1) {
		perror("sendto");
	}

	return 0;
}

#if 0
#ifdef SEND_BY_PACKAGE
static size_t sendto_package(SOCKET sockfd, void *buf, size_t len, int flag) {
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
		sendlen = len>PACKAGE_LEN?PACKAGE_LEN:len;
		retlen = sendto(sockfd, sendbuf, sendlen, flag);
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
#endif
