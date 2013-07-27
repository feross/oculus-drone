/*
 *  video_stage.c
 *  Test
 *
 *  Created by Frédéric D'HAEYER on 22/02/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#include <VP_Api/vp_api.h>
#include <VP_Api/vp_api_error.h>
#include <VP_Api/vp_api_stage.h>
#include <VP_Api/vp_api_picture.h>

#include <VP_Api/vp_api_supervisor.h>
#include <VP_Com/vp_com_socket.h>
#include <VP_Os/vp_os_malloc.h>
#include <VP_Os/vp_os_assert.h>
//#include <VP_Os/vp_os_print.h>

#include <VLIB/Stages/vlib_stage_decode.h>
#include <VP_Stages/vp_stages_yuv2rgb.h>
#include <ardrone_tool/ardrone_tool.h>
#include <ardrone_tool/Com/config_com.h>
#include <ardrone_tool/Video/video_com_stage.h>
//#define RECORD_VIDEO
#ifdef RECORD_VIDEO
#    include <ardrone_tool/Video/video_stage_recorder.h>
#endif

#include "video_stage.h"
#include "app.h"
#include "opengl_stage.h"
#include <android/log.h>

#define NB_STAGES 4

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#define SSOPTCAST_RW(x) x
#define SSOPTCAST_RO(x) x

static bool_t video_stage_in_pause = FALSE;
static vp_os_cond_t video_stage_condition;
static vp_os_mutex_t video_stage_mutex;
static video_com_config_t my_icc;
static PIPELINE_HANDLE         my_pipeline_handle;
static vp_api_io_pipeline_t    my_pipeline;

const vp_api_stage_funcs_t my_video_com_funcs = {
  (vp_api_stage_handle_msg_t) NULL,
  (vp_api_stage_open_t) video_com_stage_open,
  (vp_api_stage_transform_t) video_com_stage_transform,
  (vp_api_stage_close_t) video_com_stage_close
};

void video_stage_init(void)
{
	vp_os_mutex_init(&video_stage_mutex);
	vp_os_cond_init(&video_stage_condition, &video_stage_mutex);
}

void video_stage_suspend_thread(void)
{
	vp_os_mutex_lock(&video_stage_mutex);
	video_stage_in_pause = TRUE;
	vp_os_mutex_unlock(&video_stage_mutex);	
}

void video_stage_resume_thread(void)
{
	vp_os_mutex_lock(&video_stage_mutex);
	vp_os_cond_signal(&video_stage_condition);
	video_stage_in_pause = FALSE;
	vp_os_mutex_unlock(&video_stage_mutex);	
}

DEFINE_THREAD_ROUTINE(video_stage, data)
{
   __android_log_print( ANDROID_LOG_INFO, "ARDrone", "Enter in video_stage\n" );
	C_RESULT res;
	
	vp_api_io_data_t        out;
	vp_api_io_stage_t       my_stages[NB_STAGES];
	
	vp_api_picture_t picture;
	
	vlib_stage_decoding_config_t vec;
   
   vp_os_memset(&my_icc,       0, sizeof( video_com_config_t ));
	vp_os_memset(&vec,          0, sizeof( vec ));
	vp_os_memset(&picture,      0, sizeof( picture ));

#ifdef RECORD_VIDEO
	video_stage_recorder_config_t vrc;
#endif
	
	/// Picture configuration
	picture.format        = PIX_FMT_RGB565;
	
	picture.width         = VIDEO_WIDTH;
	picture.height        = VIDEO_HEIGHT;
	picture.framerate     = 15;
	
	picture.y_buf   = vp_os_malloc( picture.width * picture.height * 2);
	picture.cr_buf  = NULL;
	picture.cb_buf  = NULL;
	
	picture.y_line_size   = picture.width * 2;
	picture.cb_line_size  = 0;
	picture.cr_line_size  = 0;
		
	
	vec.width               = VIDEO_WIDTH;
	vec.height              = VIDEO_HEIGHT;
	vec.picture             = &picture;
	vec.luma_only           = FALSE;
	vec.block_mode_enable   = TRUE;
	
	my_pipeline.nb_stages = 0;
   
	{
		
      __android_log_print( ANDROID_LOG_INFO, "ARDrone", "=>%d, %d\n", sizeof(my_icc), sizeof(video_com_config_t) );
		my_icc.com                 = COM_VIDEO();
		my_icc.buffer_size         = 100000;
		my_icc.protocol            = VP_COM_UDP;
		COM_CONFIG_SOCKET_VIDEO(&my_icc.socket, VP_COM_CLIENT, VIDEO_PORT, WIFI_ARDRONE_IP );

		my_stages[my_pipeline.nb_stages].type    = VP_API_INPUT_SOCKET;
		my_stages[my_pipeline.nb_stages].cfg     = (void *)&my_icc;
		my_stages[my_pipeline.nb_stages].funcs   = my_video_com_funcs;
		my_pipeline.nb_stages++;
	}

	my_stages[my_pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
	my_stages[my_pipeline.nb_stages].cfg     = (void*)&vec;
	my_stages[my_pipeline.nb_stages].funcs   = vlib_decoding_funcs;
	my_pipeline.nb_stages++;
	
#ifdef RECORD_VIDEO
	my_stages[my_pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
	my_stages[my_pipeline.nb_stages].cfg     = (void*)&vrc;
	my_stages[my_pipeline.nb_stages].funcs   = video_recorder_funcs;
	my_pipeline.nb_stages++;
#endif

	my_stages[my_pipeline.nb_stages].type    = VP_API_OUTPUT_LCD;
	my_stages[my_pipeline.nb_stages].cfg     = (void*)&vec;
	my_stages[my_pipeline.nb_stages].funcs   = opengl_video_stage_funcs;
	my_pipeline.nb_stages++;
		
	my_pipeline.stages = &my_stages[0];
   
	if( !ardrone_tool_exit() )
	{
		res = vp_api_open(&my_pipeline, &my_pipeline_handle);

		if( SUCCEED(res) )
		{
			int loop = SUCCESS;
			out.status = VP_API_STATUS_PROCESSING;
#ifdef RECORD_VIDEO		    
			{
				DEST_HANDLE dest;
				dest.stage = 2;
				dest.my_pipeline = my_pipeline_handle;
				vp_api_post_message( dest, PIPELINE_MSG_START, NULL, (void*)NULL);
			}
#endif			
			
			while( !ardrone_tool_exit() && (loop == SUCCESS) )
			{
				if( SUCCEED(vp_api_run(&my_pipeline, &out)) ) {
					if( (out.status == VP_API_STATUS_PROCESSING || out.status == VP_API_STATUS_STILL_RUNNING) ) {
						loop = SUCCESS;
					}
				}
				else loop = -1; // Finish this thread
			}
		   	
			vp_api_close(&my_pipeline, &my_pipeline_handle);
		}
	}
	
   __android_log_print( ANDROID_LOG_INFO, "ARDrone", "video stage ended\n" );
	
	return (THREAD_RET)0;
}

uint32_t video_stage_get_num_retries(void)
{
   return my_icc.num_retries;
}
