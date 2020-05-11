#ifndef __VIS_OS_H__
#define __VIS_OS_H__

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>



#ifdef WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include "pthread_win.h" //if use it
//#include <mmsystem.h>

typedef int sem_t;
typedef int socklen_t;
typedef unsigned long in_addr_t;

#define	INLINE_FUNC __inline
#define	MSG_DONTWAIT 0 //linxj temp for debug
//#define	INET_ADDRSTRLEN 32//linxj temp for debug	//in WS2tcpip.h

#define usleep(us) Sleep((us)/1000)
#define sleep(s) Sleep((s)*1000)
//#define	sem_init(message, t, v) *(message)=v;
//#define	sem_post(message) *(message)=1;
static int sem_init(int* message,int t,int v) {*(message)=v;return 0;}
static int sem_post(int* message) {*(message)=1; return 0;}
#define	sem_wait(message) while(*(message)==0){Sleep(1);} *(message)=0;
#define GET_LAST_SOCK_ERROR() WSAGetLastError()
#define IS_SOCK_TIMEOUT() (GET_LAST_SOCK_ERROR()==WSAETIMEDOUT)

#define	__func__ "win32fuc"

#else	//linux
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>			//usleep
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/times.h>			//times
#include <sys/time.h>
#include <sys/select.h>
#include <errno.h>

typedef unsigned short	WORD;
typedef unsigned int	DWORD;
typedef DWORD*			LPDWORD;
typedef unsigned int	BOOL;
typedef unsigned char	BYTE;
typedef long			LONG;
typedef unsigned int	UINT;
typedef void*			HDC;
typedef void*			HWND;
typedef void*			LPVOID;
typedef void*			HANDLE;
typedef int				SOCKET;

#ifndef NULL
#define NULL	0
#endif
#define TRUE	0x01
#define FALSE	0x00

#define	INLINE_FUNC inline
#define CALL_METHOD
#define CALLBACK
#define	SOCKET_ERROR -1
#define	closesocket close
#define	ioctlsocket(sock,type,pval) ioctl(sock,type,(pval))
#ifndef	Sleep
#define Sleep(ms)  usleep((ms)*1000)
#endif
#define IS_SOCK_TIMEOUT() (errno==EAGAIN)
#define GET_LAST_SOCK_ERROR() errno

#endif

#endif	//__VIS_OS_H__
