/*
 * AR Drone demo
 *
 * code originally nased on:"San Angeles" Android demo app
 */

#ifndef APP_H_INCLUDED
#define APP_H_INCLUDED

#include <sys/types.h>
#include <unistd.h>

#include <VP_Os/vp_os_types.h>
#include <VP_Os/vp_os_thread.h>

/* native video stream dimensions */
#define VIDEO_WIDTH   176
#define VIDEO_HEIGHT  144

extern uint16_t default_image[VIDEO_WIDTH*VIDEO_HEIGHT];
extern int num_picture_decoded;
extern uint16_t picture_buf[];

extern void appInit();
extern void appDeinit();
extern void appRender(long tick, int width, int height);

extern void video_init(void);
extern void video_deinit(void);
extern void video_render(long tick, int width, int height);

DEFINE_THREAD_ROUTINE( navdata_loop, data);
DEFINE_THREAD_ROUTINE( at_cmds_loop, data);
DEFINE_THREAD_ROUTINE( stream_loop, data);

void get_screen_dimensions(int *w, int *h);
void set_screen_dimensions(int w, int h);

struct event_info {
    unsigned state:1;
    unsigned x:12;
    unsigned y:12;
	unsigned unsused:7;
};

struct orientation_info {
	int values[3];
};

extern struct event_info motion_info;
extern struct event_info trackball_info;
extern struct orientation_info orientation;

/* Value is non-zero when application is alive, and 0 when it is closing.
 * Defined by the application framework.
 */
extern int gAppAlive;

#define INFO(_fmt_, args...)   \
printf(_fmt_, ##args)                                        \

/*#define INFO(_fmt_, args...)                                        \
    __android_log_print(ANDROID_LOG_INFO, "ARDrone", _fmt_, ##args)
*/
#define GETPROP(_name_,_val_) __system_property_get((_name_),(_val_))

#define MYKONOS_REFRESH_MS        28
#define WIFI_MYKONOS_IP           "192.168.1.1"

extern uint32_t mykonos_state;

enum {
  NO_CONTROL_MODE = 0,          // Doing nothing
  MYKONOS_UPDATE_CONTROL_MODE,  // Mykonos software update reception (update is done next run)
                                // After event completion, card should power off
  PIC_UPDATE_CONTROL_MODE,      // Mykonos pic software update reception (update is done next run)
                                // After event completion, card should power off
  LOGS_GET_CONTROL_MODE,        // Send previous run's logs
  CFG_GET_CONTROL_MODE,         // Send activ configuration
  ACK_CONTROL_MODE              // Reset command mask in navdata
};

enum {
  MYKONOS_FLY_MASK            = 1 << 0, /*!< FLY MASK : (0) mykonos is landed, (1) mykonos is flying */
  MYKONOS_VIDEO_MASK          = 1 << 1, /*!< VIDEO MASK : (0) video disable, (1) video enable */
  MYKONOS_VISION_MASK         = 1 << 2, /*!< VISION MASK : (0) vision disable, (1) vision enable */
  MYKONOS_CONTROL_MASK        = 1 << 3, /*!< CONTROL ALGO : (0) euler angles control, (1) angular speed control */
  MYKONOS_ALTITUDE_MASK       = 1 << 4, /*!< ALTITUDE CONTROL ALGO : (0) altitude control inactive (1) altitude control active */
  MYKONOS_USER_FEEDBACK_START = 1 << 5, /*!< USER feedback : Start button state */
  MYKONOS_COMMAND_MASK        = 1 << 6, /*!< Control command ACK : (0) None, (1) one received */
  MYKONOS_TRIM_COMMAND_MASK   = 1 << 7, /*!< Trim command ACK : (0) None, (1) one received */
  MYKONOS_TRIM_RUNNING_MASK   = 1 << 8, /*!< Trim running : (0) none, (1) running */
  MYKONOS_TRIM_RESULT_MASK    = 1 << 9, /*!< Trim result : (0) failed, (1) succeeded */
  MYKONOS_NAVDATA_DEMO_MASK   = 1 << 10, /*!< Navdata demo : (0) All navdata, (1) only navdata demo */
  MYKONOS_NAVDATA_BOOTSTRAP   = 1 << 11, /*!< Navdata bootstrap : (0) options sent in all or demo mode, (1) no navdata options sent */
  MYKONOS_MOTORS_BRUSHED      = 1 << 12, /*!< Motors brushed : (0) brushless, (1) brushed */
  MYKONOS_COM_LOST_MASK   = 1 << 13, /*!< Communication Lost : (1) com problem, (0) Com is ok */
  MYKONOS_GYROS_ZERO          = 1 << 14, /*!< Bit means that there's an hardware problem with gyrometers */
  MYKONOS_VBAT_LOW            = 1 << 15, /*!< VBat low : (1) too low, (0) Ok */
  MYKONOS_VBAT_HIGH           = 1 << 16, /*!< VBat high (US mad) : (1) too high, (0) Ok */
  MYKONOS_TIMER_ELAPSED       = 1 << 17, /*!< Timer elapsed : (1) elapsed, (0) not elapsed */
  MYKONOS_NOT_ENOUGH_POWER    = 1 << 18, /*!< Power : (0) Ok, (1) not enough to fly */
  MYKONOS_ANGLES_OUT_OF_RANGE = 1 << 19, /*!< Angles : (0) Ok, (1) out of range */
  MYKONOS_WIND_MASK           = 1 << 20, /*!< Wind : (0) Ok, (1) too much to fly */
  MYKONOS_ULTRASOUND_MASK     = 1 << 21, /*!< Ultrasonic sensor : (0) Ok, (1) deaf */
  MYKONOS_CUTOUT_MASK         = 1 << 22, /*!< Cutout system detection : (0) Not detected, (1) detected */
  MYKONOS_PIC_VERSION_MASK    = 1 << 23, /*!< PIC Version number OK : (0) a bad version number, (1) version number is OK */
  MYKONOS_ATCODEC_THREAD_ON   = 1 << 24, /*!< ATCodec thread ON : (0) thread OFF (1) thread ON */
  MYKONOS_NAVDATA_THREAD_ON   = 1 << 25, /*!< Navdata thread ON : (0) thread OFF (1) thread ON */
  MYKONOS_VIDEO_THREAD_ON     = 1 << 26, /*!< Video thread ON : (0) thread OFF (1) thread ON */
  MYKONOS_ACQ_THREAD_ON       = 1 << 27, /*!< Acquisition thread ON : (0) thread OFF (1) thread ON */
  MYKONOS_CTRL_WATCHDOG_MASK  = 1 << 28, /*!< CTRL watchdog : (1) delay in control execution (> 5ms), (0) control is well scheduled */
  MYKONOS_ADC_WATCHDOG_MASK   = 1 << 29, /*!< ADC Watchdog : (1) delay in uart2 dsr (> 5ms), (0) uart2 is good */
  MYKONOS_COM_WATCHDOG_MASK   = 1 << 30, /*!< Communication Watchdog : (1) com problem, (0) Com is ok */
  MYKONOS_EMERGENCY_MASK      = 1 << 31  /*!< Emergency landing : (0) no emergency, (1) emergency */
};

void at_write (int8_t *buffer, int32_t len);

#endif // !APP_H_INCLUDED
