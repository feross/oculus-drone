/**
 * @file mobile_main.c
 * @author aurelien.morelle@parrot.com & sylvain.gaeremynck@parrot.com
 * @date 2006/05/01
 */

#include <ATcodec/ATcodec_api.h>

#include <ardrone_tool/ardrone_tool.h>
#include <ardrone_tool/Control/ardrone_control.h>
#include <ardrone_tool/Navdata/ardrone_navdata_client.h>
#include <ardrone_tool/Com/config_com.h>

#include <common/mobile_config.h>
//#include <UI/gamepad.h>
//#include <UI/wiimote.h>
#include <ihm/ihm.h>
#include <ihm/ihm_stages_o_gtk.h>
#include <navdata_client/navdata_client.h>

#ifdef USE_ARDRONE_VICON
#include <libViconDataStreamSDK/vicon.h>
#endif // USE_ARDRONE_VICON

#ifdef PC_USE_POLARIS
#include <libPolaris/polaris.h>
#endif // PC_USE_POLARIS

#ifdef USE_TABLE_PILOTAGE
#include "libTestBenchs/novadem.h"
#endif // USE_TABLE_PILOTAGE

#ifdef RAW_CAPTURE
PROTO_THREAD_ROUTINE(raw_capture, params);
#endif
PROTO_THREAD_ROUTINE(remote_console, params);

static mobile_config_t cfg;

static int32_t exit_ihm_program = 1;

extern Controller_info *default_control;
extern GList *devices;

C_RESULT ardrone_tool_init_custom(int argc, char **argv)
{
  gtk_init(&argc, &argv);

  /// Init specific code for application
  ardrone_navdata_handler_table[NAVDATA_IHM_PROCESS_INDEX].data   = &cfg;

  // Add inputs
  //ardrone_tool_input_add( &gamepad );
  /*ardrone_tool_input_add( &radioGP );
  ardrone_tool_input_add( &ps3pad );
  ardrone_tool_input_add( &joystick );
  ardrone_tool_input_add( &wiimote_device );*/

  
  load_ini();
  printf("Default control : %s (0x%08x, %s)\n", default_control->name, default_control->serial, default_control->filename);
  ardrone_tool_input_add( &control_device );
  cfg.default_control = default_control;
  cfg.devices = devices;

#ifdef USE_ARDRONE_VICON
  START_THREAD( vicon, &cfg);
#endif // USE_ARDRONE_VICON

  START_THREAD(ihm, &cfg);
#ifdef RAW_CAPTURE
  START_THREAD(raw_capture, &cfg);
#endif
  START_THREAD(remote_console, &cfg);
	START_THREAD(ihm_stages_vision, &cfg);
#ifdef PC_USE_POLARIS
  START_THREAD( polaris, &cfg );
#endif // PC_USE_POLARIS
#ifdef USE_TABLE_PILOTAGE
  START_THREAD( novadem, (void*)("/dev/ttyUSB0") );
#endif // USE_TABLE_PILOTAGE

  return C_OK;
}

C_RESULT ardrone_tool_shutdown_custom()
{
#ifdef USE_TABLE_PILOTAGE
  JOIN_THREAD( novadem );
#endif // USE_TABLE_PILOTAGE
#ifdef PC_USE_POLARIS
  JOIN_THREAD( polaris );
#endif // PC_USE_POLARIS
#ifdef USE_ARDRONE_VICON
  JOIN_THREAD( vicon );
#endif // USE_ARDRONE_VICON
  JOIN_THREAD(ihm);
  JOIN_THREAD(ihm_stages_vision);
#ifdef RAW_CAPTURE
  JOIN_THREAD(raw_capture);
#endif
  JOIN_THREAD(remote_console);

  /*ardrone_tool_input_remove( &gamepad );
  ardrone_tool_input_remove( &radioGP );
  ardrone_tool_input_remove( &ps3pad );
  ardrone_tool_input_remove( &wiimote_device );*/
  ardrone_tool_input_remove( &control_device );

  return C_OK;
}

C_RESULT signal_exit()
{
  exit_ihm_program = 0;

  ihm_destroyCurves();

  return C_OK;
}

bool_t ardrone_tool_exit()
{
  return exit_ihm_program == 0;
}

BEGIN_THREAD_TABLE
  THREAD_TABLE_ENTRY( ihm, 20 )
#ifdef RAW_CAPTURE
  THREAD_TABLE_ENTRY( raw_capture, 20 )
#endif
  THREAD_TABLE_ENTRY( remote_console, 20 )
  THREAD_TABLE_ENTRY( ihm_stages_vision, 20 )
  THREAD_TABLE_ENTRY( navdata_update, 20 )
  THREAD_TABLE_ENTRY( ATcodec_Commands_Client, 20 )
  THREAD_TABLE_ENTRY( ardrone_control, 20 )
#ifdef PC_USE_POLARIS
  THREAD_TABLE_ENTRY( polaris, 20 )
#endif // PC_USE_POLARIS
#ifdef USE_ARDRONE_VICON
  THREAD_TABLE_ENTRY( vicon, 20 )
#endif // USE_ARDRONE_VICON
#ifdef USE_TABLE_PILOTAGE
  THREAD_TABLE_ENTRY( novadem, 20 )
#endif // USE_TABLE_PILOTAGE
END_THREAD_TABLE

