#ifndef _NAVDATA_CLIENT_H_
#define _NAVDATA_CLIENT_H_

#include <config.h>

enum {
  NAVDATA_IHM_PROCESS_INDEX,
#ifdef ND_WRITE_TO_FILE
  NAVDATA_FILE_PROCESS_INDEX,

#ifdef PC_USE_POLARIS
  NAVDATA_POLARIS_PROCESS_INDEX,
#endif // PC_USE_POLARIS

#ifdef USE_TABLE_PILOTAGE
  NAVDATA_TABLEPILOTAGE_INDEX
#endif // USE_TABLE_PILOTAGE

#endif // ND_WRITE_TO_FILE
};

#include <ardrone_tool/Navdata/ardrone_navdata_client.h>

#endif //! _NAVDATA_CLIENT_H_

