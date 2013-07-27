//
//  ESRenderer.h
//  FreeFlight
//
//  Created by Frédéric D'HAEYER on 16/10/09.
//  Copyright Parrot SA 2009. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import "ARDrone.h"

@protocol ESRenderer <NSObject>

- (void) render:(ARDrone*)drone;
- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer;

@end
