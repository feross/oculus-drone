#include <ardrone_tool/Navdata/ardrone_navdata_file.h>

#include "navdata_client/navdata_ihm.h"

#ifdef USE_TABLE_PILOTAGE
#include "libTestBenchs/novadem.h"

C_RESULT navdata_tablepilotage_init( void* dev )
{
  return C_OK;
}

C_RESULT navdata_tablepilotage_process( const navdata_unpacked_t* const navdata )
{
  if( navdata_file != NULL )
  {
    novadem_data_t data;

    novadem_get_data( &data );

    fprintf(navdata_file,"; % d; % d", data.position, data.speed);
  }

  return C_OK;
}

C_RESULT navdata_tablepilotage_release( void )
{
  return C_OK;
}

#endif
