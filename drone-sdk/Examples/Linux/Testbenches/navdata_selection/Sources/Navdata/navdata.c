#include <ardrone_tool/Navdata/ardrone_navdata_client.h>

#include <Navdata/navdata.h>

extern C_RESULT demo_navdata_client_init( void* data );
extern C_RESULT demo_navdata_client_process( const navdata_unpacked_t* const navdata );
extern C_RESULT demo_navdata_client_release( void );

/* Registering to navdata client */
BEGIN_NAVDATA_HANDLER_TABLE
  NAVDATA_HANDLER_TABLE_ENTRY(demo_navdata_client_init, demo_navdata_client_process, demo_navdata_client_release, NULL)
END_NAVDATA_HANDLER_TABLE

