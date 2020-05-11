#CROSS_COMPILER = /opt/mv_pro_5.0/montavista/pro/devkit/arm/v5t_le/bin/arm_v5t_le-
#INCL = -I/root/from_yin/linux-2.6.18_pro500/include
all: visEnc.update
CROSS_COMPILER = arm_v5t_le-

visEnc.update:
#	$(CROSS_COMPILER)gcc $(INCL) linux_server.c -o netTest -Wall
#	$(CROSS_COMPILER)gcc linux_server.c -o visEnc.update -Wall -Os
	$(CROSS_COMPILER)gcc linux_server3_board.c -o visEnc.update -Wall -Os
	
clean:
	-rm -f visEnc.update
