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
//#define VIS_KERNEL	"/home/codec/test1"
#define VISENC_SYS		"/mnt/apps/updateFile/viscfgEnc.system"
#define VISENC_NET		"/mnt/apps/updateFile/viscfgEnc.net"
#define KERNELBLOCK		"/dev/mtd2"
#define VISENC_DIR		"/mnt/apps/dm365_encode"
#define VISCFG_DIR		"/mnt/apps/configFile"
#define VISUPDATE_DIR	"/mnt/apps/updateFile"
//#define VISUPDATE_DIR	"/home/codec/123"
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

unsigned long get_file_size(const char *filename)  
    {  
        unsigned long size;  
        FILE* fp = fopen( filename, "rb" );  
        if(fp==NULL)  
        {  
            printf("ERROR: Open file %s failed.\n", filename);  
            return 0;  
        }  
        fseek( fp, SEEK_SET, SEEK_END );  
        size=ftell(fp);  
        fclose(fp);  
        return size;  
    } 
static void get_updateFile(char **updateFile, SetProgram *updateSet,char *filename)
{	int i;
	char *name;
	char filedir[50];
	char lastname[50];
	char lastfilename[50];
	memcpy(filedir,VISUPDATE_DIR,sizeof(VISUPDATE_DIR));
	name = updateSet->prgfilename;
    switch (updateSet->prgtype) {
    case UPDATE_TYPE_ADDFILE:
		for(i=0; name[i]!='\0'; i++){
				;
			}
		for(;name[i]!='/';i--){
				;
			}
		memcpy(lastname,&(name[i+1]),40*sizeof(char));
		sprintf(lastfilename, "%s/%s", filedir, lastname);
		memcpy(filename,lastfilename,100*sizeof(char));
		*updateFile = filename;
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
	int val = 1, ret = 0, off = 0, ret1 = 0, ret2 =0;
	char recvbuf[MAXLINE];
	char *updateFile = NULL;
	char filename[100];
	char cmd[100];
	FILE *fp = NULL;
	unsigned char head[HEAD_LEN];
	SetProgram updateSet;
	SetProgram sendtoclientSet;
    vis_rs_response response = { sizeof(vis_rs_response), VIS_RQ_SETPROGRAM, RESPONSE_TYPE_UPDATE_OK };
    Vis_con vis_con  = { 'v', 'i', 's', VIS_RS_RESPONSE};
	socklen_t addr_len = sizeof(struct sockaddr_in);
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd) {
        perror("socket error");
        return -1;
    }
    memset(&address, 0, sizeof(struct sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
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
	while(1){
	printf("Accepting connections...\n");
		client = accept(sockfd, (struct sockaddr *)&address, &addr_len);
        if (client < 0) {
            perror("accept error");
            continue;
        }
        ret = recv(client, head, sizeof(head), 0);
        if (ret > 0) {
			if (head[0] != 'v' || head[1] != 'i' || head[2] != 's' || head[3] != VIS_RQ_SETPROGRAM) {
	    		printf("head error\n");
	    		continue;
            }
        }
        ret = recv(client, &updateSet, sizeof(updateSet), 0);
        if (ret > 0) {
            printf("updateSet.size = %d\n", updateSet.size);
            printf("updateSet.prgfilelen = %d\n", updateSet.prgfilelen);
            printf("updateSet.prgtype = 0x%x\n", updateSet.prgtype);
            printf("updateSet.prgfilename = %s\n", updateSet.prgfilename);
		}else{
			perror("recive updateset error");
			continue;
			}

        if(updateSet.prgtype == UPDATE_TYPE_GETFILE){
			sendtoclientSet.size = sizeof(SetProgram);
			sendtoclientSet.prgfilelen = get_file_size(updateSet.prgfilename);
			sendtoclientSet.ver_major = 0;
    		sendtoclientSet.ver_minor = 0;
    		sendtoclientSet.prgtype = UPDATE_TYPE_SENDTOCLIENT;
    		memcpy(&sendtoclientSet.prgfilename, &updateSet.prgfilename, sizeof(updateSet.prgfilename));
			ret = send(client,  &vis_con, sizeof(Vis_con), 0);
			if(ret<0){
				perror("send response head error");
				continue;
				}
			response.vis_con_data_response_result = RESPONSE_TYPE_SENDTOCLIENT;
			ret = send(client,&response,sizeof(vis_rs_response), 0);
			if(ret<0){
				perror("send response error");
				continue;
				}
			ret = send(client, &sendtoclientSet, sizeof(SetProgram), 0);
			if(ret < 0){
				perror("send sendtoclientSet error");
				continue;
				}
			fp = fopen(updateSet.prgfilename,"rb");
		if(fp == NULL){
				perror("open file failed");
				memset(recvbuf, 0, MAXLINE);
                memcpy(recvbuf, &vis_con, sizeof(vis_con));
                response.vis_con_data_response_result = RESPONSE_TYPE_GETFILE_FAIL;
                memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                if (ret < 0)
                    printf("send getfile error to host failed\n");
				continue;
		}
			do{
				ret1 = fread(recvbuf, sizeof(char), MAXLINE, fp);
				if(ferror(fp)){
					fclose(fp);
					perror("fread error");
					memset(recvbuf, 0, MAXLINE);
					memcpy(recvbuf, &vis_con, sizeof(vis_con));
					response.vis_con_data_response_result = RESPONSE_TYPE_GETFILE_FAIL;
					memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
					ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
					if (ret < 0)
						printf("send getfile error to host failed\n");
					continue;
							}
		ret2 = send(client, recvbuf, ret1, 0);
		if(ret2 < 0){
				perror("send error");
				fclose(fp);
				memset(recvbuf, 0, MAXLINE);
                memcpy(recvbuf, &vis_con, sizeof(vis_con));
                response.vis_con_data_response_result = RESPONSE_TYPE_GETFILE_FAIL;
                memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                if (ret < 0)
                    printf("send getfile error to host failed\n");
                continue;
				}
		}while(!feof(fp));
				fclose(fp);
				printf("send response\n");
				memset(recvbuf, 0, MAXLINE);
                memcpy(recvbuf, &vis_con, sizeof(vis_con));
                response.vis_con_data_response_result = RESPONSE_TYPE_GETFILE_OK;
                memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                if (ret < 0)
                    printf("send getfile error to host failed\n");
		continue;
	}
        if (updateSet.prgtype == UPDATE_TYPE_DELFILE) {
                sprintf(cmd, "rm %s \n", updateSet.prgfilename);
                ret = system(cmd);
                if (ret == -1) {
                    memset(recvbuf, 0, MAXLINE);
                    memcpy(recvbuf, &vis_con, sizeof(vis_con));
                    response.vis_con_data_response_result = RESPONSE_TYPE_DELFILE_FAIL;
                    memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                    ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                    printf("send delete file error to host failed\n"); 
                    continue;
                }
                printf("delete file ok\n");
                memset(recvbuf, 0, MAXLINE);
                memcpy(recvbuf, &vis_con, sizeof(vis_con));
                response.vis_con_data_response_result = RESPONSE_TYPE_DELFILE_OK;
                memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                if (ret < 0){
                    printf("send delete file error to host failed\n");
						}
				continue;
            }
            get_updateFile(&updateFile, &updateSet, filename);
			printf("updateFile = %s\n", updateFile);
            fp = fopen(updateFile, "wb");
            if(fp == NULL){
				perror("open file error");
				printf("add file failed\n");
                      memset(recvbuf, 0, MAXLINE);
                      memcpy(recvbuf, &vis_con, sizeof(vis_con));
                      response.vis_con_data_response_result = RESPONSE_TYPE_ADDFILE_FAIL;
                      memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                      ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                      if (ret < 0)
                            printf("send nandwrite failed to host failed\n");
				continue;
				}
				off = 0;
            do {
				
				if((updateSet.prgfilelen - off)>= sizeof(recvbuf))
                ret = recv(client, recvbuf, sizeof(recvbuf), 0);
                else 
                ret = recv(client, recvbuf, (updateSet.prgfilelen - off), 0);
                if (ret > 0) {
                    if (fwrite(recvbuf, 1, ret, fp) != ret) {
                        printf("fwrite failed.\n");
                        break;
                    }
                    off += ret;
                }
            } while(ret > 0 && off < updateSet.prgfilelen);
			fclose(fp);
            printf("received prgfilelen = %d\n", off);
                if (updateSet.prgtype == UPDATE_TYPE_KERNEL) {
                    printf("update kernel\n");
                    sprintf(cmd, "flash_eraseall %s \n", KERNELBLOCK);
                    ret = system(cmd);
                    if (ret == -1) {
                        printf("flash_eraseall failed\n");
                        memset(recvbuf, 0, MAXLINE);
                        memcpy(recvbuf, &vis_con, sizeof(vis_con));
                        response.vis_con_data_response_result = RESPONSE_TYPE_UPDATE_FAIL;
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
                        response.vis_con_data_response_result = RESPONSE_TYPE_UPDATE_FAIL;
                        memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                        ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                        if (ret < 0)
                            printf("send flase erase failed to host failed\n");
                        continue;
                    }
                printf("update kernel ok\n");
                memset(recvbuf, 0, MAXLINE);
                memcpy(recvbuf, &vis_con, sizeof(vis_con));
                response.vis_con_data_response_result = RESPONSE_TYPE_UPDATE_OK;
                memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                if (ret < 0)
                    printf("send delete file error to host failed\n");
                continue;
				}else if (updateSet.prgtype == UPDATE_TYPE_SYSCONFIG
                        || updateSet.prgtype == UPDATE_TYPE_NETCONFIG) {
					
                    sprintf(cmd, "mv %s %s \n", updateFile, VISCFG_DIR);
                    ret = system(cmd);
                    if (ret == -1) {
                        printf("cfg file update failed\n");
                        memset(recvbuf, 0, MAXLINE);
                        memcpy(recvbuf, &vis_con, sizeof(vis_con));
                        response.vis_con_data_response_result = RESPONSE_TYPE_UPDATE_FAIL;
                        memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                        ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                        if (ret < 0)
                            printf("send nandwrite failed to host failed\n");
                        continue;
                    }
                printf("update ok\n");
                memset(recvbuf, 0, MAXLINE);
                memcpy(recvbuf, &vis_con, sizeof(vis_con));
                response.vis_con_data_response_result = RESPONSE_TYPE_UPDATE_OK;
                memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                if (ret < 0)
                    printf("send delete file error to host failed\n");
                   continue;
                }else if (updateSet.prgtype == UPDATE_TYPE_ADDFILE) {
                    sprintf(cmd, "chmod +x %s\n", updateFile);
                    ret = system(cmd);
                    sprintf(cmd, "mv %s %s \n", updateFile, updateSet.prgfilename);
                    ret = system(cmd);
                    if (ret == -1) {
                        printf("add file failed\n");
                        memset(recvbuf, 0, MAXLINE);
                        memcpy(recvbuf, &vis_con, sizeof(vis_con));
                        response.vis_con_data_response_result = RESPONSE_TYPE_ADDFILE_FAIL;
                        memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                        ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                        if (ret < 0)
                            printf("send nandwrite failed to host failed\n");
                        continue;
                    }
                printf("add file ok\n");
                memset(recvbuf, 0, MAXLINE);
                memcpy(recvbuf, &vis_con, sizeof(vis_con));
                response.vis_con_data_response_result = RESPONSE_TYPE_UPDATE_OK;
                memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                if (ret < 0)
                    printf("send delete file error to host failed\n");
                continue;					
                }else if (updateSet.prgtype == UPDATE_TYPE_APP) {
                    sprintf(cmd, "mv %s %s; rm -rf %s; cd /mnt; tar zxf %s; rm %s \n",
                            updateFile, APP, APP_DIR, APP, APP);
                    ret = system(cmd);
                    if (ret == -1) {
                        printf("add file failed\n");
                        memset(recvbuf, 0, MAXLINE);
                        memcpy(recvbuf, &vis_con, sizeof(vis_con));
                        response.vis_con_data_response_result = RESPONSE_TYPE_ADDFILE_FAIL;
                        memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                        ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                        if (ret < 0)
                            printf("send nandwrite failed to host failed\n");
                        continue;
                    }
                printf("update app ok\n");
                memset(recvbuf, 0, MAXLINE);
                memcpy(recvbuf, &vis_con, sizeof(vis_con));
                response.vis_con_data_response_result = RESPONSE_TYPE_UPDATE_OK;
                memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                if (ret < 0)
                    printf("send delete file error to host failed\n");
                continue;
                }else {
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
                    memset(recvbuf, 0, MAXLINE);
                    memcpy(recvbuf, &vis_con, sizeof(vis_con));
                    response.vis_con_data_response_result = RESPONSE_TYPE_ADDFILE_OK;//update ok
                    memcpy(recvbuf + sizeof(Vis_con), &response, sizeof(vis_rs_response));
                    ret = send(client, &recvbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
                    if (ret < 0) {
                        printf("send update ok to host failed\n");
                        continue;
                    }

				}
			}
}

