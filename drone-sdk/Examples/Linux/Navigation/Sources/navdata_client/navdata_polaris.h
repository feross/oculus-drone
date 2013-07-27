#ifndef _NAVDATA_POLARIS_H_
#define _NAVDATA_POLARIS_H_

#include <ardrone_tool/Navdata/ardrone_navdata_client.h>

C_RESULT navdata_polaris_init( void* data );
C_RESULT navdata_polaris_process( const navdata_unpacked_t* const navdata );
C_RESULT navdata_polaris_release( void );

#endif // _NAVDATA_POLARIS_H_
