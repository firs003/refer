#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "serial.h"
#include "visconfig.h"

#define MAXLINE 1024

void handle_sigpipe(int signo)
{
	puts("SIGPIPE--The other side has been closed.\n");
	exit(0);
}

int tcp_connect() {
	struct sockaddr_in servaddr;
	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
    }

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, "192.168.18.223", &servaddr.sin_addr);
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
	struct sigaction newact, oldact;
	int sockfd, n;
    unsigned char sendbuf[BUFLEN+16];

    Vis_con vis_con;
    UserData user_data;
    //UserParams user_params;
    //SerialControl serial_control;

	newact.sa_handler = handle_sigpipe;
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	sigaction(SIGPIPE, &newact, &oldact);

    sockfd = tcp_connect();

#if 0 //for serial control test
	while (fgets((char *)serial_control.data, MAXLINE, stdin) != NULL) {
        vis_con.vis_con_syn0='v';
        vis_con.vis_con_syn1='i';
        vis_con.vis_con_syn2='s';
        vis_con.vis_con_cmd = VIS_RQ_SETSERIALDATA;
        memset(sendbuf, 0, BUFLEN+16);
        memcpy(sendbuf, &vis_con, sizeof(vis_con));

        serial_control.dataLen = strlen((char *)serial_control.data);
        serial_control.dataId = 0;
        serial_control.maxdataId = 1;
        serial_control.baudRate = 115200;
        serial_control.size = sizeof(serial_control);
        memcpy(sendbuf+sizeof(vis_con), &serial_control, sizeof(serial_control));

		write(sockfd, sendbuf, sizeof(vis_con)+sizeof(serial_control));
        sleep(1);
		n = read(sockfd, sendbuf, MAXLINE);
		if (n == 0) {
			printf("The other side has been closed.\n");
			break;
		}
		else
			write(STDOUT_FILENO, sendbuf, n);
	}

	while (1) {
        vis_con.vis_con_syn0='v';
        vis_con.vis_con_syn1='i';
        vis_con.vis_con_syn2='s';
        vis_con.vis_con_cmd = VIS_RQ_SETUSERPARAMS;
        memset(sendbuf, 0, BUFLEN+16);
        memcpy(sendbuf, &vis_con, sizeof(vis_con));

        user_params.iFrame = 1;
        user_params.size = sizeof(user_params);
        memcpy(sendbuf+sizeof(vis_con), &user_params, sizeof(user_params));

		write(sockfd, sendbuf, sizeof(vis_con)+sizeof(user_params));
        sleep(1);
		n = read(sockfd, sendbuf, MAXLINE);
		if (n == 0) {
			printf("The other side has been closed.\n");
			break;
		}
		else
			write(STDOUT_FILENO, sendbuf, n);
	}
#else
    unsigned char data[] = "he";
	while (1) {
        vis_con.vis_con_syn0='v';
        vis_con.vis_con_syn1='i';
        vis_con.vis_con_syn2='s';
        vis_con.vis_con_cmd = VIS_RQ_SETUSERDATA;
        memset(sendbuf, 0, BUFLEN+16);
        memcpy(sendbuf, &vis_con, sizeof(vis_con));

        user_data.dataLen = sizeof(data);
        memcpy(user_data.data, data, sizeof(data));
        user_data.size = sizeof(user_data);
        memcpy(sendbuf+sizeof(vis_con), &user_data, sizeof(user_data));
		write(sockfd, sendbuf, sizeof(vis_con)+sizeof(user_data));
        close(sockfd);

        sockfd = tcp_connect();
        vis_con.vis_con_syn0='v';
        vis_con.vis_con_syn1='i';
        vis_con.vis_con_syn2='s';
        vis_con.vis_con_cmd = VIS_RQ_GETUSERDATA;
        memset(sendbuf, 0, BUFLEN+16);
        memcpy(sendbuf, &vis_con, sizeof(vis_con));
		write(sockfd, sendbuf, sizeof(vis_con));

        memset(sendbuf, 0, sizeof(sendbuf));
		n = read(sockfd, sendbuf, sizeof(sendbuf));
		if (n == 0) {
			printf("The other side has been closed.\n");
			break;
        } else {
            if ((4 + sizeof(UserData)) <= n) {
                memset(&user_data, 0, sizeof(user_data));
                memcpy(&user_data, sendbuf + 4, sizeof(UserData));
            } else {
                printf("bad message, just drop VIS_RQ_SETUSERDATA:!\n");
            }
            printf("datalen = %d\n", user_data.dataLen);
            printf("data = %s\n", user_data.data);
            break;
        }
	}
#endif
	close(sockfd);

	return 0;
}
