#include <sys/time.h>
#include <time.h>

#include <ardrone_tool/Navdata/ardrone_navdata_file.h>

#include "navdata_client/navdata_ihm.h"
#include <ihm/ihm.h>
#include <libPolaris/polaris.h>

#ifdef PC_USE_POLARIS
static vp_os_mutex_t static_POLARIS_mutex;
vp_os_mutex_t *POLARIS_mutex=&static_POLARIS_mutex;
POLARIS_data data_POLARIS;

static int   POLARIS_val_idx=0;

C_RESULT navdata_polaris_init( void* data )
{
  return C_OK;
}

C_RESULT navdata_polaris_process( const navdata_unpacked_t* const navdata )
{
  struct timeval tv1;
  static struct timeval old_tv1;
  static float32_t polaris_old_x = 0.0f;
  static float32_t polaris_old_y = 0.0f;
  static float32_t polaris_old_z = 0.0f;
  static float32_t polaris_old_vx = 0.0f;
  static float32_t polaris_old_vy = 0.0f;
  static float32_t polaris_old_vz = 0.0f;
  static float32_t polaris_old_qx = 0.0f;
  static float32_t polaris_old_qy = 0.0f;
  float32_t polaris_delta_t_s = 0.0f;
  float32_t polaris_cos_psi = 0.0f;
  float32_t polaris_sin_psi = 0.0f;

  if( navdata_file != NULL )
  {
    gettimeofday( &tv1, NULL );
		polaris_delta_t_s=((tv1.tv_sec*1000000+tv1.tv_usec)/1000000.0)-((old_tv1.tv_sec*1000000+old_tv1.tv_usec)/1000000.0);
		old_tv1=tv1;

    vp_os_mutex_lock(POLARIS_mutex);

    if( BAD_FLOAT != data_POLARIS.y ) {
      fprintf(navdata_file,"; % 4.2f; % 4.2f; % 4.2f; % 4.2f; % 4.2f; % 4.2f; % 4.2f; % d; % 6d", 
        (double) (data_POLARIS.x),
        (double) (data_POLARIS.y),
        (double) (data_POLARIS.z),
        (double) (data_POLARIS.qx),
        (double) (data_POLARIS.qy),
        (double) (data_POLARIS.qz),
        (double) (data_POLARIS.q0),
        (int)tv1.tv_sec,
        (int)tv1.tv_usec);
    }
    else {
      fprintf(navdata_file,"; % 4.2f; % 4.2f; % 4.2f; % 4.2f; % 4.2f; % 4.2f; % 4.2f; % d; % 6d", 
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, (int32_t)tv1.tv_sec, (int32_t)tv1.tv_usec);
    }
    vp_os_mutex_unlock(POLARIS_mutex);
  }

	do {
		if (--POLARIS_val_idx < 0) POLARIS_val_idx = KIHM_N_PT2PLOT-1;

		ihm_CA[KIHM_CURVE_VX].tval[2][POLARIS_val_idx] = polaris_old_vx;
		ihm_CA[KIHM_CURVE_VY].tval[2][POLARIS_val_idx] = polaris_old_vy;
		ihm_CA[KIHM_CURVE_VZ].tval[2][POLARIS_val_idx] = polaris_old_vz;
		ihm_CA[KIHM_CURVE_PHI].tval[5][POLARIS_val_idx] = polaris_old_qx;
		ihm_CA[KIHM_CURVE_THETA].tval[5][POLARIS_val_idx] = polaris_old_qy;

		if (BAD_FLOAT==data_POLARIS.y) {
			ihm_CA[KIHM_CURVE_VX].tval[2][POLARIS_val_idx] = 0.0f;
			ihm_CA[KIHM_CURVE_VY].tval[2][POLARIS_val_idx] = 0.0f;
			ihm_CA[KIHM_CURVE_VZ].tval[2][POLARIS_val_idx] = 0.0f;
		}
		else if(data_POLARIS.y != polaris_old_y) {
			sincosf(data_POLARIS.qz * DEG_TO_RAD, &polaris_sin_psi, &polaris_cos_psi);
			ihm_CA[KIHM_CURVE_VX].tval[2][POLARIS_val_idx] = (double) (polaris_cos_psi * (data_POLARIS.x - polaris_old_x)
					+polaris_sin_psi * (data_POLARIS.y - polaris_old_y))
				/polaris_delta_t_s ; // POLARIS en x
			polaris_old_x = (double) (data_POLARIS.x);
			polaris_old_vx = ihm_CA[KIHM_CURVE_VX].tval[2][POLARIS_val_idx];


			ihm_CA[KIHM_CURVE_VY].tval[2][POLARIS_val_idx] =  (double) (-polaris_sin_psi * (data_POLARIS.x - polaris_old_x)
					+polaris_cos_psi * (data_POLARIS.y - polaris_old_y))
				/polaris_delta_t_s ; // POLARIS en y
			polaris_old_y = (double) (data_POLARIS.y);
			polaris_old_vy = ihm_CA[KIHM_CURVE_VY].tval[2][POLARIS_val_idx];

			ihm_CA[KIHM_CURVE_VZ].tval[2][POLARIS_val_idx] = (double) (data_POLARIS.z - polaris_old_z)/polaris_delta_t_s; // POLARIS en z
			polaris_old_z = (double) (data_POLARIS.z);
			polaris_old_vz = ihm_CA[KIHM_CURVE_VZ].tval[2][POLARIS_val_idx];
			ihm_CA[KIHM_CURVE_PHI].tval[5][POLARIS_val_idx] = (double) (data_POLARIS.qx); // POLARIS en qx
			polaris_old_qx =(double) (data_POLARIS.qx);
			ihm_CA[KIHM_CURVE_THETA].tval[5][POLARIS_val_idx] = (double) (data_POLARIS.qy); // POLARIS en qy
			polaris_old_qy = (double) (data_POLARIS.qy); 
		};
	} while (POLARIS_val_idx!=ihm_val_idx);	


  return C_OK;
}

C_RESULT navdata_polaris_release( void )
{
  return C_OK;
}

#endif
