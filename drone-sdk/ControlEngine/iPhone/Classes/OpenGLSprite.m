/**
 *  @file OpenGLSprite.m
 *
 * Copyright 2009 Parrot SA. All rights reserved.
 * @author D HAEYER Frederic
 * @date 2009/10/26
 */
#import "OpenGLSprite.h"

#ifdef _cplusplus
extern "C" {
#endif
	unsigned long RoundPower2(unsigned long x);
	void BindTexture(Texture* texture);
#ifdef _cplusplus
}
#endif		

static GLfloat const vertices[] =
{
	1.0f, -1.0f,
	-1.0f, -1.0f,
	1.0f,  1.0f,
	-1.0f,  1.0f,
};

static GLfloat const coordinates[] =
{
	0.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
};

unsigned long RoundPower2(unsigned long x)
{
	int rval=512;
	// rval<<=1 Is A Prettier Way Of Writing rval*=2; 
	while(rval < x)
		rval <<= 1;
	return rval;
}

void BindTexture(Texture* texture)
{
	// Check whether the texture has already been generated
	if(texture->state == INVALID)
	{
		// Generated the texture
		// Note: this must NOT be done before the initialization of OpenGL (this is why it can NOT be done in "InitializeTexture")
		glGenTextures(1, &texture->identifier);
		printf("Video texture identifier : %d\n", texture->identifier);
		texture->state = GENERATED;
	}
	
	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, texture->identifier);
	
	// Check whether the texture data have already been sent to OpenGL
	if(texture->state == GENERATED)
	{
		// Load the texture in the GPU
		glTexImage2D(GL_TEXTURE_2D, 0, texture->format, texture->widthTexture, texture->heightTexture, 0, texture->format, texture->type, texture->data);
		texture->state = SENT_TO_GPU;
	}
}

void InitTexture(Texture *texture, CGSize imageSize, CGSize screenSize, eSCALING scaling)
{
	int max_size = max(RoundPower2(imageSize.width), RoundPower2(imageSize.height)); 
	// Define the texture format
	texture->widthImage = imageSize.width;
	texture->heightImage = imageSize.height;
	texture->widthTexture = max_size;
	texture->heightTexture = max_size;
	NSLog(@"%s sizes %d, %d, %d, %d, %@\n", __FUNCTION__, texture->widthImage, texture->heightImage, texture->widthTexture, texture->heightTexture, NSStringFromCGSize(screenSize));
	if(texture->bytesPerPixel == 2)
	{
		texture->format = GL_RGB;
		texture->type = GL_UNSIGNED_SHORT_5_6_5;
	}
	else
	{
		texture->format = GL_RGBA;
		texture->type = GL_UNSIGNED_BYTE;
	}

	switch(scaling)
	{
		case NO_SCALING:
			texture->scaleModelX = texture->heightImage / screenSize.width;
			texture->scaleModelY = texture->widthImage / screenSize.height;
			break;
		case FIT_X:
			texture->scaleModelX = (screenSize.height * texture->heightImage) / (screenSize.width * texture->widthImage);
			texture->scaleModelY = 1.0f;
			break;
		case FIT_Y:
			texture->scaleModelX = 1.0f;
			texture->scaleModelY = (screenSize.width * texture->widthImage) / (screenSize.height * texture->heightImage);
			break;
		default:
			texture->scaleModelX = 1.0f;
			texture->scaleModelY = 1.0f;
			break;
	}
	texture->scaleTextureX = texture->widthImage / (float)texture->widthTexture;
	texture->scaleTextureY = texture->heightImage / (float)texture->heightTexture;
}

@implementation OpenGLSprite
@synthesize position;
@synthesize rotation;
@synthesize scale;

- (id)initWithPath:(NSString *)path withScaling:(eSCALING)_scaling withScreenSize:(CGSize)size
{
	if ((self = [super init]))
	{
		// Load the image
		image = [UIImage imageWithContentsOfFile:path];
		
		// ScreenSize
		screenSize = size;
		scaling = _scaling;
		
		// Init rotation, translation and scale
		rotation.x = 0.0;
		rotation.y = 0.0;
		rotation.z = 0.0;
		
		position.x = 0.0;
		position.y = 0.0;
		position.z = 0.0;
		
		scale.x = 1.0;
		scale.y = 1.0;
		scale.z = 1.0;
		
		texture.bytesPerPixel = 4;
		texture.format = GL_RGBA;
		texture.type = GL_UNSIGNED_BYTE;
		texture.state = INVALID;
		InitTexture(&texture, image.size, screenSize, scaling);

		NSLog(@"Default Image size : %@", NSStringFromCGSize(image.size));
		oldSize = CGSizeMake(image.size.width, image.size.height);
		oldBytesPerPixel = texture.bytesPerPixel;
		
		// Allocate memory to store the texture data
		default_image = vp_os_malloc(texture.widthTexture * texture.heightTexture * texture.bytesPerPixel);
		texture.data = default_image;
		
		CGContextRef context = CGBitmapContextCreate(texture.data, texture.widthTexture, texture.heightTexture, 8, texture.widthTexture * texture.bytesPerPixel, CGImageGetColorSpace(image.CGImage), kCGImageAlphaNoneSkipLast);
		
		// Draw the image into the texture
		CGContextDrawImage(context, CGRectMake(0.0f, texture.heightTexture - texture.heightImage, texture.widthImage, texture.heightImage), image.CGImage);
		
		// Release the context (but keep the texture data buffer to make it possible modifying the texture in real-time)
		CGContextRelease(context);
	}

	return self;
}

- (BOOL)ResetTexture
{
	BOOL result = NO;
	if ((oldSize.width != texture.widthImage) || (oldSize.height != texture.heightImage) || (oldBytesPerPixel != texture.bytesPerPixel)) 
	{
		NSLog(@"%s oldSize : %d, %d - texture : %d, %d, oldBytesPerPixel : %d", __FUNCTION__, (int)oldSize.width, (int)oldSize.height, (int)texture.widthImage, (int)texture.heightImage, oldBytesPerPixel);
		CGSize current_size = CGSizeMake(texture.widthImage, texture.heightImage); 
		InitTexture(&texture, current_size, screenSize, scaling);
		texture.state = GENERATED;
		oldSize = current_size;
		oldBytesPerPixel = texture.bytesPerPixel;
		result = YES;
	}
	
	return result;
}

- (void)drawSelfIfNeeded:(BOOL)needed
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(position.x, position.y, position.z);
	
	[self ResetTexture];
	
	// Scale Texture
	glScalef(texture.scaleModelX * scale.x, texture.scaleModelY * scale.y, scale.z);
	
	glRotatef(rotation.x, 1.0, 0.0, 0.0);
	glRotatef(rotation.y, 0.0, 1.0, 0.0);
	glRotatef(rotation.z, 0.0, 0.0, 1.0);
	
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef(texture.scaleTextureX, texture.scaleTextureY, 1.0f);
	
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, coordinates);

	// Bind the background texture if needed, resend to gpu
	if(needed)
		texture.state = GENERATED;
	
	BindTexture(&texture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// Draw the background quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

- (void)dealloc
{
	// Remove the texture from the GPU
	if(texture.state != INVALID)
	{
		glDeleteTextures(1, &texture.identifier);
	}
	
	// Free the memory allocated to store the texture data
	vp_os_free(default_image);
	[super dealloc];
}
@end
