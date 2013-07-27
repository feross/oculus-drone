#ifndef _IHM_STAGES_O_GTK_H
#define _IHM_STAGES_O_GTK_H

#include <config.h>
#include <VP_Api/vp_api_thread_helper.h>
#ifdef PC_USE_VISION
#    include <Vision/vision_tracker_engine.h>
#    include <Vision/vision_patch_detect.h>
#	 include <VLIB/video_controller.h>
#	 include <ardrone_tool/Video/vlib_stage_decode.h>
#endif

#define NUM_MAX_SCREEN_POINTS (DEFAULT_NB_TRACKERS_WIDTH * DEFAULT_NB_TRACKERS_HEIGHT)

#ifdef PC_USE_VISION
typedef struct _vp_stages_draw_trackers_config_t {

  int32_t         num_points;
  uint32_t        locked[NUM_MAX_SCREEN_POINTS];
  screen_point_t  points[NUM_MAX_SCREEN_POINTS];

  uint32_t         detected;
  patch_ogb_type_t type[4];
  screen_point_t   patch_center[4];
  uint32_t         width[4];
  uint32_t         height[4];

  vlib_stage_decoding_config_t * last_decoded_frame_info;

} vp_stages_draw_trackers_config_t;

void set_draw_trackers_config(vp_stages_draw_trackers_config_t* cfg);
#endif

typedef  struct _vp_stages_gtk_config_ {
  int max_width;
  int max_height;
  int rowstride;
  void * last_decoded_frame_info;
}  vp_stages_gtk_config_t;

void update_vision( void );

void *ihm_stages_vision(void *data);

PROTO_THREAD_ROUTINE(ihm_stages_vision, data);

#endif // _IHM_STAGES_O_GTK_H
