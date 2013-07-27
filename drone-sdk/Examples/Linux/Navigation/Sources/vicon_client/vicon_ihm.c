#include <stdlib.h>

#include <VP_Os/vp_os_malloc.h>
#include <VP_Os/vp_os_print.h>
#include <vicon.h>

#include "ihm/view_drone_attitude.h"

C_RESULT vicon_ihm_init( void* data )
{
	set_vicon_state(0);
	return C_OK;
}

C_RESULT vicon_ihm_process( const vicon_data_t* const vicon_data )
{
	return C_OK;
}

C_RESULT vicon_ihm_release( void )
{
  return C_OK;
}
