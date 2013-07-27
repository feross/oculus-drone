#include "navdata.h"
#include "ARDroneTypes.h"
#include <control_states.h>
#include <ardrone_tool/Navdata/ardrone_navdata_file.h>
#include <ardrone_tool/Navdata/ardrone_navdata_client.h>
navdata_unpacked_t inst_nav;
vp_os_mutex_t inst_nav_mutex;
extern char root_dir[];
static bool_t writeToFile = FALSE;

static inline C_RESULT ardrone_navdata_init( void* data )
{
	vp_os_mutex_init( &inst_nav_mutex );
	
	vp_os_mutex_lock( &inst_nav_mutex);
	ardrone_navdata_reset_data(&inst_nav);
	vp_os_mutex_unlock( &inst_nav_mutex);
	
	writeToFile = FALSE;
	
	return C_OK;
}

static inline C_RESULT ardrone_navdata_process( const navdata_unpacked_t* const navdata )
{
	if( writeToFile )
	{
		if( navdata_file == NULL )
		{
			ardrone_navdata_file_init(root_dir);
			
			PRINT("Saving in %s file\n", root_dir);
		}
		ardrone_navdata_file_process( navdata );
	}
	else
	{
		if(navdata_file != NULL)
			ardrone_navdata_file_release();			
	}
	
	vp_os_mutex_lock( &inst_nav_mutex);
/*	inst_nav.ardrone_state = navdata->ardrone_state;
	inst_nav.vision_defined = navdata->vision_defined;
	vp_os_memcpy(&inst_nav.navdata_demo, &navdata->navdata_demo, sizeof(navdata_demo_t));
	vp_os_memcpy(&inst_nav.navdata_vision_detect, &navdata->navdata_vision_detect, sizeof(navdata_vision_detect_t));
*/
	vp_os_memcpy(&inst_nav, navdata, sizeof(navdata_unpacked_t));
	vp_os_mutex_unlock( &inst_nav_mutex );

	return C_OK;
}

static inline C_RESULT ardrone_navdata_release( void )
{
	ardrone_navdata_file_release();
	return C_OK;
}

C_RESULT ardrone_navdata_write_to_file(bool_t enable)
{
	writeToFile = enable;
	return C_OK;
}

C_RESULT ardrone_navdata_reset_data(navdata_unpacked_t *nav)
{
	C_RESULT result = C_FAIL;
	
	if(nav)
	{
		vp_os_memset(nav, 0x0, sizeof(navdata_unpacked_t));
		result = C_OK;
	}
	
	return result;
}	

C_RESULT ardrone_navdata_get_data(navdata_unpacked_t *data)
{
	C_RESULT result = C_FAIL;
	
	if(data)
	{
		vp_os_mutex_lock( &inst_nav_mutex );
/*		data->ardrone_state = inst_nav.ardrone_state;
		data->vision_defined = inst_nav.vision_defined;
		vp_os_memcpy(&data->navdata_demo, &inst_nav.navdata_demo, sizeof(navdata_demo_t));
		vp_os_memcpy(&data->navdata_vision_detect, &inst_nav.navdata_vision_detect, sizeof(navdata_vision_detect_t));
*/
		vp_os_memcpy(data, &inst_nav, sizeof(navdata_unpacked_t));
		vp_os_mutex_unlock( &inst_nav_mutex );
		result = C_OK;
	}
	
	return result;
}

ARDRONE_FLYING_STATE ardrone_navdata_get_flying_state(navdata_unpacked_t data)
{
	ARDRONE_FLYING_STATE tmp_state;
	switch ((data.navdata_demo.ctrl_state >> 16)) 
	{
		case CTRL_FLYING:
		case CTRL_HOVERING:
		case CTRL_TRANS_GOTOFIX:
			tmp_state = ARDRONE_FLYING_STATE_FLYING;
			break;
			
		case CTRL_TRANS_TAKEOFF:
			tmp_state = ARDRONE_FLYING_STATE_TAKING_OFF;
			break;
			
		case CTRL_TRANS_LANDING:
			tmp_state = ARDRONE_FLYING_STATE_LANDING;
			break;
			
		case CTRL_DEFAULT:
		case CTRL_LANDED:
		default:
			tmp_state = ARDRONE_FLYING_STATE_LANDED;
			break;
	}
	
	return tmp_state;
}

BEGIN_NAVDATA_HANDLER_TABLE
NAVDATA_HANDLER_TABLE_ENTRY(ardrone_navdata_init, ardrone_navdata_process, ardrone_navdata_release, NULL)
END_NAVDATA_HANDLER_TABLE
