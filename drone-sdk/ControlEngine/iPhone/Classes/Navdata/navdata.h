#ifndef _NAVDATA_H_
#define _NAVDATA_H_

#include "ConstantsAndMacros.h"
#include "ARDroneTypes.h"

C_RESULT ardrone_navdata_reset_data(navdata_unpacked_t *nav);
C_RESULT ardrone_navdata_get_data(navdata_unpacked_t *data);
C_RESULT ardrone_navdata_write_to_file(bool_t enable);
ARDRONE_FLYING_STATE ardrone_navdata_get_flying_state(navdata_unpacked_t data);

#endif // _NAVDATA_H_