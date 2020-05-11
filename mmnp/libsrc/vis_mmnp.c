#include <pthread.h>

#include "vis_mmnp.h"
#include "monitor.h"
#include "ringbuffer.h"

struct vmn_global_data vmngbl;

/***********************************************************************
 * API
 ***********************************************************************/

void *vis_mmnp_create(struct vis_mmnp_attrs *attrs) {
	int err_flag = 0;
//	int log_level = VIS_MMNP_LOGLEVEL_DEFAULT;
	int log_level = VIS_MMNP_LOGLEVEL_DEBUG;
	Vis_Mmnp_Handle visMmnpHandle;

//	Vmn_Log_Debug("in create network_type=%d\n", attrs->network_type);
	if (pthread_mutex_init(&vmngbl.mutex, NULL)) {
		Vmn_Log_Error("init gbl.mutex error");
		goto cleanup;
	}
	vis_mmnp_setopt(NULL, VIS_MMNP_OPT_TYPE_SET_LOGLEVEL, (void *)&log_level, sizeof(log_level));
	if (attrs == NULL) {
		Vmn_Log_Warning("invalid params\n");
		err_flag = 1;
		goto cleanup;
	}
	visMmnpHandle = (Vis_Mmnp_Handle)calloc(1, sizeof(Vis_Mmnp_Object));
	if (visMmnpHandle == NULL) {
		Vmn_Log_Error("alloc memery for Vis_Mmnp_Object failed");
		err_flag = 1;
		goto cleanup;
	}

//	visMmnpHandle->handle_index = ++global_handle_index;
//	sem_init(&visMmnpHandle->sem_start, 0, 0);

	/* Get packet type from network type */
	if (attrs->packet_type <= 0) {
		switch (attrs->network_type) {
			case NETWORK_SEND_BROADCAST :
			case NETWORK_SEND_MULTICAST :
			case NETWORK_SEND_UNICAST :
			case NETWORK_SEND_VOD :
			case NETWORK_SEND_UDP :
			case NETWORK_SEND_TCP :
				attrs->packet_type = VIS_MMNP_PACKET_TYPE_TS;
				break;
			case NETWORK_SEND_C1 :
				attrs->packet_type = VIS_MMNP_PACKET_TYPE_C1;
				break;
			default :
//				attrs->packet_type = VIS_MMNP_PACKET_TYPE_NIL;
				break;
		}
	}
	visMmnpHandle->pbuffer = vis_ring_buffer_send_init(0, attrs->packet_type, attrs->packet_custom);
	if (visMmnpHandle->pbuffer==NULL) {
		Vmn_Log_Error("init Ringbuffer failed");
		err_flag = 1;
		goto cleanup;
	}
	if (attrs->network_type == NETWORK_SEND_RTSP) {
		visMmnpHandle->pbuffer_rsz = vis_ring_buffer_send_init(2*1024*1024, attrs->packet_type, attrs->packet_custom);
		if (visMmnpHandle->pbuffer_rsz==NULL) {
			Vmn_Log_Error("init Ringbuffer Resize failed");
			err_flag = 1;
			goto cleanup;
		}
	}
	visMmnpHandle->conn_num = 0;
	if (pthread_mutex_init(&visMmnpHandle->mutex, NULL)) {
		err_flag = 1;
		Vmn_Log_Error("init hVmn.mutex error");
		goto cleanup;
	}
	memcpy(&visMmnpHandle->attrs, attrs, sizeof(struct vis_mmnp_attrs));

cleanup:
	if (err_flag) {
		if (visMmnpHandle) {
			if (visMmnpHandle->pbuffer) {
				vis_ring_buffer_close(visMmnpHandle->pbuffer);
				visMmnpHandle->pbuffer = NULL;
			}
			if (visMmnpHandle->pbuffer_rsz) {
				vis_ring_buffer_close(visMmnpHandle->pbuffer_rsz);
				visMmnpHandle->pbuffer_rsz = NULL;
			}
			pthread_mutex_destroy(&visMmnpHandle->mutex);
			visMmnpHandle->conn_num = 0;
//			--global_handle_index;
			free(visMmnpHandle);
			visMmnpHandle = NULL;
		}
	}
	return (void *)visMmnpHandle;
}

int vis_mmnp_start(void *visMmnpHandle) {
	/* Create Monitor Thread with Type Paramter */
//	Vmn_Log_Debug("in start network_type=%d, hVmn[@%p]=%p\n", ((Vis_Mmnp_Handle)visMmnpHandle)->attrs.network_type, &visMmnpHandle, visMmnpHandle);
	return (visMmnpHandle) ? pthread_create(&((Vis_Mmnp_Handle)visMmnpHandle)->monitor_tid, NULL, vmn_monitor, visMmnpHandle) : -2;
}

int vis_mmnp_stop(void *visMmnpHandle) {
	void *listen_thread_ret = (void *)1;
	if (visMmnpHandle && !((Vis_Mmnp_Handle)visMmnpHandle)->quit_flag) {
		((Vis_Mmnp_Handle)visMmnpHandle)->quit_flag = 1;
		//Vmn_Log_Debug("1.Stop Vmn, Listen Thread tid=%u\n", (unsigned int)((Vis_Mmnp_Handle)visMmnpHandle)->monitor_tid);
//		if (((Vis_Mmnp_Handle)visMmnpHandle)->monitor_tid != 0) {
//		if (((Vis_Mmnp_Handle)visMmnpHandle)->monitor_tid)  //FIXME
		{
			Vmn_Log_Debug("1.Stop Vmn, Listen Thread tid=%u\n", (unsigned int)((Vis_Mmnp_Handle)visMmnpHandle)->monitor_tid); //linxj
			pthread_join(((Vis_Mmnp_Handle)visMmnpHandle)->monitor_tid, &listen_thread_ret);
			Vmn_Log_Debug("2.Stop Vmn, Listen Thread return %d\n", (int)listen_thread_ret);
		}
		//TODO, thread return unexcept
	}
	return (int)listen_thread_ret;
}

int vis_mmnp_close(void *visMmnpHandle) {
	int ret = 0;
	Vis_Mmnp_Handle hVmn = (Vis_Mmnp_Handle)visMmnpHandle;
	if (hVmn) {
		if (!hVmn->quit_flag) {
			ret = vis_mmnp_stop(visMmnpHandle);
		}
		if (hVmn->pbuffer) {
			vis_ring_buffer_close(hVmn->pbuffer);
			hVmn->pbuffer = NULL;
		}
		if (hVmn->pbuffer_rsz) {
			vis_ring_buffer_close(hVmn->pbuffer_rsz);
			hVmn->pbuffer_rsz = NULL;
		}
		pthread_mutex_destroy(&hVmn->mutex);
		hVmn->conn_num = 0;
		free(visMmnpHandle);
	}
	return ret;
}

int vis_mmnp_getopt(void *visMmnpHandle, enum vis_mmnp_opt_type optType, void *buf, size_t buflen) {
	int ret = 0;
	switch (optType) {
		case VIS_MMNP_OPT_TYPE_GET_LOGLEVEL :
		{
			if (buf && buflen==sizeof(int)) {
				int log_level;
				for (log_level=0; vmngbl.log_level_mask>>log_level; ++log_level);
				memcpy(buf, &log_level, buflen);
			}
			break;
		}
		default :
			break;
	}
	return ret;
}

int vis_mmnp_setopt(void *visMmnpHandle, enum vis_mmnp_opt_type optType, void *buf, size_t buflen) {
	int ret = 0;

	switch (optType) {
		case VIS_MMNP_OPT_TYPE_SET_LOGLEVEL :
		{
			if (buf == NULL) {
				Vmn_Log_Warning("Invalid params, buf=%p\n", buf);	//can't be printed
				return -2;
			}
			if (buflen==sizeof(int)) {
				vmngbl.log_level_mask = 0xffffffff >> (8*sizeof(int)-(*(int *)buf));
				Vmn_Log_Debug("mask=%04x\n", (unsigned int)vmngbl.log_level_mask);
			}
			break;
		}
		case VIS_MMNP_OPT_TYPE_SET_UDP_DYNAMIC_PARAMS :
		{
			Vis_Mmnp_Handle handle = (Vis_Mmnp_Handle)visMmnpHandle;
			if (visMmnpHandle == NULL || buf == NULL) {
				Vmn_Log_Warning("Invalid params, handle=%p, buf=%p\n", visMmnpHandle, buf);
				return -2;
			}
			if (buflen==sizeof(struct vis_mmnp_udp_dynamic_params)
				&& (handle->attrs.network_type>=NETWORK_SEND_BROADCAST && handle->attrs.network_type<=NETWORK_SEND_UDP)) {
				struct vis_mmnp_udp_dynamic_params *params = (struct vis_mmnp_udp_dynamic_params *)buf;
				if (strncmp(params->dst_ip,handle->attrs.dst_ip,16) || params->dst_port!=handle->attrs.dst_port) {
					Vmn_Log_Debug("Change UDP dynamic params:[%s:%hu] to [%s:%hu]\n", handle->attrs.dst_ip, handle->attrs.dst_port, params->dst_ip, params->dst_port);
					memset(handle->attrs.dst_ip, 0, 16);
					strncpy(handle->attrs.dst_ip, params->dst_ip, strlen(params->dst_ip));
					handle->attrs.dst_port = params->dst_port;
				}
			}
			break;
		}
		case VIS_MMNP_OPT_TYPE_GET_AUDIO_SAMPLERATE :
		{
			Vis_Mmnp_Handle handle = (Vis_Mmnp_Handle)visMmnpHandle;
			if (visMmnpHandle == NULL || buf == NULL) {
				Vmn_Log_Warning("Invalid params, handle=%p, buf=%p\n", visMmnpHandle, buf);
				return -2;
			}
			if (buflen == sizeof(int)) {
				int tmp = handle->attrs.audio_samplerate;
				memcpy(buf, &tmp, buflen);
			}
			break;
		}
		case VIS_MMNP_OPT_TYPE_SET_AUDIO_SAMPLERATE :
		{
			Vis_Mmnp_Handle handle = (Vis_Mmnp_Handle)visMmnpHandle;
			if (visMmnpHandle == NULL || buf == NULL) {
				Vmn_Log_Warning("Invalid params, handle=%p, buf=%p\n", visMmnpHandle, buf);
				return -2;
			}
			if (buflen == sizeof(unsigned int)) {
				memcpy(&handle->attrs.audio_samplerate, buf, buflen);
			}
			break;
		}
		case VIS_MMNP_OPT_TYPE_GET_CURR_CONN_NUM :
		{
			Vis_Mmnp_Handle handle = (Vis_Mmnp_Handle)visMmnpHandle;
			if (visMmnpHandle == NULL || buf == NULL) {
				Vmn_Log_Warning("Invalid params, handle=%p, buf=%p\n", visMmnpHandle, buf);
				return -2;
			}
			if (buflen ==sizeof(int)) {
				int tmp = handle->conn_num;
				memcpy(buf, &tmp, buflen);
			}
			break;
		}
		default :
			break;
	}
	return ret;
}

int vis_mmnp_putdata_tolib(void *visMmnpHandle, struct vis_mmnp_avdata *avdata) {
	Vis_Mmnp_Handle handle = visMmnpHandle;
	if (handle == NULL) return 0;
	/* For Rtsp, video into pbuffer, audio into pbuffer_rsz */
	return (handle->attrs.network_type!=NETWORK_SEND_RTSP || is_video(avdata->type)) ? vis_ring_buffer_putdata(handle->pbuffer,avdata): vis_ring_buffer_putdata(handle->pbuffer_rsz, avdata);
}
