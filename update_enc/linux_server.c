#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "visconfig.h"

//steven 01-06-2010		update files pathname
#define VISENC_MASTER	"/mnt/apps/updateFile/visEnc.master"
#define VISENC_SLAVE	"/mnt/apps/updateFile/visEnc.slave"
#define VISENC_UPDATE	"/mnt/apps/updateFile/visEnc.update"
#define VIS_KERNEL		"/mnt/apps/updateFile/visEnc.kernel"
#define VISENC_SYS		"/mnt/apps/updateFile/viscfgEnc.system"
#define VISENC_NET		"/mnt/apps/updateFile/viscfgEnc.net"
#define KERNELBLOCK		"/dev/mtd2"
#define VISENC_DIR		"/mnt/apps/dm365_encode"
#define VISCFG_DIR		"/mnt/apps/configFile"
#define VISUPDATE_DIR	"/mnt/apps/updateFile"
#define CMEM_KO			"/mnt/apps/updateFile/cmemk.ko"
#define	DM365MMAP_KO	"/mnt/apps/updateFile/dm365mmap.ko"
#define EDMA_KO			"/mnt/apps/updateFile/edmak.ko"
#define IRQ_KO			"/mnt/apps/updateFile/irqk.ko"
#define APP			    "/mnt/apps.tar.gz"
#define APP_DIR		    "/mnt/apps"

#define HEAD_LEN 4
#define MAXLINE	102400
//#define TCP_PORT	8060		//steven 12-31-2009

#define KERNELBLOCK		"/dev/mtd2"

static void get_filename(char **filename, SetProgram *updateSet)
{
    char *ptr;
    ptr = rindex(updateSet->prgfilename, '/');
    *filename = ++ptr;
}

static void get_updateFile(char **updateFile, SetProgram *updateSet)
{
    char *filename;
    char filedir[100];

    switch (updateSet->prgtype) {
    case UPDATE_TYPE_ADDFILE:
        get_filename(&filename, updateSet);
        sprintf(filedir, "%s/%s", VISUPDATE_DIR, filename);
        *updateFile = filedir;
        break;
    case UPDATE_TYPE_MASTER:
        *updateFile = VISENC_MASTER;
        break;
    case UPDATE_TYPE_SLAVE:
        *updateFile = VISENC_SLAVE;
        break;
    case UPDATE_TYPE_UPDATE:
        *updateFile = VISENC_UPDATE;
        break;
    case UPDATE_TYPE_KERNEL:
        *updateFile = VIS_KERNEL;
        break;
    case UPDATE_TYPE_SYSCONFIG:
        *updateFile = VISENC_SYS;
        break;
    case UPDATE_TYPE_NETCONFIG:
        *updateFile = VISENC_NET;
        break;
    case UPDATE_TYPE_CMEMK:
        *updateFile = CMEM_KO;
        break;
    case UPDATE_TYPE_MMAPK:
        *updateFile = DM365MMAP_KO;
        break;
    case UPDATE_TYPE_EDMAK:
        *updateFile = EDMA_KO;
        break;
    case UPDATE_TYPE_IRQK:
        *updateFile = IRQ_KO;
        break;
    case UPDATE_TYPE_APP:
        *updateFile = APP;
        break;
    default:
        printf("Unknown file!\n");
        break;
    }
}

int main(void)
{ 
	struct sockaddr_in address;
	int sockfd;
	int client = -1;
	int val = 1, ret = 0, off = 0;
	FILE *fp = NULL;
	char *updateFile = NULL;

    SetProgram updateSet;
	char head[HEAD_LEN];
	char recvbuf[MAXLINE];
    vis_rs_response response = { sizeof(vis_rs_response), VIS_RQ_SETPROGRAM, RESPONSE_TYPE_UPDATE_OK };
    Vis_con vis_con = { 'v', 'i', 's', VIS_RS_RESPONSE };

	socklen_t addr_len = sizeof(struct sockaddr_in);	 //steven 01-04-2010
	char cmd[100];

    vis_con.vis_con_syn0='v';
    vis_con.vis_con_syn1='i';
    vis_con.vis_con_syn2='s';
    vis_con.vis_con_cmd = VIS_RS_RESPONSE;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd) {
        perror("socket error");
		return -1;
    }
    memset(&address, 0, sizeof(struct sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    //address.sin_port = htons(TCP_PORT);
    address.sin_port = htons(CLIENT_UPDATETCPPORT);

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0 ) {
		perror("set setsockopt failed");
    }    
    if (-1 == bind(sockfd, (struct sockaddr *)&address, sizeof(address))) {
		perror("fail to bind");
		close(sockfd);
		return -1;
    }
    
	if (listen(sockfd, 5) < 0) {
		perror("fail to listen");
		close(sockfd);
		return -1;
	}

//	printf("Accepting connections...\n");
	while (1) {
        if (client != -1)
            close(client);
		client = accept(sockfd, (struct sockaddr *)&address, &addr_len);
        if (client < 0) {
            perror("accept error");
           continue;
        }
        ret = recv(client, head, sizeof(head), 0);
        if (ret > 0) {
			if (head[0] != 'v' || head[1] != 'i' || head[2] != 's' || head[3] != VIS_RQ_SETPROGRAM) {
	    		printf("bad message, just drop! vis ... ... ... ...\n");
	    		continue;
            }
        } else continue;

        ret = recv(client, &updateSet, sizeof(updateSet), 0);
        if (ret > 0) {
            printf("updateSet.size = %d\n", updateSet.size);
            printf("updateSet.prgfilelen = %d\n", updateSet.prgfilelen);
            printf("updateSet.prgtype = 0x%x\n", updateSet.prgtype);
            printf("updateSet.prgfilename = %s\n", updateSet.prgfilename);

            if (updateSet.prgtype == UPDATE_TYPE_DELFILE) {
                sprintf(cmd, "rm %s \n", updateSet.prgfilename);
                ret = system(cmd);
                if (ret == -1) {
                    memset(recvbuf, 0, MAXLINE);
                    memcpy(recvbuf, &vis_con, sizeof(vis_con));
                    response.vis_con_data_response_result = RESPONSE_TYPE_DELFILE_FAIL;//update failed
                    memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                    ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                    if (ret < 0)
                        printf("send delete file error to host failed\n");
                    continue;
                }
                //sleep(1);
                memset(recvbuf, 0, MAXLINE);
                memcpy(recvbuf, &vis_con, sizeof(vis_con));
                response.vis_con_data_response_result = RESPONSE_TYPE_DELFILE_OK;//update failed
                memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                if (ret < 0)
                    printf("send delete file error to host failed\n");
                continue;
            }

            get_updateFile(&updateFile, &updateSet);
			printf("updateFile = %s\n", updateFile);

            fp = fopen(updateFile, "wb");
            do {
                ret = recv(client, recvbuf, sizeof(recvbuf), 0);
                if (ret > 0) {
                    if (fwrite(recvbuf, 1, ret, fp) != ret) {
                        printf("fwrite failed.\n");
                        break;
                    }
                    off += ret;
                }
            } while(ret > 0 && off != updateSet.prgfilelen);
			fclose(fp);
            printf("received prgfilelen = %d\n", off);

            if(off == updateSet.prgfilelen) {
                off = 0;
                if (updateSet.prgtype == UPDATE_TYPE_KERNEL) {	//erase kernel partition
                    printf("update kernel\n");
                    sprintf(cmd, "flash_eraseall %s \n", KERNELBLOCK);
                    ret = system(cmd);
                    if (ret == -1) {
                        printf("flash_eraseall failed\n");
                        memset(recvbuf, 0, MAXLINE);
                        memcpy(recvbuf, &vis_con, sizeof(vis_con));
                        response.vis_con_data_response_result = RESPONSE_TYPE_UPDATE_FAIL;//update failed
                        memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                        ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                        if (ret < 0)
                            printf("send flase erase failed to host failed\n");
                        continue;
                    }
                    sprintf(cmd, "nandwrite -p %s %s \n", KERNELBLOCK, VIS_KERNEL);
                    ret = system(cmd);     
                     
                    if (ret == -1) {
                        printf("nandwrite failed\n");
                        memset(recvbuf, 0, MAXLINE);
                        memcpy(recvbuf, &vis_con, sizeof(vis_con));
                        response.vis_con_data_response_result = RESPONSE_TYPE_UPDATE_FAIL;//update failed
                        memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                        ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                        if (ret < 0)
                            printf("send flase erase failed to host failed\n");
                        continue;
                    }
                } else if (updateSet.prgtype == UPDATE_TYPE_SYSCONFIG
                        || updateSet.prgtype == UPDATE_TYPE_NETCONFIG) {
                    sprintf(cmd, "mv %s %s \n", updateFile, VISCFG_DIR);
                    ret = system(cmd);
                    if (ret == -1) {
                        printf("cfg file update failed\n");
                        memset(recvbuf, 0, MAXLINE);
                        memcpy(recvbuf, &vis_con, sizeof(vis_con));
                        response.vis_con_data_response_result = RESPONSE_TYPE_UPDATE_FAIL;//update failed
                        memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                        ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                        if (ret < 0)
                            printf("send nandwrite failed to host failed\n");
                        continue;
                    }  					
                } else if (updateSet.prgtype == UPDATE_TYPE_ADDFILE) {
                    sprintf(cmd, "mv %s %s \n", updateFile, updateSet.prgfilename);
                    ret = system(cmd);
                    if (ret == -1) {
                        printf("add file failed\n");
                        memset(recvbuf, 0, MAXLINE);
                        memcpy(recvbuf, &vis_con, sizeof(vis_con));
                        response.vis_con_data_response_result = RESPONSE_TYPE_ADDFILE_FAIL;//update failed
                        memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                        ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                        if (ret < 0)
                            printf("send nandwrite failed to host failed\n");
                        continue;
                    }  					
                } else if (updateSet.prgtype == UPDATE_TYPE_APP) {
                    sprintf(cmd, "mv %s %s; rm -rf %s; cd /mnt; tar zxf %s; rm %s \n",
                            updateFile, APP, APP_DIR, APP, APP);
                    ret = system(cmd);
                    if (ret == -1) {
                        printf("add file failed\n");
                        memset(recvbuf, 0, MAXLINE);
                        memcpy(recvbuf, &vis_con, sizeof(vis_con));
                        response.vis_con_data_response_result = RESPONSE_TYPE_ADDFILE_FAIL;//update failed
                        memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                        ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                        if (ret < 0)
                            printf("send nandwrite failed to host failed\n");
                        continue;
                    }
                } else {		//update app file
                    sprintf(cmd, "mv %s %s \n", updateFile, VISENC_DIR);
                    ret = system(cmd);
                    if (ret == -1) {
                        printf("app file update failed\n");
                        memset(recvbuf, 0, MAXLINE);
                        memcpy(recvbuf, &vis_con, sizeof(vis_con));
                        response.vis_con_data_response_result = RESPONSE_TYPE_UPDATE_FAIL;//update failed
                        memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                        ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                        if (ret < 0)
                            printf("send nandwrite failed to host failed\n");
                        continue;
                    }
                }

                //sleep(1);
                if (updateSet.prgtype == UPDATE_TYPE_ADDFILE) {
                    memset(recvbuf, 0, MAXLINE);
                    memcpy(recvbuf, &vis_con, sizeof(vis_con));
                    response.vis_con_data_response_result = RESPONSE_TYPE_ADDFILE_OK;//update ok
                    memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                    ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                    if (ret < 0) {
                        printf("send update ok to host failed\n");
                        continue;
                    }
                } else {
                    memset(recvbuf, 0, MAXLINE);
                    memcpy(recvbuf, &vis_con, sizeof(vis_con));
                    response.vis_con_data_response_result = RESPONSE_TYPE_UPDATE_OK;//update ok
                    memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                    ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                    if (ret < 0) {
                        printf("send update ok to host failed\n");
                        continue;
                    }
                }
                //printf("send update ok to host succeed\n");
            } else {
                off = 0;
                printf("download update file failed\n");
                sprintf(cmd, "rm %s \n", updateFile);
                system(cmd);
                memset(recvbuf, 0, MAXLINE);
                memcpy(recvbuf, &vis_con, sizeof(vis_con));
                response.vis_con_data_response_result = RESPONSE_TYPE_UPDATE_FAIL;//download failed
                memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                if (ret < 0) {
                    printf("send download error to host failed\n");
                    continue;
                }
            }
        }
	}	//while(1)

    return -1;
}
