
#ifndef _IHM_VISION_H
#define _IHM_VISION_H

extern char label_vision_state_value[32];
extern GtkLabel *label_vision_values;

void ihm_ImageWinDestroy( GtkWidget *widget, gpointer data );

void create_image_window( void );

#endif // _IHM_VISION_H
