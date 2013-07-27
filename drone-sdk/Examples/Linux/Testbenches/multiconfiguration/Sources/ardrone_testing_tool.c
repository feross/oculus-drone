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
  //ardrone_tool_input_add( &gamepad );
  //ardrone_tool_input_add( &ps3pad );

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
  //ardrone_tool_input_remove( &gamepad );

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

#define BLUE { printf("%s","\033[34;01m"); }
#define RED { printf("%s","\033[31;01m"); }
#define GREEN { printf("%s","\033[32;01m"); }

#define RAZ { printf("%s","\033[0m"); }

#define test_string(string,expected)\
		if (strcmp(ardrone_control_config.string,expected)!=0) {  RED; printf("[%i]  Parameter <%s> failed : drone <%s>  expected <%s>\n",__LINE__,#string,ardrone_control_config.string,expected); RAZ; } else { GREEN; printf("[%i] OK for <%s> with value <%s>\n",__LINE__,#string,expected); RAZ; }

#define test_int(myint,expected)\
		if (ardrone_control_config.myint!=expected) { RED; printf("[%i]  Parameter <%s> failed : drone <%i>,  expected <%i>\n",__LINE__,#myint,ardrone_control_config.myint,expected); RAZ; } else { GREEN; printf("[%i] OK for <%s> with value <%i>\n",__LINE__,#myint,expected); RAZ; }

#define test_nint(myint,unexpected)\
		if (ardrone_control_config.myint==unexpected) {  RED; printf("[%i]  Parameter <%s> failed : drone <%i> was unexpected.\n",__LINE__,#myint,ardrone_control_config.myint); RAZ; } else { GREEN; printf("[%i] OK for <%s> with value <%i>\n",__LINE__,#myint,ardrone_control_config.myint); RAZ; }

#define test_float(myfloat,expected)\
		if (ardrone_control_config.myfloat!=expected) {  RED; printf("[%i]  Parameter <%s> failed : drone <%f>, expected <%f>\n",__LINE__,#myfloat,(float)ardrone_control_config.myfloat,(float)expected); RAZ; } else { GREEN; printf("[%i] OK for <%s> with value <%f>\n",__LINE__,#myfloat,(float)expected); RAZ; }

#define test_nfloat(myfloat,unexpected)\
		if (ardrone_control_config.myfloat==unexpected) { RED; printf("[%i]  Parameter <%s> failed : drone <%f> was unexpected.\n",__LINE__,#myfloat,(float)ardrone_control_config.myfloat); RAZ; } else { GREEN; printf("[%i] OK for <%s> with value <%f>\n",__LINE__,#myfloat,(float)ardrone_control_config.myfloat); RAZ; }

#define title(x) { BLUE; printf("[%i] %s",__LINE__,x); RAZ; }

#define set_string(x,y) { printf(" -> Setting %s = %s\n",#x,y); ARDRONE_TOOL_CONFIGURATION_ADDEVENT(x,y,NULL); }
#define set_int(x,y)    { printf(" -> Setting %s = %i\n",#x,y); int __loc_value = y; ARDRONE_TOOL_CONFIGURATION_ADDEVENT(x,&__loc_value,NULL); }
#define set_float(x,y)  { printf(" -> Setting %s = %f\n",#x,y); float __loc_value = y; ARDRONE_TOOL_CONFIGURATION_ADDEVENT(x,&__loc_value,NULL); }

/* Use values that can be exactly be stored in memory without being rounded */
#define F1 (0.75f)
#define F2 (0.875f)

#define APPDESC1 "Application 1 - test program"
#define PROFDESC2 "Profile 2 - test program"
#define SESSDESC3 "Session 3 - test program"
#define SESSDESC4 "Session 4 - test program"

void mypause()
{
	printf("Press a key to continue...\n"); 
	vp_os_delay(1000);
	return;
}

DEFINE_THREAD_ROUTINE(main_application_thread, data)
{

	/* Mutexes for synchronisation */
	vp_os_mutex_init(&test_mutex);
	vp_os_cond_init(&test_condition,&test_mutex);

	vp_os_delay(1000);

	//fflush(stdin);

	printf("[IMPORTANT] Please check you deleted all configuration files on the drone before starting.\n");


	printf("\n Resetting the drone to the default configuration \n");

	/* Switch to the default configuration file */

		/* Always start by setting the session, then the application, then the user */
		set_string(session_id,"00000000");
		set_string(application_id,"00000000");
		set_string(profile_id,"00000000");

		/* Suppress all other settings. A session and an application were automatically created
		 * at application startup by ARDrone Tool ; we must delete them.
		 * The '-' symbol means 'delete'.
		 * '-all' deletes all the existing configurations.
		 */
		set_string(session_id,"-all");
		set_string(application_id,"-all");
		set_string(profile_id,"-all");

		set_int(navdata_demo,1);

	mypause();

	/* Ask the drone configuration and compare it to the expected values */

	/* Send time to the drone */
	//gettimeofday(&tv,NULL);
	//set_int(time,(int32_t)tv.tv_sec);

		title("\nGetting the drone configuration ...\n");

		ARDRONE_TOOL_CONFIGURATION_GET(test_callback);		vp_os_cond_wait(&test_condition);

		title("Comparing received values to the config_keys.h defaults ...\n");

			#define ARDRONE_CONFIG_KEY_IMM_a9(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK,SCOPE) \
				test_float(NAME,DEFAULT);
			#undef ARDRONE_CONFIG_KEY_REF_a9
			#define ARDRONE_CONFIG_KEY_STR_a9(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK,SCOPE) \
				test_string(NAME,DEFAULT);

			#include <config_keys.h>


			/* Modify one value of each category and see what happens */

				/* To test the 'COMMON' category */
				set_string(ardrone_name,"TEST_CONFIG");

				/* To test the 'APPLIS' category */
				test_nint(bitrate_ctrl_mode,1);     /* 0 is the default value */
				set_int(bitrate_ctrl_mode,1);

				/* To test the 'PROFILE' (aka. 'USER') category */
				test_nfloat(outdoor_euler_angle_max,F1);
				set_float(outdoor_euler_angle_max,F1);

				/* To test the 'SESSION' category */
				test_string(leds_anim,"0,0,0");
				set_string(leds_anim,"1,1,1");

				test_string(application_id,"00000000");
				test_string(profile_id,"00000000");
				test_string(session_id,"00000000");


			title("\nGetting the drone configuration ...\n");


				{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

				test_string(ardrone_name,"TEST_CONFIG"); /* CAT_COMMON param */
				test_int(bitrate_ctrl_mode,1);           /* CAT_APPLI param */
				test_float(outdoor_euler_angle_max,F1);	 /* CAT_USER param */
				test_string(leds_anim,"1,1,1");			 /* CAT_SESSION param */

				test_string(application_id,"00000000");
				test_string(profile_id,"00000000");
				test_string(session_id,"00000000");

				test_string(application_desc , DEFAULT_APPLICATION_DESC);
				test_string(profile_desc, DEFAULT_PROFILE_DESC);
				test_string(session_desc, DEFAULT_SESSION_DESC);



	/*---------------------------------------------------------------------------------------------------------*/

   /* Create an application configuration file */

		title("\nCreating the application ID 11111111 ...\n");

			set_string(application_id,"11111111");
			set_string(application_desc,APPDESC1);

			title("\nGetting the drone configuration ...\n");
			{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

			test_string(ardrone_name,"TEST_CONFIG"); /* CAT_COMMON param; its value should not be changed by creating an application config. */
			test_int(bitrate_ctrl_mode,0);           /* 0 is the default value ; default values are affected to newly created configurations */
			test_float(outdoor_euler_angle_max,F1);	 /* CAT_USER param ; its value should not be changed by creating an application config. */
			test_int(detect_type,CAD_TYPE_NONE);     /* CAT_SESSION param; its value should not be changed by creating an application config. */

			test_string(application_id,"11111111");
			test_string(profile_id,"00000000");
			test_string(session_id,"00000000");

			test_string(application_desc,APPDESC1);
			test_string(profile_desc, DEFAULT_PROFILE_DESC);
			test_string(session_desc, DEFAULT_SESSION_DESC);


			set_int(bitrate_ctrl_mode,1);
			{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}
			test_string(ardrone_name,"TEST_CONFIG");
			test_int(bitrate_ctrl_mode,1);

			set_int(bitrate_ctrl_mode,0);
			{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}
			test_string(ardrone_name,"TEST_CONFIG");
			test_int(bitrate_ctrl_mode,0);
			
			mypause();


		title("\nGoing back to the default application configuration ...\n");

			set_string(application_id,"00000000");

			{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

			test_string(ardrone_name,"TEST_CONFIG");
			test_int(bitrate_ctrl_mode,1);    		 /* 1 was the value of the config.ini configuration  */
			test_float(outdoor_euler_angle_max,F1);
			test_int(detect_type,CAD_TYPE_NONE);

			test_string(application_id,"00000000");
			test_string(profile_id,"00000000");
			test_string(session_id,"00000000");

			test_string(application_desc,DEFAULT_APPLICATION_DESC);
			test_string(profile_desc, DEFAULT_PROFILE_DESC);
			test_string(session_desc, DEFAULT_SESSION_DESC);

			mypause();

		title(" Going back to the newly created application configuration ...\n");

			set_string(application_id,"11111111");

			{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

			test_string(ardrone_name,"TEST_CONFIG");
			test_int(bitrate_ctrl_mode,0);
			test_float(outdoor_euler_angle_max,F1);
			test_int(detect_type,CAD_TYPE_NONE);

			test_string(application_id,"11111111");
			test_string(profile_id,"00000000");
			test_string(session_id,"00000000");

			test_string(application_desc , APPDESC1);
			test_string(profile_desc, DEFAULT_PROFILE_DESC);
			test_string(session_desc, DEFAULT_SESSION_DESC);

			set_string(application_id,"00000000");

			mypause();

/*-----------------------------------------------------------------------------------------------*/

   /* Create a user profile configuration file */

		title("\nCreating the user profile ID 22222222 ...\n");

			set_string(profile_id,"22222222");
			set_string(profile_desc,PROFDESC2);


			title("\nGetting the drone configuration ...\n");


			{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}



			test_string(ardrone_name,"TEST_CONFIG");
			test_int(bitrate_ctrl_mode,1);			 /* 1 was the value of the config.ini configuration  */
			test_float(outdoor_euler_angle_max,default_outdoor_euler_angle_ref_max);
			test_int(detect_type,CAD_TYPE_NONE);

			test_string(application_id,"00000000");
			test_string(profile_id,"22222222");
			test_string(session_id,"00000000");

			test_string(application_desc , DEFAULT_APPLICATION_DESC);
			test_string(profile_desc, PROFDESC2);//DEFAULT_PROFILE_DESC
			test_string(session_desc, DEFAULT_SESSION_DESC);

			set_float(outdoor_euler_angle_max,F2);

			{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

			mypause();


		title("\nGoing back to the default user profile configuration ...\n");

			set_string(profile_id,"00000000");

			{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}


			/* The default value for this is 1 after the first test */
			test_string(ardrone_name,"TEST_CONFIG");
			test_int(bitrate_ctrl_mode,1);			 /* 1 was the value of the config.ini configuration  */
			test_float(outdoor_euler_angle_max,F1);
			test_int(detect_type,CAD_TYPE_NONE);

			test_string(application_id,"00000000");
			test_string(profile_id,"00000000");
			test_string(session_id,"00000000");

			test_string(application_desc , DEFAULT_APPLICATION_DESC);
			test_string(profile_desc, DEFAULT_PROFILE_DESC);
			test_string(session_desc, DEFAULT_SESSION_DESC);

			mypause();


		title(" Going back to the newly created user profile configuration ...\n");

			set_string(profile_id,"22222222");

			{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

			test_string(ardrone_name,"TEST_CONFIG");
			test_int(bitrate_ctrl_mode,1);			 /* 1 was the value of the config.ini configuration  */
			test_float(outdoor_euler_angle_max,F2);
			test_int(detect_type,CAD_TYPE_NONE);

			test_string(ardrone_name,"TEST_CONFIG");

			test_string(application_id,"00000000");
			test_string(profile_id,"22222222");
			test_string(session_id,"00000000");

			test_string(application_desc , DEFAULT_APPLICATION_DESC);
			test_string(profile_desc, PROFDESC2);//DEFAULT_PROFILE_DESC
			test_string(session_desc, DEFAULT_SESSION_DESC);



			set_string(profile_id,"00000000");

			mypause();


/*-----------------------------------------------------------------------------------------------*/

	   /* Create a session configuration file */

		title("\nCreating the session ID 33333333 ...\n");

			set_string(session_id,"33333333");

			title("\nGetting the drone configuration ...\n");
			{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

			test_string(ardrone_name,"TEST_CONFIG"); /* CAT_COMMON param; its value should not be changed by creating a session */
			test_int(bitrate_ctrl_mode,1);           /* CAT_APPLI param; its value should not be changed by creating a session */
			test_float(outdoor_euler_angle_max,F1);  /* value in default config.ini */
			test_int(detect_type,CAD_TYPE_NONE);     /* CAT_SESSION param; its value should be the default one */

			test_string(application_id,"00000000");
			test_string(profile_id,"00000000");
			test_string(session_id,"33333333");

			test_string(application_desc , DEFAULT_APPLICATION_DESC);
			test_string(profile_desc, DEFAULT_PROFILE_DESC);
			test_string(session_desc, DEFAULT_SESSION_DESC ); //DEFAULT_SESSION_DESC);

			/* Test changing a value in the session */
			set_int(detect_type,CAD_TYPE_VISION);
			{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}
			test_int(detect_type,CAD_TYPE_VISION);

			mypause();


		title("\nGoing back to the default session configuration ...\n");

			set_string(session_id,"00000000");

			{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

			test_string(ardrone_name,"TEST_CONFIG");
			test_int(bitrate_ctrl_mode,1);           /* value in default config.ini */
			test_float(outdoor_euler_angle_max,F1);  /* value in default config.ini */
			test_int(detect_type,CAD_TYPE_NONE);

			test_string(application_id,"00000000");
			test_string(profile_id,"00000000");
			test_string(session_id,"00000000");

			test_string(application_desc , DEFAULT_APPLICATION_DESC);
			test_string(profile_desc, DEFAULT_PROFILE_DESC);
			test_string(session_desc, DEFAULT_SESSION_DESC);

			mypause();


		title(" Going back to the newly created session configuration ...\n");

			set_string(session_id,"33333333");
			set_string(session_desc,SESSDESC3);

			{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

			test_string(ardrone_name,"TEST_CONFIG");
			test_int(bitrate_ctrl_mode,1);           /* value in default config.ini */
			test_float(outdoor_euler_angle_max,F1);  /* value in default config.ini */
			test_int(detect_type,CAD_TYPE_VISION);

			test_string(ardrone_name,"TEST_CONFIG");
			test_string(application_id,"00000000");
			test_string(profile_id,"00000000");
			test_string(session_id,"33333333");

			test_string(application_desc , DEFAULT_APPLICATION_DESC);
			test_string(profile_desc, DEFAULT_PROFILE_DESC);
			test_string(session_desc, SESSDESC3 ); //DEFAULT_SESSION_DESC);

			set_int(detect_type,CAD_TYPE_COCARDE);

			set_string(session_id,"00000000");

			mypause();


/*-----------------------------------------------------------------------------------------------*/

		/* Create a session configuration file */
			title("\nCreating the session ID 44444444 ...\n");

					set_string(session_id,"44444444");

					title("\nGetting the drone configuration ...\n");

					{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

					test_int(detect_type,CAD_TYPE_NONE);

					test_string(ardrone_name,"TEST_CONFIG");
					test_int(bitrate_ctrl_mode,1);

					test_string(application_id,"00000000");
					test_string(profile_id,"00000000");
					test_string(session_id,"44444444");

					test_string(application_desc , DEFAULT_APPLICATION_DESC);
					test_string(profile_desc, DEFAULT_PROFILE_DESC);
					test_string(session_desc, DEFAULT_SESSION_DESC ); //FAULT_SESSION_DESC);

					set_int(detect_type,CAD_TYPE_VISION);

					{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

					test_int(detect_type,CAD_TYPE_VISION);

					set_string(application_id,"11111111");
					set_string(profile_id,"22222222");

					{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

					test_string(ardrone_name,"TEST_CONFIG"); 				set_string(ardrone_name,"TEST_CONFIG2");
					test_string(application_id,"11111111");
					test_string(profile_id,"22222222");
					test_string(session_id,"44444444");

					test_string(application_desc , APPDESC1);
					test_string(profile_desc, PROFDESC2);
					test_string(session_desc, DEFAULT_SESSION_DESC);

					mypause();


while(1){

			title("\nGoing back to the default session ...\n");

					set_string(session_id,"00000000");

					{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

					test_string(ardrone_name,"TEST_CONFIG2");
					test_int(bitrate_ctrl_mode,1);

					test_string(application_id,"00000000");
					test_string(profile_id,"00000000");
					test_string(session_id,"00000000");

					test_string(application_desc , DEFAULT_APPLICATION_DESC);
					test_string(profile_desc, DEFAULT_PROFILE_DESC);
					test_string(session_desc, DEFAULT_SESSION_DESC);
					
					mypause();


			title("\nGoing back to the 44444444 session ...\n");

					set_string(session_id,"44444444");
					set_string(session_desc,SESSDESC4);


					{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

					test_string(ardrone_name,"TEST_CONFIG2");
					test_int(bitrate_ctrl_mode,0);

					test_string(application_id,"11111111");
					test_string(profile_id,"22222222");
					test_string(session_id,"44444444");

					test_string(application_desc , APPDESC1);
					test_string(profile_desc, PROFDESC2);
					test_string(session_desc, SESSDESC4);

					mypause();


			title("\nGoing back to the default session ...\n");

					set_string(session_id,"00000000");

					{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

					test_string(ardrone_name,"TEST_CONFIG2"); set_string(ardrone_name,"TEST_CONFIG3");
					test_string(application_id,"00000000");
					test_string(profile_id,"00000000");
					test_string(session_id,"00000000");

					test_string(application_desc , DEFAULT_APPLICATION_DESC);
					test_string(profile_desc, DEFAULT_PROFILE_DESC);
					test_string(session_desc, DEFAULT_SESSION_DESC);

					mypause();


			title(" Going back to the 33333333 session ...\n");

					set_string(session_id,"33333333");

					{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

					test_string(ardrone_name,"TEST_CONFIG3");
					test_int(bitrate_ctrl_mode,1);           /* value in default config.ini */
					test_float(outdoor_euler_angle_max,F1);  /* value in default config.ini */
					test_int(detect_type,CAD_TYPE_COCARDE);

					test_string(application_id,"00000000");
					test_string(profile_id,"00000000");
					test_string(session_id,"33333333");

					test_string(application_desc , DEFAULT_APPLICATION_DESC);
					test_string(profile_desc, DEFAULT_PROFILE_DESC);
					test_string(session_desc, SESSDESC3);

					mypause();


			title("\nGoing back to the 44444444 session ...\n");

					set_string(session_id,"44444444");

					{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

					test_string(ardrone_name,"TEST_CONFIG3");    set_string(ardrone_name,"TEST_CONFIG4");
					test_int(bitrate_ctrl_mode,0);
					test_float(outdoor_euler_angle_max,F2);
					test_int(detect_type,CAD_TYPE_VISION);

					test_string(application_id,"11111111");
					test_string(profile_id,"22222222");
					test_string(session_id,"44444444");

					test_string(application_desc , APPDESC1);
					test_string(profile_desc, PROFDESC2);
					test_string(session_desc, SESSDESC4);

					mypause();


			title(" Going back to the 33333333 session ...\n");

					set_string(session_id,"33333333");

					{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

					test_int(bitrate_ctrl_mode,1);
					test_int(detect_type,CAD_TYPE_COCARDE);

					test_string(ardrone_name,"TEST_CONFIG4");
					test_int(bitrate_ctrl_mode,1);
					test_float(outdoor_euler_angle_max,F1);
					test_string(application_id,"00000000");
					test_string(profile_id,"00000000");
					test_string(session_id,"33333333");

					test_string(application_desc , DEFAULT_APPLICATION_DESC);
					test_string(profile_desc, DEFAULT_PROFILE_DESC);
					test_string(session_desc, SESSDESC3);

					mypause();


			title("\nGoing back to the default session ...\n");

				set_string(session_id,"00000000");

				{ARDRONE_TOOL_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

				test_string(ardrone_name,"TEST_CONFIG4");   /* common parameter whose value should never be reset */
				test_int(bitrate_ctrl_mode,1);
				test_float(outdoor_euler_angle_max,F1);
				test_int(detect_type,CAD_TYPE_NONE);

				test_string(application_id,"00000000");
				test_string(profile_id,"00000000");
				test_string(session_id,"00000000");

				test_string(application_desc , DEFAULT_APPLICATION_DESC);
				test_string(profile_desc, DEFAULT_PROFILE_DESC);
				test_string(session_desc, DEFAULT_SESSION_DESC);

				set_string(ardrone_name,"TEST_CONFIG2");

				mypause();


				/*-----------------------------------------------------------------------------------------------*/

				/* Read the list of custom configurations */
				title("\nReading the list of custom configuration files ...\n");
				{ARDRONE_TOOL_CUSTOM_CONFIGURATION_GET(test_callback);vp_os_cond_wait(&test_condition);}

				printf("\n\n ====================================== \n\n");

				if(available_configurations[CAT_APPLI].nb_configurations!=1) { printf("Bad number of custom applis.\n"); }
				if(available_configurations[CAT_USER].nb_configurations!=1) { printf("Bad number of custom applis.\n"); }
				if(available_configurations[CAT_SESSION].nb_configurations!=2) { printf("Bad number of custom applis.\n"); }

				if (available_configurations[CAT_APPLI].list[0].id==NULL) { printf("Unexpected custom app 0 NULL\n"); }
				else if (strcmp(available_configurations[CAT_APPLI].list[0].id,"11111111")) { printf("Unexpected custom app : <%s>\n",available_configurations[CAT_APPLI].list[0].id); }

				if (available_configurations[CAT_USER].list[0].id==NULL) { printf("Unexpected custom user 0 NULL\n"); }
				else if (strcmp(available_configurations[CAT_USER].list[0].id,"22222222")) { printf("Unexpected custom profile : <%s>\n",available_configurations[CAT_USER].list[0].id); }

				if (available_configurations[CAT_SESSION].list[0].id==NULL) { printf("Unexpected custom session 0 NULL\n"); }
				else if (strcmp(available_configurations[CAT_SESSION].list[0].id,"33333333")) { printf("Unexpected custom session 0 : <%s>\n",available_configurations[CAT_SESSION].list[0].id); }

				if (available_configurations[CAT_SESSION].list[1].id==NULL) { printf("Unexpected custom session 1 NULL\n"); }
				else if (strcmp(available_configurations[CAT_SESSION].list[1].id,"44444444")) { printf("Unexpected custom session 1 : <%s>\n",available_configurations[CAT_SESSION].list[1].id); }



				if (available_configurations[CAT_APPLI].list[0].description==NULL) { printf("Unexpected custom appli descrition 0 NULL\n"); }
				else if (strcmp(available_configurations[CAT_APPLI].list[0].description,APPDESC1)) { printf("Unexpected custom app description : <%s>\n",available_configurations[CAT_APPLI].list[0].description); }

				if (available_configurations[CAT_USER].list[0].description==NULL) { printf("Unexpected custom user 0 description NULL\n"); }
				else if (strcmp(available_configurations[CAT_USER].list[0].description,PROFDESC2)) { printf("Unexpected custom profile description : <%s>\n",available_configurations[CAT_USER].list[0].description); }

				if (available_configurations[CAT_SESSION].list[0].description==NULL) { printf("Unexpected custom session 0 description NULL\n"); }
				else if (strcmp(available_configurations[CAT_SESSION].list[0].description,SESSDESC3)) { printf("Unexpected custom session 0 description : <%s>\n",available_configurations[CAT_SESSION].list[0].description); }

				if (available_configurations[CAT_SESSION].list[1].description==NULL) { printf("Unexpected custom session 1 description NULL\n"); }
				else if (strcmp(available_configurations[CAT_SESSION].list[1].description,SESSDESC4)) { printf("Unexpected custom session 1 description : <%s>\n",available_configurations[CAT_SESSION].list[1].description); }



				printf("   Custom config list checked.\n");
				
				mypause();


				/*-----------------------------------------------------------------------------------------------*/

				printf("\n\n ====================================== \n\n");

				title("\nEnd of the test.\n");
	}
	return 0;
}
