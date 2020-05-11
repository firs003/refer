#include    <stdio.h>           //printf
#include    <fcntl.h>           //open
#include    <string.h>          //bzero
#include    <stdlib.h>          //exit
#include    <sys/times.h>       //times
#include    <sys/types.h>       //pid_t
#include    <termios.h>         //termios,tcgetattr(),tcsetattr()
#include    <unistd.h>
#include    <sys/ioctl.h>       //ioctl
#include	<errno.h>
#include    "mycom.h"
#include  <unistd.h>                                                                                                                                                                                                                                                           
/***************************************************
 *  波特率转换函数
****************************************************/
int convbaud(unsigned long int baudrate)
{
    switch(baudrate)
        {
            case 0:
			{
				printf("Your baudrate is zero!\n");	
				printf("example:./serial_recv /dev/ttyS0 38400 8 0 0 2 1\n");
				exit (-1);
			}
			case 50:
                return B50;
            case 75:
                return B75;
            case 110:
                return B110;
            case 134:
				return B134;
			case 150:
                return B150;
            case 200:
                return B200;
            case 300:
                return B300;
			case 600:
                return B600;
            case 1200:
                return B1200;
            case 1800:
                return B1800;
            case 2400:
                return B2400;
            case 4800:
                return B4800;
            case 9600:
                return B9600;
            case 19200:
                return B19200;
            case 38400:
                return B38400;
            case 57600:
                return B57600;
            case 115200:
                return B115200;
            case 230400:
                return B230400;
			case 230400:
				return B230400;
			case 460800:
				return B460800;
			case 576000:
				return B576000;
			case 921600:
				return B921600;
            default:
                return B9600;
        }
}
                                                                                                                                                                                                                                                           
/*******************************************************
 * Setup comm attr
 * com:串口文件描述符,pportinfo:待设置的端口信息
 *
********************************************************/
int PortSet(int com,const pportinfo_t pportinfo)
{
    struct termios termios_old,termios_new;
    int    baudrate,tmp;
    char   databit,stopbit,parity,flowcontrol;                                                                                                                                                                                                                                                           
    bzero(&termios_old,sizeof(termios_old));
    bzero(&termios_new,sizeof(termios_new));
    cfmakeraw(&termios_new);
    tcgetattr(com,&termios_old);     		 //get the serial port attributions
                                                                                                                                                                                                                                                           
/*********设置端口属性*********/

	//baudrates
	baudrate = convbaud(pportinfo -> baudrate);
	cfsetispeed(&termios_new,baudrate);		//填入串口输入端口波特率
	cfsetospeed(&termios_new,baudrate);		//填入串口输出端的波特率
	termios_new.c_cflag |= CLOCAL;			//控制模式,保证程序不会成为端口的占有者
	termios_new.c_cflag |= CREAD;

	//控制模式,flow control
	flowcontrol = pportinfo -> flowcontrol;
	switch(flowcontrol)
		{
			case '0':
				{
					termios_new.c_cflag &= ~CRTSCTS;			//no flow control
				}break;
			case '1':
				{
					termios_new.c_cflag |=CRTSCTS;			//hardware flow control
				}break;
			case '2':
				{
					termios_new.c_cflag |=IXON|IXOFF|IXANY;	//software flow control
				}break;
		}

	//控制模式,data bits
	termios_new.c_cflag &= ~CSIZE;			//控制模式,屏蔽字符宽度大小位
	databit = pportinfo -> databit;
	switch(databit)
		{
			case '5':
				termios_new.c_cflag |= CS5;
			case '6':
				termios_new.c_cflag |= CS6;
			case '7':	
				termios_new.c_cflag |= CS7;
			default:
				termios_new.c_cflag |= CS8;
		}

	//控制模式 parity check
	parity = pportinfo->parity;
	switch(parity)
		{
			case '0':
				{
					termios_new.c_cflag &= ~PARENB;		//no parity check
				}break;
			case '1':
				{
					termios_new.c_cflag |= PARENB;		//odd check
					termios_new.c_cflag &= ~PARODD;
				}break;
			case '2':
				{
					termios_new.c_cflag |= PARENB;		//even check
					termios_new.c_cflag |= PARODD;
				}break;
		}

	//控制模式,stop bits
	stopbit = pportinfo -> stopbit;
	if(stopbit == '2')
		{
			termios_new.c_cflag |= CSTOPB;			//2 top bits
		}
	else{
			termios_new.c_cflag &= ~CSTOPB;			//1 top bits
		}

	//other attributions default
	termios_new.c_oflag &= ~OPOST;					//输出模式 
	termios_new.c_cc[VMIN] = 1;						//控制字符,所要读取的字符的最小数量
	termios_new.c_cc[VTIME] = 1;					//控制字符,读取第一个字符的等待时间unit:(1/10)second

	tcflush(com,TCIFLUSH);						//溢出的数据可以接收,但不读
    tmp = tcsetattr(com,TCSANOW,&termios_new);	//设置新属性,TCSANOW:所有改变立即生效
	tcgetattr(com,&termios_old);
	return(tmp);
}

/**********************************************
 * Open serial port
 * tty:端口号 ttyS0,ttyS1,...
 * 返回值为串口文件描述符
***********************************************/
int PortOpen(pportinfo_t pportinfo)
{
	int com;		//串口文件描述符
	com = open(pportinfo->tty,O_WRONLY);
	return (com);
	
}

/************************************************
 * Close serial port
*************************************************/
void PortClose(int com)
{
	close(com);
}

/*************************************************
 * send data
 * com:串口描述符,data: 待发送数据,datalen:数据长度
 * 返回实际长度
**************************************************/
int PortSend(int com,const void *data,int datalen)
{
	int len=0;

	len = write(com,data,datalen);		//实际写入的长度
//	printf("len=%d\n",len);	
	if(len == datalen)
		{
			return(len);
		}
	else{
			tcflush(com,TCOFLUSH);
			return -1;
		}
}

/*****************Friendly Reminding***************/
void PrintUage(){ 
  printf("\t\tThe is Com send programe\n");
  printf("\t\tuage:./serial_send <device> <baudrate> <databit> <parity> <flowcontrol> <stopbit> <sleep_mode>[serial_mode]\n");
  printf("\t\texample:recv /dev/ttyS0 38400 8 0 0 2 1 485\n");
  printf("\t\tserial_mode=232 422 485 or 485u\n");
  printf("\t\t mode 1 -----sleep\n\t\tmode 0------not sleep\n");
  printf("\t\t edition communication  2008-5-5   gj\n");
  sleep(1);
}
/*************485 or485u****************************/
       	
void RTS_HIGH(int com)
{
	int status;
	status=TIOCM_RTS;
	ioctl(com,TIOCMBIC,&status);
}
void RTS_LOW(int com)
{
	int status;
	status=TIOCM_RTS;
	ioctl(com,TIOCMBIS,&status);
}
/***************************Test*******************/
int main(int argc,char *argv[])
{
	int com = 0;
	int RecvLen;     //from Recv
	char RecvBuf[128];  //from Recv
	int SendLen=0;
	int count = 0;
	const char buffer[]="1234567890abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstuvwxyz";
	portinfo_t portinfo;
    int serial_mode;
    int mode=0;	
	clock_t start, finish; //定义计时器开始，结束
	double  duration;     
	if(argc < 8 || argc > 9)
		{
			PrintUage();
			exit(1);
		}	
	strncpy(portinfo.tty,argv[1],30);
	portinfo.baudrate = atoi(argv[2]);
    portinfo.databit = argv[3][0];
	portinfo.flowcontrol = argv[5][0];
	portinfo.parity = argv[4][0];
	portinfo.stopbit = argv[6][0];
	mode=atoi(argv[7]);
    //    serial_mode=(argv[8][0]-'0');
/*
	FILE *fb; 
	char *p=portinfo.tty;
	//printf("%c",p[9]);
	char filename[7]={'S','C','o','m'};
	filename[4]=p[10];
	filename[5]=p[11];
	*/
	
	if(argc == 9)
	{
		if(strcmp(argv[8] , "485u") == 0)
			serial_mode = 4856;
		else
	        	serial_mode=atoi(argv[8]);
	}else{
		serial_mode = 232;
	}
	
	com = PortOpen(&portinfo); 
       if(com < 0)
     {
	printf("Error:open serial port error,errmsg=%s\n",strerror(errno));
			exit(1);
     }
	PortSet(com,&portinfo);
        
	switch(serial_mode)
    {
        case 232 : 
                 printf("\t\t 232 mode test\n");
                 break;
        case 422 :
                 printf("\t\t 422 mode test\n");
                 break;
        case 485 :
                 printf("\t\t nomal 485 mode test\n");
                 RTS_LOW(com);
                 break;
        case 4856 :
                 printf("\t\t unnomal 485 mode test\n");
                 RTS_HIGH(com);
                 break;
        default:
		printf("\t\t Error: serial_mode \t\t\n");
		exit(1);
		break;
   }        
	printf("Start sending:\n");
	printf("bauterate:%d databit:%d parity:%d flowcontrol:%d stop:%d sleep_mode:%d\n",argv[2][0],argv[3][0],argv[4][0],argv[5][0],argv[6][0],argv[7][0]);
	sleep(1);
	while(1)	
		{
		SendLen = PortSend(com,(const void *)buffer,strlen(buffer));//发送完成开始计时
		start = clock();
		memset(RecvBuf,0,sizeof(RecvBuf));	
		RecvLen=read(com,RecvBuf,127); //接受到数据计时结束
		finish = clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		printf( "%f seconds\n", duration );
		printf("Sendlen=%d\n",SendLen);
		if(SendLen > 0)
				{
					count++;	
					printf("count:%d\n",count);					
				}
			else{
					printf("Error:send failed,errmsg=%s\n",strerror(errno));
				}
	   if(mode)
     	sleep(1);
		}
			PortClose(com);
			
	return 0;

}
