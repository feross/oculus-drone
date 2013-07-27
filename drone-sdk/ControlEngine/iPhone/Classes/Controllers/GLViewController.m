/**
 *  @file GLViewController.m
 *
 * Copyright 2009 Parrot SA. All rights reserved.
 * @author D HAEYER Frederic
 * @date 2009/10/26
 */
#include "ConstantsAndMacros.h"
#import "GLViewController.h"

typedef struct
{
	GLint depthFunc;
	GLboolean depthWriteMask;
	GLint textureBinding2D;
	GLint textureEnvMode;
	GLint blendSrc;
	GLint blendDst;
	GLint clientActiveTexture;
	GLboolean depthTest;
	GLboolean cullFace;
	GLboolean texture2D;
	GLboolean blend;
	GLboolean lighting;
	GLboolean vertexArray;
	GLboolean textureCoordArray;
	GLboolean colorArray;
	GLboolean normalArray;
	GLfloat linewidth;
} OpenGLContext;

static CGFloat const matrixOrthoFront[] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f};
static CGFloat const matrixOrthoBack[] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

static CGFloat const matrixOrthoBackLeft[] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
static CGFloat const matrixOrthoBackRight[] = {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f};


float normalize_vector(float x, float y, float z)
{
	return sqrtf((x * x) + (y  * y) + (z * z));
}

@interface GLViewController ()
	OpenGLContext openGLContext;
	id<NavdataProtocol> navdata_delegate;

- (void)saveOpenGLContext;
- (void)restoreOpenGLContext;
@end

@implementation GLViewController

- (id)initWithFrame:(CGRect)frame withDelegate:(id<NavdataProtocol>)_navdata_delegate
{
	if((self = [super init]) != nil)
	{
		NSLog(@"Frame : %f, %f", frame.size.width, frame.size.height);
		
		navdata_delegate = _navdata_delegate;
		
		if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
		{
			video = [[OpenGLVideo alloc] initWithPath:[[NSBundle mainBundle] pathForResource:@"background-iPad" ofType:@"png"] withScreenSize:frame.size];
		}
		else
		{
			video = [[OpenGLVideo alloc] initWithPath:[[NSBundle mainBundle] pathForResource:@"background" ofType:@"png"] withScreenSize:frame.size];
		}
	}
	
	return self;
}

- (void)saveOpenGLContext
{
	// Make sure the client active texture is the same as the server active texture
	GLint activeTexture;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);	
	glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE, &openGLContext.clientActiveTexture);
	glClientActiveTexture(activeTexture);
	
	// Save OpenGL parameters that have been modified
	glGetIntegerv(GL_DEPTH_FUNC, &openGLContext.depthFunc);
	glGetBooleanv(GL_DEPTH_WRITEMASK, &openGLContext.depthWriteMask);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &openGLContext.textureBinding2D);
	glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &openGLContext.textureEnvMode);
	glGetIntegerv(GL_BLEND_SRC, &openGLContext.blendSrc);
	glGetIntegerv(GL_BLEND_DST, &openGLContext.blendDst);
	glGetFloatv(GL_LINE_WIDTH, &openGLContext.linewidth);
	
	openGLContext.depthTest = glIsEnabled(GL_DEPTH_TEST);
	openGLContext.cullFace = glIsEnabled(GL_CULL_FACE);
	openGLContext.texture2D = glIsEnabled(GL_TEXTURE_2D);
	openGLContext.blend = glIsEnabled(GL_BLEND);
	openGLContext.lighting = glIsEnabled(GL_LIGHTING);
	openGLContext.vertexArray = glIsEnabled(GL_VERTEX_ARRAY);
	openGLContext.textureCoordArray = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
	openGLContext.colorArray = glIsEnabled(GL_COLOR_ARRAY);
	openGLContext.normalArray = glIsEnabled(GL_NORMAL_ARRAY);
}

- (void)restoreOpenGLContext
{
	// Restore OpenGL parameters that have been modified
	openGLContext.normalArray ? glEnableClientState(GL_NORMAL_ARRAY) : glDisableClientState(GL_NORMAL_ARRAY);
	openGLContext.colorArray ? glEnableClientState(GL_COLOR_ARRAY) : glDisableClientState(GL_COLOR_ARRAY);
	openGLContext.textureCoordArray ? glEnableClientState(GL_TEXTURE_COORD_ARRAY) : glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	openGLContext.vertexArray ? glEnableClientState(GL_VERTEX_ARRAY) : glDisableClientState(GL_VERTEX_ARRAY);
	
	openGLContext.lighting ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
	openGLContext.blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	openGLContext.texture2D ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
	openGLContext.cullFace ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	openGLContext.depthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	
	glBlendFunc(openGLContext.blendSrc, openGLContext.blendDst);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, openGLContext.textureEnvMode);
	glBindTexture(GL_TEXTURE_2D, openGLContext.textureBinding2D);
	glDepthMask(openGLContext.depthWriteMask);
	glDepthFunc(openGLContext.depthFunc);
	glLineWidth(openGLContext.linewidth);

	// Make sure the client active texture is the same as the server active texture
	glClientActiveTexture(openGLContext.clientActiveTexture);
}

- (void)setScreenOrientationRight:(BOOL)right
{
	screenOrientationRight = right;
}

- (void)drawView
{	
	// Save OpenGLContext
	[self saveOpenGLContext];
	
	// Setup the quad
	// WARNING: it is important to set the background color to "transparent black" (0, 0, 0, 0) in Unity!
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBlendFunc(GL_ONE, GL_ONE);
	
	//glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
		
	// Set the projection matrix for the background elements
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixf(screenOrientationRight ? matrixOrthoBackRight : matrixOrthoBackLeft);
	
	// Set the model view matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Set the texture matrix
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	
	// Bind the background texture
	// Draw video
	[video drawSelf];

	// Restore the model view matrix
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	// Restore the model view matrix
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	// Restore the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Restore OpenGL context if modified
	[self restoreOpenGLContext];

//	CHECK_OPENGL_ERROR();
}

- (void)dealloc 
{
	[video release];
	[super dealloc];
}

@end
