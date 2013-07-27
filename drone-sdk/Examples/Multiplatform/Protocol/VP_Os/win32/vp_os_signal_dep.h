/**
 *  \brief    OS Api for video sdk. Public definitions.
 *  \author   Aurelien Morelle <aurelien.morelle@parrot.fr>
 *  \author   Sylvain Gaeremynck <sylvain.gaeremynck@parrot.fr>
 *  \version  2.0
 *  \date     2006/12/15
 */

#ifndef _SIGNAL_INCLUDE_OS_DEP_
#define _SIGNAL_INCLUDE_OS_DEP_


#include <VP_Os/vp_os.h>

typedef CRITICAL_SECTION vp_os_mutex_t;
typedef CRITICAL_SECTION vp_os_cond_t;
/*
CONDITION_VARIABLE only work under VISTA
typedef struct
{
  CONDITION_VARIABLE  cond;
  vp_os_mutex_t       *mutex;
}
vp_os_cond_t;
*/


#endif // ! _SIGNAL_INCLUDE_OS_DEP_

