/**
 * @file my_client.c
 * @author karl leplat
 * @date 2009/07/01
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/time.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "app.h"

/* Navdata constant */
#define NAVDATA_SEQUENCE_DEFAULT  1
#define NAVDATA_PORT              5554
#define NAVDATA_HEADER            0x55667788
#define NAVDATA_BUFFER_SIZE       2048

typedef enum _navdata_tag_t {
    NAVDATA_DEMO_TAG = 0,
    NAVDATA_VISION_DETECT_TAG = 16,
    NAVDATA_IPHONE_ANGLES_TAG = 18,
    NAVDATA_CKS_TAG = 0xFFFF
} navdata_tag_t;

typedef struct _matrix33_t
{ 
    float32_t m11;
    float32_t m12;
    float32_t m13;
    float32_t m21;
    float32_t m22;
    float32_t m23;
    float32_t m31;
    float32_t m32;
    float32_t m33;
} matrix33_t;

typedef struct _vector31_t {
    union {
        float32_t v[3];
        struct    
        {
            float32_t x;
            float32_t y;
            float32_t z;
        };
    };
} vector31_t; 

typedef struct _navdata_option_t {
    uint16_t  tag;
    uint16_t  size;

    uint8_t   data[];
} navdata_option_t;
typedef struct _navdata_t {
    uint32_t    header;
    uint32_t    mykonos_state;
    uint32_t    sequence;
    int      vision_defined;

    navdata_option_t  options[1];
} __attribute__ ((packed)) navdata_t;

typedef struct _navdata_cks_t {
    uint16_t  tag;
    uint16_t  size;

    // Checksum for all navdatas (including options)
    uint32_t  cks;
} __attribute__ ((packed)) navdata_cks_t;

typedef struct _navdata_demo_t {
    uint16_t    tag;
    uint16_t    size;

    uint32_t    ctrl_state;             /*!< instance of #def_mykonos_state_mask_t */
    uint32_t    vbat_flying_percentage; /*!< battery voltage filtered (mV) */

    float32_t   theta;                  /*!< UAV's attitude */
    float32_t   phi;                    /*!< UAV's attitude */
    float32_t   psi;                    /*!< UAV's attitude */

    int32_t     altitude;               /*!< UAV's altitude */

    float32_t   vx;                     /*!< UAV's estimated linear velocity */
    float32_t   vy;                     /*!< UAV's estimated linear velocity */
    float32_t   vz;                     /*!< UAV's estimated linear velocity */

    uint32_t    num_frames;			  /*!< streamed frame index */

    // Camera parameters compute by detection
    matrix33_t  detection_camera_rot;
    matrix33_t  detection_camera_homo;
    vector31_t  detection_camera_trans;

    // Camera parameters compute by drone
    matrix33_t  drone_camera_rot;
    vector31_t  drone_camera_trans;
} __attribute__ ((packed)) navdata_demo_t;

typedef struct _navdata_iphone_angles_t {
    uint16_t   tag;
    uint16_t   size;

    int32_t    enable;
    float32_t  ax;
    float32_t  ay;
    float32_t  az;
    uint32_t   elapsed;
} __attribute__ ((packed)) navdata_iphone_angles_t;

typedef struct _navdata_time_t {
    uint16_t  tag;
    uint16_t  size;
  
    uint32_t  time;
} __attribute__ ((packed)) navdata_time_t;

typedef struct _navdata_vision_detect_t {
    uint16_t   tag;
    uint16_t   size;
  
    uint32_t   nb_detected;  
    uint32_t   type[4];
    uint32_t   xc[4];        
    uint32_t   yc[4];
    uint32_t   width[4];     
    uint32_t   height[4];    
    uint32_t   dist[4];      
} __attribute__ ((packed)) navdata_vision_detect_t;

typedef struct _navdata_unpacked_t {
    uint32_t  mykonos_state;
    int    vision_defined;

    navdata_demo_t           navdata_demo;
    navdata_iphone_angles_t  navdata_iphone_angles;
    navdata_vision_detect_t  navdata_vision_detect;
} navdata_unpacked_t;

/* Private variables */
static pthread_t nav_thread = 0;
static int32_t nav_thread_alive = 1;

static void navdata_open_server (void);
static void navdata_write (int8_t *buffer, int32_t len);
static void mykonos_navdata_unpack_all(navdata_unpacked_t* navdata_unpacked, navdata_t* navdata, uint32_t* cks);
static void* navdata_loop(void *arg);

static inline int get_mask_from_state( uint32_t state, uint32_t mask )
{
    return state & mask ? TRUE : FALSE;
}

static inline uint8_t* navdata_unpack_option( uint8_t* navdata_ptr, uint8_t* data, uint32_t size )
{
    memcpy(data, navdata_ptr, size);

    return (navdata_ptr + size);
}


static inline navdata_option_t* navdata_next_option( navdata_option_t* navdata_options_ptr )
{
    uint8_t* ptr;

    ptr  = (uint8_t*) navdata_options_ptr;
    ptr += navdata_options_ptr->size;

    return (navdata_option_t*) ptr;
}

navdata_option_t* navdata_search_option( navdata_option_t* navdata_options_ptr, uint32_t tag );


static inline uint32_t navdata_compute_cks( uint8_t* nv, int32_t size )
{
    int32_t i;
    uint32_t cks;
    uint32_t temp;

    cks = 0;

    for( i = 0; i < size; i++ )
        {
            temp = nv[i];
            cks += temp;
        }

    return cks;
}

#define navdata_unpack( navdata_ptr, option ) (navdata_option_t*) navdata_unpack_option( (uint8_t*) navdata_ptr, \
                                                                                         (uint8_t*) &option, \
                                                                                         navdata_ptr->size )
static void mykonos_navdata_unpack_all(navdata_unpacked_t* navdata_unpacked, navdata_t* navdata, uint32_t* cks)
{
    navdata_cks_t navdata_cks = { 0 };
    navdata_option_t* navdata_option_ptr;

    navdata_option_ptr = (navdata_option_t*) &navdata->options[0];

    memset( navdata_unpacked, 0, sizeof(*navdata_unpacked) );

    navdata_unpacked->mykonos_state   = navdata->mykonos_state;
    navdata_unpacked->vision_defined  = navdata->vision_defined;

    while( navdata_option_ptr != NULL )
        {
            // Check if we have a valid option
            if( navdata_option_ptr->size == 0 )
                {
                    INFO ("One option is not a valid because its size is zero\n");
                    navdata_option_ptr = NULL;
                }
            else
                {
                    switch( navdata_option_ptr->tag )
                        {
                        case NAVDATA_DEMO_TAG:
                            //INFO ("Demo tag\n");
                            navdata_option_ptr = navdata_unpack( navdata_option_ptr, navdata_unpacked->navdata_demo );
                            break;
        
                        case NAVDATA_IPHONE_ANGLES_TAG:
                            navdata_option_ptr = navdata_unpack( navdata_option_ptr, navdata_unpacked->navdata_iphone_angles );
                            break;

                        case NAVDATA_VISION_DETECT_TAG:
                            navdata_option_ptr = navdata_unpack( navdata_option_ptr, navdata_unpacked->navdata_vision_detect );
                            break;

                        case NAVDATA_CKS_TAG:
                            navdata_option_ptr = navdata_unpack( navdata_option_ptr, navdata_cks );
                            *cks = navdata_cks.cks;
                            navdata_option_ptr = NULL; // End of structure
                            break;

                        default:
                            INFO ("Tag %d is not a valid navdata option tag\n", (int) navdata_option_ptr->tag);
                            navdata_option_ptr = NULL;
                            break;
                        }
                }
        }
}

static int navdata_udp_socket  = -1;
static void navdata_write (int8_t *buffer, int32_t len)
{
    struct sockaddr_in to;
    int32_t flags;

    if (navdata_udp_socket < 0)
        {
            /// Open udp socket to broadcast at commands to other mykonos
            struct sockaddr_in navdata_udp_addr;

            memset( (char*)&navdata_udp_addr, 0, sizeof(navdata_udp_addr) );

            navdata_udp_addr.sin_family      = AF_INET;
            navdata_udp_addr.sin_addr.s_addr = INADDR_ANY;
            navdata_udp_addr.sin_port        = htons(NAVDATA_PORT + 100);

            navdata_udp_socket = socket( AF_INET, SOCK_DGRAM, 0 );

            if( navdata_udp_socket >= 0 )
                {
                    flags = fcntl(navdata_udp_socket, F_GETFL, 0);
                    if( flags >= 0 )
                        {
                            flags |= O_NONBLOCK;

                            flags = fcntl(navdata_udp_socket, F_SETFL, flags );
                        }
                    else
                        {
                            INFO("Get Socket Options failed\n");
                        }

                    if (bind(navdata_udp_socket, (struct sockaddr*)&navdata_udp_addr, sizeof(struct sockaddr)) < 0) {
                        INFO ("navdata_write:bind: %s\n", strerror(errno));
                    }
                }
        }
    if( navdata_udp_socket >= 0 ) {
        int res;

        memset( (char*)&to, 0, sizeof(to) );
        to.sin_family       = AF_INET;
        to.sin_addr.s_addr  = inet_addr(WIFI_MYKONOS_IP); // BROADCAST address for subnet 192.168.1.xxx
        to.sin_port         = htons( NAVDATA_PORT );

        res = sendto( navdata_udp_socket, (char*)buffer, len, 0, (struct sockaddr*)&to, sizeof(to) );
    }
}

static void navdata_open_server (void)
{
    int32_t one = 1;
    navdata_write ((int8_t*)&one, sizeof( one ));
}

static void* navdata_loop(void *arg)
{
	uint8_t msg[NAVDATA_BUFFER_SIZE];
    navdata_unpacked_t navdata_unpacked;
    unsigned int cks, navdata_cks, sequence = NAVDATA_SEQUENCE_DEFAULT-1;
    int sockfd = -1, addr_in_size;
	struct sockaddr_in *my_addr, *from;

	INFO("NAVDATA thread starting (thread=%d)...\n", (int)pthread_self());

    navdata_open_server();

	addr_in_size = sizeof(struct sockaddr_in);

    navdata_t* navdata = (navdata_t*) &msg[0];

	from = (struct sockaddr_in *)malloc(addr_in_size);
	my_addr = (struct sockaddr_in *)malloc(addr_in_size);
    assert(from);
    assert(my_addr);

	memset((char *)my_addr,(char)0,addr_in_size);
	my_addr->sin_family = AF_INET;
	my_addr->sin_addr.s_addr = htonl(INADDR_ANY);
	my_addr->sin_port = htons( NAVDATA_PORT );

	if((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0){
        INFO ("socket: %s\n", strerror(errno));
        goto fail;
	};

	if(bind(sockfd, (struct sockaddr *)my_addr, addr_in_size) < 0){
        INFO ("bind: %s\n", strerror(errno));
        goto fail;
	};

	{
		struct timeval tv;
		// 1 second timeout
		tv.tv_sec   = 1;
		tv.tv_usec  = 0;
		setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	}

	INFO("Ready to receive\n");

	while ( nav_thread_alive ) {
		int size;
		size = recvfrom (sockfd, &msg[0], NAVDATA_BUFFER_SIZE, 0, (struct sockaddr *)from,
                         (socklen_t *)&addr_in_size);

		if( size == 0 )
            {
                INFO ("Lost connection \n");
                navdata_open_server();
                sequence = NAVDATA_SEQUENCE_DEFAULT-1;
            }
		if( navdata->header == NAVDATA_HEADER )
            {
                mykonos_state = navdata->mykonos_state;
			
                if( get_mask_from_state(navdata->mykonos_state, MYKONOS_COM_WATCHDOG_MASK) ) 
                    { 
                        INFO ("[NAVDATA] Detect com watchdog\n");
                        sequence = NAVDATA_SEQUENCE_DEFAULT-1; 

                        if( get_mask_from_state(navdata->mykonos_state, MYKONOS_NAVDATA_BOOTSTRAP) == FALSE ) 
                            {
                                const char cmds[] = "AT*COMWDG\r";
                                at_write ((int8_t*)cmds, strlen( cmds ));
                            }
                    } 

                if( navdata->sequence > sequence ) 
                    { 
                        if ( get_mask_from_state( mykonos_state, MYKONOS_NAVDATA_DEMO_MASK ))
                            {
                                mykonos_navdata_unpack_all(&navdata_unpacked, navdata, &navdata_cks);
                                cks = navdata_compute_cks( &msg[0], size - sizeof(navdata_cks_t) );

                                if( cks == navdata_cks )
                                    {
                                        //INFO ("Unpack navdata\n");
                                    }
                            }
                    } 
                else 
                    { 
                        INFO ("[Navdata] Sequence pb : %d (distant) / %d (local)\n", navdata->sequence, sequence); 
                    } 

                sequence = navdata->sequence;
            }
	}
 fail:
    free(from);
    free(my_addr);

    if (sockfd >= 0){
        close(sockfd);
    }

    if (navdata_udp_socket >= 0){
        close(navdata_udp_socket);
        navdata_udp_socket = -1;
    }

	INFO("NAVDATA thread stopping\n");
    return NULL;
}

/* Public functions */
void navdata_stop( void )
{
	if ( !nav_thread )
		return;
   
   nav_thread_alive = 0;
	pthread_join(nav_thread, NULL);
	nav_thread = 0;

	if (navdata_udp_socket >= 0){
		close(navdata_udp_socket);
		navdata_udp_socket = -1;
	}
}

void navdata_run( void )
{
   if ( nav_thread )
      return;

   nav_thread_alive = 1;
	if ( pthread_create( &nav_thread, NULL, navdata_loop, NULL ) )
   {
		INFO("pthread_create: %s\n", strerror(errno));
	}
}

