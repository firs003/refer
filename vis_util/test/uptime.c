#include <stdio.h>
#include "../include/util.h"

int main(int argc, char *argv[]) {
	char str[128] = {0, };
	int ret = vis_uptime_str(str);
	printf("ret=%d, uptime=%s\n", ret, str);
	return 0;
}
