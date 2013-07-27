/*
 * AR Drone demo
 *
 * OpenGL video rendering
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <assert.h>

#include <GLES/gl.h>
#include <GLES/glext.h>
#include "app.h"

static uint8_t    *pixbuf = NULL;
static GLuint      texture;

static long otick = 0;

void video_init(void)
{
    int tex_width, tex_height;
    GLint crop[4] = { VIDEO_WIDTH, 0, -VIDEO_WIDTH, VIDEO_HEIGHT };

    INFO("initializing OpenGL video rendering...\n");

    // FIXME: this should be computed (smallest power of 2 > dimension)
    tex_width = 256;
    tex_height = 256;
    otick = 0;

    if (!pixbuf) {
        pixbuf = malloc(tex_width * tex_height * 2);
        assert(pixbuf);
        memcpy(pixbuf, default_image, VIDEO_WIDTH*VIDEO_HEIGHT*2);
    }

    glEnable(GL_TEXTURE_2D);
    glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDeleteTextures(1, &texture);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Call glTexImage2D only once, and use glTexSubImage2D later
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB,
                 GL_UNSIGNED_SHORT_5_6_5, pixbuf);
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);
    // use NEAREST instead of LINEAR for faster (but uglier) results
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glShadeModel(GL_FLAT);
    glClear(GL_COLOR_BUFFER_BIT);
}

// Called from the app framework.
void video_deinit()
{
    INFO("terminating OpenGL video rendering...\n");
    //glDeleteTextures(1, &texture);
    if (pixbuf) {
        free(pixbuf);
        pixbuf = NULL;
    }
}

void video_render(long tick, int width, int height)
{
	static int num = 0;

    glBindTexture(GL_TEXTURE_2D, texture);

	if (num_picture_decoded != num) {
        /* a new frame is available, update texture */
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, VIDEO_WIDTH,VIDEO_HEIGHT,
                        GL_RGB, GL_UNSIGNED_SHORT_5_6_5, picture_buf);
		num = num_picture_decoded;
    }
    /* and draw it on the screen */
    glDrawTexiOES(0, 0, 0, width, height);
}

