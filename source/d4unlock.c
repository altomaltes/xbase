/* d4unlock.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4SINGLE
static int d4hasLocks( DATA4 *data, long clientId, long serverId )
{
      LOCK4 *lock ;

      if ( serverId == 0 )   /* likely failed open */
         return 0 ;


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
}
#endif

#ifndef S4SINGLE
/* clientId if set to 0 will unlock all client instance of the data file,
   if set to a value will only unlock the given client instance */
static int d4unlockDo( DATA4 *data, const long clientId, char doReqdUpdate )
{
   CODE4 *c4 ;
      int rc, saveUnlockAuto ;

   c4 = data->codeBase ;

   #ifndef S4OFF_TRAN
      #ifndef S4OFF_WRITE
         if ( code4transEnabled( c4 ) )
            if ( code4tranStatus( c4 ) == r4active )
               return error4( c4, e4transViolation, E92801 ) ;
      #endif
   #endif

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

         d4unlockData( data ) ;
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
}
#endif /* S4SINGLE */

#ifndef S4SINGLE
/* AS 07/08/97 externally, d4unlock() must doReqdUpdate due to fix #89
   in changes.60 / manual documentation.  Internally, this causes problems,
   so internally doReqdUpdate is always false */
int S4FUNCTION d4unlockLow( DATA4 *data, long clientId, char doReqdUpdate )
{
   int rc ;

   rc = d4unlockDo( data, clientId, doReqdUpdate ) ;

   return rc ;
}
#endif

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

      data->recNumOld = -1 ;
      #ifndef S4OFF_MEMO
         data->memoValidated =  0 ;
      #endif

      return dfile4unlockRecords( data->dataFile, data4clientId( data ), data4serverId( data ) ) ;
   #else
      return 0 ;
   #endif
}


#ifndef S4SINGLE
int code4unlockDo( LIST4 *dataList )
{
   DATA4 *dataOn ;
   CODE4 *c4 ;

   c4 = 0 ;

   #ifdef E4PARM_HIGH
      if ( dataList == 0 )
         return error4( 0, e4parm_null, E92807 ) ;
   #endif

      for ( dataOn = 0 ;; )
      {
         dataOn = (DATA4 *)l4next( dataList, dataOn ) ;
         if ( dataOn == 0 )
            break ;
         /* reset record count because this function is likely called due to a transaction rollback */
         d4unlockLow( dataOn, 0, 0 ) ;  /* 0 for clientId to ensure all get unlocked */
         c4 = dataOn->codeBase ;
      }

   if ( c4 != 0 )
   {

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

         return code4unlockDo( tran4dataList( (&(c4->c4trans.trans)) ) ) ;
   #endif
}

