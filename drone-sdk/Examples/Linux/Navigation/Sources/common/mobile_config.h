/**
 * @file mobile_config.h
 * @author aurelien.morelle@parrot.fr
 * @date 2006/05/02
 */

#ifndef _MOBILE_CONFIG_H_
#define _MOBILE_CONFIG_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

#include "UI/ardrone_ini.h"
#include <ardrone_api.h>
#include <VP_Os/vp_os_signal.h>
#include <VP_Api/vp_api_picture.h>

#ifndef STREAM_WIDTH
#define STREAM_WIDTH QVGA_WIDTH
#endif
#ifndef STREAM_HEIGHT
#define STREAM_HEIGHT QVGA_HEIGHT
#endif
/**
 * \typdef  _DA_THREAD_STATES_
 * \brief drone_acquisition thread states.
*/
typedef struct _mobile_config_
{
  int width;
  int height;
  int exit_required;

  int new_frame;

  vp_os_mutex_t *picture_mutex;
  vp_os_mutex_t *key_mutex;
  vp_os_mutex_t *exit_mutex;

  char addr[19];

  int sock;
  int at_socket;

  int speed;
  int angle;

  int *speed_list;

  int up_pressed;
  int down_pressed;
  int left_pressed;
  int right_pressed;

  void (*manage_gamepad_state)(void);

  int joydev;
  int (* pool_events)(struct _mobile_config_ *cfg);

  Controller_info *default_control;
  GList *devices;

  FILE *sample_file;

  int rssi;

  int ihm_curve_alloc_OK;

#ifdef DRONE_XBEE_LINK
  int XBee_transmit_enabled;
#endif
} mobile_config_t;



int mobile_init_config(mobile_config_t *cfg);
void mobile_exit_config(mobile_config_t *cfg);

int pool_events(mobile_config_t *cfg);

int test_exit(mobile_config_t *cfg);
int do_exit(mobile_config_t *cfg);

#endif //! _MOBILE_CONFIG_H_

