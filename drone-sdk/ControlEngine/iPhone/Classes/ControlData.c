/*
 *  ControlData.m
 *  ARDroneEngine
 *
 *  Created by Frederic D'HAEYER on 14/01/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#include "ConstantsAndMacros.h"
#include "ControlData.h"

//#define DEBUG_CONTROL

ControlData ctrldata = { 0 };
navdata_unpacked_t ctrlnavdata;
extern char iphone_mac_address[];

void setApplicationDefaultConfig(void)
{
	ardrone_application_default_config.navdata_demo = TRUE;
	ardrone_application_default_config.navdata_options = (NAVDATA_OPTION_MASK(NAVDATA_DEMO_TAG) | NAVDATA_OPTION_MASK(NAVDATA_VISION_DETECT_TAG) | NAVDATA_OPTION_MASK(NAVDATA_GAMES_TAG));
	ardrone_application_default_config.video_codec = P264_CODEC;
	ardrone_application_default_config.bitrate_ctrl_mode = ARDRONE_VARIABLE_BITRATE_MODE_DYNAMIC;
	ctrldata.applicationDefaultConfigState = CONFIG_STATE_IDLE;
}

void config_callback(bool_t result)
{
	if(result)
		ctrldata.configurationState = CONFIG_STATE_IDLE;
}

void initControlData(void)
{
	ctrldata.framecounter = 0;
	
	ctrldata.needSetEmergency = FALSE;
	ctrldata.needSetTakeOff = FALSE;
	ctrldata.isInEmergency = FALSE;
	ctrldata.navdata_connected = FALSE;
	
	ctrldata.needAnimation = FALSE;
	vp_os_memset(ctrldata.needAnimationParam, 0, sizeof(ctrldata.needAnimationParam));
	
	ctrldata.needVideoSwitch = -1;
	
	ctrldata.needLedAnimation = FALSE;
	vp_os_memset(ctrldata.needLedAnimationParam, 0, sizeof(ctrldata.needLedAnimationParam));
	
	ctrldata.wifiReachabled = FALSE;
	
	strcpy(ctrldata.error_msg, "");
	strcpy(ctrldata.takeoff_msg, "take_off");
	strcpy(ctrldata.emergency_msg, "emergency");
	
	initNavdataControlData();
	resetControlData();
	ardrone_tool_start_reset();
	
	ctrldata.configurationState = CONFIG_STATE_NEEDED;
	
	ardrone_navdata_write_to_file(FALSE);
	
	/* Setting default values for ControlEngine */
	ctrldata.applicationDefaultConfigState = CONFIG_STATE_NEEDED;
}

void initNavdataControlData(void)
{
	//drone data
	ardrone_navdata_reset_data(&ctrlnavdata);
}

void resetControlData(void)
{
	//printf("reset control data\n");
	ctrldata.accelero_flag = 0;
	inputPitch(0.0);
	inputRoll(0.0);
	inputYaw(0.0);
	inputGaz(0.0);
	initNavdataControlData();
}

void configuration_get(void)
{
	if(ctrldata.configurationState == CONFIG_STATE_IDLE)
		ctrldata.configurationState = CONFIG_STATE_NEEDED;
}

void switchTakeOff(void)
{
#ifdef DEBUG_CONTROL
	PRINT("%s\n", __FUNCTION__);
#endif		
	ctrldata.needSetTakeOff = TRUE;
}

void emergency(void)
{
#ifdef DEBUG_CONTROL
	PRINT("%s\n", __FUNCTION__);
#endif
	ctrldata.needSetEmergency = TRUE;
}

void inputYaw(float percent)
{
#ifdef DEBUG_CONTROL
	PRINT("%s : %f\n", __FUNCTION__, percent);
#endif
	if(-1.0 <= percent && percent <= 1.0)
		ctrldata.yaw = percent;
	else if(-1.0 < percent)
		ctrldata.yaw = -1.0;
	else
		ctrldata.yaw = 1.0;
}

void inputGaz(float percent)
{
#ifdef DEBUG_CONTROL
	PRINT("%s : %f\n", __FUNCTION__, percent);
#endif
	if(-1.0 <= percent && percent <= 1.0)
		ctrldata.gaz = percent;
	else if(-1.0 < percent)
		ctrldata.gaz = -1.0;
	else
		ctrldata.gaz = 1.0;
}

void inputPitch(float percent)
{
#ifdef DEBUG_CONTROL
	PRINT("%s : %f, accelero_enable : %d\n", __FUNCTION__, percent, (ctrldata.accelero_flag >> ARDRONE_PROGRESSIVE_CMD_ENABLE) & 0x1 );
#endif
	if(-1.0 <= percent && percent <= 1.0)
		ctrldata.accelero_theta = -percent;
	else if(-1.0 < percent)
		ctrldata.accelero_theta = 1.0;
	else
		ctrldata.accelero_theta = -1.0;
}

void inputRoll(float percent)
{
#ifdef DEBUG_CONTROL
	PRINT("%s : %f, accelero_enable : %d\n", __FUNCTION__, percent, (ctrldata.accelero_flag >> ARDRONE_PROGRESSIVE_CMD_ENABLE) & 0x1);
#endif
	if(-1.0 <= percent && percent <= 1.0)
		ctrldata.accelero_phi = percent;
	else if(-1.0 < percent)
		ctrldata.accelero_phi = -1.0;
	else
		ctrldata.accelero_phi = 1.0;
}

void sendControls(void)
{
	ardrone_at_set_progress_cmd(ctrldata.accelero_flag, ctrldata.accelero_phi, ctrldata.accelero_theta, ctrldata.gaz, ctrldata.yaw);
}

void checkErrors(void)
{
	input_state_t* input_state = ardrone_tool_get_input_state();
	
	strcpy(ctrldata.error_msg, "");
	
	if(!ctrldata.wifiReachabled)
	{
		strcpy(ctrldata.error_msg, "WIFI NOT REACHABLE");
		resetControlData();
	}
	else
	{
		if(ctrldata.applicationDefaultConfigState == CONFIG_STATE_NEEDED)
		{
			ctrldata.applicationDefaultConfigState = CONFIG_STATE_IN_PROGRESS;
			setApplicationDefaultConfig();
		}
		
		if(ctrldata.configurationState == CONFIG_STATE_NEEDED)
		{
			ctrldata.configurationState = CONFIG_STATE_IN_PROGRESS;
			ARDRONE_TOOL_CONFIGURATION_GET(config_callback);
		}			
		
		if(ctrldata.needSetTakeOff)
		{
			if(ctrlnavdata.ardrone_state & ARDRONE_EMERGENCY_MASK)
			{
				ctrldata.needSetEmergency = TRUE;
			}
			else
			{
				printf("Take off ...\n");
				if(!(ctrlnavdata.ardrone_state & ARDRONE_USER_FEEDBACK_START))
					ardrone_tool_set_ui_pad_start(1);
				else
					ardrone_tool_set_ui_pad_start(0);
				ctrldata.needSetTakeOff = FALSE;
			}
		}
		
		if(ctrldata.needSetEmergency)
		{
			ctrldata.isInEmergency = (ctrlnavdata.ardrone_state & ARDRONE_EMERGENCY_MASK);
			ardrone_tool_set_ui_pad_select(1);
			ctrldata.needSetEmergency = FALSE;
		}
		
		if(ctrlnavdata.ardrone_state & ARDRONE_NAVDATA_BOOTSTRAP)
		{
			configuration_get();
		}
		
		if(ardrone_navdata_client_get_num_retries() >= NAVDATA_MAX_RETRIES)
		{
			strcpy(ctrldata.error_msg, "CONTROL LINK NOT AVAILABLE");
			ctrldata.navdata_connected = FALSE;
			resetControlData();
		}
		else
		{
			ctrldata.navdata_connected = TRUE;
			if(ctrlnavdata.ardrone_state & ARDRONE_EMERGENCY_MASK)
			{
				if(!ctrldata.isInEmergency && input_state->select)
					ardrone_tool_set_ui_pad_select(0);
				
				if(ctrlnavdata.ardrone_state & ARDRONE_CUTOUT_MASK)
				{
					strcpy(ctrldata.error_msg, "CUT OUT EMERGENCY");
				}
				else if(ctrlnavdata.ardrone_state & ARDRONE_MOTORS_MASK)
				{
					strcpy(ctrldata.error_msg, "MOTORS EMERGENCY");
				}
				else if(!(ctrlnavdata.ardrone_state & ARDRONE_VIDEO_THREAD_ON))
				{
					strcpy(ctrldata.error_msg, "CAMERA EMERGENCY");
				}
				else if(ctrlnavdata.ardrone_state & ARDRONE_ADC_WATCHDOG_MASK)
				{
					strcpy(ctrldata.error_msg, "PIC WATCHDOG EMERGENCY");
				}
				else if(!(ctrlnavdata.ardrone_state & ARDRONE_PIC_VERSION_MASK))
				{
					strcpy(ctrldata.error_msg, "PIC VERSION EMERGENCY");
				}
				else if(ctrlnavdata.ardrone_state & ARDRONE_ANGLES_OUT_OF_RANGE)
				{
					strcpy(ctrldata.error_msg, "TOO MUCH ANGLE EMERGENCY");
				}
				else if(ctrlnavdata.ardrone_state & ARDRONE_VBAT_LOW)
				{
					strcpy(ctrldata.error_msg, "BATTERY LOW EMERGENCY");
				}
				else if(ctrlnavdata.ardrone_state & ARDRONE_USER_EL)
				{
					strcpy(ctrldata.error_msg, "USER EMERGENCY");
				}
				else if(ctrlnavdata.ardrone_state & ARDRONE_ULTRASOUND_MASK)
				{
					strcpy(ctrldata.error_msg, "ULTRASOUND EMERGENCY");
				}
				else
				{
					strcpy(ctrldata.error_msg, "UNKNOWN EMERGENCY");
				}
							
				strcpy(ctrldata.emergency_msg, "");
				strcpy(ctrldata.takeoff_msg, "take_off");
				
				resetControlData();
				ardrone_tool_start_reset();
			}
			else // Not emergency landing
			{	
				if(ctrldata.isInEmergency && input_state->select)
				{
					ardrone_tool_set_ui_pad_select(0);
				}
				
				if(video_stage_get_num_retries() > VIDEO_MAX_RETRIES)
				{
					strcpy(ctrldata.error_msg, "VIDEO CONNECTION ALERT");
				}
				else if(ctrlnavdata.ardrone_state & ARDRONE_VBAT_LOW)
				{
					strcpy(ctrldata.error_msg, "BATTERY LOW ALERT");
				}
				else if(ctrlnavdata.ardrone_state & ARDRONE_ULTRASOUND_MASK)
				{
					strcpy(ctrldata.error_msg, "ULTRASOUND ALERT");
				}
				else if(!(ctrlnavdata.ardrone_state & ARDRONE_VISION_MASK))
				{
					ARDRONE_FLYING_STATE tmp_state = ardrone_navdata_get_flying_state(ctrlnavdata);	
					if(tmp_state == ARDRONE_FLYING_STATE_FLYING)
					{
						strcpy(ctrldata.error_msg, "VISION ALERT");
					}
				}

				if(!(ctrlnavdata.ardrone_state & ARDRONE_TIMER_ELAPSED))
					strcpy(ctrldata.emergency_msg, "emergency");
				
				if(input_state->start)
				{
					if(ctrlnavdata.ardrone_state & ARDRONE_USER_FEEDBACK_START)
					{
						strcpy(ctrldata.takeoff_msg, "take_land");
					}
					else
					{	
						strcpy(ctrldata.takeoff_msg, "take_off");
						strcpy(ctrldata.error_msg, "START NOT RECEIVED");
						
					}
				}
				else
				{
					strcpy(ctrldata.takeoff_msg, "take_off");
				}			
			}
		}
	}
}

