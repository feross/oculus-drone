#ifndef _NAVDATA_IHM_H_
#define _NAVDATA_IHM_H_

#include "common/mobile_config.h"
#include <ardrone_tool/Navdata/ardrone_navdata_client.h>

C_RESULT navdata_ihm_init( mobile_config_t* cfg );
C_RESULT navdata_ihm_process( const navdata_unpacked_t* const navdata );
C_RESULT navdata_ihm_release( void );

#endif // _NAVDATA_IHM_H_
