/* df4lock.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4SINGLE

int dfile4lock( DATA4FILE *data, const long clientId, const long serverId, const long rec )
{
   int rc ;
   CODE4 *c4 ;
      FILE4LONG position ;
      LOCK4 *lock, *lockOn ;
      SINGLE4DISTANT singleDistant ;
   #ifdef S4FOX
      FILE4LONG pos2 ;
   #endif

   #ifdef E4PARM_LOW
      if ( data == 0 || rec < 1L || clientId == 0
           #ifdef S4SERVER
              || serverId == 0
           #endif
         )
         return error4( (data == 0 ? 0 : data->c4 ), e4parm, E91102 ) ;
   #endif

   c4 = data->c4 ;

   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

      if ( dfile4lockTest( data, clientId, serverId, rec ) > 0 )  /* if record or file already locked */
         return 0 ;

      if ( dfile4lockTest( data, 0L, 0L, rec ) > 0 )  /* if record or file already locked */
      {
         dfile4registerLocked( data, rec, 1 ) ;
         #ifndef S4SERVER
            if ( c4->lockAttempts == WAIT4EVER )
               return error4( c4, e4lock, E81523 ) ;
         #endif
         return r4locked ;
      }

      if ( c4->lockMemory == 0 )
      {
         c4->lockMemory = mem4create( c4, c4->memStartLock, sizeof( LOCK4 ), c4->memExpandLock, 0 ) ;
         if ( c4->lockMemory == 0 )
            return -1 ;
      }

      file4longAssign( position, 0, c4->largeFileOffset ) ;

      #ifdef S4CLIPPER
         file4longAdd( &position, L4LOCK_POS ) ;
         file4longAdd( &position, rec ) ;
      #endif
      #ifdef S4MDX
         file4longAdd( &position, L4LOCK_POS ) ;
         file4longSubtract( &position, rec + 1U ) ;
      #endif
      #ifdef S4FOX
         if ( c4->largeFileOffset == 0 )
         {
            if ( ( data->hasMdxMemo & 0x01 ) || data->version == 0x30 )
            {
               file4longAdd( &position, L4LOCK_POS ) ;
               file4longSubtract( &position, rec ) ;
            }
            else
            {
               file4longAdd( &position, L4LOCK_POS_OLD ) ;
               file4longAssignLong( pos2, dfile4recordPosition( data, rec ) ) ;
               file4longAddLong( &position, &pos2 ) ;
            }
         }
         else
            file4longAdd( &position, rec ) ;
      #endif

      #ifdef S4MDX
         rc = file4lockInternal( &data->file, L4LOCK_POS - 1U, 0, 1L, 0 ) ;
         if ( rc == 0 )
         {
      #endif
         rc = file4lockInternal( &data->file, file4longGetLo( position ), file4longGetHi( position ), 1L, 0 ) ;
      #ifdef S4MDX
            file4unlockInternal( &data->file, L4LOCK_POS - 1U, 0, 1L, 0 ) ;
         }
      #endif
      if ( rc )
      {
         if ( rc == r4locked )
            dfile4registerLocked( data, rec, 0 ) ;
         return rc ;
      }

      lock = (LOCK4 *)mem4alloc( c4->lockMemory ) ;
      if ( lock == 0 )
         return -1 ;

      lock->id.clientId = clientId ;
      lock->id.serverId = serverId ;
      lock->id.recNum = rec ;

      single4distantInitIterate( &singleDistant, &data->lockedRecords ) ;

      for ( ;; )
      {
         lockOn = (LOCK4 *)single4distantToItem( &singleDistant ) ;
         if ( lockOn == 0 )  /* add to end... */
         {
            single4add( single4distantToSingle( &singleDistant ), &lock->link ) ;
            break ;
         }
         else
            if ( lockOn->id.recNum > rec )  /* distant means we are 1 away, so just add at single (not at item) */
            {
               single4add( single4distantToSingle( &singleDistant ), &lock->link ) ;
               break ;
            }
         single4distantNext( &singleDistant ) ;
      }
      return 0 ;
}

#ifndef S4CLIPPER
#ifndef S4MEMO_OFF
int dfile4lockMemo( DATA4FILE *data )
{
   return memo4fileLock( &data->memoFile ) ;
}
#endif
#endif

int dfile4lockAppend( DATA4FILE *data, const long clientId, const long serverId )
{
   int rc ;
   CODE4 *c4 ;

   #ifdef E4PARM_LOW
      if ( data == 0 || clientId == 0
           #ifdef S4SERVER
              || serverId == 0
           #endif
         )
         return error4( 0, e4parm, E91102 ) ;
   #endif

   c4 = data->c4 ;
   if ( error4code( c4 ) < 0 )
      return -1 ;

      if ( dfile4lockTestAppend( data, clientId, serverId ) )
         return 0 ;

      if ( dfile4lockTestAppend( data, 0L, 0L ) )
      {
         dfile4registerLocked( data, 0L, 1 ) ;
         #ifndef S4SERVER
            if ( c4->lockAttempts == WAIT4EVER )
               return error4( c4, e4lock, E81523 ) ;
         #endif
         return r4locked ;
      }

      if ( c4->largeFileOffset == 0 )
      {
         #ifdef S4FOX
            if ( ( data->hasMdxMemo & 0x01 ) || data->version == 0x30 )
               rc = file4lockInternal( &data->file, L4LOCK_POS, c4->largeFileOffset, 1L, 0 ) ;
            else
               rc = file4lockInternal( &data->file, L4LOCK_POS_OLD, c4->largeFileOffset, 1L, 0 ) ;
         #else
            rc = file4lockInternal( &data->file, L4LOCK_POS, c4->largeFileOffset, 1L, 0 ) ;
         #endif
      }
      else  /* use 0 byte for append record */
         rc = file4lockInternal( &data->file, 0, c4->largeFileOffset, 1L, 0 ) ;
      if ( rc == 0 )
      {
         data->appendServerLock = serverId ;
         data->appendClientLock = clientId ;
      }
      if ( rc == r4locked )
         dfile4registerLocked( data, 0L, 0 ) ;

   return rc ;
}

#ifdef S4USE_ADDITIVE_LOCK
   #error additive locks no longer supported
#endif

int dfile4lockFile( DATA4FILE *data, const long clientId, const long serverId )
{
   int rc = 0 ;
   CODE4 *c4 ;

   #ifdef E4PARM_LOW
      if ( data == 0 || clientId == 0
           #ifdef S4SERVER
              || serverId == 0
           #endif
         )
         return error4( 0, e4parm, E91102 ) ;
   #endif

   c4 = data->c4 ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

      if ( dfile4lockTestFile( data, clientId, serverId ) )
         return 0 ;

      if ( dfile4lockTestFile( data, 0L, 0L ) )
      {
         dfile4registerLocked( data, -1L, 1 ) ;
         #ifndef S4SERVER
            if ( c4->lockAttempts == WAIT4EVER )
               return error4( c4, e4lock, E81523 ) ;
         #endif
         return r4locked ;
      }

      /* now check if any other lock is conflicting */
      if ( dfile4lockTestAppend( data, 0L, 0L ) )
      {
         dfile4registerLocked( data, 0L, 1 ) ;
         #ifndef S4SERVER
            if ( c4->lockAttempts == WAIT4EVER )
               return error4( c4, e4lock, E81523 ) ;
         #endif
         return r4locked ;
      }

      if ( dfile4lockTest( data, 0L, 0L, 0L ) )
      {
         dfile4registerLocked( data, -2L, 1 ) ;
         #ifndef S4SERVER
            if ( c4->lockAttempts == WAIT4EVER )
               return error4( c4, e4lock, E81523 ) ;
         #endif
         return r4locked ;
      }

      #ifdef S4CLIPPER
         if ( c4->largeFileOffset == 0 )
            rc = file4lockInternal( &data->file, L4LOCK_POS, 0, L4LOCK_POS, 0 ) ;
         else
            return error4( c4, e4notSupported, E91102 ) ;
      #endif
      #ifdef S4MDX
         if ( c4->largeFileOffset == 0 )
            rc = file4lockInternal( &data->file, L4LOCK_POS_OLD, 0, L4LOCK_POS - L4LOCK_POS_OLD + 1, 0 ) ;
         else
            return error4( c4, e4notSupported, E91102 ) ;
      #endif
      #ifdef S4FOX
         /* codebase locks the append byte as well... */
         if ( c4->largeFileOffset == 0 )
            rc = file4lockInternal( &data->file, L4LOCK_POS_OLD, 0, L4LOCK_POS_OLD - 1L, 0 ) ;
         else
            rc = file4lockInternal( &data->file, 0, c4->largeFileOffset, ULONG_MAX, 0 ) ;
      #endif
      if ( rc != 0 )
      {
         if ( rc == r4locked )
            dfile4registerLocked( data, -1L, 0 ) ;
         return rc ;
      }
      data->fileClientLock = clientId ;
      data->fileServerLock = serverId ;

      #ifndef S4OPTIMIZE_OFF
         file4refresh( &data->file ) ;   /* make sure all up to date */
      #endif
      return 0 ;
}


int S4FUNCTION dfile4lockIndex( DATA4FILE *data, const long serverId )
{
   #ifndef S4INDEX_OFF
      int rc, oldAttempts, count ;
      #ifdef S4CLIPPER
         TAG4FILE *tagOn ;
      #else
         INDEX4FILE *indexOn ;
      #endif
      CODE4 *c4 ;

      #ifdef E4PARM_LOW
         if ( data == 0 || serverId == 0 )
            return error4( 0, e4parm, E91102 ) ;
      #endif

      c4 = data->c4 ;

      if ( error4code( c4 ) < 0 )
         return e4codeBase ;

      #ifdef S4CLIPPER
         if ( data->indexLocked == 1 )
            return 0 ;
      #endif

      count = oldAttempts = c4->lockAttempts ;  /* take care of wait here */
      c4->lockAttempts = 1 ;

      rc = 0 ;
      for(;;)
      {
         #ifdef S4CLIPPER
            for ( tagOn = 0 ;; )
            {
               tagOn = dfile4tagNext( data, tagOn ) ;
               if ( tagOn == 0 )
                  break ;
               rc = tfile4lock( tagOn, serverId ) ;
               if ( rc != 0 )
                  break ;
            }
         #else
            for ( indexOn = 0 ;; )
            {
               indexOn = (INDEX4FILE *)l4next( &data->indexes, indexOn ) ;
               if ( indexOn == 0 )
                  break ;
               rc = index4lock( indexOn, serverId ) ;
               if ( rc != 0 )
                  break ;
            }
         #endif

         if ( rc == 0 )
            break ;

         #ifdef S4CLIPPER
            for ( tagOn = 0 ;; )
            {
               tagOn = dfile4tagNext( data, tagOn ) ;
               if ( tagOn == 0 )
                  break ;
               if ( tfile4unlock( tagOn, serverId ) < 0 )
                  rc = -1 ;
            }
         #else
            for ( indexOn = 0 ;; )
            {
               indexOn = (INDEX4FILE *)l4next( &data->indexes, indexOn ) ;
               if ( indexOn == 0 )
                  break ;
               if ( index4unlock( indexOn, serverId ) < 0 )
                  rc = -1 ;
            }
         #endif

         if ( rc != r4locked )
            break ;

         if ( count == 0 || rc == -1 )
            break ;

         if ( count > 0 )
            count-- ;

         #ifdef S4TEMP
            if ( d4display_quit( &display ) )
               return error4( c4, e4result, E80604 ) ;
         #endif

         u4delayHundredth( c4->lockDelay ) ;   /* wait a second & try lock again */
      }

      #ifdef S4CLIPPER
         data->indexLocked = 1 ;
      #endif
      c4->lockAttempts = oldAttempts ;
      if ( error4code( c4 ) < 0 )
         return -1 ;

      return rc ;
   #else
      return 0 ;
   #endif
}

int dfile4lockAll( DATA4FILE *data, const long clientId, const long serverId )
{
   int rc, saveUnlockAuto ;

   #ifdef E4PARM_LOW
      if ( data == 0 )
         return error4( 0, e4parm_null, E91102 ) ;
   #endif

   if ( error4code( data->c4 ) < 0 )
      return e4codeBase ;

   rc = dfile4lockFile( data, clientId, serverId ) ;

   #ifndef S4CLIPPER
      #ifndef S4MEMO_OFF
         if ( !rc )
            if ( data->nFieldsMemo > 0 )
               return dfile4lockMemo( data ) ;
      #endif
   #endif

   #ifndef S4INDEX_OFF
      if ( !rc )
         rc = dfile4lockIndex( data, serverId ) ;
   #endif

   if ( rc && code4unlockAuto( data->c4 ) == 1 )
   {
      saveUnlockAuto = code4unlockAuto( data->c4 ) ;
      code4unlockAutoSet( data->c4, 1 ) ;
      dfile4unlockData( data, clientId, serverId ) ;
      #ifndef S4CLIPPER
         #ifndef S4MEMO_OFF
            dfile4memoUnlock( data ) ;
         #endif
      #endif
      #ifndef S4INDEX_OFF
         dfile4unlockIndex( data, serverId ) ;
      #endif

      code4unlockAutoSet( data->c4, saveUnlockAuto ) ;
   }

   return rc ;
}

/* if rec == -1, then any lock is checked */
/* for client, if clientId == 0, then any d4 with a lock will return success */
/* for server, if clientId == 0 and serverId == 0, then any d4 with a lock will return success */
/* for server, if clientId == 0  then any d4 with a serverId lock will return success */
int dfile4lockTest( DATA4FILE *data, const long clientId, const long serverId, const long rec )
{
   int rc ;
      LOCK4 *lock ;
      #ifdef E4MISC
         long recSave ;
      #endif

   #ifdef E4PARM_LOW
      if ( data == 0 )
         return error4( 0, e4parm_null, E91102 ) ;
   #endif

   rc = dfile4lockTestFile( data, clientId, serverId ) ;
   if ( rc )
      return rc ;

      #ifdef E4MISC
         /* verify the order of the list */
         lock = (LOCK4 *)single4initIterate( &data->lockedRecords ) ;
         if ( lock != 0 )
            for ( ;; )
            {
               recSave = lock->id.recNum ;
               lock = (LOCK4 *)single4next( &lock->link ) ;
               if ( lock == 0 )
                  break ;
               if ( lock->id.recNum <= recSave )
                  return error4( data->c4, e4info, E91102 ) ;
            }
      #endif

      if ( clientId == 0 )
      {
         if ( data->fileServerLock != serverId )
            return 1 ;

         for ( lock = (LOCK4 *)single4initIterate( &data->lockedRecords ) ;; )
         {
            if ( lock == 0 )
               break ;
            if ( lock->id.recNum == rec || rec == 0L || rec == -1L )
               if ( lock->id.serverId == serverId || serverId == 0 )
               {
                  data->tempServerLock = lock->id.serverId ;
                  data->tempClientLock = lock->id.clientId ;
                  return 1 ;
               }

            if ( rec != 0 && rec != -1 )
               if ( lock->id.recNum > rec )   /* ordered list, so gone too far */
                  break ;
            lock = (LOCK4 *)single4next( &lock->link ) ;
         }
      }
      else
      {
         if ( data->fileServerLock == serverId && data->fileClientLock == clientId )
         {
            data->tempServerLock = data->fileServerLock ;
            data->tempClientLock = data->fileClientLock ;
            return 1 ;
         }

         for ( lock = (LOCK4 *)single4initIterate( &data->lockedRecords ) ;; )
         {
            if ( lock == 0 )
               break ;
            if ( lock->id.recNum == rec || rec == 0L || rec == -1L )
               if ( serverId == 0 || ( lock->id.clientId == clientId && lock->id.serverId == serverId ) )
               {
                  data->tempServerLock = lock->id.serverId ;
                  data->tempClientLock = lock->id.clientId ;
                  return 1 ;
               }

            if ( rec != 0 && rec != -1L )
               if ( lock->id.recNum > rec )   /* ordered list, so gone too far */
                  break ;
            lock = (LOCK4 *)single4next( &lock->link ) ;
         }
      }
      return 0 ;
}


int dfile4lockTestIndex( DATA4FILE *data, const long serverId )
{
   #ifndef S4INDEX_OFF
      #ifdef S4CLIPPER
         TAG4FILE *tagOn ;
      #else
         INDEX4FILE *indexOn ;
      #endif

      #ifdef E4PARM_LOW
         if ( data == 0 )
            return error4( 0, e4parm_null, E91102 ) ;
      #endif

      #ifdef S4LOCK_MATCH
         if ( data->file.accessMode != OPEN4DENY_NONE )
            return 1 ;
      #endif

      #ifdef S4SERVER
         if ( data->exclusiveOpen != 0 )
            return 1 ;
      #endif

      #ifdef S4CLIPPER
         for( tagOn = 0 ; ; )
         {
            tagOn = (TAG4FILE *)l4next( &data->tagfiles, tagOn ) ;
            if ( !tagOn )
               break ;
            if ( serverId == 0 && tagOn->fileLocked != 0 )
               return 1 ;
            if ( tagOn->fileLocked != serverId )
               return 0 ;
         }
      #else
         for( indexOn = 0 ; ; )
         {
            indexOn = (INDEX4FILE *)l4next( &data->indexes, indexOn ) ;
            if ( !indexOn )
               break ;
            if ( serverId == 0 && indexOn->fileLocked != 0 )
               return 1 ;
            if ( indexOn->fileLocked != serverId )
               return 0 ;
         }
      #endif
   #endif
   return 1 ;
}

#endif /* S4SINGLE */

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION dfile4lockTestAppend( DATA4FILE *data, const long clientId, const long serverId )
{
   #ifdef S4SINGLE
      return 1 ;
   #else
         int rc ;

         if ( data == 0 )
            return error4( 0, e4parm_null, E91102 ) ;

         rc = dfile4lockTestFile( data, clientId, serverId ) ;
         if ( rc )
            return rc ;

         if ( serverId == 0 )
            return ( data->appendServerLock != 0 ) ;

         if ( clientId == 0 )
            return ( data->appendServerLock == serverId ) ;
         else
            return ( data->appendServerLock == serverId && data->appendClientLock == clientId ) ;
   #endif /* S4SINGLE */
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION dfile4lockTestFile( DATA4FILE *data, const long clientId, const long serverId )
{
   #ifdef S4SINGLE
      return 1 ;
   #else
         if ( data == 0 )
            return error4( 0, e4parm_null, E91102 ) ;

         #ifdef S4LOCK_MATCH
            if ( data->file.accessMode != OPEN4DENY_NONE )
               return 1 ;
         #endif

         #ifdef S4SERVER
            if ( data->exclusiveOpen != 0 )
               /* 05/30/96 AS --> verify that the server id matches for the data which has exclusive open */
               /*           return 1 ; */
               if ( data->exclusiveOpen->serverId == serverId )
                  return 1 ;
               else
                  if ( serverId == 0 )  /* set the lockIds */
                  {
                     data->tempServerLock = data->exclusiveOpen->serverId ;
                     data->tempClientLock = data->exclusiveOpen->clientId ;
                     return 1 ;
                  }
                  else
                     return 0 ;
         #else
            if ( data->file.lowAccessMode != OPEN4DENY_NONE )
               return 1 ;
         #endif

         if ( serverId == 0 )
         {
            if ( data->fileServerLock != 0 )
            {
               data->tempServerLock = data->fileServerLock ;
               data->tempClientLock = data->fileClientLock ;
               return 1 ;
            }
            return 0 ;
         }

         if ( clientId == 0 )
         {
            if ( data->fileServerLock != serverId )
            {
               data->tempServerLock = data->fileServerLock ;
               data->tempClientLock = data->fileClientLock ;
               return 0 ;
            }
            return 1 ;
         }
         else
            return ( data->fileServerLock == serverId && data->fileClientLock == clientId ) ;
   #endif /* S4SINGLE */
}
