/*
 *  video_stage.h
 *  Test
 *
 *  Created by Frédéric D'HAEYER on 22/02/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#ifndef _VIDEO_STAGE_H_
#define _VIDEO_STAGE_H_

#include <ardrone_api.h>
#include <ardrone_tool/ardrone_tool.h>
#include <VP_Api/vp_api_thread_helper.h>

PROTO_THREAD_ROUTINE(video_stage, data);

void video_stage_init(void);
void video_stage_suspend_thread(void);
void video_stage_resume_thread(void);
uint32_t video_stage_get_num_retries(void);
#endif // _VIDEO_STAGE_H_
