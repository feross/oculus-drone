/**
 *  @file GLViewController.h
 *
 * Copyright 2009 Parrot SA. All rights reserved.
 * @author D HAEYER Frederic
 * @date 2009/10/26
 */
#include "ConstantsAndMacros.h"

#import "OpenGLVideo.h"
#import "OpenGLSprite.h"
#import "ARDrone.h"
#import "InternalProtocols.h"

@class OpenGLVideo;

@interface GLViewController : NSObject {
@private
	OpenGLVideo   *video;
	BOOL screenOrientationRight;
	id delegate;
}

- (id)initWithFrame:(CGRect)frame withDelegate:(id<NavdataProtocol>)delegate;
- (void)setScreenOrientationRight:(BOOL)right;
- (void)drawView;
@end
