/**
 *  @file OpenGLVideo.h
 *
 * Copyright 2009 Parrot SA. All rights reserved.
 * @author D HAEYER Frederic
 * @date 2009/10/26
 */
#import "OpenGLSprite.h"

@class OpenGLSprite;

@interface OpenGLVideo : OpenGLSprite {

}

- (id)initWithPath:(NSString *)path withScreenSize:(CGSize)size;
- (void)drawSelf;

@end

// Public functions definition
int get_video_current_numframes(void);
