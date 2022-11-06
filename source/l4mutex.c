/*If E4PARM_LOW is defined, the functions in this module will ensure that
  their paramaters are not invalid.  This error will be generated even
  for those functions which otherwise say that no error is generated. */

#include "d4all.h"

#ifndef S4STAND_ALONE
#ifndef S4OFF_THREAD
void list4mutexAdd(LIST4MUTEX *listMutex, void *item )
{
/* ERRORS

   No errors are possible in this module
*/

   #ifdef E4PARM_LOW
      if ( (listMutex == NULL) || (item == NULL ) )
      {
         error4(NULL, e4parm_null, E96926 ) ;
         return ;
      }
   #endif

   list4mutexWait(listMutex) ;
   l4add(&listMutex->list, item ) ;
   list4mutexRelease(listMutex) ;
}

void list4mutexInit( LIST4MUTEX *listMutex )  /* initialize a mutex object */
{
/* ERRORS

   In case of error, call error4() with a NULL CODE4 (since one is not
     available), and an error code of e4result.  This severe error
     indicates an unrecoverable failure, and indicates a major failure.
*/

   #ifdef E4PARM_LOW
      if ( listMutex == NULL)
      {
         error4(NULL, e4parm_null, E96927 ) ;
         return ;
      }
   #endif

   memset(listMutex, 0, sizeof(LIST4MUTEX) ) ;
   listMutex->mutex = CreateMutex(NULL, FALSE, NULL ) ;
   listMutex->isValid = 1 ;
}

void list4mutexInitUndo(LIST4MUTEX *listMutex )  /* uninit a mutex object */
{
/* ERRORS

   ignore any errors.
*/

   CloseHandle(listMutex->mutex) ;
   memset(listMutex, 0, sizeof(LIST4MUTEX) ) ;
}

PUBLIC void list4mutexRelease( LIST4MUTEX *listMutex)
{
/*
   ERRORS

   In case of error, call error4() with a NULL CODE4 (since one is not
     available), and an error code of e4result.  This severe error
     indicates an unrecoverable failure, and indicates a major failure.

   used to just release the mutex associcated with the list
   this function is called if list4mutexWait() was called to gain control
*/

   int rc ;

   #ifdef E4PARM_LOW
      if ( listMutex == NULL)
      {
         error4(NULL, e4parm_null, E96928 ) ;
         return ;
      }
   #endif

   rc = ReleaseMutex(listMutex->mutex) ;
   if (!rc)
      error4(NULL, e4result, E96928 ) ;
}

void *list4mutexRemove( LIST4MUTEX *listMutex )
{
/* ERRORS

   No errors are possible in this module
*/

   void *item;

   #ifdef E4PARM_LOW
      if ( listMutex == NULL)
      {
         error4(NULL, e4result, E96929 ) ;
         return NULL ;
      }
   #endif

   list4mutexWait(listMutex) ;

   item = l4first(&listMutex->list) ;
   l4remove(&listMutex->list, item ) ;

   list4mutexRelease(listMutex) ;
   return item ;
}


void list4mutexRemoveLink( LIST4MUTEX *listMutex, LINK4 *link )
{
/* ERRORS

   No errors are possible in this module
*/

   #ifdef E4PARM_LOW
      if ( listMutex == NULL)
      {
         error4(NULL, e4result, E96930 ) ;
         return ;
      }
   #endif

   list4mutexWait( listMutex ) ;
   l4remove( &listMutex->list, link ) ;
   list4mutexRelease( listMutex ) ;
}

PUBLIC void list4mutexWait(LIST4MUTEX *listMutex )
{

/* ERRORS

   In case of error, call error4() with a NULL CODE4 (since one is not
     available), and an error code of e4result.  This severe error
     indicates an unrecoverable failure, and indicates a major failure.

   used to just wait on the mutex associcated with the list
   after this call, list4mutexRelease() must be called to release control
   of the mutex so other threads may access the list
*/

   int rc ;

   #ifdef E4PARM_LOW
      if ( listMutex == NULL)
      {
         error4(NULL, e4result, E96931 ) ;
         return ;
      }
   #endif

   if ( listMutex->isValid == 0 )
      error4(NULL, e4parm, E96931 ) ;

   rc = WaitForSingleObject(listMutex->mutex, INFINITE ) ;
   if (rc == WAIT_FAILED )
      error4(NULL, e4result, E96931 ) ;
}

#endif /*!S4OFF_THREAD*/
#endif /*!S4STAND_ALONE */
