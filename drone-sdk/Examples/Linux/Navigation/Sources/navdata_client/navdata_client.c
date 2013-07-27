#include <ardrone_tool/Navdata/ardrone_navdata_file.h>
#include <ardrone_tool/Control/ardrone_navdata_control.h>
#include "navdata_client/navdata_client.h"
#include "navdata_client/navdata_ihm.h"
#include "navdata_client/navdata_polaris.h"
#include "navdata_client/navdata_tablepilotage.h"

BEGIN_NAVDATA_HANDLER_TABLE
  NAVDATA_HANDLER_TABLE_ENTRY(navdata_ihm_init, navdata_ihm_process, navdata_ihm_release, NULL)
#ifdef ND_WRITE_TO_FILE
  NAVDATA_HANDLER_TABLE_ENTRY(ardrone_navdata_file_init, ardrone_navdata_file_process, ardrone_navdata_file_release, NULL)

#ifdef PC_USE_POLARIS
  NAVDATA_HANDLER_TABLE_ENTRY(navdata_polaris_init, navdata_polaris_process, navdata_polaris_release, NULL)
#endif // PC_USE_POLARIS

#ifdef USE_TABLE_PILOTAGE
  NAVDATA_HANDLER_TABLE_ENTRY(navdata_tablepilotage_init, navdata_tablepilotage_process, navdata_tablepilotage_release, NULL)
#endif // USE_TABLE_PILOTAGE

#endif // ND_WRITE_TO_FILE
END_NAVDATA_HANDLER_TABLE
