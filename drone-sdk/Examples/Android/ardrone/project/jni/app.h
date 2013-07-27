/*
 * AR Drone demo
 *
 * code originally nased on:"San Angeles" Android demo app
 */

#ifndef APP_H_INCLUDED
#define APP_H_INCLUDED

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <android/log.h>
#include <sys/system_properties.h>

#include <ardrone_api.h>
#include <VP_Os/vp_os.h>

/* native video stream dimensions */
#define VIDEO_WIDTH   320
#define VIDEO_HEIGHT  240

extern uint16_t default_image[VIDEO_WIDTH*VIDEO_HEIGHT];
extern uint16_t picture_buf[];

extern void appInit();
extern void appDeinit();
extern void appRender(long tick, int width, int height);

extern void video_init(void);
extern void video_deinit(void);
extern void video_render(long tick, int width, int height);

void* navdata_loop(void *arg);
void* at_cmds_loop(void *arg);
void* stream_loop(void *arg);

void get_screen_dimensions(int *w, int *h);

struct event_info {
    unsigned state:1;
    unsigned x:12;
    unsigned y:12;
	unsigned unsused:7;
};

struct orientation_info {
	int values[3];
};

struct command_state {
  int takeOff_LandPressed;
  int emergencyStopPressed;
  int moveUpPressed;
  int moveDownPressed;
  int useAccSteering; // Set to TRUE or FALSE
  int usePcmd;
};

extern struct command_state commandState;

/*
 *  Clears the state of all command events
 */
extern void clearCommandState();

extern struct event_info motion_info;
extern struct event_info trackball_info;
extern struct orientation_info orientation;

/* Value is non-zero when application is alive, and 0 when it is closing.
 * Defined by the application framework.
 */
extern int gAppAlive;

#define INFO(_fmt_, args...)                                        \
    __android_log_print(ANDROID_LOG_INFO, "ARDrone", _fmt_, ##args)

#define GETPROP(_name_,_val_) __system_property_get((_name_),(_val_))

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define bool_t  int32_t

#define WIFI_ARDRONE_IP           "192.168.1.1"

extern char drone_ip[];
extern uint32_t mykonos_state;

void at_write (int8_t *buffer, int32_t len);

#endif // !APP_H_INCLUDED
