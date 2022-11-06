/* f4close.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifdef S4TEMP
   #include "t4test.h"
#endif

#ifndef S4OFF_MULTI
#ifdef S4WINTEL
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
#endif

/*#include <fcntl.h>*/

#ifdef S4TRACK_FILES_OR_SERVER
   unsigned int numFiles5 ;
   #ifdef S4TRACK_FILES
      int f4print = 0 ; /* if f4print == 0 then stdout, else stdprn, no output if -1 */
      #ifndef S4TESTING
         extern FILE4 s4test ;
         void S4FUNCTION u4writeErr( const char * err_str, int newLine )
         {
            int errCode, flushCode ;
            long len ;

            if (s4test.hand != INVALID4HANDLE && err_str != 0 )
            {
               errCode = error4code( s4test.codeBase ) ;
               error4set( s4test.codeBase, 0 ) ;

               len = file4len( &s4test ) ;
               if( len >= 0 )
               {
                  if ( newLine == 1 )
                  {
                     file4write( &s4test, len, "\r\n", 2 ) ;
                     len += 2 ;
                  }
                  flushCode = s4test.codeBase->fileFlush ;
                  s4test.codeBase->fileFlush = 1 ;
                  file4lenSet( &s4test, len + strlen(err_str) ) ;
                  file4write( &s4test, len, (void *)err_str, strlen(err_str) ) ;
                  file4flush( &s4test ) ;
                  s4test.codeBase->fileFlush = flushCode ;
               }

               error4set( s4test.codeBase, errCode ) ;
            }
         }
      #endif  /* S4TESTING */
   #endif  /* S4TRACK_FILES */
#endif  /* S4TRACK_FILES_OR_SERVER */

int S4FUNCTION file4close( FILE4 *file )
{
   int rc ;
   CODE4 *c4 ;
   #ifndef S4OPTIMIZE_OFF
      FILE4LONG len ;
   #endif

   #ifdef E4PARM_HIGH
      if ( file == 0 )
         return error4( 0, e4parm_null, E90601 ) ;
   #endif

   if ( file->hand == INVALID4HANDLE )
      return 0 ;

   c4 = file->codeBase ;

   #ifndef S4OPTIMIZE_OFF
      if ( file->fileCreated == 0 )
      {
         file4longAssign( len, 0, 0 ) ;
         file4lenSetLow( file, len ) ;
         file4optimize( file, 0, 0 ) ;
      }
      else
      {
         file4optimize( file, 0, 0 ) ;
   #endif

   #ifdef S4ADVANCE_READ
      /* just cancel any oustanding reads */
      if ( l4numNodes( &file->advanceReadFileList ) != 0 )
         file4advanceCancel( file ) ;
      if ( file->advanceReadBuf != 0 )
      {
         u4free( file->advanceReadBuf ) ;
         file->advanceReadBuf = 0 ;
      }
   #endif

   #ifdef S4WRITE_DELAY
      /* ensure that there are no outstanding file-writes delaying */
      if ( l4numNodes( &file->delayWriteFileList ) != 0 )
         file4writeDelayFlush( file, (file->isTemp ? 0 : 1 ) ) ;
   #endif

   #ifdef S4MULTI_THREAD
      DeleteCriticalSection( &file->critical4file ) ;
   #endif

   #ifdef __WIN32
      rc = (int) CloseHandle( (HANDLE)file->hand ) ;
   #else
      #ifdef S4WINDOWS
         rc = _lclose( file->hand ) ;
      #else
         #ifdef S4MACINTOSH
            rc = FSClose( file->hand ) ;
         #else
            rc = close( file->hand ) ;
         #endif
      #endif
   #endif

      if ( rc < 0 )
      {
         if ( file->name == 0 )
            return error4( c4, e4close, E90601 ) ;
         else
            return error4describe( c4, e4close, E90601, file->name, 0, 0 ) ;
      }

      #ifndef S4OPTIMIZE_OFF
         if ( file->fileCreated == 1 )
      #endif
         if ( file->isTemp )
            #ifndef S4MACINTOSH
               u4remove( file->name ) ;
            #else
               FSpDelete( &file->macSpec) ;
            else
               if (FlushVol( 0, file->macSpec.vRefNum ) != 0 )
                  return error4( c4, e4optFlush, E90601 ) ;
            #endif

   #ifndef S4OPTIMIZE_OFF
      }
   #endif

   if ( file->doAllocFree )
   {
      u4free( file->nameBuf ) ;
      file->name = 0 ;
   }

   memset( (void *)file, 0, sizeof( FILE4 ) ) ;
   file->hand = INVALID4HANDLE ;

   #ifdef S4TRACK_FILES_OR_SERVER
      numFiles5-- ;
      #ifdef S4TRACK_FILES
         if ( f4print != -1 )
         {
             #ifdef S4WINDOWS
                #ifdef S4TESTING
                   if ( mem4displayPtr == 0 )
                      error4( c4, e4info, E50101 ) ;
                   d4display_str( mem4displayPtr, "\r\nfile closed: ", 1 ) ;
                   d4display_str( mem4displayPtr, f4print, file->name ) ;
                #else
                   u4writeErr( "file closed: ", 1 ) ;
                   u4writeErr( file->name, 0 ) ;
                #endif
             #else
                if ( f4print )
                   fprintf( stdprn, "\r\nfile closed: %s", file->name ) ;
                else
                   printf( "\r\nfile closed: %s", file->name ) ;
             #endif
          }
      #endif
   #endif

   if ( c4 != 0 )
         if ( error4code( c4 ) < 0 )
            return -1 ;

   return 0 ;
}
