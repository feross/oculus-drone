
#ifndef _WIIMOTE_H_
#define _WIIMOTE_H_

#include "UI/ui.h"

#define WIIMOTE_FIND_TIMEOUT 5

extern input_device_t wiimote_device;

C_RESULT open_wiimote(void);
C_RESULT update_wiimote(void);
C_RESULT close_wiimote(void);

#endif // _WIIMOTE_H_

