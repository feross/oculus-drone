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

uint32_t mykonos_state = 0;

/* AT constant */
#define AT_PORT                   5556
#define AT_BUFFER_SIZE            1024

typedef enum {
    MYKONOS_UI_BIT_AG             = 0, /* Button turn to left */
    MYKONOS_UI_BIT_AB             = 1, /* Button altitude down (ah - ab)*/
    MYKONOS_UI_BIT_AD             = 2, /* Button turn to right */
    MYKONOS_UI_BIT_AH             = 3, /* Button altitude up (ah - ab)*/
    MYKONOS_UI_BIT_L1             = 4, /* Button - z-axis (r1 - l1) */
    MYKONOS_UI_BIT_R1             = 5, /* Not used */
    MYKONOS_UI_BIT_L2             = 6, /* Button + z-axis (r1 - l1) */ 
    MYKONOS_UI_BIT_R2             = 7, /* Not used */
    MYKONOS_UI_BIT_SELECT         = 8, /* Button emergency reset all */
    MYKONOS_UI_BIT_START          = 9, /* Button Takeoff / Landing */
    MYKONOS_UI_BIT_TRIM_THETA     = 18, /* y-axis trim +1 (Trim increase at +/- 1??/s) */
    MYKONOS_UI_BIT_TRIM_PHI       = 20, /* x-axis trim +1 (Trim increase at +/- 1??/s) */
    MYKONOS_UI_BIT_TRIM_YAW       = 22, /* z-axis trim +1 (Trim increase at +/- 1??/s) */
    MYKONOS_UI_BIT_X              = 24, /* x-axis +1 */
    MYKONOS_UI_BIT_Y              = 28, /* y-axis +1 */
} mykonos_ui_bitfield_t;

static int at_udp_socket  = -1;

static inline int get_mask_from_state( uint32_t state, uint32_t mask )
{
    return state & mask ? TRUE : FALSE;
}

void at_write (int8_t *buffer, int32_t len)
{
    struct sockaddr_in to;
    int32_t flags;

    if( at_udp_socket < 0 ) {
        struct sockaddr_in at_udp_addr;

        memset( (char*)&at_udp_addr, 0, sizeof(at_udp_addr) );

        at_udp_addr.sin_family      = AF_INET;
        at_udp_addr.sin_addr.s_addr = INADDR_ANY;
        at_udp_addr.sin_port        = htons( AT_PORT + 100 );

        at_udp_socket = socket( AF_INET, SOCK_DGRAM, 0 );
    
        if( at_udp_socket >= 0 )
            {
                flags = fcntl(at_udp_socket, F_GETFL, 0);
                if( flags >= 0 )
                    {
                        flags |= O_NONBLOCK;

                        flags = fcntl(at_udp_socket, F_SETFL, flags );
                    }
                else
                    {
                        INFO("Get Socket Options failed\n");
                    }

                if (bind(at_udp_socket, (struct sockaddr*)&at_udp_addr, sizeof(struct sockaddr)) < 0) {
                    INFO ("at_write:bind: %s\n", strerror(errno));
                }
            }
    }

    if( at_udp_socket >= 0 )
        {
            int res;

            memset( (char*)&to, 0, sizeof(to) );
            to.sin_family       = AF_INET;
            to.sin_addr.s_addr  = inet_addr(WIFI_MYKONOS_IP); // BROADCAST address for subnet 192.168.1.xxx
            to.sin_port         = htons (AT_PORT);

            res = sendto( at_udp_socket, (char*)buffer, len, 0, (struct sockaddr*)&to, sizeof(to) );
        }
}

void set_radiogp_input( int32_t pitch, int32_t roll, int32_t gaz, int32_t yaw )
{
    /*pitch, roll, yaw : values between -32767 and +32767 */
    /*gaz : GAZ_REF_MAX = 20 */
	char cmd[AT_BUFFER_SIZE];
	memset (cmd, 0, AT_BUFFER_SIZE); 
	sprintf (cmd, "AT*RADGP=%d,%d,%d,%d\r", pitch, roll, gaz, yaw);
	at_write ((int8_t*)cmd, strlen (cmd));
}
void set_ui_input( uint32_t value )
{
	char cmd[AT_BUFFER_SIZE];
	memset (cmd, 0, AT_BUFFER_SIZE); 
	sprintf (cmd, "AT*REF=%d\r", value);
	at_write ((int8_t*)cmd, strlen (cmd));
}

void set_iphone_acceleros( int enable, float32_t fax, float32_t fay, float32_t faz )
{
	char cmd[AT_BUFFER_SIZE];
	memset (cmd, 0, AT_BUFFER_SIZE); 
	sprintf (cmd, "AT*ACCS=%d,%d,%d,%d\r", (int32_t)enable, (int32_t)fax, (int32_t)fay, (int32_t)faz);
	at_write ((int8_t*)cmd, strlen (cmd));
}

void set_flat_trim( void )
{
	const char cmd[] = "AT*FTRIM\r";
	at_write ((int8_t*)cmd, strlen (cmd));
}

struct event_info motion_info;
struct event_info trackball_info;
struct orientation_info orientation;

static unsigned long get_time_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

static void boot_drone(void)
{
	const char cmds[] = "AT*CONFIG=\"general:navdata_demo\",\"TRUE\"\r";

	if ( get_mask_from_state( mykonos_state,
							  MYKONOS_NAVDATA_BOOTSTRAP )) {
		at_write ((int8_t*)cmds, strlen (cmds));

		int retry = 20;
		int bcontinue = TRUE;
		int next = 0;
		while (bcontinue && retry) {
				if (next == 0) {
					if ( get_mask_from_state( mykonos_state, MYKONOS_COMMAND_MASK )) {
						INFO ("[CONTROL] Processing the current command ... \n");
						next++;
					}
				}
				else {
					char str[AT_BUFFER_SIZE];
					memset (str, 0, AT_BUFFER_SIZE); 
					sprintf (str, "AT*CTRL=%d,%d\r", ACK_CONTROL_MODE, 0);
					at_write ((int8_t*)str, strlen (str));

					if ( !get_mask_from_state( mykonos_state, MYKONOS_COMMAND_MASK )) {
						INFO ("[CONTROL] Ack control event OK, send navdata demo\n");
						bcontinue = FALSE;
					}
				}
				sleep (1);
				retry--;
		}
	}
}

#define BUFSIZE  256

#define MYKONOS_NO_TRIM							\
    ((1 << MYKONOS_UI_BIT_TRIM_THETA) |			\
	 (1 << MYKONOS_UI_BIT_TRIM_PHI) |			\
	 (1 << MYKONOS_UI_BIT_TRIM_YAW) |			\
	 (1 << MYKONOS_UI_BIT_X) |					\
	 (1 << MYKONOS_UI_BIT_Y))

static uint32_t ui_mask = MYKONOS_NO_TRIM;

static int start_button = 0;
static int overflow = 0;
static unsigned long ocurrent = 0;

static void send_command(int nb_sequence)
{
	unsigned long current;
 	char str[BUFSIZE];
	int32_t pitch = 0, roll = 0, yaw = 0, gaz = 0, sw, sh;

	//FIXME: access to motion/trackball/orientation is not thread-safe
	// this should not really matter for this demo
	if (motion_info.state) {

		get_screen_dimensions(&sw, &sh);

		// up/down zone => gaz +/- 500 mm/s
		if (motion_info.x <= sw/3) {
			gaz = (motion_info.y > sh/2)? 10000 : -10000;
		}

		// take off/landing zone (toggle start bit)
		if (!start_button && (motion_info.x >= 2*sw/3) &&
			(motion_info.y <= sh/3)) {
			ui_mask ^= (1 << MYKONOS_UI_BIT_START);
			start_button = 1;
		}
		//INFO("touch @ (%d,%d)\n", motion_info.x, motion_info.y);
	}
	else {
		start_button = 0;
	}

	if (trackball_info.state) {
		pitch = -(orientation.values[2]+20)*1000;
		if (pitch >= 25000) {
			pitch = 25000;
		}
		if (pitch <= -25000) {
			pitch = -25000;
		}
		roll = orientation.values[1]*1000;
		if (roll >= 25000) {
			roll = 25000;
		}
		if (roll <= -25000) {
			roll = -25000;
		}
		yaw = (trackball_info.x-1)*4000;
		/*
		INFO("yaw %c orientation: %d %d\n", yaw[trackball_info.x],
			 orientation.values[1], orientation.values[2]);
		*/
	}
	// send command to drone
	current = get_time_ms();

	//FIXME: are allowed to concatenate AT commands this way ?
	snprintf(str, BUFSIZE, "AT*SEQ=%d\rAT*RADGP=%d,%d,%d,%d\rAT*REF=%d\r",
			 nb_sequence, pitch, roll, gaz, yaw, ui_mask);
	at_write((int8_t*)str, strlen (str));

	// check 30 ms overflow
	if (current > ocurrent + 30) {
		overflow += current - ocurrent - MYKONOS_REFRESH_MS;
	}
	ocurrent = current;

	/* dump command every 2s */
	if ((nb_sequence & 63) == 0) {
		INFO("seq=%d radgp(%d,%d,%d,%d) ui=0x%08x over=%d\n", nb_sequence,
			 pitch, roll, gaz, yaw, ui_mask, overflow);
		overflow = 0;
	}
}

DEFINE_THREAD_ROUTINE( at_cmds_loop, data )
{
	unsigned long current, deadline;
	unsigned int nb_sequence = 0;

	INFO("AT commands thread starting (thread=%d)...\n", (int)pthread_self());

	ui_mask = MYKONOS_NO_TRIM;
	start_button = 0;
	motion_info.state = 0;
	trackball_info.state = 0;
	trackball_info.x = 1;
	orientation.values[0] = 0;
	orientation.values[1] = 0;
	orientation.values[2] = 0;
	ocurrent = get_time_ms();

    while (gAppAlive) {

		if (get_mask_from_state(mykonos_state, MYKONOS_NAVDATA_BOOTSTRAP)) {
			INFO("attempting to boot drone...\n");
			boot_drone();
			nb_sequence = 0;
			continue;
		}

		// compute next loop iteration deadline
		deadline = get_time_ms() + MYKONOS_REFRESH_MS;

		// send pilot command
		send_command(nb_sequence++);

		// sleep until deadline
		current = get_time_ms();
		if (current < deadline) {
			usleep(1000*(deadline-current));
		}
	}

    if (at_udp_socket >= 0){
        close(at_udp_socket);
        at_udp_socket = -1;
    }

	INFO("AT commands thread stopping\n");
    return NULL;
}
