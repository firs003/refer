#include <stdio.h>
#include <stdlib.h>
#include "../include/util.h"

int main(int argc, char *argv[]) {
	unsigned char *buf = (unsigned char *)malloc(64);
	print_in_hex(buf, 50, "just a test", NULL);
	free(buf);
	return 0;
}
