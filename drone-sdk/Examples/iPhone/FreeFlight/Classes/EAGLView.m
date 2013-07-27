//
//  EAGLView.m
//  FreeFlight
//
//  Created by Frédéric D'HAEYER on 16/10/09.
//  Copyright Parrot SA 2009. All rights reserved.
//
#import "EAGLView.h"
#import "ES1Renderer.h"

@implementation EAGLView
@synthesize animating;
@dynamic animationFrameInterval;

// You must implement this method
+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
//- (id) initWithCoder:(NSCoder*)coder
//{
//    if ((self = [super initWithCoder:coder]))
- (id)initWithFrame:(CGRect)frame
{
	if ((self = [super initWithFrame: frame]))
	{
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

        eaglLayer.opaque = NO;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		drone = nil;
		renderer = [[ES1Renderer alloc] init];
		
		if (!renderer)
		{
			[self release];
			return nil;
		}
        
		animating = FALSE;
		animationFrameInterval = 2; // is 2 * (1 / 60) = 1 / 30 <=> 30 fps 
		animationTimer = nil;
	}
	
    return self;
}

- (void) setDrone:(ARDrone*)_drone
{
	drone = _drone;
}

- (void) drawView
{
	[renderer render:drone];
}

- (void) layoutSubviews
{
	[renderer resizeFromLayer:(CAEAGLLayer*)self.layer];
    [self drawView];
}

- (NSInteger) animationFrameInterval
{
	return animationFrameInterval;
}

- (void)changeState:(BOOL)inGame
{
	if(inGame)
	{
		self.hidden = NO;
		if(!animating)
		{
			animationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)((1.0 / 60.0) * animationFrameInterval) target:self selector:@selector(drawView) userInfo:nil repeats:TRUE];
			animating = TRUE;
		}
	}
	else
	{
		self.hidden = YES;
		if (animating)
		{
			[animationTimer invalidate];
			animationTimer = nil;
			animating = FALSE;
		}
	}
}

- (void) dealloc
{
    [renderer release];
	
    [super dealloc];
}

@end
