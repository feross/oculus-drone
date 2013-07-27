/********************************************************************
 *
 *  Parts of the DirectX code are from a tutorial by Microsoft
 *	which can be found in the Microsoft DirectX SDK June 2010.
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 *  
 *  The rest of the code is COPYRIGHT PARROT 2010
 *
 ********************************************************************
 *       PARROT - A.R.Drone SDK Windows Client Example
 *-----------------------------------------------------------------*/
/**
 * @file gamepad.h 
 * @brief Header file for the DirectInput gamepad management system.
 *
 * @author Stephane Piskorski <stephane.piskorski.ext@parrot.fr>
 * @date   Sept, 8. 2010
 *
 *******************************************************************/



#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include "UI/ui.h"

#define GAMEPAD_LOGICTECH_ID 0x046dc21a

typedef enum {
  PAD_X,
  PAD_Y
} PAD_AXIS;

typedef enum {
  PAD_AG = 0,
  PAD_AB,
  PAD_AD,
  PAD_AH,
  PAD_L1,
  PAD_R1,
  PAD_L2,
  PAD_R2,
  PAD_SELECT,
  PAD_START,
  PAD_NUM_BUTTONS
} PAD_BUTTONS;

extern input_device_t dx_gamepad;
extern input_device_t dx_keyboard;

/*
C_RESULT open_gamepad(void);
C_RESULT update_gamepad(void);
C_RESULT close_gamepad(void);
*/
#define RADIO_GP_ID 0x061c0010

typedef enum _TYPE
{
  TYPE_MIN      =  0,
  TYPE_BUTTON   =  1,
  TYPE_ANALOGIC =  2,
  TYPE_MAX      =  3
} TYPE;

typedef enum _ANALOGIC_RADIO_COMMAND_GP
{
  GP_ROLL  =  0,
  GP_GAZ   =  1,
  GP_PITCH =  2,
  GP_PID   =  3,
  GP_YAW   =  4,
  GP_ANALOG_MAX   =  5,
} ANALOGIC_RADIO_COMMAND_GP;

#define  NUM_A_GP_MIN GP_ROLL
#define  NUM_A_GP_MAX GP_MAX

#define OFFSET_PITCH_GP 127
#define OFFSET_ROLL_GP  127
#define OFFSET_YAW_GP   127
#define OFFSET_GAZ_GP   198

#define NUM_PITCH_GP     -3
#define NUM_ROLL_GP       3
#define NUM_YAW_GP        3
#define NUM_GAZ_GP       -7

#define DEC_PITCH_GP      1
#define DEC_ROLL_GP       1
#define DEC_YAW_GP        1
#define DEC_GAZ_GP        2


typedef enum _BUTTON_RADIO_COMMAND_GP
{
  GP_BOARD_LEFT      =   0, // switch above left joystick
  GP_SIDE_RIGHT      =   1, // right corner switch
  GP_IMPULSE         =   2, // Red button
  GP_SIDE_LEFT_DOWN  =   3, // left corner switch, down position
  GP_SIDE_LEFT_UP    =   4, // left corner switch, up position
  GP_BUTTON_MAX             =   5
} BUTTON_RADIO_COMMAND_GP;

#define  NUM_B_GP_MIN GP_BOARD_LEFT
#define  NUM_B_GP_MAX GP_MAX
extern input_device_t radioGP;


C_RESULT open_dx_gamepad(void);
C_RESULT update_dx_gamepad(void);
C_RESULT close_dx_gamepad(void);

C_RESULT open_dx_keyboard(void);
C_RESULT update_dx_keyboard(void);
C_RESULT close_dx_keyboard(void);


#endif // _GAMEPAD_H_
