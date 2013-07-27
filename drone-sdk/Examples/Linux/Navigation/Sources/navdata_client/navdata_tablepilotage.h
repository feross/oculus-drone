#ifndef _NAVDATA_TABLEPILOTAGE_H_
#define _NAVDATA_TABLEPILOTAGE_H_

#include <ardrone_tool/Navdata/ardrone_navdata_client.h>

C_RESULT navdata_tablepilotage_init( void* param );
C_RESULT navdata_tablepilotage_process( const navdata_unpacked_t* const navdata );
C_RESULT navdata_tablepilotage_release( void );

#endif // _NAVDATA_TABLEPILOTAGE_H_
