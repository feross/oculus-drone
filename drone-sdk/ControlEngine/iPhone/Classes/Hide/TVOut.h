//
//  TVOut.h
//  ARDroneEngine
//
//  Created by Frédéric D'HAEYER on 22/12/09.
//  Copyright 2009 Parrot SA. All rights reserved.
//
#include "ConstantsAndMacros.h"

#ifdef ENABLE_AUTO_TVOUT
#import <MediaPlayer/MediaPlayer.h>

extern NSString * const UIApplicationDidSetupScreenMirroringNotification;
extern NSString * const UIApplicationDidDisableScreenMirroringNotification;

@interface TVOut : NSObject

- (BOOL) isScreenMirroringActive;
- (UIScreen *) currentMirroringScreen;
- (void) setupScreenMirroring;
- (void) setupScreenMirroringWithFramesPerSecond:(double)fps;
- (void) disableScreenMirroring;

@end

#endif // ENABLE_AUTO_TVOUT
