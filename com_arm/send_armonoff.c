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
 *  ������ת������
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
            default:
                return B9600;
        }
}
                                                                                                                                                                                                                                                           
/*******************************************************
 * Setup comm attr
 * com:�����ļ�������,pportinfo:�����õĶ˿���Ϣ
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
                                                                                                                                                                                                                                                           
/*********���ö˿�����*********/

	//baudrates
	baudrate = convbaud(pportinfo -> baudrate);
	cfsetispeed(&termios_new,baudrate);		//���봮������˿ڲ�����
	cfsetospeed(&termios_new,baudrate);		//���봮������˵Ĳ�����
	termios_new.c_cflag |= CLOCAL;			//����ģʽ,��֤���򲻻��Ϊ�˿ڵ�ռ����
	termios_new.c_cflag |= CREAD;

	//����ģʽ,flow control
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

	//����ģʽ,data bits
	termios_new.c_cflag &= ~CSIZE;			//����ģʽ,�����ַ���ȴ�Сλ
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

	//����ģʽ parity check
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
			/*default:
				{
					termios_new.c_cflag &= ~PARENB;		//no parity check
				}break;*/
		}

	//����ģʽ,stop bits
	stopbit = pportinfo -> stopbit;
	if(stopbit == '2')
		{
			termios_new.c_cflag |= CSTOPB;			//2 top bits
		}
	else{
			termios_new.c_cflag &= ~CSTOPB;			//1 top bits
		}

	//other attributions default
	termios_new.c_oflag &= ~OPOST;					//���ģʽ 
	termios_new.c_cc[VMIN] = 1;						//�����ַ�,��Ҫ��ȡ���ַ�����С����
	termios_new.c_cc[VTIME] = 1;					//�����ַ�,��ȡ��һ���ַ��ĵȴ�ʱ��unit:(1/10)second

	tcflush(com,TCIFLUSH);						//��������ݿ��Խ���,������
    tmp = tcsetattr(com,TCSANOW,&termios_new);	//����������,TCSANOW:���иı�������Ч
	tcgetattr(com,&termios_old);
	return(tmp);
}

/**********************************************
 * Open serial port
 * tty:�˿ں� ttyS0,ttyS1,...
 * ����ֵΪ�����ļ�������
***********************************************/
int PortOpen(pportinfo_t pportinfo)
{
	int com;		//�����ļ�������
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
 * com:����������,data: ����������,datalen:���ݳ���
 * ����ʵ�ʳ���
**************************************************/
int PortSend(int com,const void *data,int datalen)
{
	int len=0;
	len = write(com,data,datalen);		//ʵ��д��ĳ���
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
/*void PrintUage(){ 
  printf("\t\tThe is Com send programe\n");
  printf("\t\tuage:./serial_send <device> <baudrate> <databit> <parity> <flowcontrol> <stopbit> <sleep_mode>[serial_mode]\n");
  printf("\t\texample:recv /dev/ttyS0 38400 8 0 0 2 1 485\n");
  printf("\t\tserial_mode=232 422 485 or 485u\n");
  printf("\t\t mode 1 -----sleep\n\t\tmode 0------not sleep\n");
  printf("\t\t edition communication  2008-5-5   gj\n");
  sleep(1);
}
  */     	
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
	int SendLen=0;
	int count = 0;
	const char buffer[]="k";
	portinfo_t portinfo;
       	int serial_mode;
    int mode=0;	
//    char devcom[]="/dev/ttymxc1";
    char devcom[]="/dev/ttymxc1";
	strncpy(portinfo.tty,devcom,30);
	portinfo.baudrate = 4800;
        portinfo.databit = 8;
	portinfo.flowcontrol = 0;
	portinfo.parity = 0;
	portinfo.stopbit = 2;
	mode=1;
	serial_mode = 232;
 	FILE *fb; 
	com = PortOpen(&portinfo);

       if(com < 0)
     {
	printf("Error:open serial port error,errmsg=%s\n",strerror(errno));
			exit(1);
     }
	PortSet(com,&portinfo);
        printf("\t\t Use Com1 test PowerOnOFF \n");
        SendLen = PortSend(com,(const void *)buffer,strlen(buffer));
	printf("Sendchar:%s\n",buffer);
	PortClose(com);		
	return 0;

}
