//
//  AppDelegate.m
//  FreeFlight
//
//  Created by Frédéric D'HAEYER on 16/10/09.
//  Copyright Parrot SA 2009. All rights reserved.
//
#import "AppDelegate.h"
#import "EAGLView.h"
#import "MenuUpdater.h"
#import "ES1Renderer.h"

#define SKIP_BUTTON_X_LANDSCAPE_LEFT .9f
#define SKIP_BUTTON_Y_LANDSCAPE_LEFT 0.f

#define SKIP_BUTTON_X_LANDSCAPE_RIGHT 0.f
#define SKIP_BUTTON_Y_LANDSCAPE_RIGHT .7f

#define SKIP_BUTTON_WIDTH .1f
#define SKIP_BUTTON_HEIGHT .3f

#define SKIP_BUTTON_TEXT NSLocalizedString(@"SKIP",)

@interface Movie : MPMoviePlayerViewController
{
@public
	id delegate;
	SEL rotateToInterfaceOrientationCallback;
}

@end

@implementation Movie

- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	switch (toInterfaceOrientation)
	{
		case UIInterfaceOrientationPortrait:
		case UIInterfaceOrientationPortraitUpsideDown: 
			return NO;
		default:
			[delegate performSelector:rotateToInterfaceOrientationCallback withObject:[NSNumber numberWithUnsignedInt:toInterfaceOrientation]];
			return YES;
	}
}

@end


@implementation AppDelegate
@synthesize window;
@synthesize menuController;

- (void) applicationDidFinishLaunching:(UIApplication *)application
{
	NSLog(@"app did finish launching");
	application.idleTimerDisabled = YES;
	
	window.backgroundColor = [UIColor blackColor];
	
#if	(!TARGET_CPU_X86) && (!DEBUG)
	// Display the movie:
	movie = [[Movie alloc] initWithContentURL:[NSURL URLWithString:[[NSBundle mainBundle] pathForResource:@"ARFreeFlight" ofType:@"mp4"]]];
	
	((Movie *)movie)->delegate = self;
	((Movie *)movie)->rotateToInterfaceOrientationCallback = @selector(rotateToInterfaceOrientation:);
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(initGame) name:MPMoviePlayerPlaybackDidFinishNotification object:movie.moviePlayer];
	
	movie.moviePlayer.shouldAutoplay = NO;
	movie.moviePlayer.controlStyle = MPMovieControlStyleNone;
	
	[window addSubview:[movie view]];
	[[movie moviePlayer] play];
	
	CGRect frame = [window frame];
	frame.origin.x = SKIP_BUTTON_X_LANDSCAPE_RIGHT*frame.size.width;
	frame.origin.y = SKIP_BUTTON_Y_LANDSCAPE_RIGHT*frame.size.height;
	
	frame.size.width *= SKIP_BUTTON_WIDTH;
	frame.size.height *= SKIP_BUTTON_HEIGHT;
	
	skipMovie = [UIButton buttonWithType:UIButtonTypeCustom];
	
	[skipMovie setTransform:CGAffineTransformMakeRotation(.5f*M_PI)];
	[skipMovie setFrame:frame];
	[skipMovie setTitle:SKIP_BUTTON_TEXT forState:UIControlStateNormal];
	[skipMovie setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
	
	skipMovie.titleLabel.font = [UIFont boldSystemFontOfSize:20.f];
	
	[skipMovie addTarget:movie.moviePlayer action:@selector(stop) forControlEvents:UIControlEventTouchDown];
	
	[window addSubview:skipMovie];
	[window makeKeyAndVisible];
#else
	[window makeKeyAndVisible];
	[self initGame];
#endif
	
}

- (void) rotateToInterfaceOrientation:(id)toInterfaceOrientation
{
	UIInterfaceOrientation orientation = [(NSNumber *)toInterfaceOrientation unsignedIntValue];
	
	CGRect frame = [window frame];
	
	switch (orientation)
	{
		case UIInterfaceOrientationLandscapeLeft:
			frame.origin.x = SKIP_BUTTON_X_LANDSCAPE_LEFT*frame.size.width;
			frame.origin.y = SKIP_BUTTON_Y_LANDSCAPE_LEFT*frame.size.height;
			
			frame.size.width *= SKIP_BUTTON_WIDTH;
			frame.size.height *= SKIP_BUTTON_HEIGHT;
			
			[skipMovie setTransform:CGAffineTransformMakeRotation(-.5f*M_PI)];
			break;
		case UIInterfaceOrientationLandscapeRight:
			frame.origin.x = SKIP_BUTTON_X_LANDSCAPE_RIGHT*frame.size.width;
			frame.origin.y = SKIP_BUTTON_Y_LANDSCAPE_RIGHT*frame.size.height;
			
			frame.size.width *= SKIP_BUTTON_WIDTH;
			frame.size.height *= SKIP_BUTTON_HEIGHT;
			
			[skipMovie setTransform:CGAffineTransformMakeRotation(.5f*M_PI)];
			break;
		default:
			break;
	}
	
	[skipMovie setFrame:frame];
}

- (void) initGame
{
	[[NSNotificationCenter defaultCenter] removeObserver:self name:MPMoviePlayerPlaybackDidFinishNotification object:movie.moviePlayer];
	
	[skipMovie removeTarget:movie.moviePlayer action:@selector(stop) forControlEvents:UIControlEventTouchDown];
	[skipMovie removeFromSuperview];
	skipMovie = nil;
	
	[[movie view] removeFromSuperview];
	[[movie moviePlayer] stop];
	[movie release];
	movie = nil;
	
	// Setup the menu controller
	menuController.delegate = self;
	NSLog(@"menu controller view %@", menuController.view);
	
	was_in_game = NO;
	
	// Setup the ARDrone
	ARDroneHUDConfiguration hudconfiguration = {YES, NO, YES};
	drone = [[ARDrone alloc] initWithFrame:menuController.view.frame withState:was_in_game withDelegate:menuController withHUDConfiguration:&hudconfiguration];
	
	// Setup the OpenGL view
	glView = [[EAGLView alloc] initWithFrame:menuController.view.frame];
	[glView setDrone:drone];
	[glView changeState:was_in_game];
	
	CGRect frame = drone.view.frame;
	
	if (frame.size.width < frame.size.height)
	{
		NSLog(@"Here, the ARDrone frame isn't initialized");
		frame = CGRectMake(frame.origin.x, frame.origin.y, frame.size.height, frame.size.width);
		[drone.view setFrame:frame];
	}

	[menuController setDrone:drone];
	[menuController changeState:was_in_game];

	[window addSubview:menuController.view];
	[window addSubview:glView];
	[window bringSubviewToFront:menuController.view];
 	[window makeKeyAndVisible];
}

#pragma mark -
#pragma mark Drone protocol implementation
- (void)changeState:(BOOL)inGame
{
	was_in_game = inGame;
	
	if(inGame)
	{
		int value;
		[drone setScreenOrientationRight:(menuController.interfaceOrientation == UIInterfaceOrientationLandscapeRight)];
#ifdef DISPLAY_DART
		value = ARDRONE_VIDEO_CHANNEL_HORI;
		[drone setDefaultConfigurationForKey:ARDRONE_CONFIG_KEY_VIDEO_CHANNEL withValue:&value];
		
		value = ARDRONE_CAMERA_DETECTION_H_ORIENTED_COCARDE;
#else
		value = ARDRONE_CAMERA_DETECTION_NONE;
#endif
		[drone setDefaultConfigurationForKey:ARDRONE_CONFIG_KEY_DETECT_TYPE withValue:&value];
		
		value = 0;
		[drone setDefaultConfigurationForKey:ARDRONE_CONFIG_KEY_CONTROL_LEVEL withValue:&value];
		
	}
	
	[drone changeState:inGame];
	[glView changeState:inGame];
}

- (void) applicationWillResignActive:(UIApplication *)application
{
	// Become inactive
	if(was_in_game)
	{
		[drone changeState:NO];
		[glView changeState:NO];
	}
	else
	{
		[menuController changeState:NO];
	}
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
	if(was_in_game)
	{
		[drone changeState:YES];
		[glView changeState:YES];
	}
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	printf("%s : %d\n", __FUNCTION__, was_in_game);
	
	if(was_in_game)
	{
		[drone changeState:NO];
		[glView changeState:NO];
	}
	else
	{
		[menuController changeState:NO];
	}
}

- (void)executeCommandIn:(ARDRONE_COMMAND_IN_WITH_PARAM)commandIn fromSender:(id)sender refreshSettings:(BOOL)refresh
{
	
}

- (void)executeCommandIn:(ARDRONE_COMMAND_IN)commandId withParameter:(void*)parameter fromSender:(id)sender
{
	
}

- (void)setDefaultConfigurationForKey:(ARDRONE_CONFIG_KEYS)key withValue:(void *)value
{
	
}

- (BOOL)checkState
{
	BOOL result = NO;
	
	if(was_in_game)
	{
		result = [drone checkState];
	}
	else
	{
		//result = [menuController checkState];
	}
	
	return result;
}

@end
