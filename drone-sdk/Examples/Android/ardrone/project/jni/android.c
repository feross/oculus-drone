/*
 * AR Drone demo
 *
 * code originally nased on:"San Angeles" Android demo app
 */

#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>

#include <ardrone_tool/UI/ardrone_input.h>

#include "navdata.h"
#include "app.h"

/*
*
*  Command passed down from Java UI layer
*
*/
#define  COMMAND_TAKEOFF_LAND   			1
#define  COMMAND_EMERGLAND 	2
#define  COMMAND_MOVEUP  			3
#define  COMMAND_MOVEDOWN 		4
#define  COMMAND_USE_ACCELEROMETER  		5

typedef struct _private_data
{
	float32_t pitch; 
   float32_t roll; 
   float32_t yaw; 
   float32_t gaz;
   bool_t isRefresh; 
} private_data;

static private_data data = { 0 };
static int  sWindowWidth  = 800; 
static int  sWindowHeight = 850;
static int  sDemoStopped  = 0;
static long sTimeOffset   = 0;
static int  sTimeOffsetInit = 0;
static long sTimeStopped  = 0;
static instance_navdata_t nav;

/**
 *  Returns time in milliseconds
 */
static long
_getTime(void)
{
    struct timeval  now;

    gettimeofday(&now, NULL);
    return (long)(now.tv_sec*1000 + now.tv_usec/1000);
}
   
/* Call to initialize the graphics state */
void
Java_com_parrot_ARDrone_DemoRenderer_nativeInit( JNIEnv*  env )
{
    //importGLInit();
    appInit();
    ardrone_navdata_reset_data( &nav );
    sDemoStopped = 0;
    sTimeOffsetInit = 0;
}

void
Java_com_parrot_ARDrone_DemoRenderer_nativeResize( JNIEnv*  env, jobject  thiz, jint w, jint h )
{
   // sWindowWidth  = w;
   // sWindowHeight = h;
    __android_log_print(ANDROID_LOG_INFO, "ARDrone", "resize w=%d h=%d", w, h);
}

/* Call to finalize the graphics state */
void
Java_com_parrot_ARDrone_DemoRenderer_nativeDone( JNIEnv*  env )
{
    //appDeinit();
    //importGLDeinit();
}

/* Call to render the next GL frame */
void
Java_com_parrot_ARDrone_DemoRenderer_nativeRender( JNIEnv*  env )
{
    long   curTime;

    /* NOTE: if sDemoStopped is TRUE, then we re-render the same frame
     *       on each iteration.
     */
    if (sDemoStopped) {
        curTime = sTimeStopped + sTimeOffset;
    } else {
        curTime = _getTime() + sTimeOffset;
        if (sTimeOffsetInit == 0) {
            sTimeOffsetInit = 1;
            sTimeOffset     = -curTime;
            curTime         = 0;
        }

        ardrone_navdata_get_data( &nav ); 

		  if( data.isRefresh )
		  {
			  ardrone_at_set_progress_cmd( 1,
					  data.roll/25000.0,
					  data.pitch/25000.0,
					  -data.gaz/25000.0,
					  data.yaw/25000.0);
		  }
		  else
		  {
           data.roll = 0;
           data.pitch = 0;
           data.gaz = 0;
           data.yaw = 0;

			  ardrone_at_set_progress_cmd( 1,
					  data.roll/25000.0,
					  data.pitch/25000.0,
					  -data.gaz/25000.0,
					  data.yaw/25000.0);
  
		  }
/*
		  INFO( "roll=%f, pitch=%f, gaz=%f, yaw=%f\n", 
				  data.roll/25000.0,
				  data.pitch/25000.0,
				  -data.gaz/25000.0,
				  data.yaw/25000.0);
*/
	 }

    appRender(curTime, sWindowWidth, sWindowHeight);
}

/* This is called to indicate to the render loop that it should
 * stop as soon as possible.
 */
void
Java_com_parrot_ARDrone_DemoGLSurfaceView_nativePause( JNIEnv*  env )
{
    sDemoStopped = !sDemoStopped;
    if (sDemoStopped) {
        /* we paused the animation, so store the current
         * time in sTimeStopped for future nativeRender calls */
        sTimeStopped = _getTime();
    } else {
        /* we resumed the animation, so adjust the time offset
         * to take care of the pause interval. */
        sTimeOffset -= _getTime() - sTimeStopped;
    }
}

#define TRACKBALL_THRESHOLD 32

void
Java_com_parrot_ARDrone_DemoGLSurfaceView_nativeTrackballEvent(JNIEnv *env,
                                                               jobject thiz,
                                                               jlong eventTime,
                                                               jint action,
                                                               jfloat x,
                                                               jfloat y) {
	static int nx = 0;

	if( action == 1 )
   {
		data.isRefresh = FALSE;
   }
   else
	{
	   /* horizontal gestures = YAW control
		 * x = 0: left
		 * x = 1: idle position
		 * x = 2: right
		 */
		nx += (int)(100.0*x);
      INFO( "nx=%d\n", nx );

		// FIXME: use trackball to control yaw in a better way
		if (nx <= -TRACKBALL_THRESHOLD) {
         data.yaw = 10000;
			nx = 0;
		}
		else if (nx >= TRACKBALL_THRESHOLD) {
         data.yaw = -10000;
			nx = 0;
		}
      data.isRefresh = TRUE;
	}

}

void
Java_com_parrot_ARDrone_DemoGLSurfaceView_nativeMotionEvent(JNIEnv *env,
                                                            jobject thiz,
                                                            jlong eventTime,
                                                            jint action,
                                                            jfloat x,
                                                            jfloat y) {
	/*
	if (motion_info.state == 1) {
		INFO("touch %ld @(%d,%d)\n", (long)eventTime,(int)x, (int)y);
	}
	*/
}
/*
void
Java_com_parrot_ARDrone_DemoGLSurfaceView_nativeKeyEvent(JNIEnv *env,
														 jobject thiz,
														 jint action) {
	//FIXME: thread synchronization required here !!
	trackball_info.state = (int)action;
	INFO("KEY %d\n", (int)action);
}
*/

void
Java_com_parrot_ARDrone_DemoActivity_nativeSensorEvent(JNIEnv *env,
													   jobject thiz,
													   jfloat x,
													   jfloat y,
													   jfloat z) 
{
	struct orientation_info orientation;

	if( data.isRefresh )
	{
		orientation.values[0] = (int)x;
		orientation.values[1] = (int)y;
		orientation.values[2] = (int)z;
		// orientation values in degrees

		data.pitch = -(orientation.values[2]+20)*600;
		if (data.pitch >= 25000) {
			data.pitch = 25000;
		}
		if (data.pitch <= -25000) {
			data.pitch = -25000;
		}
		data.roll = orientation.values[1]*600;
		if (data.roll >= 25000) {
			data.roll = 25000;
		}
		if (data.roll <= -25000) {
			data.roll = -25000;
		}
	}
}

void
Java_com_parrot_ARDrone_DemoActivity_nativeStop( JNIEnv*  env )
{
    appDeinit();
}
void Java_com_parrot_ARDrone_DemoActivity_nativeCommand( JNIEnv*  env,  jobject thiz, jint commandId,  jint iparam1, jfloat fparam1, jfloat fparam2, jfloat fparam3, jfloat fparam4 )
{
   static uint8_t select = 0;

	__android_log_print(ANDROID_LOG_INFO, "ARDrone", "command received %d with iparam1=%d\n", commandId, iparam1 );

	switch( (int)commandId )
	{
	case COMMAND_TAKEOFF_LAND:
      if( !nav.startPressed )
      {
         ardrone_tool_set_ui_pad_start( 1 );
      }
      else
		{
         ardrone_tool_set_ui_pad_start( 0 );
		}
		break;
	case COMMAND_EMERGLAND:
      select ^= 1;
      ardrone_tool_set_ui_pad_select( select );
		break;
	case  COMMAND_MOVEUP:
		if( iparam1 )
		{
         data.gaz = -10000;
    	   data.isRefresh = TRUE;
      }
	   else 
      {
    	   data.isRefresh = FALSE;
      }
		break;
	case COMMAND_MOVEDOWN:
		if( iparam1 )
		{
         data.gaz = 10000;
    	   data.isRefresh = TRUE;
      }
      else
      {
    	   data.isRefresh = FALSE;
      }
      break;
	case COMMAND_USE_ACCELEROMETER:
		if( iparam1 )
		{
    	   data.isRefresh = TRUE;
		}
		else
		{
    	   data.isRefresh = FALSE;
		}
		break;
	default:
		 __android_log_print(ANDROID_LOG_INFO, "ARDrone", "unrecognized command received %d", commandId );
      break;
	}
}


