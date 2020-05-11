#ifndef __VIS_MMNP_PACKET_H__
#define __VIS_MMNP_PACKET_H__

#include "vis_mmnp_api.h"

extern int packet_nil(unsigned char *dest, struct vis_mmnp_avdata *avdata);
extern int packet_av2ts(unsigned char *dest, struct vis_mmnp_avdata *avdata);
extern int packet_av2c1(unsigned char *dest, struct vis_mmnp_avdata *avdata);

#endif //__VIS_MMNP_PACKET_H__
