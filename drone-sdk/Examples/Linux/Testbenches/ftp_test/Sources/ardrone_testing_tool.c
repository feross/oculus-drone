/**
 * @file main.c
 * @author sylvain.gaeremynck@parrot.com
 * @date 2009/07/01
 */
#include <ardrone_testing_tool.h>

//ARDroneLib
#include <ardrone_tool/ardrone_time.h>
#include <ardrone_tool/Navdata/ardrone_navdata_client.h>
#include <ardrone_tool/Control/ardrone_control.h>
#include <ardrone_tool/UI/ardrone_input.h>
#include <ardrone_tool/ardrone_tool_configuration.h>

//Common
#include <config.h>
#include <ardrone_api.h>

//VP_SDK
#include <ATcodec/ATcodec_api.h>
#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_thread.h>
#include <VP_Api/vp_api_thread_helper.h>
#include <VP_Os/vp_os_signal.h>
#include <VP_Os/vp_os_types.h>
#include <VP_Os/vp_os_delay.h>

//Local project
#include <UI/gamepad.h>
#include <Video/video_stage.h>

#include <utils/ardrone_ftp.h>

#include <semaphore.h>

static int32_t exit_ihm_program = 1;

/* Implementing Custom methods for the main function of an ARDrone application */

/* The delegate object calls this method during initialization of an ARDrone application */
C_RESULT ardrone_tool_init_custom(int argc, char **argv)
{
  /* Start all threads of your application */
  START_THREAD( main_application_thread , NULL );
  return C_OK;
}

/* The delegate object calls this method when the event loop exit */
C_RESULT ardrone_tool_shutdown_custom()
{
  /* Relinquish all threads of your application */
  JOIN_THREAD( main_application_thread );
  
  return C_OK;
}

/* The event loop calls this method for the exit condition */
bool_t ardrone_tool_exit()
{
  return exit_ihm_program == 0;
}

C_RESULT signal_exit()
{
  exit_ihm_program = 0;
  
  return C_OK;
}

PROTO_THREAD_ROUTINE(main_application_thread, data);

/* Implementing thread table in which you add routines of your application and those provided by the SDK */
BEGIN_THREAD_TABLE
THREAD_TABLE_ENTRY( main_application_thread, 20 )
END_THREAD_TABLE
  
sem_t synchroSem;

#define BLUE { printf("%s","\033[34;01m"); }
#define RED { printf("%s","\033[31;01m"); }
#define GREEN { printf("%s","\033[32;01m"); }
#define RAZ { printf("%s","\033[0m"); }

#define TEST_RESULT(result, formatFAIL, formatOK, ...)  \
  do                                                    \
    {                                                   \
      _ftp_status locResult = (result);                 \
      totalOp++;                                        \
      if (FTP_FAILED (locResult))                       \
        {                                               \
          RED;                                          \
          printf ("[%i : KO] : ", __LINE__);            \
          printf (formatFAIL, __VA_ARGS__);             \
          printf ("\n");                                \
          RAZ;                                          \
        }                                               \
      else                                              \
        {                                               \
          GREEN;                                        \
          printf ("[%i : OK] : ", __LINE__);            \
          printf (formatOK, __VA_ARGS__);               \
          printf ("\n");                                \
          RAZ;                                          \
          opOk++;                                       \
        }                                               \
      if ((FTP_TIMEOUT == locResult) ||                 \
          (FTP_BUSY == locResult))                      \
        {                                               \
          runOperation = 1;                             \
        }                                               \
      else                                              \
        {                                               \
          runOperation = 0;                             \
        }                                               \
    }                                                   \
  while (0)

#define COMPUTE_PERCENT_OK                                              \
  do                                                                    \
    {                                                                   \
      if (totalOp> 0)                                                   \
        {                                                               \
          float percentOk = (100.0 * opOk) / (1.0 * totalOp);           \
          if (90.0 < percentOk)                                         \
            {                                                           \
              GREEN;                                                    \
            }                                                           \
          else                                                          \
            {                                                           \
              RED;                                                      \
            }                                                           \
          printf ("Success of %llu operations on %llu (%6.2f %%)\n", opOk, totalOp, percentOk); \
          RAZ;                                                          \
        }                                                               \
    } while (0)


#define TEST_ABORTION(abortFunc)                                \
  do                                                            \
    {                                                           \
      _ftp_status locStat = (abortFunc);                        \
      totalOp++;                                                \
      switch (locStat)                                          \
        {                                                       \
        case FTP_SUCCESS:                                       \
          GREEN;                                                \
          printf ("[%i : OK] : ", __LINE__);                    \
          printf ("Abortion successfull\n");                    \
          RAZ;                                                  \
          opOk++;                                               \
          break;                                                \
        case FTP_SAMESIZE:                                      \
          RED;                                                  \
          printf ("[%i : KO] : ", __LINE__);                    \
          printf ("Abortion failed, op was not running\n");     \
          RAZ;                                                  \
          break;                                                \
        case FTP_FAIL:                                          \
        default:                                                \
          RED;                                                  \
          printf ("[%i : KO] : ", __LINE__);                    \
          printf ("Unknown result\n");                          \
          RAZ;                                                  \
          break;                                                \
        }                                                       \
    } while (0)                                                 \
    
#define WAIT_COND(result)                       \
  do                                            \
    {                                           \
      sem_wait (&synchroSem);                   \
    } while (0)


#define SEND_COND                               \
  do                                            \
    {                                           \
      sem_post (&synchroSem);                   \
    }                                           \
  while (0)

_ftp_status globalStatus;
int runOperation = 1;
char *listBuffer = NULL;

void
ftpCallback (_ftp_status status, void *arg, _ftp_t *callingFtp)
{
  float percent = (NULL != arg) ? *(float *)arg : -1.0;
  if (NULL != arg && FTP_SUCCESS == status) { listBuffer = arg; }
  globalStatus = status;
  static int calls = 0;
#define CALLBACK_FORMAT "Callback called with status %s\n"
#ifdef DEBUG
#define PRINT_CALLBACK_FORMAT(statusString) { printf (CALLBACK_FORMAT, statusString); calls++; }
#else
#define PRINT_CALLBACK_FORMAT(statusString) { calls++; }
#endif
  switch (status)
    {
    case FTP_BUSY:
      PRINT_CALLBACK_FORMAT ("FTP_BUSY");
      SEND_COND;
      break;
    case FTP_ABORT:
      PRINT_CALLBACK_FORMAT ("FTP_ABORT");
      SEND_COND;
      break;
    case FTP_FAIL:
      PRINT_CALLBACK_FORMAT ("FTP_FAIL");
      SEND_COND;
      break;
    case FTP_SUCCESS:
      PRINT_CALLBACK_FORMAT ("FTP_SUCCESS");
      SEND_COND;
      break;
    case FTP_TIMEOUT:
      PRINT_CALLBACK_FORMAT ("FTP_TIMEOUT");
      SEND_COND;
      break;
    case FTP_BADSIZE:
      PRINT_CALLBACK_FORMAT ("FTP_BADSIZE");
      SEND_COND;
      break;
    case FTP_SAMESIZE:
      PRINT_CALLBACK_FORMAT ("FTP_SAMESIZE");
      SEND_COND;
      break;
    case FTP_PROGRESS:
      printf ("\rProgress : %5.2f", percent);
      fflush (stdout);
      if (100.0 <= percent)
        {
          printf ("\n");
        }
      break;
    default: // Should not happen ... kill the program in this case !
      printf ("Callback called with unknown status : %d\n", status);
      exit (-1);
      break;
    }
#undef CALLBACK_FORMAT
}
  
#define title(x) { BLUE; printf("[%i] %s",__LINE__,x); RAZ; }

#define _USE_EMENCIA_SERVER (0)
#if _USE_EMENCIA_SERVER
#define ARDRONE_IP "parrot02.nyx.emencia.net"
#define ARDRONE_PORT (21)
#define ARDRONE_USER "parrot"
#define ARDRONE_PASSWORD "parrot"
#else
#define ARDRONE_IP "192.168.1.1"
#define ARDRONE_PORT (21)
#define ARDRONE_USER "anonymous"
#define ARDRONE_PASSWORD ""
#endif

#define _1MB_ORIGIN_FILE "origin.txt"
#define _512K_FAILED_FILE "half_file.txt"
#define _256K_FAILED_FILE "quarter_file.txt"
#define _1MB_MERGED_FILE "merged.txt"
#define LOCAL_RESULT_FILE "result.txt"

#define NOCB_NORESUME_RESULT "result_nocb_noresume.txt"
#define CB_NORESUME_RESULT "result_cb_noresume.txt"
#define NOCB_RESUME_RESULT "result_nocb_resume.txt"
#define CB_RESUME_RESULT "result_cb_resume.txt"

#define ARDRONE_FILE_BEFORE_RENAME "origin.txt"
#define ARDRONE_FILE_AFTER_RENAME "result.txt"

DEFINE_THREAD_ROUTINE(main_application_thread, data)
{
  
  /* Semaphore for synchronisation */
  sem_init (&synchroSem, 0, 0);
  
  vp_os_delay(1000);

  char buffer [512] = {0};
  int systemRet = 0;
  
  _ftp_t *droneFtp = NULL;
  _ftp_status ftp_result = FTP_FAIL;

  int runOperation = 1;

  uint64_t totalOp = 0;
  uint64_t opOk = 0;

  
  title ("\n\n------ CLEANING ------\n\n");
  
  title ("--- Connecting to AR.Drone FTP ---\n");
  
  for (runOperation = 1; runOperation;)
    {
      droneFtp = ftpConnect (ARDRONE_IP, ARDRONE_PORT, ARDRONE_USER, ARDRONE_PASSWORD, &ftp_result);
      TEST_RESULT (ftp_result, "Unable to connect to %s@%s:%d (PASS : %s)", "Connected to %s@%s:%d (PASS : %s)", ARDRONE_USER, ARDRONE_IP, ARDRONE_PORT, ARDRONE_PASSWORD);
    }
  
  title ("--- Getting PWD ---\n");
  
  char pwd [256] = {0};
  ftp_result = ftpPwd (droneFtp, pwd, 256);
  TEST_RESULT (ftp_result, "Unable to get PWD [%1s]", "Got PWD : %s", pwd);

  title ("--- Removing previous file on AR.Drone ---\n");

  for (runOperation = 1; runOperation;)
    {
      ftp_result = ftpRemove (droneFtp, ARDRONE_FILE_BEFORE_RENAME);
      TEST_RESULT (ftp_result, " (STILL OK !) file %s does not exist", "Deleted file %s", ARDRONE_FILE_BEFORE_RENAME);
    }

  for (runOperation = 1; runOperation;)
    {
      ftp_result = ftpRemove (droneFtp, ARDRONE_FILE_AFTER_RENAME);
      TEST_RESULT (ftp_result, " (STILL OK !) file %s does not exist", "Deleted file %s", ARDRONE_FILE_AFTER_RENAME);
    }

  title ("--- Removing local result file ---\n");
 
  vp_os_memset (buffer, 0x0, 512);
  snprintf (buffer, 512, "rm -f %s", LOCAL_RESULT_FILE);
  systemRet = system (buffer);
  printf ("System : %s\n", buffer);

  title ("--- Disconnecting from AR.Drone FTP ---\n");

  TEST_RESULT (ftpClose (&droneFtp), "AR.Drone FTP (%s:%d) was not opened", "Closed AR.Drone FTP (%s:%d)", ARDRONE_IP, ARDRONE_PORT);

  while (1)
    {

      title ("\n\n------ TESTS WITHOUT RESUME NOR CALLBACK ------\n\n");
  
      title ("--- Connecting to AR.Drone FTP ---\n");
      
      for (runOperation = 1; runOperation;)
        {  
          droneFtp = ftpConnect (ARDRONE_IP, ARDRONE_PORT, ARDRONE_USER, ARDRONE_PASSWORD, &ftp_result);
          TEST_RESULT (ftp_result, "Unable to connect to %s@%s:%d (PASS : %s)", "Connected to %s@%s:%d (PASS : %s)", ARDRONE_USER, ARDRONE_IP, ARDRONE_PORT, ARDRONE_PASSWORD);
        }
  
      title ("--- Sending origin file to the AR.Drone ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpPut (droneFtp, _1MB_ORIGIN_FILE, ARDRONE_FILE_BEFORE_RENAME, 0, NULL);
          TEST_RESULT (ftp_result, "Unable to send file %s->%s", "Sent file %s->%s", _1MB_ORIGIN_FILE, ARDRONE_FILE_BEFORE_RENAME);
        }

      title ("--- Renaming file on the AR.Drone ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpRename (droneFtp, ARDRONE_FILE_BEFORE_RENAME, ARDRONE_FILE_AFTER_RENAME);
          TEST_RESULT (ftp_result, "Impossible to rename %s->%s", "Renamed %s->%s", ARDRONE_FILE_BEFORE_RENAME, ARDRONE_FILE_AFTER_RENAME);
        }

      title ("--- Getting AR.Drone FTP Listing ---\n");
  
      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpList (droneFtp, &listBuffer, NULL);
          TEST_RESULT (ftp_result, "Unable to list FTP [%1s]", "Listed FTP :\n%s", listBuffer);
          if (NULL != listBuffer) { vp_os_free (listBuffer); listBuffer = NULL; }
        }

      title ("--- Getting back result file ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpGet (droneFtp, ARDRONE_FILE_AFTER_RENAME, LOCAL_RESULT_FILE, 0, NULL);
          TEST_RESULT (ftp_result, "Unable to get file %s->%s", "Got back file %s->%s", ARDRONE_FILE_AFTER_RENAME, LOCAL_RESULT_FILE);
        }

      title ("--- Checking file ---\n");
 
      vp_os_memset (buffer, 0x0, 512);
      snprintf (buffer, 512, "diff %s %s > /dev/null", LOCAL_RESULT_FILE, _1MB_ORIGIN_FILE);
      systemRet = system (buffer);
      printf ("System : %s\n", buffer);
      totalOp++;
      if (0 != systemRet)
        {
          RED; printf ("[%d : KO] : Files are different !\n", __LINE__); RAZ;
        }
      else
        {
          opOk++;
          GREEN; printf ("[%d : OK] : Files are the same\n", __LINE__); RAZ;
        }

      title ("--- Removing file on AR.Drone ---\n");

      for (runOperation = 1; runOperation;)
        {      
          ftp_result = ftpRemove (droneFtp, ARDRONE_FILE_AFTER_RENAME);
          TEST_RESULT (ftp_result, "Unable to delete file %s", "Deleted file %s", ARDRONE_FILE_AFTER_RENAME);
        }

      title ("--- Local file backup ---\n");
  
      vp_os_memset (buffer, 0x0, 512);
      snprintf (buffer, 512, "mv %s %s", LOCAL_RESULT_FILE, NOCB_NORESUME_RESULT);
      systemRet = system (buffer);
      printf ("System : %s\n", buffer);

      title ("--- Closing AR.Drone FTP ---\n");
  
      TEST_RESULT (ftpClose (&droneFtp), "AR.Drone FTP (%s:%d) was not opened", "Closed AR.Drone FTP (%s:%d)", ARDRONE_IP, ARDRONE_PORT);








      title ("\n\n------ TESTS WITHOUT RESUME / WITH CALLBACK ------\n\n");
  
      title ("--- Connecting to AR.Drone FTP ---\n");
 
      for (runOperation = 1; runOperation;)
        {
          droneFtp = ftpConnect (ARDRONE_IP, ARDRONE_PORT, ARDRONE_USER, ARDRONE_PASSWORD, &ftp_result);
          TEST_RESULT (ftp_result, "Unable to connect to %s@%s:%d (PASS : %s)", "Connected to %s@%s:%d (PASS : %s)", ARDRONE_USER, ARDRONE_IP, ARDRONE_PORT, ARDRONE_PASSWORD);
        }
      
      title ("--- Sending origin file to the AR.Drone ---\n");

  
      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpPut (droneFtp, _1MB_ORIGIN_FILE, ARDRONE_FILE_BEFORE_RENAME, 0, ftpCallback);
          WAIT_COND (ftp_result);
          TEST_RESULT (globalStatus, "Unable to send file %s->%s", "Sent file %s->%s", _1MB_ORIGIN_FILE, ARDRONE_FILE_BEFORE_RENAME);
        }

      title ("--- Renaming file on the AR.Drone ---\n");

      ftp_result = ftpRename (droneFtp, ARDRONE_FILE_BEFORE_RENAME, ARDRONE_FILE_AFTER_RENAME);
      TEST_RESULT (ftp_result, "Impossible to rename %s->%s", "Renamed %s->%s", ARDRONE_FILE_BEFORE_RENAME, ARDRONE_FILE_AFTER_RENAME);

      title ("--- Getting AR.Drone FTP Listing ---\n");
  
      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpList (droneFtp, NULL, ftpCallback);
          WAIT_COND (ftp_result);
          TEST_RESULT (globalStatus, "Unable to list FTP [%1s]", "Listed FTP :\n%s", listBuffer);
          if (NULL != listBuffer) { vp_os_free (listBuffer); listBuffer = NULL; }
        }

      title ("--- Getting back result file ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpGet (droneFtp, ARDRONE_FILE_AFTER_RENAME, LOCAL_RESULT_FILE, 0, ftpCallback);
          WAIT_COND (ftp_result);
          TEST_RESULT (globalStatus, "Unable to get file %s->%s", "Got back file %s->%s", ARDRONE_FILE_AFTER_RENAME, LOCAL_RESULT_FILE);
        }

      title ("--- Checking file ---\n");
 
      vp_os_memset (buffer, 0x0, 512);
      snprintf (buffer, 512, "diff %s %s > /dev/null", LOCAL_RESULT_FILE, _1MB_ORIGIN_FILE);
      systemRet = system (buffer);
      printf ("System : %s\n", buffer);
      totalOp++;
      if (0 != systemRet)
        {
          RED; printf ("[%d : KO] : Files are different !\n", __LINE__); RAZ;
        }
      else
        {
          opOk++;
          GREEN; printf ("[%d : OK] : Files are the same\n", __LINE__); RAZ;
        }

      title ("--- Removing file on AR.Drone ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpRemove (droneFtp, ARDRONE_FILE_AFTER_RENAME);
          TEST_RESULT (ftp_result, "Unable to delete file %s", "Deleted file %s", ARDRONE_FILE_AFTER_RENAME);
        }
      
      title ("--- Local file backup ---\n");
  
      vp_os_memset (buffer, 0x0, 512);
      snprintf (buffer, 512, "mv %s %s", LOCAL_RESULT_FILE, CB_NORESUME_RESULT);
      systemRet = system (buffer);
      printf ("System : %s\n", buffer);

      title ("--- Closing AR.Drone FTP ---\n");
  
      TEST_RESULT (ftpClose (&droneFtp), "AR.Drone FTP (%s:%d) was not opened", "Closed AR.Drone FTP (%s:%d)", ARDRONE_IP, ARDRONE_PORT);






      title ("\n\n------ TESTS WITH RESUME / NO CALLBACK ------\n\n");

      title ("--- Preparing bad file (256K) locally ---\n");

      vp_os_memset (buffer, 0x0, 512);
      snprintf (buffer, 512, "cp %s %s", _256K_FAILED_FILE, LOCAL_RESULT_FILE);
      systemRet = system (buffer);
      printf ("System : %s\n", buffer);
  
      title ("--- Connecting to AR.Drone FTP ---\n");
  
      for (runOperation = 1; runOperation;)
        {
          droneFtp = ftpConnect (ARDRONE_IP, ARDRONE_PORT, ARDRONE_USER, ARDRONE_PASSWORD, &ftp_result);
          TEST_RESULT (ftp_result, "Unable to connect to %s@%s:%d (PASS : %s)", "Connected to %s@%s:%d (PASS : %s)", ARDRONE_USER, ARDRONE_IP, ARDRONE_PORT, ARDRONE_PASSWORD);
        }

      title ("--- Preparing bad file (512K) on AR.Drone FTP ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpPut (droneFtp, _512K_FAILED_FILE, ARDRONE_FILE_BEFORE_RENAME, 1, NULL);
          TEST_RESULT (ftp_result, "Unable to send partial file %s->%s", "Sent partial file %s->%s", _512K_FAILED_FILE, ARDRONE_FILE_BEFORE_RENAME);
        }
  
      title ("--- Sending origin file to the AR.Drone ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpPut (droneFtp, _1MB_ORIGIN_FILE, ARDRONE_FILE_BEFORE_RENAME, 1, NULL);
          TEST_RESULT (ftp_result, "Unable to send file %s->%s", "Sent file %s->%s", _1MB_ORIGIN_FILE, ARDRONE_FILE_BEFORE_RENAME);
        }

      title ("--- Renaming file on the AR.Drone ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpRename (droneFtp, ARDRONE_FILE_BEFORE_RENAME, ARDRONE_FILE_AFTER_RENAME);
          TEST_RESULT (ftp_result, "Impossible to rename %s->%s", "Renamed %s->%s", ARDRONE_FILE_BEFORE_RENAME, ARDRONE_FILE_AFTER_RENAME);
        }

      title ("--- Getting AR.Drone FTP Listing ---\n");
  
      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpList (droneFtp, &listBuffer, NULL);
          TEST_RESULT (ftp_result, "Unable to list FTP [%1s]", "Listed FTP :\n%s", listBuffer);
          if (NULL != listBuffer) { vp_os_free (listBuffer); listBuffer = NULL; }
        }

      title ("--- Getting back result file ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpGet (droneFtp, ARDRONE_FILE_AFTER_RENAME, LOCAL_RESULT_FILE, 1, NULL);
          TEST_RESULT (ftp_result, "Unable to get file %s->%s", "Got back file %s->%s", ARDRONE_FILE_AFTER_RENAME, LOCAL_RESULT_FILE);
        }

      title ("--- Checking file ---\n");
 
      vp_os_memset (buffer, 0x0, 512);
      snprintf (buffer, 512, "diff %s %s > /dev/null", LOCAL_RESULT_FILE, _1MB_MERGED_FILE);
      systemRet = system (buffer);
      printf ("System : %s\n", buffer);
      totalOp++;
      if (0 != systemRet)
        {
          RED; printf ("[%d : KO] : Files are different !\n", __LINE__); RAZ;
        }
      else
        {
          opOk++;
          GREEN; printf ("[%d : OK] : Files are the same\n", __LINE__); RAZ;
        }

      title ("--- Removing file on AR.Drone ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpRemove (droneFtp, ARDRONE_FILE_AFTER_RENAME);
          TEST_RESULT (ftp_result, "Unable to delete file %s", "Deleted file %s", ARDRONE_FILE_AFTER_RENAME);
        }

      title ("--- Local file backup ---\n");
  
      vp_os_memset (buffer, 0x0, 512);
      snprintf (buffer, 512, "mv %s %s", LOCAL_RESULT_FILE, NOCB_RESUME_RESULT);
      systemRet = system (buffer);
      printf ("System : %s\n", buffer);

      title ("--- Closing AR.Drone FTP ---\n");
  
      TEST_RESULT (ftpClose (&droneFtp), "AR.Drone FTP (%s:%d) was not opened", "Closed AR.Drone FTP (%s:%d)", ARDRONE_IP, ARDRONE_PORT);












      title ("\n\n------ TESTS WITH RESUME AND CALLBACK ------\n\n");

      title ("--- Preparing bad file (256K) locally ---\n");

      vp_os_memset (buffer, 0x0, 512);
      snprintf (buffer, 512, "cp %s %s", _256K_FAILED_FILE, LOCAL_RESULT_FILE);
      systemRet = system (buffer);
      printf ("System : %s\n", buffer);
  
      title ("--- Connecting to AR.Drone FTP ---\n");
  
      for (runOperation = 1; runOperation;)
        {
          droneFtp = ftpConnect (ARDRONE_IP, ARDRONE_PORT, ARDRONE_USER, ARDRONE_PASSWORD, &ftp_result);
          TEST_RESULT (ftp_result, "Unable to connect to %s@%s:%d (PASS : %s)", "Connected to %s@%s:%d (PASS : %s)", ARDRONE_USER, ARDRONE_IP, ARDRONE_PORT, ARDRONE_PASSWORD);
        }

      title ("--- Preparing bad file (512K) on AR.Drone FTP ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpPut (droneFtp, _512K_FAILED_FILE, ARDRONE_FILE_BEFORE_RENAME, 1, ftpCallback);
          WAIT_COND (ftp_result);
          TEST_RESULT (globalStatus, "Unable to send partial file %s->%s", "Sent partial file %s->%s", _512K_FAILED_FILE, ARDRONE_FILE_BEFORE_RENAME);
        }
  
      title ("--- Sending origin file to the AR.Drone ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpPut (droneFtp, _1MB_ORIGIN_FILE, ARDRONE_FILE_BEFORE_RENAME, 1, ftpCallback);
          WAIT_COND (ftp_result);
          TEST_RESULT (globalStatus, "Unable to send file %s->%s", "Sent file %s->%s", _1MB_ORIGIN_FILE, ARDRONE_FILE_BEFORE_RENAME);
        }

      title ("--- Renaming file on the AR.Drone ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpRename (droneFtp, ARDRONE_FILE_BEFORE_RENAME, ARDRONE_FILE_AFTER_RENAME);
          TEST_RESULT (ftp_result, "Impossible to rename %s->%s", "Renamed %s->%s", ARDRONE_FILE_BEFORE_RENAME, ARDRONE_FILE_AFTER_RENAME);
        }

      title ("--- Getting AR.Drone FTP Listing ---\n");
  
      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpList (droneFtp, NULL, ftpCallback);
          WAIT_COND (ftp_result);
          TEST_RESULT (globalStatus, "Unable to list FTP [%1s]", "Listed FTP :\n%s", listBuffer);
          if (NULL != listBuffer) { vp_os_free (listBuffer); listBuffer = NULL; }
        }

      title ("--- Getting back result file ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpGet (droneFtp, ARDRONE_FILE_AFTER_RENAME, LOCAL_RESULT_FILE, 1, ftpCallback);
          WAIT_COND (ftp_result);
          TEST_RESULT (globalStatus, "Unable to get file %s->%s", "Got back file %s->%s", ARDRONE_FILE_AFTER_RENAME, LOCAL_RESULT_FILE);
        }

      title ("--- Checking file ---\n");
 
      vp_os_memset (buffer, 0x0, 512);
      snprintf (buffer, 512, "diff %s %s > /dev/null", LOCAL_RESULT_FILE, _1MB_MERGED_FILE);
      systemRet = system (buffer);
      printf ("System : %s\n", buffer);
      totalOp++;
      if (0 != systemRet)
        {
          RED; printf ("[%d : KO] : Files are different !\n", __LINE__); RAZ;
        }
      else
        {
          opOk++;
          GREEN; printf ("[%d : OK] : Files are the same\n", __LINE__); RAZ;
        }

      title ("--- Removing file on AR.Drone ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpRemove (droneFtp, ARDRONE_FILE_AFTER_RENAME);
          TEST_RESULT (ftp_result, "Unable to delete file %s", "Deleted file %s", ARDRONE_FILE_AFTER_RENAME);
        }
  
      title ("--- Local file backup ---\n");
  
      vp_os_memset (buffer, 0x0, 512);
      snprintf (buffer, 512, "mv %s %s", LOCAL_RESULT_FILE, CB_RESUME_RESULT);
      systemRet = system (buffer);
      printf ("System : %s\n", buffer);

      title ("--- Closing AR.Drone FTP ---\n");
  
      TEST_RESULT (ftpClose (&droneFtp), "AR.Drone FTP (%s:%d) was not opened", "Closed AR.Drone FTP (%s:%d)", ARDRONE_IP, ARDRONE_PORT);

     






      title ("\n\n------ ABORTION TESTS ------\n\n");

      
      title ("--- Connecting to AR.Drone FTP ---\n");
  
      for (runOperation = 1; runOperation;)
        {
          droneFtp = ftpConnect (ARDRONE_IP, ARDRONE_PORT, ARDRONE_USER, ARDRONE_PASSWORD, &ftp_result);
          TEST_RESULT (ftp_result, "Unable to connect to %s@%s:%d (PASS : %s)", "Connected to %s@%s:%d (PASS : %s)", ARDRONE_USER, ARDRONE_IP, ARDRONE_PORT, ARDRONE_PASSWORD);
        }

      title ("--- Sending origin file to the AR.Drone ... aborted ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpPut (droneFtp, _1MB_ORIGIN_FILE, ARDRONE_FILE_BEFORE_RENAME, 1, ftpCallback);
          usleep (500000);
          TEST_ABORTION (ftpAbort (droneFtp));
          WAIT_COND (ftp_result);
          TEST_RESULT ((FTP_ABORT == globalStatus) ? FTP_SUCCESS : FTP_FAIL, "Impossible to abort %s->%s transfert", "Aborted %s->%s transfert", _1MB_ORIGIN_FILE, ARDRONE_FILE_BEFORE_RENAME);
        }

      title ("--- Sending origin file to the AR.Drone ... complete previous operation ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpPut (droneFtp, _1MB_ORIGIN_FILE, ARDRONE_FILE_BEFORE_RENAME, 1, ftpCallback);
          WAIT_COND (ftp_result);
          TEST_RESULT (globalStatus, "Unable to send file %s->%s", "Sent file %s->%s", _1MB_ORIGIN_FILE, ARDRONE_FILE_BEFORE_RENAME);
        }

      title ("--- Getting AR.Drone FTP Listing ... aborted ---\n");
  
      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpList (droneFtp, NULL, ftpCallback);
          usleep (1000);
          TEST_ABORTION (ftpAbort (droneFtp));
          WAIT_COND (ftp_result);
          TEST_RESULT ((FTP_ABORT == globalStatus) ? FTP_SUCCESS : FTP_FAIL, "Impossible to abort %s", "Aborted %s", "listing");
          if (NULL != listBuffer) { vp_os_free (listBuffer); listBuffer = NULL; }
        }

      title ("--- Getting AR.Drone FTP Listing ... ok ---\n");
  
      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpList (droneFtp, NULL, ftpCallback);
          WAIT_COND (ftp_result);
          TEST_RESULT (globalStatus, "Unable to list FTP [%1s]", "Listed FTP :\n%s", listBuffer);
          if (NULL != listBuffer) { vp_os_free (listBuffer); listBuffer = NULL; }
        }

      title ("--- Getting back origin file as result ... aborted ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpGet (droneFtp, ARDRONE_FILE_BEFORE_RENAME, LOCAL_RESULT_FILE, 1, ftpCallback);
          usleep (500000);
          TEST_ABORTION (ftpAbort (droneFtp));
          WAIT_COND (ftp_result);
          TEST_RESULT ((FTP_ABORT == globalStatus) ? FTP_SUCCESS : FTP_FAIL, "Impossible to abort %s->%s transfert", "Aborted %s->%s transfert", ARDRONE_FILE_BEFORE_RENAME, LOCAL_RESULT_FILE);
        }

      title ("--- Getting back origin file as result ... complete previous operation ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpGet (droneFtp, ARDRONE_FILE_BEFORE_RENAME, LOCAL_RESULT_FILE, 1, ftpCallback);
          WAIT_COND (ftp_result);
          TEST_RESULT (globalStatus, "Unable to get file %s->%s", "Got back file %s->%s", ARDRONE_FILE_BEFORE_RENAME, LOCAL_RESULT_FILE);
        }

      title ("--- Checking file ---\n");
 
      vp_os_memset (buffer, 0x0, 512);
      snprintf (buffer, 512, "diff %s %s > /dev/null", LOCAL_RESULT_FILE, _1MB_ORIGIN_FILE);
      systemRet = system (buffer);
      printf ("System : %s\n", buffer);
      totalOp++;
      if (0 != systemRet)
        {
          RED; printf ("[%d : KO] : Files are different !\n", __LINE__); RAZ;
        }
      else
        {
          opOk++;
          GREEN; printf ("[%d : OK] : Files are the same\n", __LINE__); RAZ;
        }

      title ("--- Removing file on AR.Drone ---\n");

      for (runOperation = 1; runOperation;)
        {
          ftp_result = ftpRemove (droneFtp, ARDRONE_FILE_BEFORE_RENAME);
          TEST_RESULT (ftp_result, "Unable to delete file %s", "Deleted file %s", ARDRONE_FILE_BEFORE_RENAME);
        }
  
      title ("--- Closing AR.Drone FTP ---\n");
  
      TEST_RESULT (ftpClose (&droneFtp), "AR.Drone FTP (%s:%d) was not opened", "Closed AR.Drone FTP (%s:%d)", ARDRONE_IP, ARDRONE_PORT);







      title ("\n\n------ RESULT ------\n\n");

      COMPUTE_PERCENT_OK;
    }

  sem_destroy (&synchroSem);
  
  exit (0);
  return 0;
}
