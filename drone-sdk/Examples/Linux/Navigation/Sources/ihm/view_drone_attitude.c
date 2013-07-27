/*
 * @view_drone_attitude.c
 * @author florian.pantaleao.ext@parrot.fr
 * @date 2006/11/08
 *
 * display of BTT-drone Euler angles and curves of IMU outputs
 * original version by Marc-Olivier DZEUKOU
 *
 */

#include <math.h>

#include "control_states.h"
#include "common/mobile_config.h"
#include "common/common.h"

#include "ihm/ihm.h"
#include "ihm/view_drone_attitude.h"
#include "ihm/ihm_vision.h"
#include "ihm/ihm_stages_o_gtk.h"
#include "UI/ui.h"

#define KIHM_RAD_TO_DEG      (180.0/KIHM_PI)
#define KIHM_MILLIDEG_TO_RAD (KIHM_PI/180000.0)

/* Allocation donnees partagees */


extern int tab_g[NB_GAINS];
extern int tab_ag[NB_GAINS_ALT];
extern int tab_fp_g[NB_GAINS_FP];
extern int tab_roundel_g[NB_GAINS_ROUNDEL];

extern GtkWidget *label_RCref;
extern GtkWidget *ihm_ImageEntry[9], *label_elapsedT;
extern GtkWidget *ihm_ImageEntry[9];
extern GtkLabel  *label_mykonos_values[NB_ARDRONE_STATES];
extern GtkWidget *darea_start_button_state;

#ifdef USE_ARDRONE_VICON
extern GtkWidget *darea_vicon_button_state;
#endif

extern ihm_time_t ihm_time;

extern int image_vision_window_status, image_vision_window_view;
extern int tab_vision_config_params[10];
extern int  MiscVar[NB_MISC_VARS];

extern drone_angular_rate_references_t wref;//, wtrim;
extern drone_euler_angles_references_t earef;//, eatrim;
extern int32_t rc_ref_gaz;
extern double tab_trims[3];

/* Stephane */
#ifdef PC_USE_VISION
extern vp_stages_draw_trackers_config_t draw_trackers_cfg;
#endif
#ifdef DEBUG
extern GtkWidget* detectionHistory_label;
#endif
extern int detectionHistory[NB_DETECTION_SAMPLES];   // stores the number of detected tags during one second
static int nb_tags_detection_samples_received = 0;   // counts the number of detection results diplayed

static int32_t mykonos_control_state = -1;
void set_control_state(int32_t control_state)
{
  if( mykonos_control_state == -1 )
    mykonos_control_state = control_state;

  {
    uint32_t major = control_state >> 16;
    uint32_t old_major = mykonos_control_state >> 16;
    input_state_t* input_state = ardrone_tool_get_input_state();

    if( (old_major == CTRL_TRANS_LANDING && major == CTRL_LANDED) || (major == CTRL_DEFAULT && old_major != CTRL_DEFAULT) ){
      ardrone_tool_input_reset();
     }
    if(major==CTRL_DEFAULT && input_state->start ){
      ardrone_tool_start_reset();  // the start i reseted if button start is pushed in CTRL_DEFAULT
     }
  }

  mykonos_control_state = control_state;
}

#ifdef USE_ARDRONE_VICON
static int32_t ardrone_vicon_state = -1;
void set_vicon_state(int32_t vicon_state)
{
	ardrone_vicon_state = vicon_state;
}
#endif

/* function for curve plotting */
/* --------------------------- */
void plot_curve(SIHM_CurveAttributes *pCA)
{
  char str_value[128], str_format[32];
  int k, val;
  int a, b;
  int grad = 0;
  SIHM_ScaleAttributes *pSA = &(pCA->tSA[pCA->range]);

  /* Clean area with a grey rectangle before drawing */
  gdk_draw_rectangle( pCA->DA->window, ihm_GC[KIHM_GC_GREY], TRUE, 0, 0,
                      KIHM_DAREA_CURVE_X_SIZE, KIHM_DAREA_CURVE_Y_SIZE+2*KIHM_DAREA_CURVE_Y_OFFSET );

  /* Draw horizontal dashed lines */
  for(k=0;k<pSA->nb_grad;k++) {
    grad = pSA->y_min - (int) ((k*pSA->phys_step * KIHM_DAREA_CURVE_Y_SIZE) / pSA->phys_range);
    gdk_draw_line(pCA->DA->window, ihm_GC[KIHM_GC_DASHLINE], 0, grad, KIHM_N_PT2PLOT, grad);
  }

  for (val=0;val<pCA->nb_val; val++) {
    // set curves points
    for(k=0;k<KIHM_N_PT2PLOT-1;k++) {
      if( k > (ihm_val_idx-1) ) {
        a = KIHM_N_PT2PLOT-1-(k-ihm_val_idx);
        b = KIHM_N_PT2PLOT-1-(k-(ihm_val_idx-1));
      }
      else {
        a = ihm_val_idx-k-1;
        b = ihm_val_idx-k-2;
      }

      if( k==(ihm_val_idx-1) ) {
        a = 0;
        b = KIHM_N_PT2PLOT-1;
      }

      gdk_draw_line(pCA->DA->window,
                    pCA->GC[val],
                    k,
                    pSA->y_orig - (int) ((pCA->tval[val][a] * KIHM_DAREA_CURVE_Y_SIZE) / pSA->phys_range),
                    k+1,
                    pSA->y_orig - (int) ((pCA->tval[val][b] * KIHM_DAREA_CURVE_Y_SIZE) / pSA->phys_range));
    }

    // set current value
    strcpy(str_format,"%s");
    strcat(str_format,pCA->val_format);
    strcat(str_format,"%s");
    sprintf(str_value, str_format, " ", pCA->tval[val][ihm_val_idx], "        ");
    gtk_label_set_text(GTK_LABEL(pCA->lblVal[val]), str_value);
  }

  /* Show refreshed image*/
  gtk_widget_show_all(pCA->DA);
}




/* function for angle drawing */
/* -------------------------- */
void draw_angle(GtkWidget *widget, PangoLayout  *pPL, double *pangle_deg, int angle_sign, char *ptitle)
{
  int k;
  double angle, cos_angle, sin_angle;
/*   GdkGC *ColorsGC[4] = { pBlueGC, pGreenGC, pRedGC, widget->style->black_gc}; */
  GdkGC *ColorsGC[4] = {ihm_GC[KIHM_GC_RED], widget->style->black_gc, ihm_GC[KIHM_GC_BLUE], ihm_GC[KIHM_GC_GREEN]};
  char legend[4][10] = {"Fus","Ref","Acc","Gyr"};
  char strval[10];
  int X, Y, L, x1, x2, y1, y2, xt, yt;

  xt = 5; yt = 20;
  L = KIHM_DAREA_ANGLE_X_SIZE*3/5;  // Lenght of line
  X = KIHM_DAREA_ANGLE_X_SIZE - L/2 - xt;  // X line center position
  Y = KIHM_DAREA_ANGLE_Y_SIZE/2;           // Y line center position

  /* Clean area with a white rectangle before drawing */
  gdk_draw_rectangle( widget->window, widget->style->white_gc, TRUE,
                      0,
                      0,
                      KIHM_DAREA_ANGLE_X_SIZE,
                      KIHM_DAREA_ANGLE_Y_SIZE );
  gdk_draw_line(widget->window, ihm_GC[KIHM_GC_DASHLINE], 0, Y, KIHM_DAREA_ANGLE_X_SIZE, Y);  // horizontal reference

  // graph title
  pango_layout_set_alignment(pPL, PANGO_ALIGN_CENTER);
  pango_layout_set_width(pPL, KIHM_DAREA_ANGLE_X_SIZE*PANGO_SCALE);
  pango_layout_set_text(pPL, ptitle, -1);
  gdk_draw_layout(widget->window, widget->style->black_gc, 0, 3, pPL);

  /* Draw angle(s), legend(s) and current value(s) */
  pango_layout_set_alignment(pPL, PANGO_ALIGN_LEFT);
  pango_layout_set_width(pPL, -1); // no line wrap
  for(k=0;k<4;k++) {
    // angle
    angle = pangle_deg[k] * 3.14159 / 180.0;
    cos_angle = cos(angle*angle_sign);
    sin_angle = sin(angle*angle_sign);
    x1 = X + (L/2)*cos_angle;
    y1 = Y - (L/2)*sin_angle;
    x2 = X - (L/2)*cos_angle;
    y2 = Y + (L/2)*sin_angle;
    gdk_draw_line(widget->window, ColorsGC[k], (int)x1, (int)y1, (int)x2, (int)y2);

    // legend
    pango_layout_set_alignment(pPL, PANGO_ALIGN_LEFT);
    pango_layout_set_width(pPL, -1); // no line wrap
    pango_layout_set_text(pPL, legend[k], -1);
    gdk_draw_layout(widget->window, ColorsGC[k], xt, yt, pPL);

    // current value
    pango_layout_set_alignment(pPL, PANGO_ALIGN_RIGHT);
    pango_layout_set_width(pPL, 50*PANGO_SCALE);
    if ((angle<1000.0)&&(angle>-1000.0)) {
      sprintf(strval, "%7.2f", pangle_deg[k]);
      pango_layout_set_text(pPL, strval, -1);
    };
    gdk_draw_layout(widget->window, widget->style->black_gc, xt+15, yt, pPL);
    yt += 18;
  }

  /* Show refrehed image*/
  gtk_widget_show_all(widget);
}



void draw_psi( void )
{
  char strval[10];
  double psi;
  GtkWidget *widget = (GtkWidget*) ihm_DA_att[KIHM_DA_ATT_YAW];

  int X, Y, L, l, diameter, xl, yl;
  diameter = KIHM_DAREA_ANGLE_Y_SIZE - 30;
  L = diameter - 2;
  l = 10;
  X = KIHM_DAREA_ANGLE_X_SIZE/2 + 20;
  Y = KIHM_DAREA_ANGLE_Y_SIZE/2;

  psi = ihm_psi * 3.14159 / 180.0;

  /* Clean area by a white rectangle before drawing psi */
  gdk_draw_rectangle( widget->window, widget->style->white_gc, TRUE,
                      0,
                      0,
                      KIHM_DAREA_ANGLE_X_SIZE,
                      KIHM_DAREA_ANGLE_Y_SIZE );

  /* Draw circle for psi */
  gdk_draw_arc( widget->window, widget->style->black_gc, FALSE,
                X - diameter/2,
                Y - diameter/2,
                diameter,
                diameter,
                0,
                360*64 );

  /* Draw psi angle in negative because heading is represented with
     negative trigonometric sense. */
  GdkPoint tPoints[4];
  // North point:
  tPoints[0].x = X - (L/2)*sin(-psi);
  tPoints[0].y = Y - (L/2)*cos(-psi);
  // East point:
  tPoints[1].x = X + (l/2)*cos(-psi);
  tPoints[1].y = Y - (l/2)*sin(-psi);
  // West point:
  tPoints[2].x = X - (l/2)*cos(-psi);
  tPoints[2].y = Y + (l/2)*sin(-psi);
  // South point:
  tPoints[3].x = X + (L/2)*sin(-psi);
  tPoints[3].y = Y + (L/2)*cos(-psi);

  gdk_draw_polygon(widget->window, ihm_GC[KIHM_GC_GREY], TRUE, &tPoints[1], 3);
  gdk_draw_polygon(widget->window, ihm_GC[KIHM_GC_RED ], TRUE, tPoints, 3);

  // graph title
  pango_layout_set_alignment(ihm_PL_DApsi, PANGO_ALIGN_LEFT);
  pango_layout_set_width(ihm_PL_DApsi, -1);
  pango_layout_set_text(ihm_PL_DApsi, "Psi (Yaw)", -1);
  gdk_draw_layout(widget->window, widget->style->black_gc, 20, 3, ihm_PL_DApsi);

  // cardinal points
  xl = 8; yl = xl*2;
  pango_layout_set_alignment(ihm_PL_DApsi, PANGO_ALIGN_LEFT);
  pango_layout_set_width(ihm_PL_DApsi, -1); // no line wrap
  pango_layout_set_text(ihm_PL_DApsi, "N", -1);
  gdk_draw_layout(widget->window, widget->style->black_gc, X-xl/2, Y - diameter/2 - yl, ihm_PL_DApsi);
  pango_layout_set_text(ihm_PL_DApsi, "E", -1);
  gdk_draw_layout(widget->window, widget->style->black_gc, X + diameter/2 + xl/2, Y-yl/2, ihm_PL_DApsi);
  pango_layout_set_text(ihm_PL_DApsi, "S", -1);
  gdk_draw_layout(widget->window, widget->style->black_gc, X-xl/2, Y + diameter/2, ihm_PL_DApsi);
  pango_layout_set_text(ihm_PL_DApsi, "W", -1);
  gdk_draw_layout(widget->window, widget->style->black_gc, X - diameter/2 - 3*xl/2, Y-yl/2, ihm_PL_DApsi);

  // current value
  pango_layout_set_alignment(ihm_PL_DApsi, PANGO_ALIGN_RIGHT);
  pango_layout_set_width(ihm_PL_DApsi, 60*PANGO_SCALE);
  sprintf(strval, "%7.2f", ihm_psi);
  pango_layout_set_text(ihm_PL_DApsi, strval, -1);
  gdk_draw_layout(widget->window, widget->style->black_gc, 5, KIHM_DAREA_ANGLE_Y_SIZE-5-yl, ihm_PL_DApsi);

  /* Show refreshed image*/
  gtk_widget_show_all( widget );
}

void draw_dir_height( void )
{
  char strval[10];
  double dir;
  GtkWidget *widget = (GtkWidget*) ihm_DA_att[KIHM_DA_ATT_DIR];
  int X, Y, L, l, diameter, xl, yl;
  diameter = KIHM_DAREA_ANGLE_Y_SIZE - 30;
  L = diameter - 2;
  l = 10;
  X = KIHM_DAREA_ANGLE_X_SIZE/2 + 20;
  Y = KIHM_DAREA_ANGLE_Y_SIZE/2;

  if( (ihm_dir != NOT_DEF_VAL) && (ihm_dir != KIHM_ZERO_F) )
    dir = ihm_dir;
  else
    dir = KIHM_ZERO_F;


/* Clean area by a white rectangle before drawing dir */
  gdk_draw_rectangle( widget->window, widget->style->white_gc, TRUE,
                      0,
                      0,
                      KIHM_DAREA_ANGLE_X_SIZE,
                      KIHM_DAREA_ANGLE_Y_SIZE );
 /* Draw circle for dir */
  gdk_draw_arc( widget->window, widget->style->black_gc, FALSE,
                X - diameter/2,
                Y - diameter/2,
                diameter,
                diameter,
                0,
                360*64 );

 /* Draw dir angle in negative because heading is represented with
     negative trigonometric sense. */
  GdkPoint tPoints[4];
  // North point:
  tPoints[0].x = X - (L/2)*sin(-dir);
  tPoints[0].y = Y - (L/2)*cos(-dir);
  // East point:
  tPoints[1].x = X + (l/2)*cos(-dir);
  tPoints[1].y = Y - (l/2)*sin(-dir);
  // West point:
  tPoints[2].x = X - (l/2)*cos(-dir);
  tPoints[2].y = Y + (l/2)*sin(-dir);
  // South point:
  tPoints[3].x = X + (L/2)*sin(-dir);
  tPoints[3].y = Y + (L/2)*cos(-dir);

 if( ihm_dir != NOT_DEF_VAL && ihm_dir != HOVER_VAL) {
    gdk_draw_polygon(widget->window, ihm_GC[KIHM_GC_GREY], TRUE, &tPoints[1], 3);
    gdk_draw_polygon(widget->window, ihm_GC[KIHM_GC_BLUE], TRUE, tPoints    , 3);
  }
  else if( ihm_dir == HOVER_VAL) {
    gdk_draw_arc( widget->window, ihm_GC[KIHM_GC_GREEN], TRUE,
		  X - L/2,
		  Y - L/2,
		  L,
		  L,
		  0,
		  360*64 );
  }
  else {
    gdk_draw_arc( widget->window, ihm_GC[KIHM_GC_RED], TRUE,
		  X - L/2,
		  Y - L/2,
		  L,
		  L,
		  0,
		  360*64 );
  }
  // graph title
  pango_layout_set_alignment(ihm_PL_DAdir, PANGO_ALIGN_LEFT);
  pango_layout_set_width(ihm_PL_DAdir, -1);
  pango_layout_set_text(ihm_PL_DAdir, "Dir (Vision)", -1);
  gdk_draw_layout(widget->window, widget->style->black_gc, 20, 3, ihm_PL_DAdir);

 // cardinal points
  xl = 8; yl = xl*2;
  pango_layout_set_alignment(ihm_PL_DAdir, PANGO_ALIGN_LEFT);
  pango_layout_set_width(ihm_PL_DAdir, -1); // no line wrap
  pango_layout_set_text(ihm_PL_DAdir, "X", -1);
  gdk_draw_layout(widget->window, widget->style->black_gc, X-xl/2, Y - diameter/2 - yl, ihm_PL_DAdir);
  pango_layout_set_text(ihm_PL_DAdir, "Y", -1);
  gdk_draw_layout(widget->window, widget->style->black_gc, X + diameter/2 + xl/2, Y-yl/2, ihm_PL_DAdir);
  pango_layout_set_text(ihm_PL_DAdir, "-X", -1);
  gdk_draw_layout(widget->window, widget->style->black_gc, X-xl/2, Y + diameter/2, ihm_PL_DAdir);
  pango_layout_set_text(ihm_PL_DAdir, "-Y", -1);
  gdk_draw_layout(widget->window, widget->style->black_gc, X - diameter/2 - 3*xl/2, Y-yl/2, ihm_PL_DAdir);

  // current value
  pango_layout_set_alignment(ihm_PL_DAdir, PANGO_ALIGN_RIGHT);
  pango_layout_set_width(ihm_PL_DAdir, 60*PANGO_SCALE);
  if( ihm_dir < 360 ){
     sprintf(strval, "%3.2f", KIHM_RAD_TO_DEG*ihm_dir);
  }
  else sprintf(strval, "%d", 0);
  pango_layout_set_text(ihm_PL_DAdir, strval, -1);
  gdk_draw_layout(widget->window, widget->style->black_gc, 5, KIHM_DAREA_ANGLE_Y_SIZE-5-yl, ihm_PL_DAdir);

  /* Show refreshed image*/
  gtk_widget_show_all( widget );
}
void display_RCreferences( void )
{
  char str[256];

  sprintf(str, "Theta %04.2f ThetaTrim %02.2f   Phi %04.2f PhiTrim %02.2f   dPsi %04.2f  dPsiTrim %04.2f   Gaz %d",
          earef.theta/1000.0, tab_trims[0],
          earef.phi/1000.0  , tab_trims[1],
          wref.r/1000.0     , tab_trims[2],
          rc_ref_gaz );
  gtk_label_set_text(GTK_LABEL(ihm_label_RCref), str);
  gtk_label_set_justify(GTK_LABEL(ihm_label_RCref), GTK_JUSTIFY_LEFT);
}

void display_ElapsedTime( void )
{
  char str[256];

  ihm_update_time( );
  sprintf(str, "%02d:%02d", ihm_time.min, ihm_time.sec - 60*ihm_time.min);
  gtk_label_set_text(GTK_LABEL(label_elapsedT), str);
  gtk_label_set_justify(GTK_LABEL(label_elapsedT), GTK_JUSTIFY_LEFT);
}


void get_controler_gain(void)
{
  int k;
  G_CONST_RETURN gchar* CtrlGain[NB_ALL_GAINS];

  for (k = 0; k < NB_GAINS; k++) {
    CtrlGain[k] = gtk_entry_get_text(GTK_ENTRY(entry_PID[k]));
    tab_g[k] = (int) atof(CtrlGain[k]);
  }

  for (k = 0; k < NB_GAINS_ALT; k++) {
    CtrlGain[NB_GAINS + k] = gtk_entry_get_text(GTK_ENTRY(entry_PID[NB_GAINS + k]));
    tab_ag[k] = (int) atof(CtrlGain[NB_GAINS + k]);
  }

  for (k = 0; k < NB_GAINS_FP; k++) {
    CtrlGain[NB_GAINS + NB_GAINS_ALT + k] = gtk_entry_get_text(GTK_ENTRY(entry_PID[NB_GAINS + NB_GAINS_ALT + k]));
    tab_fp_g[k] = (int) atof(CtrlGain[NB_GAINS + NB_GAINS_ALT + k]);
  }

  for (k = 0; k < NB_GAINS_ROUNDEL; k++) {
    CtrlGain[NB_GAINS + NB_GAINS_ALT + NB_GAINS_FP + k] = gtk_entry_get_text(GTK_ENTRY(entry_PID[NB_GAINS + NB_GAINS_ALT + NB_GAINS_FP + k]));
    tab_roundel_g[k] = (int) atof(CtrlGain[NB_GAINS + NB_GAINS_ALT + NB_GAINS_FP + k]);
  }

}

void display_ctrl_states()
{
 gtk_label_set_label(label_mykonos_values[0], label_mykonos_state_value);
 if(label_mykonos_values[1]!=NULL && GTK_IS_LABEL(label_mykonos_values[1]))
	 gtk_label_set_label(label_mykonos_values[1], label_ctrl_state_value);
 gtk_label_set_label(label_mykonos_values[2], label_vision_state_value);
}

void get_misc_var(void)
{
  G_CONST_RETURN gchar* miscvar;
  int k;

  for (k=0;k<NB_MISC_VARS;k++) {
	  	  if (entry_MiscVar[k]!=NULL && GTK_IS_ENTRY(GTK_ENTRY(entry_MiscVar[k]))){
          miscvar = gtk_entry_get_text( GTK_ENTRY(entry_MiscVar[k]) );
          MiscVar[k] = (int)atof( miscvar );
  }}
}

void get_vision_config_params(void)
{

  int k;
  G_CONST_RETURN gchar* VisionCP[9];

   if( image_vision_window_view == WINDOW_VISIBLE) {


  for (k=0;k<9;k++) {
	  if (ihm_ImageEntry[k]!=NULL){
	  if (GTK_IS_ENTRY(ihm_ImageEntry[k])){
		VisionCP[k] = gtk_entry_get_text( GTK_ENTRY(ihm_ImageEntry[k]) );
		tab_vision_config_params[k] = (int)atof( VisionCP[k] );
	}

  }}}
}

void display_start_button_state( void )
{
  GdkGC *start_button_color, *start_button_border_color;

  switch( ihm_start )
  {
  case 0:
    start_button_color = ihm_GC[KIHM_GC_RED];
    break;
  case 1:
    start_button_color = (mykonos_control_state > CTRL_LANDED) ? ihm_GC[KIHM_GC_GREEN] : ihm_GC[KIHM_GC_RED];
    break;
  default:
    start_button_color = ihm_GC[KIHM_GC_RED];
    break;
  }

  start_button_border_color = ihm_GC[KIHM_GC_BLUE];
  gdk_draw_arc( darea_start_button_state->window,
                start_button_color,
                TRUE,
                2, 2,
                START_BUTTON_DA_SIZE - 4,
                START_BUTTON_DA_SIZE - 4,
                0,
                360*64 );

  gdk_draw_arc( darea_start_button_state->window,
                start_button_border_color,
                FALSE,
                2, 2,
                START_BUTTON_DA_SIZE - 4,
                START_BUTTON_DA_SIZE - 4,
                0,
                360*64 );
}

#ifdef USE_ARDRONE_VICON
void display_vicon_button_state( void )
{
  GdkGC *vicon_button_color, *vicon_button_border_color;
  vicon_button_color = (ardrone_vicon_state != -1) ? ihm_GC[KIHM_GC_GREEN] : ihm_GC[KIHM_GC_RED];

  vicon_button_border_color = ihm_GC[KIHM_GC_BLUE];
  gdk_draw_arc( darea_vicon_button_state->window,
                vicon_button_color,
                TRUE,
                2, 2,
                VICON_BUTTON_DA_SIZE - 4,
                VICON_BUTTON_DA_SIZE - 4,
                0,
                360*64 );

  gdk_draw_arc( darea_vicon_button_state->window,
                vicon_button_border_color,
                FALSE,
                2, 2,
                VICON_BUTTON_DA_SIZE - 4,
                VICON_BUTTON_DA_SIZE - 4,
                0,
                360*64 );
}
#endif

gboolean update_display(gpointer pData)
{

  int k;
  double tangle[4];
  char str_format[32];
  // char str_value[128];
#if defined (PC_USE_VISION) && defined (DEBUG)
  /*Stephane*/ int decHist_cnt;
#endif

 // printf("%s %i : locking\n",__FUNCTION__,__LINE__);
  //vp_os_mutex_lock(&ihm_lock);
  //printf("%s %i : locked\n",__FUNCTION__,__LINE__);
  gdk_threads_enter(); //http://blogs.operationaldynamics.com/andrew/software/gnome-desktop/gtk-thread-awareness.html


  if( image_vision_window_view == WINDOW_VISIBLE )
  {
    get_vision_config_params();
  }
  update_vision();
  // theta display
  tangle[0] = ihm_CA[KIHM_CURVE_THETA].tval[0][ihm_val_idx];
  tangle[1] = ihm_CA[KIHM_CURVE_THETA].tval[1][ihm_val_idx];
  tangle[2] = ihm_CA[KIHM_CURVE_THETA].tval[2][ihm_val_idx];
  tangle[3] = ihm_CA[KIHM_CURVE_THETA].tval[3][ihm_val_idx];
  draw_angle(ihm_DA_att[KIHM_DA_ATT_PITCH], ihm_PL_DAtheta, tangle,  1, "Theta (Pitch)");

  // phi display
  tangle[0] = ihm_CA[KIHM_CURVE_PHI].tval[0][ihm_val_idx];
  tangle[1] = ihm_CA[KIHM_CURVE_PHI].tval[1][ihm_val_idx];
  tangle[2] = ihm_CA[KIHM_CURVE_PHI].tval[2][ihm_val_idx];
  tangle[3] = ihm_CA[KIHM_CURVE_PHI].tval[3][ihm_val_idx];
  draw_angle(ihm_DA_att[KIHM_DA_ATT_ROLL ], ihm_PL_DAphi  , tangle, -1, "Phi (Roll)");


  // psi display
  draw_psi();

  // dir display
  draw_dir_height();

  // Get miscllaneous variables
  get_misc_var();

  // Radiocommand references display
  display_RCreferences();

  // Elapsed time display
  display_ElapsedTime();

  // Control States display
  display_ctrl_states();

  // Start button state display
  display_start_button_state();

#ifdef USE_ARDRONE_VICON
  // Vicon button state display
  display_vicon_button_state();
#endif

#ifdef PC_USE_VISION
 /* Stephane : displays number of detected tags in the main window */
 /* The array 'draw_trackers_cfg contains info about tag detection */

  #ifdef DEBUG
    char nbDetectedTags_label_buffer[1024],nbDetectedTags_label_buffer2[1024];
#endif
    /*snprintf(nbDetectedTags_label_buffer,sizeof(nbDetectedTags_label_buffer),"Found : %i",draw_trackers_cfg.detected);
    if(nbDetectedTags_label!=NULL)
    gtk_label_set_text((GtkLabel*)nbDetectedTags_label,(const gchar*)nbDetectedTags_label_buffer);
    */


    detectionHistory[nb_tags_detection_samples_received%NB_DETECTION_SAMPLES] = draw_trackers_cfg.detected;
    nb_tags_detection_samples_received++;
#ifdef DEBUG
    if((nb_tags_detection_samples_received%NB_DETECTION_SAMPLES)==0)
    {
        snprintf(nbDetectedTags_label_buffer,sizeof(nbDetectedTags_label_buffer),"Previously detected targets : ");
        for (decHist_cnt=0;decHist_cnt<NB_DETECTION_SAMPLES;decHist_cnt++)
        {
            snprintf(nbDetectedTags_label_buffer2,
                    sizeof(nbDetectedTags_label_buffer2),
                    "%i ",
                    detectionHistory[decHist_cnt]);
            strncat(nbDetectedTags_label_buffer,
                    nbDetectedTags_label_buffer2,
                    sizeof(nbDetectedTags_label_buffer)-1);
        }
        // Displays the history on the bottom of the main window (see detectionHistory_label in ihm_init() )
        gtk_label_set_text((GtkLabel*)detectionHistory_label,(const gchar*)nbDetectedTags_label_buffer);
    }
#endif
#endif

  if (ihm_freeze_curves == FALSE) {
    // curve display
    for (k=0; k<KIHM_NB_CURVES; k++) {
      if( (GTK_IS_WIDGET(ihm_CA[k].PrivateWin)) && (ihm_CA[k].win_view == WINDOW_VISIBLE) ) {
        plot_curve(&ihm_CA[k]);
        if( k == KIHM_CURVE_VBAT) {
          // set current value
          strcpy(str_format,"%s");
          strcat(str_format,ihm_CA[k].val_format);
          strcat(str_format,"%s");
          // sprintf(str_value, str_format, " ", ihm_CA[k].tval[2][ihm_val_idx], "        ");
          // gtk_label_set_text(GTK_LABEL(ihm_CA[k].lblVal[2]), str_value);
        }
      }
    }
  }
  get_controler_gain();

  gtk_label_set_text((GtkLabel*)activeDetection_label,(const gchar*)label_detection_state_value);

 // printf("%s %i : unlocking\n",__FUNCTION__,__LINE__);
 // vp_os_mutex_unlock(&ihm_lock);

  gdk_threads_leave(); //http://blogs.operationaldynamics.com/andrew/software/gnome-desktop/gtk-thread-awareness.html

	return TRUE;
}
