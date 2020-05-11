#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ti/sdo/dmai/ColorSpace.h>

#include "osd.h"
//#include "font.h"
#include "visconfig.h"
#include "demo.h"

static char OsdY = 0xff;
static char OsdU = 0x80;
static char OsdV = 0x80;

//static struct wide_unicode_font16x16 *gfont;
//static unsigned int g_fontsize;

const Vis_OSD_Attrs Vis_OSD_Attrs_DEFAULT = {
	1024,
	768,
	ColorSpace_YUV420PSEMI,
	NULL
};

int OSD_ImportChar(int unicode, struct wide_unicode_font16x16 *pfont);

int OSD_UTF8_Char_YUV420SEMI(struct wide_unicode_font16x16 *font_table, int table_size, int x, int y, int imageWidth, int imageHeight,unsigned short unicode, unsigned char *buffer)
{
	unsigned char OsdY = 0;
	int i=0, j=0, pix_index=0;
	int charHeight=0, charWidth=0;
	unsigned char *pchar = NULL;

	for (i=0; i<table_size; ++i) {
		if (unicode == font_table[i].value) {
			pchar = font_table[i].data;
			break;
		}
	}
	if (i == table_size) {
		unicode = '?';
		for (i=0; i<table_size; ++i) {
			if (unicode == font_table[i].value) {
				pchar = font_table[i].data;
				break;
			}
		}
	}

	if (unicode > 0xff) {
		charWidth  = EX_FONT_UNICODE_16x16_CHS_WIDTH;
		charHeight = EX_FONT_UNICODE_16x16_CHS_HEIGHT;
		for (i=0; i<charHeight; ++i) {
			for (j=0; j<charWidth/2; ++j) {		//表中连续的两个值代表一行，0xffff-16bit-1charWidth，先显示左半边再右半边
				if (1&((pchar[i*2])>>(8-1-j))) {
					pix_index = imageWidth*(i+y)+x+j;
					if (buffer[pix_index]<120) {
						OsdY = 0xff;
					} else if (buffer[pix_index]>140) {
						OsdY = 0x00;
					}
					buffer[imageWidth*(i+y)+x+j] = OsdY;
				}
			}
			for (j=0; j<charWidth/2; ++j) {
				if (1&((pchar[i*2+1])>>(8-1-j))) {
					pix_index = imageWidth*(i+y)+x+j+8;
					if (buffer[pix_index]<120) {
						OsdY = 0xff;
					} else if (buffer[pix_index]>140) {
						OsdY = 0x00;
					}
					buffer[imageWidth*(i+y)+x+j+8] = OsdY;
				}
			}
		}
	} else {
		charWidth  = EX_FONT_UNICODE_16x16_ENG_WIDTH;
		charHeight = EX_FONT_UNICODE_16x16_ENG_HEIGHT;
		for (i=0; i<charHeight; ++i) {
			for (j=0; j<charWidth; ++j) {
				if (1&((pchar[i])>>(8-1-j))) {
					pix_index = imageWidth*(i+y)+x+j;
					if (buffer[pix_index]<110) {
						OsdY = 0xff;
					} else if (buffer[pix_index]>140) {
						OsdY = 0x00;
					}
					buffer[imageWidth*(i+y)+x+j] = OsdY;
				}
			}
		}
	}

	return 0;
}

/* display string horizontal */
int OSD_Str2Stream(Vis_OSD_Handle hVisOSD, unsigned short *str, unsigned char *buffer) {
	int imageWidth;
	int imageHeight;
	ColorSpace_Type colorSpace;
	int i = 0, ret = 0, offset = 0, x = 0, y = 0, str_len = 0;
	int (*char2stream_handle)(struct wide_unicode_font16x16*, int, int, int, int, int, unsigned short, unsigned char*) = NULL;

	if (hVisOSD==NULL || buffer==NULL) {
		printf("OSD_Str2Stream() params invalid\n");
		return -1;
	}
	if (hVisOSD->osdParams->enable==0) {
		return 0;
	}
	imageWidth = hVisOSD->imageWidth;
	imageHeight = hVisOSD->imageHeight;
	colorSpace = hVisOSD->colorSpace;
	
	switch (colorSpace) {
		case ColorSpace_YUV420PSEMI :
			char2stream_handle = OSD_UTF8_Char_YUV420SEMI;
			break;
		case ColorSpace_UYVY :
			break;
		default :
			break;
	}

	x = hVisOSD->osdParams->xpos;
	y = hVisOSD->osdParams->ypos;
	if (!str) str = hVisOSD->osdParams->text;
	for (str_len=0; str[str_len]; ++str_len);
	for (i=0; i<str_len; ++i) {
		offset = (str[i]>0xff)?EX_FONT_UNICODE_16x16_CHS_WIDTH:EX_FONT_UNICODE_16x16_ENG_WIDTH;
		ret = char2stream_handle(hVisOSD->pfont_table, hVisOSD->font_table_size, x, y, imageWidth, imageHeight, str[i], buffer);
		if (ret == -1) {
			printf("<w>Osd_Str2YUV420 ret %d\n", ret);
//			return -1;
		}
		x += offset;
	}
	return 0;
}

void OSD_SetTextColor (char OsdR, char OsdG, char OsdB)
{
	OsdY = 0.25 * OsdR + 0.504 * OsdG + 0.098 * OsdB + 16;
	OsdU = -0.148 * OsdR - 0.291 * OsdG + 0.439 * OsdB + 128;
	OsdV = 0.439 * OsdR - 0.368 * OsdG - 0.071 * OsdB + 128;
}

Vis_OSD_Handle Vis_OSD_create(Vis_OSD_Attrs *osdAttrs) {
	Vis_OSD_Handle hVisOSD = NULL;
	struct wide_unicode_font16x16 *pfont = NULL;
	int i=0, index=0, ret=0;
	
	if ((hVisOSD=(Vis_OSD_Handle)calloc(1, sizeof(Vis_OSD_Object))) == NULL) {
		perror("[E]Vis_OSD_create alloc mem for Vis_OSD_Object failed");
		return NULL;
	}
	hVisOSD->imageWidth = osdAttrs->imageWidth;
	hVisOSD->imageHeight = osdAttrs->imageHeight;
	hVisOSD->colorSpace = osdAttrs->colorSpace;
	hVisOSD->osdParams = osdAttrs->osdParams;
	if (!hVisOSD->osdParams) return NULL;	//osd params is NULL
	for (hVisOSD->font_table_size=0; hVisOSD->osdParams->text[hVisOSD->font_table_size] && hVisOSD->font_table_size<64; ++hVisOSD->font_table_size);	//get table size, 0x00 in text[] is end	

/*
//	hVisOSD->params.content.enable = 1;	//test
//	hVisOSD->params.time.enable = 1;	//test
	for (i=0; i<10; ++i) {
		hVisOSD->params.time.text[i] = (unsigned short)(i+'0');
	}
	hVisOSD->params.time.text[10] = ' ';
	hVisOSD->params.time.text[11] = '/';
	hVisOSD->params.time.text[12] = ':';
	hVisOSD->params.time.text[13] = '?';
	hVisOSD->params.time.text_len = 14;
	hVisOSD->params.time.xpos = hVisOSD->imageWidth-EX_FONT_UNICODE_16x16_ENG_WIDTH*19-5;
	hVisOSD->params.time.ypos = 10;
	hVisOSD->params.content.xpos = 5;
	hVisOSD->params.content.ypos = 10;
	hVisOSD->font_table_size = hVisOSD->params.content.text_len*hVisOSD->params.content.enable + hVisOSD->params.time.text_len*hVisOSD->params.time.enable;
*/
//	printf("channel=%d, table_size=%d, content_size=%d, time_size=%d\n",hVisOSD->params.channel, hVisOSD->font_table_size, hVisOSD->params.content.text_len, hVisOSD->params.time.text_len);
	if (hVisOSD->font_table_size > 0) {
		if ((hVisOSD->pfont_table=calloc(hVisOSD->font_table_size, sizeof(struct wide_unicode_font16x16))) == NULL) {
			perror("[E]Vis_OSD_create() alloc mem for font table failed");
			if (hVisOSD) {
				free(hVisOSD);
				hVisOSD = NULL;
			}
			return NULL;
		}
	} else {
		ERR("<w>font_table_size = %d", hVisOSD->font_table_size);
		if (hVisOSD) {
			free(hVisOSD);
			hVisOSD = NULL;
		}
		return NULL;
	}

	/* load several UCS chars to memery */
	index = 0;
	pfont = hVisOSD->pfont_table;

	if (hVisOSD->font_table_size != hVisOSD->osdParams->text_len) {
		ERR("<w>font_table_size = %d, text_len = %d", hVisOSD->font_table_size, hVisOSD->osdParams->text_len);
		if (hVisOSD) {
			free(hVisOSD);
			hVisOSD = NULL;
		}
		return NULL;
	}
	if (hVisOSD->osdParams->enable) {
		for (i=0; i<hVisOSD->osdParams->text_len; ++i, ++index) {
			ret = OSD_ImportChar(hVisOSD->osdParams->text[i], hVisOSD->pfont_table+index);
			if (ret == -1) {
				printf("[E]import char failed, index=%d\n", i);
				if (hVisOSD->pfont_table) {
					free(hVisOSD->pfont_table);
					hVisOSD->pfont_table = NULL;
				}
				if (hVisOSD) {
					free(hVisOSD);
					hVisOSD = NULL;
				}
				return NULL;
			}
		}
	}
	printf("import font table success\n");

	return hVisOSD;
}

int Vis_OSD_close(Vis_OSD_Handle hVisOSD) {
	if (hVisOSD && hVisOSD->osdParams) {
		free(hVisOSD->osdParams);
		hVisOSD->osdParams = NULL;
	}
	if (hVisOSD && hVisOSD->pfont_table) {
		free(hVisOSD->pfont_table);
		hVisOSD->pfont_table = NULL;
	}
	if (hVisOSD) {
		free(hVisOSD);
		hVisOSD = NULL;
	}
	return 0;
}

int OSD_ImportChar(int unicode, struct wide_unicode_font16x16 *pfont) {
	FILE *fp_font = NULL;
	int i = 0, eof_flag = 1;
	long offset = 0;
	unsigned int tmp[33] = {0, };
	fp_font = fopen(OSD_FONT_PATH, "r");
	if (NULL == fp_font) {
		perror("[E]open utf8.font failed");
		return -1;
	}
	do {
		fscanf(fp_font, "value=%x\t", &tmp[32]);
		if (feof(fp_font)) {
			break;
		}
		if (tmp[32] == unicode) {
			eof_flag = 0;
			break;
		}
		offset = (tmp[32]>0xff)?103:55;
		fseek(fp_font, offset, SEEK_CUR);
//		printf("tmp[32]=%04x, offset=%ld\n", tmp[32], offset);
	} while (!feof(fp_font));
	if (eof_flag) {
		printf("<w>OSD_ImportChar() can not find unicode=0x%02x\n", unicode);
		fseek(fp_font, 0L, SEEK_SET);
		fscanf(fp_font, "value=%x\t", &tmp[32]);
	}
	pfont->value = (unsigned short)tmp[32];
	if (pfont->value > 0xff) {
		fscanf(fp_font,
				"data={%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x}\n",
				&tmp[0],
				&tmp[1],
				&tmp[2],
				&tmp[3],
				&tmp[4],
				&tmp[5],
				&tmp[6],
				&tmp[7],
				&tmp[8],
				&tmp[9],
				&tmp[10],
				&tmp[11],
				&tmp[12],
				&tmp[13],
				&tmp[14],
				&tmp[15],
				&tmp[16],
				&tmp[17],
				&tmp[18],
				&tmp[19],
				&tmp[20],
				&tmp[21],
				&tmp[22],
				&tmp[23],
				&tmp[24],
				&tmp[25],
				&tmp[26],
				&tmp[27],
				&tmp[28],
				&tmp[29],
				&tmp[30],
				&tmp[31]);
		for (i=0; i<EX_FONT_UNICODE_16x16_CHS_WIDTH*2; ++i) {
			pfont->data[i] = (unsigned char)tmp[i];
		}
	} else {
		fscanf(fp_font,
				"data={%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x}\n",
				&tmp[0],
				&tmp[1],
				&tmp[2],
				&tmp[3],
				&tmp[4],
				&tmp[5],
				&tmp[6],
				&tmp[7],
				&tmp[8],
				&tmp[9],
				&tmp[10],
				&tmp[11],
				&tmp[12],
				&tmp[13],
				&tmp[14],
				&tmp[15]);
		for (i=0; i<EX_FONT_UNICODE_16x16_ENG_WIDTH*2; ++i) {
			pfont->data[i] = (unsigned char)tmp[i];
		}
	}
//	print_font(&font);
	fclose(fp_font);
	return 0;
}

void print_font(struct wide_unicode_font16x16 *pfont) {
	int i = 0;
	printf("value=%04x\tdata={", (unsigned int)pfont->value);
	fflush(stdout);
	for (i=0; i<32; ++i) {
		printf("%02x ", pfont->data[i]);
		fflush(stdout);
	}
	printf("\b \b}\n");
}

int Vis_OSD_LoadAttrs(struct vis_osd_attrs *pattrs, int imageWidth) {
	FILE *fp_osdconfig = NULL;
	EncodeOSD encOSD;
	int i;

	memset(&encOSD, 0, sizeof(EncodeOSD));
	/* read osd.config */
	fp_osdconfig = fopen(OSD_CONFIG_PATH, "rb");
	if (NULL == fp_osdconfig) {
		perror("<w>Vis_OSD_create() open osd.config failed, OSD hide");
		encOSD.content.enable = 0;
		encOSD.content.text_len = 0;
		encOSD.time.enable = 0;
		encOSD.time.text_len = 0;
//		hVisOSD->font_table_size = 0;
		return -1;
	}
#ifdef PROFILE_SAVE_IN_TXT
	fscanf(fp_osdconfig, "Channel:%d\n", &encOSD.channel);
	fscanf(fp_osdconfig, "Name:%s\n", osdtype);
	printf("osdtype=%s\n", osdtype);
	fscanf(fp_osdconfig, "Enable:%d\n", &encOSD.content.enable);
	fscanf(fp_osdconfig, "AutoText:%d\n", &encOSD.content.auto_type);
	fscanf(fp_osdconfig, "Xpos:%d\n", &encOSD.content.xpos);
	fscanf(fp_osdconfig, "Ypos:%d\n", &encOSD.content.ypos);
	fscanf(fp_osdconfig, "TextLen:%d\n", &encOSD.content.text_len);
	fscanf(fp_osdconfig, "TextTran:%d\n", &encOSD.content.text_tran);
	fscanf(fp_osdconfig, "TextColor:%d\n", &encOSD.content.text_color);
	fscanf(fp_osdconfig, "TextSize:%d\n", &encOSD.content.text_size);
	fscanf(fp_osdconfig, "BgTran:%d\n", &encOSD.content.bg_tran);
	fscanf(fp_osdconfig, "BgColor:%d\n", &encOSD.content.bg_color);
	fscanf(fp_osdconfig, "BgWidth:%d\n", &encOSD.content.bg_width);
	//TODO, import utf8 code(unsigned short)

	fscanf(fp_osdconfig, "Name:%s\n", osdtype);
	printf("osdtype=%s\n", osdtype);
	fscanf(fp_osdconfig, "Enable:%d\n", &encOSD.time.enable);
	fscanf(fp_osdconfig, "AutoText:%d\n", &encOSD.time.auto_type);
	fscanf(fp_osdconfig, "Xpos:%d\n", &encOSD.time.xpos);
	fscanf(fp_osdconfig, "Ypos:%d\n", &encOSD.time.ypos);
	fscanf(fp_osdconfig, "TextLen:%d\n", &encOSD.time.text_len);
	fscanf(fp_osdconfig, "TextTran:%d\n", &encOSD.time.text_tran);
	fscanf(fp_osdconfig, "TextColor:%d\n", &encOSD.time.text_color);
	fscanf(fp_osdconfig, "TextSize:%d\n", &encOSD.time.text_size);
	fscanf(fp_osdconfig, "BgTran:%d\n", &encOSD.time.bg_tran);
	fscanf(fp_osdconfig, "BgColor:%d\n", &encOSD.time.bg_color);
	fscanf(fp_osdconfig, "BgWidth:%d\n", &encOSD.time.bg_width);
	fscanf(fp_osdconfig, "BgHeight:%d\n", &encOSD.time.bg_height);
	fscanf(fp_osdconfig, "BgHeight:%d\n", &encOSD.time.bg_height);
	//TODO, import utf8 code(unsigned short)
#else	//PROFILE_SAVE_IN_BIN
	if(sizeof(EncodeOSD) != fread(&encOSD, 1, sizeof(EncodeOSD), fp_osdconfig)) {
		perror("[E]Vis_OSD_create() read params from osd.config error");
		fclose(fp_osdconfig);
		fp_osdconfig = NULL;
		return -1;
	}
	fclose(fp_osdconfig);
	fp_osdconfig = NULL;
#endif

	for (i=0; i<10; ++i) {
		encOSD.time.text[i] = (unsigned short)(i+'0');
	}
	encOSD.time.text[10] = ' ';
	encOSD.time.text[11] = '/';
	encOSD.time.text[12] = ':';
	encOSD.time.text[13] = '?';
	encOSD.time.text_len = 14;
//	encOSD.time.xpos = hVisOSD->imageWidth-EX_FONT_UNICODE_16x16_ENG_WIDTH*19-5;
	encOSD.time.xpos = imageWidth-EX_FONT_UNICODE_16x16_ENG_WIDTH*19-5;
	encOSD.time.ypos = 10;
	encOSD.content.xpos = 5;
	encOSD.content.ypos = 10;
//	hVisOSD->font_table_size = encOSD.content.text_len*encOSD.content.enable + encOSD.time.text_len*encOSD.time.enable;

	(pattrs+0)->osdParams = (struct osd_param *)calloc(1, sizeof(struct osd_param));
	(pattrs+1)->osdParams = (struct osd_param *)calloc(1, sizeof(struct osd_param));
	if (!(pattrs+0)->osdParams || !(pattrs+1)->osdParams) {
		printf("[E]Vis_OSD_Attrs alloc mem for osdParmas failed\n");
		return -1;
	}
	memcpy((pattrs+0)->osdParams, &encOSD.content, sizeof(struct osd_param));
	memcpy((pattrs+1)->osdParams, &encOSD.time, sizeof(struct osd_param));
//	*pattrs = encOSD.content;
//	*(pattrs+1) = encOSD.time;

	return 2;
}
