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

#define VISENC_MASTER		"/mnt/apps/updateFile/visEnc.master"
#define VISENC_SLAVE		"/mnt/apps/updateFile/visEnc.slave"
#define VISENC_UPDATE		"/mnt/apps/updateFile/visEnc.update"

//#define VIS_KERNEL		"/mnt/apps/updateFile/visEnc.kernel"
#define VIS_KERNEL	"/opt/dvsdk_dm365/dvsdk_demos_2_10_00_17/apps/update_enc/client"
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
#define APP		        "/mnt/apps.tar.gz"
#define APP_DIR		    "/mnt/apps"

#define HEAD_LEN 4
#define MAXLINE	102400

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
int setupdateSet (char *opt,SetProgram *updateSet,char* clientfilename){
	char filename[256];
	switch(*opt){
    	case 'a':
    		(*updateSet).size = sizeof(SetProgram);
    		(*updateSet).prgfilelen = get_file_size(VIS_KERNEL);
    		(*updateSet).ver_major = 0;
    		(*updateSet).ver_minor = 0;
    		(*updateSet).prgtype = UPDATE_TYPE_KERNEL;
    		memcpy((*updateSet).prgfilename, VIS_KERNEL,sizeof(VIS_KERNEL));
    		return 0;
    	case 'b':
    		(*updateSet).size = sizeof(SetProgram);
    		(*updateSet).prgfilelen = 0;
    		(*updateSet).ver_major = 0;
    		(*updateSet).ver_minor = 0;
    		(*updateSet).prgtype = UPDATE_TYPE_SYSCONFIG;
    		memcpy((*updateSet).prgfilename, VISENC_SYS,sizeof(VISENC_SYS));
    		return 0;
    	case 'c':
    		(*updateSet).size = sizeof(SetProgram);
    		(*updateSet).prgfilelen = 0;
    		(*updateSet).ver_major = 0;
    		(*updateSet).ver_minor = 0;
    		(*updateSet).prgtype = UPDATE_TYPE_NETCONFIG;
    		memcpy((*updateSet).prgfilename, VISENC_NET,sizeof(VISENC_NET));
    		return 0;
    	case 'd':
    		(*updateSet).size = sizeof(SetProgram);
    		(*updateSet).prgfilelen = 0;
    		(*updateSet).ver_major = 0;
    		(*updateSet).ver_minor = 0;
    		(*updateSet).prgtype = UPDATE_TYPE_APP;
    		memcpy((*updateSet).prgfilename, APP,sizeof(APP));
    		return 0;
    	case 'e':
    		(*updateSet).size = sizeof(SetProgram);
    		(*updateSet).ver_major = 0;
    		(*updateSet).ver_minor = 0;
    		(*updateSet).prgtype = UPDATE_TYPE_ADDFILE;
    		printf("server file name:\n");
    		gets((*updateSet).prgfilename);
    		printf("client file name:\n");
    		gets(clientfilename);
    		(*updateSet).prgfilelen = get_file_size(clientfilename);
    		return 0;
    	case 'f':
    		(*updateSet).size = sizeof(SetProgram);
    		(*updateSet).prgfilelen = 0;
    		(*updateSet).ver_major = 0;
    		(*updateSet).ver_minor = 0;
    		(*updateSet).prgtype = UPDATE_TYPE_DELFILE;
    		printf("file name:\n");
    		gets((*updateSet).prgfilename);
    		return 0;
    	case 'g':
    		(*updateSet).size = sizeof(SetProgram);
    		(*updateSet).prgfilelen = 0;
    		(*updateSet).ver_major = 0;
    		(*updateSet).ver_minor = 0;
    		(*updateSet).prgtype = UPDATE_TYPE_GETFILE;
    		printf("file name:\n");
    		gets((*updateSet).prgfilename);
    		return 0;
    	default:return -1;
    	}

}
int sendfile(char *opt,FILE *fp,char *sendbuf,int *sockfd, SetProgram *updateSet, vis_rs_response* response,char* clientfilename){
	int ret = 0;
	int off = 0;
	int i;
	char *name;
	char lastname[50];
	char openfilename[100];
	unsigned char head[HEAD_LEN];
	SetProgram sendtoclientSet;
	switch(*opt){
	case 'a':
		fp = fopen(VIS_KERNEL,"rb");
		if(fp == NULL){
			perror("open file error");
			return -1;
		}
		do{
			memset(sendbuf,0,MAXLINE);
			ret = fread(sendbuf, sizeof(char), MAXLINE, fp);
			if(ferror(fp)){
				fclose(fp);
				perror("fread error");
				return -1;
				}
			off = send(*sockfd, sendbuf, ret, 0);
			if(off < 0){
				perror("send error");
				return -1;
				}
		}while(!feof(fp));
		fclose(fp);
		return 0;
	case 'b':
		fp = fopen(VISENC_SYS,"rb");
		if(fp == NULL){
			perror("open file error");
			return -1;
		}
		do{
		memset(sendbuf,0,MAXLINE);
		ret = fread(sendbuf, sizeof(char), MAXLINE, fp);
		if(ferror(fp)){
			fclose(fp);
			perror("fread error");
			return -1;
			}
		off = send(*sockfd, sendbuf, ret, 0);
		if(off < 0){
			perror("send error");
			return -1;
			}
		}while(!feof(fp));
		return 0;
	case 'c':
		fp = fopen(VISENC_NET,"rb");
		if(fp == NULL){
			perror("open file error");
			return -1;
		}
		do{
		memset(sendbuf, 0, MAXLINE);
		ret = fread(sendbuf, sizeof(char), MAXLINE, fp);
		if(ferror(fp)){
			fclose(fp);
			perror("fread error");
			return -1;
			}
		off = send(*sockfd, sendbuf, ret, 0);
		if(off < 0){
			perror("send error");
			return -1;
			}
		}while(!feof(fp));
		fclose(fp);
		return 0;
	case 'd':
		fp = fopen(APP,"rb");
		if(fp == NULL){
			perror("open file error");
			return -1;
		}
		do{
		memset(sendbuf,0,MAXLINE);
		ret = fread(sendbuf, sizeof(char), MAXLINE, fp);
		if(ferror(fp)){
			fclose(fp);
			perror("fread error");
			return -1;
			}
		off = send(*sockfd, sendbuf, ret, 0);
		if(off < 0){
			perror("send error");
			return -1;
			}
		}while(!feof(fp));
		return 0;
	case 'e': 
		memcpy(openfilename, clientfilename, 100*sizeof(char));
		fp = fopen(openfilename,"rb");
		if(fp == NULL){
			perror("open file error");
			return -1;
		}
		do{
		memset(sendbuf,0,MAXLINE);
		ret = fread(sendbuf, sizeof(char), MAXLINE, fp);
		if(ferror(fp)){
			fclose(fp);
			perror("fread error");
			return -1;
			}
		off = send(*sockfd, sendbuf, ret, 0);
		if(off < 0){
			perror("send error");
			return -1;
			}
		}while(!feof(fp));
		fclose(fp);
		return 0;
	case 'f': return 0;
	case 'g':
		ret = recv(*sockfd, head, sizeof(head),0);	
		if(ret > 0) {
			if (head[0] != 'v' || head[1] != 'i' || head[2] != 's' || head[3] != VIS_RS_RESPONSE){
	    		perror("response head error");
	    		return -1;
			}
        }else{
			perror("response head error");
			return -1;
			}
		ret =recv(*sockfd, response, sizeof(vis_rs_response), 0);
		if(ret > 0) {
			if (response->vis_con_data_responseType != VIS_RQ_SETPROGRAM || response->vis_con_data_response_result != RESPONSE_TYPE_SENDTOCLIENT){
	    		perror("response error");
	    		return -1;
			}
        }else{
			perror("response error");
			return -1;
			}
        ret = recv(*sockfd, &sendtoclientSet, sizeof(SetProgram), 0);
		if (ret > 0) {
            printf("sendtoclientSet.size = %d\n", sendtoclientSet.size);
            printf("sendtoclientSet.prgfilelen = %d\n", sendtoclientSet.prgfilelen);
            printf("sendtoclientSet.prgtype = 0x%x\n", sendtoclientSet.prgtype);
            printf("sendtoclientSet.prgfilename = %s\n", sendtoclientSet.prgfilename);
		}else{
			perror("recive sendtoclientSet error");
			return -1;
			}
		name = sendtoclientSet.prgfilename;
		for(i=0; name[i]!='\0'; i++){
				;
			}
		for(;name[i]!='/';i--){
				;
			}
		memcpy(lastname,&(name[i+1]),40*sizeof(char));
		fp = fopen(lastname, "wb");
            if(fp == NULL){
				perror("open file error");
				return -1;
				}
				off = 0;
            do {
				if((sendtoclientSet.prgfilelen - off) > sizeof(sendbuf))
					ret = recv(*sockfd, sendbuf, sizeof(sendbuf), 0);
                else
					ret = recv(*sockfd, sendbuf, (sendtoclientSet.prgfilelen - off), 0);
                if (ret > 0) {
                    if (fwrite(sendbuf, 1, ret, fp) != ret) {
                        printf("fwrite failed.\n");
                        return -1;
                    }
                    off += ret;
                }
            } while(ret > 0 && off < sendtoclientSet.prgfilelen);
			fclose(fp);
            printf("received prgfilelen = %d\n", off);
            return 0;
	default:return -1;
	}
}
void checkupdate(vis_rs_response *Vis_rs_response){
	switch((*Vis_rs_response).vis_con_data_response_result){
		case RESPONSE_TYPE_UPDATE_FAIL:
			perror("update error");
			break;
		case RESPONSE_TYPE_UPDATE_OK:
			printf("update ok\n");
			break;
		case RESPONSE_TYPE_ADDFILE_FAIL:
			perror("add file error");
			break;	
		case RESPONSE_TYPE_ADDFILE_OK:
			printf("add file ok\n");
			break;		
		case RESPONSE_TYPE_DELFILE_FAIL:
			perror("delete file error\n");
			break;		
		case RESPONSE_TYPE_DELFILE_OK:
			printf("delete file ok\n");
			break;
		case RESPONSE_TYPE_GETFILE_OK:
			printf("get file ok\n");
			break;
		case RESPONSE_TYPE_GETFILE_FAIL:
			printf("get file error\n");
			break;
		default:
			perror("unkown response\n");
			break;		
	}
}
int main(void){
 	char server_ip[20];
	char opt,ch,enter;
	char sendbuf[MAXLINE];
	char clientfilename[100];
	unsigned char head[HEAD_LEN];
	int sockfd;
	int ret = 0;
	SetProgram updateSet;
	struct sockaddr_in address;
	FILE *fp = NULL;
	vis_rs_response response;
    Vis_con vis_con = { 'v', 'i', 's',VIS_RQ_SETPROGRAM};
	socklen_t addr_len = sizeof(struct sockaddr_in);
	char cmd[100];
    	printf("server ip:\n");
   	gets(server_ip);
while(1){
   	 printf(
           "update opt:\n"
    	   "a: update kernel    b: update sys\n"
    	   "c: update net       d: update app\n"
    	   "e: add file         f: delete file\n"
    	   "g: get file         q: quit\n"
   	 );
	opt = getchar();
	while((ch = getchar())!='\n'){
		continue;
		}
	
	ret = setupdateSet(&opt,&updateSet,clientfilename);

	if(ret < 0){
	 return 0;
	}
	printf("set updateSet ok\n");
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (-1 == sockfd) {
        perror("socket error");
        close(sockfd);
	    continue;
	}
	printf("socket ok\n");
	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	inet_pton(AF_INET,server_ip,&address.sin_addr.s_addr);
	address.sin_port = htons(CLIENT_UPDATETCPPORT);

	ret = connect(sockfd,(struct sockaddr *)&address,sizeof(address));
	if (ret < 0){
		perror("connect error");
		continue;
	}
	printf("connect ok\n");
	memset(sendbuf, 0, MAXLINE);
    memcpy(sendbuf, &vis_con, sizeof(vis_con));
    	ret = send(sockfd, sendbuf,sizeof(vis_con),0);
	if(ret < 0){
		perror("head send error");
		continue;
	}
	printf("send head ok\n");
	memset(sendbuf, 0, MAXLINE);
        memcpy(sendbuf, &updateSet, sizeof(updateSet));
	ret = send(sockfd, sendbuf, sizeof(updateSet), 0);
	if (ret < 0){
		perror("updateSet send error");
		continue;
	}
	printf("send updateset ok\n");
	ret = sendfile(&opt, fp, sendbuf, &sockfd, &updateSet, &response,clientfilename);
	if(ret < 0){
		perror("send or get file error\n");
		}
	ret = recv(sockfd, sendbuf, sizeof(Vis_con) + sizeof(vis_rs_response), 0);
	if(ret < 0){
		perror("recive response error");
		continue;
		}
	memcpy(head, sendbuf,sizeof(Vis_con));
        if(ret > 0) {
			if (head[0] != 'v' || head[1] != 'i' || head[2] != 's'|| head[3] !=VIS_RS_RESPONSE)
	    		perror("response head error");
        }
        memcpy(&response, sendbuf + sizeof(Vis_con), sizeof(vis_rs_response));
        checkupdate(&response);
        continue;
	}
}
