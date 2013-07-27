#include <ardrone_api.h>
#include "vicon_client/vicon_client.h"
#include "vicon_client/vicon_ihm.h"

C_RESULT vicon_at_client_init( void* data )
{
	return C_OK;
}

C_RESULT vicon_at_client_process( const vicon_data_t* const vicon_data )
{
	ardrone_at_set_vicon_data(vicon_data->time, vicon_data->frame_number, vicon_data->latency, vicon_data->global_translation[0], vicon_data->global_rotation_eulerXYZ[0]);
	return C_OK;
}

C_RESULT vicon_at_client_release( void )
{
  return C_OK;
}

BEGIN_VICON_HANDLER_TABLE
	VICON_HANDLER_TABLE_ENTRY(vicon_file_init, vicon_file_process, vicon_file_release, NULL)
	VICON_HANDLER_TABLE_ENTRY(vicon_ihm_init, vicon_ihm_process, vicon_ihm_release, NULL)
	VICON_HANDLER_TABLE_ENTRY(vicon_at_client_init, vicon_at_client_process, vicon_at_client_release, NULL)
END_VICON_HANDLER_TABLE
