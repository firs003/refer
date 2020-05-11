#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "visconfig.h"


int tcp_connect() {
	struct sockaddr_in servaddr;
	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
    }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, "192.168.18.123", &servaddr.sin_addr);
	//servaddr.sin_port = htons(SERV_PORT); //for serial control connect
    servaddr.sin_port = htons(CLIENT_CMDTCPPORT); //for common params set connect

	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("connect");
    }
    printf("after connect\n");

    return sockfd;
}


int main(void)
{

	int sockfd, ret;
    char sendbuf[1024];
    char recvbuf[1024];
    Vis_con vis123;
    BoardTime boardtime;
    sockfd = tcp_connect();
        vis123.vis_con_syn0='v';
        vis123.vis_con_syn1='i';
        vis123.vis_con_syn2='s';
        vis123.vis_con_cmd = VIS_RQ_GETBOARDTIME;
        memset(sendbuf, 0, 1024);
        memcpy(sendbuf, &vis123, sizeof(vis123));
		 ret = send(sockfd, sendbuf,sizeof(vis123),0);
		 printf("send ret = %d\n",ret);
		recv(sockfd, recvbuf,sizeof(recvbuf),0);
		memcpy(&boardtime, recvbuf+sizeof(vis123),sizeof(boardtime));
		
		printf("year = %d, month = %d, mday = %d\n",boardtime.year,boardtime.month,boardtime.mday);
		printf("yday = %d, wday = %d, hour = %d\n",boardtime.yday,boardtime.wday,boardtime.hour);
		printf("min = %d, sec = %d, usec = %d\n",boardtime.min,boardtime.sec,boardtime.usec);
		printf("isdst = %d\n",boardtime.isdst);
	close(sockfd);

	return 0;
}
