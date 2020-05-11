#include <stdio.h>
#include <stdlib.h>

#include "../../demo_common.h"


/**********************************************************************
 * function:get system uptime
 * input:	uptime str for output
 * output:	if function success or failure
 **********************************************************************/
#define DAY_TOKEN (3600*24)		//seconds for a day
#define HOUR_TOKEN 3600			//seconds in an hour
#define MIN_TOKEN 60			//seconds in a minite
int vis_uptime_str(char *outstr) {
	if (outstr == NULL) {
		ERR("params invalid");
		return -1;
	}
	char buf[32] = {0, };
	int time[4] = {0, };
	int i;
	FILE *fp = fopen("/proc/uptime", "r");
	if (fp == NULL) {
		return -1;
	}

	if (-1 == fread(buf, 1, sizeof(buf), fp)) {
		fclose(fp);
		return -1;
	}
//	printf("1.buf=%s\n", buf);
	for(i=0; buf[i]>='0'&&buf[i]<='9'; ++i);
	buf[i] = 0;
//	printf("2.buf=%s\n", buf);
	time[0] = atoi(buf);
	time[1] = time[0]/HOUR_TOKEN;
	time[2] = time[0]%HOUR_TOKEN/MIN_TOKEN;
	sprintf(outstr, "%d:%d", time[1], time[2]);
	if(fp) fclose(fp);
	return 0;
}
#undef DAY_TOKEN
#undef HOUR_TOKEN
#undef MIN_TOKEN

/**********************************************************************
 * function:print info in format like Ultra Edit
 * input:	buf to print,
 * 			length to print, 
 * 			prestr before info, 
 * 			endstr after info
 * output:	void
 **********************************************************************/
void print_in_hex(void *buf, size_t len, char *pre, char *end) {
	int i, j, k, row=(len>>4);
	if (buf == NULL) {
		ERR("params invalid, buf=%p", buf);
		return;
	}
	if (pre) printf("%s:\n", pre);
	for (i=0, k=0; k<row; ++k) {
		printf("\t[0%02d0] ", k);
		for (j=0; j<8; ++j, ++i) printf("%02hhx ", *((unsigned char *)buf+i));
		printf("  ");
		for (j=8; j<16; ++j, ++i) printf("%02hhx ", *((unsigned char *)buf+i));
		printf("\n");
	}
	if (len&0xf) {
		printf("\t[0%02d0] ", k);
		for (k=0; k<(len&0xf); ++k, ++i) {
			if (k==4) printf("  ");
			printf("%02hhx ", *((unsigned char *)buf+i));
		}
		printf("\n");
	}
	if (end) printf("%s", end);
	printf("\n");
}
