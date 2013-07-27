#import "HUD.h"
#import <CoreMotion/CoreMotion.h>

// Ratio to activate control yaw and gaz
#define CONTROL_RATIO	(1.0 / 3.0)
#define CONTROL_RATIO_IPAD (1.0 / 6.0)

#define ACCELERO_THRESHOLD	0.1
#define ACCELERO_NORM_THRESHOLD	0.0
#define ACCELERO_FASTMOVE_THRESHOLD	1.3

// Determine if a point within the boundaries of the joystick.
static bool_t isPointInCircle(CGPoint point, CGPoint center, float radius) {
	float dx = (point.x - center.x);
	float dy = (point.y - center.y);
	return (radius >= sqrt( (dx * dx) + (dy * dy) ));
}

static float sign(float value)
{
	float result = 1.0;
	if(value < 0)
		result = -1.0;
	
	return result;
}

static float Normalize(float x, float y, float z)
{
	return sqrt(x * x + y * y + z * z);
}

static float Clamp(float v, float min, float max)
{
	float result = v;
	if(v > max)
		result = max;
	else if(v < min)
		result = min;

	return result;
}

static CONTROLS controls_table[CONTROL_MODE_MAX] = 
{
	[CONTROL_MODE1] = { {FALSE, inputPitch, inputYaw}, {FALSE, inputGaz, inputRoll} },
	[CONTROL_MODE2] = { {FALSE, inputGaz, inputYaw}, {TRUE, inputPitch, inputRoll} },
	[CONTROL_MODE3] = { {TRUE, inputPitch, inputRoll}, {FALSE, inputGaz, inputYaw} },
	[CONTROL_MODE4] = { {FALSE, inputGaz, inputRoll}, {FALSE, inputPitch, inputYaw} }
};

@interface HUD ()
ControlData *controlData;
ARDroneHUDConfiguration config;
bool_t acceleroEnabled, combinedYawEnabled;
CONTROL_MODE controlMode;
float accelero[3];
float accelero_rotation[3][3];
UIAccelerationValue lastX, lastY, lastZ;
double lowPassFilterConstant, highPassFilterConstant;
BOOL accelerometer_started;
CGPoint joystickRightCurrentPosition, joystickLeftCurrentPosition;
CGPoint joystickRightInitialPosition, joystickLeftInitialPosition;
BOOL buttonRightPressed, buttonLeftPressed;
CGPoint rightCenter, leftCenter;
float tmp_phi, tmp_theta;
BOOL screenOrientationRight;
float alpha;
SystemSoundID plop_id, batt_id;
FILE *mesures_file;
BOOL running;

BOOL compiledForIPad;
BOOL usingIPad;
BOOL using2xInterface;


#if TARGET_IPHONE_SIMULATOR == 1
NSTimer *timer;
- (void)simulate_accelerometer:(id)sender;
#else

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration;
#endif
- (void)setAcceleroRotationWithPhi:(float)phi withTheta:(float)theta withPsi:(float)psi;
- (void)refreshJoystickRight;
- (void)refreshJoystickLeft;
- (void)updateVelocity:(CGPoint)point isRight:(BOOL)isRight;
- (void)hideInfos;
- (void)showInfos;
- (void)refreshControlInterface;
@end

@implementation HUD
@synthesize firePressed;
@synthesize settingsPressed;
@synthesize mainMenuPressed;

- (id)initWithFrame:(CGRect)frame withState:(BOOL)inGame withHUDConfiguration:(ARDroneHUDConfiguration)hudconfiguration withControlData:(ControlData*)data
{
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
	
	if (usingIPad && compiledForIPad)
	{
		self = [super initWithNibName:@"HUD-iPad" bundle:nil];
	}
	else
	{
		self = [super initWithNibName:@"HUD" bundle:nil];
	}
	NSLog(@"HUD frame => w : %f, h : %f", data, frame.size.width, frame.size.height);
	if(self)
	{
		controlData = data;
		running = inGame;
		firePressed = NO;
		settingsPressed = NO;
		mainMenuPressed = NO;
		
		vp_os_memcpy(&config, &hudconfiguration, sizeof(ARDroneHUDConfiguration));
		
		acceleroEnabled = TRUE;
		combinedYawEnabled = FALSE;
		controlMode = CONTROL_MODE3;
		buttonRightPressed = buttonLeftPressed = NO;
		
		rightCenter = CGPointZero;	
		leftCenter = CGPointZero;
		lowPassFilterConstant = 0.2;
		highPassFilterConstant = (1.0 / 5.0) / ((1.0 / kAPS) + (1.0 / 5.0));
		
		joystickRightInitialPosition = CGPointZero;
		joystickRightCurrentPosition = CGPointZero;
		joystickLeftInitialPosition = CGPointZero;
		joystickLeftCurrentPosition = CGPointZero;
		
		accelero[0] = 0.0;
		accelero[1] = 0.0;
		accelero[2] = 0.0;
		
		tmp_phi = 0.0;
		tmp_theta = 0.0;
		
		[self setAcceleroRotationWithPhi:0.0 withTheta:0.0 withPsi:0.0];
		
		accelerometer_started = NO;

#if TARGET_IPHONE_SIMULATOR == 1
		[NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)(1.0 / kAPS) target:self selector:@selector(simulate_accelerometer:) userInfo:nil repeats:YES];
#else
		[[UIAccelerometer sharedAccelerometer] setUpdateInterval: (NSTimeInterval)(1.0 / kAPS)];
		[[UIAccelerometer sharedAccelerometer] setDelegate:self];
#endif		

#ifdef WRITE_DEBUG_ACCELERO	
		char filename[128];
		NSString *documentsDirectory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask, YES) objectAtIndex:0];
		sprintf(filename, "%s/accelero_iphones_mesures.txt",
				[documentsDirectory cStringUsingEncoding:NSUTF8StringEncoding]);
		mesures_file = fopen(filename, "wb");
		fprintf(mesures_file, "ARDrone\nNumSamples;AccEnable;AccXbrut;AccYbrut;AccZbrut;AccX;AccY;AccZ;PhiAngle;ThetaAngle;AlphaAngle\n");
#endif
	}

	return self;
}

- (void) setAcceleroRotationWithPhi:(float)phi withTheta:(float)theta withPsi:(float)psi
{	
	accelero_rotation[0][0] = cosf(psi)*cosf(theta);
	accelero_rotation[0][1] = -sinf(psi)*cosf(phi) + cosf(psi)*sinf(theta)*sinf(phi);
	accelero_rotation[0][2] = sinf(psi)*sinf(phi) + cosf(psi)*sinf(theta)*cosf(phi);
	accelero_rotation[1][0] = sinf(psi)*cosf(theta);
	accelero_rotation[1][1] = cosf(psi)*cosf(phi) + sinf(psi)*sinf(theta)*sinf(phi);
	accelero_rotation[1][2] = -cosf(psi)*sinf(phi) + sinf(psi)*sinf(theta)*cosf(phi);
	accelero_rotation[2][0] = -sinf(theta);
	accelero_rotation[2][1] = cosf(theta)*sinf(phi);
	accelero_rotation[2][2] = cosf(theta)*cosf(phi);

#ifdef WRITE_DEBUG_ACCELERO	
	NSLog(@"Accelero rotation matrix changed :"); 
	NSLog(@"%0.1f %0.1f %0.1f", accelero_rotation[0][0], accelero_rotation[0][1], accelero_rotation[0][2]);
	NSLog(@"%0.1f %0.1f %0.1f", accelero_rotation[1][0], accelero_rotation[1][1], accelero_rotation[1][2]);
	NSLog(@"%0.1f %0.1f %0.1f", accelero_rotation[2][0], accelero_rotation[2][1], accelero_rotation[2][2]);	
#endif
}

#if TARGET_IPHONE_SIMULATOR == 1
- (void)simulate_accelerometer:(id)sender
{
	UIAcceleration *acceleration = [[UIAcceleration alloc] init];
#else
- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{		
#endif
	static double highPassFilterX = 0.0, highPassFilterY = 0.0, highPassFilterZ = 0.0;
	static bool_t prev_accelero_enabled = TRUE;
	float norm, alpha;
	float accelerox, acceleroy, acceleroz;
	highPassFilterX = highPassFilterConstant * (highPassFilterX + acceleration.x - lastX);
	highPassFilterY = highPassFilterConstant * (highPassFilterY + acceleration.y - lastY);
	highPassFilterZ = highPassFilterConstant * (highPassFilterZ + acceleration.z - lastZ);
	lastX = acceleration.x;
	lastY = acceleration.y;
	lastZ = acceleration.z;
	
	if(running)
	{
		if(fabs(highPassFilterX) > ACCELERO_FASTMOVE_THRESHOLD || 
		   fabs(highPassFilterY) > ACCELERO_FASTMOVE_THRESHOLD || 
		   fabs(highPassFilterZ) > ACCELERO_FASTMOVE_THRESHOLD)
		{
			firePressed = YES;
		}
		else
		{
			firePressed = NO;
					
  			if(acceleroEnabled)
			{
				if(screenOrientationRight)
				{
					accelerox = acceleration.x;
					acceleroy = -acceleration.y;
				}
				else
				{
					accelerox = -acceleration.x;
					acceleroy = acceleration.y;
				}
				acceleroz = -acceleration.z;
				
				// Initialize previous values of acceleros
				if(accelerometer_started == NO)
				{
					accelerometer_started = YES;
					accelero[0] = accelerox;
					accelero[1] = acceleroy;
					accelero[2] = acceleroz;
				}		

				// Apply low pass filter on acceleros
				accelero[0] = accelerox * lowPassFilterConstant + accelero[0] * (1.0 - lowPassFilterConstant);
				accelero[1] = acceleroy * lowPassFilterConstant + accelero[1] * (1.0 - lowPassFilterConstant);
				accelero[2] = acceleroz * lowPassFilterConstant + accelero[2] * (1.0 - lowPassFilterConstant);

				// Apply rotation matrix and Clamp
				accelerox = Clamp((accelero_rotation[0][0] * accelero[0]) + (accelero_rotation[0][1] * accelero[1]) + (accelero_rotation[0][2] * accelero[2]), -1.0, 1.0);
				acceleroy = Clamp((accelero_rotation[1][0] * accelero[0]) + (accelero_rotation[1][1] * accelero[1]) + (accelero_rotation[1][2] * accelero[2]), -1.0, 1.0);
				acceleroz = Clamp((accelero_rotation[2][0] * accelero[0]) + (accelero_rotation[2][1] * accelero[1]) + (accelero_rotation[2][2] * accelero[2]), -1.0, 1.0);
				
				// compute
				if(fabs(acceleroy) < ACCELERO_THRESHOLD)
				{
					if(accelerox >= 0.0)
						alpha = M_PI_2;
					else	
						alpha = -M_PI_2;
				}
				else
				{
					alpha = atan2(accelerox, acceleroy);
				}
				
				norm = Normalize(accelerox, acceleroy, 0.0);
				if(norm > ACCELERO_NORM_THRESHOLD)
				{
					float tmp = (norm - ACCELERO_NORM_THRESHOLD) * (norm - ACCELERO_NORM_THRESHOLD);
					controlData->accelero_phi = tmp * cosf(alpha);
					controlData->accelero_theta = -tmp * sinf(alpha);
				}
				else
				{
					controlData->accelero_phi = 0.0;
					controlData->accelero_theta = 0.0;
				}
			}
			else if(prev_accelero_enabled != acceleroEnabled)
			{
				controlData->accelero_phi = 0.0;
				controlData->accelero_theta = 0.0;
			}
			
			prev_accelero_enabled = acceleroEnabled;
			sendControls();
		}
	}	
}

- (void)refreshJoystickRight
{
	CGRect frame = joystickRightBackgroundImageView.frame;
	frame.origin = joystickRightCurrentPosition;
	joystickRightBackgroundImageView.frame = frame;
}    

- (void)refreshJoystickLeft
{
	CGRect frame = joystickLeftBackgroundImageView.frame;
	frame.origin = joystickLeftCurrentPosition;
	joystickLeftBackgroundImageView.frame = frame;
}    

- (void)refreshControlInterface
{
	if(acceleroEnabled)
	{
		switch (controlMode) {
			case CONTROL_MODE2:
				joystickLeftBackgroundImageView.hidden = NO;
				joystickRightBackgroundImageView.hidden = YES;
				break;
				
			case CONTROL_MODE3:
			default:
				joystickLeftBackgroundImageView.hidden = YES;
				joystickRightBackgroundImageView.hidden = NO;
				break;
		}
	}
	else
	{
		joystickLeftBackgroundImageView.hidden = NO;
		joystickRightBackgroundImageView.hidden = NO;
	}

	[self refreshJoystickRight];
	[self refreshJoystickLeft];
}
	
- (void)changeState:(BOOL)inGame
{
	printf("%s - running : %d, inGame : %d\n", __FUNCTION__, running, inGame);
	running = inGame;
	if(!inGame)
	{
		if(buttonRightPressed)
			[joystickRightButton sendActionsForControlEvents:UIControlEventTouchCancel];
		
		if(buttonLeftPressed)
			[joystickLeftButton sendActionsForControlEvents:UIControlEventTouchCancel];
	}
}

- (void)acceleroValueChanged:(bool_t)enabled
{
	acceleroEnabled = enabled;
	[self performSelectorOnMainThread:@selector(refreshControlInterface) withObject:nil waitUntilDone:YES];
}

- (void)combinedYawValueChanged:(bool_t)enabled
{
	combinedYawEnabled = enabled;
	[self performSelectorOnMainThread:@selector(refreshControlInterface) withObject:nil waitUntilDone:YES];
}
	
- (void)interfaceAlphaValueChanged:(CGFloat)value
{
	joystickRightThumbImageView.alpha = value;
	joystickRightBackgroundImageView.alpha = value;
	joystickLeftThumbImageView.alpha = value;
	joystickLeftBackgroundImageView.alpha = value;
	alpha = value;
}
	
- (void)controlModeChanged:(CONTROL_MODE)mode
{
	controlMode = mode;
	[self performSelectorOnMainThread:@selector(refreshControlInterface) withObject:nil waitUntilDone:YES];
}

- (void)updateVelocity:(CGPoint)point isRight:(BOOL)isRight
{
	CGPoint nextpoint = CGPointMake(point.x, point.y);
	CGPoint center = (isRight ? rightCenter : leftCenter);
	UIImageView *backgroundImage = (isRight ? joystickRightBackgroundImageView : joystickLeftBackgroundImageView);
	UIImageView *thumbImage = (isRight ? joystickRightThumbImageView : joystickLeftThumbImageView);
	
	// Calculate distance and angle from the center.
	float dx = nextpoint.x - center.x;
	float dy = nextpoint.y - center.y;
	
	float distance = sqrt(dx * dx + dy * dy);
	float angle = atan2(dy, dx); // in radians
	
	// NOTE: Velocity goes from -1.0 to 1.0.
	// BE CAREFUL: don't just cap each direction at 1.0 since that
	// doesn't preserve the proportions.
	float joystick_radius = backgroundImage.frame.size.width / 2;
	if (distance > joystick_radius) {
		dx = cos(angle) * joystick_radius;
		dy = sin(angle) * joystick_radius;
	}
	
	// Constrain the thumb so that it stays within the joystick
	// boundaries.  This is smaller than the joystick radius in
	// order to account for the size of the thumb.
	float thumb_radius = thumbImage.frame.size.width / 2;
	if (distance > thumb_radius) {
		nextpoint.x = center.x + (cos(angle) * thumb_radius);
		nextpoint.y = center.y + (sin(angle) * thumb_radius);
	}
	
	// Update the thumb's position
	CGRect frame = thumbImage.frame;
	frame.origin.x = nextpoint.x - (thumbImage.frame.size.width / 2);
	frame.origin.y = nextpoint.y - (thumbImage.frame.size.height / 2);	
	thumbImage.frame = frame;
}
	
- (void)setMessageBox:(NSString*)str
{
	static int prevSound = 0;
	[messageBoxLabel performSelectorOnMainThread:@selector(setText:) withObject:str waitUntilDone:YES];
	
	struct timeval nowSound;
	gettimeofday(&nowSound, NULL);
	if (([str compare:ARDroneEngineLocalizeString(@"BATTERY LOW ALERT")] == NSOrderedSame) &&
		2 < (nowSound.tv_sec - prevSound))
	{
		AudioServicesPlaySystemSound(batt_id);
		prevSound = nowSound.tv_sec;
	}
}

- (void)setTakeOff:(NSString*)str
{
	UIImage *image = [UIImage imageNamed:[str stringByAppendingString:@".png"]];
	[takeOffButton setImage:image forState:UIControlStateNormal];
}

- (void)setEmergency:(NSString*)str
{
	if([str length] != 0)
	{
		UIImage *image = [UIImage imageNamed:[str stringByAppendingString:@".png"]];
		emergencyButton.enabled = TRUE;
		emergencyButton.alpha = 1.0;
		[emergencyButton setImage:image forState:UIControlStateNormal];
	}
	else
	{
		emergencyButton.enabled = FALSE;
		emergencyButton.alpha = 0.5;
	}
}

- (void)hideInfos
{
	batteryLevelLabel.hidden = YES;
	batteryImageView.hidden = YES;	
}

- (void)showInfos
{
	batteryLevelLabel.hidden = (config.enableBatteryPercentage == NO);
	batteryImageView.hidden = NO;
}
	
	
- (void)enableBackToMainMenu
{
	backToMainMenuButton.enabled = YES;
	backToMainMenuButton.alpha = 1.0;
}

- (void)disableBackToMainMenu
{
	backToMainMenuButton.enabled = NO;
	backToMainMenuButton.alpha = 0.0;
}
	
- (void)showBackToMainMenu:(BOOL)show
{
	if(show)
		[self performSelectorOnMainThread:@selector(enableBackToMainMenu) withObject:nil waitUntilDone:YES];
	else
		[self performSelectorOnMainThread:@selector(disableBackToMainMenu) withObject:nil waitUntilDone:YES];
}

- (void)setBattery:(int)percent
{
	if(percent < 0)
	{
		[self performSelectorOnMainThread:@selector(hideInfos) withObject:nil waitUntilDone:YES];		
	}
	else
	{
		UIImage *image = [UIImage imageNamed:[NSString stringWithFormat:@"battery%d.png", ((percent < 10) ? 0 : (int)((percent / 33.4) + 1))]];
		[batteryImageView performSelectorOnMainThread:@selector(setImage:) withObject:image waitUntilDone:YES];
		
		[self performSelectorOnMainThread:@selector(showInfos) withObject:nil waitUntilDone:YES];		
		
		[batteryLevelLabel performSelectorOnMainThread:@selector(setTextColor:) withObject:((percent < 10) ? [UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:0.7] : [UIColor colorWithRed:0.0 green:0.65 blue:0.0 alpha:0.7]) waitUntilDone:YES];		
		[batteryLevelLabel performSelectorOnMainThread:@selector(setText:) withObject:[NSString stringWithFormat:@"%d %%", percent] waitUntilDone:YES];
	}
}

- (IBAction)buttonPress:(id)sender forEvent:(UIEvent *)event 
{
	UITouch *touch = [[event touchesForView:sender] anyObject];
	CGPoint point = [touch locationInView:self.view];
	bool_t acceleroModeOk = NO;

	if(sender == joystickRightButton)
	{
		buttonRightPressed = YES;
		// Start only if the first touch is within the pad's boundaries.
		// Allow touches to be tracked outside of the pad as long as the
		// screen continues to be pressed.
		BOOL joystickIsOutside =  ((point.x + (joystickRightBackgroundImageView.frame.size.width / 2) > (joystickRightButton.frame.origin.x + joystickRightButton.frame.size.width)) ||
								   (point.x - (joystickRightBackgroundImageView.frame.size.width / 2) < joystickRightButton.frame.origin.x) ||
								   (point.y + (joystickRightBackgroundImageView.frame.size.height / 2) > (joystickRightButton.frame.origin.y + joystickRightButton.frame.size.height)) ||
								   (point.y - (joystickRightBackgroundImageView.frame.size.height / 2) < joystickRightButton.frame.origin.y));
		
		if(joystickIsOutside)
		{
			AudioServicesPlaySystemSound(plop_id);
		}
		
		joystickRightCurrentPosition.x = point.x - (joystickRightBackgroundImageView.frame.size.width / 2);
		joystickRightCurrentPosition.y = point.y - (joystickRightBackgroundImageView.frame.size.height / 2);
		
		joystickRightBackgroundImageView.alpha = joystickRightThumbImageView.alpha = 1.0;

		// Refresh Joystick
		[self refreshJoystickRight];
		
		// Update center
		rightCenter = CGPointMake(joystickRightBackgroundImageView.frame.origin.x + (joystickRightBackgroundImageView.frame.size.width / 2), joystickRightBackgroundImageView.frame.origin.y + (joystickRightBackgroundImageView.frame.size.height / 2));
	
		// Update velocity
		[self updateVelocity:rightCenter isRight:YES];
		
		acceleroModeOk = controls_table[controlMode].Right.can_use_accelero;
		
		if(combinedYawEnabled && buttonLeftPressed)
			controlData->accelero_flag |= (1 << ARDRONE_PROGRESSIVE_CMD_COMBINED_YAW_ACTIVE);
	}
	else if(sender == joystickLeftButton)
	{
		buttonLeftPressed = YES;
		
		joystickLeftCurrentPosition.x = point.x - (joystickLeftBackgroundImageView.frame.size.width / 2);
		joystickLeftCurrentPosition.y = point.y - (joystickLeftBackgroundImageView.frame.size.height / 2);
		
		joystickLeftBackgroundImageView.alpha = joystickLeftThumbImageView.alpha = 1.0;
		
		// Refresh Joystick
		[self refreshJoystickLeft];
		
		// Update center
		leftCenter = CGPointMake(joystickLeftBackgroundImageView.frame.origin.x + (joystickLeftBackgroundImageView.frame.size.width / 2), joystickLeftBackgroundImageView.frame.origin.y + (joystickLeftBackgroundImageView.frame.size.height / 2));
		
		// Update velocity
		[self updateVelocity:leftCenter isRight:NO];

		acceleroModeOk = controls_table[controlMode].Left.can_use_accelero;
		if(combinedYawEnabled && buttonRightPressed)
			controlData->accelero_flag |= (1 << ARDRONE_PROGRESSIVE_CMD_COMBINED_YAW_ACTIVE);
	}
	
	if(acceleroModeOk)
	{
		controlData->accelero_flag |= (1 << ARDRONE_PROGRESSIVE_CMD_ENABLE);
		if(acceleroEnabled)
		{
			// Start only if the first touch is within the pad's boundaries.
			// Allow touches to be tracked outside of the pad as long as the
			// screen continues to be pressed.			
			float phi, theta;
			phi = (float)atan2f(accelero[1], accelero[2]);
			theta = -(float)atan2f(accelero[0] * cosf(phi), accelero[2]);
			[self setAcceleroRotationWithPhi:phi withTheta:theta withPsi:0.0];
		}
	}			
}

- (IBAction)buttonRelease:(id)sender forEvent:(UIEvent *)event 
{
	bool_t acceleroModeOk = NO;
	if(sender == joystickRightButton)
	{
		buttonRightPressed = NO;
		
		// Reinitialize joystick position
		joystickRightCurrentPosition = joystickRightInitialPosition;
		joystickRightBackgroundImageView.alpha = joystickRightThumbImageView.alpha = alpha;
		
		// Refresh joystick
		[self refreshJoystickRight];
		
		// Update center
		rightCenter = CGPointMake(joystickRightBackgroundImageView.frame.origin.x + (joystickRightBackgroundImageView.frame.size.width / 2), joystickRightBackgroundImageView.frame.origin.y + (joystickRightBackgroundImageView.frame.size.height / 2));
		
		// reset joystick
		[self updateVelocity:rightCenter isRight:YES];
		
		controls_table[controlMode].Right.up_down(0.0);
		controls_table[controlMode].Right.left_right(0.0);

		acceleroModeOk = controls_table[controlMode].Right.can_use_accelero;
		
		if(combinedYawEnabled)
			controlData->accelero_flag &= ~(1 << ARDRONE_PROGRESSIVE_CMD_COMBINED_YAW_ACTIVE);
	}
	else if(sender == joystickLeftButton)
	{
		//printf("button left released\n");
		buttonLeftPressed = NO;
		// Reinitialize joystick position
		joystickLeftCurrentPosition = joystickLeftInitialPosition;
		joystickLeftBackgroundImageView.alpha = joystickLeftThumbImageView.alpha = alpha;
		
		// Refresh joystick
		[self refreshJoystickLeft];
		
		// Update center
		leftCenter = CGPointMake(joystickLeftBackgroundImageView.frame.origin.x + (joystickLeftBackgroundImageView.frame.size.width / 2), joystickLeftBackgroundImageView.frame.origin.y + (joystickLeftBackgroundImageView.frame.size.height / 2));
		
		// reset joystick
		[self updateVelocity:leftCenter isRight:NO];

		controls_table[controlMode].Left.up_down(0.0);
		controls_table[controlMode].Left.left_right(0.0);

		acceleroModeOk = controls_table[controlMode].Left.can_use_accelero;

		if(combinedYawEnabled)
			controlData->accelero_flag &= ~(1 << ARDRONE_PROGRESSIVE_CMD_COMBINED_YAW_ACTIVE);
	}
	
	if(acceleroModeOk)
	{
		controlData->accelero_flag &= ~(1 << ARDRONE_PROGRESSIVE_CMD_ENABLE);
		if(acceleroEnabled)
		{
			[self setAcceleroRotationWithPhi:0.0 withTheta:0.0 withPsi:0.0];
		}
	}
}

- (IBAction)buttonDrag:(id)sender forEvent:(UIEvent *)event 
{
	UITouch *touch = [[event touchesForView:sender] anyObject];
	CGPoint point = [touch locationInView:self.view];
	bool_t acceleroModeOk = NO;
	
	
	float controlRatio = 0.5 - (CONTROL_RATIO / 2.0);
	if (usingIPad && compiledForIPad)
	{
		controlRatio = 0.5 - (CONTROL_RATIO_IPAD / 2.0);
	}
	
	
	
	if(sender == joystickRightButton)
	{
		if((rightCenter.x - point.x) > ((joystickRightBackgroundImageView.frame.size.width / 2) - (controlRatio * joystickRightBackgroundImageView.frame.size.width)))
		{
			float percent = ((rightCenter.x - point.x) - ((joystickRightBackgroundImageView.frame.size.width / 2) - (controlRatio * joystickRightBackgroundImageView.frame.size.width))) / ((controlRatio * joystickRightBackgroundImageView.frame.size.width));
			//NSLog(@"Percent (left)  : %f\n", percent);
			if(percent > 1.0)
				percent = 1.0;
			controls_table[controlMode].Right.left_right(-percent);
		}
		else if((point.x - rightCenter.x) > ((joystickRightBackgroundImageView.frame.size.width / 2) - (controlRatio * joystickRightBackgroundImageView.frame.size.width)))
		{
			float percent = ((point.x - rightCenter.x) - ((joystickRightBackgroundImageView.frame.size.width / 2) - (controlRatio * joystickRightBackgroundImageView.frame.size.width))) / ((controlRatio * joystickRightBackgroundImageView.frame.size.width));
			//NSLog(@"Percent (right) : %f\n", percent);
			if(percent > 1.0)
				percent = 1.0;
			controls_table[controlMode].Right.left_right(percent);
		}
		else
		{
			controls_table[controlMode].Right.left_right(0.0);
		}

		if((point.y - rightCenter.y) > ((joystickRightBackgroundImageView.frame.size.height / 2) - (controlRatio * joystickRightBackgroundImageView.frame.size.height)))
		{
			float percent = ((point.y - rightCenter.y) - ((joystickRightBackgroundImageView.frame.size.height / 2) - (controlRatio * joystickRightBackgroundImageView.frame.size.height))) / ((controlRatio * joystickRightBackgroundImageView.frame.size.height));
			//NSLog(@"Percent (down)  : %f\n", percent);
			if(percent > 1.0)
				percent = 1.0;
			controls_table[controlMode].Right.up_down(-percent);
		}
		else if((rightCenter.y - point.y) > ((joystickRightBackgroundImageView.frame.size.height / 2) - (controlRatio * joystickRightBackgroundImageView.frame.size.height)))
		{
			float percent = ((rightCenter.y - point.y) - ((joystickRightBackgroundImageView.frame.size.height / 2) - (controlRatio * joystickRightBackgroundImageView.frame.size.height))) / ((controlRatio * joystickRightBackgroundImageView.frame.size.height));
			//NSLog(@"Percent (top)   : %f\n", percent);
			if(percent > 1.0)
				percent = 1.0;
			controls_table[controlMode].Right.up_down(percent);
		}
		else
		{
			controls_table[controlMode].Right.up_down(0.0);
		}

		acceleroModeOk = controls_table[controlMode].Right.can_use_accelero;
	}
	else if(sender == joystickLeftButton)
	{
		if((leftCenter.x - point.x) > ((joystickLeftBackgroundImageView.frame.size.width / 2) - (controlRatio * joystickLeftBackgroundImageView.frame.size.width)))
		{
			float percent = ((leftCenter.x - point.x) - ((joystickLeftBackgroundImageView.frame.size.width / 2) - (controlRatio * joystickLeftBackgroundImageView.frame.size.width))) / ((controlRatio * joystickLeftBackgroundImageView.frame.size.width));
			if(percent > 1.0)
				percent = 1.0;
			controls_table[controlMode].Left.left_right(-percent);
		}
		else if((point.x - leftCenter.x) > ((joystickLeftBackgroundImageView.frame.size.width / 2) - (controlRatio * joystickLeftBackgroundImageView.frame.size.width)))
		{
			float percent = ((point.x - leftCenter.x) - ((joystickLeftBackgroundImageView.frame.size.width / 2) - (controlRatio * joystickLeftBackgroundImageView.frame.size.width))) / ((controlRatio * joystickLeftBackgroundImageView.frame.size.width));
			if(percent > 1.0)
				percent = 1.0;
			controls_table[controlMode].Left.left_right(percent);
		}
		else
		{
			controls_table[controlMode].Left.left_right(0.0);
		}	
		
		if((point.y - leftCenter.y) > ((joystickLeftBackgroundImageView.frame.size.height / 2) - (controlRatio * joystickLeftBackgroundImageView.frame.size.height)))
		{
			float percent = ((point.y - leftCenter.y) - ((joystickLeftBackgroundImageView.frame.size.height / 2) - (controlRatio * joystickLeftBackgroundImageView.frame.size.height))) / ((controlRatio * joystickLeftBackgroundImageView.frame.size.height));
			if(percent > 1.0)
				percent = 1.0;
			controls_table[controlMode].Left.up_down(-percent);
		}
		else if((leftCenter.y - point.y) > ((joystickLeftBackgroundImageView.frame.size.height / 2) - (controlRatio * joystickLeftBackgroundImageView.frame.size.height)))
		{
			float percent = ((leftCenter.y - point.y) - ((joystickLeftBackgroundImageView.frame.size.height / 2) - (controlRatio * joystickLeftBackgroundImageView.frame.size.height))) / ((controlRatio * joystickLeftBackgroundImageView.frame.size.height));
			if(percent > 1.0)
				percent = 1.0;
			controls_table[controlMode].Left.up_down(percent);
		}
		else
		{
			controls_table[controlMode].Left.up_down(0.0);
		}		

		acceleroModeOk = controls_table[controlMode].Left.can_use_accelero;
	}
		
	if(acceleroModeOk)
	{
		if(!acceleroEnabled)
		{
			// Update joystick velocity
			[self updateVelocity:point isRight:(sender == joystickRightButton)];
		}
	}
	else
	{
		// Update joystick velocity
		[self updateVelocity:point isRight:(sender == joystickRightButton)];
	}

}

- (IBAction)buttonClick:(id)sender forEvent:(UIEvent *)event 
{
	static ARDRONE_VIDEO_CHANNEL channel = ARDRONE_VIDEO_CHANNEL_FIRST;
	if(sender == settingsButton)
		settingsPressed = YES;
	else if(sender == backToMainMenuButton)
		mainMenuPressed = YES;
	else if(sender == switchScreenButton)
	{
		if(channel++ == ARDRONE_VIDEO_CHANNEL_LAST)
			channel = ARDRONE_VIDEO_CHANNEL_FIRST;
		
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(video_channel, (int32_t*)&channel, NULL);
	}
	else if(sender == takeOffButton)
	{
		switchTakeOff();
	}
	else if(sender == emergencyButton)
	{
		emergency();
	}
}

- (void)viewDidLoad
{
	switchScreenButton.hidden = (config.enableSwitchScreen == NO);
	backToMainMenuButton.hidden = (config.enableBackToMainMenu == NO);
	batteryLevelLabel.hidden = (config.enableBatteryPercentage == NO);
	
	rightCenter = CGPointMake(joystickRightThumbImageView.frame.origin.x + (joystickRightThumbImageView.frame.size.width / 2), joystickRightThumbImageView.frame.origin.y + (joystickRightThumbImageView.frame.size.height / 2));
	joystickRightInitialPosition = CGPointMake(rightCenter.x - (joystickRightBackgroundImageView.frame.size.width / 2), rightCenter.y - (joystickRightBackgroundImageView.frame.size.height / 2));
	leftCenter = CGPointMake(joystickLeftThumbImageView.frame.origin.x + (joystickLeftThumbImageView.frame.size.width / 2), joystickLeftThumbImageView.frame.origin.y + (joystickLeftThumbImageView.frame.size.height / 2));
	joystickLeftInitialPosition = CGPointMake(leftCenter.x - (joystickLeftBackgroundImageView.frame.size.width / 2), leftCenter.y - (joystickLeftBackgroundImageView.frame.size.height / 2));

	joystickLeftCurrentPosition = joystickLeftInitialPosition;
	joystickRightCurrentPosition = joystickRightInitialPosition;
	
	alpha = MIN(joystickRightBackgroundImageView.alpha, joystickRightThumbImageView.alpha);
	joystickRightBackgroundImageView.alpha = joystickRightThumbImageView.alpha = alpha;
	joystickLeftBackgroundImageView.alpha = joystickLeftThumbImageView.alpha = alpha;

    // Get the URL to the sound file to play
	CFURLRef batt_url  = CFBundleCopyResourceURL (CFBundleGetMainBundle(),
												   CFSTR ("battery"),
												   CFSTR ("wav"),
												  NULL);
    CFURLRef plop_url  = CFBundleCopyResourceURL (CFBundleGetMainBundle(),
												   CFSTR ("plop"),
												   CFSTR ("wav"),
												   NULL);
	
    // Create a system sound object representing the sound file
    AudioServicesCreateSystemSoundID (plop_url, &plop_id);
    AudioServicesCreateSystemSoundID (batt_url, &batt_id);
	CFRelease(plop_url);
	CFRelease(batt_url);
	
	[self setBattery:-1];
}

- (void) dealloc 
{
#ifdef WRITE_DEBUG_ACCELERO
	fclose(mesures_file);
#endif
	AudioServicesDisposeSystemSoundID (plop_id);
	AudioServicesDisposeSystemSoundID (batt_id);
	[super dealloc];
}

@end
