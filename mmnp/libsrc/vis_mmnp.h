#ifndef __VIS_NNMP_PRIVATE_H__
#define __VIS_NNMP_PRIVATE_H__


#ifdef __cplusplus
extern "C"
{
#endif


#include "vis_os.h"

#include "vis_mmnp_api.h"
#include "ringbuffer.h"

/* Function error codes */
#define SUCCESS             0
#define FAILURE             -1

/* Thread error codes */
#define THREAD_SUCCESS      (void *) 0
#define THREAD_FAILURE      (void *) -1

/* Global data structure */
typedef struct vmn_global_data {
	unsigned int global_handle_index;
	int log_level_mask;
	int quit;				/* Global quit flag, not use */
    pthread_mutex_t mutex;	/* Mutex to protect the global data */
} Vmn_Global_Data, vmn_global_data_t;

#define GBL_DATA_INIT { 0 }

extern struct vmn_global_data vmngbl;

#if 0
/* Functions to protect the global data */
static INLINE_FUNC int vmnGblGetQuit(void)
{
    int quit;
    pthread_mutex_lock(&vmngbl.mutex);
    quit = vmngbl.quit;
    pthread_mutex_unlock(&vmngbl.mutex);
    return quit;
}

static INLINE_FUNC void vmnGblSetQuit(void)
{
    pthread_mutex_lock(&vmngbl.mutex);
    vmngbl.quit = 1;
    pthread_mutex_unlock(&vmngbl.mutex);
}
#endif

struct ring_buffer;
typedef struct vis_mmnp_object {
	int handle_index;
	pthread_t monitor_tid;
//	pthread_t listen_tid;
//	pthread_t writer_tid;
//	sem_t sem_start;
	struct ring_buffer *pbuffer;
	struct ring_buffer *pbuffer_rsz;
	int quit_flag;
	unsigned int conn_num;
	pthread_mutex_t mutex;
	struct vis_mmnp_attrs attrs;
//	int package_callback();	//use in vis_ringbuffer_putdate(), when divide frame into different package for different protocol
} Vis_Mmnp_Object, *Vis_Mmnp_Handle;	

/* Log Macro */
static const char *log_pre[] = {
	"Vmn_Silence",
	"[Vmn_CRIT]: ",
	"[Vmn_Error]: ",
	"<Vmn_Warning>: ",
	"Vmn_Info: ",
	"__Vmn_Debug__: ",
	"Vmn_All: "
};
#ifdef WIN32
static void _mmnp_log( const char *fmt, ... )
{
		static char info[512];
		va_list list;
		va_start( list, fmt );
		vsprintf( info, fmt, list );
		va_end( list );
		printf( info );
}
#define Vmn_Log_Error  _mmnp_log
#define Vmn_Log_Warning _mmnp_log
#define Vmn_Log_Info _mmnp_log
#define Vmn_Log_Debug _mmnp_log
#define Vmn_Log_Silence _mmnp_log

//#define Vmn_Log_Error(format, args,...) printf(format, ##args)
//#define Vmn_Log_Warning(format, args,...) printf(format, ##args)
//#define Vmn_Log_Info(format, args,...) printf(format, ##args)
//#define Vmn_Log_Debug(format, args,...) printf(format, ##args)
//#define Vmn_Log_Silence(format, args,...) printf(format, ##args)

#else

#define Vmn_Log(level, format, args...) \
		do {\
			if (1<<(level-1) & vmngbl.log_level_mask) {\
				fprintf(stdout, "%s", log_pre[level]);\
				if (level <= 3) {\
					fprintf(stdout, "%s:%d:%s(): ", __FILE__, __LINE__, __func__);\
				}\
				fprintf(stdout, format, ##args);\
				if (level <= 2) {\
					fprintf(stdout, ":%s\n", strerror(errno));\
				}\
			}\
		} while(0)
#define Vmn_Log_Silence(format, args...) do{Vmn_Log(VIS_MMNP_LOGLEVEL_SILENCE, format, ##args);}while(0)
#define Vmn_Log_Crit(format, args...) do{Vmn_Log(VIS_MMNP_LOGLEVEL_CRIT, format, ##args);}while(0)
#define Vmn_Log_Error(format, args...) do{Vmn_Log(VIS_MMNP_LOGLEVEL_ERROR, format, ##args);}while(0)
#define Vmn_Log_Warning(format, args...) do{Vmn_Log(VIS_MMNP_LOGLEVEL_WARNING, format, ##args);}while(0)
#define Vmn_Log_Info(format, args...) do{Vmn_Log(VIS_MMNP_LOGLEVEL_INFO, format, ##args);}while(0)
#define Vmn_Log_Debug(format, args...) do{Vmn_Log(VIS_MMNP_LOGLEVEL_DEBUG, format, ##args);}while(0)
#define Vmn_Log_All(format, args...) do{Vmn_Log(VIS_MMNP_LOGLEVEL_ALL, format, ##args);}while(0)

#endif

/* Cleans up cleanly after a failure */
#define cleanup(x) do{status=(x);goto cleanup;}while(0)

#define DBG_STOP(is_stop)	\
	while (is_stop) {				\
		sleep(1);			\
	}

#define is_video(type) (type>=0x00 && type<0x100)
#define is_audio(type) (type>=0x100 && type<0x200)



#ifdef __cplusplus
}
#endif

#endif //__VIS_NNMP_PRIVATE_H__
