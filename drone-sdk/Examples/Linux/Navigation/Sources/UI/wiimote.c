/**
 *  \brief    Wiimote handling implementation
 *  \author   Aurelien Morelle <aurelien.morelle@parrot.com>
 *  \version  1.0
 *  \date     04/06/2007
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include <generated_custom.h>

#if defined(WIIMOTE_SUPPORT)
	//#warning Wiimote support enabled
	#include <bluetooth/bluetooth.h>
	#include <cwiid.h>
#else
	//#warning Wiimote support disabled
#endif

#include "wiimote.h"

input_device_t wiimote_device = {
  "Wiimote",
  open_wiimote,
  update_wiimote,
  close_wiimote
};


#if !defined(WIIMOTE_SUPPORT)
//#warning Wiimote compilation disabled
C_RESULT open_wiimote(void)
{
  return C_FAIL;
}

C_RESULT update_wiimote(void)
{
  return C_FAIL;
}

C_RESULT close_wiimote(void)
{
  return C_OK;
}

#else // ! WIIMOTE_SUPPORT
#warning Wiimote compilation enabled

#include <VP_Os/vp_os_print.h>

static bdaddr_t bdaddr = {0};
static cwiid_wiimote_t *wiimote = NULL;

static struct acc_cal wm_cal;

C_RESULT open_wiimote(void)
{
  C_RESULT res = C_OK;

  printf("\nSearching for Wiimote - press both '1' and '2' on your Wiimote !!\n");

  if(cwiid_find_wiimote(&bdaddr, WIIMOTE_FIND_TIMEOUT) == 0)
  {
    if( ! (wiimote = cwiid_open(&bdaddr, 0)) )
      PRINT("Unable to connect to wiimote\n");
    else
      res = C_OK;
  }

  if(SUCCEED(res))
    if(cwiid_set_rpt_mode(wiimote, CWIID_RPT_BTN|CWIID_RPT_ACC))
      res = C_FAIL;

  if(SUCCEED(res))
    if (cwiid_get_acc_cal(wiimote, CWIID_EXT_NONE, &wm_cal))
      res = C_FAIL;

  cwiid_set_led(wiimote,1);

  printf(" ... Wiimote found.\n");

  return res;
}

void print_state(struct cwiid_state *state)
{
  printf("Battery: %d%%\n", (int)(100.0 * state->battery / CWIID_BATTERY_MAX));
  printf("Buttons: %X\n", state->buttons);
  printf("Acc: x=%d y=%d z=%d\n", state->acc[CWIID_X], state->acc[CWIID_Y], state->acc[CWIID_Z]);
}

//extern void api_set_iphone_acceleros(bool_t enable, float32_t fax, float32_t fay, float32_t faz);

#define NEW_AMOUNT 0.1
#define OLD_AMOUNT (1.0-NEW_AMOUNT)

typedef struct{
	float x,y,z;
}vec3;

typedef struct{
	float r,p,t;
}vec3sph;


static void rumble(int flag)
		{
		static int localflag=0;
		if (flag!=localflag){
			localflag=flag;
			cwiid_set_rumble(wiimote,flag);
		}
}

static void leds(int flags)
{
		static int localflags=0;
		if (flags!=localflags){
			localflags=flags;
			cwiid_set_led(wiimote,flags);
		}
}

C_RESULT update_wiimote(void)
{
  C_RESULT res = C_OK;
  static struct cwiid_state state,previous_state;

  static int control_mode = 1;
  static int valid_domain = 0;

  int8_t x, y;
  static int start=0;
  static int select=0;

  static vec3 a={0,0,0},s={0,0,0};
  static vec3sph r={0,0,0},pr={0,0,0},rref={0,0,0};

  static float pitch=0.0f,roll=0.0f,gaz=0.0f,yaw=0.0f;

  float tmp;
  static int fly=0;



  if (cwiid_get_state(wiimote, &state))
    {
      fprintf(stderr, "Error getting state\n");
      res = C_FAIL;
    }


#define SWITCHING(X) ((state.buttons&X) && !(previous_state.buttons&X))
#define RELEASING(X) ((!state.buttons&X) && (previous_state.buttons&X))
#define PRESSED(X) ((state.buttons&X))
  static int flag_rumble=0;
#define RUMBLE_ON { if (!flag_rumble) {flag_rumble=1;  } }
  /* Sets how to use the wiimote */
#define MAXMODE 4
  if (SWITCHING(CWIID_BTN_MINUS)) {
	  control_mode--;
	  if (control_mode<1) control_mode=MAXMODE;
	  leds(1<<(control_mode-1));
  }
  if (SWITCHING(CWIID_BTN_PLUS))  {
	  control_mode++;
	  if (control_mode>MAXMODE) control_mode=1;
	  leds(1<<(control_mode-1));
  }

 /* Gets gravitation G projection on x,y,z axis */
	  a.x = - (float32_t) ((((double)state.acc[CWIID_X] - wm_cal.zero[CWIID_X]) /
				  (wm_cal.one[CWIID_X] - wm_cal.zero[CWIID_X])));
	  a.y = - (float32_t) ((((double)state.acc[CWIID_Y] - wm_cal.zero[CWIID_Y]) /
				  (wm_cal.one[CWIID_Y] - wm_cal.zero[CWIID_Y])));
	  a.z = + (float32_t) ((((double)state.acc[CWIID_Z] - wm_cal.zero[CWIID_Z]) /
			  (wm_cal.one[CWIID_Z] - wm_cal.zero[CWIID_Z])));

	  s.x = (a.x<0.0f)?(1.0f):(-1.0f);
	  s.y = (a.y<0.0f)?(1.0f):(-1.0f);
	  s.z = (a.z<0.0f)?(1.0f):(-1.0f);

	  float ax2 = a.x*a.x;
	  float ay2 = a.y*a.y;
	  float az2 = a.z*a.z;

	  r.r = sqrtf((ax2+ay2)+az2);

	  switch(control_mode)
	  {
	  case 1:
	  case 2:

	  	  if (r.r==0.0f) { r.p=r.t=0.0f; } else {
	  		  // Angle gauche/droite
	  		  r.p = asin(a.y);  if (isnan(r.p)) r.p=0.0f;

			  // Sur plan vertical radial
				  r.t = acos(a.z/(sqrtf(az2+ax2)));  if (isnan(r.t)) r.t=0.0f;
				  r.t*=s.x;
	  	  }
	  	  break;

	  case 3:
	  case 4:

	  	  	  if (r.r==0.0f) { r.p=r.t=0.0f; } else {
	  	  		  // Angle entre le projete de G sur le plan vertical longitudinal (yz) et l'axe z
	  				  r.p = acos(a.z/(sqrtf(az2+ay2)));  if (isnan(r.p)) r.p=0.0f;
	  				  /* If wiimote faces the ground */
	  				    //if (a.z<0.0f) r.p= M_PI-r.p;
	  				  r.p*=s.y;
	  			  // Idem sur le plan vertical radial
	  				  r.t = acos(a.z/(sqrtf(az2+ax2)));  if (isnan(r.t)) r.t=0.0f;
	  				  r.t*=s.x;
	  	  	  }
	  	  	  break;
	    }

	  r.r = (r.r+pr.r)/2.0f;
	  r.t = (r.t+pr.t)/2.0f;
	  r.p = (r.p+pr.p)/2.0f;



  switch(control_mode)
  {

		  case 1:
		  case 2:
			  /* Wiimote is handled horizontally.
			   * '2' button under left thumb
			   * directionnal cross under right thumb
			   */


			  /* 0 -> buttons facing sky */

			  if ((SWITCHING(CWIID_BTN_1)||SWITCHING(CWIID_BTN_2)||SWITCHING(CWIID_BTN_B))){  rref=r;  }

			  if (PRESSED(CWIID_BTN_1)||PRESSED(CWIID_BTN_2)||PRESSED(CWIID_BTN_B))
			  {
				  /* If wiimote facing ground */
				  if (a.z<0 && a.x>0)
				  {
					  rumble(1);
				  }
				  else
				  {
					  rumble(0);
					  leds(1<<(control_mode-1));
					  pitch = (r.t-rref.t)*1.0f; if (pitch<-1.0f) pitch=-1.0f; if (pitch>1.0f) pitch=1.0f;
					  roll  = -(r.p-rref.p)*0.75f; if (roll<-1.0f)  roll=-1.0f; if (roll>1.0f) roll=1.0f;
					  fly=1;
				  }
			  }
			  else
			  {
				  pitch=roll=0;
				  rumble(0);
				  leds(1<<(control_mode-1));
				  fly=0;
			  }


			  gaz  = (PRESSED(CWIID_BTN_LEFT))? 1.0f: (PRESSED(CWIID_BTN_RIGHT)) ? -1.0f : 0.0f;
			  yaw  = (PRESSED(CWIID_BTN_DOWN))? -1.0f: (PRESSED(CWIID_BTN_UP))   ? 1.0f : (control_mode==2) ? (-pitch*roll) : 0.0f;

			  break;


		  case 3:
		  case 4:

			  if (PRESSED(CWIID_BTN_B))
 			  {
				  if (a.z<-0.5f)
				  {
					  rumble(1);
				  }
				  else
				  {
					  rumble(0);
					  leds(1<<(control_mode-1));
					  pitch = -(r.p-rref.p)*1.5f; if (pitch<-1.0f) pitch=-1.0f; if (pitch>1.0f) pitch=1.0f;
					  roll  = (r.t-rref.t)*1.5f; if (roll<-1.0f)  roll=-1.0f; if (roll>1.0f) roll=1.0f;
					  fly=1;
				  }
			  }
			  else
				  {
				  	 rumble(0);
				     leds(1<<(control_mode-1));
				  	 fly=0;
				  	 pitch=roll=0;

				  }

			  if (SWITCHING(CWIID_BTN_B))  { rref = r; }

			  gaz  = (PRESSED(CWIID_BTN_DOWN))? -1.0f: (PRESSED(CWIID_BTN_UP)) ? +1.0f : 0.0f;
			  yaw  = (PRESSED(CWIID_BTN_LEFT))? -1.0f: (PRESSED(CWIID_BTN_RIGHT))   ? 1.0f : (control_mode==4) ? (-pitch*roll) : 0.0f;

  }
  /* Buttons common to all modes */
  	  if (SWITCHING(CWIID_BTN_A)) { start^=1;  ardrone_tool_set_ui_pad_start(start); }
  	  if (SWITCHING(CWIID_BTN_HOME)) {
  				  				  select^=1;
  				  				  ardrone_tool_set_ui_pad_select(select);
  				  				  ardrone_tool_set_ui_pad_start(0);
  				  			  }


	  //
	//printf("Wiimote mode 2 [ax %f][ay %f][az %f]\n\033[1A", a.x,a.y,a.z);
  	printf("Wiimode %i  [rr %3.2f][rp %3.2f][rt %3.2f]\n", control_mode,r.r,r.p,r.t);
  	printf("[Fly %i][Ptc %3.2f][Rl %3.2f][Yw %3.2f][Gz %3.2f][Strt. %i][Sel. %i]\n\033[2A", fly,pitch,roll,yaw,gaz,start,select);
	  ardrone_at_set_progress_cmd( fly,/*roll*/roll,/*pitch*/pitch,/*gaz*/gaz,/*yaw*/yaw);

  /*api_set_iphone_acceleros(
			   (state.buttons&CWIID_BTN_2 ? 1 : 0)|(state.buttons&CWIID_BTN_A ? 2 : 0),
			   a_x, a_y, a_z);*/

  previous_state = state;
  pr=r;

  return C_OK;
}

C_RESULT close_wiimote(void)
{
  cwiid_close(wiimote);

  return C_OK;
}

#endif // ! WIIMOTE_SUPPORT

