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

#include "app.h"

static int screen_width = 0;
static int screen_height = 0;
static int gAppAlive = 0;

void appInit()
{
	int status;

   //Run AT command process
   at_run();
   
   //Run Navdata process
   navdata_run();

   //Run Stream process
   stream_run();

#ifdef BUILD_OGLES
	// video rendering
	video_init();
#endif
   gAppAlive = 1;
}

void appDeinit()
{
	INFO("shutting down application...\n");

   gAppAlive = 0;

#ifdef BUILD_OGLES
	video_deinit();
#endif
	// all threads should implement a loop polling gAppAlive
	navdata_stop();
	stream_stop();
   at_stop();

	INFO("application was cleanly exited\n");
}

#ifdef BUILD_OGLES
void appRender(long tick, int width, int height)
{
	if (!gAppAlive) {
		return;
	}
   screen_width = width;
   screen_height = height;
	video_render(tick, screen_width, screen_height);
}
#endif

void get_screen_dimensions(int *w, int *h)
{
	if (screen_width) {
		*w = screen_width;
		*h = screen_height;
	}
	else {
		// dummy dimensions
		*w = 480;
		*h = 320;
	}
}

