/* d4lock.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifdef S4CLIENT
int d4localLockSet( DATA4 *data, const long rec )
{
   LOCK4LINK *lock, *lockOn ;
   CODE4 *c4 ;
   DATA4FILE *dfile ;
   SINGLE4DISTANT singleDistant ;
   #ifdef E4MISC
      long recSave ;
   #endif

   #ifdef E4PARM_LOW
      if ( data == 0 || rec < 1L )
         return error4( 0, e4parm_null, E92723 ) ;
   #endif

   if ( d4lockTest( data, rec ) == 1 )
      return 0 ;

   c4 = data->codeBase ;
   dfile = data->dataFile ;

   #ifdef E4MISC
      /* verify the order of the list */
      lock = (LOCK4LINK *)single4initIterate( &dfile->lockedRecords ) ;
      if ( lock != 0 )
         for ( ;; )
         {
            recSave = lock->recNo ;
            lock = (LOCK4LINK *)single4next( &lock->link ) ;
            if ( lock == 0 )
               break ;
            if ( lock->recNo <= recSave )
               return error4( c4, e4info, E91102 ) ;
         }
   #endif

   for ( lock = (LOCK4LINK *)single4initIterate( &dfile->lockedRecords ) ;; )
   {
      if ( lock == 0 )
         break ;
      if ( lock->data == data && lock->recNo == rec )
         return 0 ;
      lock = (LOCK4LINK *)single4next( &lock->link ) ;
   }

   if ( c4->lockLinkMemory == 0 )
   {
      c4->lockLinkMemory = mem4create( c4, c4->memStartLock, sizeof(LOCK4LINK), c4->memExpandLock, 0 ) ;
      if ( c4->lockLinkMemory == 0 )
         return e4memory ;
   }
   lock = (LOCK4LINK *)mem4alloc( c4->lockLinkMemory ) ;
   if ( lock == 0 )
      return e4memory ;
   lock->data = data ;
   lock->recNo = rec ;

   single4distantInitIterate( &singleDistant, &dfile->lockedRecords ) ;
   for ( ;; )
   {
      lockOn = (LOCK4LINK *)single4distantToItem( &singleDistant ) ;
      if ( lockOn == 0 || lockOn->recNo > rec )
      {
         single4add( single4distantToSingle( &singleDistant ), &lock->link ) ;
         break ;
      }
      single4distantNext( &singleDistant ) ;
   }

   return 0 ;
}
#endif

#ifdef S4CB51
int S4FUNCTION d4lock_group( DATA4 *data, const long *recs, const int n_recs )
{
   CODE4 *c4 ;
   #ifndef S4SINGLE
      int i, rc ;
   #endif

   #ifdef E4PARM_LOW
      if ( data == 0 || recs == 0 || n_recs < 0 )
         return error4( 0, e4parm_null, E92724 ) ;
   #endif

   c4 = data->codeBase ;

   #ifndef S4SINGLE
      if ( error4code( c4 ) < 0 )
         return e4codeBase ;

      if( d4lockTestFile( data ) )
         return 0 ;

         switch( code4unlockAuto( c4 ) )
         {
            case LOCK4ALL :
               code4lockClear( c4 ) ;
               rc = error4code( c4 ) ;
               break ;
            case LOCK4DATA :
               rc = d4unlockLow( data, data4clientId( data ), 0 ) ;
               break ;
         }
         if( rc < 0 )
            return error4stack( c4, rc, E92724 ) ;

      for ( i = 0 ; i < n_recs ; i++ )
      {
         rc = d4lockAdd( data, recs[i] ) ;
         if ( rc != 0 )
         {
            code4lockClear( c4 ) ;
            return rc ;
         }
      }

      return code4lock( c4 ) ;
   #else
      return 0 ;
   #endif
}
#endif  /* S4CB51 */

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4lock( DATA4 *data, const long rec )
{
   #ifndef S4SINGLE
      int rc ;
      CODE4 *c4 ;

      #ifdef S4VBASIC
         if ( c4parm_check( data, 2, E92701 ) )
            return -1 ;
      #endif

      #ifdef E4PARM_HIGH
         if ( data == 0 || rec < 1L )
            return error4( 0, e4parm, E92701 ) ;
      #endif

      c4 = data->codeBase ;

      if ( error4code( c4 ) < 0 )
         return e4codeBase ;

      if ( d4lockTest( data, rec ) > 0 )  /* if record or file already locked */
         return 0 ;

      #ifdef S4CLIENT
         /* verify that this app doesn't have it locked elsewhere */
         if ( data->dataFile->lockTest != 0 && code4unlockAuto( c4 ) != LOCK4ALL )
         {
            if ( c4->lockAttempts == WAIT4EVER )
               return error4( c4, e4lock, E81523 ) ;
            else
               return r4locked ;
         }
      #else
         rc = 0 ;
         switch( code4unlockAuto( c4 ) )
         {
            case LOCK4ALL :
               rc = code4unlockDo( tran4dataList( data->trans ) ) ;
               break ;

            case LOCK4DATA :
               #ifndef S4INDEX_OFF
                  rc = dfile4unlockIndex( data->dataFile, data4serverId( data ) );
                  if( rc < 0 )
                     break ;
               #endif
               rc = d4unlockRecords( data ) ;
               /* next 2 lines added Feb 08/96 AS --> t4mul, not unlocking append bytes */
               if ( rc == 0 )
                  rc = d4unlockAppend( data ) ;
               break ;
            default:
               break ;
         }
         if( rc < 0 )
            return error4stack( c4, rc, E92701 ) ;
      #endif

      rc = dfile4lock( data->dataFile, data4clientId( data ), data4serverId( data ), rec ) ;
      #ifdef S4CLIENT
         if ( rc == 0 )
            rc = d4localLockSet( data, rec ) ;
      #endif

      return rc ;
   #else
      return 0 ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4lockAddAppend( DATA4 *d4 )
{
   #ifndef S4SINGLE
      LOCK4 *lock ;
      CODE4 *c4 ;
      int rc ;

      #ifdef E4PARM_HIGH
         if ( d4 == 0 )
            return error4( 0, e4parm, E92719 ) ;
      #endif

      if ( ( rc = d4verify( d4, 1 ) ) < 0 )
         return rc ;

      c4 = d4->codeBase ;

      if ( c4->lockMemory == 0 )
      {
         c4->lockMemory = mem4create( c4, c4->memStartLock, sizeof(LOCK4), c4->memExpandLock, 0 ) ;
         if ( c4->lockMemory == 0 )
            return 0 ;
      }

      lock = (LOCK4 *)mem4alloc( c4->lockMemory ) ;
      if ( lock == 0 )
         return error4stack( c4, e4memory, E92719 ) ;

      #ifdef S4CLIENT
         lock->data = d4 ;
      #else
         lock->data = d4->dataFile ;
      #endif
      lock->id.type = LOCK4APPEND ;
      lock->id.serverId = data4serverId( d4 ) ;
      lock->id.clientId = data4clientId( d4 ) ;
      single4add( &d4->trans->locks, &lock->link ) ;
   #endif

   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4lockAddAll( DATA4 *d4 )
{
   #ifndef S4SINGLE
      LOCK4 *lock ;
      CODE4 *c4 ;
      int rc ;

      #ifdef E4PARM_HIGH
         if ( d4 == 0 )
            return error4( 0, e4parm, E92725 ) ;
      #endif

      if ( ( rc = d4verify( d4, 1 ) ) < 0 )
         return rc ;

      c4 = d4->codeBase ;

      if ( c4->lockMemory == 0 )
      {
         c4->lockMemory = mem4create( c4, c4->memStartLock, sizeof(LOCK4), c4->memExpandLock, 0 ) ;
         if ( c4->lockMemory == 0 )
            return 0 ;
      }

      lock = (LOCK4 *)mem4alloc( c4->lockMemory ) ;
      if ( lock == 0 )
         return error4stack( c4, e4memory, E92725 ) ;

      #ifdef S4CLIENT
         lock->data = d4 ;
      #else
         lock->data = d4->dataFile ;
      #endif
      lock->id.type = LOCK4ALL ;
      lock->id.serverId = data4serverId( d4 ) ;
      lock->id.clientId = data4clientId( d4 ) ;
      single4add( &d4->trans->locks, &lock->link ) ;
   #endif

   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4lockAddFile( DATA4 *d4 )
{
   #ifndef S4SINGLE
      LOCK4 *lock ;
      CODE4 *c4 ;
      int rc ;

      #ifdef E4PARM_HIGH
         if ( d4 == 0 )
            return error4( 0, e4parm, E92720 ) ;
      #endif

      if ( ( rc = d4verify( d4, 1 ) ) < 0 )
         return rc ;

      c4 = d4->codeBase ;

      if ( c4->lockMemory == 0 )
      {
         c4->lockMemory = mem4create( c4, c4->memStartLock, sizeof(LOCK4), c4->memExpandLock, 0 ) ;
         if ( c4->lockMemory == 0 )
            return 0 ;
      }

      lock = (LOCK4 *)mem4alloc( c4->lockMemory ) ;
      if ( lock == 0 )
         return error4stack( c4, e4memory, E92720 ) ;

      #ifdef S4CLIENT
         lock->data = d4 ;
      #else
         lock->data = d4->dataFile ;
      #endif
      lock->id.type = LOCK4FILE ;
      lock->id.serverId = data4serverId( d4 ) ;
      lock->id.clientId = data4clientId( d4 ) ;
      single4add( &d4->trans->locks, &lock->link ) ;
   #endif

   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4lockAdd( DATA4 *d4, long rec )
{
   #ifndef S4SINGLE
      LOCK4 *lock ;
      CODE4 *c4 ;
      int rc ;

      #ifdef E4PARM_HIGH
         if ( d4 == 0 || rec < 1L )
            return error4( 0, e4parm, E92721 ) ;
      #endif

      if ( ( rc = d4verify( d4, 1 ) ) < 0 )
         return rc ;

      c4 = d4->codeBase ;

      if ( c4->lockMemory == 0 )
      {
         c4->lockMemory = mem4create( c4, c4->memStartLock, sizeof(LOCK4), c4->memExpandLock, 0 ) ;
         if ( c4->lockMemory == 0 )
            return error4stack( c4, e4memory, E92721 ) ;
      }

      lock = (LOCK4 *)mem4alloc( c4->lockMemory ) ;
      if ( lock == 0 )
         return error4stack( c4, e4memory, E92721 ) ;

      #ifdef S4CLIENT
         lock->data = d4 ;
      #else
         lock->data = d4->dataFile ;
      #endif
      lock->id.type = LOCK4RECORD ;
      lock->id.recNum = rec ;
      lock->id.serverId = data4serverId( d4 ) ;
      lock->id.clientId = data4clientId( d4 ) ;

      single4add( &d4->trans->locks, &lock->link ) ;
   #endif

   return 0 ;
}

#ifdef S4CLIENT
int S4FUNCTION d4lockAll( DATA4 *data )
{
   int rc ;

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm, E92702 ) ;
   #endif

   if ( error4code( data->codeBase ) < 0 )
      return e4codeBase ;

   if ( d4lockTestFile( data ) == 0 )
   {
      rc = dfile4lockAll( data->dataFile, data4clientId( data ), data4serverId( data ) ) ;
      if ( rc == 0 )
         data->dataFile->fileLock = data ;
      return rc ;
   }

   return 0 ;
}
#else
/* locks database, memo, and index files */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4lockAll( DATA4 *data )
{
   #ifndef S4SINGLE
      int rc, rc2 ;

      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm, E92702 ) ;
      #endif

      if ( error4code( data->codeBase ) < 0 )
         return e4codeBase ;

      /* d4lockFile will perform an auto unlock if required */
      rc = d4lockFile( data ) ;

      #ifndef S4CLIPPER
         #ifndef S4MEMO_OFF
            if ( !rc )
               if ( data->dataFile->nFieldsMemo > 0 )
                  rc = dfile4lockMemo( data->dataFile ) ;
         #endif
      #endif

      #ifndef S4INDEX_OFF
         if ( !rc )
            rc = d4lockIndex( data ) ;
      #endif

      if( rc )
      {
         switch( code4unlockAuto( data->codeBase ) )
         {
            case LOCK4ALL :
               rc2 = code4unlockDo( tran4dataList( data->trans ) ) ;
               break ;
            case LOCK4DATA :
               rc2 = d4unlockLow( data, data4clientId( data ), 0 ) ;
               break ;
            default:
               rc2 = 0 ;
               break ;
         }
         if( rc2 < 0 )
            return rc2 ;
      }

      return rc ;
   #else
      return 0 ;
   #endif
}
#endif

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4lockAppend( DATA4 *data )
{
   #ifdef S4SINGLE
      return 0 ;
   #else
      CODE4 *c4 ;
      #ifdef S4CLIENT
         int rc ;
      #endif

      #ifdef S4VBASIC
         if ( c4parm_check( data, 2, E92708 ) )
            return -1 ;
      #endif

      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm, E92708 ) ;
      #endif

      c4 = data->codeBase ;
      if ( error4code( c4 ) < 0 )
         return e4codeBase ;

      if ( d4lockTestAppend( data ) == 0 )
      {
         #ifdef S4CLIENT
            rc = dfile4lockAppend( data->dataFile, data4clientId( data ), data4serverId( data ) ) ;
            if ( rc == 0 )
               data->dataFile->appendLock = data ;
            #ifdef E4LOCK
               return request4lockTest( d4, LOCK4APPEND, 0L, rc ) ;
            #else
               return rc ;
            #endif
         #else
            /* in append case, unlock records only with unlock auto in order to avoid memo reset */
            /* changed 06/16 because is failing on lock due to not unlocking */
            switch( code4unlockAuto( c4 ) )
            {
               case LOCK4ALL :
                  if ( code4unlockDo( tran4dataList( data->trans ) ) != 0 )
                     return error4stack( c4, e4unlock, E92709 ) ;
                  break ;
               case LOCK4DATA :
                  #ifndef S4INDEX_OFF
                     if ( dfile4unlockIndex( data->dataFile, data4serverId( data ) )!= 0 )
                        return error4stack( c4, e4unlock, E92709 ) ;
                  #endif
                  if ( d4unlockRecords( data ) != 0 )
                     return error4stack( c4, e4unlock, E92709 ) ;
                  break;
               default:
                  break ;
            }
            return dfile4lockAppend( data->dataFile, data4clientId( data ), data4serverId( data ) ) ;
         #endif
      }
      else
         return 0 ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4lockFile( DATA4 *data )
{
   #ifdef S4SINGLE
      return 0 ;
   #else
      CODE4 *c4 ;
         int rc ;

      #ifdef S4VBASIC
         if ( c4parm_check( data, 2, E92709 ) )
            return -1 ;
      #endif

      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm, E92709 ) ;
      #endif

      c4 = data->codeBase ;
      if ( error4code( c4 ) < 0 )
         return e4codeBase ;

      if ( d4lockTestFile( data ) )
         return 0 ;

      #ifdef S4CLIENT
         return dfile4lockFile( data->dataFile, data4clientId( data ), data4serverId( data ), data ) ;
      #else
         switch( code4unlockAuto( c4 ) )
         {
            case LOCK4ALL :
               rc = code4unlockDo( tran4dataList( data->trans ) ) ;
               break ;
            case LOCK4DATA :
               rc = d4unlockLow( data, data4clientId( data ), 0 ) ;
               break ;
            default:
               rc = 0 ;
               break ;
         }
         if( rc < 0 )
            return error4stack( c4, e4unlock, E92709 ) ;
         return dfile4lockFile( data->dataFile, data4clientId( data ), data4serverId( data ) ) ;
      #endif
   #endif
}

#ifndef S4SINGLE
#ifdef S4CLIENT
#ifdef E4LOCK
/* this function does a compare of a lock with the server.  client and server
   should always return the same result (input value) */
static int request4lockTest( DATA4 *data, short int lockType, long rec, int clientVal )
{
   CONNECTION4 *connection ;
   CONNECTION4LOCK_INFO_IN *info ;
   int rc ;
   long recNum ;

   if ( error4code( data->codeBase )
      return -1 ;

   connection = data->dataFile->connection ;
   if ( connection == 0 )
      rc = e4connection ;
   else
   {
      connection4assign( connection, CON4LOCK, data4clientId( data ), data4serverId( data ) ) ;
      connection4addData( connection, NULL, sizeof( CONNECTION4LOCK_INFO_IN ), &info ) ;
      info->type = htons(lockType) ;
      info->test = htons(1) ;
      if ( lockType == LOCK4RECORD )
      {
         recNum = htonl(rec) ;
         connection4addData( connection, &recNum, sizeof( recNum ), NULL ) ;
      }
      connection4send( connection ) ;
      rc = connection4receive( connection ) ;
      if ( rc == 0 )
      {
         rc = connection4status( connection ) ;
         if ( rc < 0 )
            return connection4error( connection, data->codeBase, rc, E92722 ) ;
      }
   }
   if ( rc != clientVal )
      return error4( data->codeBase, e4info, E92722 ) ;

   return rc ;
}
#endif
#endif
#endif

/* for client, this function also checks the data's CODE4 to see if another
   access point for the DATA4 has the desired lock.
   If it does, data->dataFile->lockTest is set to the offending DATA4 */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4lockTest( DATA4 *data, const long rec )
{
   #ifndef S4SINGLE
      int rc ;

      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm, E92703 ) ;
      #endif

      rc = dfile4lockTest( data->dataFile, data4clientId( data ), data4serverId( data ), rec ) ;

      #ifdef S4CLIENT
         if ( rc < 0 )
            return rc ;
         #ifdef E4LOCK
            return request4lockTest( data, LOCK4RECORD, rec, rc )  ;
         #endif
      #endif

      return rc ;
   #else
      return 1 ;
   #endif
}

#ifndef S4STAND_ALONE
int S4FUNCTION d4lockTestAppendLow( DATA4 *data )
{
   #ifndef S4SINGLE
      int rc ;

      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm, E92704 ) ;
      #endif

         if ( data->accessMode == OPEN4DENY_RW )
            return 1 ;

      rc = dfile4lockTestAppend( data->dataFile, data4clientId( data ), data4serverId( data ) ) ;

      #ifdef S4CLIENT
         #ifdef E4LOCK
            return request4lockTest( data, LOCK4APPEND, 0L, rc )  ;
         #endif
      #endif

      return rc ;
   #else
      return 1 ;
   #endif
}
#endif

#ifdef S4CLIENT
int S4FUNCTION d4lockTestFileLow( DATA4 *data )
{
   #ifndef S4SINGLE
      int rc ;

      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm_null, E92705 ) ;
      #endif

         if ( data->accessMode != OPEN4DENY_NONE )
            return 1 ;

      rc = dfile4lockTestFile( data->dataFile, data4clientId( data ), data4serverId( data ) ) ;

      #ifdef S4CLIENT
         #ifdef E4LOCK
            return request4lockTest( data, LOCK4FILE, 0L, rc ) ;
         #endif
      #endif

      return rc ;
   #else
      return 1 ;
   #endif
}
#endif /* S4CLIENT */
