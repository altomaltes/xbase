/* f4write.c (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifdef S4TEMP
   #include "t4test.h"
#endif

#ifdef S4WINTEL
   #ifndef S4OFF_MULTI
      #ifndef S4IBMOS2
         #ifndef __TURBOC__
            #include <sys\locking.h>
            #define S4LOCKING
         #endif
         #ifdef _MSC_VER
            #include <sys\types.h>
            #include <sys\locking.h>
         #endif
      #endif
   #endif
   #ifndef S4OFF_OPTIMIZE
      #ifdef E4ANALYZE_ALL
         #include <sys\stat.h>
         #include <share.h>
         #include <fcntl.h>
      #endif
   #endif

/*   #include <sys\stat.h>*/
/*   #include <share.h>*/
#endif

/*#include <fcntl.h>*/

/* returns the length written */
static unsigned file4writeLowDo( FILE4 *f4, FILE4LONG pos, const void *ptr, unsigned len )
{
   unsigned long urc ;
   #ifdef S4MACINTOSH
      long longPtrLen ;
   #endif

   #ifdef S4MULTI_THREAD
      EnterCriticalSection( &f4->critical4file ) ;
   #endif

   #ifdef __WIN32
      if ( SetFilePointer( f4->hand, file4longGetLo( pos ), file4longGetHiAddress( pos ), FILE_BEGIN ) == (DWORD)-1 )
   #else
      #ifdef S4MACINTOSH
         if ( MAClseek( f4->hand, pos, 0, 1 ) != pos )
      #else
         #ifdef S4WINDOWS
            if ( _llseek( f4->hand, pos, 0 ) != pos )
         #else
            #ifdef S4LSEEK
               if ( f4lseek( f4->hand, pos, 0, 1 ) != pos )
            #else
               if ( lseek( f4->hand, pos, 0 ) != pos )
            #endif
         #endif
      #endif
   #endif
   {
      #ifdef S4MULTI_THREAD
         LeaveCriticalSection( &f4->critical4file ) ;
      #endif
      error4describe( f4->codeBase, e4write, E90615, f4->name, 0, 0 ) ;
      return 0 ;
   }

   #ifdef __WIN32
      WriteFile( f4->hand, ptr, len, &urc, NULL ) ;
      len = urc ;
   #else
      #ifdef S4MACINTOSH
         longPtrLen = (long)len ;
         urc = FSWrite( f4->hand, &longPtrLen, ptr ) ;
         if ( urc != 0 )  /* otherwise, len all was written, so leave as is */
            len = urc ;
      #else
         #ifdef S4WINDOWS
            len = (unsigned)_lwrite( f4->hand, (char *) ptr, len ) ;
         #else
            len = (unsigned)write( f4->hand, (char *)ptr, len ) ;
         #endif
      #endif
   #endif

   #ifdef S4MULTI_THREAD
      LeaveCriticalSection( &f4->critical4file ) ;
   #endif

   return len ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
/*AS 06/20/97 doFlush paramater added otherwise delay write getting in endless loop */
int file4writeLow( FILE4 *f4, FILE4LONG pos, const void *ptr, unsigned len, const int checkDelayList, const int checkAdvanceList, const int doFlush )
{
   unsigned urc ;
   CODE4 *c4 ;
   #ifdef S4WRITE_DELAY
      FILE4WRITE_DELAY *writeDelay ;
      LINK4 *delayLink ;
      FILE4LONG tLong ;
      unsigned posShort ;
   #endif

   c4 = f4->codeBase ;

   if ( f4->isReadOnly )
      return error4( c4, e4write, E80606 ) ;

   #ifdef S4ADVANCE_READ
      /* take care of over-writing advance-read buffers where appropriate */
      if ( c4->advanceReadsEnabled
         #ifdef S4FILE_EXTENDED
            && ( f4->isLong == 0 ) && file4longGetHi( pos ) == 0
         #endif
         )
         file4advanceReadWriteOver( f4, file4longGetLo( pos ), len, ptr, 1 ) ;
   #endif

   #ifdef S4MULTI_THREAD
      EnterCriticalSection( &f4->critical4file ) ;
   #endif

   /* now flush the transaction file if a data file */
   #ifndef S4OFF_TRAN
      #ifndef S4OFF_OPTIMIZE
         if ( f4->type == OPT4DBF && (code4trans( c4 ))->c4trans->transFile != NULL )
            tran4fileFlush( (code4trans( c4 ))->c4trans->transFile ) ;
      #endif
   #endif

   #ifdef S4WRITE_DELAY
      if ( checkDelayList /* check to see if write request covers delayed areas */
         #ifdef S4FILE_EXTENDED
            && f4->isLong == 0
         #endif
         )
         if ( l4numNodes( &f4->delayWriteFileList ) != 0 )
         {
            #ifdef E4ANALYZE
               if ( file4longGetHi( pos ) != 0 )
                  return error4( c4, e4result, E80606 ) ;
            #endif
            posShort = file4longGetLo( pos ) ;
            for ( delayLink = (LINK4 *)l4first( &f4->delayWriteFileList ) ;; )
            {
               if ( delayLink == 0 )
                  break ;
               writeDelay = (FILE4WRITE_DELAY *)(delayLink - 1 ) ;
               delayLink = (LINK4 *)l4next( &f4->delayWriteFileList, delayLink ) ;

               if ( posShort >= ( writeDelay->pos + (long)writeDelay->len ) )  /* outside of block */
                  continue ;
               if ( ( posShort + (long)len ) <= writeDelay->pos )  /* outside of block */
                  continue ;

               /* now, if the delay piece belongs in the buffer, then read all
                  the info before the delay piece, copy the delay piece over,
                  and read all the info after the delay piece */
               while ( writeDelay->usageFlag == r4inUse )  /* is being written to disk, just wait until it is done... */
                  Sleep( 0 ) ;

               if ( writeDelay->usageFlag == r4finished ) /* is written to disk, so can just ignore */
                  continue ;

               /* if the entire block is within the range, then can just delete
                  it, otherwise must write it now (note that, due to constant
                  contents of the delay-write buffer, cannot copy into it
                  directly */

               if ( ( writeDelay->pos >= posShort ) && ( posShort + len >= writeDelay->pos + writeDelay->len ) ) /* remove */
                  writeDelay->status = 0 ;
               else /* otherwise must write the block now, and then continue */
               {
                  file4longAssign( tLong, writeDelay->pos, 0 ) ;
                  writeDelay->status = file4writeLow( writeDelay->file, tLong, writeDelay->data, writeDelay->len, 0, 1, 0 ) ;
               }

               writeDelay->usageFlag = r4finished ;  /* outside of critical section, to allow a wait for completion while keeping the critical section */
               l4remove( &c4->delayWriteList, writeDelay ) ;
               l4remove( &f4->delayWriteFileList, &writeDelay->fileLink ) ;
               writeDelay->completionRoutine( writeDelay ) ;
               mem4free( c4->delayWriteMemory, writeDelay ) ;
            }
         }
   #endif

   urc = file4writeLowDo( f4, pos, ptr, len ) ;

   #ifdef S4MULTI_THREAD
      LeaveCriticalSection( &f4->critical4file ) ;
   #endif

   if ( urc != len )
      return error4describe( c4, e4write, E90615, f4->name, 0, 0 ) ;

   #ifndef S4OFF_MULTI
      /* doFlush is false in write-delay scenario where this is not relevant (i.e. gets
         flushed by non-delay thread) */
      if ( f4->codeBase->fileFlush && doFlush )
         file4flush( f4 ) ;
   #endif

   return 0 ;
}

#ifndef S4OFF_OPTIMIZE
#ifdef P4ARGS_USED
   #pragma argsused
#endif
#ifdef S4WRITE_DELAY
int file4writeOpt( FILE4 *f4, unsigned long pos, const void *ptr, const unsigned len, int doDelay, S4DELAY_FUNCTION *completionRoutine, void *completionData )
#else
int file4writeOpt( FILE4 *f4, unsigned long pos, const void *ptr, const unsigned len, int doDelay, void *completionRoutine, void *completionData )
#endif
{
   int rc ;
   CODE4 *c4 ;
   FILE4LONG tLong ;
   #ifndef S4OFF_OPTIMIZE
      unsigned urc ;
      #ifdef E4ANALYZE_ALL
         char buf[512] ;
         long bufWritePos, bufWriteLen ;
      #endif
      #ifdef S4OPTIMIZE_STATS
         DATA4 *stat ;
      #endif
   #endif

   c4 = f4->codeBase ;
   rc = 0 ;

   if ( f4->doBuffer )
   {
      #ifdef S4OPTIMIZE_STATS
         stat = c4->statusDbf ;
         if ( stat != 0 )  /* track stats */
         {
            if ( f4 != &stat->dataFile->file )  /* don't do for the stat file! */
            {
               if ( d4appendStart( stat, 0 ) == 0 )
               {
                  f4assignChar( c4->typeFld, 'W' ) ;  /* high-level */
                  f4assign( c4->fileNameFld, f4->name ) ;
                  f4assignLong( c4->offsetFld, pos ) ;
                  f4assignLong( c4->lengthFld, len ) ;
                  d4append( stat ) ;
               }
            }
         }
      #endif

      urc = (unsigned int)opt4fileWrite( f4, pos, len, ptr, f4->bufferWrites ) ;
      if ( urc != len )
         return error4describe( c4, e4write, E90619, f4->name, 0, 0 ) ;
   }

   if ( f4->doBuffer == 0 || f4->writeBuffer == 0 || f4->bufferWrites == 0 )
   {
      if ( f4->fileCreated == 0 )
      {
         c4->opt.forceCurrent = 1 ;
            file4tempLow( f4, c4, 1, 1, NULL ) ;
         c4->opt.forceCurrent = 0 ;
      }

      #ifdef E4ANALYZE_ALL
         if ( f4->hasDup == 1 )
         {
            if ( file4longGetLo( file4lenLow( f4 ) ) < pos )
            {
               /* need to null out any extra file contents in order for file verification for optimization
                  to work properly */
               memset( buf, 0, sizeof( buf ) ) ;
               while ( file4longGetLo( file4lenLow( f4 ) ) < pos )
               {
                  bufWritePos = file4longGetLo( file4lenLow( f4 ) ) ;
                  bufWriteLen = (pos - bufWritePos) ;
                  if ( bufWriteLen > (long)sizeof( buf ) )
                     bufWriteLen = (long)sizeof( buf ) ;
                  file4longAssign( tLong, bufWritePos, 0 ) ;
                  if ( file4writeInternal( f4, tLong, buf, (int)bufWriteLen ) != 0 )
                     break ;
               }
            }
         }
      #endif  /* E4ANALYZE_ALL */

      #ifdef S4OPTIMIZE_STATS
         stat = c4->statusDbf ;
         if ( stat != 0 )  /* track stats */
         {
            if ( f4 != &stat->dataFile->file )  /* don't do for the stat file! */
            {
               if ( d4appendStart( stat, 0 ) == 0 )
               {
                  f4assignChar( c4->typeFld, 'X' ) ;  /* low-level */
                  f4assign( c4->fileNameFld, f4->name ) ;
                  f4assignLong( c4->offsetFld, pos ) ;
                  f4assignLong( c4->lengthFld, len ) ;
                  d4append( stat ) ;
               }
            }
         }
      #endif
      #ifdef S4WRITE_DELAY
         if ( doDelay == 1
            #ifdef S4FILE_EXTENDED
               && f4->isLong == 0
            #endif
            && pos == 0 )
            rc = file4writeDelay( f4, pos, ptr, len, completionRoutine, completionData ) ;
         else
      #endif
      {
         file4longAssign( tLong, pos, 0 ) ;
         rc = file4writeLow( f4, tLong, ptr, len, 1, 1, 1 ) ;
      }
   }

   #ifdef E4ANALYZE_ALL
      if ( f4->hasDup == 1 )
         if ( f4->doBuffer == 1 || f4->link.n == 0 )
            if ( file4writePart( ptr, f4, pos, urc ) != 0 )
               return error4( c4, e4opt, E80602 ) ;
   #endif

   return rc ;
}
#endif /* S4OFF_OPTIMIZE */

int S4FUNCTION file4write( FILE4 *f4, const long pos, const void *ptr, const unsigned len )
{
   FILE4LONG fpos ;

   file4longAssign( fpos, pos, 0 ) ;

   return file4writeInternal( f4, fpos, ptr, len ) ;
}

int file4writeInternal( FILE4 *f4, FILE4LONG pos, const void *ptr, unsigned len )
{
   CODE4 *c4 ;

   #ifdef E4PARM_HIGH
      if ( f4 == 0 )
         return error4( 0, e4parmNull, E90619 ) ;
      if ( file4longError( pos ) < 0 || ( ptr == 0 && len > 0 ) )
         return error4( f4->codeBase, e4parm, E90619 ) ;
      if ( f4->hand == INVALID4HANDLE )
         return error4( f4->codeBase, e4parm, E90619 ) ;
   #endif

   c4 = f4->codeBase ;

   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   if ( f4->isReadOnly )
      return error4( c4, e4write, E80606 ) ;

   #ifndef S4OFF_OPTIMIZE
      #ifdef S4ADVANCE_READ
         /* take care of over-writing advance-read buffers where appropriate */
         if ( c4->advanceReadsEnabled
            #ifdef S4FILE_EXTENDED
               && f4->isLong == 0
            #endif
            )
            file4advanceReadWriteOver( f4, file4longGetLo( pos ), len, ptr, 1 ) ;
      #endif
      #ifdef S4FILE_EXTENDED
         if ( f4->isLong == 0 && file4longGetHi( pos ) == 0 )
            return file4writeOpt( f4, file4longGetLo( pos ), ptr, len, 0, 0, 0 ) ;
         else  /* not optimized in this case */
         {
            if ( f4->doBuffer == 1 )
               file4optimize( f4, OPT4OFF, OPT4OTHER ) ;
            return file4writeLow( f4, pos, ptr, len, 1, 0, 1 ) ;
         }
      #else
         return file4writeOpt( f4, file4longGetLo( pos ), ptr, len, 0, 0, 0 ) ;
      #endif
   #else
      /* file4writeLow() itself takes care of the advance-read condition... */
      return file4writeLow( f4, pos, ptr, len, 1, 0, 1 ) ;
   #endif
}

#ifdef S4WRITE_DELAY

/* f4writed.c (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#define MEM4DELAY_START 10
#define MEM4DELAY_EXPAND 10

int file4writeDelay( FILE4 *f4, unsigned long pos, const void *data, const unsigned len, S4DELAY_FUNCTION *completionRoutine, void *completionData )
{
   FILE4WRITE_DELAY *writeDelay ;
   LINK4 *delayLink ;
   CODE4 *c4 ;
   int rc ;
   FILE4LONG tLong ;

   c4 = f4->codeBase ;

   #ifdef E4PARM_LOW
      if ( completionRoutine == 0 || c4 == 0 )
         error4( c4, e4write, E90619 ) ;
   #endif

   /* first check to see if the current delay-write over-rides an existing one */
   if ( l4numNodes( &f4->delayWriteFileList ) != 0 )
   {
      EnterCriticalSection( &c4->critical4delayWriteList ) ;

      for ( delayLink = (LINK4 *)l4first( &f4->delayWriteFileList ) ;; )
      {
         if ( delayLink == 0 )
            break ;
         writeDelay = (FILE4WRITE_DELAY *)(delayLink - 1 ) ;
         delayLink = (LINK4 *)l4next( &f4->delayWriteFileList, delayLink ) ;

         if ( pos >= ( writeDelay->pos + (long)writeDelay->len ) )  /* outside of block */
            continue ;
         if ( ( pos + (long)len ) <= writeDelay->pos )  /* outside of block */
            continue ;

         while ( writeDelay->usageFlag == r4inUse )  /* is being written to disk, just wait until it is done... */
            Sleep( 0 ) ;

         if ( writeDelay->usageFlag == r4finished ) /* is written to disk, so can just ignore */
            continue ;

         /* we have a delay-write which overlaps the current request */

         /* if the entire block is within the range, then can just delete
            it, otherwise must write it now (note that, due to constant
            contents of the delay-write buffer, cannot copy into it
            directly */

         if ( ( writeDelay->pos >= pos ) && ( pos + len >= writeDelay->pos + writeDelay->len ) ) /* remove */
            writeDelay->status = 0 ;
         else /* otherwise must write the block now, and then continue */
         {
            file4longAssign( tLong, writeDelay->pos, 0 ) ;
            writeDelay->status = file4writeLow( writeDelay->file, tLong, writeDelay->data, writeDelay->len, 0, 1, 1 ) ;
         }

         writeDelay->usageFlag = r4finished ;  /* outside of critical section, to allow a wait for completion while keeping the critical section */
         l4remove( &f4->codeBase->delayWriteList, writeDelay ) ;
         l4remove( &f4->delayWriteFileList, &writeDelay->fileLink ) ;
         writeDelay->completionRoutine( writeDelay ) ;
         mem4free( c4->delayWriteMemory, writeDelay ) ;
      }
      LeaveCriticalSection( &c4->critical4delayWriteList ) ;
   }

   if ( c4->delayWriteMemory == 0 )
      writeDelay = (FILE4WRITE_DELAY *)mem4createAlloc( c4, &c4->delayWriteMemory, MEM4DELAY_START, sizeof( FILE4WRITE_DELAY ), MEM4DELAY_EXPAND, 0 ) ;
   else
      writeDelay = (FILE4WRITE_DELAY *)mem4alloc( c4->delayWriteMemory ) ;

   if ( writeDelay == 0 )
      return error4( c4, e4memory, E90624 ) ;

   writeDelay->file = f4 ;
   writeDelay->data = (const char *)data ;
   writeDelay->len = len ;
   writeDelay->pos = pos ;
   writeDelay->usageFlag = r4queued ;
   writeDelay->completionRoutine = completionRoutine ;
   writeDelay->completionData = completionData ;

   if ( c4->delayWritesEnabled == 0 )   /* delay-writes not enabled, so just write */
   {
      file4longAssign( tLong, pos, 0 ) ;
      rc = file4writeLow( f4, tLong, data, len, 1, 1, 1 ) ;
      completionRoutine( writeDelay ) ;
      return rc ;
   }

   EnterCriticalSection( &c4->critical4delayWriteList ) ;

   l4add( &c4->delayWriteList, writeDelay ) ;
   l4add( &f4->delayWriteFileList, &writeDelay->fileLink ) ;

   LeaveCriticalSection( &c4->critical4delayWriteList ) ;

   SetEvent( c4->pendingWriteEvent ) ;  /* notify the write thread */
   Sleep( 0 ) ;

   return r4delay ;
}

/* flush out any delayed-writes for the file in question */
/* if doWrite is false, then the blocks are just dumped (eg. temp. files) */
int file4writeDelayFlush( FILE4 *file, const int doWrite )
{
   FILE4WRITE_DELAY *writeDelay ;
   LINK4 *writeDelayLink, *saved ;
   CODE4 *c4 ;
   FILE4LONG tLong ;

   c4 = file->codeBase ;

   /* by obtaining the critical4delayWriteList critical section, we can
      guarantee that the other thread will be suspended.  Therefore
      the flushes for this file will get high-priority treatment, which
      is what is desired
   */

   EnterCriticalSection( &c4->critical4delayWriteList ) ;

   /* go through the list and manually flush each one belonging to our file */

   for ( writeDelayLink = (LINK4 *)l4first( &file->delayWriteFileList ) ;; )
   {
      if ( writeDelayLink == 0 )
         break ;
      writeDelay = (FILE4WRITE_DELAY *)(writeDelayLink - 1) ;
      saved = (LINK4 *)l4next( &file->delayWriteFileList, writeDelayLink ) ;
      if ( writeDelay->usageFlag == r4queued )  /* do ourselves */
      {
         l4remove( &file->delayWriteFileList, writeDelayLink ) ;
         l4remove( &c4->delayWriteList, writeDelay ) ;
         if ( doWrite == 1 )
         {
            file4longAssign( tLong, writeDelay->pos, 0 ) ;
            writeDelay->status = file4writeLow( writeDelay->file, tLong, writeDelay->data, writeDelay->len, 0, 1, 1 ) ;
         }
         else
            writeDelay->status = 0 ;
         writeDelay->usageFlag = r4finished ;
         writeDelay->completionRoutine( writeDelay ) ;
         mem4free( c4->delayWriteMemory, writeDelay ) ;
      }
      writeDelayLink = saved ;
   }

   LeaveCriticalSection( &c4->critical4delayWriteList ) ;

   for ( ;; )
   {
      /* now verify that the checkInUse write gets completed */
      if ( l4numNodes( &file->delayWriteFileList ) == 0 )
         break ;
      #ifdef E4ANALYZE
         if ( l4numNodes( &file->delayWriteFileList ) > 1 )   /* in theory impossible, it means delay-write has 2 files writing at same time */
            return error4( c4, e4struct, E90624 ) ;
      #endif
      SetEvent( c4->pendingWriteEvent ) ;  /* notify the write thread */
      Sleep( 0 ) ;   /* give up our time slice to get the delay-write going */
   }

   return 0 ;
}

/* main function for delay-write thread (i.e. paramater to CreateThread()) */
#ifdef S4USE_INT_DELAY
   int file4writeDelayMain( void *data )
#else
   void file4writeDelayMain( void *data )
#endif
{
   CODE4 *c4 ;
   FILE4WRITE_DELAY *writeDelay ;
   FILE4LONG tLong ;

   c4 = (CODE4 *)data ;
   c4->delayWritesEnabled = 1 ;

   for ( ;; )
   {
      if ( l4numNodes( &c4->delayWriteList ) == 0 )
      {
         if ( c4->uninitializeDelayWrite == 1 )   /* shutdown */
         {
            SetEvent( c4->initUndoDelayWrite ) ;
            #ifdef S4USE_INT_DELAY
               return 0 ;
            #else
               return ;
            #endif
         }
         else
         {
            WaitForSingleObject( c4->pendingWriteEvent, INFINITE ) ;
            ResetEvent( c4->pendingWriteEvent ) ;
         }
      }
      else  /* perform a write on the first available block */
      {
         EnterCriticalSection( &c4->critical4delayWriteList ) ;
         writeDelay = (FILE4WRITE_DELAY *)l4first( &c4->delayWriteList ) ;
         if ( writeDelay == 0 )   /* maybe got removed by main thread, so none to write... */
         {
            LeaveCriticalSection( &c4->critical4delayWriteList ) ;
            Sleep( 0 ) ;
            continue ;
         }
         writeDelay->usageFlag = r4inUse ;
         LeaveCriticalSection( &c4->critical4delayWriteList ) ;

         file4longAssign( tLong, writeDelay->pos, 0 ) ;
         writeDelay->status = file4writeLow( writeDelay->file, tLong, writeDelay->data, writeDelay->len, 0, 1, 0 ) ;
         writeDelay->usageFlag = r4finished ;  /* outside of critical section, to allow a wait for completion while keeping the critical section */
         EnterCriticalSection( &c4->critical4delayWriteList ) ;
         l4remove( &c4->delayWriteList, writeDelay ) ;
         l4remove( &writeDelay->file->delayWriteFileList, &writeDelay->fileLink ) ;
         LeaveCriticalSection( &c4->critical4delayWriteList ) ;
         writeDelay->completionRoutine( writeDelay ) ;
         mem4free( c4->delayWriteMemory, writeDelay ) ;
      }
   }
}

#endif /* S4WRITE_DELAY */
