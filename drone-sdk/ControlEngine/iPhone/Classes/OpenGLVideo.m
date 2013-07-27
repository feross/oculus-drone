/**
 *  @file OpenGLVideo.m
 *
 * Copyright 2009 Parrot SA. All rights reserved.
 * @author D HAEYER Frederic
 * @date 2009/10/26
 */
#import "OpenGLVideo.h"

static int32_t current_num_picture_decoded = 0;

int get_video_current_numframes( void )
{
	return current_num_picture_decoded;
}

@implementation OpenGLVideo

- (id)initWithPath:(NSString *)path withScreenSize:(CGSize)size
{
	if((self = [super initWithPath:path withScaling:FIT_XY withScreenSize:size]) != nil)
	{
		
	}
	
	return self;
}

- (void)drawSelf
{
	BOOL needToUpdate = NO;
	video_stage_config_t *config = video_stage_get();
	
	if ((config != NULL) && (config->data != NULL) && (config->num_picture_decoded > 0))
	{
		vp_os_mutex_lock( &config->mutex );
		
		texture.bytesPerPixel	= config->bytesPerPixel;
		texture.widthImage		= config->widthImage;
		texture.heightImage		= config->heightImage;
		texture.data            = config->data; 
		
		if (config->num_picture_decoded > current_num_picture_decoded)
		{
			needToUpdate = YES;
		}
		
		current_num_picture_decoded = config->num_picture_decoded;
		
		vp_os_mutex_unlock( &config->mutex );
	}

	[super drawSelfIfNeeded:needToUpdate];		
}

@end
