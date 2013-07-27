#include <linux/joystick.h>
#include <ardrone_api.h>
#include <VP_Os/vp_os_print.h>

#include "ardrone_ini.h"

Controller_info *control;
Controller_info *default_control;
GList *devices;

typedef struct {
  int32_t bus;
  int32_t vendor;
  int32_t product;
  int32_t version;
  char    name[MAX_NAME_LENGTH];
  char    handlers[MAX_NAME_LENGTH];
} device_t;

extern int32_t MiscVar[];

input_device_t control_device = {
  "Control",
  open_control_device,
  update_control_device,
  close_control_device
};

static int32_t joy_dev = 0;

C_RESULT open_control_device(void) {
  C_RESULT res = C_FAIL;

  printf("%s\n", default_control->filename);
  if (default_control->serial) {
    gchar **str;
    str = g_strsplit(default_control->filename, "/", -1);
    strncpy(control_device.name, str[3], MAX_NAME_LENGTH);
  } else {
    strncpy(control_device.name, "strdup", MAX_NAME_LENGTH);
  }
  joy_dev = open(default_control->filename, O_NONBLOCK | O_RDONLY);

  res = C_OK;
  
  return res;
}

C_RESULT update_control_device(void) {
  static int32_t stick1LR = 0, stick1UD = 0 , stick2LR = 0 , stick2UD = 0;
  //static int32_t x = 0, y = 0;
  static bool_t refresh_values = FALSE;
  ssize_t res;
  static struct js_event js_e_buffer[64];
  static int32_t start = 0;
  input_state_t* input_state;
  
  static int center_x1=0;
  static int center_y1=0;
  static int center_x2=0;
  static int center_y2=0;

/*  static int32_t theta_trim = 0;
  static int32_t phi_trim   = 0;
  static int32_t yaw_trim   = 0;*/

  if (default_control->config)
    return C_OK;
  
  res = read(joy_dev, js_e_buffer, sizeof(struct js_event) * 64);

  if( !res || (res < 0 && errno == EAGAIN) )
    return C_OK;

  if( res < 0 )
    return C_FAIL;
    
  if (res < (int) sizeof(struct js_event)) // If non-complete bloc: ignored
    return C_OK;

  // Buffer decomposition in blocs (if the last is incomplete, it's ignored)
  int32_t idx = 0;
  refresh_values = FALSE;
  input_state = ardrone_tool_get_input_state();
  for (idx = 0; idx < res / sizeof(struct js_event); idx++) {
    if (js_e_buffer[idx].type & JS_EVENT_INIT ) { // If Init, the first values are ignored
      break;
    } else if(js_e_buffer[idx].type & JS_EVENT_BUTTON ) { // Event Button detected
      if ((js_e_buffer[idx].number == default_control->commands[ROLL_LEFT].value) && (default_control->commands[ROLL_LEFT].type == BUTTON)) {
          //printf("buttons.control_ag : %d => %d\n", js_e_buffer[idx].number, js_e_buffer[idx].value);
          stick1LR = -32767 * js_e_buffer[idx].value;
		  } else if ((js_e_buffer[idx].number == default_control->commands[SPEED_DOWN].value) && (default_control->commands[SPEED_DOWN].type == BUTTON)) {
          //printf("buttons.control_ab : %d => %d\n", js_e_buffer[idx].number, js_e_buffer[idx].value);
          stick2UD = 32767 * js_e_buffer[idx].value;
		  } else if ((js_e_buffer[idx].number == default_control->commands[ROLL_RIGHT].value) && (default_control->commands[ROLL_RIGHT].type == BUTTON)) {
          //printf("buttons.control_ad : %d => %d\n", js_e_buffer[idx].number, js_e_buffer[idx].value);
          stick1LR = 32767 * js_e_buffer[idx].value;
		  } else if ((js_e_buffer[idx].number == default_control->commands[SPEED_UP].value) && (default_control->commands[SPEED_UP].type == BUTTON)) {
          //printf("buttons.control_ah : %d => %d\n", js_e_buffer[idx].number, js_e_buffer[idx].value);
          stick2UD = -32767 * js_e_buffer[idx].value;
		  } else if ((js_e_buffer[idx].number == default_control->commands[YAW_LEFT].value) && (default_control->commands[YAW_LEFT].type == BUTTON)) {
          //printf("buttons.control_l1 : %d => %d\n", js_e_buffer[idx].number, js_e_buffer[idx].value);
          stick2LR = -32767 * js_e_buffer[idx].value;
		  } else if ((js_e_buffer[idx].number == default_control->commands[YAW_RIGHT].value) && (default_control->commands[YAW_RIGHT].type == BUTTON)) {
          //printf("buttons.control_r1 : %d => %d\n", js_e_buffer[idx].number, js_e_buffer[idx].value);
          stick2LR = 32767 * js_e_buffer[idx].value;
		  } else if ((js_e_buffer[idx].number == default_control->commands[EMERGENCY].value) && (default_control->commands[EMERGENCY].type == BUTTON)) {
          //printf("buttons.control_select : %d => %d\n", js_e_buffer[idx].number, js_e_buffer[idx].value);
          ardrone_tool_set_ui_pad_select(js_e_buffer[idx].value);
		  } else if ((js_e_buffer[idx].number == default_control->commands[START].value) && (default_control->commands[START].type == BUTTON)) {
          //printf("buttons.control_start : %d => %d\n", js_e_buffer[idx].number, js_e_buffer[idx].value);
          if( js_e_buffer[idx].value ) {
            start ^= 1;
            ardrone_tool_set_ui_pad_start( start );
  		    }
		  } else if ((js_e_buffer[idx].number == default_control->commands[PITCH_FRONT].value) && (default_control->commands[PITCH_FRONT].type == BUTTON)) {
          //printf("buttons.pitch_front : %d => %d\n", js_e_buffer[idx].number, js_e_buffer[idx].value);
		      stick1UD = -32767;
		  } else if ((js_e_buffer[idx].number == default_control->commands[PITCH_BACK].value) && (default_control->commands[PITCH_BACK].type == BUTTON)) {
          //printf("buttons.pitch_back : %d => %d\n", js_e_buffer[idx].number, js_e_buffer[idx].value);
		      stick1UD = 32767;
      }
    } else if(js_e_buffer[idx].type & JS_EVENT_AXIS ) { // Event Axis detected
      refresh_values = TRUE;
      int axis_value = (js_e_buffer[idx].number + 1) * (js_e_buffer[idx].value > 0 ? 1 : -1);
      if ((axis_value == default_control->commands[PITCH_FRONT].value) && (default_control->commands[PITCH_FRONT].type == AXIS)) {
        //printf("axis.pitch_front : %d => %d (%d)\n", js_e_buffer[idx].number, js_e_buffer[idx].value, ( js_e_buffer[idx].value + 1 ) >> 15);
        stick1UD = ( js_e_buffer[idx].value );
		  } else if ((axis_value == default_control->commands[PITCH_BACK].value) && (default_control->commands[PITCH_BACK].type == AXIS)) {
        //printf("axis.pitch_back : %d => %d (%d)\n", js_e_buffer[idx].number, js_e_buffer[idx].value, ( js_e_buffer[idx].value + 1 ) >> 15);
        stick1UD = ( js_e_buffer[idx].value );
		  } else if ((axis_value == default_control->commands[ROLL_LEFT].value) && (default_control->commands[ROLL_LEFT].type == AXIS)) {
        //printf("axis.roll_left : %d => %d (%d)\n", js_e_buffer[idx].number, js_e_buffer[idx].value, ( js_e_buffer[idx].value + 1 ) >> 15);
        stick1LR = ( js_e_buffer[idx].value );
		  } else if ((axis_value == default_control->commands[ROLL_RIGHT].value) && (default_control->commands[ROLL_RIGHT].type == AXIS)) {
        //printf("axis.roll_right : %d => %d (%d)\n", js_e_buffer[idx].number, js_e_buffer[idx].value, ( js_e_buffer[idx].value + 1 ) >> 15);
        stick1LR = ( js_e_buffer[idx].value );
		  } else if ((axis_value == default_control->commands[SPEED_UP].value) && (default_control->commands[SPEED_UP].type == AXIS)) {
        //printf("axis.speed_up : %d => %d (%d)\n", js_e_buffer[idx].number, js_e_buffer[idx].value, ( js_e_buffer[idx].value + 1 ) >> 15);
        stick2UD = ( js_e_buffer[idx].value );
		  } else if ((axis_value == default_control->commands[SPEED_DOWN].value) && (default_control->commands[SPEED_DOWN].type == AXIS)) {
        //printf("axis.speed_down : %d => %d (%d)\n", js_e_buffer[idx].number, js_e_buffer[idx].value, ( js_e_buffer[idx].value + 1 ) >> 15);
        stick2UD = ( js_e_buffer[idx].value );
		  } else if ((axis_value == default_control->commands[YAW_LEFT].value) && (default_control->commands[YAW_LEFT].type == AXIS)) {
        //printf("axis.yaw_left : %d => %d (%d)\n", js_e_buffer[idx].number, js_e_buffer[idx].value, ( js_e_buffer[idx].value + 1 ) >> 15);
        stick2LR = ( js_e_buffer[idx].value );
		  } else if ((axis_value == default_control->commands[YAW_RIGHT].value) && (default_control->commands[YAW_RIGHT].type == AXIS)) {
        //printf("axis.yaw_right : %d => %d (%d)\n", js_e_buffer[idx].number, js_e_buffer[idx].value, ( js_e_buffer[idx].value + 1 ) >> 15);
        stick2LR = ( js_e_buffer[idx].value );
      }

    } else {
      // TODO: default: ERROR (non-supported)
    }
  }

  //if(refresh_values)// Axis values to refresh
    {
    //printf("roll : %f, pitch : %f, gaz : %f, yaw : %f\n", 
    //                                /*roll*/(float)(stick1LR-center_x1)/32767.0f,
    //                                /*pitch*/(float)(stick1UD-center_y1)/32767.0f,
    //                                /*gaz*/-(float)(stick2UD-center_x2)/32767.0f,
    //                                /*yaw*/(float)(stick2LR-center_y2)/32767.0f);
      int enable = 1;
	if (	327 > stick1LR && -327 < stick1LR &&
		327 > stick1UD && -327 < stick1UD &&
		327 > stick2LR && -327 < stick2LR &&
		327 > stick2UD && -327 < stick2UD) 
	{
		enable = 0;
	}
      ardrone_at_set_progress_cmd( enable,
                                    /*roll*/(float)(stick1LR-center_x1)/32767.0f,
                                    /*pitch*/(float)(stick1UD-center_y1)/32767.0f,
                                    /*gaz*/-(float)(stick2UD-center_x2)/32767.0f,
                                    /*yaw*/(float)(stick2LR-center_y2)/32767.0f );
    }

  return C_OK;
}

C_RESULT close_control_device(void) {
  close( joy_dev );

  return C_OK;
}

uint32_t uint32_atoi(char* buf){
	uint32_t ret = 0;
	char tmp;
	while((*buf >= '0' && *buf <= '9') || (*buf >= 'a' && *buf <= 'f') || (*buf >= 'A' && *buf <= 'F')){
		tmp = *buf;
		if(tmp >= 'a' && tmp <= 'f')
			tmp -= 'a' - 'A';
		if(tmp >= 'A' && tmp <= 'F')
			tmp -= 0x7;
		tmp -= 0x30;
		
		ret = ret << 4;
		ret |= tmp;
		
		buf++;
	}
	return ret;
}

/**
 * Search all Joystick devices
 */
gboolean search_devices(GList **list_controllers) {
  int i;
  Controller_info *ctrl_info;
  struct udev *udev;
  struct udev_device *dev;

  for(i = 0; i < 32; ++i) {
    gchar *str = NULL;
    str = g_strdup_printf("/dev/input/js%d", i);
    int fd = open(str, O_RDONLY);
    if (fd < 0) {
      //printf("Could not found joystick: %s\n", str->str);
      break;
    } else {
      ctrl_info = g_malloc(sizeof(Controller_info));
      uint8_t num_axis   = 0;
      uint8_t num_button = 0;
      ioctl(fd, JSIOCGAXES,    &num_axis);
      ioctl(fd, JSIOCGBUTTONS, &num_button);
      ctrl_info->filename    = g_strdup(str);
      ctrl_info->num_axis    = num_axis;
      ctrl_info->num_buttons = num_button;
      // Get Name 
      char name_c_str[1024];
      if (ioctl(fd, JSIOCGNAME(sizeof(name_c_str)), name_c_str) < 0) {
          printf("%s : %s", str, strerror(errno));
          break;
      } else {
         ctrl_info->name = g_convert_with_fallback(name_c_str, sizeof(name_c_str), "UTF-8", "ISO-8859-1", NULL, NULL, NULL, NULL);
      }

      /* Create the udev object */
      udev = udev_new();
      if (!udev) {
        printf("Can't create udev\n");
        exit(0);
      }

      dev = udev_device_new_from_subsystem_sysname(udev, "input", g_strdup_printf("js%d", i));
      if (dev == NULL)
            break;

      ctrl_info->serial = uint32_atoi(g_strdup_printf("%s%s", udev_list_entry_get_value(udev_list_entry_get_by_name(udev_device_get_properties_list_entry(dev), "ID_VENDOR_ID")), udev_list_entry_get_value(udev_list_entry_get_by_name(udev_device_get_properties_list_entry(dev), "ID_MODEL_ID"))));

      udev_device_unref(dev);
      udev_unref(udev);
      printf("%s : %d, %d, 0x%08x\n", ctrl_info->name, ctrl_info->num_axis, ctrl_info->num_buttons, ctrl_info->serial);
    }
    *list_controllers = g_list_append(*list_controllers, ctrl_info);
    close(fd);
  }

  return TRUE;
}

void parseControls(xmlDocPtr doc, xmlNodePtr cur) {
  cur = cur->xmlChildrenNode;
  while (cur != NULL) {
    if ((!xmlStrcmp(cur->name, (const xmlChar *)"control"))) {
      if ((!xmlStrcmp(xmlGetProp(cur, BAD_CAST "name"), (const xmlChar *) "takeoff"))) {
        control->commands[START].value = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "value")));
        control->commands[START].type = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "type")));
      } else if ((!xmlStrcmp(xmlGetProp(cur, BAD_CAST "name"), (const xmlChar *) "emergency"))) {
        control->commands[EMERGENCY].value = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "value")));
        control->commands[EMERGENCY].type = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "type")));
      } else if ((!xmlStrcmp(xmlGetProp(cur, BAD_CAST "name"), (const xmlChar *) "pitch_front"))) {
        control->commands[PITCH_FRONT].value = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "value")));
        control->commands[PITCH_FRONT].type = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "type")));
      } else if ((!xmlStrcmp(xmlGetProp(cur, BAD_CAST "name"), (const xmlChar *) "pitch_back"))) {
        control->commands[PITCH_BACK].value = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "value")));
        control->commands[PITCH_BACK].type = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "type")));
      } else if ((!xmlStrcmp(xmlGetProp(cur, BAD_CAST "name"), (const xmlChar *) "roll_left"))) {
        control->commands[ROLL_LEFT].value = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "value")));
        control->commands[ROLL_LEFT].type = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "type")));
      } else if ((!xmlStrcmp(xmlGetProp(cur, BAD_CAST "name"), (const xmlChar *) "roll_right"))) {
        control->commands[ROLL_RIGHT].value = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "value")));
        control->commands[ROLL_RIGHT].type = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "type")));
      } else if ((!xmlStrcmp(xmlGetProp(cur, BAD_CAST "name"), (const xmlChar *) "yaw_left"))) {
        control->commands[YAW_LEFT].value = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "value")));
        control->commands[YAW_LEFT].type = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "type")));
      } else if ((!xmlStrcmp(xmlGetProp(cur, BAD_CAST "name"), (const xmlChar *) "yaw_right"))) {
        control->commands[YAW_RIGHT].value = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "value")));
        control->commands[YAW_RIGHT].type = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "type")));
      } else if ((!xmlStrcmp(xmlGetProp(cur, BAD_CAST "name"), (const xmlChar *) "speed_up"))) {
        control->commands[SPEED_UP].value = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "value")));
        control->commands[SPEED_UP].type = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "type")));
      } else if ((!xmlStrcmp(xmlGetProp(cur, BAD_CAST "name"), (const xmlChar *) "speed_down"))) {
        control->commands[SPEED_DOWN].value = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "value")));
        control->commands[SPEED_DOWN].type = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "type")));
      } 
    }
    cur = cur->next;
  }
}

void parseDevice(xmlDocPtr doc, xmlNodePtr cur) {
  cur = cur->xmlChildrenNode;
  while (cur != NULL) {
    if ((!xmlStrcmp(cur->name, (const xmlChar *)"controls"))) {
      parseControls(doc, cur);
    }
    cur = cur->next;
  }
}

void parseDevices(xmlDocPtr doc, xmlNodePtr cur) {
  cur = cur->xmlChildrenNode;
  while (cur != NULL) {
    if ((!xmlStrcmp(cur->name, (const xmlChar *)"device"))) {
      control = g_malloc(sizeof(Controller_info));
      control->serial = atoi(g_strdup((const char *) xmlGetProp(cur, BAD_CAST "id")));
      control->name = g_strdup((const char *) xmlGetProp(cur, BAD_CAST "name"));
      control->def = xmlStrEqual(xmlGetProp(cur, BAD_CAST "default"), (const xmlChar *)"yes") ? TRUE : FALSE;
      control->config = FALSE;
      if (!control->serial)
        control->filename = g_strdup("/dev/stdin");
      else
        control->filename = g_strdup("");
      printf("Device 0x%08x : %s (%s) (%s)\n", control->serial, control->name, (control->def)?"yes":"no", control->filename);
      parseDevice(doc, cur);
      devices = g_list_append(devices, control);
      if (control->def)
        default_control = control;
    }
    cur = cur->next;
  }
}

void load_ini() {
  xmlDocPtr doc;
  xmlNodePtr cur;
  GList* list_controllers = NULL;
  
  printf("Loading configuration file %s\n", FILENAME);

  doc = xmlParseFile(FILENAME);
  
  if (doc == NULL ) {
    printf("Document not parsed successfully. \n");
    create_ini();
    goto suite;
  }

  cur = xmlDocGetRootElement(doc);
  if (cur == NULL) {
    printf("Empty document\n");
    xmlFreeDoc(doc);
    create_ini();
    goto suite;
  }
  
  if (xmlStrcmp(cur->name, (const xmlChar *) "ardrone")) {
    printf("Document of the wrong type, root node != ardrone");
    xmlFreeDoc(doc);
    create_ini();
    goto suite;
  }
  
  cur = cur->xmlChildrenNode;
  while (cur != NULL) {
    if ((!xmlStrcmp(cur->name, (const xmlChar *)"devices"))){
      parseDevices (doc, cur);
    }
     
    cur = cur->next;
  }
  
  xmlFreeDoc(doc);

suite:
  search_devices(&list_controllers);
  GList *it, *it2;
  for (it = devices; it; it = it->next) {
    Controller_info *c;
    c = (Controller_info *) it->data;
    for (it2 = list_controllers; it2; it2 = it2->next) {
      Controller_info *c2;
      c2 = (Controller_info *) it2->data;
      if (c->serial == c2->serial) {
        c->filename = strdup(c2->filename);
        break;
      }
    }
  }
  for (it = list_controllers; it; it = it->next) {
    Controller_info *c;
    c = (Controller_info *) it->data;
    if (c->serial == default_control->serial)
      break;
  }
  if (!it) {
    default_control->def = FALSE;
    default_control = devices->data;
    default_control->def = TRUE;
  }
  
  printf("Loading complete\n");
}

void save_init(Controller_info *def) {
  xmlDocPtr doc;
  xmlNodePtr devices_xml, device_xml, controls_xml, control_xml;

  printf("Beginning writing %s\n", FILENAME);

  doc = xmlNewDoc(BAD_CAST "1.0");
  doc->children = xmlNewDocNode(doc, NULL, BAD_CAST "ardrone", NULL);
  devices_xml = xmlNewChild(doc->children, NULL, BAD_CAST "devices", NULL);

  GList *it;
  for (it = devices; it; it = it->next) {
    Controller_info *c;
    c = (Controller_info *) it->data;

    device_xml = xmlNewChild(devices_xml, NULL, BAD_CAST "device", NULL);
    xmlSetProp(device_xml, BAD_CAST "id", BAD_CAST g_strdup_printf("%u", c->serial));
    xmlSetProp(device_xml, BAD_CAST "name", BAD_CAST c->name);
    xmlSetProp(device_xml, BAD_CAST "default", BAD_CAST (c == def ? "yes" : "no"));
    controls_xml = xmlNewChild(device_xml, NULL, BAD_CAST "controls", NULL);
    control_xml = xmlNewChild(controls_xml, NULL, BAD_CAST "control", NULL);
    xmlSetProp(control_xml, BAD_CAST "name", BAD_CAST "takeoff");
    xmlSetProp(control_xml, BAD_CAST "value", BAD_CAST g_strdup_printf("%d", c->commands[START].value));
    xmlSetProp(control_xml, BAD_CAST "type", BAD_CAST g_strdup_printf("%d", c->commands[START].type));
    control_xml = xmlNewChild(controls_xml, NULL, BAD_CAST "control", NULL);
    xmlSetProp(control_xml, BAD_CAST "name", BAD_CAST "emergency");
    xmlSetProp(control_xml, BAD_CAST "value", BAD_CAST g_strdup_printf("%d", c->commands[EMERGENCY].value));
    xmlSetProp(control_xml, BAD_CAST "type", BAD_CAST g_strdup_printf("%d", c->commands[EMERGENCY].type));
    control_xml = xmlNewChild(controls_xml, NULL, BAD_CAST "control", NULL);
    xmlSetProp(control_xml, BAD_CAST "name", BAD_CAST "pitch_front");
    xmlSetProp(control_xml, BAD_CAST "value", BAD_CAST g_strdup_printf("%d", c->commands[PITCH_FRONT].value));
    xmlSetProp(control_xml, BAD_CAST "type", BAD_CAST g_strdup_printf("%d", c->commands[PITCH_FRONT].type));
    control_xml = xmlNewChild(controls_xml, NULL, BAD_CAST "control", NULL);
    xmlSetProp(control_xml, BAD_CAST "name", BAD_CAST "pitch_back");
    xmlSetProp(control_xml, BAD_CAST "value", BAD_CAST g_strdup_printf("%d", c->commands[PITCH_BACK].value));
    xmlSetProp(control_xml, BAD_CAST "type", BAD_CAST g_strdup_printf("%d", c->commands[PITCH_BACK].type));
    control_xml = xmlNewChild(controls_xml, NULL, BAD_CAST "control", NULL);
    xmlSetProp(control_xml, BAD_CAST "name", BAD_CAST "roll_left");
    xmlSetProp(control_xml, BAD_CAST "value", BAD_CAST g_strdup_printf("%d", c->commands[ROLL_LEFT].value));
    xmlSetProp(control_xml, BAD_CAST "type", BAD_CAST g_strdup_printf("%d", c->commands[ROLL_LEFT].type));
    control_xml = xmlNewChild(controls_xml, NULL, BAD_CAST "control", NULL);
    xmlSetProp(control_xml, BAD_CAST "name", BAD_CAST "roll_right");
    xmlSetProp(control_xml, BAD_CAST "value", BAD_CAST g_strdup_printf("%d", c->commands[ROLL_RIGHT].value));
    xmlSetProp(control_xml, BAD_CAST "type", BAD_CAST g_strdup_printf("%d", c->commands[ROLL_RIGHT].type));
    control_xml = xmlNewChild(controls_xml, NULL, BAD_CAST "control", NULL);
    xmlSetProp(control_xml, BAD_CAST "name", BAD_CAST "yaw_left");
    xmlSetProp(control_xml, BAD_CAST "value", BAD_CAST g_strdup_printf("%d", c->commands[YAW_LEFT].value));
    xmlSetProp(control_xml, BAD_CAST "type", BAD_CAST g_strdup_printf("%d", c->commands[YAW_LEFT].type));
    control_xml = xmlNewChild(controls_xml, NULL, BAD_CAST "control", NULL);
    xmlSetProp(control_xml, BAD_CAST "name", BAD_CAST "yaw_right");
    xmlSetProp(control_xml, BAD_CAST "value", BAD_CAST g_strdup_printf("%d", c->commands[YAW_RIGHT].value));
    xmlSetProp(control_xml, BAD_CAST "type", BAD_CAST g_strdup_printf("%d", c->commands[YAW_RIGHT].type));
    control_xml = xmlNewChild(controls_xml, NULL, BAD_CAST "control", NULL);
    xmlSetProp(control_xml, BAD_CAST "name", BAD_CAST "speed_up");
    xmlSetProp(control_xml, BAD_CAST "value", BAD_CAST g_strdup_printf("%d", c->commands[SPEED_UP].value));
    xmlSetProp(control_xml, BAD_CAST "type", BAD_CAST g_strdup_printf("%d", c->commands[SPEED_UP].type));
    control_xml = xmlNewChild(controls_xml, NULL, BAD_CAST "control", NULL);
    xmlSetProp(control_xml, BAD_CAST "name", BAD_CAST "speed_down");
    xmlSetProp(control_xml, BAD_CAST "value", BAD_CAST g_strdup_printf("%d", c->commands[SPEED_DOWN].value));
    xmlSetProp(control_xml, BAD_CAST "type", BAD_CAST g_strdup_printf("%d", c->commands[SPEED_DOWN].type));
  }
  xmlKeepBlanksDefault(0);
  xmlSaveFormatFile(FILENAME, doc, 1);

  default_control = def;
  printf("Writing %s OK\n", FILENAME);
}

/**
 * Create the initial configuration file with default keyboard values
 */
void create_ini() {
  int i;
  
  control = g_malloc(sizeof(Controller_info));
  control->serial = 0;
  control->name = g_strdup("Keyboard");
  control->def = TRUE;
  control->config = FALSE;
  control->filename = g_strdup("/dev/stdin");
  control->commands[START].value = 13;           // start decollage / atterissage
  control->commands[EMERGENCY].value = 32;       // emergency coupe les moteurs
  control->commands[SPEED_DOWN].value = 88;      // descendre
  control->commands[SPEED_UP].value = 90;        // monter
  control->commands[ROLL_LEFT].value = 81;       // tourner a gauche
  control->commands[ROLL_RIGHT].value = 68;      // tourner a droite
  control->commands[YAW_LEFT].value = 65361;     // pivoter a gauche
  control->commands[YAW_RIGHT].value = 65363;    // pivoter a droite
  control->commands[PITCH_FRONT].value = 65362;  // deplacement en avant
  control->commands[PITCH_BACK].value = 65364;   // deplacement en arriere
  for (i = 0; i < NUM_COMMAND; i++) {
    control->commands[i].type = NONE;
  }
  
  devices = g_list_append(devices, control);
  default_control = control;

  save_init(default_control);
}
