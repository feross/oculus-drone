#include <config.h>
#include <ardrone_api.h>
#include <UI/ui.h>
#include <ihm/ihm.h>

C_RESULT custom_reset_user_input(input_state_t* input_state, uint32_t user_input )
{
  return C_OK;
}

C_RESULT custom_update_user_input(input_state_t* input_state, uint32_t user_input )
{
  //int32_t mayday_timeout = 2;
  static  int32_t mayday_type    = ARDRONE_ANIM_PHI_M30_DEG;
  static  int32_t old_ad = 0;
  
  ihm_set_start_button_state( ( user_input & (0x1 << ARDRONE_UI_BIT_START) ) >> ARDRONE_UI_BIT_START );

 
  if (( input_state->ad == 1 ) && (old_ad ==0)) {
    mayday_type++;
    mayday_type=mayday_type&7;
    ardrone_at_set_anim(mayday_type, MAYDAY_TIMEOUT[mayday_type]);
    printf("%d\n",mayday_type);
  }
  old_ad = input_state->ad;

  return C_OK;
}

