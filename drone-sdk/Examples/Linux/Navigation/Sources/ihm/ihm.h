
#ifndef IHM_H
#define IHM_H

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <ardrone_api.h>
#include <VP_Api/vp_api_thread_helper.h>
#include <VP_Os/vp_os_signal.h>

#include <ardrone_tool/Control/ardrone_control.h>
#include <ardrone_tool/Control/ardrone_control_configuration.h>

// shared data allocation management
#ifdef KIHM_ALLOC_SHARED_DATA   // defined only in ihm.c
#define KIHM_EXTERN
#else
#define KIHM_EXTERN extern
#endif


/* general declarations */
/* -------------------- */
extern int32_t ihm_val_idx;
KIHM_EXTERN int ihm_freeze_curves;
KIHM_EXTERN double ihm_psi;
KIHM_EXTERN double ihm_dir;

/* constants */
#define KIHM_PI              3.14159
#define KIHM_ZERO_F          0.0
#define KIHM_MIL_F           1000.0

/* ctrl states  */  
#define MAX_STR_CTRL_STATE 256

// graphic contexts
enum {
  KIHM_GC_RED, KIHM_GC_GREEN, KIHM_GC_BLUE, KIHM_GC_LIGHTRED, KIHM_GC_LIGHTBLUE, KIHM_GC_GREY, KIHM_GC_LIGHTGREY, KIHM_GC_DASHLINE, KIHM_GC_BLACK,  KIHM_GC_WHITE, KIHM_NB_GC
};
KIHM_EXTERN GdkGC *ihm_GC[KIHM_NB_GC];



/* declarations for attitude display */
/* --------------------------------- */
enum {
  KIHM_DA_ATT_PITCH,
  KIHM_DA_ATT_ROLL,
  KIHM_DA_ATT_YAW,
  KIHM_DA_ATT_DIR,
  KIHM_NB_DA_ATT
};
KIHM_EXTERN GtkWidget *ihm_DA_att[KIHM_NB_DA_ATT];
KIHM_EXTERN PangoLayout  *ihm_PL_DApsi, *ihm_PL_DAtheta, *ihm_PL_DAphi, *ihm_PL_DAdir;

enum {
  ARDRONE_GLOBAL_STATE,
  ARDRONE_CONTROL_STATE,
  ARDRONE_VISION_STATE,
  NB_ARDRONE_STATES
};

/* declarations for altitude display */
/* --------------------------------- */
enum {
  KIHM_AKP,
  KIHM_AKI,
  KIHM_AKD,
  KIHM_ATD,
  NB_GAINS_ALT
};

/* declarations for fix point display */
/* ---------------------------------- */
enum {
  KIHM_FPKP,
  KIHM_FPKI,
  NB_GAINS_FP
};

/* declarations for cocarde control display */
/* ---------------------------------------- */
enum {
  KIHM_ROUNDELKP,
  KIHM_ROUNDELKI,
  NB_GAINS_ROUNDEL
};


/* declarations for control gains */
/* ------------------------------ */
#define SEND_PID_GAIN
#define NB_GAINS_W   3
#define NB_GAINS_EA  2
#define NB_GAINS     (NB_GAINS_W + NB_GAINS_EA)
#define NB_ALL_GAINS (NB_GAINS + NB_GAINS_ALT + NB_GAINS_FP + NB_GAINS_ROUNDEL)

/* Stephane : number of samples to display in detection history */
#define NB_DETECTION_SAMPLES 30


/* declarations for window size */
/* ---------------------------- */
#define KIHM_DAREA_ANGLE_X_SIZE 230
#define KIHM_DAREA_ANGLE_Y_SIZE 100
#define KIHM_DAREA_CURVE_X_SIZE 700
#define KIHM_DAREA_CURVE_Y_SIZE 170
#define KIHM_DAREA_CURVE_Y_OFFSET    8 // = half the height of a graduation label

#define KIHM_N_PT2PLOT   KIHM_DAREA_CURVE_X_SIZE

#define KIHM_BORDER      10
/* #define KIHM_WX          3*KIHM_DAREA_ANGLE_X_SIZE+2*KIHM_BORDER+50 */
/* #define KIHM_WY          KIHM_DAREA_ANGLE_Y_SIZE+2*KIHM_DAREA_CURVE_Y_SIZE+2*KIHM_BORDER */


#define GYRO_WIN_TITLE      "Angular rates (gyrometers measures) in deg/s"
#define ACC_WIN_TITLE       "Accelerations (accelerometers measures) in g"
#define THETA_WIN_TITLE     "Theta (pitch) in deg"
#define PHI_WIN_TITLE       "Phi (roll) in deg"
#define IMOT_WIN_TITLE      "Imot in ADC LSB"
#define VBAT_WIN_TITLE      "Vbat in V"
#define PWM_WIN_TITLE       "PWM (0 < ratio < 255)"
#define CURRENT_WIN_TITLE   "Motors currents in mA"
#define VX_WIN_TITLE        "VX : speed in mm/s"
#define VY_WIN_TITLE        "VY : speed in mm/s"
#define VZ_WIN_TITLE        "VZ : speed in mm/s"
#define ALTITUDE_WIN_TITLE  "Altitude : Reference and measure in mm"

#define NOT_DEF_VAL 0xdeadbeaf
#define HOVER_VAL   0xdecafef


extern GtkWidget *entry_PID[NB_ALL_GAINS], *entry_MiscVar[NB_MISC_VARS];


/* declarations for curves */
/* ----------------------- */
enum {
  KIHM_CURVE_ACC = 0,
  KIHM_CURVE_GYRO,
  KIHM_CURVE_THETA,
  KIHM_CURVE_PHI,
  KIHM_CURVE_VBAT,
  KIHM_CURVE_PWM,
  KIHM_CURVE_ALTITUDE,
  KIHM_CURVE_VX,
  KIHM_CURVE_VY,
  KIHM_CURVE_VZ,
  KIHM_CURVE_CURRENT,
  KIHM_NB_CURVES
};

/* dock status of a curve */
/* -----------------------*/
enum {
  CURVE_DOCKED   ,
  CURVE_UNDOCKED ,
  WINDOW_VISIBLE ,
  WINDOW_HIDE,
  WINDOW_OPENED,
  WINDOW_CLOSED
};

typedef struct _SIHM_ScaleAttributes
{
  double phys_range, phys_orig, phys_min, phys_max, phys_step;
  int y_orig, y_min, y_max;
  int nb_grad;
  char grad_format[10];
  GtkWidget *vBox_Grad;
}
SIHM_ScaleAttributes;

typedef struct _SIHM_CurveAttributes
{
  char title[128], ctrl_lbl[32];
  int show;
  char **legend;
  GdkGC **GC;
  int nb_val;
  double **tval;
  int nb_range, range;
  SIHM_ScaleAttributes *tSA;
  char val_format[10];
  GtkWidget *top, *DA, **lblVal, **label_Legend;

  GtkWidget *PrivateWin, *PrivateVBox;
  int win_view;
}
SIHM_CurveAttributes;

typedef struct _ihm_time {
  double time;
  double time_init;
  double time_last;
  int sec;
  int min;
  double deltaT;
} ihm_time_t;

KIHM_EXTERN SIHM_CurveAttributes ihm_CA[KIHM_NB_CURVES];

/* declarations for references */
/* --------------------------- */
extern int tab_r[4];	                // Pitch, roll, yaw, gaz references
KIHM_EXTERN GtkWidget *ihm_label_RCref;

/*Stephane*/
KIHM_EXTERN GtkWidget *nbDetectedTags_label;
KIHM_EXTERN GtkWidget *activeDetection_label;

extern char label_mykonos_state_value[32];
extern char label_ctrl_state_value[MAX_STR_CTRL_STATE];
extern char label_detection_state_value[32];

extern int32_t ihm_start;

void ihm_set_start_button_state( int32_t start );

void  ihm_initCurves(GtkWidget *widget, int curve);
void ihm_destroyCurves(void);

void  ihm_update_time( void );
void  ihm_init_time( void );

PROTO_THREAD_ROUTINE(ihm , data);

extern vp_os_mutex_t  ihm_lock;
extern int ihm_is_initialized;


#endif  // IHM_H
