/* Standard Linux headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#include "serial.h"
#include "visconfig.h"
#include "../demo_common.h"

#define SERIAL_CONFIG
//#define DYNAMIC_SERIAL_TCP_PORT
#define LSERR(FORMAT, ARGS...)	fprintf(stderr, "[E]%d %s:", __LINE__, __FILE__);fprintf(stderr, FORMAT, ##ARGS);

extern int serial_flg;

static int serv_listen(void)
{
    int listenfd, opt = 1;
    struct sockaddr_in servaddr;
#ifdef DYNAMIC_SERIAL_TCP_PORT
	extern unsigned short g_port;
#endif
    //struct timeval tv = {10, 0};

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
    }
    if ( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0 ) {
        perror("setsockopt reuse");
    }    
#if 0
    if (setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv)) < 0) {
        perror("setsockopt timeout");
    }    
#endif

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
#ifndef DYNAMIC_SERIAL_TCP_PORT
    servaddr.sin_port = htons(SERV_PORT);
#else	//ls 2013-07-23, for gainuo, dynamic serial2tcp port
	servaddr.sin_port = (g_port<0xffff-1000) ? htons(g_port+1000) : htons(g_port-1000);
	printf("!!!!!!!!!!!!!!!!!serial.c: serial_tcp_port=%hu\n", g_port);
#endif

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
		close(listenfd);
    }
	if (listen(listenfd, 4) < 0) {
		close(listenfd);
	}        

    return listenfd;
}

static int setup_com(unsigned int baudrate)
{
    int fd = -1;
    unsigned int value;
    struct termios options;

//	printf("master: baudrate = %u\n", baudrate);
    if (baudrate == 300) {
        value = B300;
    } else if (baudrate == 600) {
        value = B600;
    } else if (baudrate == 1200) {
        value = B1200;
    } else if (baudrate == 2400) {
//		printf("in 2400\n");
        value = B2400;
    } else if (baudrate == 4800) {
        value = B4800;
    } else if (baudrate == 9600) {
		printf("in 9600\n");
        value = B9600;
    } else if (baudrate == 19200) {
        value = B19200;
    } else if (baudrate == 38400) {
        value = B38400;
//    } else if (baudrate == 56000) {
//        value = B56000;
    } else if (baudrate == 57600) {
        value = B57600;
    } else if (baudrate == 115200) {
        value = B115200;
    } else {
        value = B115200;
    }

    if ((fd = open(SERIALDEV, O_RDWR | O_NOCTTY | O_NDELAY)) == -1) {
        perror("open serial device");
    }
    tcgetattr(fd, &options);

    /* Set the baud rate to baudrate */
    cfsetispeed(&options, value);
    cfsetospeed(&options, value);

    /* Enable the receiver and set local mode...*/
    options.c_cflag |= (CLOCAL | CREAD);
#if 0
    /* Set c_cflag options.*/
    options.c_cflag |= PARENB;
    options.c_cflag &= ~PARODD;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;    
#else
    /* Set c_cflag options.*/
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~PARODD;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
	options.c_iflag &= ~(INPCK|ISTRIP);
	tcsetattr(fd, TCSANOW, &options);
#endif
    /* Set c_iflag input options */
    options.c_iflag &=~(IXON | IXOFF | IXANY);
    options.c_iflag &=~(INLCR | IGNCR | ICRNL);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /* Set c_oflag output options */
    options.c_oflag &= ~OPOST;   

    /* Set the timeout options */
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 10;
    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

void *serialThrFxn(void *arg)
{
    int	listenfd, connfd = -1, comfd = -1;
    int result, write_len;
    unsigned char recvbuf[BUFLEN];
    socklen_t cliaddr_len;
    struct sockaddr_in cliaddr;
	struct serial_config scfg = {0,};
	FILE *fp = NULL;
	int scfg_err = 0;
    SerialControl serial_control;
//	int i;	//ls 2013-06-07, write 1 byte once a time

    pthread_detach(pthread_self());

	fp = fopen(SERIAL_CONFIG_FILE, "r");
	if (fp) {
		if (sizeof(struct serial_config) != fread(&scfg, 1, sizeof(scfg), fp)) {
			perror("<w>master: serial.c init scfg read from config error");
			scfg_err = 1;
		}
		fclose(fp);fp=NULL;
	} else {
		perror("<w>master: serial.c init scfg open config file failed");
		scfg_err = 1;
	}
	if (scfg_err) {
		scfg.protocol = SERIAL_CONFIG_PROTOCOL_TYPE_DEFAULT;
		scfg.const_baudrate = SERIAL_CONFIG_CONST_BAUDRATE_DEFAULT;
		printf("<w>master: serial.c init scfg for default config , protocol=%d, const_baudrate=%u\n", scfg.protocol, scfg.const_baudrate);
		scfg_err = 0;
	} else {
		printf("master: serial.c init scfg from config success, protocol=%d, const_baudrate=%u\n", scfg.protocol, scfg.const_baudrate);
	}

    listenfd = serv_listen();

    while (1) {
		printf("serial.c: serial_flg=%d\n", serial_flg);
		if (serial_flg) {
			printf("serial.c: reconfig 1\n");
			fp = fopen(SERIAL_CONFIG_FILE, "r");
			if (fp) {
				if (sizeof(struct serial_config) != fread(&scfg, 1, sizeof(scfg), fp)) {
					perror("<w>master: serial.c change scfg read from config error");
					scfg_err = 1;
				}
				fclose(fp);fp=NULL;
			} else {
				perror("<w>master: serial.c change scfg open config file failed");
				scfg_err = 1;
			}
			printf("serial.c: reconfig 2, scfg_err=%d, scfg.protocol=%d, scfg.const_baudrate=%u\n", scfg_err, scfg.protocol, scfg.const_baudrate);
			if (scfg_err) {
				scfg.protocol = SERIAL_CONFIG_PROTOCOL_TYPE_DEFAULT;
				scfg.const_baudrate = SERIAL_CONFIG_CONST_BAUDRATE_DEFAULT;
				scfg_err = 0;
			} else {
				printf("master: serial.c change scfg from config success, protocol=%d, const_baudrate=%u\n", scfg.protocol, scfg.const_baudrate);
			}
			printf("serial.c: reconfig 3\n");
			serial_flg = 0;
		}
		if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len)) == -1) {
            perror("serial.c: accept");
            continue;
        }
//		printf("serial.c: after accept\n");
        result = recv(connfd, recvbuf, sizeof(recvbuf), 0);
        if (result == -1) {
            perror("serial.c: recv");
            close(connfd);
            continue;
        } else if (result == 0) {
            perror("serial.c: recv");
            close(connfd);
            continue;
        }
        if (recvbuf[0] != 'v' || recvbuf[1] != 'i' || recvbuf[2] != 's') {
            printf("bad sync, just drop!\n");
            close(connfd);
            continue;
        }

        if (recvbuf[3] == VIS_RQ_SETSERIALDATA ) {
            if ((4 + sizeof(serial_control)) <= result) {
                memcpy(&serial_control, recvbuf + 4, sizeof(serial_control));
            } else {
                printf("bad message, just drop VIS_RQ_SETSERIALDATA!\n");
                close(connfd);
                continue;
            }
			printf("serial.c: recv_len=%d, scfg.const_baudrate=%u\n", result, scfg.const_baudrate);
			if (scfg.const_baudrate) {
				comfd = setup_com(scfg.const_baudrate);
			} else {
				comfd = setup_com(serial_control.baudrate);
			}
#if 1
            write_len = write(comfd, serial_control.data, serial_control.dataLen);
//			printf("serial.c: writer_len=%d\n", write_len);
#else
			printf("serial.c: baudrate=%d, dataLen=%d, data=[", serial_control.baudrate, serial_control.dataLen);
			for (i=0; i<serial_control.dataLen; ++i) {
				if (write(comfd, serial_control.data+i, 1) < 0) LSERR("write to comfd failed\n");
				printf("%02hhx ", serial_control.data[i]);
				usleep(1000);
			}
			printf("\b \b]\n");
#endif			
            close(comfd);
        }
        close(connfd);
        connfd = -1;
    }

    return 0;
}
