/*
 *  ControlData.h
 *  ARDroneEngine
 *
 *  Created by Frederic D'HAEYER on 14/01/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#ifndef _CONTROLDATA_H_
#define _CONTROLDATA_H_
#include "ConstantsAndMacros.h"

#define SMALL_STRING_SIZE	16
#define MEDIUM_STRING_SIZE	64

typedef enum _EMERGENCY_STATE_
{
	EMERGENCY_STATE_EMERGENCY,
	EMERGENCY_STATE_RESET
} EMERGENCY_STATE;

typedef enum _CONFIG_STATE_
{
	CONFIG_STATE_IDLE,
	CONFIG_STATE_NEEDED,
	CONFIG_STATE_IN_PROGRESS,
} CONFIG_STATE;	

typedef struct
{
	/**
	 * Current navdata_connected
	 */
	bool_t navdata_connected;
	
	/**
	 * Progressive commands
	 * And accelerometers values transmitted to drone, FALSE otherwise
	 */
	float yaw, gaz, accelero_phi, accelero_theta;
	int32_t accelero_flag;
	
	/**
	 * variable to know if setting is needed
	 */
	EMERGENCY_STATE	isInEmergency;
	
	bool_t wifiReachabled;
	
	int framecounter;
	bool_t needSetEmergency;
	bool_t needSetTakeOff;
	
	int needVideoSwitch;

	bool_t needAnimation;
	char needAnimationParam[SMALL_STRING_SIZE];
	
	bool_t needLedAnimation;
	char needLedAnimationParam[SMALL_STRING_SIZE];
	
	CONFIG_STATE applicationDefaultConfigState;
	CONFIG_STATE configurationState;
	
	/**
	 * Strings to display in interface
	 */	
	char error_msg[MEDIUM_STRING_SIZE];
	char takeoff_msg[SMALL_STRING_SIZE];
	char emergency_msg[SMALL_STRING_SIZE];
} ControlData;

void initControlData(void);
void resetControlData(void);
void initNavdataControlData(void);
void checkErrors(void);
void controlfps(void);
void sendControls(void);
void configuration_get(void);
void setApplicationDefaultConfig(void);
void switchTakeOff(void);
void emergency(void);
void handleAccelerometers(void);
void disableAccelerometers(void);
void inputYaw(float percent);
void inputGaz(float percent);
void inputPitch(float percent);
void inputRoll(float percent);
void signal_input(int new_input);
void send_inputs(void);
#endif // _CONTROLDATA_H_
