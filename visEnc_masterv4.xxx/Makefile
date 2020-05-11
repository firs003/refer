all: visEnc.master test
VIS_UTIL_FLAG = -I/opt/dvsdk_dm365/dvsdk_demos_2_10_00_17/apps/
VIS_UTIL_FLAG += -L/opt/dvsdk_dm365/dvsdk_demos_2_10_00_17/apps/vis_util/release/ -lvis_util_ti

#visEnc.master: main.c netconfig.c refresh.c monitor.c serial.c netconfig.h
visEnc.master: main.c refresh.c monitor.c serial.c
	arm_v5t_le-gcc $^ -o $@ -lpthread -Wall -Os $(VIS_UTIL_FLAG)

test: test.c
	gcc $^ -o $@ -Wall -Os

clean:
	rm -rf visEnc.master
	rm -rf test

