/**
 * @file ARDrone.m
 *
 * Copyright 2009 Parrot SA. All rights reserved.
 * @author D HAEYER Frederic
 * @date 2009/10/26
 */
#import "MainViewController.h"
#import "GLViewController.h"
#import "InternalProtocols.h"
#import "ARDrone.h"
#import "TVOut.h"

//#define DEBUG_ENNEMIES_DETECTON
//#define DEBUG_NAVIGATION_DATA
//#define DEBUG_DETECTION_CAMERA
//#define DEBUG_DRONE_CAMERA

extern navdata_unpacked_t ctrlnavdata;
extern char drone_address[];
extern ControlData ctrldata;
static bool_t threadStarted = false;
char root_dir[256];

static void ARDroneCallback(ARDRONE_ENGINE_MESSAGE msg)
{
	switch(msg)
	{
		case ARDRONE_ENGINE_INIT_OK:
			threadStarted = true;
			break;
			
		default:
			break;
	}
}

@interface ARDrone () <NavdataProtocol>
	GLViewController *glviewctrl;
	MainViewController *mainviewctrl;
	BOOL inGameOnDemand;
	CGRect screenFrame;
	id <ARDroneProtocolOut> _uidelegate;
	ARDroneHUDConfiguration *hudconfig;

- (void) parrotNavdata:(navdata_unpacked_t*)data;
- (void) checkThreadStatus;
@end

/**
 * Define a few methods to make it possible for the game engine to control the Parrot drone
 */
@implementation ARDrone
@synthesize running;
@synthesize view;

/**
 * Initialize the Parrot library.<br/>
 * Note: the library will clean-up everything it allocates by itself, when being destroyed (i.e. when its retain count reaches 0).
 *
 * @param frame Frame of parent view, used to create the library view (which shall cover the whole screen).
 * @param inGame Initial state of the game at startup.
 * @param uidelegate Pointer to the object that implements the Parrot protocol ("ARDroneProtocol"), which will be called whenever the library needs the game engine to change its state.
 * @return Pointer to the newly initialized Parrot library instance.
 */
- (id) initWithFrame:(CGRect)frame withState:(BOOL)inGame withDelegate:(id <ARDroneProtocolOut>)uidelegate withHUDConfiguration:(ARDroneHUDConfiguration*)hudconfiguration
{
	if((self = [super init]) != nil)
	{
		NSLog(@"Frame ARDrone Engine : %f, %f", frame.size.width, frame.size.height);

#ifdef ENABLE_AUTO_TVOUT
		[[[TVOut alloc] init] setupScreenMirroringWithFramesPerSecond:10];
#endif

		running = NO;
		inGameOnDemand = inGame;
		threadStarted = false;
		_uidelegate = uidelegate;
		
		// Update user path
		[[NSFileManager defaultManager]changeCurrentDirectoryPath:[[NSBundle mainBundle]resourcePath]];
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES); //creates paths so that you can pull the app's path from it
		strcpy(root_dir, [[paths objectAtIndex:0] cStringUsingEncoding:NSUTF8StringEncoding]);
		
		// Create View Controller
		glviewctrl = [[GLViewController alloc] initWithFrame:frame withDelegate:self];
		
		// Create main view controller
		initControlData();
		mainviewctrl = [[MainViewController alloc] initWithFrame:frame withState:inGame withDelegate:uidelegate withNavdataDelegate:self withControlData:&ctrldata withHUDConfiguration:hudconfiguration];
		view = mainviewctrl.view;
		
		NSString *bundleName = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleIdentifier"];
        NSString *username = [NSString stringWithFormat:@".%@:%@", [[UIDevice currentDevice] model], [[UIDevice currentDevice] uniqueIdentifier]];
		ardroneEngineStart(ARDroneCallback, [bundleName cStringUsingEncoding:NSUTF8StringEncoding], [username cStringUsingEncoding:NSUTF8StringEncoding]);
		[self checkThreadStatus];
	}
	
	return( self );
}

- (void) checkThreadStatus
{
	NSLog(@"%s", __FUNCTION__);

	[mainviewctrl setWifiReachabled:threadStarted];

	if(threadStarted)
	{
		running = YES;
		[_uidelegate executeCommandOut:ARDRONE_COMMAND_RUN withParameter:(void*)drone_address fromSender:self];
		[self changeState:inGameOnDemand];
	}
	else
	{
		[NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(checkThreadStatus) userInfo:nil repeats:NO];
	}
}

/*
 * Render a frame. Basically, the Parrot Library renders:<ul>
 * <li> A full screen textured quad in the background (= the video sent by the drone);</li>
 * <li> A set of elements in the foreground (=HUD).</li>
 * </ul>
 */
- (void) render
{		
	// Make sure the library is "running"
	[glviewctrl drawView];
}

- (void) dealloc
{
	[self changeState:NO];
	
	[mainviewctrl release];
	[glviewctrl release];
	
	[super dealloc];
}

/**
 * Set what shall be the orientation of the screen when rendering a frame.
 *
 * @param right Orientation of the screen: FALSE for "landscape left", TRUE for "landscape right".
 */
- (void)setScreenOrientationRight:(BOOL)right
{
	NSLog(@"Screen orientation right : %d", right);
	if(mainviewctrl != nil)
		[mainviewctrl setScreenOrientationRight:right];
	
	if(glviewctrl != nil)
		[glviewctrl setScreenOrientationRight:right];	
}

- (void)parrotNavdata:(navdata_unpacked_t*)data
{
	ardrone_navdata_get_data(data);
}

/**
 * Get the latest drone's navigation data.
 *
 * @param data Pointer to a navigation data structure.
 */
- (void)navigationData:(ARDroneNavigationData*)data
{
	if(view != nil)
	{
		vp_os_memcpy(data, [mainviewctrl navigationData], sizeof(ARDroneNavigationData));
#ifdef DEBUG_NAVIGATION_DATA
		static int numsamples = 0;
		if(numsamples++ > 64)
		{
			NSLog(@"x : %f, y : %f, z : %f, flying state : %d, navdata video num frame : %d, video num frames : %d", data->angularPosition.x, data->angularPosition.y, data->angularPosition.z, data->flyingState, data->navVideoNumFrames, data->videoNumFrames);		
			numsamples = 0;
		}
#endif
	}
}

/**
 * Get the latest detection camera structure (rotation and translation).
 *
 * @param data Pointer to a detection camera structure.
 */
- (void)detectionCamera:(ARDroneDetectionCamera*)camera
{
	if(view != nil)
	{
		vp_os_memcpy(camera, [mainviewctrl detectionCamera], sizeof(ARDroneDetectionCamera));
#ifdef DEBUG_DETECTION_CAMERA
		static int numsamples = 0;
		if(numsamples++ > 64)
		{
			NSLog(@"Detection Camera Rotation : %f %f %f %f %f %f %f %f %f",
					camera->rotation[0][0], camera->rotation[0][1], camera->rotation[0][2],
					camera->rotation[1][0], camera->rotation[1][1], camera->rotation[1][2],
					camera->rotation[2][0], camera->rotation[2][1], camera->rotation[2][2]);
			NSLog(@"Detection Camera Translation : %f %f %f", camera->translation[0], camera->translation[1], camera->translation[2]);
			NSLog(@"Detection Camera Tag Index : %d", camera->tag_index);
			
			numsamples = 0;
		}
#endif
	}
}

/**
 * Get the latest drone camera structure (rotation and translation).
 *
 * @param data Pointer to a drone camera structure.
 */
- (void)droneCamera:(ARDroneCamera*)camera
{
	if(view != nil)
	{
		vp_os_memcpy(camera, [mainviewctrl droneCamera], sizeof(ARDroneCamera));
#ifdef DEBUG_DRONE_CAMERA
		static int numsamples = 0;
		if(numsamples++ > 64)
		{
			NSLog(@"Drone Camera Rotation : %f %f %f %f %f %f %f %f %f",
				  camera->rotation[0][0], camera->rotation[0][1], camera->rotation[0][2],
				  camera->rotation[1][0], camera->rotation[1][1], camera->rotation[1][2],
				  camera->rotation[2][0], camera->rotation[2][1], camera->rotation[2][2]);
			NSLog(@"Drone Camera Translation : %f %f %f", camera->translation[0], camera->translation[1], camera->translation[2]);
			numsamples = 0;
		}
#endif
	}
}

/**
 * Exchange enemies data.<br/>
 * Note: basically, data should be provided by the Parrot library when in multiplayer mode (enemy type = "HUMAN"), and by the game controller when in single player mode (enemy type = "AI").
 *
 * @param data Pointer to an enemies data structure.
 */
- (void)humanEnemiesData:(ARDroneEnemiesData*)data
{
	if(view != nil)
	{
		vp_os_memcpy(data, [mainviewctrl humanEnemiesData], sizeof(ARDroneEnemiesData));
#ifdef DEBUG_ENNEMIES_DETECTON
		static int old_value = 0;
		if(old_value != data->count) 
			NSLog(@"enemies detected : %d", data->count);
		old_value = data->count;
#endif
	}
}

- (void)changeState:(BOOL)inGame
{
	// Check whether there is a change of state
	if(threadStarted)
	{
		// Change the state of the library
		if(inGame)
			ardroneEngineResume();
		else
			ardroneEnginePause();

		running = inGame;
	}
	else
	{
		inGameOnDemand = inGame;
	}
	
	// Change state of view
	[mainviewctrl changeState:inGame];

}

- (void)executeCommandIn:(ARDRONE_COMMAND_IN_WITH_PARAM)commandIn fromSender:(id)sender refreshSettings:(BOOL)refresh
{
	[mainviewctrl executeCommandIn:commandIn fromSender:sender refreshSettings:refresh];	
}

- (void)executeCommandIn:(ARDRONE_COMMAND_IN)commandId withParameter:(void*)parameter fromSender:(id)sender
{
	[mainviewctrl executeCommandIn:commandId withParameter:parameter fromSender:sender];	
}

- (void)setDefaultConfigurationForKey:(ARDRONE_CONFIG_KEYS)key withValue:(void *)value
{
	[mainviewctrl setDefaultConfigurationForKey:key withValue:value];
}

- (BOOL)checkState
{
	return running;
}

@end
