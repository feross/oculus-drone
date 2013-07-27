/**
 * @file mobile_config.c
 * @author aurelien.morelle@parrot.fr
 * @date 2006/05/02
 */

#ifdef _INCLUDED_FOR_DOXYGEN_

#else // ! _INCLUDED_FOR_DOXYGEN_

#include <VP_Api/vp_api_picture.h>
#include <VP_Stages/vp_stages_i_camif.h>
#include <linux/joystick.h>
#include "common/mobile_config.h"
#include "UI/ui.h"

#endif // >  _INCLUDED_FOR_DOXYGEN_

static vp_os_mutex_t static_picture_mutex;
static vp_os_mutex_t static_key_mutex;
static vp_os_mutex_t static_exit_mutex;

int mobile_init_config(mobile_config_t *cfg)
{
  cfg->ihm_curve_alloc_OK = 0;

  cfg->picture_mutex = &static_picture_mutex;
  vp_os_mutex_init(cfg->picture_mutex);

  cfg->key_mutex = &static_key_mutex;
  vp_os_mutex_init(cfg->key_mutex);

  cfg->exit_mutex = &static_exit_mutex;
  vp_os_mutex_init(cfg->exit_mutex);

  cfg->width  = STREAM_WIDTH;
  cfg->height = STREAM_HEIGHT;

  cfg->new_frame = 0;

  cfg->exit_required = 0;

  cfg->speed = 0;
  cfg->angle = 0;

  cfg->speed_list = NULL;

  cfg->up_pressed = 0;
  cfg->down_pressed = 0;
  cfg->left_pressed = 0;
  cfg->right_pressed = 0;

  cfg->manage_gamepad_state = NULL;

  // cfg->sample_file = fopen(SAMPLE_FILENAME, "wb");

  return 0;
}

void mobile_exit_config(mobile_config_t *cfg)
{
  vp_os_mutex_destroy(cfg->picture_mutex);
  vp_os_mutex_destroy(cfg->key_mutex);
  vp_os_mutex_destroy(cfg->exit_mutex);

  if(cfg->speed_list)
    vp_os_free(cfg->speed_list);

  cfg->picture_mutex = NULL;
  cfg->key_mutex = NULL;
  cfg->exit_required = 1;

  cfg->speed = 0;
  cfg->angle = 0;

  cfg->speed_list = NULL;

  cfg->up_pressed = 0;
  cfg->down_pressed = 0;
  cfg->left_pressed = 0;
  cfg->right_pressed = 0;

#ifdef WRITE_TO_FILE
  fclose(cfg->sample_file);
#endif

  if(cfg->joydev != -1)
    close(cfg->joydev);
  cfg->joydev = -1;
  cfg->pool_events = NULL;
}

int do_exit(mobile_config_t *cfg)
{
  vp_os_mutex_lock(cfg->exit_mutex);
/*     { */
/*       fprintf(stderr, "Couldn't lock mutex\n"); */
/*       return -1; */
/*     } */

  /* Do stuff while mutex is locked */
  cfg->exit_required = 1;

  vp_os_mutex_unlock(cfg->exit_mutex);
/*     { */
/*       fprintf(stderr, "Couldn't unlock mutex\n"); */
/*       return -1; */
/*     } */

  return 0;
}

int test_exit(mobile_config_t *cfg)
{
  int res;

  vp_os_mutex_lock(cfg->exit_mutex);
/*     { */
/*       fprintf(stderr, "Couldn't lock mutex\n"); */
/*       return -1; */
/*     } */

  res = cfg->exit_required;

  vp_os_mutex_unlock(cfg->exit_mutex);
/*     { */
/*       fprintf(stderr, "Couldn't unlock mutex\n"); */
/*       return -1; */
/*     } */

  return res;
}

