
#ifndef VIEW_DRONE_ATTITUDE_H
#define VIEW_DRONE_ATTITUDE_H 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <gtk/gtk.h>

#define START_BUTTON_DA_SIZE 18

#ifdef USE_ARDRONE_VICON
#define VICON_BUTTON_DA_SIZE 18
#endif

void set_control_state(int32_t control_state);

#ifdef USE_ARDRONE_VICON
void set_vicon_state(int32_t vicon_state);
#endif

gboolean update_display(gpointer pData);

#endif  //  VIEW_DRONE_ATTITUDE_H 
