/* *********************************************************************************************** */
/* Copyright (C) 1999-2015 by Sequiter, Inc., 9644-54 Ave, NW, Suite 209, Edmonton, Alberta Canada.*/
/* This program is free software: you can redistribute it and/or modify it under the terms of      */
/* the GNU Lesser General Public License as published by the Free Software Foundation, version     */
/* 3 of the License.                                                                               */
/*                                                                                                 */
/* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;       */
/* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.       */
/* See the GNU Lesser General Public License for more details.                                     */
/*                                                                                                 */
/* You should have received a copy of the GNU Lesser General Public License along with this        */
/* program. If not, see <https://www.gnu.org/licenses/>.                                           */
/* *********************************************************************************************** */

/* d4unlock.c   (c)Copyright Sequiter Software Inc., 1988-2001.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_MULTI
   // AS Apr 15/03 - support for new lockId for shared clone locking
   static int d4hasLocks( DATA4 *data, long lockId, long serverId )
   {
      assert5( data != 0 ) ;
      #ifdef S4CLIENT
         LOCK4LINK *lock ;

         #ifdef L4LOCK_CHECK
            at return time, make sure call server to see if the expected lock matches...
         #endif

         if ( serverId == 0 )   /* likely failed open */
            return 0 ;

         DATA4FILE *d4file = data->dataFile ;
         assert5( d4file != 0 ) ;
         // AS May 27/03 - change for cloned locking, store the lockid/serverid, not the data4 itself
         // if ( d4file->fileLock != data && d4file->appendLock != data )
         if ( (d4file->fileLockLockId != data4lockId( data ) || d4file->fileLockServerId != data4serverId( data )) &&
              (d4file->appendLockLockId != data4lockId( data ) || d4file->appendLockServerId != data4serverId( data )) )
         {
            for ( lock = (LOCK4LINK *)(d4file->lockedRecords.initIterate()) ;; )
            {
               if ( lock == 0 )
                  return 0 ;
               // if ( lock->data == data )
               if ( lock->serverId == data4serverId( data ) && lock->lockId == data4lockId( data ) )
                  return 1 ;
               lock = (LOCK4LINK *)single4next( &lock->link ) ;
            }
         }

         return 1 ;
      #else
         Single4lock *lock ;

         if ( serverId == 0 )   /* likely failed open */
            return 0 ;

         #ifdef S4SERVER
            if ( data->accessMode == OPEN4DENY_RW )
               return 0 ;
         #endif

         DATA4FILE *d4file = data->dataFile ;
         assert5( d4file != 0 ) ;

         if ( ( d4file->fileServerWriteLock == serverId && ( d4file->fileClientWriteLock == lockId || lockId == 0 ) ) ||
              ( ( d4file->appendClientLock == lockId || lockId == 0 ) && d4file->appendServerLock == serverId ) )
            return 1 ;

         Lock4 *readFileLocks ;
         readFileLocks = (Lock4 *)((Single4lock *)(d4file->fileReadLocks.initIterate())) ;
         for ( ;; )
         {
            if ( readFileLocks == 0 )
               break ;
            if ( readFileLocks->data == data )
            {
               #ifdef S4SERVER
                  if ( readFileLocks->clientId == lockId || lockId == 0 )
               #endif
                  return 1 ;
            }
            readFileLocks = (Lock4 *)(Single4lock *)((Single4lock *)(readFileLocks))->next() ;
         }

         lock = (Single4lock *)(data->lockedRecords.initIterate()) ;
         #ifdef S4SERVER
            for ( ;; )
            {
               if ( lock == 0 )
                  return 0 ;
               if ( ( ((Lock4 *)lock)->clientId == lockId || lockId == 0 ) )
                  return 1 ;
               lock = (Single4lock *)(lock->next()) ;
            }
         #else
            return ( (lock == 0) ? 0 : 1 ) ;
         #endif
      #endif
   }
#endif /* !S4OFF_MULTI */



#ifndef S4OFF_MULTI
   // AS Apr 15/03 - support for new lockId for shared clone locking
   static int d4unlockDo( DATA4 *data, const long lockId, char doReqdUpdate )
   {
      /* lockId if set to 0 will unlock all client instance of the data file,
         if set to a value will only unlock the given client instance */
      CODE4 *c4 ;
      #ifdef S4CLIENT
         int rc ;
         CONNECTION4 *connection ;
      #else
         int rc, saveUnlockAuto ;
      #endif

      c4 = data->codeBase ;

      // AS Apr 28/03 - transcations are run-time in odbc now
      // AS Jun 20/03 - was checking wrong flag
      #if defined( S4SERVER ) && defined( S4ODBC_BUILD )
         if ( c4->server->odbcTrans )  // odbc build, no trans available
      #endif
         {
            #if !defined( S4OFF_TRAN ) && !defined( S4OFF_WRITE )
               if ( code4transEnabled( c4 ) && code4tranStatus( c4 ) == r4active )
               {
                  #ifdef S4STAND_ALONE
                     // AS Feb 14/03 - Relax constraint for temporary tables
                     if ( data->dataFile->file.isReadOnly != 1 && data->dataFile->file.isTemporary == 0 )  // AS 04/10/00 allow to close if readOnly
                  #endif
                     {
                        // AS Feb 5/03 - with ODBC, just continue the unlock since it is called on a close
                        if ( c4->odbc != 1 )
                           return error4( c4, e4transViolation, E92801 ) ;
                     }
               }
            #endif
         }

         // AS July 16/02 - d4hasLocks() only returns true if there are data file locks, not index locks.
         // for example, calling d4reindex() only locks the index.
         Bool5 dbfHasLocks = 1 ;
         rc = 0 ;
         #ifndef S4OFF_WRITE
            // AS Apr 28/03 - transcations are run-time in odbc now
            // AS Jun 20/03 - was checking wrong flag
            #if defined( S4SERVER ) && defined( S4ODBC_BUILD )
               if ( c4->server->odbcTrans )  // odbc build, no trans available
            #endif
               {
                  #ifndef S4OFF_TRAN
                     // AS Feb 17/03 - other cases we allow to continue with unlock (on close)
                     if ( code4transEnabled( c4 ) && code4tranStatus( c4 ) != r4inactive )  // can't unlock in this instance
                     {
                        TRAN4 *trans = code4trans( c4 ) ;
                        if ( data->dataFile->file.isReadOnly == 0 && trans != 0 && trans->currentTranStatus == r4active && data->dataFile->file.isTemporary == 0 )
                           return 0 ;
                     }
                  #endif
               }


            if ( doReqdUpdate == 0 )
               if ( d4hasLocks( data, lockId, data4serverId( data ) ) == 0 )  /* first make sure there are locks to undo */
                  dbfHasLocks = 0 ;

            if ( dbfHasLocks == 1 )  // if there is a locked record, update it...
            {
               rc =  d4update( data ) ;  /* returns -1 if error4code( codeBase ) < 0 */
               if ( rc < 0 )
                  return error4stack( c4, (short int)rc, E92801 ) ;
               if ( d4hasLocks( data, lockId, data4serverId( data ) ) == 0 )  /* first make sure there are locks to undo */
                  dbfHasLocks = 0 ;
            }
         #else
            rc = 0 ;
         #endif

         saveUnlockAuto = code4unlockAuto( c4 ) ;
         if ( saveUnlockAuto == 0 )   /* leave if 1 or 2 -- don't change 2 */
            code4unlockAutoSet( c4, 1 ) ;

         // AS July 16/02 - d4hasLocks() only returns true if there are data file locks, not index locks.
         // for example, calling d4reindex() only locks the index.
         if ( dbfHasLocks == 1 )
         {
               d4unlockData( data ) ;
         }

         #if !defined( S4CLIPPER ) && !defined( S4OFF_MEMO )
            dfile4memoUnlock( data->dataFile ) ;
         #endif

         #ifndef S4OFF_INDEX
            dfile4unlockIndex( data->dataFile, data4serverId( data ) ) ;
         #endif

         code4unlockAutoSet( c4, saveUnlockAuto ) ;

         if ( error4code( c4 ) < 0 )
            return -1 ;
         return rc ;
   }
#endif /* S4OFF_MULTI */



#ifndef S4OFF_MULTI
   // AS Apr 15/03 - support for new lockId for shared clone locking
   int S4FUNCTION d4unlockLow( DATA4 *data, long lockId, char doReqdUpdate )
   {
      /* AS 07/08/97 externally, d4unlock() must doReqdUpdate due to fix #89
         in changes.60 / manual documentation.  Internally, this causes problems,
         so internally doReqdUpdate is always false */
      int rc ;

      #ifdef S4SERVER
         // AS Oct 23/02 - It is possible that the data is not initialized yet, in which
         // case there is nothing to unlock (happens on server with system table initialization)
         if ( data->dataFile == 0 )
            return 0 ;
      #endif

      #ifdef S4CLIENT
         int oldLock = code4unlockAuto( data->codeBase ) ;
         code4unlockAutoSet( data->codeBase, LOCK4DATA ) ;
      #endif
      rc = d4unlockDo( data, lockId, doReqdUpdate ) ;
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
      #ifndef S4OFF_MULTI
         #ifdef E4PARM_HIGH
            if ( data == 0 )
               return error4( 0, e4parm_null, E92801 ) ;
         #endif

         // AS Jul 30/02 - was not checking param validitiy for v.b.
         #ifdef E4VBASIC
            if ( c4parm_check( data, 2, E92801 ) )
               return -1 ;
         #endif

         return d4unlockLow( data, data4lockId( data ), 1 ) ;
      #else
         return 0 ;
      #endif
   }
#endif



#ifdef P4ARGS_USED
   #pragma argsused
#endif
// AS Apr 15/03 - support for new lockId for shared clone locking and in client/server
int S4FUNCTION d4unlockAppendInternal( DATA4 *data, long lockId )
{
      #ifndef S4OFF_MULTI
         #ifdef E4PARM_HIGH
            if ( data == 0 )
               return error4( 0, e4parm_null, E92802 ) ;
         #endif
         // AS Apr 15/03 - d4unlockAppend() now ignores the Code4.unlockAuto
         // if ( code4unlockAuto( data->codeBase ) == LOCK4OFF )
         //    return 0 ;

         #ifdef S4SERVER
            if ( data->accessMode == OPEN4DENY_RW )
               return 0 ;
         #endif

         if ( lockId == -1 )
            lockId = data4lockId( data ) ;
         return dfile4unlockAppend( data->dataFile, lockId, data4serverId( data ) ) ;
      #else
         return 0 ;
      #endif
}


   // AS Apr 15/03 - support for new lockId for shared clone locking
   int S4FUNCTION d4unlockAppend( DATA4 *data )
   {
      return d4unlockAppendInternal( data, data4lockId( data ) ) ;
   }



#ifndef S4CLIENT
   #ifdef P4ARGS_USED
      #pragma argsused
   #endif
   /* AS Nov 13/02 - export for dot */
   // AS Apr 15/03 - support for new lockId for shared clone locking
   // !S4CLIENT
   int S4FUNCTION d4unlockData( DATA4 *data, long lockId )
   {
      #ifndef S4OFF_MULTI
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

         d4unlockFile( data, lockId ) ;
         // AS Apr 15/03 - support for new lockId for shared clone locking
         d4unlockAppendInternal( data, lockId ) ;
         d4unlockRecords( data, lockId ) ;
         if ( error4code( data->codeBase ) < 0 )
            return error4code( data->codeBase ) ;
      #endif
      return 0 ;
   }



   #ifdef P4ARGS_USED
      #pragma argsused
   #endif
   /* AS Nov 13/02 - export for dot */
   // AS Apr 15/03 - support for new lockId for shared clone locking
   // !S4CLIENT
   int S4FUNCTION d4unlockFile( DATA4 *data, long lockId )
   {
      #ifndef S4OFF_MULTI
         int rc ;
         #ifdef E4VBASIC
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

         if ( lockId == -1 )
            lockId = data4lockId( data ) ;

         Single4distant distant ;
         distant.initIterate( &data->dataFile->fileReadLocks ) ;

         for ( ;; )  // just remove the read file locks, no locking required
         {
            if ( distant.toItem() == 0 )
               break ;
            if ( ((Lock4 *)((Single4lock *)(distant.toItem())))->data == data )
            {
               #ifdef S4SERVER
                  if ( ((Lock4 *)((Single4lock *)(distant.toItem())))->clientId == lockId || lockId == 0 )
               #endif
               {
                  distant.remove() ;
                  continue ;
               }
            }
            distant.next() ;
         }

         rc = dfile4unlockFile( data->dataFile, lockId, data4serverId( data ) ) ;
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
   // !S4CLIENT
   int S4FUNCTION d4unlockRecord( DATA4 *data, long rec )
   {
      #ifndef S4OFF_MULTI
         Lock4 *lock ;
         Single4distant singleDistant ;

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

         singleDistant.initIterate( &data->lockedRecords ) ;
         for ( ;; )
         {
            lock = (Lock4 *)((Single4lock *)singleDistant.toItem()) ;
            if ( lock == 0 )
               break ;
            if ( lock->recNum == rec )
            {
               if ( lock->lockType == lock4write )
               {
                  if ( dfile4unlockRecordDo( data->dataFile, rec ) < 0 )
                     return -1 ;
               }
               else
                  data->dataFile->recordLockReadCount-- ;
               singleDistant.remove() ;
               #ifdef S4LOCK_HASH
                  data->dataFile->lockHash->remove( lock ) ;
               #endif
               mem4free( data->codeBase->lockMemory, lock ) ;
               break ;
            }
            singleDistant.next() ;
         }
      #endif
      return 0 ;
   }



   #ifdef P4ARGS_USED
      #pragma argsused
   #endif
   /* AS Nov 13/02 - export for dot */
   // AS Apr 15/03 - support for new lockId for shared clone locking
   // !S4CLIENT
   int S4FUNCTION d4unlockRecords( DATA4 *data, long lockId )
   {
      #ifndef S4OFF_MULTI
         #ifndef S4CLIENT
            Lock4 *lock ;
            Single4distant singleDistant ;
         #endif
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

         #ifdef S4CLIENT
            return dfile4unlockRecords( data->dataFile, data4lockId( data ), data4serverId( data ) ) ;
         #else
            if ( lockId == -1 )
               lockId = data4lockId( data ) ;

            singleDistant.initIterate( &data->lockedRecords ) ;
            for ( ;; )
            {
               lock = (Lock4 *)((Single4lock *)singleDistant.toItem()) ;
               if ( lock == 0 )
                  break ;
               #ifdef S4SERVER
                  if ( lockId == lock->clientId || lockId == 0 )
                  {
               #endif
                     if ( lock->lockType == lock4write )  // only physically lock/unlcok write locks
                     {
                        if ( dfile4unlockRecordDo( data->dataFile, lock->recNum ) < 0 )
                        {
                           data->lockedRecords.init() ;   /* clear out for error case */
                           return -1 ;
                        }
                     }
                     else
                        data->dataFile->recordLockReadCount-- ;
                     #ifdef S4LOCK_HASH
                        data->dataFile->lockHash->remove( lock ) ;
                     #endif
                     singleDistant.remove() ;  /* update before freeing memory */
                     mem4free( data->codeBase->lockMemory, lock ) ;
               #ifdef S4SERVER
                  }
                  else
                     singleDistant.next() ;
               #endif
            }
            return 0 ;
         #endif /* S4CLIENT */
      #else
         return 0 ;
      #endif
   }
#endif /* S4CLIENT */



#ifndef S4OFF_MULTI
   int code4unlockDo( LIST4 *dataList )
   {
      DATA4 *dataOn ;
      CODE4 *c4 ;
      #ifdef S4CLIENT
         int oldLock ;
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
            if ( d4hasLocks( dataOn, data4lockId( dataOn ), data4serverId( dataOn ) ) != 0 )
            {
               c4 = dataOn->codeBase ;
               oldLock = code4unlockAuto( c4 ) ;
               code4unlockAutoSet( c4, LOCK4ALL ) ;
               d4unlockDo( dataOn, data4lockId(dataOn), 1 ) ;
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
            d4unlockLow( dataOn, 0, 0 ) ;  // 0 for lockId to ensure all get unlocked
            c4 = dataOn->codeBase ;
         }
      #endif

      if ( c4 != 0 )
      {

         if ( error4code( c4 ) < 0 )
            return error4code( c4 ) ;
      }
      return 0 ;
   }
#endif  /* S4OFF_MULTI */



#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION code4unlock( CODE4 *c4 )
{
   #ifdef S4OFF_MULTI
      return 0 ;
   #else
      #if !defined( S4OFF_WRITE ) && !defined( S4OFF_TRAN )
         // AS Apr 28/03 - transcations are run-time in odbc now
         // AS Jun 20/03 - was checking wrong flag
         #if defined( S4SERVER ) && defined( S4ODBC_BUILD )
            if ( c4->server->odbcTrans )  // odbc build, no trans available
         #endif
            {
               if ( code4transEnabled( c4 ) && code4tranStatus( c4 ) == r4active )
                  return error4( c4, e4transViolation, E92807 ) ;
            }
      #endif

      #ifdef S4SERVER
         return server4clientUnlock( c4->currentClient ) ;
      #else
         return code4unlockDo( tran4dataList( (&(c4->c4trans.trans)) ) ) ;
      #endif
   #endif
}

