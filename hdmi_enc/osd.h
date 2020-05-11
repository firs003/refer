#define __OSD_H__
#ifdef __OSD_H__

#include <ti/sdo/dmai/ColorSpace.h>
#include "visconfig.h"

#define MAX_OSD_NUM							4
#define EX_FONT_UNICODE_16x16_CHS_WIDTH		16
#define EX_FONT_UNICODE_16x16_CHS_HEIGHT	16
#define EX_FONT_UNICODE_16x16_ENG_WIDTH		8
#define EX_FONT_UNICODE_16x16_ENG_HEIGHT	16

#define OSD_CONFIG_PATH "/mnt/apps/configFile/osd.config"
#define OSD_FONT_PATH	"/mnt/apps/configFile/unicode.font"

struct wide_unicode_font16x16 {
	unsigned short value;
	unsigned char data[32];
};

typedef struct reduce_osd_param {	//diffrent from struct in visconfig.h
//	OSDType			type;
	int				enable;		//display osd? 0 or 1, -1 for unchange
//	OSDAutoTextType	auto_type;
	int				xpos;		//position
	int				ypos;
//	unsigned short	text[64];	//osd content, customers define
	unsigned int	text_len;	//text char counts
	int				text_tran;	//osd transparency, 0~100 percent
	Color			text_color;	//text color
	int				text_size;	//zoom, percent
	int				bg_tran;	//background transparency, 0~100 percent
	Color			bg_color;	//osd background color
	int				bg_width;
	int				bg_height;
} RdcOSDParams;

typedef struct vis_osd_object {
	int imageWidth;
	int imageHeight;
	ColorSpace_Type colorSpace;
//	EncodeOSD params;
	struct osd_param *osdParams;	//ls 2013-04-09
	struct wide_unicode_font16x16 *pfont_table;
	int	font_table_size;
}Vis_OSD_Object, *pVis_OSD_Object, *Vis_OSD_Handle;

typedef struct vis_osd_attrs {
	int imageWidth;
	int imageHeight;
	ColorSpace_Type colorSpace;
	struct osd_param *osdParams;
} Vis_OSD_Attrs, *pVis_OSD_Attrs;

extern const Vis_OSD_Attrs Vis_OSD_Attrs_DEFAULT;

//int OSD_UTF8_Char_YUV420SEMI(struct wide_unicode_font16x16 *font_table, int table_size, int x, int y, int imageWidth, int imageHeight, unsigned short unicode, unsigned char *buffer);
int OSD_Str2Stream(Vis_OSD_Handle hVisOSD, unsigned short *str, unsigned char *buffer);
void OSD_SetTextColor(char OsdR, char OsdG, char OsdB);
Vis_OSD_Handle Vis_OSD_create(Vis_OSD_Attrs *osdAttrs);
int Vis_OSD_close(Vis_OSD_Handle hVisOSD);
void print_font(struct wide_unicode_font16x16 *pfont);
int Vis_OSD_LoadAttrs(struct vis_osd_attrs *pattrs, int imageWidth);

#endif
