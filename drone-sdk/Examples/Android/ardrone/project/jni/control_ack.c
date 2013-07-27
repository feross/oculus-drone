//
//  control_ack.m
//  iPhoneDemo
//
//  Created by Mykonos on 20/10/08.
//  Copyright 2008 Parrot SA. All rights reserved.
//
#include "control_ack.h"
#include "app.h"

#include <ardrone_api.h>
#include <ardrone_tool/Control/ardrone_control.h>
#include <ardrone_tool/Control/ardrone_control_ack.h>

#undef ARDRONE_CONFIG_KEY_IMM
#undef ARDRONE_CONFIG_KEY_REF
#undef ARDRONE_CONFIG_KEY_STR
#define ARDRONE_CONFIG_KEY_IMM(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK)
#define ARDRONE_CONFIG_KEY_REF(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK)
#define ARDRONE_CONFIG_KEY_STR(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK)
#include <config_keys.h> // must be included before to have types

static ardrone_control_ack_event_t control_ack_event = { 0 };
static bool_t control_ack_is_init = FALSE;

void control_ack_init(void)
{
	if(!control_ack_is_init)
	{
		control_ack_event.status = ARDRONE_CONTROL_EVENT_IDLE;
		control_ack_is_init = TRUE;
	}
}

static void control_ack_event_configure_start(struct _ardrone_control_event_t* event)
{
	//INFO("%s\n", __FUNCTION__);
}

static void control_ack_event_configure_end(struct _ardrone_control_event_t* event)
{
	switch(event->status)
	{
		case ARDRONE_CONTROL_EVENT_FINISH_SUCCESS:
			
			break;
			
		case ARDRONE_CONTROL_EVENT_FINISH_FAILURE:
			break;
			
		default:
			// Nothing to do
			break;
	}
}

static void control_ack_event_configure(void)
{
	control_ack_event.event			= ACK_CONTROL_MODE;
	control_ack_event.num_retries	= 5;
	control_ack_event.status		= ARDRONE_CONTROL_EVENT_WAITING;

	control_ack_event.ardrone_control_event_start = control_ack_event_configure_start;
	control_ack_event.ardrone_control_event_end	  = control_ack_event_configure_end;
	
	control_ack_event.ack_state = ACK_COMMAND_MASK_TRUE;
	
	ardrone_control_send_event( (ardrone_control_event_t*)&control_ack_event );
}

bool_t control_ack_configure_navdata_demo(bool_t navdata_demo)
{
	bool_t b = FALSE;
	if(control_ack_is_init)
	{
		switch(control_ack_event.status)
		{
			case ARDRONE_CONTROL_EVENT_IDLE:
				INFO("control_ack_event navdata demo configure \n");
				control_ack_event_configure();
				if(navdata_demo)
					ardrone_at_configuration_set_navdata_demo(TRUE);
				else
					ardrone_at_configuration_set_navdata_demo(FALSE);
				break;

			case ARDRONE_CONTROL_EVENT_FINISH_FAILURE:
				control_ack_event.status = ARDRONE_CONTROL_EVENT_IDLE;
				break;
				
			case ARDRONE_CONTROL_EVENT_FINISH_SUCCESS:
				control_ack_event.status = ARDRONE_CONTROL_EVENT_IDLE;
				b = TRUE;
				break;
	
			default:
				// Nothing to do
				break;
		}
	}
	
	return b;
}

