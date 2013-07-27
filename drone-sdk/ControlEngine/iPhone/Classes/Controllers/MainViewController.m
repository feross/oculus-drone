//
//  MainViewController.m
//  ARDroneEngine
//
//  Created by Mykonos on 17/12/09.
//  Copyright 2009 Parrot SA. All rights reserved.
//
#import "MainViewController.h"
#import "OpenGLVideo.h"

extern navdata_unpacked_t ctrlnavdata;

@interface MainViewController ()
ARDroneNavigationData navigationData;
ARDroneEnemiesData humanEnemiesData;	
ARDroneDetectionCamera detectionCamera;
ARDroneCamera droneCamera;

id<NavdataProtocol>navdata_delegate;
CGRect screenFrame;
BOOL screenOrientationRight;
BOOL bContinue;
ARDroneHUDConfiguration hudconfig; 
ControlData *controlData;

-(void)TimerHandler;
-(void)update;
@end

@implementation MainViewController
- (id) initWithFrame:(CGRect)frame withState:(BOOL)inGame withDelegate:(id<ARDroneProtocolOut>)delegate withNavdataDelegate:(id<NavdataProtocol>)_navdata_delegate withControlData:(ControlData*)_controlData withHUDConfiguration:(ARDroneHUDConfiguration*)hudconfiguration
{
	NSLog(@"Main View Controller Frame : %@", NSStringFromCGRect(frame));
	if(self = [super init])
	{
		bContinue = TRUE;
		screenOrientationRight = YES;
		screenFrame = frame;
		gameEngine = delegate;
		navdata_delegate = _navdata_delegate;
		controlData = _controlData;

		vp_os_memset(&navigationData, 0x0, sizeof(ARDroneNavigationData)); 
		vp_os_memset(&detectionCamera, 0x0, sizeof(ARDroneCamera));
		vp_os_memset(&droneCamera, 0x0, sizeof(ARDroneCamera)); 
		humanEnemiesData.count = 0;
		
		for(int i = 0 ; i < ARDRONE_MAX_ENEMIES ; i++)
		{
			vp_os_memset(&humanEnemiesData.data[i], 0x0, sizeof(ARDroneEnemyData));
			humanEnemiesData.data[i].width = 1.0;
			humanEnemiesData.data[i].height = 1.0;
		}

		if(hudconfiguration == nil)
		{
			hudconfig.enableBackToMainMenu = NO;
			hudconfig.enableSwitchScreen = YES;
			hudconfig.enableBatteryPercentage = YES;
		}
		else
		{
			vp_os_memcpy(&hudconfig, hudconfiguration, sizeof(ARDroneHUDConfiguration));
		}
		
		hud = [[HUD alloc] initWithFrame:screenFrame withState:inGame withHUDConfiguration:hudconfig withControlData:controlData];
		[self.view addSubview:hud.view];
		hud.view.multipleTouchEnabled = YES;
		
		menuSettings = [[SettingsMenu alloc] initWithFrame:screenFrame AndHUDConfiguration:hudconfig withDelegate:hud withControlData:controlData];
		menuSettings.view.hidden = YES;
		menuSettings.view.multipleTouchEnabled = YES;
		[self.view addSubview:menuSettings.view];
		self.view.multipleTouchEnabled = YES;
		
		[self changeState:inGame];
		
		[NSThread detachNewThreadSelector:@selector(TimerHandler) toTarget:self withObject:nil];
	}
	
	return self;
}

- (void)setWifiReachabled:(BOOL)reachabled
{
	controlData->wifiReachabled = reachabled;
}

-(void)setScreenOrientationRight:(BOOL)right
{
	screenOrientationRight = right;
}

- (void) update
{
	static CONFIG_STATE prev_config_state = CONFIG_STATE_IDLE;
	static bool_t prev_navdata_connected = FALSE;
	
	if(prev_config_state != controlData->configurationState)
	{
		if(controlData->configurationState == CONFIG_STATE_IDLE)
			[menuSettings configChanged];
		prev_config_state = controlData->configurationState;
	}		
		
	if(prev_navdata_connected != controlData->navdata_connected)
	{
		[menuSettings configChanged];
		prev_navdata_connected = controlData->navdata_connected;
	}
	
	if(hud.firePressed == YES) 
	{
		[gameEngine executeCommandOut:ARDRONE_COMMAND_FIRE withParameter:nil fromSender:self.view];
	}
	else if(hud.mainMenuPressed == YES)
	{
		hud.mainMenuPressed = NO;
		[gameEngine executeCommandOut:ARDRONE_COMMAND_PAUSE withParameter:nil fromSender:self.view];
	}
	else if(hud.settingsPressed == YES)
	{
		hud.settingsPressed = NO;
		[menuSettings performSelectorOnMainThread:@selector(switchDisplay) withObject:nil waitUntilDone:YES];
	}
	
	// Set velocities	
	navigationData.linearVelocity.x = -ctrlnavdata.navdata_demo.vy;
	navigationData.linearVelocity.y = ctrlnavdata.navdata_demo.vz;
	navigationData.linearVelocity.z = ctrlnavdata.navdata_demo.vx;
	navigationData.angularPosition.x = -ctrlnavdata.navdata_demo.theta / 1000;
	navigationData.angularPosition.y = ctrlnavdata.navdata_demo.psi / 1000;
	navigationData.angularPosition.z = ctrlnavdata.navdata_demo.phi / 1000;
	navigationData.navVideoNumFrames = ctrlnavdata.navdata_demo.num_frames;
	navigationData.videoNumFrames    = get_video_current_numframes();

	// Set flying state.
	ARDRONE_FLYING_STATE tmp_state = ardrone_navdata_get_flying_state(ctrlnavdata);	
	if(navigationData.flyingState != tmp_state)
	{
		NSLog(@"Flying state switch to %d", tmp_state);
		if(hudconfig.enableBackToMainMenu)
			[hud showBackToMainMenu:(tmp_state == ARDRONE_FLYING_STATE_LANDED)];
	}
	
	navigationData.flyingState = tmp_state;
	navigationData.emergencyState = ctrlnavdata.ardrone_state & ARDRONE_EMERGENCY_MASK;
	navigationData.detection_type = (ARDRONE_CAMERA_DETECTION_TYPE)ctrlnavdata.navdata_demo.detection_camera_type;
	navigationData.finishLineCount = ctrlnavdata.navdata_games.finish_line_counter;
	navigationData.doubleTapCount = ctrlnavdata.navdata_games.double_tap_counter;
    navigationData.isInit = (ctrlnavdata.ardrone_state & ARDRONE_NAVDATA_BOOTSTRAP) ? 0 : 1;

	// Set detected ARDRONE_ENEMY_HUMAN enemies if detected.
	humanEnemiesData.count = MIN(ctrlnavdata.navdata_vision_detect.nb_detected, ARDRONE_MAX_ENEMIES);
	
	//printf("enemies count : %d\n", humanEnemiesData.count);
	for(int i = 0 ; i < humanEnemiesData.count ; i++)
	{
		humanEnemiesData.data[i].width = 2 * ctrlnavdata.navdata_vision_detect.width[i] / 1000.0;
		humanEnemiesData.data[i].height = 2 * ctrlnavdata.navdata_vision_detect.height[i] / 1000.0;		
		humanEnemiesData.data[i].position.x = (2 * ctrlnavdata.navdata_vision_detect.xc[i] / 1000.0) - 1.0;
		humanEnemiesData.data[i].position.y = -(2 * ctrlnavdata.navdata_vision_detect.yc[i] / 1000.0) + 1.0;
		humanEnemiesData.data[i].position.z = ctrlnavdata.navdata_vision_detect.dist[i];
		humanEnemiesData.data[i].orientation_angle = ctrlnavdata.navdata_vision_detect.orientation_angle[i];
	}
	
	// Set Detection Camera
	vp_os_memcpy(detectionCamera.rotation, &ctrlnavdata.navdata_demo.detection_camera_rot, sizeof(float) * 9);
	vp_os_memcpy(detectionCamera.translation, &ctrlnavdata.navdata_demo.detection_camera_trans, sizeof(float) * 3);
	detectionCamera.tag_index = ctrlnavdata.navdata_demo.detection_tag_index;
	
	// Set Drone Camera rotation
	vp_os_memcpy(droneCamera.rotation, &ctrlnavdata.navdata_demo.drone_camera_rot, sizeof(float) * 9);
	
	// Set Drone Camera translation
	// Get enemies data
	if ([gameEngine respondsToSelector:@selector(AIEnemiesData:)])
	{
		ARDroneEnemiesData AIEnemiesData;
		vp_os_memset(&AIEnemiesData, 0x0, sizeof(ARDroneEnemiesData));
		[gameEngine AIEnemiesData:&AIEnemiesData];		
	}
	
	vp_os_memcpy(&droneCamera.translation[0], &ctrlnavdata.navdata_demo.drone_camera_trans, sizeof(float) * 3);
	
	// Set battery level in hud view
	[hud setBattery:(int)ctrlnavdata.navdata_demo.vbat_flying_percentage];
	
	// Set  all texts in Hud view
	if((strlen(controlData->error_msg) != 0) && (controlData->framecounter >= (kFPS / 2.0)))
		[hud setMessageBox:[[NSBundle mainBundle] localizedStringForKey :[NSString stringWithCString:controlData->error_msg encoding:NSUTF8StringEncoding] value:@"" table:@"languages"]];
	else
		[hud setMessageBox:@""];
	
	[hud performSelectorOnMainThread:@selector(setTakeOff:) withObject:[NSString stringWithFormat:@"%s", controlData->takeoff_msg] waitUntilDone:YES];
	[hud performSelectorOnMainThread:@selector(setEmergency:) withObject:[NSString stringWithFormat:@"%s", controlData->emergency_msg] waitUntilDone:YES];
}

- (void) TimerHandler {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]; // Top-level pool
	
	ardrone_timer_t timer;
	int refreshTimeInUs = 1000000 / kFPS;
	
	ardrone_timer_reset(&timer);
	ardrone_timer_update(&timer);
	
	while(bContinue)
	{
		int delta = ardrone_timer_delta_us(&timer);
		if( delta >= refreshTimeInUs)
		{
			// Render frame
			ardrone_timer_update(&timer);
			
			if(self.view.hidden == NO)
			{
				[navdata_delegate parrotNavdata:&ctrlnavdata];
				[self performSelectorOnMainThread:@selector(update) withObject:nil waitUntilDone:YES];
				checkErrors();
			}
			
			controlData->framecounter = (controlData->framecounter + 1) % kFPS;
		}
		else
		{
			//printf("Time waited : %d us\n", refreshTimeInUs - delta);
			usleep(refreshTimeInUs - delta);
		}
	}

    [pool release];  // Release the objects in the pool.
}

- (ARDroneNavigationData*)navigationData
{
	return &navigationData;
}

- (ARDroneDetectionCamera*)detectionCamera
{
	return &detectionCamera;
}

- (ARDroneCamera*)droneCamera
{
	return &droneCamera;
}

- (ARDroneEnemiesData*)humanEnemiesData
{
	return &humanEnemiesData;
}

- (void)changeState:(BOOL)inGame
{
	self.view.hidden = !inGame;
	[hud changeState:inGame];
    
    if (inGame)
        configuration_get();
}

- (void)executeCommandIn:(ARDRONE_COMMAND_IN_WITH_PARAM)commandIn fromSender:(id)sender refreshSettings:(BOOL)refresh
{
	int32_t i_value;
	switch (commandIn.command) {
	
		case ARDRONE_COMMAND_ISCLIENT:
			i_value = ((int)commandIn.parameter == 0) ? ADC_CMD_SELECT_ULTRASOUND_25Hz : ADC_CMD_SELECT_ULTRASOUND_22Hz;
			ARDRONE_TOOL_CONFIGURATION_ADDEVENT(ultrasound_freq, &i_value, commandIn.callback);
			break;
			
		case ARDRONE_COMMAND_DRONE_ANIM:
			{
				ARDRONE_ANIMATION_PARAM *param = (ARDRONE_ANIMATION_PARAM*)commandIn.parameter;
				char str_param[SMALL_STRING_SIZE];
				sprintf(str_param, "%d,%d", param->drone_anim, ((param->timeout == 0) ? MAYDAY_TIMEOUT[param->drone_anim] : param->timeout));
				ARDRONE_TOOL_CONFIGURATION_ADDEVENT(flight_anim, str_param, commandIn.callback);
			}
			break;
			
		case ARDRONE_COMMAND_DRONE_LED_ANIM:
			{
				char param[SMALL_STRING_SIZE];
				float_or_int_t freq;
				freq.f = ((ARDRONE_LED_ANIMATION_PARAM*)commandIn.parameter)->frequency;
				sprintf(param, "%d,%d,%d", ((ARDRONE_LED_ANIMATION_PARAM*)commandIn.parameter)->led_anim, freq.i, ((ARDRONE_LED_ANIMATION_PARAM*)commandIn.parameter)->duration);
				ARDRONE_TOOL_CONFIGURATION_ADDEVENT(leds_anim, param, commandIn.callback);
			}
			break;
			
		case ARDRONE_COMMAND_ENABLE_COMBINED_YAW:
			{
				bool_t enable = (bool_t)commandIn.parameter;
				i_value = enable ? (ardrone_control_config.control_level | (1 << CONTROL_LEVEL_COMBINED_YAW)) : (ardrone_control_config.control_level & ~(1 << CONTROL_LEVEL_COMBINED_YAW));
				ARDRONE_TOOL_CONFIGURATION_ADDEVENT(control_level, &i_value, commandIn.callback);
				[hud combinedYawValueChanged:enable];
			}
			break;
			
		case ARDRONE_COMMAND_SET_CONFIG:
		{
			ARDRONE_CONFIG_PARAM *param = (ARDRONE_CONFIG_PARAM *)commandIn.parameter;
			switch (param->config_key)
			{
#undef COMMAND_IN_CONFIG_KEY
#undef COMMAND_IN_CONFIG_KEY_STRING
#define COMMAND_IN_CONFIG_KEY(CASE, KEY, TYPE)													\
case CASE:																						\
	ardrone_control_config.KEY = *(TYPE *)(param->pvalue);										\
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT (KEY, &ardrone_control_config.KEY, commandIn.callback);	\
	break;
#define COMMAND_IN_CONFIG_KEY_STRING(CASE, KEY)													\
case CASE:																						\
	strcpy (ardrone_control_config.KEY, (char *)(param->pvalue));								\
	ARDRONE_TOOL_CONFIGURATION_ADDEVENT (KEY, ardrone_control_config.KEY, commandIn.callback);	\
	break;
#include "ARDroneGeneratedCommandIn.h"
#undef COMMAND_IN_CONFIG_KEY
#undef COMMAND_IN_CONFIG_KEY_STRING
					
				default:
					NSLog(@"The ARDRONE_CONFIG_KEY %d is not implemented !", param->config_key);
					break;
			}
		}	
		break;
		
		default:
			NSLog(@"The ARDRONE_COMMAND_IN %d is not implemented !", commandIn.command);
			break;
	}
	
	if (refresh)
	{
		[menuSettings performSelectorOnMainThread:@selector(configChanged) withObject:nil waitUntilDone:YES];
	}
}

- (void)executeCommandIn:(ARDRONE_COMMAND_IN)commandId withParameter:(void*)parameter fromSender:(id)sender
{
	int32_t i_value;
	switch (commandId) {
		case ARDRONE_COMMAND_ISCLIENT:
        {
            i_value = ((int)parameter == 0) ? ADC_CMD_SELECT_ULTRASOUND_25Hz : ADC_CMD_SELECT_ULTRASOUND_22Hz;
            ARDRONE_TOOL_CONFIGURATION_ADDEVENT(ultrasound_freq, &i_value, NULL);
        }
            break;
			
		case ARDRONE_COMMAND_DRONE_ANIM:
        {
            ARDRONE_ANIMATION_PARAM *param = (ARDRONE_ANIMATION_PARAM*)parameter;
            char str_param[SMALL_STRING_SIZE];
            sprintf(str_param, "%d,%d", param->drone_anim, ((param->timeout == 0) ? MAYDAY_TIMEOUT[param->drone_anim] : param->timeout));
            ARDRONE_TOOL_CONFIGURATION_ADDEVENT(flight_anim, str_param, NULL);
        }
			break;
			
		case ARDRONE_COMMAND_VIDEO_CHANNEL:
			i_value = (int32_t)parameter;
			ARDRONE_TOOL_CONFIGURATION_ADDEVENT(video_channel, &i_value, NULL);
			break;
			
		case ARDRONE_COMMAND_SET_FLY_MODE:
			i_value = (int32_t)parameter;
			ARDRONE_TOOL_CONFIGURATION_ADDEVENT(flying_mode, &i_value, NULL);
			break;
			
		case ARDRONE_COMMAND_CAMERA_DETECTION:
			i_value = (int32_t)parameter;
			ARDRONE_TOOL_CONFIGURATION_ADDEVENT(detect_type, &i_value, NULL);
			break;
			
		case ARDRONE_COMMAND_ENEMY_SET_PARAM:
			i_value = ((ARDRONE_ENEMY_PARAM*)parameter)->color;
			ARDRONE_TOOL_CONFIGURATION_ADDEVENT(enemy_colors, &i_value, NULL);
			i_value = ((ARDRONE_ENEMY_PARAM*)parameter)->outdoor_shell;
			ARDRONE_TOOL_CONFIGURATION_ADDEVENT(enemy_without_shell, &i_value, NULL);
			break;
			
		case ARDRONE_COMMAND_DRONE_LED_ANIM:
		{
			char param[SMALL_STRING_SIZE];
			float_or_int_t freq;
			freq.f = ((ARDRONE_LED_ANIMATION_PARAM*)parameter)->frequency;
			sprintf(param, "%d,%d,%d", ((ARDRONE_LED_ANIMATION_PARAM*)parameter)->led_anim, freq.i, ((ARDRONE_LED_ANIMATION_PARAM*)parameter)->duration);
			ARDRONE_TOOL_CONFIGURATION_ADDEVENT(leds_anim, param, NULL);
		}
			break;
			
		case ARDRONE_COMMAND_ENABLE_COMBINED_YAW:
        {
            bool_t enable = (bool_t)parameter;
            i_value = enable ? (ardrone_control_config.control_level | (1 << CONTROL_LEVEL_COMBINED_YAW)) : (ardrone_control_config.control_level & ~(1 << CONTROL_LEVEL_COMBINED_YAW));
            ARDRONE_TOOL_CONFIGURATION_ADDEVENT(control_level, &i_value, NULL);
            [hud combinedYawValueChanged:enable];
        }
			break;
			
		default:
			NSLog(@"The ARDRONE_COMMAND_IN %d is not implemented !");
			break;
	}
}

- (void)setDefaultConfigurationForKey:(ARDRONE_CONFIG_KEYS)key withValue:(void *)value
{
	switch (key)
	{
#undef COMMAND_IN_CONFIG_KEY
#undef COMMAND_IN_CONFIG_KEY_STRING
#define COMMAND_IN_CONFIG_KEY(CASE, KEY, TYPE)									\
		case CASE:																\
			ardrone_application_default_config.KEY = *(TYPE *)(value);			\
			break;
#define COMMAND_IN_CONFIG_KEY_STRING(CASE, KEY)									\
		case CASE:																\
			strcpy (ardrone_application_default_config.KEY, (char *)(value));	\
			break;
#include "ARDroneGeneratedCommandIn.h"
#undef COMMAND_IN_CONFIG_KEY
#undef COMMAND_IN_CONFIG_KEY_STRING
		default:
			NSLog(@"The ARDRONE_CONFIG_KEY %d is not implemented !", key);
			break;
	}
}

- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return (toInterfaceOrientation == UIInterfaceOrientationLandscapeRight || toInterfaceOrientation == UIInterfaceOrientationLandscapeLeft);
}

- (void)didReceiveMemoryWarning 
{
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
}


- (void)dealloc {
	[self changeState:NO];
	bContinue = TRUE;
	
	[hud release];
	[menuSettings release];
	
	[super dealloc];
}

@end
