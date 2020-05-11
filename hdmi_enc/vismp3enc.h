/********************************************************************
* vismp3ab.h: Vismp3enc Vismp3dec is based on G.729
*********************************************************************
*	Copyright (C) 2005-2008 viscodec (http://www.viscodec.com)
*
*	Version:	1.0
*	Authors:	linxj	<zerolinxj@hotmail.com>
*	Tel:		(+86) 21-34293702
*
********************************************************************/
#ifndef __VISMP3ENC_H__
#define __VISMP3ENC_H__


#ifdef __cplusplus
extern "C"
{
#endif

#define VISMP3ENC_ERROR_DEBUG		0
#define VISMP3ENC_ERROR_IGNORE		5
	
//error type
#define VISMP3ENC_ERROR_BITRATE			0x00000000	//bitrate error
#define VISMP3ENC_ERROR_SAMPLERATE		0x00000001	//the parameter is illegal 
#define VISMP3ENC_ERROR_INPUTPCM		0x00000002	//input pcm  error
#define VISMP3ENC_ERROR_MEMALLOC		0x00000004	//malloc failed
#define VISMP3ENC_ERROR_ENCODE			0x00000008	//the data have something wrong
#define VISMP3ENC_ERROR_INPUTCONTMUL	0x00000010  //input pcm count not the multiple of 1152 or 576



//VISMP3ENC return value	
#define VISMP3ENC_ERR		0x00
//#define VISMP3ENC_NOERR		0x80
//VISMP3ENC Type
	
//Get information
#define VISMP3ENC_GETERROR			1	
//macro
	
//frame size
#define VISMP3ENC_FRAMESIZE 	80
//bits type
#define VISMP3ENC_BITSTYPE_RAW		0
#define VISMP3ENC_BITSTYPE_FILE		1
#define VISMP3ENC_BITSTYPE_SYN1		2
#define VISMP3ENC_BITSTYPE_SYN2		3

//frame type
#define VISMP3ENC_SAMPLERATE_8K    		8000      		/* 8k Hz */
#define VISMP3ENC_SAMPLERATE_16K    	16000      		/* 16k Hz */
#define VISMP3ENC_SAMPLERATE_44K1    	44100      		/* 44.1k Hz */

	// input type
#define VISMP3ENC_TYPE_STEREO		0
#define VISMP3ENC_TYPE_MONO			1

//typedef	void*	vismp3enc_t;
typedef	struct vismp3enc_t	vismp3enc_t;

typedef struct vismp3enc_params
{
	int				Bitrate;				/* 输出的码流大小*/
	int				InputSampleRate;		/* 输入的PCM的采样率*/
	int				InputType;				/* 输入的PCM格式*/
	unsigned short*	InputPCM[2];  			/* 编码输入的16位线性PCM码流缓冲区指针 */
	int				InputPcmCount[2];		/* 编码输入的PCM码流字节数目 */
	int				FrameSize;
	int				OutputBitsType;		//
	unsigned char*	OutputBits;			/* 编码输出的参数bit流缓冲区指针 */
	int				OutputBitLength;		/* 编码输出bit数目 */
	int				OutputFrameType;   	/* 帧类型*/  
	int				OutputFrameNum;   	/* 编码帧数*/  
	int				EncodeError;
}vismp3enc_params;


/******************************** API:vismp3enc *******************************************/
//Create vismp3enc_t for the codec
vismp3enc_t *Vismp3enc_Init(vismp3enc_params* params);

//Encode ,the input should be yuv420 or yuv422
int		Vismp3enc_Frame(vismp3enc_t *v, vismp3enc_params* params);

//Close , after Encode the all frame .
int 	Vismp3enc_Close(vismp3enc_t * v,vismp3enc_params *params);

//Get information:error,bytes generated,output length and so on;
int		Vismp3enc_GetStatus(vismp3enc_t * v,int status_type,vismp3enc_params *params);

//Set dynamic parameter
int		Vismp3enc_SetParams(vismp3enc_t * v,int params_type,vismp3enc_params *params);
//Get version of this lib
int 	Vismp3enc_Version(vismp3enc_t * v,int *Version_main,int *Version_sub,int *Product_ID);
/***************************** end of API:vismp3enc ***************************************/

#ifdef __cplusplus
};
#endif
#endif	//end of __VISMP3ENC_H__
