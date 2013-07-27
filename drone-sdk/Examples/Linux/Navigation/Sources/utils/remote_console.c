
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>

#include <VP_Api/vp_api.h>
#include <VP_Api/vp_api_error.h>
#include <VP_Stages/vp_stages_configs.h>
#include <VP_Stages/vp_stages_io_console.h>
#include <VP_Stages/vp_stages_io_com.h>
#include <VP_Stages/vp_stages_io_file.h>
#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_malloc.h>

#include <ardrone_tool/Com/config_com.h>
#include <ardrone_tool/ardrone_tool.h>


#define NB_STAGES 3


static PIPELINE_HANDLE pipeline_handle;
static char filename[100];


DEFINE_THREAD_ROUTINE(remote_console, data)
{
  vp_api_io_pipeline_t    pipeline;
  vp_api_io_data_t        out;
  vp_api_io_stage_t       stages[NB_STAGES];

  vp_stages_input_com_config_t     icc;
  vp_stages_output_file_config_t   ofc;

  struct timeval tv;
  struct tm atm;

  gettimeofday(&tv,NULL);
  localtime_r(&tv.tv_sec, &atm);
  sprintf(&filename[0], "logs_%04d%02d%02d_%02d%02d%02d.txt",
          atm.tm_year+1900, atm.tm_mon+1, atm.tm_mday,
          atm.tm_hour, atm.tm_min, atm.tm_sec);

  vp_os_memset( &icc,         0, sizeof(vp_stages_input_com_config_t));
  vp_os_memset( &ofc,         0, sizeof(vp_stages_output_file_config_t));

  icc.com                 = COM_REMOTE_CONSOLE();
  icc.config              = COM_CONFIG_REMOTE_CONSOLE();
  icc.connection          = COM_CONNECTION_REMOTE_CONSOLE();
  icc.buffer_size         = 1024;
  COM_CONFIG_SOCKET_REMOTE_CONSOLE(&icc.socket, VP_COM_CLIENT, PRINTF_PORT, wifi_ardrone_ip);

  ofc.name = &filename[0];

  stages[0].type = VP_API_INPUT_SOCKET;
  stages[0].cfg = (void *)&icc;
  stages[0].funcs = vp_stages_input_com_funcs;

  stages[1].type = VP_API_OUTPUT_CONSOLE;
  stages[1].cfg = NULL;
  stages[1].funcs = vp_stages_output_console_funcs;

  stages[2].type = VP_API_OUTPUT_FILE;
  stages[2].cfg = (void *)&ofc;
  stages[2].funcs = vp_stages_output_file_funcs;

  pipeline.nb_stages = NB_STAGES;
  pipeline.stages = &stages[0];

  if(SUCCEED(vp_api_open(&pipeline, &pipeline_handle)))
    {
      out.status = VP_API_STATUS_PROCESSING;
      while(SUCCEED(vp_api_run(&pipeline, &out)) && (out.status == VP_API_STATUS_PROCESSING || out.status == VP_API_STATUS_STILL_RUNNING) && !ardrone_tool_exit());
      vp_api_close(&pipeline, &pipeline_handle);
    }

  return EXIT_SUCCESS;
}

