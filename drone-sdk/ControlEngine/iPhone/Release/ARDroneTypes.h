/*
 *  ARDroneTypes.h
 *  ARDroneEngine
 *
 *  Created by Frédéric D'HAEYER on 21/05/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#ifndef _ARDRONE_TYPES_H_
#define _ARDRONE_TYPES_H_
#include "ARDroneGeneratedTypes.h"

/**
 * Define the command identifiers from drone to Game Engine
 */
typedef enum {
	ARDRONE_COMMAND_RUN, 
	ARDRONE_COMMAND_PAUSE, 
	ARDRONE_COMMAND_FIRE,
} ARDRONE_COMMAND_OUT;

/**
 * Define the command identifiers from Game Engine to drone
 */
typedef enum {
	ARDRONE_COMMAND_ISCLIENT,			// Command to set if the multiplayer is client
	ARDRONE_COMMAND_DRONE_ANIM,			// Command to set a drone animation
	ARDRONE_COMMAND_DRONE_LED_ANIM,     // Command to set a drone led animation
	ARDRONE_COMMAND_SET_CONFIG,			// Command to set a drone configuration key
	ARDRONE_COMMAND_ENABLE_COMBINED_YAW,// Command to enable / disable combined yaw command. 
	ARDRONE_COMMAND_VIDEO_CHANNEL,		// Command to set the channel of video								-- DEPRECATED
	ARDRONE_COMMAND_CAMERA_DETECTION,	// Command to set camera type for detection.						-- DEPRECATED
	ARDRONE_COMMAND_ENEMY_SET_PARAM,	// Command to set enemy parameter for detection (color and hull).	-- DEPRECATED
	ARDRONE_COMMAND_SET_FLY_MODE,		// Command to change flying mode									-- DEPRECATED
} ARDRONE_COMMAND_IN;

typedef void (*command_in_configuration_callback)(int result);

typedef struct
{
	ARDRONE_COMMAND_IN command;
	command_in_configuration_callback callback;
	void *parameter;
} ARDRONE_COMMAND_IN_WITH_PARAM;

typedef enum {
	ARDRONE_FLYING_STATE_LANDED = 0,
	ARDRONE_FLYING_STATE_FLYING,
	ARDRONE_FLYING_STATE_TAKING_OFF,
	ARDRONE_FLYING_STATE_LANDING,
} ARDRONE_FLYING_STATE;

typedef struct
{
	ARDRONE_ENEMY_COLOR color;
	int outdoor_shell;			// 1 if enemy has outdoor shell, else 0 
} ARDRONE_ENEMY_PARAM;

typedef struct
{
	ARDRONE_LED_ANIMATION led_anim;
	float frequency;
	unsigned int duration;
} ARDRONE_LED_ANIMATION_PARAM;

typedef struct
{
	ARDRONE_ANIMATION drone_anim;
	int timeout;
} ARDRONE_ANIMATION_PARAM;

/* Comments are used for autogeneration
 * Do not modify these !
 */
// MATCH_TYPES : int : int32_t bool_t
// MATCH_TYPES : unsigned int : uint32_t
// MATCH_TYPES : float : float32_t
// MATCH_TYPES : double : float64_t
/* End of autogeneration comments */

typedef struct
{
	ARDRONE_CONFIG_KEYS config_key;
	void *pvalue;
} ARDRONE_CONFIG_PARAM;

/**
 * Define what a 3D vector is
 */
typedef struct
{
	float x;
	float y;
	float z;
}
ARDroneVector3D;

/**
 * Define a structure to collect drone's navigation data
 */
typedef struct
{
	/**
	 * Translation speed of the drone, in meters per second
	 */
	ARDroneVector3D linearVelocity;
	
	/**
	 * Rotation speed of the drone, in degré
	 */
	ARDroneVector3D angularPosition;
	
	/**
	 * Navdata video num frames to synchronized Navdata with video
	 */
	int navVideoNumFrames;
	
	/**
	 * Video num frames to synchronized Navdata with video
	 */
	int videoNumFrames;
	
	/**
	 * Value indicates drone flying state (see ARDRONE_FLYING_STATE enum) 
	 */
	ARDRONE_FLYING_STATE flyingState;
	
	/**
	 * int indicates drone is in emergency state  (1 if is in emergency, 0 else)
	 */
	int emergencyState;
	
	/**
	 * Camera detection type
	 */
	ARDRONE_CAMERA_DETECTION_TYPE detection_type;
	
	/**
	 * Number of finish lines detected
	 */
	unsigned int finishLineCount;
	
	/**
	 * Number of double taps detected
	 */
	unsigned int doubleTapCount;
    
    /**
     * Tells the application that the ardrone engine is done with initial configuration so the application can send their own configs
     * (1 if application can send config, 0 otherwise)
     */
    int isInit;
}
ARDroneNavigationData;

/**
 * Define a structure to exchange an enemy data
 */
typedef struct
{
	/**
	 * Position of the enemy (between -1.0 and 1.0)
	 */
	ARDroneVector3D position;
	
	/**
	 * Size of the enemy (between 0.0 and 2.0)
	 */
	float height, width;
	
	/**
	 * Angle of the enemy (between -90.0° and 90.0°)
	 */
	float orientation_angle;
}
ARDroneEnemyData;

/**
 * Define a structure to exchange camera parameters compute by detection
 */
typedef struct
{
	/**
	 * Rotation matrix of camera
	 */
	float rotation[3][3];
	
	/**
	 * Translation matrix of camera
	 */
	float translation[3];
	
	/**
	 * Index of tag detected
	 */
	int tag_index;
} 
ARDroneDetectionCamera;

/**
 * Define a structure to exchange camera parameters compute by drone
 */
typedef struct
{
	/**
	 * Rotation matrix of camera
	 */
	float rotation[3][3];
	
	/**
	 * Translation matrix of camera
	 */
	float translation[3];
} 
ARDroneCamera;

/**
 * Define a structure to exchange all enemies data
 */
typedef struct
{
	/**
	 * Number of enemies
	 */
	unsigned int count;
	
	/**
	 * Pointer to an array that contains the data structure of each enemy
	 */
	ARDroneEnemyData data[ARDRONE_MAX_ENEMIES];
}
ARDroneEnemiesData;

#endif // _ARDRONE_TYPES_H_
