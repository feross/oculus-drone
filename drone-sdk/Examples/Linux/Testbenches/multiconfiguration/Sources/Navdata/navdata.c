#include <ardrone_tool/Navdata/ardrone_navdata_client.h>

#include <Navdata/navdata.h>

#include <stdio.h>

#define CTRL_STATES_STRING
#include "control_states.h"

/* Initialization local variables before event loop  */
inline C_RESULT demo_navdata_client_init( void* data )
{
  return C_OK;
}

#define MAX_STR_CTRL_STATE 2048
/*============================================================================*/
/**
  * Builds a string describing the drone control state.
  * @param ctrl_state Integer containing the control state from the unpacked navdata.
  * @return A pointer to the built string. The returned address always points to the last built string.
  */
const char* ctrl_state_str(uint32_t ctrl_state)
{
  static char str_ctrl_state[MAX_STR_CTRL_STATE];

  ctrl_string_t* ctrl_string;
  uint32_t major = ctrl_state >> 16;
  uint32_t minor = ctrl_state & 0xFFFF;

  if( strlen(ctrl_states[major]) < MAX_STR_CTRL_STATE )
  {
    vp_os_memset(str_ctrl_state, 0, sizeof(str_ctrl_state));

    strcat(str_ctrl_state, ctrl_states[major]);
    ctrl_string = control_states_link[major];

    if( ctrl_string != NULL && (strlen(ctrl_states[major]) + strlen(ctrl_string[minor]) < MAX_STR_CTRL_STATE) )
    {
      strcat( str_ctrl_state, " | " );
      strcat( str_ctrl_state, ctrl_string[minor] );
    }
  }
  else
  {
    vp_os_memset( str_ctrl_state, '#', sizeof(str_ctrl_state) );
  }

  return str_ctrl_state;
}

/* Receving navdata during the event loop */
inline C_RESULT demo_navdata_client_process( const navdata_unpacked_t* const navdata )
{
	static int cpt=0;

	return C_OK;

	const navdata_demo_t*nd = &navdata->navdata_demo;
	const navdata_vision_detect_t*nv = &navdata->navdata_vision_detect;

	printf("==========================\nNavdata for flight demonstrations \n==========================\n");

	printf("Control state : %s\n",ctrl_state_str(nd->ctrl_state));
	printf("Battery level : %i %%\n",nd->vbat_flying_percentage);
	printf("Orientation   : [Theta] %4.3f  [Phi] %4.3f  [Psi] %4.3f\n",nd->theta,nd->phi,nd->psi);
	printf("Altitude      : %i\n",nd->altitude);
	printf("Speed         : [vX] %4.3f  [vY] %4.3f  [vZPsi] %4.3f\n",nd->vx,nd->vy,nd->vz);
	printf("Position      : [x] %4.3f [Y] %4.3f [Z] %4.3f\n",
							nd->drone_camera_trans.x,
							nd->drone_camera_trans.y,
							nd->drone_camera_trans.z);

	printf("Detec. trans. : [x] %4.3f [Y] %4.3f [Z] %4.3f\n",
								nd->detection_camera_trans.x,
								nd->detection_camera_trans.y,
								nd->detection_camera_trans.z);

	printf("Orientation   :\n%4.3f | %4.3f | %4.3f\n%4.3f | %4.3f | %4.3f\n%4.3f | %4.3f | %4.3f\n",
								nd->drone_camera_rot.m11,nd->drone_camera_rot.m12,nd->drone_camera_rot.m13,
								nd->drone_camera_rot.m21,nd->drone_camera_rot.m22,nd->drone_camera_rot.m23,
								nd->drone_camera_rot.m31,nd->drone_camera_rot.m32,nd->drone_camera_rot.m33 );

	printf("Detec. rot.   :\n%4.3f | %4.3f | %4.3f\n%4.3f | %4.3f | %4.3f\n%4.3f | %4.3f | %4.3f\n",
								nd->detection_camera_rot.m11,nd->detection_camera_rot.m12,nd->detection_camera_rot.m13,
								nd->detection_camera_rot.m21,nd->detection_camera_rot.m22,nd->detection_camera_rot.m23,
								nd->detection_camera_rot.m31,nd->detection_camera_rot.m32,nd->detection_camera_rot.m33 );


	printf("=====================\nNavdata for detection \n=====================\n");
	printf("Type     :  %i\n",nd->detection_camera_type);
	printf("Tag index:  %i\n",nd->detection_tag_index);
	printf("Targets  :  %i\n",nv->nb_detected);
	printf("Target 1 :  [X] %4i    [Y] %4i\n",nv->xc[0],nv->yc[0]);
	printf("Target 2 :  [X] %4i    [Y] %4i\n",nv->xc[1],nv->yc[1]);

	printf("\033[27A");

	if ((cpt++)==100)
	  {
	    cpt=system("clear"); // Avoid compiler warning (unused return value)
	    cpt=0;
	  }

  return C_OK;
}

/* Relinquish the local resources after the event loop exit */
inline C_RESULT demo_navdata_client_release( void )
{
  return C_OK;
}

/* Registering to navdata client */
BEGIN_NAVDATA_HANDLER_TABLE
  NAVDATA_HANDLER_TABLE_ENTRY(demo_navdata_client_init, demo_navdata_client_process, demo_navdata_client_release, NULL)
END_NAVDATA_HANDLER_TABLE

