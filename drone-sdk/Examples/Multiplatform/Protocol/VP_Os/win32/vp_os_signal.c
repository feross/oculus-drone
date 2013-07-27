/**
 * @file signal.c
 * @author aurelien.morelle@parrot.fr
 * @date 2006/12/15
 */

#include "VP_Os/vp_os_signal.h"


void
vp_os_mutex_init(vp_os_mutex_t *mutex)
{
  InitializeCriticalSection((CRITICAL_SECTION *)mutex);
}


void
vp_os_mutex_destroy(vp_os_mutex_t *mutex)
{
  DeleteCriticalSection((CRITICAL_SECTION *)mutex);
}


void
vp_os_mutex_lock(vp_os_mutex_t *mutex)
{
  EnterCriticalSection((CRITICAL_SECTION *)mutex);
}


void
vp_os_mutex_unlock(vp_os_mutex_t *mutex)
{
  LeaveCriticalSection((CRITICAL_SECTION *)mutex);
}


void
vp_os_cond_init(vp_os_cond_t *cond, vp_os_mutex_t *mutex)
{
/* // InitializeConditionVariable only work under VISTA
  InitializeConditionVariable(&cond->cond);
  cond->mutex = mutex;
*/
}


void
vp_os_cond_destroy(vp_os_cond_t *cond)
{
}


void
vp_os_cond_wait(vp_os_cond_t *cond)
{
  WaitForSingleObject(cond->LockSemaphore,INFINITE); // TODO: to test
/* // SleepConditionVariableCS only work under VISTA
  SleepConditionVariableCS(&cond->cond, (CRITICAL_SECTION *)cond->mutex, INFINITE);
*/
}

C_RESULT
vp_os_cond_timed_wait(vp_os_cond_t *cond, uint32_t ms)
{
  return WaitForSingleObject(cond->LockSemaphore, ms) == WAIT_TIMEOUT ? FAIL : SUCCESS; // TODO: to test
}

void
vp_os_cond_signal(vp_os_cond_t *cond)
{
/* // WakeConditionVariable only work under VISTA
  WakeConditionVariable(&cond->cond);
*/
}

