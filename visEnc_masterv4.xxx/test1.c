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

#include "main.h"
#include "netconfig.h"
#include "refresh.h"
#include "monitor.h"
#include "serial.h"
#include "visconfig.h"
#include "slavestatus.h"


Peripheral peripheral2;



char g_resolution1[50]="HDMI_1234x567x70Hz";
char g_resolution2[50]="HDMI50i_480x50Hz";
char g_resolution3[50]="VGA_987x654x75Hz";

void gresolution_to_valum(Peripheral *peripheral,char *g_resolution){
		int frequency;
		int hz;
		char iorp;
		int otherfbl;
		if(g_resolution[0]=='V'&&g_resolution[1]=='G'&&g_resolution[2]=='A'&&g_resolution[3]=='_'){
		sscanf(g_resolution,"VGA_%dx%dx%dHz",&(peripheral->VideoWidth),&(peripheral->VideoHeight),&frequency);
		if(frequency == 60)
			peripheral->VideoType= VIDEO_TYPE_CAPTURE_VGA;
		if(frequency == 55)
			peripheral->VideoType= VIDEO_TYPE_CAPTURE_VGA_55;
		if(frequency ==70)
			peripheral->VideoType= VIDEO_TYPE_CAPTURE_VGA_70;
		if(frequency == 72)
			peripheral->VideoType= VIDEO_TYPE_CAPTURE_VGA_72;
		if(frequency == 75)
			peripheral->VideoType= VIDEO_TYPE_CAPTURE_VGA_75;
		if(frequency == 85)
			peripheral->VideoType= VIDEO_TYPE_CAPTURE_VGA_85;
		}else if(g_resolution[0]=='H'&&g_resolution[1]=='D'
		&&g_resolution[2]=='M'&&g_resolution[3]=='I'&&g_resolution[4]=='_'){
	sscanf(g_resolution+4,"_%dx%dx%dHz",&(peripheral->VideoWidth),&(peripheral->VideoHeight),&frequency);
		if(frequency == 60)
			peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI;
		if(frequency == 55)
			peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_55;
		if(frequency == 70)
			peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_70;
		if(frequency == 72)
			peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_72;
		if(frequency == 75)
			peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_75;
		if(frequency == 85)
			peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_85;
		}else{
			sscanf(g_resolution,"HDMI%d%c_%dx%dHz",&hz,&iorp,&otherfbl,&frequency);
			printf("hz = %d,iorp = %c\n",hz,iorp);
			if(hz == 50 && iorp =='i')
				peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_50i;
			else if(hz == 50 && iorp == 'p')
				peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_50p;
			else if(hz == 60 && iorp == 'i')
				peripheral->VideoType= VIDEO_TYPE_CAPTURE_HDMI_60i;
			else;
			switch (otherfbl){
				case 480:
				peripheral->VideoWidth=720;
				peripheral->VideoHeight=480;
				break;
				case 576:
				peripheral->VideoWidth=720;
				peripheral->VideoHeight=576;
				break;
				case 720:
				peripheral->VideoWidth=1280;
				peripheral->VideoHeight=720;
				break;
				case 1080:
				peripheral->VideoWidth=19200;
				peripheral->VideoHeight=1080;
				break;
				}
			}
}
int main(void){
	
	gresolution_to_valum(&peripheral2,g_resolution1);
	printf("%d  %d  %d \n",peripheral2.VideoType,peripheral2.VideoWidth,peripheral2.VideoHeight);
	gresolution_to_valum(&peripheral2,g_resolution2);
	printf("%d  %d  %d \n",peripheral2.VideoType,peripheral2.VideoWidth,peripheral2.VideoHeight);
	gresolution_to_valum(&peripheral2,g_resolution3);
	printf("%d  %d  %d \n",peripheral2.VideoType,peripheral2.VideoWidth,peripheral2.VideoHeight);
	
	}
