#ifndef _COMMON_INCLUDE_
#define _COMMON_INCLUDE_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef _MOBILE
#include <parrotos/parrotos.h>
#endif


//#define USE_AT_RESPONSES

// AT commands and navigation data link definition
// comment out definition of DRONE_UART_LINK below to enable Wi-Fi communication
#define DRONE_UART_LINK
#ifdef DRONE_UART_LINK
// comment out definition of DRONE_XBEE_LINK below to enable wired UART communication
//#define DRONE_XBEE_LINK
#ifdef DRONE_XBEE_LINK
#define EMB_UART_LINK_SPEED CYGNUM_SERIAL_BAUD_9600
#define MOB_UART_LINK_SPEED B9600
#else // !DRONE_XBEE_LINK = wired UART link
#define EMB_UART_LINK_SPEED CYGNUM_SERIAL_BAUD_460800
#define MOB_UART_LINK_SPEED B460800
#endif // <- DRONE_XBEE_LINK
#endif // <- DRONE_UART_LINK



// Inertial Measures sampling frequency
/**
 *  \var     IMU_SAMPLING_FREQ_HZ
 *  \brief   Inertial Measurement Unit sampling frequency in Hz
 */
#define IMU_SAMPLING_FREQ_HZ 200

/**
 *  \var     IMU_SAMPLING_PERIOD_S
 *  \brief   Inertial Measurement Unit sampling period in seconds (float32_t value)
 */
#define IMU_SAMPLING_PERIOD_S (1.0/(float32_t)IMU_SAMPLING_FREQ_HZ)

/**
 *  \var     IMU_SAMPLING_PERIOD_US
 *  \brief   Inertial Measurement Unit sampling period in microseconds (integer value)
 */
#define IMU_SAMPLING_PERIOD_US (1000000/IMU_SAMPLING_FREQ_HZ)



// Default control loops gains
// To avoid divisions in embedded software, gains are defined as ratios where
//  - the numerator is an integer
//  - the denominator is a power of 2 (division is replaced by right shift)
/**
 *  \var     CTRL_H_SHIFT
 *  \brief   Shift for denominator of default proportionnal gain of pitch (p) and roll (q) angular rate control loops
 */
#define CTRL_W_SHIFT 7

/**
 *  \var     CTRL_DEFAULT_NUM_H_PQ
 *  \brief   Numerator of default proportionnal gain for pitch (p) and roll (q) angular rate control loops
 */
#define CTRL_DEFAULT_NUM_H_PQ 80

/**
 *  \var     CTRL_DEFAULT_NUM_KI_PQ
 *  \brief   Numerator of default integral gain for Euler Angle control loops
 */
#define CTRL_DEFAULT_NUM_KI_PQ 50

/**
 *  \var     CTRL_DEFAULT_NUM_H_R
 *  \brief   Numerator of default proportionnal gain for yaw (r) angular rate control loop
 */
#define CTRL_DEFAULT_NUM_H_R 200

/**
 *  \var     CTRL_EA_SHIFT
 *  \brief   Shift for denominator of default gains for Euler Angle control loops
 */
#define CTRL_EA_SHIFT 10

/**
 *  \var     CTRL_DEFAULT_NUM_KP_EA
 *  \brief   Numerator of default proportionnal gain for Euler Angle control loops
 */
#define CTRL_DEFAULT_NUM_KP_EA 6000

/**
 *  \var     CTRL_DEFAULT_NUM_KI_EA
 *  \brief   Numerator of default integral gain for Euler Angle control loops
 */
#define CTRL_DEFAULT_NUM_KI_EA 0

/**
 *  \var     CTRL_DEFAULT_NUM_KD_EA
 *  \brief   Numerator of default derivative gain for Euler Angle control loops
 */
#define CTRL_DEFAULT_NUM_KD_EA 0



///////////////////////////////////////////////
// TYPEDEFS

/**
 * \struct _drone_angular_rate_references_
 * \brief  Current pitch (p), roll (q) and yaw (r) angular rate references in mdeg/s.
 *
 * These are the references for the angular rate control loop.
 */
typedef struct _drone_angular_rate_references_
{
  int p;  /**< Angular rate reference in mdeg/s about x axis */
  int q;  /**< Angular rate reference in mdeg/s about y axis */
  int r;  /**< Angular rate reference in mdeg/s about z axis */

} drone_angular_rate_references_t;

/**
 * \struct _drone_euler_angles_references_
 * \brief  Current Euler angle references in mdeg.
 *
 * These are the references for the Euler angle control loop. psi is currently unused.
 */
typedef struct _drone_euler_angles_references_
{
  int theta;    /**< theta Euler angle reference in mdeg */
  int phi;      /**< phi Euler angle reference in mdeg */
  int psi;      /**< psi Euler angle reference in mdeg */

} drone_euler_angles_references_t;

#ifdef _MOBILE
#define BOOL 	unsigned int
#endif


#ifndef MAX
#define MAX(A,B) \
	((A) > (B) ? A : B)
#endif


#ifdef _MOBILE
#ifndef MIN
#define MIN(A,B) \
	((A) < (B) ? A : B)
#endif

#endif


#ifndef ABS
#define ABS(A) \
	((A) >= 0 ? A : -(A))
#endif


#endif // !_COMMON_INCLUDE_
