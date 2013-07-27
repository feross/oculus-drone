/*
 * @raw_capture.c
 * @author aurelien.morelle@parrot.com
 * @date 2008/05/29
 *
 * Raw video capture from Mykonos RAM
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <VP_Api/vp_api.h>
#include <VP_Api/vp_api_thread_helper.h>
#include <VP_Api/vp_api_error.h>
#include <VP_Api/vp_api_stage.h>
#include <VP_Stages/vp_stages_io_com.h>
#include <VP_Stages/vp_stages_io_file.h>

#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_malloc.h>

#include <ardrone_tool/ardrone_tool.h>
#include <ardrone_tool/Com/config_com.h>
#include <Vision/vision_stage.h>

#define NB_STAGES_MAX 10

PIPELINE_HANDLE raw_capture_pipeline_handle;


// useful ?
extern int exit_ihm_program;


C_RESULT
raw_capture_output_file_stage_open(vp_stages_output_file_config_t *cfg)
{
  return (SUCCESS);
}


C_RESULT
raw_capture_output_file_stage_transform(vp_stages_output_file_config_t *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  static int total_size = 0, local_size, size2read;
  static int start = 1;

  static struct {
    int32_t size2read;
    vp_stages_vision_config_t saved_cfg;
  } special_struct;

  struct timeval tv;
  struct tm atm;

  static char raw_filename[256];
  char struct_filename[256];
  FILE *f;

  vp_os_mutex_lock(&out->lock);

  if(in->status == VP_API_STATUS_PROCESSING && in->size > 0)
    {
      local_size = in->size;

      if(start)
	{
	  if(!cfg->f)
	    {
	      gettimeofday(&tv,NULL);
	      localtime_r(&tv.tv_sec, &atm);
	      sprintf(&raw_filename[0], "raw_%04d%02d%02d_%02d%02d%02d.yuv", atm.tm_year+1900, atm.tm_mon+1, atm.tm_mday, atm.tm_hour, atm.tm_min, atm.tm_sec);
	      sprintf(&struct_filename[0], "vision_state_%04d%02d%02d_%02d%02d%02d.raw", atm.tm_year+1900, atm.tm_mon+1, atm.tm_mday, atm.tm_hour, atm.tm_min, atm.tm_sec);
	      cfg->name = &raw_filename[0];
	      cfg->f = fopen(cfg->name, "wb");
	      PRINT("Open %s                      \n", cfg->name);
	    }

	  if(total_size < sizeof(special_struct))
	    {
	      vp_os_memcpy(total_size+(int8_t*)&special_struct, &in->buffers[in->indexBuffer][0], min(sizeof(special_struct)-total_size, local_size));
	      local_size -= min(sizeof(special_struct)-total_size, in->size);
	      total_size += min(sizeof(special_struct)-total_size, in->size);
	    }

	  if(total_size == sizeof(special_struct))
	    {
	      size2read = special_struct.size2read;
	      f = fopen(&struct_filename[0], "wb");
	      fwrite(&special_struct.saved_cfg, sizeof(int8_t), sizeof(special_struct.saved_cfg), f);
	      fclose(f);
	      start = 0;
	      total_size = 0;
	    }
	}

      if(start == 0 && local_size > 0)
	{
	  fwrite(&in->buffers[in->indexBuffer][0], sizeof(int8_t), local_size*sizeof(int8_t), cfg->f);
	  total_size += local_size;
	  PRINT("red size = %07d / total size = %07d\r", total_size, size2read);
	  if(total_size == size2read)
	    {
	      total_size = 0;
	      start = 1;
	      fclose(cfg->f);
	      cfg->f = NULL;
	      PRINT("Close %s                    \n", cfg->name);
	    }
	}
    }

  out->status = in->status;

  vp_os_mutex_unlock(&out->lock);

  return (SUCCESS);
}


C_RESULT
raw_capture_output_file_stage_close(vp_stages_output_file_config_t *cfg)
{
  return (SUCCESS);
}


const vp_api_stage_funcs_t raw_capture_output_file_funcs =
{
  (vp_api_stage_handle_msg_t) NULL,
  (vp_api_stage_open_t) raw_capture_output_file_stage_open,
  (vp_api_stage_transform_t) raw_capture_output_file_stage_transform,
  (vp_api_stage_close_t) raw_capture_output_file_stage_close
};


DEFINE_THREAD_ROUTINE(raw_capture, data)
{
  C_RESULT res;

  vp_api_io_pipeline_t    pipeline;
  vp_api_io_data_t        out;
  vp_api_io_stage_t       stages[NB_STAGES_MAX];

  vp_stages_input_com_config_t    icc;
  vp_stages_output_file_config_t  ofc;

  vp_os_memset(&icc,        0, sizeof( icc ));
  vp_os_memset(&ofc,        0, sizeof( ofc ));

  icc.com                 = COM_RAW_CAPTURE();
  icc.config              = COM_CONFIG_RAW_CAPTURE();
  icc.connection          = COM_CONNECTION_RAW_CAPTURE();
  icc.buffer_size         = 16*16*3;
  COM_CONFIG_SOCKET_VIDEO(&icc.socket, VP_COM_CLIENT, RAW_CAPTURE_PORT, wifi_ardrone_ip);

  ofc.name = "toto.out";

  pipeline.nb_stages = 0;

  stages[pipeline.nb_stages].type    = VP_API_INPUT_SOCKET;
  stages[pipeline.nb_stages].cfg     = (vp_stages_input_com_config_t *)&icc;
  stages[pipeline.nb_stages++].funcs = vp_stages_input_com_funcs;

  stages[pipeline.nb_stages].type    = VP_API_OUTPUT_FILE;
  stages[pipeline.nb_stages].cfg     = (vp_stages_output_file_config_t *)&ofc;
  stages[pipeline.nb_stages++].funcs = raw_capture_output_file_funcs;

  pipeline.stages = &stages[0];

  PRINT("\n   Raw video capture thread initialisation\n\n");

  res = vp_api_open(&pipeline, &raw_capture_pipeline_handle);

  if( SUCCEED(res) )
  {
    int loop = SUCCESS;
    out.status = VP_API_STATUS_PROCESSING;

    while( !ardrone_tool_exit() && (loop == SUCCESS) )
    {
      if( SUCCEED(vp_api_run(&pipeline, &out)) ) {
	if( (out.status == VP_API_STATUS_PROCESSING || out.status == VP_API_STATUS_STILL_RUNNING) ) {
	  loop = SUCCESS;
	}
        else loop = -1; // Finish this thread
      }
    }

    vp_api_close(&pipeline, &raw_capture_pipeline_handle);
  }

  PRINT("\n   Raw video capture thread closed\n\n");

  return (THREAD_RET)0;
}

