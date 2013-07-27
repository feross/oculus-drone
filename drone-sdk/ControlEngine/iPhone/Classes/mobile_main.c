/*
 *  mobile_main.c
 *  Test
 *
 *  Created by Karl Leplat on 19/02/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#include "mobile_main.h"
#include "wifi.h"

#define MAXADDRS	16
//#define DEBUG_THREAD	1

static bool_t bContinue = TRUE;
char drone_address[MAXADDRS];

DEFINE_THREAD_ROUTINE(mobile_main, data)
{
	C_RESULT res = C_FAIL;
	vp_com_wifi_config_t *config = NULL;
	
	mobile_main_param_t *param = (mobile_main_param_t *)data;

	ardroneEngineCallback callback = param->callback;
	vp_os_memset(drone_address, 0x0, sizeof(drone_address));
	
	while(((config = (vp_com_wifi_config_t *)wifi_config()) != NULL) && (strcmp(config->itfName, WIFI_ITFNAME) != 0))
	{
		PRINT("Wait WIFI connection !\n");
		vp_os_delay(250);
	}
	
	// Get drone_address
	vp_os_memcpy(drone_address, config->server, strlen(config->server));
	PRINT("Drone address %s\n", drone_address);
	
	// Get iphone_mac_address
	get_iphone_mac_address(config->itfName);
	PRINT("Iphone MAC Address %s\n", iphone_mac_address);
	
	res = ardrone_tool_setup_com( NULL );
	
	if( FAILED(res) )
	{
		PRINT("Wifi initialization failed. It means either:\n");
		PRINT("\t* you're not root (it's mandatory because you can set up wifi connection only as root)\n");
		PRINT("\t* wifi device is not present (on your pc or on your card)\n");
		PRINT("\t* you set the wrong name for wifi interface (for example rausb0 instead of wlan0) \n");
		PRINT("\t* ap is not up (reboot card or remove wifi usb dongle)\n");
		PRINT("\t* wifi device has no antenna\n");
	}
	else
	{
		START_THREAD(video_stage, NULL);
		
		res = ardrone_tool_init(drone_address, strlen(drone_address), NULL, param->appName, param->usrName);
		
		callback(ARDRONE_ENGINE_INIT_OK);
		
		ardrone_tool_set_refresh_time(1000 / kAPS);

		while( SUCCEED(res) && bContinue == TRUE )
		{
			ardrone_tool_update();
		}
		
		JOIN_THREAD(video_stage);

		res = ardrone_tool_shutdown();
	}
	
	vp_os_free (data);
	
	return (THREAD_RET)res;
}

void ardroneEnginePause( void )
{
#ifdef DEBUG_THREAD
	PRINT( "%s\n", __FUNCTION__ );
#endif
	video_stage_suspend_thread();
	ardrone_tool_pause();
}

void ardroneEngineResume( void )
{
#ifdef DEBUG_THREAD
	PRINT( "%s\n", __FUNCTION__ );
#endif
	video_stage_resume_thread();
	ardrone_tool_resume();
}

void ardroneEngineStart ( ardroneEngineCallback callback, const char *appName, const char *usrName )
{
#ifdef DEBUG_THREAD
	PRINT( "%s\n", __FUNCTION__ );
#endif	
	video_stage_init();
	mobile_main_param_t *param = vp_os_malloc (sizeof (mobile_main_param_t));
	if (NULL != param)
	{
		param->callback = callback;
		strcpy(param->appName, appName);
		strcpy(param->usrName, usrName);
		START_THREAD( mobile_main, param);
	}
}

void ardroneEngineStop (void)
{
	PRINT( "%s\n", __FUNCTION__ );
	ardroneEnginePause();
	bContinue = FALSE;
}

C_RESULT custom_update_user_input(input_state_t* input_state, uint32_t user_input)
{
#ifdef DEBUG_THREAD
	printf("%s\n", __FUNCTION__);
#endif	
	return C_OK;	
	
}

C_RESULT custom_reset_user_input(input_state_t* input_state, uint32_t user_input)
{
#ifdef DEBUG_THREAD
	printf("%s\n", __FUNCTION__);
#endif	
	return C_OK;
}

C_RESULT ardrone_tool_display_custom() 
{
#ifdef DEBUG_THREAD
	printf("%s\n", __FUNCTION__);
#endif	
	return C_OK;
}
BEGIN_THREAD_TABLE
THREAD_TABLE_ENTRY(mobile_main, 20)
THREAD_TABLE_ENTRY(ardrone_control, 20)
THREAD_TABLE_ENTRY(navdata_update, 20)
THREAD_TABLE_ENTRY(video_stage, 20)
END_THREAD_TABLE
