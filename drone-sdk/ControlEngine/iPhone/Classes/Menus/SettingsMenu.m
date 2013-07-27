#import "SettingsMenu.h"

#define REFRESH_TRIM_TIMEOUT	1
#define ALTITUDE_LIMITED		3000
#define NO_ALTITUDE				10000

extern char iphone_mac_address[];


@interface SettingsMenu ()
id <SettingsMenuDelegate> _delegate;
ControlData *controlData;
UIScrollView *scrollView;
BOOL ssidChangeInProgress;

BOOL compiledForIPad;
BOOL usingIPad;
BOOL using2xInterface;

- (void)refresh;
- (BOOL)textFieldShouldReturn:(UITextField *)theTextField;
- (void)saveForDebugSettings;
@end

@implementation SettingsMenu
- (id)initWithFrame:(CGRect)frame AndHUDConfiguration:(ARDroneHUDConfiguration)configuration withDelegate:(id <SettingsMenuDelegate>)delegate withControlData:(ControlData*)data
{
	NSLog(@"SettingsMenu frame => w : %f, h : %f", frame.size.width, frame.size.height);
	
	NSArray *targetArray = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"UIDeviceFamily"];
	compiledForIPad = NO;
	for (int i = 0; i < [targetArray count]; i++)
	{
		NSNumber* num = (NSNumber*)[targetArray objectAtIndex:i];
		int value;
		[num getValue:&value];
		if (2 == value)
		{
			compiledForIPad = YES;
			break;
		}
	}
	
	usingIPad = NO;
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
	{
		usingIPad = YES;
	}
	
	using2xInterface = NO;
	if ([UIScreen instancesRespondToSelector:@selector(scale)]) {
		CGFloat scale = [[UIScreen mainScreen] scale];
		if (scale > 1.0) {
			using2xInterface = YES;
		}
	}	

#ifdef INTERFACE_WITH_DEBUG
	if(self = [super initWithNibName:@"SettingsMenu-Debug" bundle:nil])
#else
	if (usingIPad && compiledForIPad)
	{
		self = [super initWithNibName:@"SettingsMenu-iPad" bundle:nil];
	}
	else
	{
		self = [super initWithNibName:@"SettingsMenu-iPhone" bundle:nil];
	}
	if(self)
#endif
	{
		controlData = data;
		ssidChangeInProgress = NO;
		_delegate = delegate;
		// Set parameters of scrollview
		scrollView = (UIScrollView*)self.view;
		scrollView.alwaysBounceHorizontal = NO;
		scrollView.alwaysBounceVertical = YES;
		scrollView.clipsToBounds = YES;
		scrollView.contentSize = CGSizeMake(scrollView.frame.size.width, scrollView.frame.size.height);
		scrollView.frame = CGRectMake(frame.origin.x, frame.origin.y, frame.size.height, frame.size.width);		
		scrollView.delegate = self;
		scrollView.scrollEnabled = YES;
		scrollView.indicatorStyle = UIScrollViewIndicatorStyleWhite;
		
		yawSpeedMinLabel.text = [NSString stringWithFormat:@"%d", (int)yawSpeedSlider.minimumValue];
		yawSpeedMaxLabel.text = [NSString stringWithFormat:@"%d", (int)yawSpeedSlider.maximumValue];		
		verticalSpeedMinLabel.text = [NSString stringWithFormat:@"%d", (int)verticalSpeedSlider.minimumValue];
		verticalSpeedMaxLabel.text = [NSString stringWithFormat:@"%d", (int)verticalSpeedSlider.maximumValue];
		droneTiltMinLabel.text = [NSString stringWithFormat:@"%d", (int)droneTiltSlider.minimumValue];
		droneTiltMaxLabel.text = [NSString stringWithFormat:@"%d", (int)droneTiltSlider.maximumValue];		
		iPhoneTiltMinLabel.text = [NSString stringWithFormat:@"%d", (int)iPhoneTiltSlider.minimumValue];
		iPhoneTiltMaxLabel.text = [NSString stringWithFormat:@"%d", (int)iPhoneTiltSlider.maximumValue];		
		droneTrimPitchMinLabel.text = [NSString stringWithFormat:@"%d", (int)droneTrimPitchSlider.minimumValue];
		droneTrimPitchMaxLabel.text = [NSString stringWithFormat:@"%d", (int)droneTrimPitchSlider.maximumValue];
		
		interfaceAlphaMinLabel.text = [NSString stringWithFormat:@"%d", (int)interfaceAlphaSlider.minimumValue];
		interfaceAlphaMaxLabel.text = [NSString stringWithFormat:@"%d", (int)interfaceAlphaSlider.maximumValue];
		
		softwareVersion.text	= [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
		
		deviceNameText.text = [[UIDevice currentDevice] model];
		
		[clearButton0 setTitle:ARDroneEngineLocalizeString(@"Default settings") forState:UIControlStateNormal];
		[clearButton1 setTitle:ARDroneEngineLocalizeString(@"Default settings") forState:UIControlStateNormal];
		[flatTrimButton0 setTitle:ARDroneEngineLocalizeString(@"Flat trim") forState:UIControlStateNormal];
		[flatTrimButton1 setTitle:ARDroneEngineLocalizeString(@"Flat trim") forState:UIControlStateNormal];
		[okButton0 setTitle:ARDroneEngineLocalizeString(@"OK") forState:UIControlStateNormal];
		[okButton1 setTitle:ARDroneEngineLocalizeString(@"OK") forState:UIControlStateNormal];
		
		softwareVersionText.text = ARDroneEngineLocalizeString(@"Software version");
		acceleroDisabledText.text = ARDroneEngineLocalizeString(@"Accelero disabled");
		leftHandedText.text = ARDroneEngineLocalizeString(@"Left-handed");
		iPhoneTiltText.text = [NSString stringWithFormat:ARDroneEngineLocalizeString(@"%@ tilt max"), [[UIDevice currentDevice] model]];
		interfaceAlphaText.text = ARDroneEngineLocalizeString(@"Interface opacity");
		droneVersionText.text = ARDroneEngineLocalizeString(@"AR.Drone");
		droneHardVersionText.text = ARDroneEngineLocalizeString(@"hardware");
		droneSoftVersionText.text = ARDroneEngineLocalizeString(@"software");
		dronePicVersionText.text = ARDroneEngineLocalizeString(@"Inertial");
		dronePicHardVersionText.text = ARDroneEngineLocalizeString(@"hardware");
		dronePicSoftVersionText.text = ARDroneEngineLocalizeString(@"software");
		droneMotorVersionsText.text = ARDroneEngineLocalizeString(@"Motors versions");
		droneMotor1HardVersionText.text = ARDroneEngineLocalizeString(@"hardware");
		droneMotor1SoftVersionText.text = ARDroneEngineLocalizeString(@"software");
		droneMotor1SupplierVersionText.text = ARDroneEngineLocalizeString(@"Motor 1 type");
		droneMotor2HardVersionText.text = ARDroneEngineLocalizeString(@"hardware");
		droneMotor2SoftVersionText.text = ARDroneEngineLocalizeString(@"software");
		droneMotor2SupplierVersionText.text = ARDroneEngineLocalizeString(@"Motor 2 type");
		droneMotor3HardVersionText.text = ARDroneEngineLocalizeString(@"hardware");
		droneMotor3SoftVersionText.text = ARDroneEngineLocalizeString(@"software");
		droneMotor3SupplierVersionText.text = ARDroneEngineLocalizeString(@"Motor 3 type");
		droneMotor4HardVersionText.text = ARDroneEngineLocalizeString(@"hardware");
		droneMotor4SoftVersionText.text = ARDroneEngineLocalizeString(@"software");
		droneMotor4SupplierVersionText.text = ARDroneEngineLocalizeString(@"Motor 4 type");
		pairingText.text = ARDroneEngineLocalizeString(@"Pairing");
		droneSSIDText.text = ARDroneEngineLocalizeString(@"Network name");
		altituteLimitedText.text = ARDroneEngineLocalizeString(@"Altitude limited");
		adaptiveVideoText.text = ARDroneEngineLocalizeString(@"Adaptive video");
        videoCodecText.text = ARDroneEngineLocalizeString(@"Video codec");
		outdoorShellText.text = ARDroneEngineLocalizeString(@"Outdoor hull");
		outdoorFlightText.text = ARDroneEngineLocalizeString(@"Outdoor flight");
		yawSpeedText.text = ARDroneEngineLocalizeString(@"Yaw speed max");
		verticalSpeedText.text = ARDroneEngineLocalizeString(@"Vertical speed max");
		droneTiltText.text = ARDroneEngineLocalizeString(@"Tilt");
		
#ifdef INTERFACE_WITH_DEBUG
		logsSwitch.on = NO;
#endif	
		[self loadIPhoneSettings];
		[self refresh];
	}
	
	return self;
}

- (void)checkDisplay
{
	BOOL enabled = (controlData->navdata_connected) ? YES : NO; 
	float alpha = (controlData->navdata_connected) ? 1.0 : 0.5;
	
#ifdef INTERFACE_WITH_DEBUG
	logsSwitch.enabled = enabled;
	logsSwitch.alpha = alpha;
    wifiModeControl.enabled = enabled;
    wifiModeControl.alpha = alpha;
#endif	
    
	adaptiveVideoSwitch.enabled = acceleroDisabledSwitch.enabled = leftHandedSwitch.enabled = videoCodecSwitch.enabled = enabled;
	adaptiveVideoSwitch.alpha = acceleroDisabledSwitch.alpha = leftHandedSwitch.alpha = videoCodecSwitch.alpha = alpha;
	pairingSwitch.enabled = enabled;
	pairingSwitch.alpha = alpha;
	altituteLimitedSwitch.enabled = outdoorFlightSwitch.enabled = outdoorShellSwitch.enabled = enabled;
	altituteLimitedSwitch.alpha = outdoorFlightSwitch.alpha = outdoorShellSwitch.alpha = alpha;
	droneSSIDTextField.enabled = enabled;
	droneSSIDTextField.alpha = alpha;
	droneTiltSlider.enabled = iPhoneTiltSlider.enabled = verticalSpeedSlider.enabled = yawSpeedSlider.enabled = enabled;
	droneTiltSlider.alpha = iPhoneTiltSlider.alpha = verticalSpeedSlider.alpha = yawSpeedSlider.alpha = alpha;
	clearButton0.enabled = clearButton1.enabled = enabled;
	clearButton0.alpha = clearButton1.alpha = alpha;
	
	flatTrimButton0.enabled = flatTrimButton1.enabled = enabled;
	flatTrimButton0.alpha = flatTrimButton1.alpha = alpha;
}

- (void)configChanged
{
	[self checkDisplay];
	
	altituteLimitedSwitch.on = (ardrone_control_config.altitude_max == ALTITUDE_LIMITED) ? YES : NO;	
	pairingSwitch.on = strcmp(ardrone_control_config.owner_mac, NULL_MAC) != 0 ? YES : NO;
	outdoorFlightSwitch.on = ardrone_control_config.outdoor ? YES : NO;
	outdoorShellSwitch.on = ardrone_control_config.flight_without_shell ? YES : NO;
	droneTiltSlider.value = ardrone_control_config.euler_angle_max * RAD_TO_DEG;
	iPhoneTiltSlider.value = ardrone_control_config.control_iphone_tilt * RAD_TO_DEG;
	verticalSpeedSlider.value = ardrone_control_config.control_vz_max;
	yawSpeedSlider.value = ardrone_control_config.control_yaw * RAD_TO_DEG;
	adaptiveVideoSwitch.on = (ARDRONE_VARIABLE_BITRATE_MODE_DYNAMIC == ardrone_control_config.bitrate_ctrl_mode) ? YES : NO;
    videoCodecSwitch.selectedSegmentIndex = (ARDRONE_VIDEO_CODEC_UVLC == ardrone_control_config.video_codec) ? 0 : 1;
    
#ifdef INTERFACE_WITH_DEBUG
    wifiModeControl.selectedSegmentIndex = ardrone_control_config.wifi_mode;
#endif
	
	// Update SSID AR.Drone network
	if(!ssidChangeInProgress && (strcmp(ardrone_control_config.ssid_single_player, "") != 0))
		droneSSIDTextField.text = [NSString stringWithCString:ardrone_control_config.ssid_single_player encoding:NSUTF8StringEncoding];
	
	// Update pic version number
	if(ardrone_control_config.pic_version != 0)
	{
		dronePicHardVersion.text = [NSString stringWithFormat:@"%.1f", (float)(ardrone_control_config.pic_version >> 24)];
		dronePicSoftVersion.text = [NSString stringWithFormat:@"%d.%d", (int)((ardrone_control_config.pic_version & 0xFFFFFF) >> 16),(int)(ardrone_control_config.pic_version & 0xFFFF)];
	}
	else
	{
		dronePicHardVersion.text = @"none";
		dronePicSoftVersion.text = @"none";
	}
	
	// update AR.Drone software version 
	if(strcmp(ardrone_control_config.num_version_soft, "") != 0)
		droneSoftVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.num_version_soft];
	else
		droneSoftVersion.text = @"none";
	
	// update AR.Drone hardware version 
	if(ardrone_control_config.num_version_mb != 0)
		droneHardVersion.text = [NSString stringWithFormat:@"%x.0", ardrone_control_config.num_version_mb];
	else
		droneHardVersion.text = @"none";

	// Update motor 1 version (soft / hard / supplier)
	if(strcmp(ardrone_control_config.motor1_soft, "") != 0)
		droneMotor1SoftVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor1_soft];
	else
		droneMotor1SoftVersion.text = [NSString stringWithString:@"none"];

	if(strcmp(ardrone_control_config.motor1_hard, "") != 0)
		droneMotor1HardVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor1_hard];
	else
		droneMotor1HardVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor1_supplier, "") != 0)
		droneMotor1SupplierVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor1_supplier];
	else
		droneMotor1SupplierVersion.text = [NSString stringWithString:@"none"];
	
	// Update motor 2 version (soft / hard / supplier)
	if(strcmp(ardrone_control_config.motor2_soft, "") != 0)
		droneMotor2SoftVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor2_soft];
	else
		droneMotor2SoftVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor2_hard, "") != 0)
		droneMotor2HardVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor2_hard];
	else
		droneMotor2HardVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor2_supplier, "") != 0)
		droneMotor2SupplierVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor2_supplier];
	else
		droneMotor2SupplierVersion.text = [NSString stringWithString:@"none"];
	
	// Update motor 3 version (soft / hard / supplier)
	if(strcmp(ardrone_control_config.motor3_soft, "") != 0)
		droneMotor3SoftVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor3_soft];
	else
		droneMotor3SoftVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor3_hard, "") != 0)
		droneMotor3HardVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor3_hard];
	else
		droneMotor3HardVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor3_supplier, "") != 0)
		droneMotor3SupplierVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor3_supplier];
	else
		droneMotor3SupplierVersion.text = [NSString stringWithString:@"none"];
	
	// Update motor 4 version (soft / hard / supplier)
	if(strcmp(ardrone_control_config.motor4_soft, "") != 0)
		droneMotor4SoftVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor4_soft];
	else
		droneMotor4SoftVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor4_hard, "") != 0)
		droneMotor4HardVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor4_hard];
	else
		droneMotor4HardVersion.text = [NSString stringWithString:@"none"];
	
	if(strcmp(ardrone_control_config.motor4_supplier, "") != 0)
		droneMotor4SupplierVersion.text = [NSString stringWithFormat:@"%s", ardrone_control_config.motor4_supplier];
	else
		droneMotor4SupplierVersion.text = [NSString stringWithString:@"none"];

	if(controlData->navdata_connected)
	{
		outdoorFlightSwitch.enabled = YES;
		outdoorFlightSwitch.alpha = 1.0;
	}
	
	[self refresh];
}

- (void)refresh
{
#ifdef INTERFACE_WITH_DEBUG
	if(logsSwitch.on == YES)
	{
		clearLogsButton.enabled = NO;
		clearLogsButton.alpha = 0.50;
	}
	else 
	{
		clearLogsButton.enabled = YES;
		clearLogsButton.alpha = 1.0;
	}
#endif
	
	yawSpeedCurrentLabel.text = [NSString stringWithFormat:@"%0.2f %@", yawSpeedSlider.value, ARDroneEngineLocalizeString(@"°/s")];
	verticalSpeedCurrentLabel.text = [NSString stringWithFormat:@"%0.1f %@", verticalSpeedSlider.value, ARDroneEngineLocalizeString(@"mm/s")];
	droneTiltCurrentLabel.text = [NSString stringWithFormat:@"%0.2f %@", droneTiltSlider.value, ARDroneEngineLocalizeString(@"°")];
	iPhoneTiltCurrentLabel.text = [NSString stringWithFormat:@"%0.2f %@", iPhoneTiltSlider.value, ARDroneEngineLocalizeString(@"°")];
	droneTrimPitchCurrentLabel.text = [NSString stringWithFormat:@"%0.2f", ((droneTrimPitchSlider.value < -0.01) || (droneTrimPitchSlider.value > 0.01)) ? droneTrimPitchSlider.value : 0.0];
	interfaceAlphaCurrentLabel.text = [NSString stringWithFormat:@"%d %%", (int)interfaceAlphaSlider.value];
}

- (IBAction)valueChanged:(id)sender
{
	if(sender == acceleroDisabledSwitch)
	{
		[_delegate acceleroValueChanged:(acceleroDisabledSwitch.on == NO)];
		[self saveIPhoneSettings];
	}
	else if(sender == leftHandedSwitch)
	{
		[_delegate controlModeChanged:(leftHandedSwitch.on == YES) ? CONTROL_MODE2 : CONTROL_MODE3];
		[self saveIPhoneSettings];
	}
	else if(sender == pairingSwitch)
	{
		strcpy(ardrone_control_config.owner_mac, (pairingSwitch.on == YES) ? iphone_mac_address : NULL_MAC);
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(owner_mac, ardrone_control_config.owner_mac, NULL);
	}
	else if(sender == altituteLimitedSwitch)
	{
		uint32_t value = (altituteLimitedSwitch.on == YES) ? ALTITUDE_LIMITED : NO_ALTITUDE;
		ardrone_control_config.altitude_max = value;
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(altitude_max, &ardrone_control_config.altitude_max, NULL);
	}
	else if(sender == outdoorFlightSwitch)
	{
		bool_t enabled = (outdoorFlightSwitch.on == YES);
		ardrone_control_config.outdoor = enabled;
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(outdoor, &ardrone_control_config.outdoor, NULL);
		outdoorFlightSwitch.enabled = NO;
		outdoorFlightSwitch.alpha = 0.5;
		configuration_get();
	}
	else if(sender == outdoorShellSwitch)
	{
		bool_t enabled = (outdoorShellSwitch.on == YES);
		ardrone_control_config.flight_without_shell = enabled;
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(flight_without_shell, &ardrone_control_config.flight_without_shell, NULL);
	}
	else if(sender == adaptiveVideoSwitch)
	{
        ARDRONE_VARIABLE_BITRATE enabled = (adaptiveVideoSwitch.on == YES) ? ARDRONE_VARIABLE_BITRATE_MODE_DYNAMIC : ARDRONE_VARIABLE_BITRATE_MANUAL;
        uint32_t constantBitrate = (ARDRONE_VIDEO_CODEC_UVLC == ardrone_control_config.video_codec) ? 20000 : 15000;
        ardrone_control_config.bitrate_ctrl_mode = enabled;
        ardrone_control_config.bitrate = constantBitrate;
        ARDRONE_TOOL_CONFIGURATION_ADDEVENT(bitrate_ctrl_mode, &ardrone_control_config.bitrate_ctrl_mode, NULL);
        ARDRONE_TOOL_CONFIGURATION_ADDEVENT(bitrate, &ardrone_control_config.bitrate, NULL);
	}
    else if(sender == videoCodecSwitch)
    {
        ARDRONE_VIDEO_CODEC _codec = (videoCodecSwitch.selectedSegmentIndex == 0) ? ARDRONE_VIDEO_CODEC_UVLC : ARDRONE_VIDEO_CODEC_P264;
        ardrone_control_config.video_codec = _codec;
        ARDRONE_TOOL_CONFIGURATION_ADDEVENT(video_codec, &ardrone_control_config.video_codec, NULL);
        ardrone_control_config.bitrate = (ARDRONE_VIDEO_CODEC_UVLC == ardrone_control_config.video_codec) ? 20000 : 15000;
        ARDRONE_TOOL_CONFIGURATION_ADDEVENT(bitrate, &ardrone_control_config.bitrate, NULL);
    }
#ifdef INTERFACE_WITH_DEBUG
    else if(sender == wifiModeControl)
    {
        int wifiMode = wifiModeControl.selectedSegmentIndex;
        ardrone_control_config.wifi_mode = wifiMode;
        ARDRONE_TOOL_CONFIGURATION_ADDEVENT(wifi_mode, &ardrone_control_config.wifi_mode, NULL);
    }
#endif
	else
	{
		[self refresh];		
	}
}

- (IBAction)sliderRelease:(id)sender
{
	if(sender == droneTiltSlider)
	{
		float value = droneTiltSlider.value;
		ardrone_control_config.euler_angle_max = value * DEG_TO_RAD;
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(euler_angle_max, &ardrone_control_config.euler_angle_max, NULL);
	}
	else if(sender == iPhoneTiltSlider)
	{
		float value = iPhoneTiltSlider.value;
		ardrone_control_config.control_iphone_tilt = value * DEG_TO_RAD;
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(control_iphone_tilt, &ardrone_control_config.control_iphone_tilt, NULL);
	}
	else if(sender == verticalSpeedSlider)
	{
		float value = verticalSpeedSlider.value;
		ardrone_control_config.control_vz_max = value;
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(control_vz_max, &ardrone_control_config.control_vz_max, NULL);
	}
	else if(sender == yawSpeedSlider)
	{
		float value = yawSpeedSlider.value;
		ardrone_control_config.control_yaw = value * DEG_TO_RAD;
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(control_yaw, &ardrone_control_config.control_yaw, NULL);
	}
	else if(sender == interfaceAlphaSlider)
	{
		CGFloat value = interfaceAlphaSlider.value / 100.0f;
		[_delegate interfaceAlphaValueChanged:value];
		[self saveIPhoneSettings];
	}
}

#ifdef INTERFACE_WITH_DEBUG
- (IBAction)logsChanged:(id)sender
{
	bool_t b_value;
	
	if(logsSwitch.on)
		[self saveForDebugSettings];
	b_value = (logsSwitch.on == NO);
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(navdata_demo, &b_value, NULL);
	ardrone_navdata_write_to_file((bool_t)logsSwitch.on);
	[self refresh];
}

- (IBAction)clearLogsButtonClick:(id)sender
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask, YES);
	if([paths count] > 0)
	{
		NSString *documentsDirectory = [paths objectAtIndex:0];
		NSFileManager *fileManager = [[NSFileManager alloc] init];
		NSArray *documentsDirectoryContents = [fileManager contentsOfDirectoryAtPath:documentsDirectory error:nil];
		for(int i = 0 ; i < [documentsDirectoryContents count] ; i++)
		{
			if([[documentsDirectoryContents objectAtIndex:i] hasPrefix:@"mesures"] || [[documentsDirectoryContents objectAtIndex:i] hasPrefix:@"settings_"])
			{
				char filename[256];
				sprintf(filename, "%s/%s", [documentsDirectory cStringUsingEncoding:NSUTF8StringEncoding], [[documentsDirectoryContents objectAtIndex:i] cStringUsingEncoding:NSUTF8StringEncoding]);
				NSLog(@"- Remove %s", filename);
				remove(filename);
			}
		}
		[fileManager release];
	}
}
#endif

- (IBAction)clearButtonClick:(id)sender
{
//	NSLog(@"%s", __FUNCTION__);
	ardrone_control_config.indoor_euler_angle_max = ardrone_application_default_config.indoor_euler_angle_max;
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(indoor_euler_angle_max, &ardrone_control_config.indoor_euler_angle_max, NULL);

	ardrone_control_config.indoor_control_vz_max = ardrone_application_default_config.indoor_control_vz_max;
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(indoor_control_vz_max, &ardrone_control_config.indoor_control_vz_max, NULL);
	
	ardrone_control_config.indoor_control_yaw = ardrone_application_default_config.indoor_control_yaw;
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(indoor_control_yaw, &ardrone_control_config.indoor_control_yaw, NULL);
	
	ardrone_control_config.outdoor_euler_angle_max = ardrone_application_default_config.outdoor_euler_angle_max;
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(outdoor_euler_angle_max, &ardrone_control_config.outdoor_euler_angle_max, NULL);
	
	ardrone_control_config.outdoor_control_vz_max = ardrone_application_default_config.outdoor_control_vz_max;
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(outdoor_control_vz_max, &ardrone_control_config.outdoor_control_vz_max, NULL);
	
	ardrone_control_config.outdoor_control_yaw = ardrone_application_default_config.outdoor_control_yaw;
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(outdoor_control_yaw, &ardrone_control_config.outdoor_control_yaw, NULL);
	
	ardrone_control_config.outdoor = ardrone_application_default_config.outdoor;
	outdoorFlightSwitch.on = (ardrone_control_config.outdoor ? YES : NO);
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(outdoor, &ardrone_control_config.outdoor, NULL);
	
	ardrone_control_config.euler_angle_max = ardrone_application_default_config.euler_angle_max;
	droneTiltSlider.value = ardrone_control_config.euler_angle_max * RAD_TO_DEG;
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(euler_angle_max, &ardrone_control_config.euler_angle_max, NULL);
	
	ardrone_control_config.control_vz_max = ardrone_application_default_config.control_vz_max;
	verticalSpeedSlider.value = ardrone_control_config.control_vz_max;
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(control_vz_max, &ardrone_control_config.control_vz_max, NULL);
	
	ardrone_control_config.control_yaw = ardrone_application_default_config.control_yaw;
	yawSpeedSlider.value = ardrone_control_config.control_yaw * RAD_TO_DEG;
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(control_yaw, &ardrone_control_config.control_yaw, NULL);
	
	ardrone_control_config.outdoor_euler_angle_max = ardrone_application_default_config.outdoor_euler_angle_max;
	droneTiltSlider.value = ardrone_control_config.euler_angle_max * RAD_TO_DEG;
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(euler_angle_max, &ardrone_control_config.euler_angle_max, NULL);
	
	ardrone_control_config.control_vz_max = ardrone_application_default_config.control_vz_max;
	verticalSpeedSlider.value = ardrone_control_config.control_vz_max;
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(control_vz_max, &ardrone_control_config.control_vz_max, NULL);
	
	ardrone_control_config.control_yaw = ardrone_application_default_config.control_yaw;
	yawSpeedSlider.value = ardrone_control_config.control_yaw * RAD_TO_DEG;
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(control_yaw, &ardrone_control_config.control_yaw, NULL);

	ardrone_control_config.control_iphone_tilt = ardrone_application_default_config.control_iphone_tilt;
	iPhoneTiltSlider.value = ardrone_control_config.control_iphone_tilt * RAD_TO_DEG;
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(control_iphone_tilt, &ardrone_control_config.control_iphone_tilt, NULL);
	
	ardrone_control_config.flight_without_shell = ardrone_application_default_config.flight_without_shell;
	outdoorShellSwitch.on = (ardrone_control_config.flight_without_shell ? YES : NO);
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(flight_without_shell, &ardrone_control_config.flight_without_shell, NULL);

	ardrone_control_config.altitude_max = ardrone_application_default_config.altitude_max;
	altituteLimitedSwitch.on = ((ardrone_control_config.altitude_max == ALTITUDE_LIMITED) ? YES : NO);
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(altitude_max, &ardrone_control_config.altitude_max, NULL);
    
    ardrone_control_config.video_codec = ardrone_application_default_config.video_codec;
    videoCodecSwitch.selectedSegmentIndex = (ARDRONE_VIDEO_CODEC_UVLC == ardrone_control_config.video_codec) ? 0 : 1;
    ARDRONE_TOOL_CONFIGURATION_ADDEVENT(video_codec, &ardrone_control_config.video_codec, NULL);

	ardrone_control_config.bitrate_ctrl_mode = ardrone_application_default_config.bitrate_ctrl_mode;
	adaptiveVideoSwitch.on = (ardrone_control_config.bitrate_ctrl_mode ? YES : NO);
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(bitrate_ctrl_mode, &ardrone_control_config.bitrate_ctrl_mode, NULL);
	
	acceleroDisabledSwitch.on = NO;
	[_delegate acceleroValueChanged:(acceleroDisabledSwitch.on == NO)];

	leftHandedSwitch.on = NO;
	[_delegate controlModeChanged:CONTROL_MODE3];
	
	interfaceAlphaSlider.value = 50.0;
	[_delegate interfaceAlphaValueChanged: (interfaceAlphaSlider.value / 100.0)];

	[self saveIPhoneSettings];
	
	[self refresh];
}

- (IBAction)flatTrimButtonClick:(id)sender
{
//	NSLog(@"%s", __FUNCTION__);
	ardrone_at_set_flat_trim();
}

- (IBAction)okButtonClick:(id)sender
{
//	NSLog(@"%s", __FUNCTION__);
	self.view.hidden = YES;
}

- (void)switchDisplay
{
	self.view.hidden = !self.view.hidden;
}

- (void)textFieldDidBeginEditing:(UITextField *)theTextField           // became first responder
{
//	NSLog(@"%s", __FUNCTION__);
	if(theTextField == droneSSIDTextField)
	{
		ssidChangeInProgress = NO;
		CGPoint scrollOffset = { 0.0, droneSSIDTextField.frame.origin.y - 30.0 };
		if (usingIPad && compiledForIPad)
		{
			scrollOffset.y -= 270;
		}
		[scrollView setContentOffset:scrollOffset animated:YES];
	}
}

- (BOOL)textFieldShouldReturn:(UITextField *)theTextField 
{
//	NSLog(@"%s", __FUNCTION__);
	if(theTextField == droneSSIDTextField)
	{
		NSString *str;
		str = [NSString stringWithFormat:ARDroneEngineLocalizeString (@"Your changes will be applied after rebooting the AR.Drone !\n" \
																	  "\t- Quit application\n" \
																	  "\t- Reboot your AR.Drone\n" \
																	  "\t- Connect your %@ on %s network\n" \
																	  "\t- Launch application"),
																		[[UIDevice currentDevice] model],
																		[droneSSIDTextField.text cStringUsingEncoding:NSUTF8StringEncoding]];
		strcpy(ardrone_control_config.ssid_single_player, [droneSSIDTextField.text cStringUsingEncoding:NSUTF8StringEncoding]);
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(ssid_single_player, ardrone_control_config.ssid_single_player, NULL);
		
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:ARDroneEngineLocalizeString(@"New AR.Drone network") message:str delegate:self cancelButtonTitle:@"Ok" otherButtonTitles:nil, nil];
		[alert show];
		[alert release];
	}

	[theTextField resignFirstResponder];
	return YES;
}

- (BOOL)textFieldShouldEndEditing:(UITextField *)theTextField
{
	if(theTextField == droneSSIDTextField)
	{
		if(!ssidChangeInProgress && (strcmp(ardrone_control_config.ssid_single_player, "") != 0))
			droneSSIDTextField.text = [NSString stringWithCString:ardrone_control_config.ssid_single_player encoding:NSUTF8StringEncoding];

	}
	return YES;
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	NSLog(@"%s", __FUNCTION__);
	ssidChangeInProgress = YES;
}


- (void)saveIPhoneSettings
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentDirectory = [paths objectAtIndex:0];
	const char *filename = [[documentDirectory stringByAppendingFormat:@"/settings.txt"] cStringUsingEncoding:NSUTF8StringEncoding];
	FILE *file = fopen(filename, "w+");
	if(file)
	{
		fprintf(file, "Interface Alpha : %d\n", (int)interfaceAlphaSlider.value);
		fprintf(file, "Accelero Disabled : %d\n", (int)acceleroDisabledSwitch.on);
		fprintf(file, "Left Handed : %d\n", (int)leftHandedSwitch.on);
		
		fclose(file);
	}
}

- (void)loadIPhoneSettings
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentDirectory = [paths objectAtIndex:0];
	const char *filename = [[documentDirectory stringByAppendingFormat:@"/settings.txt"] cStringUsingEncoding:NSUTF8StringEncoding];
	FILE *file = fopen(filename, "r");
	int alpha = 50;
	int acceleroDisabled = 0;
	int leftHanded = 0;

	if(file)
	{
		fscanf(file,"Interface Alpha : %d\n",&alpha);
		fscanf(file,"Accelero Disabled : %d\n", &acceleroDisabled);
		fscanf(file,"Left Handed : %d\n", &leftHanded);
		fclose(file);
	}

	CGFloat alpha_value = (float)alpha / 100.0f;
	[_delegate interfaceAlphaValueChanged:alpha_value];
	interfaceAlphaSlider.value = (float)alpha;

	[_delegate acceleroValueChanged:!(bool_t)acceleroDisabled];
	acceleroDisabledSwitch.on = (BOOL)acceleroDisabled;

	[_delegate controlModeChanged:(leftHanded ? CONTROL_MODE2 : CONTROL_MODE3)];
	leftHandedSwitch.on = (BOOL)leftHanded;
}

- (void)saveForDebugSettings
{
	struct timeval tv;
	// Save backups of Settings in text
	gettimeofday(&tv,NULL);
	struct tm *settings_atm = localtime(&tv.tv_sec);
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES); //creates paths so that you can pull the app's path from it
	NSString *documentsDirectory = [paths objectAtIndex:0]; // gets the applications directory on the users iPhone
	
	const char *filename = [[documentsDirectory stringByAppendingFormat:@"/settings_%04d%02d%02d_%02d%02d%02d.txt",
							 settings_atm->tm_year+1900, settings_atm->tm_mon+1, settings_atm->tm_mday,
							 settings_atm->tm_hour, settings_atm->tm_min, settings_atm->tm_sec] cStringUsingEncoding:NSUTF8StringEncoding];
	FILE *file = fopen(filename, "w");
	if(file)
	{
		fprintf(file, "iPhone section\n");
		fprintf(file, "Accelero Disabled  : %s\n", (int)acceleroDisabledSwitch.on ? "YES" : "NO");
		fprintf(file, "Combined yaw       : %s\n", ((ardrone_control_config.control_level >> CONTROL_LEVEL_COMBINED_YAW) & 0x1) ? "ON" : "OFF");
		fprintf(file, "Left Handed		  : %s\n", (int)leftHandedSwitch.on ? "YES" : "NO");
		fprintf(file, "Iphone Tilt        : %0.2f\n", ardrone_control_config.control_iphone_tilt * RAD_TO_DEG);
		fprintf(file, "Interface Alpha    : %d\n", (int)interfaceAlphaSlider.value);
		fprintf(file, "\n");
		fprintf(file, "AR.Drone section\n");
        fprintf(file, "Video codec        : %s\n", (ARDRONE_VIDEO_CODEC_UVLC == ardrone_control_config.video_codec) ? "VLIB" : "P264");
        fprintf(file, "Wifi Mode          : %s\n", (2 == ardrone_control_config.wifi_mode) ? "Managed" : ((1 == ardrone_control_config.wifi_mode) ? "Ad-Hoc" : "Access Point"));
        fprintf(file, "Adaptive video	  : %s\n", ardrone_control_config.bitrate_ctrl_mode ? "ON" : "OFF");
		fprintf(file, "Pairing            : %s\n", strcmp(ardrone_control_config.owner_mac, NULL_MAC) != 0 ? "ON" : "OFF");
		fprintf(file, "Drone Network SSID : %s\n", ardrone_control_config.ssid_single_player);
		fprintf(file, "Altitude Limited   : %s\n", (ardrone_control_config.altitude_max == ALTITUDE_LIMITED) ? "ON" : "OFF");
		fprintf(file, "Outdoor Shell      : %s\n", ardrone_control_config.flight_without_shell ? "ON" : "OFF");
		fprintf(file, "Outdoor Flight     : %s\n", ardrone_control_config.outdoor ? "ON" : "OFF");
		fprintf(file, "Yaw Speed          : %0.2f\n", ardrone_control_config.control_yaw * RAD_TO_DEG);
		fprintf(file, "Vertical Speed     : %0.2f\n", (float)ardrone_control_config.control_vz_max);
		fprintf(file, "Drone Tilt         : %0.2f\n", ardrone_control_config.euler_angle_max * RAD_TO_DEG);
		fclose(file);
	}
	else 
	{
		NSLog(@"%s not found", filename);
	}		
}

- (void)dealloc
{
	[super dealloc];	
}
@end
