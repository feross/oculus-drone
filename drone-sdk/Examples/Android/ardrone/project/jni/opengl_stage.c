/*
 *  opengl_stage.c
 *  Test
 *
 *  Created by Karl Leplat on 22/02/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#include "opengl_stage.h"
#include "app.h"
#include <android/log.h>

static opengl_video_stage_config_t opengl_video_config;

const vp_api_stage_funcs_t opengl_video_stage_funcs = {
	(vp_api_stage_handle_msg_t) NULL,
	(vp_api_stage_open_t) opengl_video_stage_open,
	(vp_api_stage_transform_t) opengl_video_stage_transform,
	(vp_api_stage_close_t) opengl_video_stage_close
};

C_RESULT opengl_video_stage_open(vlib_stage_decoding_config_t *cfg)
{
   __android_log_print( ANDROID_LOG_INFO, "ARDrone", "Enter in opengl_video_stage_open\n" );
	vp_os_mutex_init( &opengl_video_config.mutex );
	
	vp_os_mutex_lock( &opengl_video_config.mutex );
	opengl_video_config.data = vp_os_malloc(VIDEO_HEIGHT * VIDEO_WIDTH * 4);
	vp_os_memset(opengl_video_config.data, 0x0, VIDEO_HEIGHT * VIDEO_WIDTH * 4);
	vp_os_mutex_unlock( &opengl_video_config.mutex );
   
   __android_log_print( ANDROID_LOG_INFO, "ARDrone", "Exit opengl_video_stage_open\n" );
	
	return C_OK;
}

C_RESULT opengl_video_stage_transform(vlib_stage_decoding_config_t *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
	vp_os_mutex_lock( &out->lock );

	if(out->status == VP_API_STATUS_INIT)
	{
		out->status = VP_API_STATUS_PROCESSING;
	}
	
	if( in->status == VP_API_STATUS_ENDED ) 
	{
		out->status = in->status;
	}
	
	if(out->status == VP_API_STATUS_PROCESSING )
	{
		vp_os_mutex_lock( &opengl_video_config.mutex );
		
		if(cfg->num_picture_decoded > opengl_video_config.num_picture_decoded)
		{
			opengl_video_config.num_picture_decoded = cfg->num_picture_decoded;
			/*
         opengl_video_config.bytesPerPixel	= 2;
			opengl_video_config.widthImage		= cfg->controller.width;
			opengl_video_config.heightImage		= cfg->controller.height;		
			
			if(opengl_video_config.bytesPerPixel == 2)
			{
				opengl_video_config.format = GL_RGB;
				opengl_video_config.type = GL_UNSIGNED_SHORT_5_6_5;				
			}
		   */ 
			if (opengl_video_config.data != NULL)
			{   
				vp_os_memcpy(opengl_video_config.data, cfg->picture->y_buf, cfg->picture->width * cfg->picture->height );
			}
		
			out->numBuffers = in->numBuffers;
			out->indexBuffer = in->indexBuffer;
			out->buffers = in->buffers;
		}

		vp_os_mutex_unlock( &opengl_video_config.mutex );
	}
	
	vp_os_mutex_unlock( &out->lock );

	return C_OK;
}

C_RESULT opengl_video_stage_close(vlib_stage_decoding_config_t *cfg)
{
	vp_os_free(opengl_video_config.data);
	
	return C_OK;
}

opengl_video_stage_config_t* opengl_video_stage_get(void)
{
	return &opengl_video_config;
}
