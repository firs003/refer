#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "visconfig.h"

#define BUFLEN SERIALCONTROL_MAXLEN
#define SERV_PORT  SERIALCONTROLTCPPORT
#define SERIALDEV "/dev/tts/1"
#define SERIAL_CONFIG_FILE	"/mnt/apps/configFile/serial.cfg"
#define SERIAL_CONFIG_PROTOCOL_TYPE_DEFAULT		0
#define SERIAL_CONFIG_CONST_BAUDRATE_DEFAULT	0

extern void *serialThrFxn(void *);

#endif
