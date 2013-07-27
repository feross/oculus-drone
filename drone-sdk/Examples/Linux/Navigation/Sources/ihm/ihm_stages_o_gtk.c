/*
 * @ihm_stages_o_gtk.c
 * @author marc-olivier.dzeukou@parrot.com
 * @date 2007/07/27
 *
 * ihm vision thread implementation
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <gtk/gtkcontainer.h>
#include <sys/time.h>
#include <time.h>

#include <VP_Api/vp_api.h>
#include <VP_Api/vp_api_error.h>
#include <VP_Api/vp_api_stage.h>
#include <VP_Api/vp_api_picture.h>
#include <VP_Stages/vp_stages_io_file.h>
#ifdef USE_ELINUX
#include <VP_Stages/vp_stages_V4L2_i_camif.h>
#else
#include <VP_Stages/vp_stages_i_camif.h>
#endif

#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_malloc.h>
#include <VP_Os/vp_os_delay.h>
#include <VP_Stages/vp_stages_yuv2rgb.h>
#include <VP_Stages/vp_stages_buffer_to_picture.h>

#ifdef PC_USE_VISION
#     include <Vision/vision_draw.h>
#     include <Vision/vision_stage.h>
#endif

#include <config.h>

#ifdef JPEG_CAPTURE
#include <VP_Stages/vp_stages_io_jpeg.h>
#else
#include <VLIB/Stages/vlib_stage_decode.h>
#endif


#include <ardrone_tool/ardrone_tool.h>
#include <ardrone_tool/Com/config_com.h>

//#define USE_FFMPEG_RECORD

#ifndef RECORD_VIDEO
#define RECORD_VIDEO
#endif

#ifdef RECORD_VIDEO
#include <ardrone_tool/Video/video_stage_recorder.h>
	#ifdef USE_FFMPEG_RECORDER
	#include <ardrone_tool/Video/video_stage_ffmpeg_recorder.h>
	#endif
#endif

#include <ardrone_tool/Video/video_com_stage.h>

#include "ihm/ihm.h"
#include "ihm/ihm_vision.h"
#include "ihm/ihm_stages_o_gtk.h"
#include "common/mobile_config.h"

#define NB_STAGES 10

#define FPS2TIME  0.10

#define CAMIF_V_CAMERA_USED CAMIF_CAMERA_CRESYN
#define CAMIF_H_CAMERA_USED CAMIF_CAMERA_OVTRULY

PIPELINE_HANDLE pipeline_handle;

extern GtkWidget *ihm_ImageWin, *ihm_ImageEntry[9], *ihm_ImageDA, *ihm_VideoStream_VBox;
/* For fullscreen video display */
extern GtkWindow *fullscreen_window;
extern GtkImage *fullscreen_image;
extern GdkScreen *fullscreen;

extern int tab_vision_config_params[10];
extern int vision_config_options;
extern int image_vision_window_view, image_vision_window_status;
extern char video_to_play[16];

static GtkImage *image = NULL;
static GdkPixbuf *pixbuf = NULL;

static int32_t   pixbuf_width      = 0;
static int32_t   pixbuf_height     = 0;
static int32_t   pixbuf_rowstride  = 0;
static uint8_t*  pixbuf_data       = NULL;



C_RESULT
output_gtk_stage_open( vp_stages_gtk_config_t *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  //  printf("In gtk stage open\n" );
 /*pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
                          FALSE             ,
                          8                 ,
                          cfg->width        ,
                          cfg->height       );*/

  return (SUCCESS);
}



void destroy_image_callback( GtkWidget *widget, gpointer data )
{
	image=NULL;
}

extern GtkWidget * ihm_fullScreenFixedContainer;
extern GtkWidget * ihm_fullScreenHBox;

C_RESULT
output_gtk_stage_transform( vp_stages_gtk_config_t *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
	vlib_stage_decoding_config_t* vec;

	if (!ihm_is_initialized) return SUCCESS;
	if (ihm_ImageWin==NULL) return SUCCESS;
	if( image_vision_window_view != WINDOW_VISIBLE) return SUCCESS;

	gdk_threads_enter(); //http://library.gnome.org/devel/gdk/stable/gdk-Threads.html

	vec = (vlib_stage_decoding_config_t*)cfg->last_decoded_frame_info;

	  pixbuf_width     = vec->controller.width;
	  pixbuf_height    = vec->controller.height;
	  pixbuf_rowstride = cfg->rowstride;
	  pixbuf_data      = (uint8_t*)in->buffers[0];

	  //printf("Taille cfg : %i %i \n",cfg->max_width,cfg->max_height);

  if(pixbuf!=NULL)
  {
	 g_object_unref(pixbuf);
	 pixbuf=NULL;
  }

	  pixbuf = gdk_pixbuf_new_from_data(pixbuf_data,
  											 GDK_COLORSPACE_RGB,
  											 FALSE,
  											 8,
  											 pixbuf_width,
  											 pixbuf_height,
  											 pixbuf_rowstride,
  											 NULL,
  											 NULL);

	  if (fullscreen!=NULL && fullscreen_window!=NULL)
	  {
		  pixbuf = gdk_pixbuf_scale_simple ( pixbuf,
				  	  	  	  	  	  	  	 gdk_screen_get_width ( fullscreen ),
				  	  	  	  	  	  	  	 gdk_screen_get_height ( fullscreen ),
				  	  	  	  	  	  	  	 /*GDK_INTERP_HYPER*/
				  	  	  	  	  	  	  	 GDK_INTERP_BILINEAR) ;
		  /*if (fullscreen_image == NULL)
		  {
			  fullscreen_image  = (GtkImage*) gtk_image_new_from_pixbuf( pixbuf );
			  //if (fullscreen_image == NULL) { printf("Probleme.\n"); }
			  //gtk_container_add( GTK_CONTAINER( fullscreen_window ), GTK_WIDGET(fullscreen_image) );
			  gtk_fixed_put(ihm_fullScreenFixedContainer,fullscreen_image,0,0);
		  }*/
		  if (fullscreen_image != NULL)
		  {
			  gtk_image_set_from_pixbuf(fullscreen_image, pixbuf);
			  //gtk_widget_show_all (GTK_WIDGET(fullscreen_window));
			  gtk_widget_show (GTK_WIDGET(fullscreen_image));
			  //gtk_widget_show(ihm_fullScreenHBox);
		  }
	  }
	  else
	  {
		  pixbuf = gdk_pixbuf_scale_simple ( pixbuf,
		  				  	  	  	  	  	  	  	 cfg->max_width,
		  				  	  	  	  	  	  	  	 cfg->max_height,
		  				  	  	  	  	  	  	  	 /*GDK_INTERP_HYPER*/
		  				  	  	  	  	  	  	  	 GDK_INTERP_BILINEAR) ;

		  if( image == NULL && pixbuf!=NULL)
		  {
				image  = (GtkImage*) gtk_image_new_from_pixbuf( pixbuf );
				gtk_signal_connect(GTK_OBJECT(image), "destroy", G_CALLBACK(destroy_image_callback), NULL );
				if(GTK_IS_WIDGET(ihm_ImageWin))
						if (GTK_IS_WIDGET(ihm_VideoStream_VBox))
							gtk_container_add( GTK_CONTAINER( ihm_VideoStream_VBox ), (GtkWidget*)image );
		  }
		  if( image!=NULL && pixbuf!=NULL )
		  {
			 gtk_image_set_from_pixbuf(image, pixbuf);
		  }
		  gtk_widget_show_all( ihm_ImageWin );
	  }

	 gdk_threads_leave();
	 return (SUCCESS);
}


C_RESULT
output_gtk_stage_close( vp_stages_gtk_config_t *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  return (SUCCESS);
}


const vp_api_stage_funcs_t vp_stages_output_gtk_funcs =
{
  NULL,
  (vp_api_stage_open_t)output_gtk_stage_open,
  (vp_api_stage_transform_t)output_gtk_stage_transform,
  (vp_api_stage_close_t)output_gtk_stage_close
};

#ifdef PC_USE_VISION
static vp_os_mutex_t draw_trackers_update;
/*static*/ vp_stages_draw_trackers_config_t draw_trackers_cfg = { 0 };

void set_draw_trackers_config(vp_stages_draw_trackers_config_t* cfg)
{
void*v;
  vp_os_mutex_lock( &draw_trackers_update );
  v = draw_trackers_cfg.last_decoded_frame_info;
  vp_os_memcpy( &draw_trackers_cfg, cfg, sizeof(draw_trackers_cfg) );
  draw_trackers_cfg.last_decoded_frame_info = v;
  vp_os_mutex_unlock( &draw_trackers_update );
}

C_RESULT draw_trackers_stage_open( vp_stages_draw_trackers_config_t *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  vp_os_mutex_lock( &draw_trackers_update );

  int32_t i;
  for( i = 0; i < NUM_MAX_SCREEN_POINTS; i++ )
  {
    cfg->locked[i] = C_OK;
  }

  PRINT("Draw trackers inited with %d trackers\n", cfg->num_points);

  vp_os_mutex_unlock( &draw_trackers_update );

  return (SUCCESS);
}

C_RESULT draw_trackers_stage_transform( vp_stages_draw_trackers_config_t *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  int32_t i;

  vp_os_mutex_lock( &draw_trackers_update );

  vp_api_picture_t *picture = (vp_api_picture_t *) in->buffers;

  if( in->size > 0 )
  {
#ifdef DEBUG
    for(i = 0 ; i < cfg->num_points; i++)
    {
      int32_t dist;
      uint8_t color;
      screen_point_t point;

      point    = cfg->points[i];
//       point.x += ACQ_WIDTH / 2;
//       point.y += ACQ_HEIGHT / 2;

      if( point.x >= STREAM_WIDTH || point.x < 0 || point.y >= STREAM_HEIGHT || point.y < 0 )
      {
        PRINT("Bad point (%d,%d) received at index %d on %d points\n", point.x, point.y, i, cfg->num_points);
        continue;
      }

      if( SUCCEED(cfg->locked[i]) )
      {
        dist  = 3;
        color = 0;
      }
      else
      {
        dist  = 1;
        color = 0xFF;
      }

      vision_trace_cross(&point, dist, color, picture);
    }
#endif

    for(i = 0 ; i < cfg->detected ; i++)
    {
      //uint32_t centerX,centerY;
      uint32_t width,height;
      screen_point_t center;
      if (cfg->last_decoded_frame_info!=NULL){
		  center.x = cfg->patch_center[i].x*cfg->last_decoded_frame_info->controller.width/1000;
		  center.y = cfg->patch_center[i].y*cfg->last_decoded_frame_info->controller.height/1000;
		  width  = cfg->width[i]*cfg->last_decoded_frame_info->controller.width/1000;
		  height = cfg->height[i]*cfg->last_decoded_frame_info->controller.height/1000;

		  width = min(2*center.x,width); width = min(2*(cfg->last_decoded_frame_info->controller.width-center.x),width) -1;
		  height = min(2*center.y,height); width = min(2*(cfg->last_decoded_frame_info->controller.height-center.y),height) -1;

		  vision_trace_colored_rectangle(&center, width, height, 0, 255, 128, picture);
		  /*Stephane*/vision_trace_colored_rectangle(&center, width-2, height-2, 200, 128-80, 0, picture); // blue
      }else{printf("Problem drawing rectangle.\n");}
    }
  }

  vp_os_mutex_unlock( &draw_trackers_update );

  out->size         = in->size;
  out->indexBuffer  = in->indexBuffer;
  out->buffers      = in->buffers;

  out->status       = VP_API_STATUS_PROCESSING;

  return (SUCCESS);
}

C_RESULT draw_trackers_stage_close( vp_stages_draw_trackers_config_t *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  return (SUCCESS);
}

const vp_api_stage_funcs_t draw_trackers_funcs =
{
  NULL,
  (vp_api_stage_open_t)draw_trackers_stage_open,
  (vp_api_stage_transform_t)draw_trackers_stage_transform,
  (vp_api_stage_close_t)draw_trackers_stage_close
};

#endif


#ifdef RAW_CAPTURE

#ifdef RECORD_VISION_DATA
#ifdef RECORD_VIDEO
extern char video_filename[];
#endif
static void save_vision_attitude(vision_attitude_t* vision_attitude, int32_t custom_data_size )
{
#ifdef RECORD_VIDEO
  static FILE* fp = NULL;

  if( fp == NULL )
  {
    char filename[VIDEO_FILENAME_LENGTH];
    char* dot;

    strcpy( filename, video_filename );
    dot = strrchr( filename, '.' );
    dot[1] = 'd';
    dot[2] = 'a';
    dot[3] = 't';
    dot[4] = '\0';

    fp = fopen(filename, "wb");
  }

  if( fp != NULL )
  {
    fwrite( vision_attitude, custom_data_size, 1, fp );
    fflush( fp );
  }
#endif
}
#endif

DEFINE_THREAD_ROUTINE(ihm_stages_vision, data)
{
  int32_t i;
  C_RESULT res;

  vp_api_io_pipeline_t    pipeline;
  vp_api_io_data_t        out;
  vp_api_io_stage_t       stages[NB_STAGES];

  vp_api_picture_t picture;

  video_com_config_t                      icc;
  vp_stages_buffer_to_picture_config_t    bpc;
  vp_stages_yuv2rgb_config_t              yuv2rgbconf;
  vp_stages_gtk_config_t                  gtkconf;
  video_stage_recorder_config_t           vrc;

  /// Picture configuration
  picture.format        = PIX_FMT_YUV420P;

  picture.width         = V_ACQ_WIDTH;
  picture.height        = V_ACQ_HEIGHT;
  picture.framerate     = CAMIF_V_FRAMERATE_USED;

  picture.y_buf   = vp_os_malloc( V_ACQ_WIDTH * V_ACQ_HEIGHT     );
  picture.cr_buf  = vp_os_malloc( V_ACQ_WIDTH * V_ACQ_HEIGHT / 4 );
  picture.cb_buf  = vp_os_malloc( V_ACQ_WIDTH * V_ACQ_HEIGHT / 4 );

  picture.y_line_size   = V_ACQ_WIDTH;
  picture.cb_line_size  = V_ACQ_WIDTH / 2;
  picture.cr_line_size  = V_ACQ_WIDTH / 2;

  for(i = 0; i < V_ACQ_WIDTH * V_ACQ_HEIGHT/ 4; i++ )
  {
    picture.cr_buf[i] = 0x80;
    picture.cb_buf[i] = 0x80;
  }

  vp_os_memset(&icc,          0, sizeof( icc ));
  vp_os_memset(&bpc,          0, sizeof( bpc ));
  vp_os_memset(&yuv2rgbconf,  0, sizeof( yuv2rgbconf ));
  vp_os_memset(&gtkconf,      0, sizeof( gtkconf ));

  icc.com                 = COM_VIDEO();
  icc.buffer_size         = 1024;
  icc.protocol            = VP_COM_TCP;
  COM_CONFIG_SOCKET_VIDEO(&icc.socket, VP_COM_CLIENT, VIDEO_PORT, wifi_ardrone_ip);

  bpc.picture             = &picture;
  bpc.y_buffer_size       = picture.width*picture.height;
  bpc.y_blockline_size    = picture.width*CAMIF_BLOCKLINES; // each blockline have 16 lines
  bpc.y_current_size      = 0;
  bpc.num_frames          = 0;
  bpc.y_buf_ptr           = NULL;
#ifdef USE_VIDEO_YUV
  bpc.cr_buf_ptr          = NULL;
  bpc.cb_buf_ptr          = NULL;
#endif
#ifdef USE_VIDEO_YUV
  bpc.luma_only           = FALSE;
#else
  bpc.luma_only           = TRUE;
#endif // USE_VIDEO_YUV

#ifdef BLOCK_MODE
  bpc.block_mode_enable   = TRUE;
#else
  bpc.block_mode_enable   = FALSE;
#endif

#ifdef RECORD_VISION_DATA
  bpc.custom_data_size    = sizeof(vision_attitude_t);
  bpc.custom_data_handler = (custom_data_handler_cb)save_vision_attitude;
#else
  bpc.custom_data_size    = 0;
  bpc.custom_data_handler = 0;
#endif

  yuv2rgbconf.rgb_format = VP_STAGES_RGB_FORMAT_RGB24;
  if( CAMIF_H_CAMERA_USED == CAMIF_CAMERA_OVTRULY_UPSIDE_DOWN_ONE_BLOCKLINE_LESS )
    yuv2rgbconf.mode = VP_STAGES_YUV2RGB_MODE_UPSIDE_DOWN;

  vrc.fp = NULL;

  pipeline.nb_stages = 0;

  stages[pipeline.nb_stages].type    = VP_API_INPUT_SOCKET;
  stages[pipeline.nb_stages].cfg     = (void *)&icc;
  stages[pipeline.nb_stages].funcs   = video_com_funcs;

  pipeline.nb_stages++;

  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void *)&bpc;
  stages[pipeline.nb_stages].funcs   = vp_stages_buffer_to_picture_funcs;

  pipeline.nb_stages++;

#ifdef RECORD_VIDEO
  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void*)&vrc;
  stages[pipeline.nb_stages].funcs   = video_recorder_funcs;

  pipeline.nb_stages++;
#endif // RECORD_VIDEO

#ifdef PC_USE_VISION
  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void*)&draw_trackers_cfg;
  stages[pipeline.nb_stages].funcs   = draw_trackers_funcs;

  pipeline.nb_stages++;
#endif

  stages[pipeline.nb_stages].type    = VP_API_FILTER_YUV2RGB;
  stages[pipeline.nb_stages].cfg     = (void*)&yuv2rgbconf;
  stages[pipeline.nb_stages].funcs   = vp_stages_yuv2rgb_funcs;

  pipeline.nb_stages++;

  stages[pipeline.nb_stages].type    = VP_API_OUTPUT_SDL;
  stages[pipeline.nb_stages].cfg     = (vp_stages_gtk_config_t *)&gtkconf;
  stages[pipeline.nb_stages].funcs   = vp_stages_output_gtk_funcs;

  pipeline.nb_stages++;

  pipeline.stages = &stages[0];

  // Wait for ihm image window to be visible
  while( !ardrone_tool_exit() && image_vision_window_view != WINDOW_VISIBLE ) {
    vp_os_delay( 200 );
  }

  if( !ardrone_tool_exit() )
  {
    PRINT("\n   IHM stage vision thread initialisation\n\n");

    res = vp_api_open(&pipeline, &pipeline_handle);

    if( SUCCEED(res) )
    {
      int loop = SUCCESS;
      out.status = VP_API_STATUS_PROCESSING;

      while( !ardrone_tool_exit() && (loop == SUCCESS) )
      {
#ifdef ND_WRITE_TO_FILE
        num_picture_decoded = bpc.num_picture_decoded;
#endif
        if( image_vision_window_view == WINDOW_VISIBLE ) {
          if( SUCCEED(vp_api_run(&pipeline, &out)) ) {
            if( (out.status == VP_API_STATUS_PROCESSING || out.status == VP_API_STATUS_STILL_RUNNING) ) {
              loop = SUCCESS;
            }
          }
          else loop = -1; // Finish this thread
        }
        //vp_os_delay( 25 );
      }

      vp_api_close(&pipeline, &pipeline_handle);
    }
  }

  return (THREAD_RET)0;
}
#elif defined(JPEG_CAPTURE)
DEFINE_THREAD_ROUTINE(ihm_stages_vision, data)
{
  C_RESULT res;

  image = NULL;

  vp_api_io_pipeline_t    pipeline;
  vp_api_io_data_t        out;
  vp_api_io_stage_t       stages[NB_STAGES];

  vp_api_picture_t picture;

  video_com_config_t              icc;
  vp_stages_decoder_jpeg_config_t jdc;
  vp_stages_yuv2rgb_config_t      yuv2rgbconf;
  vp_stages_gtk_config_t          gtkconf;
  video_stage_recorder_config_t   vrc;

  /// Picture configuration
  picture.format        = PIX_FMT_YUV420P;

  picture.width         = STREAM_WIDTH;
  picture.height        = STREAM_HEIGHT;
  picture.framerate     = 30;

  picture.y_buf   = vp_os_malloc( STREAM_WIDTH * STREAM_HEIGHT     );
  picture.cr_buf  = vp_os_malloc( STREAM_WIDTH * STREAM_HEIGHT / 4 );
  picture.cb_buf  = vp_os_malloc( STREAM_WIDTH * STREAM_HEIGHT / 4 );

  picture.y_line_size   = STREAM_WIDTH;
  picture.cb_line_size  = STREAM_WIDTH / 2;
  picture.cr_line_size  = STREAM_WIDTH / 2;

  vp_os_memset(&icc,          0, sizeof( icc ));
  vp_os_memset(&jdc,          0, sizeof( jdc ));
  vp_os_memset(&yuv2rgbconf,  0, sizeof( yuv2rgbconf ));
  vp_os_memset(&gtkconf,      0, sizeof( gtkconf ));

  icc.com                 = COM_VIDEO();
  icc.buffer_size         = 8192;
  icc.protocol            = VP_COM_UDP;
  COM_CONFIG_SOCKET_VIDEO(&icc.socket, VP_COM_CLIENT, VIDEO_PORT, wifi_ardrone_ip);


  jdc.width				= STREAM_WIDTH;
  jdc.height			= STREAM_HEIGHT;
  jdc.dct_method		= JDCT_FLOAT;

/*  vec.width               = ACQ_WIDTH;
  vec.height              = ACQ_HEIGHT;
  vec.picture             = &picture;
#ifdef USE_VIDEO_YUV
  vec.luma_only           = FALSE;
#else
  vec.luma_only           = TRUE;
#endif // USE_VIDEO_YUV
  vec.block_mode_enable   = TRUE;*/

  yuv2rgbconf.rgb_format = VP_STAGES_RGB_FORMAT_RGB24;
  if( CAMIF_CAMERA_USED == CAMIF_CAMERA_OVTRULY_UPSIDE_DOWN_ONE_BLOCKLINE_LESS )
    yuv2rgbconf.mode = VP_STAGES_YUV2RGB_MODE_UPSIDE_DOWN;

  gtkconf.width     = picture.width;
  gtkconf.height    = picture.height;
  gtkconf.rowstride = picture.width * 3;

  vrc.fp = NULL;

  pipeline.nb_stages = 0;

  stages[pipeline.nb_stages].type    = VP_API_INPUT_SOCKET;
  stages[pipeline.nb_stages].cfg     = (void *)&icc;
  stages[pipeline.nb_stages].funcs   = video_com_funcs;

  pipeline.nb_stages++;

  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void*)&jdc;
  stages[pipeline.nb_stages].funcs   = vp_stages_decoder_jpeg_funcs;

  pipeline.nb_stages++;

#ifdef RECORD_VIDEO
  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void*)&vrc;
  stages[pipeline.nb_stages].funcs   = video_recorder_funcs;

  pipeline.nb_stages++;
#endif // RECORD_VIDEO

  draw_trackers_cfg.last_decoded_frame_info = &vec;
  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void*)&draw_trackers_cfg;
  stages[pipeline.nb_stages].funcs   = draw_trackers_funcs;

  pipeline.nb_stages++;

  stages[pipeline.nb_stages].type    = VP_API_FILTER_YUV2RGB;
  stages[pipeline.nb_stages].cfg     = (void*)&yuv2rgbconf;
  stages[pipeline.nb_stages].funcs   = vp_stages_yuv2rgb_funcs;

  pipeline.nb_stages++;

  stages[pipeline.nb_stages].type    = VP_API_OUTPUT_SDL;
  stages[pipeline.nb_stages].cfg     = (vp_stages_gtk_config_t *)&gtkconf;
  stages[pipeline.nb_stages].funcs   = vp_stages_output_gtk_funcs;

  pipeline.nb_stages++;

  pipeline.stages = &stages[0];

  // Wait for ihm image window to be visible
  while( !ardrone_tool_exit() && image_vision_window_view != WINDOW_VISIBLE ) {
    vp_os_delay( 200 );
  }

  if( !ardrone_tool_exit() )
  {
    PRINT("\n   IHM stage vision thread initialisation\n\n");

    res = vp_api_open(&pipeline, &pipeline_handle);

    if( SUCCEED(res) )
    {
      int loop = SUCCESS;
      out.status = VP_API_STATUS_PROCESSING;

      while( !ardrone_tool_exit() && (loop == SUCCESS) )
      {
//#ifdef ND_WRITE_TO_FILE
//    	  num_picture_decoded = vec.num_picture_decoded;
//#endif
        if( image_vision_window_view == WINDOW_VISIBLE ) {
          if( SUCCEED(vp_api_run(&pipeline, &out)) ) {
            if( (out.status == VP_API_STATUS_PROCESSING || out.status == VP_API_STATUS_STILL_RUNNING) ) {
              loop = SUCCESS;
            }
          }
          else loop = -1; // Finish this thread
        }
        // vp_os_delay( 25 );
      }

      vp_api_close(&pipeline, &pipeline_handle);
    }
  }

  PRINT("   IHM stage vision thread ended\n\n");

  return (THREAD_RET)0;
}

#else// RAW_CAPTURE
DEFINE_THREAD_ROUTINE(ihm_stages_vision, data)
{
  C_RESULT res;

  //image = NULL;

  vp_api_io_pipeline_t    pipeline;
  vp_api_io_data_t        out;
  vp_api_io_stage_t       stages[NB_STAGES];

  vp_api_picture_t picture;

  video_com_config_t              icc;
  vlib_stage_decoding_config_t    vec;
  vp_stages_yuv2rgb_config_t      yuv2rgbconf;
  vp_stages_gtk_config_t          gtkconf;
#ifdef RECORD_VIDEO
  video_stage_recorder_config_t   vrc;
#ifdef USE_FFMPEG_RECORDER
  video_stage_recorder_config_t   vrc_ffmpeg;
#endif
#endif
  /// Picture configuration
  picture.format        = PIX_FMT_YUV420P;
  picture.width         = STREAM_WIDTH;
  picture.height        = STREAM_HEIGHT;
  picture.framerate     = 30;

  picture.y_buf   = vp_os_malloc( STREAM_WIDTH * STREAM_HEIGHT     );
  picture.cr_buf  = vp_os_malloc( STREAM_WIDTH * STREAM_HEIGHT / 4 );
  picture.cb_buf  = vp_os_malloc( STREAM_WIDTH * STREAM_HEIGHT / 4 );

  picture.y_line_size   = STREAM_WIDTH;
  picture.cb_line_size  = STREAM_WIDTH / 2;
  picture.cr_line_size  = STREAM_WIDTH / 2;

  vp_os_memset(&icc,          0, sizeof( icc ));
  vp_os_memset(&vec,          0, sizeof( vec ));
  vp_os_memset(&yuv2rgbconf,  0, sizeof( yuv2rgbconf ));
  vp_os_memset(&gtkconf,      0, sizeof( gtkconf ));

  icc.com                 = COM_VIDEO();
  icc.buffer_size         = 100000;
  icc.protocol            = VP_COM_UDP;
  COM_CONFIG_SOCKET_VIDEO(&icc.socket, VP_COM_CLIENT, VIDEO_PORT, wifi_ardrone_ip);

  vec.width               = STREAM_WIDTH;
  vec.height              = STREAM_HEIGHT;
  vec.picture             = &picture;
#ifdef USE_VIDEO_YUV
  vec.luma_only           = FALSE;
#else
  vec.luma_only           = TRUE;
#endif // USE_VIDEO_YUV
  vec.block_mode_enable   = TRUE;

  vec.luma_only           = FALSE;
  yuv2rgbconf.rgb_format = VP_STAGES_RGB_FORMAT_RGB24;
  if( CAMIF_H_CAMERA_USED == CAMIF_CAMERA_OVTRULY_UPSIDE_DOWN_ONE_BLOCKLINE_LESS )
    yuv2rgbconf.mode = VP_STAGES_YUV2RGB_MODE_UPSIDE_DOWN;
  gtkconf.max_width     = picture.width;
  gtkconf.max_height    = picture.height;
  gtkconf.rowstride = picture.width * 3;
  gtkconf.last_decoded_frame_info = (void*)&vec;

#ifdef RECORD_VIDEO
  vrc.fp = NULL;
#endif

  pipeline.nb_stages = 0;

  stages[pipeline.nb_stages].type    = VP_API_INPUT_SOCKET;
  stages[pipeline.nb_stages].cfg     = (void *)&icc;
  stages[pipeline.nb_stages].funcs   = video_com_funcs;

  pipeline.nb_stages++;

  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void*)&vec;
  stages[pipeline.nb_stages].funcs   = vlib_decoding_funcs;

  pipeline.nb_stages++;

#ifdef RECORD_VIDEO
//#warning Recording video option enabled in Navigation.
  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void*)&vrc;
  stages[pipeline.nb_stages].funcs   = video_recorder_funcs;

  pipeline.nb_stages++;

#ifdef USE_FFMPEG_RECORDER
//#warning FFMPEG Recording video option enabled in Navigation.
  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void*)&vrc_ffmpeg;
  stages[pipeline.nb_stages].funcs   = video_ffmpeg_recorder_funcs;

  pipeline.nb_stages++;
#endif
#endif // RECORD_VIDEO


#ifdef PC_USE_VISION
  draw_trackers_cfg.last_decoded_frame_info = &vec;
  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void*)&draw_trackers_cfg;
  stages[pipeline.nb_stages].funcs   = draw_trackers_funcs;

  pipeline.nb_stages++;
#endif

  stages[pipeline.nb_stages].type    = VP_API_FILTER_YUV2RGB;
  stages[pipeline.nb_stages].cfg     = (void*)&yuv2rgbconf;
  stages[pipeline.nb_stages].funcs   = vp_stages_yuv2rgb_funcs;

  pipeline.nb_stages++;

  stages[pipeline.nb_stages].type    = VP_API_OUTPUT_SDL;
  stages[pipeline.nb_stages].cfg     = (void*)&gtkconf;
  stages[pipeline.nb_stages].funcs   = vp_stages_output_gtk_funcs;

  pipeline.nb_stages++;

  pipeline.stages = &stages[0];

  // Wait for ihm image window to be visible
  while( !ardrone_tool_exit() && image_vision_window_view != WINDOW_VISIBLE ) {
    vp_os_delay( 200 );
  }

  if( !ardrone_tool_exit() )
  {
    PRINT("\n   IHM stage vision thread initialisation\n\n");

    res = vp_api_open(&pipeline, &pipeline_handle);

    if( SUCCEED(res) )
    {
      int loop = SUCCESS;
      out.status = VP_API_STATUS_PROCESSING;

      while( !ardrone_tool_exit() && (loop == SUCCESS) )
      {
#ifdef ND_WRITE_TO_FILE
        num_picture_decoded = vec.num_picture_decoded;
#endif
        if( image_vision_window_view == WINDOW_VISIBLE ) {
          if( SUCCEED(vp_api_run(&pipeline, &out)) ) {
            if( (out.status == VP_API_STATUS_PROCESSING || out.status == VP_API_STATUS_STILL_RUNNING) ) {
              loop = SUCCESS;
            }
          }
          else loop = -1; // Finish this thread
        }
        // vp_os_delay( 25 );
      }

      vp_api_close(&pipeline, &pipeline_handle);
    }
  }

  PRINT("   IHM stage vision thread ended\n\n");

  return (THREAD_RET)0;
}
#endif // RAW_CAPTURE
