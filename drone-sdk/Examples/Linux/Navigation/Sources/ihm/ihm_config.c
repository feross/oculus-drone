#include <gtk/gtk.h>
#include <VP_Os/vp_os_malloc.h>
#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_signal.h>

#include <ardrone_api.h>
#include <config_keys.h>
#include <ardrone_tool/ardrone_tool_configuration.h>

#include "ihm_config.h"

ihm_config_widgets_t ihm_config = {0};


#undef ARDRONE_CONFIG_KEY_IMM
#undef ARDRONE_CONFIG_KEY_REF
#undef ARDRONE_CONFIG_KEY_STR
#include <config_keys.h>

/* Prototypes */
	static void configuration_received_callback();
	/* These prototypes must be present, to check at compile time that the callback functions have the right number of arguments */
//	ardrone_tool_configuration_callback send_custom_application_request_callback;
//	ardrone_tool_configuration_callback send_custom_user_request_callback;
//	ardrone_tool_configuration_callback send_custom_session_request_callback;
	static void get_configuration_callback(GtkWidget *widget, gpointer data);
	static void set_configuration_callback(GtkWidget *widget, gpointer data);

/*-------------------------------------------------------------------------------------------------------------*/

	void send_custom_application_request_callback(bool_t res)  {	printf("Switched application config.\n");  }

	void send_custom_application_request(GtkWidget *widget, gpointer data)
	{
		const char * id;
		id = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));

		if (configuration_check_config_id(id)==C_OK)
		{
			printf("Requesting application user configuration <%s>\n",id);
			ARDRONE_TOOL_CONFIGURATION_ADDEVENT(application_id, (char*)id, send_custom_application_request_callback);
			ARDRONE_TOOL_CONFIGURATION_GET(configuration_received_callback);
		}
	}
	void send_new_custom_application_request(GtkWidget *widget, gpointer data)
	{
		const char * id;
		printf("reading from <%p>\n",ihm_config.new_custom_configs[CAT_APPLI].entry);

		id = gtk_entry_get_text(GTK_ENTRY(ihm_config.new_custom_configs[CAT_APPLI].entry));

		printf("Checking <%s>\n",id);
		if (configuration_check_config_id(id)==C_OK)
		{
			printf("Creating/requesting custom application configuration <%s>\n",id);
			ARDRONE_TOOL_CONFIGURATION_ADDEVENT(application_id, (char*)id, send_custom_application_request_callback);
			ARDRONE_TOOL_CONFIGURATION_GET(configuration_received_callback);
		}
	}

/*-------------------------------------------------------------------------------------------------------------*/

	void send_custom_user_request_callback(bool_t res)  { printf("Switched profile config.\n"); }

	void send_custom_user_request(GtkWidget *widget, gpointer data)
	{
		const char * id;
		id = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));

		if (configuration_check_config_id(id)==C_OK)
		{
			printf("Requesting custom user configuration <%s>\n",id);
			ARDRONE_TOOL_CONFIGURATION_ADDEVENT(profile_id, (char*)id, send_custom_user_request_callback);
			ARDRONE_TOOL_CONFIGURATION_GET(configuration_received_callback);
		}
	}
	void send_new_custom_user_request(GtkWidget *widget, gpointer data)
	{
		const char * id;
		id = gtk_entry_get_text(GTK_ENTRY(ihm_config.new_custom_configs[CAT_USER].entry));

		if (configuration_check_config_id(id)==C_OK)
		{
			printf("Creating/requesting custom user configuration <%s>\n",id);
			ARDRONE_TOOL_CONFIGURATION_ADDEVENT(profile_id, (char*)id, send_custom_application_request_callback);
			ARDRONE_TOOL_CONFIGURATION_GET(configuration_received_callback);
		}
	}

/*-------------------------------------------------------------------------------------------------------------*/

	void send_custom_session_request_callback(bool_t res)  { printf("Switched session config. acknowlegded by the drone.\n"); }

	void send_custom_session_request(GtkWidget *widget, gpointer data)
	{
		const char * id;
		id = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));

		if (configuration_check_config_id(id)==C_OK)
		{
			printf("Requesting custom session <%s>\n",id);
			ARDRONE_TOOL_CONFIGURATION_ADDEVENT(session_id, (char*)id, send_custom_session_request_callback);
			ARDRONE_TOOL_CONFIGURATION_GET(configuration_received_callback);
		}
	}
	void send_new_custom_session_request(GtkWidget *widget, gpointer data)
	{
		const char * id = gtk_entry_get_text(GTK_ENTRY(ihm_config.new_custom_configs[CAT_SESSION].entry));

		if (configuration_check_config_id(id)==C_OK)
		{
			printf("Creating/requesting custom session <%s>\n",id);
			ARDRONE_TOOL_CONFIGURATION_ADDEVENT(session_id, (char*)id, send_custom_application_request_callback);
			ARDRONE_TOOL_CONFIGURATION_GET(configuration_received_callback);
		}
	}

/*-------------------------------------------------------------------------------------------------------------*/


static void custom_configurations_received_callback(bool_t res)
{
	int i,j;

	if (res==TRUE)
	{
		PRINT("%s %s %i - Request success !\n",__FILE__,__FUNCTION__,__LINE__);

		gdk_threads_enter();

		for (i=1;i<NB_CONFIG_CATEGORIES;i++)
		{
			PRINT("Got %i ids for category %i\n",available_configurations[i].nb_configurations,i);

			if (available_configurations[i].nb_configurations==0)
			{
				gtk_combo_box_remove_text (GTK_COMBO_BOX(ihm_config.custom_config_values[i].combo),0);
				gtk_combo_box_insert_text(GTK_COMBO_BOX(ihm_config.custom_config_values[i].combo),0,"no custom config. detected");
			}
			else
			{
				for (j=0;j<available_configurations[i].nb_configurations;j++)
				{
					gtk_combo_box_remove_text (GTK_COMBO_BOX(ihm_config.custom_config_values[i].combo),j);
					gtk_combo_box_insert_text(GTK_COMBO_BOX(ihm_config.custom_config_values[i].combo),j,available_configurations[i].list[j].id);
				}
			}
		}
		//gtk_widget_show_all(ihm_config.window);
		gdk_threads_leave();
	}
	else
	{
		PRINT("%s %s %i - Request failure !\n",__FILE__,__FUNCTION__,__LINE__);
	}
}


static void configuration_received_callback(bool_t res)
{

	char labels[1024];
	char values[1024];
	int i,nb_values=0;

	gdk_threads_enter();

	PRINT("%s %s\n",__FILE__,__FUNCTION__);

	GtkRcStyle *rc_style1,*rc_style2;
	GdkColor color1,color2,white;

	  color1.red=40000;
	  color1.blue=40000;
	  color1.green=65535;
	  rc_style1=gtk_rc_style_new();
	 // rc_style1->bg[GTK_STATE_NORMAL]=color1;
	  rc_style1->base[GTK_STATE_NORMAL]=color1;
	 // rc_style1->color_flags[GTK_STATE_NORMAL]|=GTK_RC_BG;
	  rc_style1->color_flags[GTK_STATE_NORMAL]|=GTK_RC_BASE;

	  color2.red=0x8fff;
	  color2.blue=0x8fff;
	  color2.green=0x8fff;
	  white.red=0xffff;
	  white.blue=0xffff;
	  white.green=0xffff;

	  rc_style2=gtk_rc_style_new();
	  //rc_style2->bg[GTK_STATE_NORMAL]=color2;
	  rc_style2->base[GTK_STATE_NORMAL]=white;
	  //rc_style2->color_flags[GTK_STATE_NORMAL]|=GTK_RC_BG;
	  rc_style2->color_flags[GTK_STATE_NORMAL]|=GTK_RC_BASE;


	if (!GTK_IS_WIDGET(ihm_config.config_values_frame)) //{ gtk_widget_destroy(ihm_config.config_values_frame); ihm_config.config_values_frame = NULL; }
	{
		if (ihm_config.config_values!=NULL){	vp_os_free(ihm_config.config_values); ihm_config.config_values = NULL;	}

		ihm_config.config_values_frame = gtk_scrolled_window_new(NULL,NULL);
		gtk_box_pack_start(GTK_BOX(ihm_config.config_frame_vbox),ihm_config.config_values_frame,TRUE, TRUE, 0);
		ihm_config.config_values_hbox  = gtk_hbox_new(FALSE,0);
		ihm_config.config_values_vbox1 = gtk_vbox_new(FALSE,0);
		ihm_config.config_values_vbox2 = gtk_vbox_new(FALSE,0);
		gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW (ihm_config.config_values_frame),ihm_config.config_values_hbox);
		gtk_box_pack_start(GTK_BOX(ihm_config.config_values_hbox),ihm_config.config_values_vbox1,TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(ihm_config.config_values_hbox),ihm_config.config_values_vbox2,TRUE, TRUE, 0);

		#define DISPLAY(KEY,NAME,C_TYPE,RW)\
			ihm_config.config_values = vp_os_realloc(ihm_config.config_values,(nb_values+1)*sizeof(ihm_config_value_t)); \
			sprintf(labels,"%s:%s",KEY,CFG_STRINGIFY(NAME)); \
			ihm_config.config_values[nb_values].label = gtk_label_new(labels); \
			ihm_config.config_values[nb_values].value = gtk_entry_new(); \
			ihm_config.config_values[nb_values].hbox = gtk_hbox_new(FALSE,0); \
			if ((RW)&K_WRITE) ihm_config.config_values[nb_values].send_button = gtk_button_new_with_label("send"); else ihm_config.config_values[nb_values].send_button=NULL; \
			gtk_box_pack_start(GTK_BOX(ihm_config.config_values_vbox1),ihm_config.config_values[nb_values].label,FALSE, FALSE, ((RW)&K_WRITE)?6:5);\
			gtk_box_pack_start(GTK_BOX(ihm_config.config_values_vbox2),ihm_config.config_values[nb_values].hbox,FALSE, FALSE, 0);\
			gtk_box_pack_start(GTK_BOX(ihm_config.config_values[nb_values].hbox),ihm_config.config_values[nb_values].value,TRUE, TRUE, 0);\
			gtk_widget_modify_style(GTK_WIDGET(ihm_config.config_values[nb_values].value),rc_style2); \
			if (ihm_config.config_values[nb_values].send_button)\
			{gtk_box_pack_start(GTK_BOX(ihm_config.config_values[nb_values].hbox),ihm_config.config_values[nb_values].send_button,FALSE, FALSE, 0);\
			gtk_signal_connect(GTK_OBJECT(ihm_config.config_values[nb_values].send_button), "clicked", G_CALLBACK(set_configuration_callback), NULL);}\
			nb_values++;

		#undef ARDRONE_CONFIG_KEY_IMM
		#undef ARDRONE_CONFIG_KEY_REF
		#undef ARDRONE_CONFIG_KEY_STR
		#define ARDRONE_CONFIG_KEY_IMM(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK) DISPLAY(KEY,NAME,C_TYPE,RW)
		#define ARDRONE_CONFIG_KEY_REF(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK) DISPLAY(KEY,NAME,C_TYPE,RW)
		#define ARDRONE_CONFIG_KEY_STR(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK) DISPLAY(KEY,NAME,C_TYPE,RW)
		#include <config_keys.h>
	}

	//sprintf(labels,"%s:%s",STRINGIFY(abc),STRINGIFY(def));
	// sprintf(values,PRINTF_KEY(C_TYPE)),get_##NAME());
	//gtk_entry_set_text( GTK_ENTRY(ihm_config.config_values[nb_values].value) , values );


	//sprintf(values,PRINTF_KEY(uint32_t),123);

	i=0;

#define CONFIGURATION_GETTER(x) ardrone_control_config.x

#define ARDRONE_CONFIG_KEY_IMM(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK) \
		if(strcmp("" #INI_TYPE, "INI_FLOAT") == 0)	  { sprintf( values,"%f",(float)CONFIGURATION_GETTER(NAME)); } \
		else if(strcmp("" #INI_TYPE, "INI_INT") == 0) { sprintf( values,"%d",(int)CONFIGURATION_GETTER(NAME)); }  \
		else if(strcmp("" #INI_TYPE, "INI_BOOLEAN") == 0) { sprintf( values,"%s",(CONFIGURATION_GETTER(NAME)) ? "TRUE" : "FALSE"); }	   \
		if (strcmp(gtk_entry_get_text(GTK_ENTRY(ihm_config.config_values[i].value)),values)!=0) 	\
			gtk_widget_modify_style(GTK_WIDGET(ihm_config.config_values[i].value),rc_style1); \
			else gtk_widget_modify_style(GTK_WIDGET(ihm_config.config_values[i].value),rc_style2); \
		gtk_entry_set_text( GTK_ENTRY(ihm_config.config_values[i].value) , values );\
		i++;

		//PRINT(" Received parameter <%s:%s> with value <%f>\n",#KEY,#NAME,(float)CONFIGURATION_GETTER(NAME) );

#define ARDRONE_CONFIG_KEY_REF(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK)  \
		i++;

#define ARDRONE_CONFIG_KEY_STR(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK)  \
		if (strcmp(gtk_entry_get_text(GTK_ENTRY(ihm_config.config_values[i].value)),CONFIGURATION_GETTER(NAME))!=0) \
			gtk_widget_modify_style(GTK_WIDGET(ihm_config.config_values[i].value),rc_style1); \
			else gtk_widget_modify_style(GTK_WIDGET(ihm_config.config_values[i].value),rc_style2); \
		gtk_entry_set_text( GTK_ENTRY(ihm_config.config_values[i].value) , CONFIGURATION_GETTER(NAME) ); i++;

#include <config_keys.h>

	ihm_config.nb_config_values = i;

	gtk_widget_show_all(ihm_config.window);
	gtk_rc_style_unref(rc_style1);
	gtk_rc_style_unref(rc_style2);

	gdk_threads_leave();
}


static void get_configuration_callback(GtkWidget *widget, gpointer data)
{
	ARDRONE_TOOL_CONFIGURATION_GET(configuration_received_callback);
}

static void set_configuration_callback(GtkWidget *widget, gpointer data)
{
	int i;
	int config_index=-1;
	char * textval;

	//gdk_threads_enter();

	printf("Nb config values : %i\n",ihm_config.nb_config_values);

	for (i=0;i<ihm_config.nb_config_values;i++) { if (widget==ihm_config.config_values[i].send_button) config_index=i; }
	if (config_index==-1) { return; }
	GtkWidget * w = ihm_config.config_values[config_index].value;
	textval = (char*)gtk_entry_get_text((GtkEntry*)w);

	i=0;  // Index of the scanned configuration key

#define ARDRONE_CONFIG_KEY_INI_FLOAT(KEY, NAME, C_TYPE)                 \
        if (i==config_index){                                           \
          printf("Sending configuration <%s:%s> = <%s>\n",KEY,CFG_STRINGIFY(NAME),textval); \
          float fval; sscanf (textval, "%f", &fval);                    \
          ARDRONE_TOOL_CONFIGURATION_ADDEVENT (NAME, &fval, NULL);      \
        }i++;
#define ARDRONE_CONFIG_KEY_INI_DOUBLE(KEY, NAME, C_TYPE)                \
        if (i==config_index){                                           \
          printf("Sending configuration <%s:%s> = <%s>\n",KEY,CFG_STRINGIFY(NAME),textval); \
          float64_t dval; sscanf (textval, "%lf", &fdal);               \
          ARDRONE_TOOL_CONFIGURATION_ADDEVENT (NAME, &dval, NULL);      \
        }i++;
#define ARDRONE_CONFIG_KEY_INT_uint32_t(KEY, NAME)                      \
        if (i==config_index){                                           \
          printf("Sending configuration <%s:%s> = <%s>\n",KEY,CFG_STRINGIFY(NAME),textval); \
          uint32_t ival; sscanf (textval, "%u", &ival);                  \
          ARDRONE_TOOL_CONFIGURATION_ADDEVENT (NAME, &ival, NULL);      \
        }i++;
#define ARDRONE_CONFIG_KEY_INT_int32_t(KEY, NAME)                      \
        if (i==config_index){                                           \
          printf("Sending configuration <%s:%s> = <%s>\n",KEY,CFG_STRINGIFY(NAME),textval); \
          int32_t ival; sscanf (textval, "%d", &ival);                  \
          ARDRONE_TOOL_CONFIGURATION_ADDEVENT (NAME, &ival, NULL);      \
        }i++;
#define ARDRONE_CONFIG_KEY_INI_INT(KEY, NAME, C_TYPE)                   \
        ARDRONE_CONFIG_KEY_INT_##C_TYPE(KEY, NAME)
#define ARDRONE_CONFIG_KEY_INI_BOOLEAN(KEY, NAME, C_TYPE)               \
        if (i==config_index){                                           \
          printf("Sending configuration <%s:%s> = <%s>\n",KEY,CFG_STRINGIFY(NAME),textval); \
          bool_t bval = ((strcmp (textval, "TRUE")==0) ? TRUE : FALSE); \
          ARDRONE_TOOL_CONFIGURATION_ADDEVENT (NAME, &bval, NULL);      \
        }i++;          

#define ARDRONE_CONFIG_KEY_IMM(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK) \
        ARDRONE_CONFIG_KEY_##INI_TYPE(KEY, NAME, C_TYPE);
        /*        if(i==config_index){                                  \
          printf("Sending configuration <%s:%s> = <%s>\n",KEY,CFG_STRINGIFY(NAME),textval); \
          if     (strcmp("" #INI_TYPE, "INI_FLOAT") == 0) { float fval; sscanf( textval,"%f",&fval);	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(NAME, (C_TYPE_PTR)&fval, NULL); } \
          else if(strcmp("" #INI_TYPE, "INI_DOUBLE") == 0) { float64_t dval; sscanf( textval, "%lf", &dval);	ARDRONE_TOOL_CONFIGURATION_ADDEVENT(NAME, (C_TYPE_PTR)&dval, NULL); } \
          else if(strcmp("" #INI_TYPE, "INI_INT") == 0)	{ int32_t ival = atoi(textval); ARDRONE_TOOL_CONFIGURATION_ADDEVENT(NAME, (C_TYPE_PTR)&ival, NULL); } \
          else if(strcmp("" #INI_TYPE, "INI_BOOLEAN") == 0) { bool_t bval = ((strcmp(textval,"TRUE")==0) ? TRUE : FALSE); ARDRONE_TOOL_CONFIGURATION_ADDEVENT(NAME, (C_TYPE_PTR)&bval, NULL); } \
          }i++;*/
        
#define ARDRONE_CONFIG_KEY_REF(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK) i++;
        
#define ARDRONE_CONFIG_KEY_STR(KEY, NAME, INI_TYPE, C_TYPE, C_TYPE_PTR, RW, DEFAULT, CALLBACK) \
        if(i==config_index){                                            \
          printf("Sending configuration <%s:%s> = <%s>\n",KEY,CFG_STRINGIFY(NAME),textval); \
          ARDRONE_TOOL_CONFIGURATION_ADDEVENT(NAME, textval, NULL);     \
        }i++;
        
#include <config_keys.h>

	//gdk_threads_leave();
}

static void get_custom_configurations_callback(GtkWidget *widget, gpointer data)
{
	printf("%s %s %i\n",__FILE__,__FUNCTION__,__LINE__);
	ARDRONE_TOOL_CUSTOM_CONFIGURATION_GET(custom_configurations_received_callback);
	printf("%s %s %i\n",__FILE__,__FUNCTION__,__LINE__);
}



void ihm_config_create_window()
{
	int i;

	if (ihm_config.window!=NULL && GTK_IS_WIDGET(ihm_config.window)) { gtk_widget_show_all(ihm_config.window); return; }

	vp_os_memset(&ihm_config,0,sizeof(ihm_config));

	/* Main window */
		ihm_config.window = gtk_window_new( GTK_WINDOW_TOPLEVEL);

	/* Split the window in two halves */
		ihm_config.panes = gtk_hbox_new(FALSE, 20);
		gtk_container_add( GTK_CONTAINER(ihm_config.window), ihm_config.panes);

	/* Make two vertical frames next to each other*/
		ihm_config.config_frame = gtk_frame_new("Config");
		ihm_config.custom_config_frame = gtk_frame_new("Custom config.");
		gtk_box_pack_start(GTK_BOX(ihm_config.panes), ihm_config.config_frame ,TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(ihm_config.panes), ihm_config.custom_config_frame ,TRUE, TRUE, 0);
		ihm_config.config_frame_vbox = gtk_vbox_new(FALSE,0);
		ihm_config.custom_config_frame_vbox = gtk_vbox_new(FALSE,0);
		gtk_container_add( GTK_CONTAINER(ihm_config.config_frame) , ihm_config.config_frame_vbox  );
		gtk_container_add( GTK_CONTAINER(ihm_config.custom_config_frame) , ihm_config.custom_config_frame_vbox );

	/* Config frame */
		ihm_config.get_config_button = gtk_button_new_with_label("Get configuration");
		gtk_box_pack_start(GTK_BOX(ihm_config.config_frame_vbox),ihm_config.get_config_button,FALSE, FALSE, 0);
		gtk_signal_connect(GTK_OBJECT(ihm_config.get_config_button), "clicked", G_CALLBACK(get_configuration_callback), NULL);

	/* Custom Configs frame */
		ihm_config.get_custom_configs_button = gtk_button_new_with_label("Get custom configurations list");
		gtk_signal_connect(GTK_OBJECT(ihm_config.get_custom_configs_button), "clicked", G_CALLBACK(get_custom_configurations_callback), NULL);

		ihm_config.custom_config_values[1].label = gtk_label_new("Application");
		ihm_config.custom_config_values[2].label = gtk_label_new("User profile");
		ihm_config.custom_config_values[3].label = gtk_label_new("Session");

		gtk_box_pack_start(GTK_BOX(ihm_config.custom_config_frame_vbox),ihm_config.get_custom_configs_button,FALSE, FALSE, 0);

		for (i=1;i<NB_CONFIG_CATEGORIES;i++)
		{
			ihm_config.custom_config_values[i].hbox = gtk_hbox_new(FALSE,0);
			ihm_config.custom_config_values[i].combo = gtk_combo_box_new_text();
			ihm_config.custom_config_values[i].raz_button = gtk_button_new_with_label("reset");
			gtk_box_pack_start(GTK_BOX(ihm_config.custom_config_values[i].hbox),ihm_config.custom_config_values[i].label,FALSE,FALSE,0);
			gtk_box_pack_start(GTK_BOX(ihm_config.custom_config_values[i].hbox),ihm_config.custom_config_values[i].combo,FALSE,FALSE,0);
			gtk_box_pack_start(GTK_BOX(ihm_config.custom_config_values[i].hbox),ihm_config.custom_config_values[i].raz_button,FALSE,FALSE,0);

			gtk_box_pack_start(GTK_BOX(ihm_config.custom_config_frame_vbox),ihm_config.custom_config_values[i].hbox,FALSE,FALSE,0);

			gtk_combo_box_insert_text(GTK_COMBO_BOX(ihm_config.custom_config_values[i].combo),0,"press button to retrieve list");
		}

		gtk_signal_connect(GTK_OBJECT(ihm_config.custom_config_values[1].combo), "changed", G_CALLBACK(send_custom_application_request),NULL);
		gtk_signal_connect(GTK_OBJECT(ihm_config.custom_config_values[2].combo), "changed", G_CALLBACK(send_custom_user_request),NULL);
		gtk_signal_connect(GTK_OBJECT(ihm_config.custom_config_values[3].combo), "changed", G_CALLBACK(send_custom_session_request),NULL);


		ihm_config.new_custom_configs[1].label = gtk_label_new("New application");
		ihm_config.new_custom_configs[2].label = gtk_label_new("New user profile");
		ihm_config.new_custom_configs[3].label = gtk_label_new("New session");

		for (i=1;i<NB_CONFIG_CATEGORIES;i++)
		{
			ihm_config.new_custom_configs[i].hbox = gtk_hbox_new(FALSE,0);
			ihm_config.new_custom_configs[i].entry = gtk_entry_new();
			ihm_config.new_custom_configs[i].send_button = gtk_button_new_with_label("send");
			gtk_box_pack_start(GTK_BOX(ihm_config.new_custom_configs[i].hbox),ihm_config.new_custom_configs[i].label,FALSE,FALSE,0);
			gtk_box_pack_start(GTK_BOX(ihm_config.new_custom_configs[i].hbox),ihm_config.new_custom_configs[i].entry,FALSE,FALSE,0);
			gtk_box_pack_start(GTK_BOX(ihm_config.new_custom_configs[i].hbox),ihm_config.new_custom_configs[i].send_button,FALSE,FALSE,0);

			gtk_box_pack_start(GTK_BOX(ihm_config.custom_config_frame_vbox),ihm_config.new_custom_configs[i].hbox,FALSE,FALSE,0);
		}

		gtk_signal_connect(GTK_OBJECT(ihm_config.new_custom_configs[1].send_button), "clicked", G_CALLBACK(send_new_custom_application_request),NULL);
		gtk_signal_connect(GTK_OBJECT(ihm_config.new_custom_configs[2].send_button), "clicked", G_CALLBACK(send_new_custom_user_request),NULL);
		gtk_signal_connect(GTK_OBJECT(ihm_config.new_custom_configs[3].send_button), "clicked", G_CALLBACK(send_new_custom_session_request),NULL);

		gtk_widget_show_all(ihm_config.window);
}
