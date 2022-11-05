/* d4unlock.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4SINGLE
static int d4hasLocks( DATA4 *data, long clientId, long serverId )
{
   #ifdef S4CLIENT
      LOCK4LINK *lock ;

      #ifdef L4LOCK_CHECK
         at return time, make sure call server to see if the expected lock matches...
      #endif

      if ( serverId == 0 )   /* likely failed open */
         return 0 ;

      if ( data->dataFile->fileLock != data && data->dataFile->appendLock != data )
      {
         for ( lock = (LOCK4LINK *)single4initIterate( &data->dataFile->lockedRecords ) ;; )
         {
            if ( lock == 0 )
               return 0 ;
            if ( lock->data == data )
               return 1 ;
            lock = (LOCK4LINK *)single4next( &lock->link ) ;
         }
      }

      return 1 ;
   #else
      LOCK4 *lock ;

      if ( serverId == 0 )   /* likely failed open */
         return 0 ;

      #ifdef S4SERVER
         if ( data->accessMode == OPEN4DENY_RW )
            return 0 ;
      #endif

      if ( ( data->dataFile->fileServerLock == serverId && ( data->dataFile->fileClientLock == clientId || clientId == 0 ) ) ||
           ( ( data->dataFile->appendClientLock == clientId || clientId == 0 ) && data->dataFile->appendServerLock == serverId ) )
         return 1 ;

      for ( lock = (LOCK4 *)single4initIterate( &data->dataFile->lockedRecords ) ;; )
      {
         if ( lock == 0 )
            return 0 ;
         if ( ( lock->id.clientId == clientId || clientId == 0 ) && lock->id.serverId == serverId )
            return 1 ;
         lock = (LOCK4 *)single4next( &lock->link ) ;
      }
   #endif
}
#endif

/*
#ifdef S4CLIENT
void d4unlockClientData( DATA4 *data )
{
   DATA4FILE *dfile ;
   LOCK4LINK *lock, *nextLock ;

   #ifdef S4SERVER
      if ( data->accessMode == OPEN4DENY_RW )
         return ;
   #endif

   dfile = data->dataFile ;

   if ( dfile->fileLock == data )
   {
      data->dataFile->numRecs = -1 ;
      dfile->fileLock = 0 ;
   }
   if ( dfile->appendLock == data )
   {
      data->dataFile->numRecs = -1 ;
      dfile->appendLock = 0 ;
   }
   lock = (LOCK4LINK *)l4first( &dfile->lockedRecords ) ;
   if ( lock != 0 )
      for ( ;; )
      {
         nextLock = (LOCK4LINK *)l4next( &dfile->lockedRecords, lock ) ;
         if ( lock->data == data )
         {
            l4remove( &dfile->lockedRecords, lock ) ;
            mem4free( data->codeBase->lockLinkMemory, lock ) ;
         }
         if ( nextLock == 0 )
            break ;
         lock = nextLock ;
      }

   return ;
}
#endif
*/

#ifndef S4SINGLE
/* clientId if set to 0 will unlock all client instance of the data file,
   if set to a value will only unlock the given client instance */
static int d4unlockDo( DATA4 *data, const long clientId, char doReqdUpdate )
{
   CODE4 *c4 ;
   #ifdef S4CLIENT
      int rc ;
      CONNECTION4 *connection ;
   #else
      int rc, saveUnlockAuto ;
   #endif
   #ifdef S4SERVER
      unsigned short int mType ;
      long ID ;
   #endif

   c4 = data->codeBase ;

   #ifndef S4OFF_TRAN
      #ifndef S4OFF_WRITE
         if ( code4transEnabled( c4 ) )
            if ( code4tranStatus( c4 ) == r4active )
               return error4( c4, e4transViolation, E92801 ) ;
      #endif
   #endif

   #ifdef S4CLIENT
      if ( doReqdUpdate == 0 )
         if ( d4hasLocks( data, clientId, data4serverId( data ) ) == 0 )  /* first make sure there are locks to undo */
            return 0 ;

      rc =  d4update( data ) ;  /* returns -1 if error4code( codeBase ) < 0 */
      if ( rc < 0 )
         return error4stack( c4, (short int)rc, E92801 ) ;

      if ( d4hasLocks( data, clientId, data4serverId( data ) ) == 0 )  /* first make sure there are locks to undo */
         return 0 ;

      /* in case of rollback and exclusive files, make sure count set to -1 */
      data->dataFile->numRecs = -1 ;

      connection = data->dataFile->connection ;
      if ( connection == 0 )
         return error4stack( c4, e4connection, E92801 ) ;
      connection4assign( connection, CON4UNLOCK, clientId, data4serverId( data ) ) ;

      connection4sendMessage( connection ) ;
      rc = connection4receiveMessage( connection ) ;
      if ( rc < 0 )
         return error4stack( c4, rc, E92801 ) ;

      rc = connection4status( connection ) ;
      if ( rc < 0 )
         return connection4error( connection, c4, rc, E92801 ) ;

      /* AS 01/09/97, since now have STREAM4UNLOCK_DATA handles, don't need this code */
        /* cb51 compat, remove lock */
        /* if ( code4trans( c4 )->unlockAuto == 2 ) */
        /*   d4unlockClientData( data ) ; */

      return rc ;
   #else
      #ifndef S4OFF_WRITE
         #ifndef S4OFF_TRAN
            if ( code4transEnabled( c4 ) )
               if ( code4tranStatus( c4 ) != r4inactive )
                  return 0 ;
         #endif

         if ( doReqdUpdate == 0 )
            if ( d4hasLocks( data, clientId, data4serverId( data ) ) == 0 )  /* first make sure there are locks to undo */
               return 0 ;

         rc =  d4update( data ) ;  /* returns -1 if error4code( codeBase ) < 0 */
         if ( rc < 0 )
            return error4stack( c4, (short int)rc, E92801 ) ;
         if ( d4hasLocks( data, clientId, data4serverId( data ) ) == 0 )  /* first make sure there are locks to undo */
            return 0 ;
      #else
         rc = 0 ;
      #endif

      saveUnlockAuto = code4unlockAuto( c4 ) ;
      if ( saveUnlockAuto == 0 )   /* leave if 1 or 2 -- don't change 2 */
         code4unlockAutoSet( c4, 1 ) ;

      #ifdef S4SERVER
         dfile4unlockData( data->dataFile, clientId, data4serverId( data ) ) ;
         #ifdef S4JAVA
            if ( c4->currentClient->javaClient == 0 )
         #endif
            if ( code4unlockAuto( c4 ) == LOCK4DATA )
               if ( &c4->currentClient->connection != 0 )
               {
                  /* send a STREAM4UNLOCK_DATA message to the client to tell it to unlock stuff */
                  mType = htons(STREAM4UNLOCK_DATA) ;
                  connection4send( &c4->currentClient->connection, &mType, sizeof( mType ) ) ;
                  /* also send the clientId and serverId */
                  ID = htonl( data->clientId ) ;
                  connection4send( &c4->currentClient->connection, &ID, sizeof( ID ) ) ;
                  ID = htonl( data->serverId ) ;
                  connection4send( &c4->currentClient->connection, &ID, sizeof( ID ) ) ;
               }
      #else
         d4unlockData( data ) ;
      #endif
      #ifndef N4OTHER
         #ifndef S4OFF_MEMO
            dfile4memoUnlock( data->dataFile ) ;
         #endif
      #endif
      #ifndef S4OFF_INDEX
         dfile4unlockIndex( data->dataFile, data4serverId( data ) ) ;
      #endif

      code4unlockAutoSet( c4, saveUnlockAuto ) ;

      if ( error4code( c4 ) < 0 )
         return -1 ;
      return rc ;
   #endif
}
#endif /* S4SINGLE */

#ifndef S4SINGLE
/* AS 07/08/97 externally, d4unlock() must doReqdUpdate due to fix #89
   in changes.60 / manual documentation.  Internally, this causes problems,
   so internally doReqdUpdate is always false */
int S4FUNCTION d4unlockLow( DATA4 *data, long clientId, char doReqdUpdate )
{
   int rc ;

   #ifdef S4CLIENT
      int oldLock = code4unlockAuto( data->codeBase ) ;
      code4unlockAutoSet( data->codeBase, LOCK4DATA ) ;
   #endif
   rc = d4unlockDo( data, clientId, doReqdUpdate ) ;
   #ifdef S4CLIENT
      code4unlockAutoSet( data->codeBase, oldLock ) ;
   #endif

   return rc ;
}
#endif

#ifndef S4SERVER
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4unlock( DATA4 *data )
{
   #ifndef S4SINGLE
      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm_null, E92801 ) ;
      #endif

      return d4unlockLow( data, data4clientId( data ), 1 ) ;
   #else
      return 0 ;
   #endif
}
#endif

/*#ifdef S4STAND_ALONE*/

/* only unlocks the append byte */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int d4unlockAppend( DATA4 *data )
{
   #ifndef S4SINGLE
      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm_null, E92802 ) ;
      #endif
      if ( code4unlockAuto( data->codeBase ) == LOCK4OFF )
         return 0 ;

      #ifdef S4SERVER
         if ( data->accessMode == OPEN4DENY_RW )
            return 0 ;
      #endif

      return dfile4unlockAppend( data->dataFile, data4clientId( data ), data4serverId( data ) ) ;
   #else
      return 0 ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int d4unlockData( DATA4 *data )
{
   #ifndef S4SINGLE
      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm_null, E92803 ) ;
      #endif
      if ( code4unlockAuto( data->codeBase ) == LOCK4OFF )
         return 0 ;

      #ifdef S4SERVER
         if ( data->accessMode == OPEN4DENY_RW )
            return 0 ;
      #endif

      d4unlockFile( data ) ;
      d4unlockAppend( data ) ;
      d4unlockRecords( data ) ;
      if ( error4code( data->codeBase ) < 0 )
         return error4code( data->codeBase ) ;
   #endif
   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int d4unlockFile( DATA4 *data )
{
   #ifndef S4SINGLE
      int rc ;
      #ifdef S4VBASIC
         if ( c4parm_check( data, 2, E92804 ) )
            return -1 ;
      #endif

      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm_null, E92804 ) ;
      #endif

      if ( code4unlockAuto( data->codeBase ) == LOCK4OFF )
         return 0 ;

      #ifdef S4SERVER
         if ( data->accessMode == OPEN4DENY_RW )
            return 0 ;
      #endif

      rc = dfile4unlockFile( data->dataFile, data4clientId( data ), data4serverId( data ) ) ;
      if ( rc < 0 )
         return error4stack( data->codeBase, rc, E92804 ) ;

      data->recNumOld =  -1 ;
      #ifndef S4OFF_MEMO
         data->memoValidated =  0 ;
      #endif
   #endif
   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4unlockRecord( DATA4 *data, long rec )
{
   #ifndef S4SINGLE
      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm_null, E92805 ) ;
      #endif

      if ( code4unlockAuto( data->codeBase ) == LOCK4OFF )
         return 0 ;

      #ifdef S4SERVER
         if ( data->accessMode == OPEN4DENY_RW )
            return 0 ;
      #endif

      if ( rec == data->recNum )
      {
         data->recNumOld =  -1 ;
         #ifndef S4OFF_MEMO
            data->memoValidated =  0 ;
         #endif
      }

      return dfile4unlockRecord( data->dataFile, data4clientId( data ), data4serverId( data ), rec ) ;
   #else
      return 0 ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int d4unlockRecords( DATA4 *data )
{
   #ifndef S4SINGLE
      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parm_null, E92806 ) ;
      #endif

      if ( code4unlockAuto( data->codeBase ) == LOCK4OFF )
         return 0 ;

      #ifdef S4SERVER
         if ( data->accessMode == OPEN4DENY_RW )
            return 0 ;
      #endif

      data->recNumOld = -1 ;
      #ifndef S4OFF_MEMO
         data->memoValidated =  0 ;
      #endif

      return dfile4unlockRecords( data->dataFile, data4clientId( data ), data4serverId( data ) ) ;
   #else
      return 0 ;
   #endif
}
/*#endif  */ /* S4STAND_ALONE */

#ifndef S4SINGLE
int code4unlockDo( LIST4 *dataList )
{
   DATA4 *dataOn ;
   CODE4 *c4 ;
   #ifdef S4CLIENT
      int oldLock ;
   #endif
   #ifdef S4SERVER
      unsigned short int mType ;
   #endif

   c4 = 0 ;

   #ifdef E4PARM_HIGH
      if ( dataList == 0 )
         return error4( 0, e4parm_null, E92807 ) ;
   #endif

   #ifdef S4CLIENT
      /* for client, any request with LOCK4ALL should cause complete
         unlocking of everything at the lower level.
         Therefore, only need to call on a single database--but that
         database better have a lock.  if none have locks, call is
         avioded. */
      for( dataOn = 0 ;; )
      {
         dataOn = (DATA4 *)l4next( dataList, dataOn ) ;
         if ( dataOn == 0 )
            break ;
         if ( d4hasLocks( dataOn, data4clientId( dataOn ), data4serverId( dataOn ) ) != 0 )
         {
            c4 = dataOn->codeBase ;
            oldLock = code4unlockAuto( c4 ) ;
            code4unlockAutoSet( c4, LOCK4ALL ) ;
            d4unlockDo( dataOn,data4clientId(dataOn), 1 ) ;
            code4unlockAutoSet( c4, oldLock ) ;
            break ;
         }
      }
   #else
      for ( dataOn = 0 ;; )
      {
         dataOn = (DATA4 *)l4next( dataList, dataOn ) ;
         if ( dataOn == 0 )
            break ;
         /* reset record count because this function is likely called due to a transaction rollback */
         d4unlockLow( dataOn, 0, 0 ) ;  /* 0 for clientId to ensure all get unlocked */
         c4 = dataOn->codeBase ;
      }
   #endif

   if ( c4 != 0 )
   {
      #ifdef S4SERVER
         #ifdef S4JAVA
            if ( c4->currentClient->javaClient == 0 )
         #endif
            if ( c4->currentClient->connection.connect != 0 && c4->currentClient->isStream == 0)
            {
               /* send a STREAM4UNLOCK_ALL message to the client to tell it to unlock stuff */
               mType = htons(STREAM4UNLOCK_ALL) ;
               connection4send( &c4->currentClient->connection, &mType, sizeof( mType ) ) ;
            }
      #endif

      if ( error4code( c4 ) < 0 )
         return error4code( c4 ) ;
   }
   return 0 ;
}
#endif  /* S4SINGLE */

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION code4unlock( CODE4 *c4 )
{
   #ifdef S4SINGLE
      return 0 ;
   #else
      #ifndef S4OFF_WRITE
         #ifndef S4OFF_TRAN
            if ( code4transEnabled( c4 ) )
               if ( code4tranStatus( c4 ) == r4active )
                  return error4( c4, e4transViolation, E92807 ) ;
         #endif
      #endif

      #ifdef S4SERVER
         return server4clientUnlock( c4->currentClient ) ;
      #else
         return code4unlockDo( tran4dataList( (&(c4->c4trans.trans)) ) ) ;
      #endif
   #endif
}

