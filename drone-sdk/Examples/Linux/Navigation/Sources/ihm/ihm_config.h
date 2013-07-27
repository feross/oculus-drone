#include <config_keys.h>

typedef struct
{
	GtkWidget * label;
	GtkWidget * hbox;
	GtkWidget * value;
	GtkWidget * send_button;
	int isnew;
}ihm_config_value_t;

typedef struct
{
	GtkWidget * hbox;
	GtkWidget * label;
	GtkWidget * combo;
	GtkWidget * raz_button;
}ihm_config_customconf_box_t;

typedef struct
{
	GtkWidget * hbox;
	GtkWidget * label;
	GtkWidget * entry;
	GtkWidget * send_button;
}ihm_config_newcustomconf_box_t;

typedef struct{

	GtkWidget * window;

	GtkWidget * panes;

	GtkWidget * config_frame;
	GtkWidget * custom_config_frame;
	GtkWidget * config_frame_vbox;
	GtkWidget * custom_config_frame_vbox;


	GtkWidget * get_config_button;
	GtkWidget * config_values_frame;
	GtkWidget * config_values_hbox;
	GtkWidget * config_values_vbox1;
	GtkWidget * config_values_vbox2;

	int nb_config_values;
	ihm_config_value_t* config_values;

	GtkWidget * get_custom_configs_button;

	ihm_config_customconf_box_t custom_config_values[NB_CONFIG_CATEGORIES];
	ihm_config_newcustomconf_box_t new_custom_configs[NB_CONFIG_CATEGORIES];



}ihm_config_widgets_t;

void ihm_config_create_window();
