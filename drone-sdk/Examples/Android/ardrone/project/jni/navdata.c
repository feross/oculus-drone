#include "navdata.h"

#include "app.h"
#include <control_states.h>
#include <ardrone_tool/Navdata/ardrone_navdata_file.h>
#include <ardrone_tool/Navdata/ardrone_navdata_client.h>
#include <VP_Os/vp_os_signal.h>

instance_navdata_t inst_nav;
vp_os_mutex_t inst_nav_mutex;

static inline C_RESULT ardrone_navdata_init( void* data )
{
	ardrone_navdata_reset_data(&inst_nav);

	vp_os_mutex_init( &inst_nav_mutex );
	
	return C_OK;
}

static inline C_RESULT ardrone_navdata_process( const navdata_unpacked_t* const navdata )
{
	vp_os_mutex_lock( &inst_nav_mutex );
	inst_nav.flyingState = ((navdata->navdata_demo.ctrl_state >> 16) == CTRL_FLYING) 
						|| ((navdata->navdata_demo.ctrl_state >> 16) == CTRL_HOVERING)
						|| ((navdata->navdata_demo.ctrl_state >> 16) == CTRL_TRANS_GOTOFIX);

	inst_nav.pitch = navdata->navdata_demo.theta / 1000;
	inst_nav.yaw = navdata->navdata_demo.psi / 1000;
	inst_nav.roll = navdata->navdata_demo.phi / 1000;
	inst_nav.trimPitch = navdata->navdata_trims.euler_angles_trim_theta;
	inst_nav.trimYaw = navdata->navdata_trims.angular_rates_trim_r;
	inst_nav.trimRoll = navdata->navdata_trims.euler_angles_trim_phi;					
	inst_nav.altitude = navdata->navdata_demo.altitude;
	inst_nav.vx = navdata->navdata_demo.vx;
	inst_nav.vy = navdata->navdata_demo.vy;
	inst_nav.vz = navdata->navdata_demo.vz;
	inst_nav.videoNumFrames = navdata->navdata_demo.num_frames;
	inst_nav.nbDetectedOpponent = navdata->navdata_vision_detect.nb_detected;
	vp_os_memcpy(inst_nav.xOpponent, navdata->navdata_vision_detect.xc, sizeof(int) * 4);
	vp_os_memcpy(inst_nav.yOpponent, navdata->navdata_vision_detect.yc, sizeof(int) * 4);
	vp_os_memcpy(inst_nav.widthOpponent, navdata->navdata_vision_detect.width, sizeof(int) * 4);
	vp_os_memcpy(inst_nav.heightOpponent, navdata->navdata_vision_detect.height, sizeof(int) * 4);
	vp_os_memcpy(inst_nav.distOpponent, navdata->navdata_vision_detect.dist, sizeof(int) * 4);
	vp_os_memcpy(inst_nav.detectCameraRot, &navdata->navdata_demo.detection_camera_rot, sizeof(float) * 9);
	vp_os_memcpy(inst_nav.detectCameraTrans, &navdata->navdata_demo.detection_camera_trans, sizeof(float) * 3);
	vp_os_memcpy(inst_nav.droneCameraRot, &navdata->navdata_demo.drone_camera_rot, sizeof(float) * 9);
	vp_os_memcpy(inst_nav.droneCameraTrans, &navdata->navdata_demo.drone_camera_trans, sizeof(float) * 3);

	if(inst_nav.detectionType != navdata->navdata_demo.detection_camera_type)
		printf("%s : detection type %d changed to %d\n", __FUNCTION__, inst_nav.detectionType,  navdata->navdata_demo.detection_camera_type);
	
	inst_nav.detectionType = navdata->navdata_demo.detection_camera_type;
	inst_nav.detectTagIndex = navdata->navdata_demo.detection_tag_index;
	inst_nav.ardrone_state = navdata->ardrone_state;
	inst_nav.commandState = (navdata->ardrone_state & ARDRONE_COMMAND_MASK);
	inst_nav.comWatchdog = (navdata->ardrone_state & ARDRONE_COM_WATCHDOG_MASK);
	inst_nav.emergencyLanding = (navdata->ardrone_state & ARDRONE_EMERGENCY_MASK);
	inst_nav.startPressed = (navdata->ardrone_state & ARDRONE_USER_FEEDBACK_START);
	inst_nav.cutoutProblem = (navdata->ardrone_state & ARDRONE_CUTOUT_MASK);
	inst_nav.motorsProblem = (navdata->ardrone_state & ARDRONE_MOTORS_MASK);
	inst_nav.cameraProblem = !(navdata->ardrone_state & ARDRONE_VIDEO_THREAD_ON);
	inst_nav.adcVersionProblem = !(navdata->ardrone_state & ARDRONE_PIC_VERSION_MASK);
	inst_nav.adcProblem = (navdata->ardrone_state & ARDRONE_ADC_WATCHDOG_MASK);
	inst_nav.ultrasoundProblem = (navdata->ardrone_state & ARDRONE_ULTRASOUND_MASK);
	inst_nav.visionProblem = !(navdata->ardrone_state & ARDRONE_VISION_MASK);
	inst_nav.anglesProblem = (navdata->ardrone_state & ARDRONE_ANGLES_OUT_OF_RANGE);
	inst_nav.vbatLowProblem = (navdata->ardrone_state & ARDRONE_VBAT_LOW);
	inst_nav.userEmergency = (navdata->ardrone_state & ARDRONE_USER_EL);
	inst_nav.timerElapsed = (navdata->ardrone_state & ARDRONE_TIMER_ELAPSED);
	inst_nav.videoThreadOn = (navdata->ardrone_state & ARDRONE_VIDEO_THREAD_ON);
	inst_nav.navdataThreadOn = (navdata->ardrone_state & ARDRONE_NAVDATA_THREAD_ON);
	inst_nav.bootstrap = (navdata->ardrone_state & ARDRONE_NAVDATA_BOOTSTRAP);
	inst_nav.remainingBattery = navdata->navdata_demo.vbat_flying_percentage;
    
	//PRINT( "command navdata demo:%d   ", (navdata->ardrone_state & ARDRONE_NAVDATA_DEMO_MASK) );
	
	
	vp_os_mutex_unlock( &inst_nav_mutex );

	return C_OK;
}

static inline C_RESULT ardrone_navdata_release( void )
{
	return C_OK;
}

C_RESULT ardrone_navdata_write_to_file(bool_t enable)
{
	return C_OK;
}

C_RESULT ardrone_navdata_reset_data(instance_navdata_t *nav)
{
	nav->ardrone_state = 0;
	nav->flyingState = FALSE;
	nav->commandState = FALSE;
	nav->comWatchdog = FALSE;
	nav->bootstrap = TRUE;
	nav->emergencyLanding = FALSE;
	nav->startPressed = FALSE;
	nav->cutoutProblem = FALSE;
	nav->motorsProblem = FALSE;
	nav->cameraProblem = FALSE;
	nav->adcVersionProblem =FALSE;
	nav->adcProblem = FALSE;
	nav->ultrasoundProblem = FALSE;
	nav->visionProblem = TRUE;
	nav->anglesProblem = FALSE;
	nav->vbatLowProblem = FALSE;
	nav->userEmergency = FALSE;
	nav->pitch = 0.0f;
	nav->yaw = 0.0f;
	nav->roll = 0.0f;
	nav->trimPitch = 0.0f;
	nav->trimYaw = 0.0f;
	nav->trimRoll = 0.0f;
	nav->altitude = 0.0f;
	nav->vx = 0.0f;
	nav->vy = 0.0f;
	nav->vz = 0.0f;
	nav->remainingBattery = -1;
	nav->nbDetectedOpponent = 0;
	nav->videoNumFrames = 0;
	vp_os_memset(nav->xOpponent, 0x0, (sizeof(int) * 4));
	vp_os_memset(nav->yOpponent, 0x0, (sizeof(int) * 4));
	vp_os_memset(nav->widthOpponent, 0x0, (sizeof(int) * 4));
	vp_os_memset(nav->heightOpponent, 0x0, (sizeof(int) * 4));
	vp_os_memset(nav->distOpponent, 0x0, (sizeof(int) * 4));
	vp_os_memset(nav->detectCameraRot, 0x0, (sizeof(float) * 9));
	vp_os_memset(nav->detectCameraHomo, 0x0, (sizeof(float) * 9));			
	vp_os_memset(nav->detectCameraTrans, 0x0, (sizeof(float) * 3));
	vp_os_memset(nav->droneCameraRot, 0x0, (sizeof(float) * 9));
	vp_os_memset(nav->droneCameraTrans, 0x0, (sizeof(float) * 3));
	
	nav->detectionType = CAD_TYPE_NUM;
	nav->detectTagIndex = 0;
	nav->timerElapsed = FALSE;
	nav->videoThreadOn = FALSE;
	nav->navdataThreadOn = FALSE;

	return C_OK;
}	

C_RESULT ardrone_navdata_get_data(instance_navdata_t *data)
{
	C_RESULT result = C_FAIL;
	
	if(data)
	{
		vp_os_mutex_lock( &inst_nav_mutex );
		vp_os_memcpy(data, &inst_nav, sizeof(instance_navdata_t));
		vp_os_mutex_unlock( &inst_nav_mutex );
		result = C_OK;
	}
	
	return result;
}

BEGIN_NAVDATA_HANDLER_TABLE
NAVDATA_HANDLER_TABLE_ENTRY(ardrone_navdata_init, ardrone_navdata_process, ardrone_navdata_release, NULL)
END_NAVDATA_HANDLER_TABLE
