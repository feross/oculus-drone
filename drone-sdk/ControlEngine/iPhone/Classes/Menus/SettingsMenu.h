#include "ConstantsAndMacros.h" 
#import "ARDrone.h"

typedef enum CONTROL_MODE
{
	CONTROL_MODE3,
	CONTROL_MODE2,
	CONTROL_MODE1,
	CONTROL_MODE4,
	CONTROL_MODE_MAX,
} CONTROL_MODE;

typedef struct 
{
	BOOL  logsActivated;
} SettingsParams;

@protocol SettingsMenuDelegate<NSObject>

- (void)acceleroValueChanged:(bool_t)enabled;
- (void)combinedYawValueChanged:(bool_t)enabled;
- (void)controlModeChanged:(CONTROL_MODE)mode;
- (void)interfaceAlphaValueChanged:(CGFloat)value;

@end

@interface SettingsMenu : UIViewController <UIScrollViewDelegate> {
	// Settings field
	IBOutlet UISwitch *pairingSwitch;
	IBOutlet UISwitch *altituteLimitedSwitch;
    IBOutlet UISwitch *outdoorFlightSwitch;
    IBOutlet UISwitch *outdoorShellSwitch;

	IBOutlet UISwitch *acceleroDisabledSwitch;
    IBOutlet UISwitch *leftHandedSwitch;
	IBOutlet UISwitch *adaptiveVideoSwitch;
    IBOutlet UISegmentedControl *videoCodecSwitch;
  
	IBOutlet UIButton *clearButton0, *clearButton1;
	IBOutlet UIButton *flatTrimButton0, *flatTrimButton1;
	IBOutlet UIButton *okButton0, *okButton1;
	
    IBOutlet UILabel  *droneTiltMinLabel;
    IBOutlet UILabel  *droneTiltMaxLabel;
    IBOutlet UILabel  *droneTiltCurrentLabel;
    IBOutlet UISlider *droneTiltSlider;
	
	IBOutlet UILabel  *iPhoneTiltMinLabel;
    IBOutlet UILabel  *iPhoneTiltMaxLabel;
    IBOutlet UILabel  *iPhoneTiltCurrentLabel;
    IBOutlet UISlider *iPhoneTiltSlider;

	IBOutlet UILabel  *verticalSpeedMinLabel;
    IBOutlet UILabel  *verticalSpeedMaxLabel;
    IBOutlet UILabel  *verticalSpeedCurrentLabel;
	IBOutlet UISlider *verticalSpeedSlider;
	
	IBOutlet UILabel  *yawSpeedMinLabel;
    IBOutlet UILabel  *yawSpeedMaxLabel;
    IBOutlet UILabel  *yawSpeedCurrentLabel;
    IBOutlet UISlider *yawSpeedSlider;
	
	IBOutlet UITextField *droneSSIDTextField;
	
	IBOutlet UILabel  *droneTrimPitchMinLabel;
    IBOutlet UILabel  *droneTrimPitchMaxLabel;
    IBOutlet UILabel  *droneTrimPitchCurrentLabel;
    IBOutlet UISlider *droneTrimPitchSlider;
	
	IBOutlet UILabel  *interfaceAlphaMinLabel;
	IBOutlet UILabel  *interfaceAlphaMaxLabel;
	IBOutlet UILabel  *interfaceAlphaCurrentLabel;
	IBOutlet UISlider *interfaceAlphaSlider;

	IBOutlet UILabel  *softwareVersion;
	IBOutlet UILabel  *droneHardVersion, *droneSoftVersion;
	IBOutlet UILabel  *dronePicHardVersion, *dronePicSoftVersion;
	IBOutlet UILabel  *droneMotor1HardVersion, *droneMotor1SoftVersion, *droneMotor1SupplierVersion;
	IBOutlet UILabel  *droneMotor2HardVersion, *droneMotor2SoftVersion, *droneMotor2SupplierVersion;
	IBOutlet UILabel  *droneMotor3HardVersion, *droneMotor3SoftVersion, *droneMotor3SupplierVersion;
	IBOutlet UILabel  *droneMotor4HardVersion, *droneMotor4SoftVersion, *droneMotor4SupplierVersion;
	
	IBOutlet UILabel  *deviceNameText;
	
	IBOutlet UILabel  *softwareVersionText;
	IBOutlet UILabel  *acceleroDisabledText;
	IBOutlet UILabel  *leftHandedText;
	IBOutlet UILabel  *iPhoneTiltText;
	IBOutlet UILabel  *interfaceAlphaText;
	IBOutlet UILabel  *droneVersionText, *droneHardVersionText, *droneSoftVersionText;
	IBOutlet UILabel  *dronePicVersionText, *dronePicHardVersionText, *dronePicSoftVersionText;
	IBOutlet UILabel  *droneMotorVersionsText;
	IBOutlet UILabel  *droneMotor1HardVersionText, *droneMotor1SoftVersionText, *droneMotor1SupplierVersionText;
	IBOutlet UILabel  *droneMotor2HardVersionText, *droneMotor2SoftVersionText, *droneMotor2SupplierVersionText;
	IBOutlet UILabel  *droneMotor3HardVersionText, *droneMotor3SoftVersionText, *droneMotor3SupplierVersionText;
	IBOutlet UILabel  *droneMotor4HardVersionText, *droneMotor4SoftVersionText, *droneMotor4SupplierVersionText;
	IBOutlet UILabel  *pairingText;
	IBOutlet UILabel  *droneSSIDText;
	IBOutlet UILabel  *altituteLimitedText;
	IBOutlet UILabel  *adaptiveVideoText;
    IBOutlet UILabel  *videoCodecText;
	IBOutlet UILabel  *outdoorShellText;
	IBOutlet UILabel  *outdoorFlightText;
	IBOutlet UILabel  *yawSpeedText;
	IBOutlet UILabel  *verticalSpeedText;
	IBOutlet UILabel  *droneTiltText;
	
#ifdef INTERFACE_WITH_DEBUG
	IBOutlet UIButton *clearLogsButton;
	IBOutlet UISwitch *logsSwitch;
    IBOutlet UILabel  *wifiModeLabel;
    IBOutlet UISegmentedControl *wifiModeControl;
#endif
}

- (id)initWithFrame:(CGRect)frame AndHUDConfiguration:(ARDroneHUDConfiguration)configuration withDelegate:(id <SettingsMenuDelegate>)delegate withControlData:(ControlData*)data;
- (void)switchDisplay;
- (void)configChanged;
- (void)saveIPhoneSettings;
- (void)loadIPhoneSettings;	

- (IBAction)valueChanged:(id)sender;
- (IBAction)clearButtonClick:(id)sender;
- (IBAction)flatTrimButtonClick:(id)sender;
- (IBAction)okButtonClick:(id)sender;
- (IBAction)sliderRelease:(id)sender;

#ifdef INTERFACE_WITH_DEBUG
- (IBAction)logsChanged:(id)sender;
- (IBAction)clearLogsButtonClick:(id)sender;
#endif
@end
