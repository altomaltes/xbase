#include "d4all.h"
#ifndef S4STAND_ALONE
#ifndef S4OFF_THREAD
void code4freeEvent(CODE4 *c4, EVENT4 *event)
{
/* ERRORS

   No errors are possible in this function

   NOTES

   Used to make an EVENT4 available for use later
*/

   #ifdef E4PARM_LOW
      if ( c4 == NULL )
      {
         error4(NULL, e4parmNull, E96932 ) ;
         return ;
      }
   #endif

   l4add(&c4->eventsAvail, event ) ;
}


EVENT4 *code4getEvent(CODE4 *c4 )
{
/* ERRORS

   In case of failure, call error4 and return NULL

   NOTES

   This function is used to obtain an EVENT4 object

*/
   EVENT4 *event ;

   #ifdef E4PARM_LOW
      if (c4 == NULL )
      {
         error4(NULL, e4parmNull, E96933 ) ;
         return NULL ;
      }
   #endif

   event = (EVENT4 *)l4pop(&c4->eventsAvail ) ;
   if (event != NULL )
      return event ;
   event = (EVENT4 *)mem4createAlloc(c4, &c4->eventMemory, MEMORY4START_EVENT, sizeof(EVENT4), MEMORY4EXPAND_EVENT, 0 ) ;
   event4init(event) ;
   return event ;
}

void code4clearEvents(CODE4 *c4 )
{
   EVENT4 *event ;

   for(;;)
   {
      event = (EVENT4 *)l4pop(&c4->eventsAvail ) ;
      if (event == NULL )
         break ;
      event4initUndo(event) ;
      mem4free(c4->eventMemory, event) ;
   }
}

int event4init(EVENT4 *event)
{
/* PARAMATERS

   event is the event to initialize

   NOTES

   This function does whatever intialization is required to create a valid
     event.

   In __WIN32, this function calls the WIN32 API function CreateEvent
     to create a valid event handle, and stores that handle in the
     EVENT4 structure.

   The event should be created as follows:
     no security paramaters
     manual reset, which allows us to wait again in the GetOverlappedResult
       function for the same event.  ** Note that this means that the
       event must be to nonsignalled everytime before it is used in a
       pending operation.
     initial set is non-signalled because no event has occurred.
     event Name is not applicable (i.e. set to NULL)

   RETURNS

   r4success
   < 0 error
*/

   #ifdef E4PARM_LOW
      if (event == NULL )
         return e4parmNull ;
   #endif

   event->handle = CreateEvent( NULL, TRUE, FALSE, NULL ) ;
   if ( event->handle == NULL )
      return e4event ;
   return r4success ;
}

int event4initUndo(EVENT4 *event)
{
/* PARAMATERS

   event is the event to uninitialize

   NOTES

   This function does whatever unintialization is required to destroy a
     valid event

   In __WIN32, this function calls the WIN32 API function CloseHandle on
     EVENT4.handle.

   RETURNS

   r4success
   < 0 error
*/

   int rc ;

   #ifdef E4PARM_LOW
      if (event == NULL )
         return e4parmNull ;
   #endif

   rc = CloseHandle(event->handle) ;
   if (!rc)
      return e4event ;
   return r4success  ;
}

int event4reset(EVENT4 *event)
{
/* NOTES

   This function resets the event to non-signalled
*/
   int rc ;

   #ifdef E4PARM_LOW
      if (event == NULL )
         return e4parmNull ;
   #endif

   rc = ResetEvent(event->handle) ;
   if (!rc)
      return e4event ;
   return r4success ;
}


/* NOTES

   This function is used to set an event, which indirectly causes any
     thread waiting on the EVENT4 to be activated and able to respond to
     this event.

   In __WIN32, this function calls the WIN32 API function setEvent
     on the event handle to set the event.
*/
/*
int event4set(EVENT4 *event)
{
   int rc ;

   #ifdef E4PARM_LOW
      if (event == NULL )
         return e4parmNull ;
   #endif

   rc = SetEvent(event->handle) ;
   if (!rc)
      return e4event ;
   return r4success ;
}
*/

int event4wait(EVENT4 *event, int doWait )
{
/* PARAMATERS

   event is the event to wait on
   doWait, if true, results in this routine not returning until the event
     has been setd by another thread.  If false, this function returns
     immediately with the results as to whether or not an event was
     setd.

   RETURNS

   > 0 means the event was setd
   0 means the event was not setd and doWait was false

   NOTES

   This function is used to suspend a thread until the given EVENT4 has
     been seted

   in __WIN32, this function calls the WIN32 API function
     WaitForSingleObject() on EVENT4.handle
*/

   int rc ;

   #ifdef E4PARM_LOW
      if (event == NULL )
         return e4parmNull ;
   #endif

   if (doWait)
      rc = WaitForSingleObject(event->handle, INFINITE ) ;
   else
      rc = WaitForSingleObject(event->handle, 0) ;

   if (rc == WAIT_TIMEOUT )
      return 0 ;
   if (rc == WAIT_FAILED )
       return -1 ;
   return 1 ;
}
#endif /*!S4OFF_THREAD */
#endif /*!S4STAND_ALONE */
