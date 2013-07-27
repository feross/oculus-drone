/*
 * AR Drone demo
 *
 * originally based on Android NDK "San Angeles" demo app
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>

#include "app.h"
#include "control_ack.h"
#include "video_stage.h"

#include <ardrone_api.h>
#include <ardrone_tool/ardrone_tool.h>
#include <ardrone_tool/Control/ardrone_control.h>
#include <ardrone_tool/Navdata/ardrone_navdata_client.h>
#include <ardrone_tool/ardrone_tool.>
#include <VP_Api/vp_api_thread_helper.h>

char drone_ip[24];
int gAppAlive   = 0;

// assume drone has address x.x.x.1
#define WIFI_ARDRONE_ADDR_LAST_BYTE (1)

#define DEBUG_THREAD	1
static bool_t bContinue = TRUE;

void get_drone_ip(void)
{
    int ret;
    char prop_value[PROP_VALUE_MAX];
    char prop_name[PROP_NAME_MAX];
    int inet_part[4];

    // quick and dirty way to guess Drone IP address
    ret = GETPROP("wifi.interface", prop_value);
    if (ret > 0) {
        INFO("wifi interface = %s\n", prop_value);
        snprintf(prop_name, PROP_NAME_MAX, "dhcp.%s.ipaddress", prop_value);
        ret = GETPROP(prop_name, prop_value);
    }
    if (ret > 0) {
        INFO("IP address = %s\n", prop_value);
        ret = 0;
        if (sscanf(prop_value, "%d.%d.%d.%d",
                   &inet_part[0],
                   &inet_part[1],
                   &inet_part[2],
                   &inet_part[3]) == 4) {
            ret = 1;
            sprintf(drone_ip, "%u.%u.%u.%u",
                    inet_part[0]& 0xff, inet_part[1]& 0xff, inet_part[2]&0xff,
                    WIFI_ARDRONE_ADDR_LAST_BYTE);
        }
    }
    if (ret == 0) {
        // fallback to default address
        sprintf(drone_ip, WIFI_ARDRONE_IP);
    }

    INFO("assuming drone IP address is %s\n", drone_ip);
}

PROTO_THREAD_ROUTINE(mobile_main, data);
DEFINE_THREAD_ROUTINE(mobile_main, data)
{
	__android_log_print( ANDROID_LOG_INFO, "ARDrone", "Enter in mobile_main thread\n" );
	C_RESULT res = C_OK;
	char drone_address[24];

	vp_os_memset(drone_address, 0x0, sizeof(drone_address));
	sprintf(drone_address, WIFI_ARDRONE_IP);

	res = ardrone_tool_setup_com( NULL );

	if( FAILED(res) )
	{
		__android_log_print( ANDROID_LOG_INFO, "ARDrone", "Setup com failed\n" );
		INFO("Wifi initialization failed. It means either:\n");
		INFO("\t* you're not root (it's mandatory because you can set up wifi connection only as root)\n");
		INFO("\t* wifi device is not present (on your pc or on your card)\n");
		INFO("\t* you set the wrong name for wifi interface (for example rausb0 instead of wlan0) \n");
		INFO("\t* ap is not up (reboot card or remove wifi usb dongle)\n");
		INFO("\t* wifi device has no antenna\n");
	}
	else
	{
		START_THREAD(video_stage, NULL);

		__android_log_print( ANDROID_LOG_INFO, "ARDrone", "Processing ardrone_tool_init\n" );
		res = ardrone_tool_init(drone_address, strlen(drone_address), NULL);

      control_ack_init();
      control_ack_configure_navdata_demo( FALSE );

		if(SUCCEED(res))
		{   
			res = ardrone_tool_set_refresh_time(25);

			while( SUCCEED(res) && bContinue == TRUE )
			{
				res = ardrone_tool_update();
			}
		}

		JOIN_THREAD(video_stage);

		res = ardrone_tool_shutdown();
	}

	__android_log_print( ANDROID_LOG_INFO, "ARDrone", "Exit mobile_main thread\n" );
	return (THREAD_RET)0;
}

void appInit(void)
{
   __android_log_print( ANDROID_LOG_INFO, "ARDrone", "Enter in appInit\n" );
	
   // video rendering
	video_init();
//    int status;

  //  get_drone_ip();

	gAppAlive = 1;
	START_THREAD( mobile_main, NULL);
}

void appDeinit()
{
    gAppAlive = 0;

    INFO("shutting down application...\n");
    
    video_deinit();
    JOIN_THREAD( mobile_main );
}

void appRender(long tick, int width, int height)
{
    if (!gAppAlive) {
        return;
    }
    video_render(tick, width, height);
}

BEGIN_THREAD_TABLE
THREAD_TABLE_ENTRY(mobile_main, 20)
THREAD_TABLE_ENTRY(ardrone_control, 20)
THREAD_TABLE_ENTRY(navdata_update, 20)
THREAD_TABLE_ENTRY(video_stage, 20)
END_THREAD_TABLE

