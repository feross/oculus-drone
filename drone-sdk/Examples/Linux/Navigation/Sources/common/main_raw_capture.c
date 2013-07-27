/**
 * @file mobile_main.c
 * @author aurelien.morelle@parrot.fr
 * @date 2006/05/01
 */


#include "rfcomm.h"
#include "bnep.h"
#include "common/mobile_config.h"

#include <VP_Api/vp_api.h>
#include <VP_Api/vp_api_error.h>
#include <VP_Api/vp_api_thread_helper.h>
#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_types.h>
#include <VP_Os/vp_os_malloc.h>
#include <VP_Api/vp_api_io_com.h>

#include "joystick.h"

//#include <netdb.h>
//#include <sys/socket.h>

//#include <net/if.h>
//#include <netinet/in.h>

/* #include "Common.h" */


// #include "pthread.h"
/* #ifndef UART_LINK */
/* #endif */


// #include "mutex.h"
// #include "thread.h"

/* #include "commonControl.h" */

/* #if defined(USE_AT_RESPONSES) && !defined(BITRATE_TEST) */
/* #include "AT_Processing/AT_read.h" */
/* #endif */

/* #ifdef USE_IHM */
/* #include "ihm.h" */
/* #endif */

/* #ifdef NAVDATA_CLIENT */
/* #include "navdata_client.h" */
/* #endif */

#include "ref_send.h"
#include "UI/ui.h"

extern int connect_wifi();

char shMemory[SH_MEMORY_SIZE];

/************************************************************************
 * Mutex related
 ************************************************************************/

/* extern vp_os_mutex_t PrintfLock; */
/* vp_os_mutex_t SHMemoryLock; */
/* extern vp_os_mutex_t SHMemoryReadLock; */
/* extern vp_os_mutex_t SHMemoryWriteLock; */
/* vp_os_mutex_t ResponseReceivedLock; */

/* void init_all_mutex() */
/* { */
/*  vp_os_mutex_init(&PrintfLock); */
/*  vp_os_mutex_init(&SHMemoryLock); */
/* #ifdef BLUETOOTH_CARD */
/*  vp_os_mutex_init(&SHMemoryReadLock); */
/* #endif */
/*  vp_os_mutex_init(&SHMemoryWriteLock); */
/*  vp_os_mutex_init(&ResponseReceivedLock); */

/*   vp_os_mutex_unlock(&PrintfLock); */
/*   vp_os_mutex_unlock(&SHMemoryLock); */
/* #ifdef BLUETOOTH_CARD */
/*   vp_os_mutex_unlock(&SHMemoryReadLock); */
/* #endif */
/*   vp_os_mutex_unlock(&SHMemoryWriteLock); */
/*   vp_os_mutex_unlock(&ResponseReceivedLock); */
/* } */

/* void delete_all_mutex() */
/* { */
/*   vp_os_mutex_destroy(&PrintfLock); */
/*   vp_os_mutex_destroy(&SHMemoryLock); */
/* #ifdef BLUETOOTH_CARD */
/*   vp_os_mutex_destroy(&SHMemoryReadLock); */
/* #endif */
/*   vp_os_mutex_destroy(&SHMemoryWriteLock); */
/*   vp_os_mutex_destroy(&ResponseReceivedLock); */
/* } */

#ifdef RFCOMM_OVER_IP
#  define OPEN_AT_SOCKET(Server)  open_bnep_socket(SERVERHOST, PORT_AT)
#  define READ_AT_SOCKET  read_bnep_socket
#  define CLOSE_AT_SOCKET close_bnep_socket
#else // ! RFCOMM_OVER_IP
#  define OPEN_AT_SOCKET(Server)  open_rfcomm_socket(Server)
#  define READ_AT_SOCKET  read_rfcomm_socket
#  define CLOSE_AT_SOCKET close_rfcomm_socket
#endif // <- RFCOMM_OVER_IP

PROTO_THREAD_ROUTINE(raw_image_sequence_capture, params);

BEGIN_THREAD_TABLE
THREAD_TABLE_ENTRY(process_events  , 10)
THREAD_TABLE_ENTRY(ref_send,       20)
THREAD_TABLE_ENTRY(raw_image_sequence_capture,       20)
END_THREAD_TABLE


/////////////////////////////////////////////////////////////////////////////////////
///                            MAIN
/////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
/*   void *res; */
/*   int attempt, ret; */
  mobile_config_t cfg;

/*   pthread_attr_t pthread_custom_attr; */

/*   pthread_t process_events_thread;  /\* joystick *\/ */
/*   pthread_t ref_send_thread; /\* envoie des consignes *\/ */



  /* beginning of code */
  /* ----------------- */
  if(argc > 1)
    {
      fprintf(stderr, "Usage : raw_image_capture\n");
      return EXIT_FAILURE;
    }

  ///////////////////////////  ///////////////////////////
  //           STEP 1 : CONNECTION WIFI PAR CLE USB
  ///////////////////////////  ///////////////////////////
/*   attempt = 0; */
/*   while((ret = connect_wifi()) && ++attempt < 5) */
/*     { */
/*       printf("Connexion attempt(s) failed : %i\n", attempt); */
/*     } */
/*   if(ret) */
/*     { */
/*       return EXIT_FAILURE; */
/*     } */
/*   printf("connect_wifi OK!\n"); */



  ///////////////////////////  ///////////////////////////
  //           STEP 1 : CONFIGURATION DU PAD
  ///////////////////////////  ///////////////////////////
/*   init_all_mutex(); */

  if(mobile_init_config(&cfg))
    {
      return EXIT_FAILURE;
    }
  joystick_selection(&cfg);


  ///////////////////////////  ///////////////////////////
  //           STEP 1 : THREAD DES COMMANDES AT
  ///////////////////////////  ///////////////////////////
  if ((cfg.at_socket = OPEN_AT_SOCKET(cfg.addr)) == -1)
    {
      return EXIT_FAILURE;
    }

  START_THREAD(process_events, &cfg);
  START_THREAD(ref_send, &cfg);
  START_THREAD(raw_image_sequence_capture, &cfg);

  JOIN_THREAD(process_events);
  JOIN_THREAD(ref_send);
  JOIN_THREAD(raw_image_sequence_capture);

/*   pthread_kill_other_threads_np(); */

/*   delete_all_mutex(); */
  mobile_exit_config(&cfg);

  return EXIT_SUCCESS;
}


/* void *video_decode(void *data) */
PROTO_THREAD_ROUTINE(raw_image_sequence_capture, params)
{
  int size, sizeCumul;
  uint8_t inbuf[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];

  mobile_config_t *cfg = (mobile_config_t *) params;

  if ((cfg->sock = open_bnep_socket(SERVERHOST, PORT)) == -1)
    {
      return NULL;
    }

  /* set end of buffer to 0 (this ensures that no overreading happens for damaged mpeg streams) */
  vp_os_memset(inbuf + INBUF_SIZE, 0, FF_INPUT_BUFFER_PADDING_SIZE);
  sizeCumul = 0;

  for(;;) {

    size = read(cfg->sock, inbuf, INBUF_SIZE);
    printf(".");
    if(size>0)
      {
	sizeCumul += size;
	printf("READ SIZE = %d\t sizeCumul = %d\t image = %d\n", size, sizeCumul, sizeCumul / 25344);
	fwrite(inbuf, sizeof(char), size, cfg->sample_file);
	if(sizeCumul==253440)
	  {
	    break;
	  }
      }

    if (size == 0 || (size < 0 && errno == EAGAIN))
      continue;
    if (size < 0)
      break;


  }
  fclose(cfg->sample_file);

  if(do_exit(cfg))
    {
      close_bnep_socket(cfg->sock);
      return (void *)-1;
    }

  
  return NULL;
}



