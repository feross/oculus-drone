/**
 *  @file OpenGLSprite.h
 *
 * Copyright 2009 Parrot SA. All rights reserved.
 * @author D HAEYER Frederic
 * @date 2009/10/26
 */
#include "ConstantsAndMacros.h"

typedef enum
{
	NO_SCALING,
	FIT_X,
	FIT_Y,
	FIT_XY
} eSCALING;

typedef struct
{
	GLuint widthImage;
	GLuint widthTexture;
	GLuint heightImage;
	GLuint heightTexture;
	GLfloat scaleModelX;
	GLfloat scaleModelY;
	GLfloat scaleTextureX;
	GLfloat scaleTextureY;
	GLuint bytesPerPixel;
	GLenum format;
	GLenum type;
	void* data;
	GLuint identifier;
	enum {INVALID, GENERATED, SENT_TO_GPU} state;
} Texture;

@interface OpenGLSprite : NSObject {
@private
	CGSize		screenSize;
	Vertex3D	position;
	Rotation3D	rotation;
	Scale3D		scale;
	CGSize		oldSize;
	unsigned int oldBytesPerPixel;
	
@protected
	UIImage*	image;
	void		*default_image;
	eSCALING	scaling;
	Texture		texture;
}
@property (nonatomic, assign) Vertex3D	position;
@property (nonatomic, assign) Rotation3D rotation;
@property (nonatomic, assign) Scale3D	scale;

- (id)initWithPath:(NSString *)path withScaling:(eSCALING)_scaling withScreenSize:(CGSize)size;
- (BOOL)ResetTexture;
- (void)drawSelfIfNeeded:(BOOL)needed;

@end
