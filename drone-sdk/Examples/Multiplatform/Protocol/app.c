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
#include <sys/time.h>
#include <time.h>

#include <app.h>

THREAD_HANDLE nav_thread;
THREAD_HANDLE at_thread;
THREAD_HANDLE stream_thread;

int gAppAlive   = 0;

static int screen_width = 0;
static int screen_height = 0;

void appInit()
{
	int status;

	gAppAlive = 1;

	// navigation
	if (!nav_thread) {
      vp_os_thread_create( thread_navdata_loop, 0, &nav_thread);
	}

	// video stream
	if (!stream_thread) {
      vp_os_thread_create( thread_stream_loop, 0, &stream_thread);
	}

	// AT cmds loop
	if (!at_thread) {
      vp_os_thread_create( thread_at_cmds_loop, 0, &at_thread);
	}

#ifdef BUILD_OGLES
	// video rendering
	video_init();
#endif
}

void appDeinit()
{
	gAppAlive = 0;

	INFO("shutting down application...\n");

#ifdef BUILD_OGLES
	video_deinit();
#endif
	// all threads should implement a loop polling gAppAlive
	vp_os_thread_join( nav_thread );
	vp_os_thread_join( at_thread );
	vp_os_thread_join( stream_thread );

	nav_thread = 0;
	at_thread = 0;
	stream_thread = 0;

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

