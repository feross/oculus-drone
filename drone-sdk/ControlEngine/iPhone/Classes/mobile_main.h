/*
 *  mobile_main.h
 *  Test
 *
 *  Created by Karl Leplat on 19/02/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#ifndef _MOBILE_MAIN_H_
#define _MOBILE_MAIN_H_

#include "ConstantsAndMacros.h"

typedef enum
{
	ARDRONE_ENGINE_INIT_OK,
	ARDRONE_ENGINE_MAX
} ARDRONE_ENGINE_MESSAGE;

typedef void (*ardroneEngineCallback)(ARDRONE_ENGINE_MESSAGE msg);

typedef struct {
	ardroneEngineCallback callback;
	char appName[APPLI_NAME_SIZE];
	char usrName[USER_NAME_SIZE];
 } mobile_main_param_t;

void ardroneEnginePause( void );
void ardroneEngineResume( void );
void ardroneEngineStart( ardroneEngineCallback callback, const char *appName, const char *usrName );
void ardroneEngineStop( void );
PROTO_THREAD_ROUTINE(mobile_main, data);

#endif