#include "ConstantsAndMacros.h"
#import "ARDrone.h"
#import "SettingsMenu.h"

typedef void (*control_callback)(float percent);

typedef struct Joystick
{
	// Value between -1.0 and 1.0
	bool_t can_use_accelero;
	control_callback up_down;
	control_callback left_right;
} JOYSTICK;

typedef struct Controls
{
	JOYSTICK Left;
	JOYSTICK Right;
} CONTROLS;

@interface HUD : UIViewController <UIAccelerometerDelegate, SettingsMenuDelegate> {
    IBOutlet UILabel	 *messageBoxLabel;
    IBOutlet UILabel	 *batteryLevelLabel;
	
	IBOutlet UIImageView *batteryImageView;
	IBOutlet UIImageView *joystickRightThumbImageView;
	IBOutlet UIImageView *joystickRightBackgroundImageView;
	IBOutlet UIImageView *joystickLeftThumbImageView;
	IBOutlet UIImageView *joystickLeftBackgroundImageView;
	
	IBOutlet UIButton	 *backToMainMenuButton;
	IBOutlet UIButton    *settingsButton;
	IBOutlet UIButton    *switchScreenButton;
    IBOutlet UIButton    *takeOffButton;
	IBOutlet UIButton	 *emergencyButton;
	
	IBOutlet UIButton	 *joystickRightButton;
	IBOutlet UIButton	 *joystickLeftButton;
	
	BOOL firePressed;
	BOOL settingsPressed;
	BOOL mainMenuPressed;
	
	CONTROLS controls;
}

@property (nonatomic, assign) BOOL firePressed;
@property (nonatomic, assign) BOOL settingsPressed;
@property (nonatomic, assign) BOOL mainMenuPressed;

- (id)initWithFrame:(CGRect)frame withState:(BOOL)inGame withHUDConfiguration:(ARDroneHUDConfiguration)hudconfiguration withControlData:(ControlData*)data;
- (void)setMessageBox:(NSString*)str;
- (void)setTakeOff:(NSString*)str;
- (void)setEmergency:(NSString*)str;
- (void)setBattery:(int)percent;
- (void)changeState:(BOOL)inGame;
- (void)showBackToMainMenu:(BOOL)show;

- (IBAction)buttonPress:(id)sender forEvent:(UIEvent *)event;
- (IBAction)buttonRelease:(id)sender forEvent:(UIEvent *)event;
- (IBAction)buttonClick:(id)sender forEvent:(UIEvent *)event;
- (IBAction)buttonDrag:(id)sender forEvent:(UIEvent *)event;
@end
