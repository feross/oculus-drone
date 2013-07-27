#ifndef _VICON_IHM_H_
#define _VICON_IHM_H_

#include <vicon.h>
#include <vicon_file.h>

C_RESULT vicon_ihm_init( void* data );
C_RESULT vicon_ihm_process( const vicon_data_t* const vicon_data );
C_RESULT vicon_ihm_release( void );

#endif //! _VICON_IHM_H_

