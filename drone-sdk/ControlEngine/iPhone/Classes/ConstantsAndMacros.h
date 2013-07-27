//
// ConstantsAndMacros.h
//  Constants and macros for opengl view.
//
//  Created by Frédéric D'HAEYER on 09/10/30.
//  Copyright 2009 Parrot SA. All rights reserved.
//
// Macros
#ifndef _CONSTANTS_AND_MACROS_H_
#define _CONSTANTS_AND_MACROS_H_
#include <ardrone_api.h>
#include <control_states.h>
#include <ardrone_tool/ardrone_tool.h>
#include <ardrone_tool/ardrone_time.h>
#include <ardrone_tool/ardrone_tool_configuration.h>
#include <ardrone_tool/Control/ardrone_control.h>
#include <ardrone_tool/Control/ardrone_control_ack.h>
#include <ardrone_tool/Control/ardrone_control_configuration.h>
#include <ardrone_tool/Control/ardrone_control_soft_update.h>
#include <ardrone_tool/Navdata/ardrone_navdata_client.h>
#include <ardrone_tool/UI/ardrone_input.h>
#include <ardrone_tool/Com/config_com.h>
#include <ardrone_tool/Video/video_com_stage.h>
#include <ardrone_tool/Video/video_stage.h>

#include <Maths/time.h>

#include <VP_Os/vp_os.h>
#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_types.h>
#include <VP_Os/vp_os_signal.h>
#include <VP_Os/vp_os_malloc.h>
#include <VP_Os/vp_os_delay.h>

#include <VP_Api/vp_api.h>
#include <VP_Api/vp_api_error.h>
#include <VP_Api/vp_api_stage.h>
#include <VP_Api/vp_api_picture.h>
#include <VP_Api/vp_api_thread_helper.h>

#include <VLIB/Stages/vlib_stage_decode.h>

#include <iniparser3.0b/src/iniparser.h>

#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include <TargetConditionals.h>
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include "mobile_main.h"
#include "navdata.h"
#include "ControlData.h"

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <MediaPlayer/MediaPlayer.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <CoreMotion/CoreMotion.h>
#endif

#if TARGET_CPU_X86 == 1 // We are on iPhone simulator
#define WIFI_ITFNAME "en1"
#endif // TARGET_CPU_X86

#if TARGET_CPU_ARM == 1 // We are on real iPhone
#define WIFI_ITFNAME "en0"
#endif // TARGET_CPU_ARM

// How many times a second to refresh the screen
#define kFPS 20		// Frame per second
#define kAPS 40		// Number of accelerometer() function calls by second 

//#define CHECK_OPENGL_ERROR() ({ GLenum __error = glGetError(); if(__error) NSLog(@"OpenGLES error 0x%04X in %s\n", __error, __FUNCTION__); (__error ? NO : YES); })

#define ARDroneEngineLocalizeString(str) ([[NSBundle mainBundle] localizedStringForKey :str value:@"" table:@"languages"])

//#define WRITE_DEBUG_ACCELERO
//#define ENABLE_AUTO_TVOUT
//#define INTERFACE_WITH_DEBUG

typedef struct {
	float x;
	float y;
	float z;
} Vertex3D, Coord3D, Rotation3D, Scale3D;

typedef struct {
	float v1;
	float v2;
	float v3;
} Face3D;

typedef struct {
	float r;
	float g;
	float b;
	float a;	
} ColorRGBA;

#endif // _CONSTANTS_AND_MACROS_H_