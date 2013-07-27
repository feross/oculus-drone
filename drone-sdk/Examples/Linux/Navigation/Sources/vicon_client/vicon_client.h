#ifndef _VICON_CLIENT_H_
#define _VICON_CLIENT_H_

#include <vicon.h>
#include <vicon_file.h>

C_RESULT vicon_at_client_init( void* data );
C_RESULT vicon_at_client_process( const vicon_data_t* const vicon_data );
C_RESULT vicon_at_client_release( void );

#endif //! _VICON_CLIENT_H_

