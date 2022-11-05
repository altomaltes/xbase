/* df4unlok.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4OFF_MULTI

#ifndef S4CLIPPER
#ifndef S4MEMO_OFF
int dfile4memoUnlock( DATA4FILE *data )
{
   #ifdef E4PARM_LOW
      if ( data == 0 )
         return error4( 0, e4parm_null, E91102 ) ;
   #endif

   if ( code4unlockAuto( data->c4 ) == LOCK4OFF )
      return 0 ;

   if ( data->memoFile.file.hand != INVALID4HANDLE )
      return memo4fileUnlock( &data->memoFile ) ;
   else
      return 0 ;
}
#endif

int dfile4unlockAppend( DATA4FILE *data, const long clientId, const long serverId )
{
   #ifndef S4OFF_MULTI
         int rc ;

      #ifdef E4PARM_LOW
         if ( data == 0 || serverId == 0 )
            return error4( 0, e4parm_null, E91102 ) ;
      #endif
      if ( code4unlockAuto( data->c4 ) == LOCK4OFF )
         return 0 ;

      if ( data->appendServerLock == serverId && ( clientId == 0 || clientId == data->appendClientLock ) )
      {
         #ifndef S4OFF_WRITE
            #ifdef S4OFF_MULTI
               dfile4updateHeader( data, 1, 1 ) ;
            #else
               if ( data->file.lowAccessMode == OPEN4DENY_RW )
                  dfile4updateHeader( data, 1, 1 ) ;
            #endif
            if ( data->doDate == 1 )
            {
               u4yymmdd( &data->yy ) ;
               data->doDate = 0 ;
            }
         #endif

            if ( data->c4->largeFileOffset == 0 )
            {
               #ifdef S4FOX
                  if ( ( data->hasMdxMemo & 0x01 ) || data->version == 0x30 )
                     rc = file4unlockInternal( &data->file, L4LOCK_POS, 0, 1L, 0 ) ;
                  else
                     rc = file4unlockInternal( &data->file, L4LOCK_POS_OLD, 0, 1L, 0 ) ;
               #else
                  rc = file4unlockInternal( &data->file, L4LOCK_POS, 0, 1L, 0 ) ;
               #endif
            }
            else
               rc = file4unlockInternal( &data->file, 0, data->c4->largeFileOffset, 1L, 0 ) ;
            if ( rc < 0 )
               return error4stack( data->c4, (short)rc, E91102 ) ;
         data->appendServerLock = 0 ;
         data->appendClientLock = 0 ;
         data->numRecs = -1 ;
      }
      if ( error4code( data->c4 ) < 0 )
         return error4code( data->c4 ) ;
   #endif
   return 0 ;
}

int dfile4unlockFile( DATA4FILE *data, const long clientId, const long serverId )
{
   #ifndef S4OFF_MULTI
      CODE4 *c4 ;
      #ifdef E4PARM_LOW
         if ( data == 0 || serverId == 0 )
            return error4( 0, e4parm, E91102 ) ;
      #endif

      c4 = data->c4 ;

      if ( code4unlockAuto( c4 ) == LOCK4OFF )
         return 0 ;

      #ifdef S4CLIENT
         data->fileLock =  0 ;
         data->numRecs = -1 ;
      #else
         if ( data->fileServerLock == serverId && ( clientId == 0 || clientId == data->fileClientLock ) )
         {
            #ifndef S4OFF_WRITE
               #ifdef S4OFF_MULTI
                  dfile4updateHeader( data, 1, 1 ) ;
               #else
                  if ( data->file.lowAccessMode == OPEN4DENY_RW )
                     dfile4updateHeader( data, 1, 1 ) ;
               #endif
               if ( data->doDate == 1 )
               {
                  u4yymmdd( &data->yy ) ;
                  data->doDate = 0 ;
               }
            #endif

            #ifdef S4CLIPPER
               if ( file4unlockInternal( &data->file, L4LOCK_POS, 0, L4LOCK_POS, 0 ) < 0 )
                  return -1 ;
            #endif
            #ifdef S4MDX
               if ( file4unlockInternal( &data->file, L4LOCK_POS_OLD, 0, L4LOCK_POS - L4LOCK_POS_OLD + 1, 0 ) < 0 )
                  return -1 ;
            #endif
            #ifdef S4FOX
               /* codebase locks the append byte as well... */
               if ( c4->largeFileOffset == 0 )
               {
                  if ( file4unlockInternal( &data->file, L4LOCK_POS_OLD, 0, L4LOCK_POS_OLD - 1L, 0 ) < 0 )
                     return -1 ;
               }
               else
                  if ( file4unlockInternal( &data->file, 0, c4->largeFileOffset, ULONG_MAX, 0 ) < 0 )
                     return -1 ;
            #endif
            data->fileServerLock =  0 ;
            data->fileClientLock =  0 ;
            data->numRecs = -1 ;
         }
      #endif
      if ( error4code( c4 ) < 0 )
         return error4code( c4 ) ;
   #endif
   return 0 ;
}

#ifndef S4INDEX_OFF
int S4FUNCTION dfile4unlockIndex( DATA4FILE *data, const long serverId )
{
   #ifdef S4OFF_MULTI
      return 0 ;
   #else
      int rc ;
      #ifdef S4CLIPPER
         TAG4FILE *tagOn ;
      #else
         INDEX4FILE *indexOn ;
      #endif

      #ifdef E4PARM_LOW
         if ( data == 0 || serverId == 0 )
            return error4( 0, e4parm, E91102 ) ;
      #endif

      #ifdef S4CLIPPER
         if ( data->indexLocked == 0 )
            return 0 ;
      #endif

      /* if the data file is locked, then delay unlocking index if lock off */
      if ( code4unlockAuto( data->c4 ) == LOCK4OFF )
         if ( data->fileServerLock == serverId )
             return 0 ;

      #ifdef S4CLIPPER
         data->indexLocked = 0 ;

         for ( tagOn = 0 ;; )
         {
            tagOn = dfile4tagNext( data, tagOn ) ;
            if ( tagOn == 0 )
            {
               rc = error4code( data->c4 ) ;
               if ( rc < 0 )
                  return rc ;
               return 0 ;
            }
            rc = tfile4unlock( tagOn, serverId ) ;
            if ( rc < 0 )
               return rc ;
         }
      #else
         for ( indexOn = 0 ;; )
         {
            indexOn = (INDEX4FILE *)l4next(&data->indexes,indexOn) ;
            if ( indexOn == 0 )
            {
               rc = error4code( data->c4 ) ;
               if ( rc < 0 )
                  return rc ;
               return 0 ;
            }
            index4unlock( indexOn, serverId ) ;
         }
      #endif
   #endif
}
#endif

static int dfile4unlockRecordDo( DATA4FILE *data, long rec )
{
   FILE4LONG position ;
   CODE4 *c4 ;
   #ifdef S4FOX
      FILE4LONG pos2 ;
    #endif

   c4 = data->c4 ;

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
   #endif
   if ( c4->largeFileOffset == 0 )
   {
      if ( file4unlockInternal( &data->file, file4longGetLo( position ), 0, 1L, 0 ) < 0 )
         return -1 ;
   }
   else
      if ( file4unlockInternal( &data->file, rec, c4->largeFileOffset, 1L, 0 ) < 0 )
         return -1 ;
   return 0 ;
}

int S4FUNCTION dfile4unlockRecord( DATA4FILE *data, const long clientId, const long serverId, const long rec )
{
      LOCK4 *lock ;
      SINGLE4DISTANT singleDistant ;
      single4distantInitIterate( &singleDistant, &data->lockedRecords ) ;

      for ( ;; )
      {
         lock = (LOCK4 *)single4distantToItem( &singleDistant ) ;
         if ( lock == 0 )
            break ;
         if ( lock->id.serverId == serverId && ( clientId == 0 || lock->id.clientId == clientId ) && lock->id.recNum == rec )
         {
            if ( dfile4unlockRecordDo( data, rec ) < 0 )
               return -1 ;
            single4distantPop( &singleDistant ) ;
            mem4free( data->c4->lockMemory, lock ) ;
            break ;
         }
         single4distantNext( &singleDistant ) ;
      }

   return 0 ;
}

int dfile4unlockRecords( DATA4FILE *data, const long clientId, const long serverId )
{
   #ifndef S4OFF_MULTI
      LOCK4 *lock ;
      SINGLE4DISTANT singleDistant ;

      #ifdef E4PARM_LOW
         if ( data == 0 || serverId == 0 )
            return error4( 0, e4parm, E91102 ) ;
      #endif

      if ( code4unlockAuto( data->c4 ) == LOCK4OFF )
         return 0 ;

      single4distantInitIterate( &singleDistant, &data->lockedRecords ) ;

      for ( ;; )
      {
         lock = (LOCK4 *)single4distantToItem( &singleDistant ) ;
         if ( lock == 0 )
            break ;

         if ( lock->id.serverId == serverId && ( clientId == 0 || lock->id.clientId == clientId ) )
         {
               if ( dfile4unlockRecordDo( data, lock->id.recNum ) < 0 )
                  return -1 ;
            single4distantPop( &singleDistant ) ;
            mem4free( data->c4->lockMemory, lock ) ;
         }
         else
            single4distantNext( &singleDistant ) ;
      }
   #endif
   return 0 ;
}

int dfile4unlockData( DATA4FILE *data, const long clientId, const long serverId )
{
   #ifndef S4OFF_MULTI
      #ifdef E4PARM_LOW
         if ( data == 0 )
            return error4( 0, e4parm_null, E91102 ) ;
      #endif
      if ( code4unlockAuto( data->c4 ) == LOCK4OFF )
         return 0 ;

      dfile4unlockFile( data, clientId, serverId ) ;
      dfile4unlockAppend( data, clientId, serverId ) ;
      dfile4unlockRecords( data, clientId, serverId ) ;
      if ( error4code( data->c4 ) < 0 )
         return error4code( data->c4 ) ;
   #endif
   return 0 ;
}

#endif  /* S4CLIENT */
#endif  /* S4OFF_MULTI */
