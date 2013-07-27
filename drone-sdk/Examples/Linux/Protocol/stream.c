/**
 * @file my_client.c
 * @author karl leplat
 * @date 2009/07/01
 */
#include <assert.h>
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

#include "vlib.h"
#include "app.h"

/* Video constant */
#define VIDEO_PORT                5555
#define RAW_CAPTURE_PORT          5557

#define H_ACQ_WIDTH               VIDEO_WIDTH
#define H_ACQ_HEIGHT              VIDEO_HEIGHT
#define VIDEO_BUFFER_SIZE         (2*(VIDEO_WIDTH)*(VIDEO_HEIGHT))

#define DATA_RECEIVED_MAX_SIZE    8192

/* Private variables  */
static video_controller_t controller;
static vp_api_picture_t	picture;
static int pictureBpp;

// note: allocate twice the size of an image
uint16_t picture_buf[VIDEO_BUFFER_SIZE];
int num_picture_decoded = 0;
static pthread_t stream_thread = 0;
static int32_t stream_thread_alive = 1;
static int video_udp_socket  = -1;

void video_write (int8_t *buffer, int32_t len)
{
  struct sockaddr_in to;
  int32_t flags;

  if( video_udp_socket < 0 )
  {
    struct sockaddr_in video_udp_addr;

    memset( (char*)&video_udp_addr, 0, sizeof(video_udp_addr) );

    video_udp_addr.sin_family      = AF_INET;
    video_udp_addr.sin_addr.s_addr = INADDR_ANY;
    video_udp_addr.sin_port        = htons( VIDEO_PORT + 100 );

    video_udp_socket = socket( AF_INET, SOCK_DGRAM, 0 );

    if( video_udp_socket > 0 )
    {
		 flags = fcntl(video_udp_socket, F_GETFL, 0);
		 if( flags >= 0 )
		 {
			 flags |= O_NONBLOCK;

			 flags = fcntl(video_udp_socket, F_SETFL, flags );
		 }
		 else
		 {
			 INFO("Get Socket Options failed\n");
		 }

       bind(video_udp_socket, (struct sockaddr*)&video_udp_addr, sizeof(struct sockaddr) );
    }
  }

  if( video_udp_socket > 0 )
  {
     int res;

	  memset( (char*)&to, 0, sizeof(to) );
	  to.sin_family       = AF_INET;
	  to.sin_addr.s_addr  = inet_addr(WIFI_MYKONOS_IP); // BROADCAST address for subnet 192.168.1.xxx
	  to.sin_port         = htons (VIDEO_PORT);

	  res = sendto( video_udp_socket, (char*)buffer, len, 0, (struct sockaddr*)&to, sizeof(to) );
  }
}

void* stream_loop(void *arg)
{
	C_RESULT status;
	int sockfd, addr_in_size;
	struct sockaddr_in *my_addr, *from;

	INFO("VIDEO stream thread starting (thread=%d)...\n", (int)pthread_self());

	addr_in_size = sizeof(struct sockaddr_in);

	from = (struct sockaddr_in *)malloc(addr_in_size);
	my_addr = (struct sockaddr_in *)malloc(addr_in_size);
	assert(from);
	assert(my_addr);

	memset((char *)my_addr,(char)0,addr_in_size);
	my_addr->sin_family = AF_INET;
	my_addr->sin_addr.s_addr = htonl(INADDR_ANY);
	my_addr->sin_port = htons( VIDEO_PORT );

	if((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0){
		INFO ("socket: %s\n", strerror(errno));
		goto fail;
	};

	if (bind(sockfd, (struct sockaddr *)my_addr, addr_in_size) < 0){
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

	INFO("Ready to receive video data...\n");
	{
		int32_t one = 1;
		video_write ((int8_t*)&one, sizeof( one ));
	}

	while( stream_thread_alive )
	{
		int size;
		size = recvfrom (sockfd,
				video_controller_get_stream_ptr(&controller),
				DATA_RECEIVED_MAX_SIZE, 0, (struct sockaddr *)from,
				(socklen_t*)&addr_in_size);

		if (size == 0)
		{
			INFO ("Lost connection \n");
			int32_t one = 1;
			video_write ((int8_t*)&one, sizeof(one));
		}
		else if (size > 0) {
			int decodeOK = FALSE;

			controller.in_stream.used	= size;
			controller.in_stream.size	= size;
			controller.in_stream.index	= 0;
			controller.in_stream.length	= 32;
			controller.in_stream.code	= 0;

			//INFO("decode_blockline(size=%d)\n", size);
			status = video_decode_blockline(&controller, &picture, &decodeOK);
			if (status) {
				INFO("video_decode_blockline() failed\n");
			}
			else if (decodeOK) {
				// this is ugly: update counter and notify video renderer
				num_picture_decoded = controller.num_frames;
				//INFO("num_picture_decoded = %d\n", num_picture_decoded);
			}
		}
	}
fail:
	free(from);
	free(my_addr);
	if (sockfd >= 0){
		close(sockfd);
		sockfd = -1;
	}

	INFO("VIDEO stream thread stopping\n");

	return NULL;
}

void stream_run(void)
{
	C_RESULT status;

	if( stream_thread )
		return;

	memset(&controller, 0, sizeof(controller));
	memset(&picture, 0, sizeof(picture));
	memset(picture_buf, 0, sizeof(picture_buf));
	pictureBpp			  = 2;

	/// Picture configuration
	picture.format	      = PIX_FMT_RGB565;
	picture.width         = H_ACQ_WIDTH;
	picture.height        = H_ACQ_HEIGHT;
	picture.framerate     = 15;
	picture.y_line_size   = picture.width * pictureBpp;
	picture.cb_line_size  = 0;
	picture.cr_line_size  = 0;
	picture.y_buf         = (uint8_t *)picture_buf;
	picture.cb_buf	      = NULL;
	picture.cr_buf	      = NULL;

	status = video_codec_open( &controller, UVLC_CODEC );
	if (status) {
		INFO("video_codec_open() failed\n");
	}
	video_controller_set_motion_estimation(&controller, FALSE);
	video_controller_set_format( &controller, H_ACQ_WIDTH, H_ACQ_HEIGHT );

	if( pthread_create(&stream_thread, NULL, stream_loop, NULL) )
	{
		video_codec_close(&controller);
		INFO("pthread_create: %s\n", strerror(errno));
	}
}

void stream_stop(void)
{
	if ( !stream_thread )
		return;

	stream_thread_alive = 0;
	pthread_join(stream_thread, NULL);
	stream_thread = 0;

	if ( video_udp_socket >= 0 ){
		close( video_udp_socket );
		video_udp_socket = -1;
	}

	video_codec_close( &controller );
}

