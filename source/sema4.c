/*If E4PARMLOW is defined, the functions in this module will ensure that
  their paramaters are not invalid.  This error will be generated even
  for those functions which otherwise say that no error is generated.*/

/* the SEMAPHORE4 module contains some operating system specific coding
   for the actual implementation of a semaphore */

#include "d4all.h"
#ifndef S4OFF_THREAD
int semaphore4init(SEMAPHORE4 *semaphore )
{
/* PARAMATERS

   semaphore is the semaphore to initialize

   ERRORS

   In case of error, call error4() with a NULL CODE4 (since one is not
     available), and an error code of e4result.  This severe error
     indicates an unrecoverable failure, and indicates a major failure.

   NOTES

   This function does whatever intialization is required to create a valid
     semaphore.

   In __WIN32, this function calls the WIN32 API function CreateSemaphore
     to create a valid semaphore handle, and stores that handle in the
     SEMAPHORE4 structure.

   RETURNS

   r4success
   < 0 error
*/

   E4PARMLOW( semaphore, E96934 ) ;

   semaphore->handle = CreateSemaphore(NULL, 0, INT_MAX, NULL ) ;
   if (semaphore->handle == NULL )
      return (error4(NULL, e4semaphore, E96934 )) ;
   return r4success ;
}

int semaphore4initUndo(SEMAPHORE4 *semaphore )
{
/* PARAMATERS

   semaphore is the semaphore to uninitialize

   ERRORS

   ignore any errors.

   NOTES

   This function does whatever unintialization is required to create a valid
     semaphore.

   In __WIN32, this function calls the WIN32 API function CloseHandle on
     SEMAPHORE4.handle

   RETURNS

   r4success
   < 0 error
*/

   int rc ;

   E4PARMLOW( semaphore, E96935 ) ;

   rc = CloseHandle(semaphore->handle ) ;
   if (rc ==FALSE)
      return e4semaphore ;
   semaphore->handle = 0 ;
   return r4success ;
}

void semaphore4release(SEMAPHORE4 *semaphore )
{
/*
   ERRORS

   In case of error, call error4() with a NULL CODE4 (since one is not
     available), and an error code of e4result.  This severe error
     indicates an unrecoverable failure, and indicates a major failure.

   NOTES

   This function is used to release a semaphore, which indirectly causes any
     thread waiting on the SEMAPHORE4 to be activated and able to respond to
     this semaphore.

   In __WIN32, this function calls the WIN32 API function ReleaseSemaphore
     on the semaphore handle to release the semaphore.
*/

   int rc ;

   E4PARMLOW( semaphore, E96936 )  ;

   rc = ReleaseSemaphore(semaphore->handle, 1, NULL ) ;
   if (rc == FALSE )
      error4(NULL, e4semaphore, E96936 ) ;
}

int semaphore4wait (SEMAPHORE4 *semaphore, int waitSecs )
{
/* PARAMATERS

   semaphore is the semaphore to wait on
   waitSecs is the number of seconds to wait on the semaphore.  A value of
     zero means no wait, a value of WAIT4EVER means an infinite wait.

   ERRORS

   In case of E4PARMLOW error, call error4() with a NULL CODE4 (since one is
     not available), and an error code of e4result.  This severe error
     indicates an unrecoverable failure, and indicates a major failure.

   If the Wait fails due to error, return FALSE.  This is because worker
     threads expect to just fail out when the semphore is deleted (see
     server4worker()).

   RETURNS

   > 0 means the semaphore was released
   0 means the semaphore was not released and doWait was not infinite

   NOTES

   This function is used to suspend a thread until the given SEMAPHORE4 has
     been releaseed or the timeout has elapsed

   in __WIN32, this function calls the WIN32 API function
     WaitForSingleObject() on SEMAPHORE4.handle
*/

   int rc ;

   E4PARMLOW( semaphore, E96937 )  ;

   if (waitSecs<0)
      rc = WaitForSingleObject(semaphore->handle, INFINITE) ;
   else
      rc = WaitForSingleObject(semaphore->handle, waitSecs*1000) ;

   if ( rc == WAIT_OBJECT_0 )
      return 1 ;
   else
      return 0 ;
}
#endif /*!S4OFF_THREAD */
