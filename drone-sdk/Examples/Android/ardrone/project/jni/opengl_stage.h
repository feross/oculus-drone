/*
 *  opengl_stage.h
 *  Test
 *
 *  Created by Karl Leplat on 22/02/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#ifndef _OPENGL_STAGE_H_
#define _OPENGL_STAGE_H_

#include <ardrone_api.h>
#include <ardrone_tool/ardrone_tool.h>
#include <VP_Api/vp_api_thread_helper.h>
#include <VP_Os/vp_os_signal.h>
#include <VLIB/Stages/vlib_stage_decode.h>

typedef struct _opengl_video_config_t
{
	vp_os_mutex_t mutex;
	void* data;
	uint32_t num_picture_decoded;
	
} opengl_video_stage_config_t;

C_RESULT opengl_video_stage_open(vlib_stage_decoding_config_t *cfg);
C_RESULT opengl_video_stage_transform(vlib_stage_decoding_config_t *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out);
C_RESULT opengl_video_stage_close(vlib_stage_decoding_config_t *cfg);

opengl_video_stage_config_t* opengl_video_stage_get(void);

extern const vp_api_stage_funcs_t opengl_video_stage_funcs;
extern const opengl_video_stage_config_t* opengl_video_stage_config;

#endif // _OPENGL_STAGE_H_
