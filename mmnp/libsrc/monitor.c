//#include <pthread.h>

#include "vis_mmnp.h"
#include "ringbuffer.h"
#include "tcp.h"
#include "udp.h"
#include "c1.h"
#include "rtsp.h"
#include "rtmp.h"

void *vmn_monitor(void *args) {
	void *status = THREAD_SUCCESS;
	int ret = 0, cleanup_flag = 0, restart_thread_flag = 1;
//	Vis_Mmnp_Handle *handle_ptr = NULL;
	Vis_Mmnp_Handle hVmn = NULL;
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	if (args == NULL) {
		return THREAD_FAILURE;
	}
	/* if I want use handle_ptr to get handle, I must keep the handle value valid
	 * but void *visMmnpHandle is a param of vis_mmnp_start() which copied from who call it in main.c
	 * void *visMmnpHandle will be release when vis_mmnp_start() return */
//	handle_ptr = (Vis_Mmnp_Handle *)args;
//	hVmn = *handle_ptr;		//WHY?????????????????????????
	hVmn = (Vis_Mmnp_Handle)args;
	hVmn->quit_flag = 0;	//checkout

	/* Start Writer Thread */
	if (hVmn->attrs.data_transmit_mode) {
		//TODO, create writer thread
	}

	if (pthread_mutex_init(&mutex, NULL) != 0) {
		Vmn_Log_Error("init mutex for monitor");
		status = THREAD_FAILURE;
		goto MONITOR_CLEANUP;
	}
	cleanup_flag = 1;
	if (pthread_cond_init(&cond, NULL) != 0) {
		Vmn_Log_Error("init cond for monitor");
		status = THREAD_FAILURE;
		goto MONITOR_CLEANUP;
	}
	cleanup_flag = 2;

	Vmn_Log_Debug("monitor.c:before main loop, loop_flag=%d\n", !hVmn->quit_flag && restart_thread_flag && status!=THREAD_FAILURE);
	/* Call Network Fuction in different protocol */
    while(restart_thread_flag && status!=THREAD_FAILURE) {
		Vmn_Log_Debug("in monitor network_type = %d\n", hVmn->attrs.network_type);
		switch (hVmn->attrs.network_type) {
			case NETWORK_SEND_BROADCAST :
			case NETWORK_SEND_MULTICAST :
			case NETWORK_SEND_UNICAST :
			case NETWORK_SEND_VOD :
			case NETWORK_SEND_UDP :
			{
				pthread_t demand_tid, udp_tid;
				void *ret_demand=THREAD_FAILURE, *ret_udp=THREAD_FAILURE;
				DemandEnv demandenv;
				UdpEnv udpenv;
				conn_client_t *conn_client = NULL;
				int udp_cleanup_flag = 0;
				
				memset(&demandenv, 0, sizeof(demandenv));
				memset(&udpenv, 0, sizeof(udpenv));

				conn_client = (conn_client_t *)calloc(1, sizeof(conn_client_t));
				if (conn_client == NULL) {
					Vmn_Log_Error("alloc memery for conn_client failed");
					status = THREAD_FAILURE;
					goto UDP_CLEANUP;
				}
				++udp_cleanup_flag;	//1
				conn_client->type_ptr = &hVmn->attrs.network_type;
				conn_client->maxConnNum = (hVmn->attrs.maxConnNum<=8 && hVmn->attrs.maxConnNum>0)?hVmn->attrs.maxConnNum:8;
				conn_client->conn_num_ptr = &hVmn->conn_num;
				conn_client->url = hVmn->attrs.url;
				conn_client->src_ipAddr = hVmn->attrs.src_ip;
				conn_client->src_port_ptr = &hVmn->attrs.src_port;
				conn_client->dst_ipAddr = hVmn->attrs.dst_ip;
				conn_client->dst_port_ptr = &hVmn->attrs.dst_port;
				Vmn_Log_Debug("maxConnNum = %hu\n", conn_client->maxConnNum);
				/* The aguments below will be release/free in udp thread clean up, when udp moduel init success */
				if (pthread_mutex_init(&conn_client->mutex, NULL)) {
					Vmn_Log_Error("init conn_client.mutex failed");
					status = THREAD_FAILURE;
					goto UDP_CLEANUP;
				}
				++udp_cleanup_flag;	//2
				if ((conn_client->port_table=(unsigned short *)calloc(conn_client->maxConnNum, sizeof(unsigned short))) == NULL) {
					Vmn_Log_Error("alloc memery for conn_client->port_table failed");
					status = THREAD_FAILURE;
					goto UDP_CLEANUP;
				}
				++udp_cleanup_flag;	//3
				if ((conn_client->dstaddr_table=(struct sockaddr_in *)calloc(conn_client->maxConnNum, sizeof(struct sockaddr_in))) == NULL) {
					Vmn_Log_Error("alloc memery for conn_client->dstaddr_table failed");
					status = THREAD_FAILURE;
					goto UDP_CLEANUP;
				}
				++udp_cleanup_flag;	//4
				if ((conn_client->flag_table=(char *)calloc(conn_client->maxConnNum, sizeof(char))) == NULL) {
					Vmn_Log_Error("alloc memery for conn_client->flag_table failed");
					status = THREAD_FAILURE;
					goto UDP_CLEANUP;
				}
				++udp_cleanup_flag;	//5
				if ((conn_client->time_table=(unsigned int *)calloc(conn_client->maxConnNum, sizeof(unsigned int))) == NULL) {
					Vmn_Log_Error("alloc memery for conn_client->time_table failed");
					status = THREAD_FAILURE;
					goto UDP_CLEANUP;
				}
				++udp_cleanup_flag;	//6
	//			status = THREAD_FAILURE;goto UDP_CLEANUP;	//test

				/* Create Demand Thread */
				demandenv.mutex_initsync = &mutex;
				demandenv.cond_initsync = &cond;
//				demandenv.handle_ptr = handle_ptr;
				demandenv.quit_flag_ptr = &hVmn->quit_flag;
				demandenv.conn_client = conn_client;
				Vmn_Log_Debug("Before create Demand Thread, quit_flag[@%p]=%d.\n", demandenv.quit_flag_ptr, *demandenv.quit_flag_ptr);
				ret = pthread_create(&demand_tid, NULL, vmn_demandThrFxn, (void *)&demandenv);
				if (ret == 0) {
					Vmn_Log_Debug("1.monitor before cond_wait\n");
					pthread_mutex_lock(&mutex);		//not very nessery, because monitor will wait for thread at pthread, env will NOT be destroy any more
					pthread_cond_wait(&cond, &mutex);
					pthread_mutex_unlock(&mutex);
					Vmn_Log_Debug("4.monitor after cond_wait\n");
				} else {
					Vmn_Log_Error("Create Demand Thread Failed");
					status = THREAD_FAILURE;
					goto UDP_CLEANUP;
				}
				++udp_cleanup_flag;	//7
				
				/* Create UDP Thread */
				udpenv.mutex_initsync = &mutex;
				udpenv.cond_initsync = &cond;
//				udpenv.handle_ptr = handle_ptr;
				udpenv.quit_flag_ptr = &hVmn->quit_flag;
				udpenv.pbuffer = hVmn->pbuffer;
				udpenv.conn_client = conn_client;
				Vmn_Log_Debug("Before create UDP Thread, quit_flag[@%p]=%d.\n", udpenv.quit_flag_ptr, *udpenv.quit_flag_ptr);
				ret = pthread_create(&udp_tid, NULL, vmn_udpThrFxn, (void *)&udpenv);
				if (ret == 0) {
					Vmn_Log_Debug("1.monitor before cond_wait\n");
					pthread_mutex_lock(&mutex);
					pthread_cond_wait(&cond, &mutex);
					pthread_mutex_unlock(&mutex);
					Vmn_Log_Debug("4.monitor after cond_wait\n");
//					udp_cleanup_flag = 0;
				} else {
					Vmn_Log_Error("Create UDP Thread Failed");
					status = THREAD_FAILURE;
					goto UDP_CLEANUP;
				}

//#ifdef WIN32
#if 1
				pthread_join(demand_tid, &ret_demand);
				pthread_join(udp_tid, &ret_udp);
				/* if any thread return THREAD_FAILURE indicate that thread failed itself
				 * else the THREAD_SUCCESS indicate that user set quit extern */
				restart_thread_flag = (ret_demand==THREAD_FAILURE || ret_udp==THREAD_FAILURE);
#else
				/* Not very exactly, when quit_flag is set to 1, udp_thread will return immediantlly, 
				 * but demand_thread will return in a will(about 1 sec), because accept() will be timeout
				 * in 1 sec every loop in demand_thread*/
				do {
					int demand_is_running = pthread_tryjoin_np(demand_tid, &ret_demand);
					int udp_is_running = pthread_tryjoin_np(udp_tid, &ret_udp);
					Vmn_Log_Debug("demand_is_running=%d, udp_is_running=%d, ret_demand=%d, ret_udp=%d\n", demand_is_running, udp_is_running, (int)ret_demand, (int)ret_udp);
					if (!demand_is_running || !udp_is_running) break;
					Sleep(1000);
				} while(1);
				/* if any thread return THREAD_FAILURE indicate that thread failed itself
				 * else the THREAD_SUCCESS indicate that user set quit extern */
				restart_thread_flag = (ret_demand==THREAD_FAILURE || ret_udp==THREAD_FAILURE);
#endif

UDP_CLEANUP:
				Vmn_Log_Debug("monitor.c: udp_cleanup_flag = %d\n", udp_cleanup_flag);
				Vmn_Log_Debug("time_table[@%p]\n", conn_client->time_table);
				Vmn_Log_Debug("flag_table[@%p]\n", conn_client->flag_table);
				Vmn_Log_Debug("addr_table[@%p]\n", conn_client->dstaddr_table);
				Vmn_Log_Debug("port_table[@%p]\n", conn_client->port_table);
				switch (udp_cleanup_flag) {
					case 7:		//Cancel Demand Thread
					{
						hVmn->quit_flag = 1;
						if (pthread_join(demand_tid, &ret_demand) == 0) {
							Vmn_Log_Debug("Join Demand Thread when monitor init udp moduel failed - Success, ret=%d\n", (int)ret_demand);
						} else {
							Vmn_Log_Error("Join Demand Thread when monitor init udp moduel failed - Error");
						}
						hVmn->quit_flag = 0;
					}
					case 6:
						Vmn_Log_Debug("monitor.c:free 1\n");
						free(conn_client->time_table); conn_client->time_table = NULL;
					case 5:
						Vmn_Log_Debug("monitor.c:free 2\n");
						free(conn_client->flag_table); conn_client->flag_table = NULL;
					case 4:
						Vmn_Log_Debug("monitor.c:free 3\n");
						free(conn_client->dstaddr_table); conn_client->dstaddr_table = NULL;
					case 3:
						Vmn_Log_Debug("monitor.c:free 4\n");
						free(conn_client->port_table); conn_client->port_table = NULL;
					case 2:
						Vmn_Log_Debug("monitor.c:free 5\n");
						pthread_mutex_destroy(&conn_client->mutex);
					case 1:
						Vmn_Log_Debug("monitor.c:free 6\n");
						free(conn_client); conn_client = NULL;
					default:
						break;
				}
				
				break;
			}

			case NETWORK_SEND_TCP :
			{
				pthread_t tcp_tid;
				void *ret_tcp = THREAD_SUCCESS;
				TcpEnv env;

				env.url = hVmn->attrs.url;
				env.ip = hVmn->attrs.src_ip;
				env.port = hVmn->attrs.src_port;
	//			env.maxConnNum = hVmn->attrs.maxConnNum;
				env.mutex_initsync = &mutex;
				env.cond_initsync = &cond;
//				env.handle_ptr = handle_ptr;
				env.quit_flag_ptr = &hVmn->quit_flag;
				Vmn_Log_Debug("quit_flag_ptr=%p\n", env.quit_flag_ptr);
				env.conn_num_ptr = &hVmn->conn_num;
				env.conn_num_mutex = &hVmn->mutex;
				env.pbuffer = hVmn->pbuffer;
				Vmn_Log_Debug("Before create TCP Thread.\n");
				ret = pthread_create(&tcp_tid, NULL, vmn_tcpThrFxn, (void *)&env);
				if (ret == 0) {
	//				sem_wait(&sem_initsync);	//wait for sender thread init complete
					Vmn_Log_Debug("1.monitor before cond_wait\n");
					pthread_mutex_lock(&mutex);
					pthread_cond_wait(&cond, &mutex);
					pthread_mutex_unlock(&mutex);
					Vmn_Log_Debug("4.monitor after cond_wait\n");
				} else {
					Vmn_Log_Error("Create TCP Thread Failed");
					status = THREAD_FAILURE;
					goto TCP_CLEANUP;
				}
				Vmn_Log_Debug("Before join tcp thread\n");
				pthread_join(tcp_tid, &ret_tcp);
				/* if any thread return THREAD_FAILURE indicate that thread failed itself
				 * else the THREAD_SUCCESS indicate that user set quit extern */
				restart_thread_flag = (ret_tcp==THREAD_FAILURE);
				Vmn_Log_Debug("After join tcp thread, restart_thread_flag=%d\n", restart_thread_flag);
TCP_CLEANUP:
				break;
			}

			case NETWORK_SEND_C1:
			{
				pthread_t c1_tid;
				void *ret_c1 = THREAD_SUCCESS;
				C1Env env;

				env.url = hVmn->attrs.url;
				env.ip = hVmn->attrs.src_ip;
				env.port = hVmn->attrs.src_port;
	//			env.maxConnNum = hVmn->attrs.maxConnNum;
				env.mutex_initsync = &mutex;
				env.cond_initsync = &cond;
//				env.handle_ptr = handle_ptr;
				env.quit_flag_ptr = &hVmn->quit_flag;
				Vmn_Log_Debug("quit_flag_ptr=%p\n", env.quit_flag_ptr);
				env.conn_num_ptr = &hVmn->conn_num;
				env.conn_num_mutex = &hVmn->mutex;
				env.pbuffer = hVmn->pbuffer;
				env.pbuffer_rsz = hVmn->pbuffer_rsz;
				Vmn_Log_Debug("Before create C1 Thread.\n");
				ret = pthread_create(&c1_tid, NULL, vmn_c1ThrFxn, (void *)&env);
				if (ret == 0) {
					Vmn_Log_Debug("1.monitor before cond_wait\n");
					pthread_mutex_lock(&mutex);
					pthread_cond_wait(&cond, &mutex);
					pthread_mutex_unlock(&mutex);
					Vmn_Log_Debug("4.monitor after cond_wait\n");
				} else {
					Vmn_Log_Error("Create C1 Thread Failed");
					status = THREAD_FAILURE;
					goto C1_CLEANUP;
				}
				pthread_join(c1_tid, &ret_c1);
				/* if any thread return THREAD_FAILURE indicate that thread failed itself
				 * else the THREAD_SUCCESS indicate that user set quit extern */
				restart_thread_flag = (ret_c1==THREAD_FAILURE);
C1_CLEANUP:
				break;
			}

			case NETWORK_SEND_RTSP :
			{
				pthread_t rtsp_tid;
				void *ret_rtsp = THREAD_SUCCESS;
				RtspEnv env;

				memset(&env, 0, sizeof(env));
				env.videoEnable = hVmn->attrs.video_enable;
				env.audioEnable = hVmn->attrs.audio_enable;
				env.videofps = hVmn->attrs.video_fps;
				env.audioChannel = hVmn->attrs.audio_channel;
				env.audio_samplerate_ptr = &hVmn->attrs.audio_samplerate;
				env.url = hVmn->attrs.url;
				env.ip = hVmn->attrs.src_ip;
				env.port = hVmn->attrs.src_port;
				env.maxConnNum = hVmn->attrs.maxConnNum;
				env.quit_flag_ptr = &hVmn->quit_flag;
				Vmn_Log_Debug("quit_flag_ptr=%p\n", env.quit_flag_ptr);
				env.conn_num_ptr = &hVmn->conn_num;
				env.conn_num_mutex = &hVmn->mutex;
				env.pbuffer_video = hVmn->pbuffer;
				env.pbuffer_audio = hVmn->pbuffer_rsz;
				Vmn_Log_Debug("Before create Rtsp Thread.\n");
				ret = pthread_create(&rtsp_tid, NULL, vmn_rtspThrFxn, (void *)&env);
				if (ret) {
					Vmn_Log_Error("Create RTSP Thread Failed");
					status = THREAD_FAILURE;
					goto RTSP_CLEANUP;
				}
				pthread_join(rtsp_tid, &ret_rtsp);
				/* if any thread return THREAD_FAILURE indicate that thread failed itself
				 * else the THREAD_SUCCESS indicate that user set quit extern */
				restart_thread_flag = (ret_rtsp==THREAD_FAILURE);

RTSP_CLEANUP:
				break;
			}

			case NETWORK_SEND_RTMP :
			{
				pthread_t rtmp_tid;
				void *ret_rtmp = THREAD_SUCCESS;
				RtmpEnv env;

				memset(&env, 0, sizeof(env));
				env.url					= hVmn->attrs.url;
				env.video_width			= hVmn->attrs.video_width;
				env.video_height		= hVmn->attrs.video_width;
				env.video_compress		= hVmn->attrs.video_compress;
				//env.video_frameType		= ;
				env.audio_bits			= hVmn->attrs.audio_bits;
				env.audio_samplerate_ptr= (int *)&hVmn->attrs.audio_samplerate;
				env.audio_channels		= hVmn->attrs.audio_channel;
				env.audio_compress		= hVmn->attrs.audio_compress;
				env.quit_flag_ptr		= &hVmn->quit_flag;
				env.pbuffer 			= hVmn->pbuffer;
				Vmn_Log_Debug("quit_flag_ptr=%p\n", env.quit_flag_ptr);
				Vmn_Log_Debug("Before create Rtmp Thread.\n");
				ret = pthread_create(&rtmp_tid, NULL, vmn_rtmpThrFxn, (void *)&env);
				if (ret) {
					Vmn_Log_Error("Create RTMP Thread Failed");
					status = THREAD_FAILURE;
					goto RTMP_CLEANUP;
				}
				pthread_join(rtmp_tid, &ret_rtmp);
				/* if any thread return THREAD_FAILURE indicate that thread failed itself
				 * else the THREAD_SUCCESS indicate that user set quit extern */
				restart_thread_flag = (ret_rtmp==THREAD_FAILURE);

RTMP_CLEANUP:
				break;
			}

			default :
				status = THREAD_FAILURE;
				break;
		}
		Sleep(1000);	//why?make sure processor is not so busy when failed
	}

MONITOR_CLEANUP:
	/* Cleanup */
	switch (cleanup_flag) {
		case 2 :
			pthread_cond_destroy(&cond);
		case 1 :
			pthread_mutex_destroy(&mutex);
		default :
			break;
	}
	Vmn_Log_Debug("Monitor Thread Return\n");
	return status;
}
