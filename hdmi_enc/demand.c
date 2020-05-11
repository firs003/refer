/* Standard Linux headers */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "demand.h"
#include "visconfig.h"
#include "vis_common.h"
#include "../demo.h"

#define MAX_BUFLEN 64
#define SERV_PORT  CLIENT_CONNECTTCPPORT

extern conn_client_t conn_client;

//linxj2011-08-22 UDPPORT should be unsigned short
int connection_return(int num, int connfd, Connection *connection)
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
    connection_ret.connectCounter = MAX_CONNECTION;
    if (conn_client.current == NOCAST) {
        if (connection->UDPPORT !=0 ) {
            connection_ret.connectStatus = CONNECTION_STATUS_NOERROR;
            connection_ret.senddataType = 2; //udp type
        } 
        
        if (num < 0 || connection->UDPPORT==0) {
            connection_ret.connectStatus = CONNECTION_STATUS_PARAM;
            printf("illegal param! port=%d\n",connection->UDPPORT);
        }

        if (num == MAX_CONNECTION) {
            if (connection->connectEnable!=1) {
                connection_ret.connectStatus = CONNECTION_STATUS_NOTINLIST;
                printf("disconnect device not in the list\n");
            }else
            {
                connection_ret.connectStatus = CONNECTION_STATUS_TOOMUCH;
                printf("more than eight connections\n");
            }
        }
    } else 
        connection_ret.connectStatus = CONNECTION_STATUS_INVALID;
	    memcpy(sendbuf + sizeof(vis_con), &connection_ret, sizeof(ConnectionReturn));
    if (send(connfd, sendbuf, sizeof(ConnectionReturn)+sizeof(vis_con), 0) == -1) {
        perror("sendto");
    }

    return 0;
}

/******************************************************************************
 * demandThrFxn
 ******************************************************************************/
void *demandThrFxn(void *arg)
{
    DemandEnv	*envp            = (DemandEnv *) arg;
    void 		*status          = THREAD_SUCCESS;
    int			listenfd, connfd = 0, opt = 1;
    int         result, i = 0;
    char recvbuf[MAX_BUFLEN];
//    char sendbuf[MAX_BUFLEN];
    struct sockaddr_in servaddr, cliaddr;
    struct timeval tv;
    socklen_t cliaddr_len = sizeof(cliaddr);
    unsigned int sec = 0;

//    Vis_con             vis_con;
    Connection          connection;
//    ConnectionReturn    connection_ret;
//	memset(&cliaddr, 1, sizeof(cliaddr));	//test

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        goto cleanup;
    }
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        goto cleanup;
    }

    listen(listenfd, MAX_CONNECTION);

    /* Signal that initialization is done and wait for other threads */
    Rendezvous_meet(envp->hRendezvousInit);
    printf("demand.c: after meet quit_flag = %d \n", gblGetQuit());

    while (!gblGetQuit()) {
        if (conn_client.current == NOCAST) {
            for (i=0; i<MAX_CONNECTION; i++) {
                if (conn_client.flag[i] == 1) {
                    gettimeofday(&tv, NULL);
                    sec = tv.tv_sec -conn_client.time[i];
                    if (sec > 60) {
                        pthread_mutex_lock(&conn_client.mutex);
                        conn_client.num_connect--;
                        conn_client.flag[i] = 0;
                        conn_client.time[i] = 0;
                        pthread_mutex_unlock(&conn_client.mutex);
                        printf("ip = 0x%x port = %hu, time out\n", (unsigned int)conn_client.dest_addr[i].sin_addr.s_addr, ntohs(conn_client.dest_addr[i].sin_port));
                    }
                }
            }
        }
		if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len)) == -1) {
            continue;
        }
//		printf("1.demand.c: connfd=%d, client:ipaddr=0x%08x, port=%d\n", connfd, cliaddr.sin_addr.s_addr, cliaddr.sin_port);
        if ((result = recv(connfd, recvbuf, sizeof(recvbuf), 0)) == -1) {
            printf("receive error, continue\n");
            close(connfd);
            continue;
        }
        if (recvbuf[0] != 'v' || recvbuf[1] != 'i' || recvbuf[2] != 's') {
            printf("bad sync, just drop!\n");
            close(connfd);
            continue;
        }

        if (recvbuf[3] == VIS_RQ_CONNECTION ) {
            if ((4 + sizeof(Connection)) <= result) {
                memcpy(&connection, recvbuf + 4, sizeof(Connection));
            } else {
                printf("bad message, just drop VIS_RQ_CONNECTION!\n");
                close(connfd);
                continue;
            }
            if (conn_client.current == NOCAST) {
                i=-1;   //nothing match
                if (connection.connectEnable && connection.UDPPORT!=0) {
                    /* Already connected, refresh the timestamp */
                    for (i=0; i<MAX_CONNECTION; i++) {
                        if ( conn_client.flag[i] == 1 && conn_client.dest_addr[i].sin_port == htons(connection.UDPPORT)
                            && memcmp(&conn_client.dest_addr[i].sin_addr, &cliaddr.sin_addr, sizeof(cliaddr.sin_addr)) == 0) {
                            gettimeofday(&tv, NULL);
                            conn_client.time[i] = tv.tv_sec;
                            //printf("ip = 0x%x, port = %d already in send queue\n", cliaddr.sin_addr, connection.UDPPORT);
                            break;
                        }
                    }
                    if (i < MAX_CONNECTION) {
                        connection_return(i, connfd, &connection);
                        close(connfd);
                        continue;
                    }

					/* New connection request */
                    for (i=0; i<MAX_CONNECTION; i++) {
                        if (conn_client.flag[i] == 0) {
                            pthread_mutex_lock(&conn_client.mutex);
                            conn_client.num_connect++;
                            conn_client.dest_addr[i] = cliaddr;
                            conn_client.dest_addr[i].sin_family = AF_INET;
                            conn_client.dest_addr[i].sin_port = htons(connection.UDPPORT);
                            conn_client.flag[i] = 1;
                            gettimeofday(&tv, NULL);
                            conn_client.time[i] = tv.tv_sec;
                            pthread_mutex_unlock(&conn_client.mutex);
                            printf("new connect:ip = 0x%x, port = %hu\n", (unsigned int)cliaddr.sin_addr.s_addr, connection.UDPPORT);
                            break;
                        }
                    }
                } else if (connection.connectEnable==0 && connection.UDPPORT!=0) {
                    printf("connectDisable\n");
                    for (i=0; i<MAX_CONNECTION; i++) {
                        if (conn_client.flag[i]==1 && conn_client.dest_addr[i].sin_port == htons(connection.UDPPORT)
                            && memcmp(&conn_client.dest_addr[i].sin_addr, &cliaddr.sin_addr, sizeof(cliaddr.sin_addr)) == 0) {
                            pthread_mutex_lock(&conn_client.mutex);
                            conn_client.num_connect--;
                            conn_client.flag[i] = 0;
                            conn_client.time[i] = 0;
                            pthread_mutex_unlock(&conn_client.mutex);
                            printf("ip = 0x%x, port = %hu\n", (unsigned int)cliaddr.sin_addr.s_addr, connection.UDPPORT);
                            break;
                        }
                    }
                }
            }

            connection_return(i, connfd, &connection);
            close(connfd);
#if 0
            vis_con.vis_con_syn0='v';
            vis_con.vis_con_syn1='i';
            vis_con.vis_con_syn2='s';
            vis_con.vis_con_cmd = VIS_RS_CONNECTIONRETURN;
            memset(sendbuf, 0, MAX_BUFLEN);
            memcpy(sendbuf, &vis_con, sizeof(vis_con));
            connection_ret.size = sizeof(ConnectionReturn);
            connection_ret.connectCounter = MAX_CONNECTION;
            if (conn_client.current == NOCAST) {
                if (connection.UDPPORT >=0 ) {
                    connection_ret.connectStatus = CONNECTION_STATUS_NOERROR;
                    connection_ret.senddataType = 2; //udp type
                } else if (connection.UDPPORT < 0) {
                    connection_ret.connectStatus = CONNECTION_STATUS_NOTINLIST;
                } else if (i == MAX_CONNECTION) {
                    connection_ret.connectStatus = CONNECTION_STATUS_TOOMUCH;
                }
            } else 
                connection_ret.connectStatus = CONNECTION_STATUS_INVALID;
            memcpy(sendbuf + sizeof(vis_con), &connection_ret, sizeof(ConnectionReturn));
            if (send(connfd, sendbuf, sizeof(ConnectionReturn)+sizeof(vis_con), 0) == -1) {
                perror("sendto");
            }
#endif
        }
    }

cleanup:
    gblSetQuit();

    /* Make sure the other threads aren't waiting for init to complete */
    Rendezvous_force(envp->hRendezvousInit);

    /* Meet up with other threads before cleaning up */
    Rendezvous_meet(envp->hRendezvousCleanup);

    if (connfd != -1) {
	    close(connfd);
    }
    return status;
}
