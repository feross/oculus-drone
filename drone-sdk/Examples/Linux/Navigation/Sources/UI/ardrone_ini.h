#ifndef _ARDRONE_INI_H_
#define _ARDRONE_INI_H_

#include <glib.h>
#include <libudev.h>
#include <linux/joystick.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/ioctl.h>

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "UI/ui.h"

#define FILENAME "ardrone.xml"

enum { NONE = 0, AXIS, HAT, BUTTON };

enum {
  START = 0,
  EMERGENCY,
  PITCH_FRONT,
  PITCH_BACK,
  ROLL_LEFT,
  ROLL_RIGHT,
  YAW_LEFT,
  YAW_RIGHT,
  SPEED_UP,
  SPEED_DOWN,
  NUM_COMMAND
};

typedef struct {
  int value;
  int type;
} Control_Type;

extern input_device_t control_device;

C_RESULT open_control_device(void);
C_RESULT update_control_device(void);
C_RESULT close_control_device(void);

typedef struct {
  int num_axis;
  int num_buttons;
  gchar *name;
  gchar *filename;
  int32_t serial;
  gboolean def;
  gboolean config;
  Control_Type commands[NUM_COMMAND];
} Controller_info;

extern Controller_info *control;
extern Controller_info *default_control;
extern GList *devices;

gboolean search_devices(GList **list_controllers);
void create_ini();
void load_ini();
void save_init(Controller_info *def);

#endif // _ARDRONE_INI_H_

