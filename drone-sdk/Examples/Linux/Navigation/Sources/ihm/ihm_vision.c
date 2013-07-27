/*
 * @ihm_vision.c
 * @author marc-olivier.dzeukou@parrot.com
 * @date 2007/07/27
 *
 * @author stephane.piskorski@parrot.com
 * @date 2010/10/18
 * ihm vision source file
 *
 */

#include   <pthread.h>
#include   <gtk/gtk.h>

#include <ardrone_api.h>
#ifdef PC_USE_VISION
#    include <Vision/vision_tracker_engine.h>
#endif
#include "UI/ui.h"
#include "ihm/ihm_vision.h"
#include "ihm/ihm.h"
#include "common/mobile_config.h"
#include "ihm/ihm_stages_o_gtk.h"

#include <ardrone_tool/ardrone_tool_configuration.h>

#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_malloc.h>
#include <VP_Api/vp_api_supervisor.h>
#include <ardrone_tool/Video/video_stage_recorder.h>
#include <VLIB/video_codec.h>

enum {
  STATE_FRAME=0,
  TRACKING_PARAMETERS_FRAME,
  TRACKING_OPTION_FRAME,
  COMPUTING_OPTION_FRAME,
  VIDEO_STREAM_FRAME,
  VIDEO_DISPLAY_FRAME,
  NB_IMAGES_FRAMES
};

enum {
  TRACKING_PARAM_HBOX1=0,
  TRACKING_PARAM_HBOX2,
  TRACKING_PARAMS_HBOX,
  TRACKING_OPTION_HBOX,
  COMPUTING_OPTION_HBOX,
  VIDEO_STREAM_HBOX,
  VIDEO_DISPLAY_HBOX,
  NB_IMAGES_H_BOXES
};

enum {
  CS_ENTRY=0,
  NB_P_ENTRY,
  LOSS_ENTRY,
  NB_TLG_ENTRY,
  NB_TH_ENTRY,
  SCALE_ENTRY,
  DIST_MAX_ENTRY,
  MAX_DIST_ENTRY,
  NOISE_ENTRY,
  FAKE_ENTRY,
  NB_IMAGES_ENTRIES
};

enum {
  UPDATE_VISION_PARAMS_BUTTON = 0,
  TZ_KNOWN_BUTTON,
  NO_SE_BUTTON,
  SE2_BUTTON,
  SE3_BUTTON,
  PROJ_OVERSCENE_BUTTON,
  LS_BUTTON,
  FRONTAL_SCENE_BUTTON,
  FLAT_GROUND_BUTTON,
  RAW_CAPTURE_BUTTON,
  ZAPPER_BUTTON,
  FULLSCREEN_BUTTON,
  NB_IMAGES_BUTTONS
};

enum {
  CODEC_TYPE_LIST=0,
  BITRATE_MODE_LIST,
  MANUAL_BITRATE_ENTRY,
  NB_VIDEO_STREAM_WIDGET
};

char  ihm_ImageTitle[128] = "VISION : Image" ;
char *ihm_ImageFrameCaption[NB_IMAGES_FRAMES]  = {"Vision states","Tracking parameters","Tracking options","Computing options","Video Stream","Video Display"};
char *ihm_ImageEntryCaption[NB_IMAGES_ENTRIES]  = {  "      CS ",
                                     "           NB_P ",
                                     "       Loss% ",
                                     "     NB_TLg ",
                                     "   NB_TH ",
                                     " Scale ",
                                     "    Dist_Max ",
                                     "   Max_Dist ",
                                     "        Noise ",
                                     ""};
char *ihm_ImageButtonCaption[NB_IMAGES_BUTTONS] = { "Update\nvision\nparams",
                                     " TZ_Known  ",
                                     "   No_SE    ",
                                     "   SE2      ",
                                     "     SE3     ",
                                     "Proj_OverScene",
                                     "    LS   ",
                                     " Frontal_Scene  ",
                                     " Flat_ground ",
                                     "Record Video\non local disk",
                                     " Change camera",
                                     " GTK Full Screen "};
char *ihm_ImageVideoStreamCaption[NB_VIDEO_STREAM_WIDGET] = {" Codec type "," Bitrate control mode ", " Manual target bitrate "};


GtkWidget *ihm_ImageVBox, *ihm_ImageVBoxPT, *ihm_ImageHBox[NB_IMAGES_H_BOXES], *displayvbox,*ihm_ImageButton[NB_IMAGES_BUTTONS], *ihm_ImageLabel[NB_IMAGES_ENTRIES],  *ihm_ImageFrames[NB_IMAGES_FRAMES], *ihm_VideoStreamLabel[NB_VIDEO_STREAM_WIDGET];

extern GtkWidget *button_show_image,*button_show_image2;
extern mobile_config_t *pcfg;
extern PIPELINE_HANDLE pipeline_handle;
/* Vision image var */
GtkLabel *label_vision_values=NULL;
GtkWidget *ihm_ImageWin=NULL, *ihm_ImageEntry[NB_IMAGES_ENTRIES], *ihm_VideoStream_VBox=NULL;

/* For fullscreen video display */
GtkWidget *fullscreen_window = NULL;
GtkWidget *fullscreen_eventbox = NULL;
GtkImage *fullscreen_image = NULL;
GdkScreen *fullscreen = NULL;
GtkWidget *ihm_fullScreenButton[4], *ihm_fullScreenHBox;
GtkWidget *ihm_fullScreenFixedContainer;
GtkWidget *align;
int flag_timer_is_active = 0;
int timer_counter = 0;

GtkWidget* video_bitrateEntry;
GtkWidget*  video_bitrateModeList;
GtkWidget * video_codecList;
GtkWidget *video_bitrateButton;
int tab_vision_config_params[10];
int vision_config_options;
int image_vision_window_status, image_vision_window_view;
char label_vision_state_value[32];
extern GtkImage *image;

static void ihm_sendVisionConfigParams(GtkWidget *widget, gpointer data) {
  api_vision_tracker_params_t params;

  params.coarse_scale       = tab_vision_config_params[0]; // scale of current picture with respect to original picture
  params.nb_pair            = tab_vision_config_params[1]; // number of searched pairs in each direction
  params.loss_per           = tab_vision_config_params[2]; // authorized lost pairs percentage for tracking
  params.nb_tracker_width   = tab_vision_config_params[3]; // number of trackers in width of current picture
  params.nb_tracker_height  = tab_vision_config_params[4]; // number of trackers in height of current picture
  params.scale              = tab_vision_config_params[5]; // distance between two pixels in a pair
  params.trans_max          = tab_vision_config_params[6]; // largest value of trackers translation between two adjacent pictures
  params.max_pair_dist      = tab_vision_config_params[7]; // largest distance of pairs research from tracker location
  params.noise              = tab_vision_config_params[8]; // threshold of significative contrast

  ardrone_at_set_vision_track_params( &params );

  DEBUG_PRINT_SDK("CS %04d NB_P %04d Lossp %04d NB_Tlg %04d NB_TH %04d Scale %04d Dist_Max %04d Max_Dist %04d Noise %04d\n",
          tab_vision_config_params[0],
          tab_vision_config_params[1],
          tab_vision_config_params[2],
          tab_vision_config_params[3],
          tab_vision_config_params[4],
          tab_vision_config_params[5],
          tab_vision_config_params[6],
          tab_vision_config_params[7],
          tab_vision_config_params[8] );
}


void ihm_video_recording_callback(video_stage_recorder_config_t * cfg) {
  printf("Started recording %s\n", cfg->video_filename);
}

static void ihm_RAWCapture(GtkWidget *widget, gpointer data) {
  static int is_recording = 0;

  DEBUG_PRINT_SDK("   RAW video capture\n");
  DEST_HANDLE dest;

  is_recording ^= 1;

  // Sending AT command to drone.
  ardrone_at_start_raw_capture();

  /*
   * Tells the Raw capture stage to start dumping YUV frames from
   *  the pipeline to a disk file.
   */
  dest.stage = 2;
  dest.pipeline = pipeline_handle;
  vp_api_post_message(dest, PIPELINE_MSG_START, ihm_video_recording_callback, NULL);

  /* Tells the FFMPEG recorder stage to start dumping the video in a
   * MPEG4 video file.
   */
  dest.stage = 3;
  vp_api_post_message(dest, PIPELINE_MSG_START, NULL, NULL);

  if (is_recording) {
    gtk_button_set_label((GtkButton*) ihm_ImageButton[RAW_CAPTURE_BUTTON], (const gchar*) "Recording...\n(click again to stop)");
    gtk_button_set_label((GtkButton*) ihm_fullScreenButton[0], (const gchar*) "Recording...\n(click again to stop)");
  }
  else {
    gtk_button_set_label((GtkButton*) ihm_ImageButton[RAW_CAPTURE_BUTTON], (const gchar*) "Recording stopped.\nClick again to start a new video");
    gtk_button_set_label((GtkButton*) ihm_fullScreenButton[0], (const gchar*) "Recording stopped.\nClick again to start a new video");
  }

}

static void ihm_Zapper(GtkWidget *widget, gpointer data) {
  int32_t channel = ZAP_CHANNEL_NEXT;
  printf("   Zap\n");
  ARDRONE_TOOL_CONFIGURATION_ADDEVENT(video_channel, &channel, NULL);
}

static void ihm_VideoFullScreenStop(GtkWidget *widget, gpointer data) {
  printf("Quitting fullscreen.\n");
  fullscreen = NULL;
  fullscreen_image = NULL;
  fullscreen_window = NULL;
}

gboolean hide_fullscreen_buttons(gpointer pData) {
  timer_counter--;
  if (timer_counter <= 0) {
    if (GTK_IS_WIDGET(ihm_fullScreenHBox))
      gtk_widget_hide(ihm_fullScreenHBox);
    timer_counter = 0;
  }
  return TRUE;
}

void ihm_VideoFullScreenMouseMove(GtkWidget *widget, gpointer data) {
  timer_counter = 2;
  if (GTK_IS_WIDGET(ihm_fullScreenHBox)) {
    gtk_widget_show(ihm_fullScreenHBox);
  }
}

static void ihm_QuitFullscreenRequest(GtkWidget *widget, gpointer data) {
  gtk_widget_destroy(fullscreen_window);
}

static void ihm_VideoFullScreen(GtkWidget *widget, gpointer data) {
  int w, h;

  if (fullscreen != NULL) {
    printf("   Already fullscreen\n");
    return;
  }
  printf("   Go Fullscreen\n");

  /* Builds the image */
  fullscreen_image = (GtkImage*) gtk_image_new();
  fullscreen_eventbox = gtk_event_box_new();
  //align = gtk_alignment_new(0.5f,0.5f,0.0f,0.0f);


  /* Add three buttons on the fullscreen window */
  ihm_fullScreenHBox = gtk_hbox_new(FALSE, 0);

  ihm_fullScreenButton[0] = gtk_button_new_with_label(ihm_ImageButtonCaption[RAW_CAPTURE_BUTTON]);
  g_signal_connect(G_OBJECT(ihm_fullScreenButton[0]), "clicked", (GCallback) ihm_RAWCapture, NULL);
  gtk_container_add(GTK_CONTAINER(ihm_fullScreenHBox), ihm_fullScreenButton[0]);

  ihm_fullScreenButton[1] = gtk_button_new_with_label(ihm_ImageButtonCaption[ZAPPER_BUTTON]);
  g_signal_connect(G_OBJECT(ihm_fullScreenButton[1]), "clicked", (GCallback) ihm_Zapper, NULL);
  gtk_container_add(GTK_CONTAINER(ihm_fullScreenHBox), ihm_fullScreenButton[1]);

  ihm_fullScreenButton[2] = gtk_button_new_with_label("Quit Fullscreen");
  g_signal_connect(G_OBJECT(ihm_fullScreenButton[2]), "clicked", (GCallback) ihm_QuitFullscreenRequest, NULL);
  gtk_container_add(GTK_CONTAINER(ihm_fullScreenHBox), ihm_fullScreenButton[2]);

  //ihm_fullScreenButton[3] = gtk_button_new(); // Fake button

  //gtk_container_add(GTK_CONTAINER (align),ihm_fullScreenHBox);

  /* Create window (full screen) */
  fullscreen_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  /* the screen */
  fullscreen = gtk_window_get_screen(GTK_WINDOW(fullscreen_window));
  w = gdk_screen_get_width(fullscreen);
  h = gdk_screen_get_height(fullscreen);
  gtk_widget_set_size_request(GTK_WIDGET(fullscreen_window), w, h);

  /* The fixed container */
  ihm_fullScreenFixedContainer = gtk_fixed_new();
  gtk_fixed_put((GtkFixed*) (ihm_fullScreenFixedContainer), GTK_WIDGET(fullscreen_image), 0, 0);
  gtk_fixed_put((GtkFixed*) (ihm_fullScreenFixedContainer), GTK_WIDGET(ihm_fullScreenHBox), 0, 0);

  /* Build the fullscreen window with the fixed container */
  gtk_container_add(GTK_CONTAINER(fullscreen_eventbox), ihm_fullScreenFixedContainer);
  gtk_container_add(GTK_CONTAINER(fullscreen_window), fullscreen_eventbox);

  gtk_window_set_decorated(GTK_WINDOW(fullscreen_window), FALSE);
  gtk_window_set_resizable(GTK_WINDOW(fullscreen_window), FALSE);

  printf("Fullscreen size : %ix%i\n", w, h);

  g_signal_connect(G_OBJECT(fullscreen_window), "destroy", (GCallback) ihm_VideoFullScreenStop, NULL);
  g_signal_connect(fullscreen_eventbox, "motion_notify_event", (GCallback) ihm_VideoFullScreenMouseMove, NULL);
  gtk_widget_add_events(fullscreen_eventbox, GDK_POINTER_MOTION_MASK);

  gtk_window_fullscreen(GTK_WINDOW(fullscreen_window));
  gtk_widget_show_all(GTK_WIDGET(fullscreen_window));
  gtk_widget_hide(ihm_fullScreenHBox);
  //gtk_widget_get_size_request(ihm_fullScreenHBox,&w2,&h2);
  //printf("Fullscreen size2 : %ix%i    %ix%i\n",w,h,w2,h2);

  //gtk_fixed_put(ihm_fullScreenFixedContainer,ihm_fullScreenHBox,0,h-30);

  if (!flag_timer_is_active) {
    g_timeout_add(1000, (GtkFunction) hide_fullscreen_buttons, NULL);
    flag_timer_is_active = 1;
  }

}


static void ihm_ImageButtonCB(GtkWidget *widget, gpointer data) {
  int button = (int) data;

  printf("   Button clicked No: %d\n", button);

  ardrone_at_set_vision_update_options(button);
}

// void ihm_ImageWinDestroy ( void )
void ihm_ImageWinDestroy(GtkWidget *widget, gpointer data) {
  image_vision_window_status = WINDOW_CLOSED;
  printf("Destroying the Video window.\n");
  if (fullscreen != NULL) {
    ihm_VideoFullScreenStop(NULL, NULL);
  }
  ihm_VideoStream_VBox = NULL; /* this var. is tested by stage Gtk */
  ihm_ImageWin = NULL;
}

gint ihm_ImageWinDelete(GtkWidget *widget, GdkEvent *event, gpointer data) {
  image_vision_window_status = WINDOW_CLOSED;
  printf("Deleting the Video window.\n");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_show_image), FALSE);
  return FALSE;
}

static void ihm_showImage(gpointer pData) {
  GtkWidget* widget = (GtkWidget*) pData;

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    if (!GTK_IS_WIDGET(ihm_ImageWin)) {
      create_image_window(); // Recreate window if it has been killed
    }
    gtk_widget_show_all(ihm_ImageWin);
    image_vision_window_view = WINDOW_VISIBLE;
  }
  else {
    if (GTK_IS_WIDGET(ihm_ImageWin)) {
      gtk_widget_hide_all(ihm_ImageWin);
      image_vision_window_view = WINDOW_HIDE;
    }
  }
}

static void ihm_send_VideoBitrate(GtkWidget *widget, gpointer data) {
  int32_t bitrateValue = atoi(gtk_entry_get_text(GTK_ENTRY(video_bitrateEntry)));
  ARDRONE_TOOL_CONFIGURATION_ADDEVENT(bitrate, &bitrateValue, NULL);
}

static void ihm_send_VideoCodec(GtkComboBox *widget, gpointer data) {
  gint pos;
  int32_t codec;
  pos = gtk_combo_box_get_active( widget );
  codec = (pos == 0) ? UVLC_CODEC : P264_CODEC;
  ARDRONE_TOOL_CONFIGURATION_ADDEVENT(video_codec, &codec, NULL);
}

static void ihm_send_VideoBitrateMode(GtkComboBox *widget, gpointer data) {
  int32_t pos;
  pos = (int32_t)gtk_combo_box_get_active( widget );
  ARDRONE_TOOL_CONFIGURATION_ADDEVENT(bitrate_ctrl_mode, &pos, NULL);
}

void update_vision( void )
{
  if (ihm_ImageWin != NULL && GTK_IS_WIDGET(ihm_ImageWin)) {
    if (image_vision_window_view == WINDOW_VISIBLE) {

      // Vision state refresh
      if (label_vision_values != NULL && GTK_IS_LABEL(label_vision_values))
        gtk_label_set_label(label_vision_values, label_vision_state_value);
      if (ihm_ImageWin != NULL && GTK_IS_WIDGET(ihm_ImageWin))
        gtk_widget_show_all(ihm_ImageWin);
    }
  }
}

void create_image_window( void )
{
  /* Image display main window */
  /* ------------------------- */
  int k;

  printf("Creating the Video window.\n");

  // Image main window
  ihm_ImageWin = gtk_window_new( GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(ihm_ImageWin), 10);
  gtk_window_set_title(GTK_WINDOW(ihm_ImageWin), ihm_ImageTitle);
  gtk_signal_connect(GTK_OBJECT(ihm_ImageWin), "destroy", G_CALLBACK(ihm_ImageWinDestroy), NULL );

  // Boxes
  ihm_ImageVBox = gtk_vbox_new(FALSE, 0);
  ihm_VideoStream_VBox = gtk_vbox_new(FALSE, 0);
  ihm_ImageVBoxPT = gtk_vbox_new(FALSE, 0);

  //hBox_vision_state = gtk_hbox_new(FALSE, 0);
  for (k=0; k<NB_IMAGES_H_BOXES; k++)  ihm_ImageHBox[k] = gtk_hbox_new(FALSE, 0);
  // Frames
  for (k=0; k<NB_IMAGES_FRAMES; k++)  ihm_ImageFrames[k] = gtk_frame_new( ihm_ImageFrameCaption[k] );
  // Entries
  for (k=0; k<NB_IMAGES_ENTRIES; k++) {
    ihm_ImageEntry[k] = gtk_entry_new();
    gtk_widget_set_size_request(ihm_ImageEntry[k], 80, 20);
  }
  // Video Stream

  for (k=0; k<NB_VIDEO_STREAM_WIDGET; k++)  ihm_VideoStreamLabel[k] = gtk_label_new( ihm_ImageVideoStreamCaption[k] );

  video_bitrateEntry =   gtk_entry_new();
  gtk_widget_set_size_request(video_bitrateEntry, 150, 20);

  video_codecList = gtk_combo_box_new_text();
  gtk_combo_box_insert_text( (GtkComboBox*)video_codecList, 0, (const gchar*)"UVLC");
  gtk_combo_box_insert_text( (GtkComboBox*)video_codecList, 1, (const gchar*)"P264");
  gtk_widget_set_size_request(video_codecList, 150, 20);

  video_bitrateModeList = gtk_combo_box_new_text();
  gtk_combo_box_insert_text( (GtkComboBox*)video_bitrateModeList, 0, (const gchar*)"None");
  gtk_combo_box_insert_text( (GtkComboBox*)video_bitrateModeList, 1, (const gchar*)"Adaptative");
  gtk_combo_box_insert_text( (GtkComboBox*)video_bitrateModeList, 2, (const gchar*)"Manual");
  gtk_widget_set_size_request(video_codecList, 150, 20);

  for (k=0; k<NB_IMAGES_ENTRIES; k++)  ihm_ImageLabel[k] = gtk_label_new( ihm_ImageEntryCaption[k] );

  /* Creates buttons and links them to callbacks */
  for (k=0; k<NB_IMAGES_BUTTONS; k++)
  {
    ihm_ImageButton[k] = gtk_button_new();// ihm_ImageButtonCaption[k] );
    gtk_button_set_label((GtkButton*)ihm_ImageButton[k] ,ihm_ImageButtonCaption[k]);

    switch (k)
    {
      case UPDATE_VISION_PARAMS_BUTTON:
        g_signal_connect( G_OBJECT(ihm_ImageButton[k]), "clicked", G_CALLBACK(ihm_sendVisionConfigParams), (gpointer)k );
        break;
      case RAW_CAPTURE_BUTTON:
        g_signal_connect( G_OBJECT(ihm_ImageButton[k]), "clicked", G_CALLBACK(ihm_RAWCapture), (gpointer)k );
        break;
      case ZAPPER_BUTTON:
        g_signal_connect( G_OBJECT(ihm_ImageButton[k]), "clicked", G_CALLBACK(ihm_Zapper), (gpointer)k );
        break;
      case FULLSCREEN_BUTTON:
        g_signal_connect( G_OBJECT(ihm_ImageButton[k]), "clicked", G_CALLBACK(ihm_VideoFullScreen), (gpointer)k );
        break;
      default:
        g_signal_connect( G_OBJECT(ihm_ImageButton[k]), "clicked", G_CALLBACK(ihm_ImageButtonCB), (gpointer)k );
      }
  }

  GdkColor color;
  gdk_color_parse ("red", &color);
  gtk_widget_modify_text (ihm_ImageLabel[RAW_CAPTURE_BUTTON], GTK_STATE_NORMAL, &color);

  video_bitrateButton = gtk_button_new_with_label( "Send" );
  g_signal_connect(G_OBJECT(video_bitrateButton), "clicked", G_CALLBACK(ihm_send_VideoBitrate), 0 );
  g_signal_connect(G_OBJECT(video_codecList), "changed", G_CALLBACK(ihm_send_VideoCodec), 0 );
  g_signal_connect(G_OBJECT(video_bitrateModeList), "changed", G_CALLBACK(ihm_send_VideoBitrateMode), 0 );

  /* Creates input boxes (aka. entries) */
  char label_vision_default_val[NB_IMAGES_ENTRIES] ;
  tab_vision_config_params[0] = DEFAULT_CS;
  tab_vision_config_params[1] = DEFAULT_NB_PAIRS;
  tab_vision_config_params[2] = DEFAULT_LOSS_PER;
  tab_vision_config_params[3] = DEFAULT_NB_TRACKERS_WIDTH;
  tab_vision_config_params[4] = DEFAULT_NB_TRACKERS_HEIGHT;
  tab_vision_config_params[5] = DEFAULT_SCALE;
  tab_vision_config_params[6] = DEFAULT_TRANSLATION_MAX;
  tab_vision_config_params[7] = DEFAULT_MAX_PAIR_DIST;
  tab_vision_config_params[8] = DEFAULT_NOISE;

  for (k=0; k<NB_IMAGES_ENTRIES; k++)  {
    if (k==FAKE_ENTRY) continue;
    sprintf(label_vision_default_val, "%d", tab_vision_config_params[k]);
    gtk_entry_set_text( GTK_ENTRY(ihm_ImageEntry[k]), label_vision_default_val);
  }
  gtk_entry_set_text( GTK_ENTRY(video_bitrateEntry), "frame size (bytes)");

  /* Builds the vision state frame */
  vp_os_memset(label_vision_state_value, 0, sizeof(label_vision_state_value));
  strcat(label_vision_state_value, "Not Connected");
  label_vision_values = (GtkLabel*) gtk_label_new(label_vision_state_value);

  gtk_container_add( GTK_CONTAINER(ihm_ImageFrames[STATE_FRAME]), (GtkWidget*) label_vision_values );

  /* Builds the vision parameters frame */

  /* First line of parameters */
  for (k=CS_ENTRY; k<NB_TH_ENTRY; k++)  {
    gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[TRACKING_PARAM_HBOX1]), ihm_ImageLabel[k], FALSE , FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[TRACKING_PARAM_HBOX1]), ihm_ImageEntry[k], FALSE , FALSE, 0);
  }
  /* Second line of parameters */
  for (k=NB_TH_ENTRY; k<NB_IMAGES_ENTRIES; k++)  {
    if (k==FAKE_ENTRY) continue;
    gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[TRACKING_PARAM_HBOX2]), ihm_ImageLabel[k], FALSE , FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[TRACKING_PARAM_HBOX2]), ihm_ImageEntry[k], FALSE , FALSE, 0);
  }

  gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[TRACKING_PARAM_HBOX2]), ihm_ImageLabel[FAKE_ENTRY], FALSE , FALSE, 0); // To fill space
  /* Fuses the two line in a single VBox */
  gtk_box_pack_start(GTK_BOX(ihm_ImageVBoxPT), ihm_ImageHBox[TRACKING_PARAM_HBOX1], FALSE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX(ihm_ImageVBoxPT), ihm_ImageHBox[TRACKING_PARAM_HBOX2], FALSE , FALSE, 0);

  /* Builds the whole parameter block */
  gtk_container_add(GTK_CONTAINER(ihm_ImageFrames[TRACKING_PARAMETERS_FRAME]), ihm_ImageVBoxPT );
  gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[TRACKING_PARAMS_HBOX]), ihm_ImageFrames[TRACKING_PARAMETERS_FRAME], FALSE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[TRACKING_PARAMS_HBOX]), ihm_ImageButton[UPDATE_VISION_PARAMS_BUTTON], TRUE  , FALSE, 5);

  for (k=TZ_KNOWN_BUTTON; k<=SE3_BUTTON; k++)
    gtk_box_pack_start(GTK_BOX( ihm_ImageHBox[TRACKING_OPTION_HBOX]), ihm_ImageButton[k], TRUE , FALSE, 0);
  for (k=PROJ_OVERSCENE_BUTTON; k<=FLAT_GROUND_BUTTON; k++)
    gtk_box_pack_start(GTK_BOX( ihm_ImageHBox[COMPUTING_OPTION_HBOX]), ihm_ImageButton[k], TRUE , FALSE, 0);


  gtk_box_pack_start(GTK_BOX( ihm_ImageHBox[VIDEO_STREAM_HBOX]), ihm_VideoStreamLabel[CODEC_TYPE_LIST], FALSE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX( ihm_ImageHBox[VIDEO_STREAM_HBOX]), video_codecList, TRUE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX( ihm_ImageHBox[VIDEO_STREAM_HBOX]), ihm_VideoStreamLabel[BITRATE_MODE_LIST], FALSE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX( ihm_ImageHBox[VIDEO_STREAM_HBOX]), video_bitrateModeList, TRUE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX( ihm_ImageHBox[VIDEO_STREAM_HBOX]), ihm_VideoStreamLabel[MANUAL_BITRATE_ENTRY], FALSE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX( ihm_ImageHBox[VIDEO_STREAM_HBOX]), video_bitrateEntry, TRUE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX( ihm_ImageHBox[VIDEO_STREAM_HBOX]), video_bitrateButton, TRUE , FALSE, 0);

  /* */
  gtk_container_add(GTK_CONTAINER( ihm_ImageFrames[TRACKING_OPTION_FRAME]) , ihm_ImageHBox[TRACKING_OPTION_HBOX] );
  gtk_container_add(GTK_CONTAINER( ihm_ImageFrames[COMPUTING_OPTION_FRAME]), ihm_ImageHBox[COMPUTING_OPTION_HBOX] );
  gtk_container_add(GTK_CONTAINER( ihm_ImageFrames[VIDEO_STREAM_FRAME])    , ihm_ImageHBox[VIDEO_STREAM_HBOX] );

  /* Frame where to show buttons controlling how the drone video is displayed */
  displayvbox = gtk_vbox_new(FALSE,0);

  gtk_box_pack_start(GTK_BOX(displayvbox), ihm_ImageButton[RAW_CAPTURE_BUTTON], FALSE  , FALSE, 5);
  gtk_box_pack_start(GTK_BOX(displayvbox), ihm_ImageButton[ZAPPER_BUTTON], FALSE  , FALSE, 5);
  gtk_box_pack_start(GTK_BOX(displayvbox), ihm_ImageButton[FULLSCREEN_BUTTON], FALSE  , FALSE, 5);

  gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[VIDEO_DISPLAY_HBOX]),ihm_VideoStream_VBox,FALSE,FALSE,5);
  gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[VIDEO_DISPLAY_HBOX]),displayvbox,FALSE,FALSE,5);

  gtk_container_add(GTK_CONTAINER( ihm_ImageFrames[VIDEO_DISPLAY_FRAME])    , ihm_ImageHBox[VIDEO_DISPLAY_HBOX] );

  /* Builds the final window */
  gtk_box_pack_start(GTK_BOX(ihm_ImageVBox), ihm_ImageFrames[VIDEO_DISPLAY_FRAME], FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(ihm_ImageVBox), ihm_ImageFrames[STATE_FRAME], FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(ihm_ImageVBox), ihm_ImageHBox[TRACKING_PARAMS_HBOX], FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(ihm_ImageVBox), ihm_ImageFrames[TRACKING_OPTION_FRAME], FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(ihm_ImageVBox), ihm_ImageFrames[COMPUTING_OPTION_FRAME], FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(ihm_ImageVBox), ihm_ImageFrames[VIDEO_STREAM_FRAME], FALSE, FALSE, 5);

  gtk_container_add(GTK_CONTAINER(ihm_ImageWin), ihm_ImageVBox);
  image_vision_window_view = WINDOW_HIDE;
  image_vision_window_status = WINDOW_OPENED;

  /* Set the callback for the checkbox inside the main application window */
  g_signal_connect(G_OBJECT(button_show_image), "clicked", G_CALLBACK(ihm_showImage), (gpointer) ihm_ImageWin);
  g_signal_connect(G_OBJECT(button_show_image2), "clicked", G_CALLBACK(ihm_showImage), (gpointer) ihm_ImageWin);
}
