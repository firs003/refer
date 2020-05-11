#define __DAPT_SAVE_H__
#ifdef __DAPT_SAVE_H__

#define		SAVE_BLOCK_SIZE				(1024*1024)
#define		SAVE_BUFFER_SIZE			(SAVE_BLOCK_SIZE*4)
#define		SAVE_BUFFER_MAXSIZE			(SAVE_BLOCK_SIZE*6)
#define		MAX_FILE_SIZE_MB			10
#define		MAX_FILE_SIZE				(1024*1024*MAX_FILE_SIZE_MB)
#define		MIN_FILE_SIZE_MB			1
#define		MIN_FILE_SIZE				(1024*1024*MIN_FILE_SIZE_MB)
#define     MAX_FILE_NUMS				50
#define		MAX_DISK_SPACE_FOR_DATA		(MAX_FILE_NUMS*MAX_FILE_SIZE_MB)
#define		CONST_FRAMES_SAVE_DEFAULT	25
#define		FRAMES_PER_SAVE_DEFAULT		1
#define		MAX_FILE_SIZE_MB_DEFAULT	200
//#define		RES_DEV_SIZE_DEFAULT		MAX_FILE_SIZE_MB
#define		RES_DEV_SIZE_MB_DEFAULT		0
//#define		MAX_DATA_SIZE_MB_DEFAULT	MAX_DISK_SPACE_FOR_DATA
#define		MAX_DATA_SIZE_MB_DEFAULT	0xffffffff

#define		USB_DISK_DEV_PATH			"/dev/sda1"
#define		USB_DISK_MOUNT_PATH			"/var/usbmedia/"
#define     FILELIST_PATH				"/var/usbmedia/channel_1/filelist"
#define		DPAT_SAVE_DIR_PATH			"/var/usbmedia/"
#define		DAPT_SAVE_PATH				"/var/usbmedia/savetest.264"
#define		FIFO_MOVE2SAVE_PATH			"./fifodir/fifo_move2save"
#define     CHANNEL_NAME_PRE			"channel_"
#define		SAVE_CONFIG_PATH			"/mnt/apps/configFile/save.config"		//save in ascii

#endif
