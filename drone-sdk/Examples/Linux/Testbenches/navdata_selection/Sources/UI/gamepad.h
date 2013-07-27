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




extern input_device_t gamepad;

C_RESULT open_gamepad(void);
C_RESULT update_gamepad(void);
C_RESULT close_gamepad(void);



#define GAMEPAD_PLAYSTATION3_ID 0x054C0268

typedef enum
{
    PS3BTN_SELECT=0,
    PS3BTN_L3,
    PS3BTN_R3,
    PS3BTN_START,
    PS3BTN_UPARROW,
    PS3BTN_RIGHTARROW,
    PS3BTN_DOWNARROW,
    PS3BTN_LEFTARROW,
    PS3BTN_L2,
    PS3BTN_R2,
    PS3BTN_L1,
    PS3BTN_R1,
    PS3BTN_TRIANGLE,
    PS3BTN_CIRCLE,
    PS3BTN_CROSS,
    PS3BTN_SQUARE,
    PS3BTN_PS3
}PS3PAD_BUTTONS;

/* On Playstation3 Gamepad, buttons can return analogic values (though non-precise) */
typedef enum
{
    PS3AXIS_STICK1_LR=0,
    PS3AXIS_STICK1_UD,
    PS3AXIS_STICK2_LR,
    PS3AXIS_STICK2_UD,
    PS3AXIS_UPARROW=8,
    PS3AXIS_RIGHTARROW,
    PS3AXIS_DOWNARROW,
    PS3AXIS_LEFTARROW,
    PS3AXIS_L2,
    PS3AXIS_R2,
    PS3AXIS_L1,
    PS3AXIS_R1,
    PS3AXIS_TRIANGLE,
    PS3AXIS_CIRCLE,
    PS3AXIS_CROSS,
    PS3AXIS_SQUARE
}PS3PAD_AXIS;


extern input_device_t ps3pad;


C_RESULT open_ps3pad(void);
C_RESULT update_ps3pad(void);
C_RESULT close_ps3pad(void);






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

C_RESULT open_radioGP(void);
C_RESULT update_radioGP(void);
C_RESULT close_radioGP(void);

#endif // _GAMEPAD_H_
