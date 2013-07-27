/**
 * @file main.c
 * @author sylvain.gaeremynck@parrot.com
 * @date 2009/07/01
 */
#include <ardrone_testing_tool.h>

//ARDroneLib
#include <ardrone_tool/ardrone_time.h>
#include <ardrone_tool/Navdata/ardrone_navdata_client.h>
#include <ardrone_tool/Control/ardrone_control.h>
#include <ardrone_tool/UI/ardrone_input.h>
#include <ardrone_tool/ardrone_tool_configuration.h>

//Common
#include <config.h>
#include <ardrone_api.h>

//VP_SDK
#include <ATcodec/ATcodec_api.h>
#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_thread.h>
#include <VP_Api/vp_api_thread_helper.h>
#include <VP_Os/vp_os_signal.h>
#include <VP_Os/vp_os_types.h>
#include <VP_Os/vp_os_delay.h>
//#include <VP_Os/vp_os_signal_dep.h>

//Local project
#include <UI/gamepad.h>
#include <Video/video_stage.h>

static int32_t exit_ihm_program = 1;

/* Implementing Custom methods for the main function of an ARDrone application */

/* The delegate object calls this method during initialization of an ARDrone application */
C_RESULT ardrone_tool_init_custom(int argc, char **argv)
{
  /* Registering for a new device of game controller */
  ardrone_tool_input_add( &gamepad );
  ardrone_tool_input_add( &ps3pad );

  /* Start all threads of your application */
  START_THREAD( video_stage, NULL );
  START_THREAD( main_application_thread , NULL );
  return C_OK;
}

/* The delegate object calls this method when the event loop exit */
C_RESULT ardrone_tool_shutdown_custom()
{
  /* Relinquish all threads of your application */
  JOIN_THREAD( video_stage );

  /* Unregistering for the current device */
  ardrone_tool_input_remove( &gamepad );

  return C_OK;
}

/* The event loop calls this method for the exit condition */
bool_t ardrone_tool_exit()
{
  return exit_ihm_program == 0;
}

C_RESULT signal_exit()
{
  exit_ihm_program = 0;

  return C_OK;
}

PROTO_THREAD_ROUTINE(main_application_thread, data);

/* Implementing thread table in which you add routines of your application and those provided by the SDK */
BEGIN_THREAD_TABLE
  THREAD_TABLE_ENTRY( ardrone_control, 20 )
  THREAD_TABLE_ENTRY( navdata_update, 20 )
  THREAD_TABLE_ENTRY( video_stage, 20 )
  THREAD_TABLE_ENTRY( main_application_thread, 20 )
END_THREAD_TABLE


vp_os_mutex_t test_mutex;
vp_os_cond_t  test_condition;

vp_os_mutex_t navdata_mutex;
vp_os_cond_t  navdata_condition;

navdata_unpacked_t last_navdata;

 C_RESULT demo_navdata_client_init( void* data ) { return C_OK; }

 char anim[]={'-','\\','|','/'};

 C_RESULT demo_navdata_client_process( const navdata_unpacked_t* const navdata )
 {
	 static int counter=0;
	 printf("%c\n\033[1A",anim[(counter++)%sizeof(anim)]);
	 vp_os_memcpy(&last_navdata,navdata,sizeof(last_navdata));
	 vp_os_cond_signal(&navdata_condition);
	 return C_OK;
 }

 C_RESULT demo_navdata_client_release( void ) {return C_OK;}


void wait_for_next_navdata()
{
	vp_os_mutex_lock(&navdata_mutex);
 	vp_os_cond_wait(&navdata_condition);
 	vp_os_mutex_unlock(&navdata_mutex);
}


void test_callback(int res)
{
	PRINT("<ArdroneTool callback>\n");
	/* Make the test program continue */
	if (res)
	{
		vp_os_cond_signal(&test_condition);
	}
	else
	{
		printf("  -- Configuration command is taking time to succeed ... ---\n");
	}
}

void wait_for_setting_to_be_acknowlegded()
{
	vp_os_mutex_lock(&test_mutex);
	vp_os_cond_wait(&test_condition);
	vp_os_mutex_unlock(&test_mutex);
}


#define BLUE { printf("%s","\033[34;01m"); }
#define RED { printf("%s","\033[31;01m"); }
#define GREEN { printf("%s","\033[32;01m"); }

#define RAZ { printf("%s","\033[0m"); }

#define title(x) { BLUE; printf("[%i] %s",__LINE__,x); RAZ; }

#define ok(x)      { GREEN; printf("[%i] %s",__LINE__,x); RAZ; }
#define failure(x) { RED; printf("[%i] %s",__LINE__,x); RAZ; }

void mypause()
{
	//char buffer[128];
	char c;
	printf("Press a key to continue...\n"); 
	vp_os_delay(2000);
	return;
	c=fgetc(stdin); //getchar_unlocked();
	perror("Erreur");
	printf(" Key : %i\n",(int)c);
}

//#define USE_STEP_1
//#define USE_STEP_2
#define USE_STEP_3


DEFINE_THREAD_ROUTINE(main_application_thread, data)
{
	int i;
	int expected_mask=0;
	int mask_to_test = 0;
	int32_t i_value;

	/* Mutexes for synchronisation */
		vp_os_mutex_init(&test_mutex);
		vp_os_cond_init(&test_condition,&test_mutex);

		vp_os_mutex_init(&navdata_mutex);
		vp_os_cond_init(&navdata_condition,&navdata_mutex);

	vp_os_delay(1000);

	//fflush(stdin);

	title("[IMPORTANT]  Make sure no client application connected to the drone before.\n");

	printf("Navdata full option mask : <%x>\n",NAVDATA_OPTION_FULL_MASK);
	printf("Navdata nb options : <%i>\n",NAVDATA_NUM_TAGS);


	mypause();

#ifdef USE_STEP_1

	/* Check that the drone is in bootstrap mode */

/*Ardronetool sets the navdata full mode at startup */
#ifdef NOARDRONETOOL
	title("-- Checking drone state at bootstrap -- \n");

		wait_for_next_navdata();

		if (ardrone_get_mask_from_state(last_navdata.ardrone_state,ARDRONE_NAVDATA_BOOTSTRAP))
		{ ok("Bootstrap bit is set.\n"); }
		else
		{ failure("Bootstrap bit is not set.\n"); }

		if (!ardrone_get_mask_from_state(last_navdata.ardrone_state,ARDRONE_NAVDATA_DEMO_MASK))
		{ ok("Demo mask bit is not set\n"); }
		else
		{ failure("Demo mask bit is set.\n"); }
#endif

	title("-- Checking drone state in navdata DEMO mode -- \n");

		i_value = 1;
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(navdata_demo, &i_value, NULL);

		ARDRONE_TOOL_CONFIGURATION_GET(test_callback);
		wait_for_setting_to_be_acknowlegded();

		wait_for_next_navdata();

		if (ardrone_get_mask_from_state(last_navdata.ardrone_state,ARDRONE_NAVDATA_BOOTSTRAP))
		{ failure("Bootstrap bit is set.\n"); }
		else
		{ ok("Bootstrap bit is not set.\n"); }

		if (!ardrone_get_mask_from_state(last_navdata.ardrone_state,ARDRONE_NAVDATA_DEMO_MASK))
		{ failure("Demo mask bit is not set\n"); }
		else
		{ ok("Demo mask bit is set.\n"); }

		if (last_navdata.last_navdata_refresh == ( NAVDATA_OPTION_MASK(NAVDATA_DEMO_TAG) | NAVDATA_OPTION_MASK(NAVDATA_VISION_DETECT_TAG) ) )
		{	ok("Received the right options in the navdata\n");		}
		else
		{   failure("Received the wrong navdata options"); printf(" - mask : %x\n",last_navdata.last_navdata_refresh); }

		if (ardrone_control_config.navdata_options == ( NAVDATA_OPTION_MASK(NAVDATA_DEMO_TAG) | NAVDATA_OPTION_MASK(NAVDATA_VISION_DETECT_TAG) ) )
		{	ok("Navdata option mask is ok in the configuration\n");		}
		else
		{   failure("Wrong navdata option mask in the configuration"); printf(" - mask : %x\n",ardrone_control_config.navdata_options); }

		mypause();



	title("-- Checking drone state in navdata FULL mode -- \n");

		i_value = 0;
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(navdata_demo, &i_value, NULL);

		ARDRONE_TOOL_CONFIGURATION_GET(test_callback);
		wait_for_setting_to_be_acknowlegded();

		wait_for_next_navdata();

		if (ardrone_get_mask_from_state(last_navdata.ardrone_state,ARDRONE_NAVDATA_BOOTSTRAP))
		{ failure("Bootstrap bit is set.\n"); }
		else
		{ ok("Bootstrap bit is not set.\n"); }

		if (!ardrone_get_mask_from_state(last_navdata.ardrone_state,ARDRONE_NAVDATA_DEMO_MASK))
		{ ok("Demo mask bit is not set\n"); }
		else
		{ failure("Demo mask bit is set.\n"); }

		if ( (last_navdata.last_navdata_refresh  & (NAVDATA_OPTION_FULL_MASK)) == NAVDATA_OPTION_FULL_MASK )
		{	ok("Received the right options in the navdata\n");		}
		else
		{   failure("Received the wrong navdata options"); printf("mask : %x\n",last_navdata.last_navdata_refresh); }

		if ( (ardrone_control_config.navdata_options & (NAVDATA_OPTION_FULL_MASK)) == NAVDATA_OPTION_FULL_MASK )
		{   ok("Navdata option mask part is ok\n");		}
		else
		{   failure("Wrong navdata option mask"); printf("mask : %x\n",ardrone_control_config.navdata_options); }

		mypause();

#endif

	/*----------------------------*/
#ifdef USE_STEP_2

		/* Set the demo mode to get 15 navdatas per second */
		i_value = 1;
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(navdata_demo, &ivalue, NULL);

		for (i=NAVDATA_DEMO_TAG ; i<NAVDATA_NUM_TAGS ; i++)
		{
			title("-- Checking navdata tag "); printf("[%d]",i);

					expected_mask = ( NAVDATA_OPTION_MASK(i) | NAVDATA_OPTION_MASK(NAVDATA_DEMO_TAG) |  NAVDATA_OPTION_MASK(NAVDATA_VISION_DETECT_TAG) );
					i_value = NAVDATA_OPTION_MASK(i);
					ARDRONE_TOOL_CONFIGURATION_ADDEVENT(navdata_options, &i_value, NULL);
					ARDRONE_TOOL_CONFIGURATION_GET(test_callback);

					wait_for_setting_to_be_acknowlegded();
					wait_for_next_navdata();

					if (ardrone_get_mask_from_state(last_navdata.ardrone_state,ARDRONE_NAVDATA_BOOTSTRAP))
					{ failure("Bootstrap bit is set.\n"); }
					else
					{ ok("Bootstrap bit is not set.\n"); }

					if (ardrone_get_mask_from_state(last_navdata.ardrone_state,ARDRONE_NAVDATA_DEMO_MASK))
					{ ok("Demo mask bit is set\n"); }
					else
					{ failure("Demo mask bit is not set.\n"); }

					if ( (last_navdata.last_navdata_refresh  & (NAVDATA_OPTION_FULL_MASK)) == expected_mask )
					{	ok("Received the right options in the navdata\n");		}
					else
					{   failure("Received the wrong navdata options"); printf("mask : %x\n",last_navdata.last_navdata_refresh); }

					if ( (ardrone_control_config.navdata_options & (NAVDATA_OPTION_FULL_MASK)) == expected_mask )
					{   ok("Navdata option mask in the configuration is ok\n");		}
					else
					{   failure("Wrong navdata option mask in the configuration"); printf("mask : %x\n",ardrone_control_config.navdata_options); }

					//mypause();

		}

#endif

#ifdef USE_STEP_3

		/* Set the demo mode to get 15 navdatas per second */
		i_value = 1;
		ARDRONE_TOOL_CONFIGURATION_ADDEVENT(navdata_demo, &i_value, NULL);

		while(1)
		{
			i = mask_to_test = rand() & NAVDATA_OPTION_FULL_MASK;

			title("-- Checking navdata tag "); printf("[%x]",i);

								expected_mask = ( mask_to_test | NAVDATA_OPTION_MASK(NAVDATA_DEMO_TAG) |  NAVDATA_OPTION_MASK(NAVDATA_VISION_DETECT_TAG) );
								i_value = (int32_t)mask_to_test;
								ARDRONE_TOOL_CONFIGURATION_ADDEVENT(navdata_options, &i_value, NULL);
								ARDRONE_TOOL_CONFIGURATION_GET(test_callback);

								wait_for_setting_to_be_acknowlegded();
								wait_for_next_navdata();

								if (ardrone_get_mask_from_state(last_navdata.ardrone_state,ARDRONE_NAVDATA_BOOTSTRAP))
								{ failure("Bootstrap bit is set.\n"); }
								else
								{ ok("Bootstrap bit is not set.\n"); }

								if (ardrone_get_mask_from_state(last_navdata.ardrone_state,ARDRONE_NAVDATA_DEMO_MASK))
								{ ok("Demo mask bit is set\n"); }
								else
								{ failure("Demo mask bit is not set.\n"); }

								if ( (last_navdata.last_navdata_refresh  & (NAVDATA_OPTION_FULL_MASK)) == expected_mask )
								{	ok("Received the right options in the navdata\n");		}
								else
								{   failure("Received the wrong navdata options"); printf("mask : %x  expected : %x\n",last_navdata.last_navdata_refresh,expected_mask); }

								if ( (ardrone_control_config.navdata_options & (NAVDATA_OPTION_FULL_MASK)) == expected_mask )
								{   ok("Navdata option mask in the configuration is ok\n");		}
								else
								{   failure("Wrong navdata option mask in the configuration"); printf("mask : %x  epxpected : %x\n",ardrone_control_config.navdata_options,expected_mask); }

		}

#endif


	title("\nEnd of the test.\n");

}
