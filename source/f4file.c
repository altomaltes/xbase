/* f4file.c (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifdef __TURBOC__
   #pragma hdrstop
#endif

#ifdef S4TEMP
   #include "t4test.h"
#endif

#ifdef _MSC_VER
   #ifdef S4WINDOWS
      #ifndef S4WINCE
         #include <dos.h>
      #endif
   #endif
#endif

#ifdef S4WINTEL
   #ifndef S4IBMOS2
      #ifndef S4OFF_MULTI
         #ifndef __TURBOC__
            #include <sys\locking.h>
            #define S4LOCKING
         #endif
      #endif
      #ifdef __ZTC__
      #endif
      #ifdef _MSC_VER
         #ifndef S4WINCE
            #include <sys\types.h>
            #include <sys\locking.h>
         #endif
      #endif
   #endif

/*   #include <sys\stat.h>*/
/*   #include <share.h>*/
#endif

/*#include <fcntl.h>*/

#ifdef __SC__
   #include <dos.h>
#endif

#ifdef S4NO_FILELENGTH
   #ifdef S4MACINTOSH
      FILE4LONG u4filelength( int hand )
      {
         long fileLen ;

         if ( GetEOF(hand, &fileLen) != 0 )
            return error4( 0, e4result, E90603 ) ;

         return fileLen ;
      }
   #else
      #ifdef __WIN32
         FILE4LONG u4filelength( HANDLE hand )
         {
            FILE4LONG rc ;

            file4longSetLo( rc, (long)GetFileSize( hand, (unsigned long *)file4longGetHiAddress(rc) ) ) ;
            file4longCheckError( rc ) ;

            #ifdef E4PARM_LOW
               if ( file4longError( rc ) == (unsigned long)-1L )
               {
                  error4( 0, e4result, E90603 ) ;
                  file4longAssignError( rc ) ;
               }
            #endif

            return rc ;
         }
      #else
         #ifdef S4UNIX
            #include  <sys/types.h>
            #include  <sys/stat.h>

            FILE4LONG u4filelength( int hand )
            {
               struct stat strStat ;

               if (fstat( hand, &strStat ) )
                  return error4( 0, e4result, E90603 ) ;

               return( (long) strStat.st_size ) ;
            }
         #else
            #ifdef _MSC_VER
               /*
                  fstat() does not work correctly under Microsoft C++ 1.5 on a
                  Novell drive so lseek() is used instead.
                  On a Novell drive fstat() always returns the file length when
                  the file was opened and does not reflect appends from other
                  users.
               */
               FILE4LONG u4filelength( int hand )
               {
                  long length, current ;

                  current = lseek( hand, 0, SEEK_CUR ) ;
                  length = lseek( hand, 0, SEEK_END ) ;
                  lseek( hand, current, SEEK_SET ) ;

                  return length ;
               }
            #else
               #include  <sys\stat.h>

               FILE4LONG u4filelength( int hand )
               {
                  struct stat strStat ;

                  if (fstat( hand, &strStat ) )
                     return error4( 0, e4result, E90603 ) ;

                  return( (long) strStat.st_size ) ;
               }
            #endif
         #endif
      #endif
   #endif
#endif

#ifdef S4MACINTOSH
   long MAClseek(int hand, long offset, int fromWhere, int extend )
   {
      long fileLen ;
      if ( offset != 0 )
      {
         fileLen = u4filelength( hand ) ;

         if (extend)
         {
            if ( fileLen < offset )
               SetEOF( hand, offset ) ;
         }
         else
            if ( fileLen < offset )
            {
               if ( SetFPos(hand, fsFromStart, fileLen ) == 0 )
                  return offset ;
               else
                  return -1L ;
            }
      }

      if ( SetFPos(hand, fsFromStart, offset ) == 0 )
         return offset ;
      else
         return -1L ;
   }
#endif

#ifdef S4LSEEK
   /* if extend is set, file is extended, else lseek to EOF */
   long f4lseek(FILE4 *f4, long offset, int fromWhere, int extend )
   {
      long fileLen ;

      if ( offset != 0 )
      {
         fileLen = u4filelength( f4->hand ) ;

         if (extend)
         {
            if ( fileLen < offset )
               file4changeSize( f4, offset ) ;
         }
         else
            if ( fileLen < offset )
            {
               if ( lseek( f4->hand, fileLen, 0 ) )
                  return offset ;
               else
                  return -1L ;
            }
      }
      return lseek( f4->hand, offset, fromWhere ) ;
   }
#endif

#ifdef S4NO_CHSIZE

#define E4MAXLINE 129   /* maximum file path length */

#ifdef S4MACINTOSH
   int S4FUNCTION file4changeSize( FILE4 *f4, FILE4LONG size )
   {
      if ( SetEOF( f4->hand, size ) != 0 )
         return error4( 0, e4result, E90604 ) ;

      return 0 ;
   }
#else
   #ifdef _MSC_VER
      #ifdef S4WINDOWS
         int S4FUNCTION file4changeSize( FILE4 *f4, FILE4LONG size )
         {
            unsigned int rc, num ;
            char a ;

            a = (char)0x00 ;
            _llseek( f4->hand, size, SEEK_SET ) ;

            rc = _dos_write( f4->hand, &a, 0, &num ) ;
            if ( num != 0 || rc != 0 )
               return error4( f4->codeBase, e4lenSet, E90604 ) ;

            return 0 ;
         }
      #endif
   #else
      int S4FUNCTION file4changeSize( FILE4 *f4, FILE4LONG size )
      {
         return ftruncate( f4->hand, size ) ;
      }
   #endif /* MSC_VER */
#endif  /* ifdef S4MACINTOSH */
#endif   /* ifdef S4NO_CHSIZE */

FILE4LONG S4FUNCTION file4lenLow( FILE4 *f4 )
{
   FILE4LONG lrc ;

   #ifdef E4PARM_HIGH
      if ( f4 == 0 )
      {
         error4( 0, e4parm_null, E90605 ) ;
         file4longAssignError( lrc ) ;
         return lrc ;
      }
   #endif
   #ifdef E4ANALYZE
      if ( f4->hand == INVALID4HANDLE )
      {
         error4( f4->codeBase, e4parm, E90605 ) ;
         file4longAssignError( lrc ) ;
         return lrc ;
      }
   #endif

   #ifndef S4OFF_OPTIMIZE
      if ( f4->isTemp == 1 && f4->fileCreated == 0 )
      {
         /* 04/24/96 AS fix for c/s t4commit.c */
         if ( file4longError( f4->len ) == (unsigned long)-1L )
         {
            file4longAssign( lrc, 0, 0 ) ;
            return lrc ;
         }
         else
            return f4->len ;
      }
      if ( f4->doBuffer && file4longError( f4->len ) != (unsigned long)-1L )
         lrc = f4->len ;
      else
   #endif
   lrc = u4filelength( f4->hand ) ;
   if ( file4longError( lrc ) == (unsigned long)-1L )
      error4describe( f4->codeBase, e4len, E90605, f4->name, 0, 0 ) ;
   return lrc ;
}

#ifdef S4FILE_EXTENDED
int S4FUNCTION file4lenSet( FILE4 *f4, long newLen )
{
   FILE4LONG lenSet ;

   file4longAssign( lenSet, newLen, 0 ) ;

   return file4lenSetLow( f4, lenSet ) ;
}

int file4lenSetLow( FILE4 *f4, FILE4LONG newLen )
#else
int S4FUNCTION file4lenSetLow( FILE4 *f4, FILE4LONG newLen )
#endif
{
   int rc, isLong = 0 ;
   CODE4 *c4 ;
   #ifdef S4WRITE_DELAY
      FILE4WRITE_DELAY *writeDelay ;
      LINK4 *delayLink ;
   #endif
   #ifdef __SC__
      union REGS dosFlush ;
   #endif
   unsigned long newLenShort ;

   #ifdef E4PARM_HIGH
      if ( f4 == 0 || file4longError( newLen ) == (unsigned long)-1L )
         return error4( 0, e4parm_null, E90606 ) ;
   #endif

   c4 = f4->codeBase ;

   #ifdef E4ANALYZE
      if ( f4->hand == INVALID4HANDLE || f4->codeBase == 0 )
         return error4( c4, e4struct, E90606 ) ;
      if ( f4->isReadOnly )
         return error4( c4, e4struct, E80601 ) ;
   #else
      if ( c4 == 0 )
         return -1 ;
   #endif

   if ( error4code( c4 ) > 0 && error4code( c4 ) < 200 )  /* file error */
      return -1 ;

   if ( f4->isReadOnly )
      return error4( c4, e4parm, E80607 ) ;

   #ifdef S4FILE_EXTENDED
      if ( file4longGetHi( newLen ) )
         f4->isLong = 1 ;
      else
         f4->isLong = 0 ;
   #endif

   newLenShort = file4longGetLo( newLen ) ;

   #ifndef S4OFF_OPTIMIZE
      if ( f4->doBuffer )
      {
         #ifdef S4FILE_EXTENDED
            if ( f4->isLong != 0 )  /* it means file is extenced past 4 gig mark, stop optimizing it */
            {
               assert5( file4longGetHi( newLen ) >= 1 ) ; /* no error value */
               file4optimize( f4, OPT4OFF, 0 ) ;
               if ( f4->doBuffer != 0 )
                  return error4( 0, e4result, E90606 ) ;
            }
         #endif
      }
      if ( f4->doBuffer )
      {
         /* if len is -1 but bufferWrites true, may still have buffered data
            which must be removed, so get disk file length for reference */
         if ( f4->bufferWrites == 1 && f4->fileCreated != 0 )
         {
            f4->len = file4lenLow( f4 ) ;
            if ( file4longGetHi( f4->len ) != 0 )
               error4( 0, e4result, E90606 ) ;
         }
         if ( file4longGetLo( f4->len ) > newLenShort )   /* must do a partial delete of memory */
            opt4fileDelete( f4, newLenShort, file4longGetLo( f4->len ) ) ;
         if ( f4->bufferWrites )
            f4->len = newLen ;
         #ifdef E4ANALYZE
            else
               if ( file4longError( f4->len ) != (unsigned long)-1L )
                  return error4( 0, e4result, E90606 ) ;
         #endif
      }

      #ifdef E4ANALYZE_ALL
         if ( f4->hasDup == 1 )
            if ( f4->doBuffer == 1 || f4->link.n == 0 )
               if ( file4partLenSet( f4, newLenShort ) < 0 )
                  return error4( c4, e4opt, E80602 ) ;
      #endif

      #ifdef S4SAFE
         if ( f4->fileCreated != 0 )   /* don't need to safeguard temporary files */
      #else
         /* E4ANALYZE must explicitly set file length for later verifications */
         #ifndef E4ANALYZE
            if ( f4->doBuffer == 0 || f4->bufferWrites == 0 )
         #endif
      #endif
   #endif

   {      /* needed !!! */
      #ifndef S4OFF_OPTIMIZE
         if ( f4->fileCreated == 1 )
      #endif
      {
         #ifdef S4MACINTOSH
            rc = SetEOF( f4->hand, newLen ) ;
         #else
            #ifdef __WIN32
               #ifdef S4MULTI_THREAD
                  EnterCriticalSection( &f4->critical4file ) ;
               #endif
               #ifdef S4WRITE_DELAY
                  if ( l4numNodes( &f4->delayWriteFileList ) != 0 )
                  {
                     for ( delayLink = (LINK4 *)l4first( &f4->delayWriteFileList ) ;; )
                     {
                        if ( delayLink == 0 )
                           break ;
                        writeDelay = (FILE4WRITE_DELAY *)(delayLink - 1 ) ;
                        delayLink = (LINK4 *)l4next( &f4->delayWriteFileList, delayLink ) ;
                        /* now, if the delay-write is without the boundaries of
                           the len-set, then remove that part */
                        if ( ( writeDelay->pos + writeDelay->len ) > newLenShort )
                        {
                           /* maybe is being written to disk now, in which case
                              must wait */
                           while ( writeDelay->usageFlag == r4inUse )  /* is being written to disk, just wait until it is done... */
                              Sleep( 0 ) ;
                           if ( writeDelay->usageFlag == r4finished ) /* is written to disk, so can just ignore */
                              continue ;
                           if ( writeDelay->pos > newLenShort )   /* just remove */
                           {
                              writeDelay->status = 0 ;
                              writeDelay->usageFlag = r4finished ;  /* outside of critical section, to allow a wait for completion while keeping the critical section */
                              l4remove( &f4->codeBase->delayWriteList, writeDelay ) ;
                              l4remove( &f4->delayWriteFileList, &writeDelay->fileLink ) ;
                              writeDelay->completionRoutine( writeDelay ) ;
                              mem4free( c4->delayWriteMemory, writeDelay ) ;
                           }
                           else  /* just reduce the length */
                              writeDelay->len = newLenShort - writeDelay->pos ;
                        }
                     }
                  }
               #endif
               if ( SetFilePointer( (HANDLE)f4->hand, file4longGetLo( newLen ), file4longGetHiAddress( newLen ), FILE_BEGIN ) == (DWORD)-1 )
                  return error4describe( c4, e4lenSet, E90606, f4->name, 0, 0 ) ;
               if ( SetEndOfFile( (HANDLE)f4->hand ) )
                  rc = 0 ;
               else
                  rc = -1 ;
               #ifdef S4MULTI_THREAD
                  LeaveCriticalSection( &f4->critical4file ) ;
               #endif
            #else
               #ifdef __SC__
                  rc = 0 ;
                  dosFlush.x.ax = 0x4200;
                  dosFlush.x.bx = f4->hand ;
                  memcpy((void *)&dosFlush.x.dx,(void *)&newLen,2);
                  memcpy((void *)&dosFlush.x.cx,((char *)&newLen)+2,2);
                  intdos( &dosFlush, &dosFlush ) ;
                  if ( dosFlush.x.cflag != 0 )
                    return error4( c4, e4lenSet, E90606 ) ;
                  dosFlush.h.ah = 0x40;
                  dosFlush.x.bx = f4->hand ;
                  dosFlush.x.cx = 0x00;
                  intdos( &dosFlush, &dosFlush ) ;
                  if ( dosFlush.x.cflag != 0 )
                    return error4( c4, e4lenSet, E90606 ) ;
                  rc = 0 ;
               #else
                  #ifdef S4NO_CHSIZE
                     rc = file4changeSize( f4, newLen ) ;
                  #else
                     rc = chsize( f4->hand, newLen ) ;
                  #endif
               #endif
            #endif
         #endif

         if ( rc < 0 )
            return error4describe( c4, e4lenSet, E90606, f4->name, (char *)0, (char *)0 ) ;
      }
   }  /* needed ! ! ! */

   return 0 ;
}

static unsigned file4readLowDo( FILE4 *f4, FILE4LONG pos, void *ptr, unsigned len )
{
   unsigned long urc ;
   #ifndef __WIN32
      unsigned long rc ;
   #endif
   #ifndef S4OFF_OPTIMIZE
      #ifdef S4OPTIMIZE_STATS
         DATA4 *stat ;
         CODE4 *c4 ;
      #endif
   #endif

   #ifdef S4MULTI_THREAD
      EnterCriticalSection( &f4->critical4file ) ;
   #endif

   #ifndef S4OFF_OPTIMIZE
      #ifdef S4OPTIMIZE_STATS
         c4 = f4->codeBase ;
         stat = c4->statusDbf ;
         if ( stat != 0 )  /* track stats */
         {
            if ( f4 != &stat->dataFile->file )  /* don't do for the stat file! */
            {
               if ( d4appendStart( stat, 0 ) == 0 )
               {
                  f4assignChar( c4->typeFld, 'L' ) ;  /* low-level */
                  f4assign( c4->fileNameFld, f4->name ) ;
                  f4assignLong( c4->offsetFld, pos ) ;
                  f4assignLong( c4->lengthFld, len ) ;
                  d4append( stat ) ;
               }
            }
         }
      #endif
   #endif

   #ifdef __WIN32
      file4longSetLo( pos, SetFilePointer( (HANDLE)f4->hand, file4longGetLo( pos ), file4longGetHiAddress( pos ), FILE_BEGIN ) ) ;
      file4longCheckError( pos ) ;
   #else
      #ifdef S4MACINTOSH
         rc = MAClseek( f4->hand, pos, 0, 0 ) ;
      #else
         #ifdef S4WINDOWS
            rc = _llseek( f4->hand, pos, 0 ) ;
         #else
            #ifdef S4LSEEK
               rc = f4lseek( f4, pos, 0, 0 ) ;
            #else
               rc = lseek( f4->hand, pos, 0 ) ;
            #endif
         #endif
      #endif
      if ( rc != pos )
         rc = (unsigned long)-1 ;
   #endif

   if ( file4longError( pos ) == (unsigned long)-1L )
   {
      #ifdef S4MULTI_THREAD
         LeaveCriticalSection( &f4->critical4file ) ;
      #endif
      file4readError( f4, pos, len, "file4readLow" ) ;
      return 0 ;
   }

   #ifdef __WIN32
      ReadFile( (HANDLE)f4->hand, ptr, len, &urc, 0 ) ;
   #else
      #ifdef S4MACINTOSH
         rc = (long) len ;
         urc = FSRead( f4->hand, &rc, ptr ) ;
         if ( urc == 0 )
            urc = len ;
         if ( urc == eofErr )  /* attempt to read past EOF OK */
            urc = rc ;
      #else
         #ifdef S4WINDOWS
            urc = (unsigned)_lread( f4->hand, (char *)ptr, len ) ;
         #else
            #ifdef S4LSEEK
               /* if reading past EOF */
               if ( pos+len > u4filelength( f4->hand ) )
                  urc = (unsigned)read( f4->hand, ptr, u4filelength(f4->hand) - pos ) ;
               else
                  urc = (unsigned)read( f4->hand, ptr, len ) ;
            #else
               urc = (unsigned)read( f4->hand, (char *)ptr, len ) ;
            #endif
         #endif
      #endif
   #endif

   if ( urc > len )
   {
      #ifdef S4MULTI_THREAD
         LeaveCriticalSection( &f4->critical4file ) ;
      #endif

      file4readError( f4, pos, len, "file4readLow" ) ;
      return 0 ;
   }

   #ifndef S4OFF_OPTIMIZE
      #ifdef E4ANALYZE_ALL
         if ( f4->hasDup == 1 )
            if ( f4->doBuffer == 1 || f4->link.n == 0 )
               if ( file4cmpPart( f4->codeBase, ptr, f4, file4longGetLo( pos ), urc ) != 0 )
               {
                  #ifdef S4MULTI_THREAD
                     LeaveCriticalSection( &f4->critical4file ) ;
                  #endif
                  error4( f4->codeBase, e4opt, E80602 ) ;
                  return 0 ;
               }
      #endif
   #endif

   #ifdef S4MULTI_THREAD
      LeaveCriticalSection( &f4->critical4file ) ;
   #endif

   return urc ;
}

/* this function also includes advance-read/delay-write checking */
unsigned file4readLow( FILE4 *f4, FILE4LONG pos, void *ptr, unsigned len )
{
   unsigned urc ;
   #ifndef S4OFF_OPTIMIZE
      #ifdef S4OPTIMIZE_STATS
         DATA4 *stat ;
         CODE4 *c4 ;
      #endif
   #endif
   #ifdef S4WRITE_DELAY
      FILE4WRITE_DELAY *writeDelay ;
      LINK4 *delayLink ;
      long beforeLen, afterLen ;
      FILE4LONG afterPos ;
      unsigned copyLen, copyPos ;
      unsigned posShort ;
      int noDelay ;
   #endif

   #ifdef S4ADVANCE_READ
      /* check the special advance-read-buffer for the file first, if it fits
         in entirely, then copy from there instead of performing read */
      #ifdef S4FILE_EXTENDED
         if ( f4->isLong == 0 )  /* if it is set put still in area, then wait for the advance-read to finish */
      #endif
      if ( f4->advanceReadBufStatus != AR4EMPTY )  /* if it is set put still in area, then wait for the advance-read to finish */
      {
         unsigned posShort ;
         int noAdvance = 0 ;
         if ( file4longGetHi( pos ) != 0 )  /* we only advance read for files < 4 gigs */
            noAdvance = 1 ;
         posShort = file4longGetLo( pos ) ;
         if ( posShort > ULONG_MAX - len )  /* means we will exceed - i.e. large file */
            noAdvance = 1 ;

         if ( noAdvance == 0 )
         {
            if ( ( posShort >= f4->advanceReadBufPos  ) && (file4longGetLo( pos ) + len) <= (f4->advanceReadBufPos + f4->advanceReadBufLen) )
            {
               while( f4->advanceReadBufStatus == AR4SET )
                  Sleep( 0 ) ;
               if ( f4->advanceReadBufStatus == AR4FULL ) /* successful read */
               {
                  memcpy( (char *)ptr, f4->advanceReadBuf + posShort - f4->advanceReadBufPos, len ) ;
                  return len ;
               }
            }
         }
      }
   #endif

   #ifdef S4MULTI_THREAD
      EnterCriticalSection( &f4->critical4file ) ;
   #endif

   #ifdef S4WRITE_DELAY
      /* make sure that the data to read isn't in memory */
         noDelay = 0 ;
         if ( file4longGetHi( pos ) != 0 )  /* we only advance read for files < 4 gigs */
            noDelay = 1 ;
         posShort = file4longGetLo( pos ) ;
         if ( posShort > ULONG_MAX - len )  /* means we will exceed - i.e. large file */
            noDelay = 1 ;

      if ( noDelay == 0 )
      {
         urc = 0 ;
         if ( l4numNodes( &f4->delayWriteFileList ) != 0 )  /* check for pieces already in memory */
         {
            for ( delayLink = 0 ;; )
            {
               delayLink = (LINK4 *)l4next( &f4->delayWriteFileList, delayLink ) ;
               if ( delayLink == 0 )
                  break ;
               writeDelay = (FILE4WRITE_DELAY *)(delayLink - 1 ) ;
               /* now, if the delay piece belongs in the buffer, then read all
                  the info before the delay piece, copy the delay piece over,
                  and read all the info after the delay piece */
               if ( ( writeDelay->pos + writeDelay->len) <= posShort )  /* outside of block */
                  continue ;
               if ( ( posShort + len ) <= writeDelay->pos )  /* outside of block */
                  continue ;
               beforeLen = writeDelay->pos - posShort ;
               if ( beforeLen < 0 )
                   beforeLen = 0 ;
               if ( beforeLen == 0 )
               {
                  copyPos = posShort - writeDelay->pos ;
                  copyLen = len ;
               }
               else
               {
                  copyPos = 0 ;
                  copyLen = len - beforeLen ;
               }
               if ( copyLen > ( writeDelay->len - copyPos ) )
                  copyLen = writeDelay->len - copyPos ;
               file4longAssign( afterPos, posShort + beforeLen + copyLen, 0 ) ;

               afterLen = posShort - writeDelay->pos + len - writeDelay->len ;
               if ( beforeLen != 0 )
                  urc = file4readLow( f4, pos, ptr, beforeLen ) ;
               if ( urc == (unsigned int)beforeLen )
               {
                  memcpy( (char *)ptr + beforeLen, writeDelay->data + copyPos, copyLen ) ;
                  urc += copyLen ;
                  if ( afterLen > 0 )  /* is negative if read ends within block */
                     urc += file4readLow( f4, afterPos, (char *)ptr + beforeLen + copyLen, afterLen ) ;
               }
               LeaveCriticalSection( &f4->critical4file ) ;
               return urc ;
            }
         }
      }
   #endif

   urc = file4readLowDo( f4, pos, ptr, len ) ;

   #ifdef S4MULTI_THREAD
      LeaveCriticalSection( &f4->critical4file ) ;
   #endif

   #ifdef S4ADVANCE_READ
      #ifdef S4FILE_EXTENDED
         if ( f4->isLong == 0 )
      #endif
         file4advanceReadWriteOver( f4, file4longGetLo( pos ), len, ptr, 0 ) ;
   #endif

   return urc ;
}

#ifndef S4INTERNAL_COMPILE_CHECK
unsigned S4FUNCTION file4read( FILE4 *f4, const long posIn, void *ptr, const unsigned lenIn )
{
   FILE4LONG pos ;

   #ifdef E4PARM_HIGH
      if ( f4 == 0 || posIn < 0 || ptr == 0  )
      {
         error4( 0, e4parm_null, E90607 ) ;
         return 0 ;
      }
   #endif
   file4longAssign( pos, posIn, 0 ) ;

   return file4readInternal( f4, pos, ptr, lenIn ) ;
}
#endif

unsigned file4readInternal( FILE4 *f4, FILE4LONG pos, void *ptr, unsigned len )
{
   #ifndef S4OFF_OPTIMIZE
      unsigned urc ;
      #ifdef S4OPTIMIZE_STATS
         DATA4 *stat ;
         CODE4 *c4 ;
      #endif
   #endif

   #ifdef E4ANALYZE
      if ( f4->hand == INVALID4HANDLE )
      {
         error4( f4->codeBase, e4parm, E90607 ) ;
         return 0 ;
      }
   #endif

   if ( error4code( f4->codeBase ) < 0 )
      return 0 ;

   if ( len == 0 )
      return 0 ;

   #ifndef S4OFF_OPTIMIZE
      if ( f4->doBuffer )
      {
         if ( file4longGetHi( pos ) != 0 )  /* means out of range, nothing read */
            return 0 ;
         #ifdef S4OPTIMIZE_STATS
            c4 = f4->codeBase ;
            stat = c4->statusDbf ;
            if ( stat != 0 )  /* track stats */
            {
               if ( f4 != &stat->dataFile->file )  /* don't do for the stat file! */
               {
                  if ( d4appendStart( stat, 0 ) == 0 )
                  {
                     f4assignChar( c4->typeFld, 'H' ) ;  /* high-level */
                     f4assign( c4->fileNameFld, f4->name ) ;
                     f4assignLong( c4->offsetFld, file4longGetLo( pos ) ) ;
                     f4assignLong( c4->lengthFld, file4longGetLo( len ) ) ;
                     d4append( stat ) ;
                  }
               }
            }
         #endif

         urc = (unsigned)opt4fileRead( f4, file4longGetLo( pos ), ptr, len )  ;
         if ( urc > len )
         {
            file4readError( f4, pos, len, "file4read" ) ;
            return 0 ;
         }
         return urc ;
      }
      else
      {
         if ( f4->fileCreated == 0 )   /* cannot read from non-existant file */
            return 0 ;
   #endif
      return file4readLow( f4, pos, ptr, len ) ;

   #ifndef S4OFF_OPTIMIZE
      }
   #endif
}

#ifndef S4INTERNAL_COMPILE_CHECK
int S4FUNCTION file4readAll( FILE4 *f4, const long posIn, void *ptr, const unsigned lenIn )
{
   FILE4LONG pos ;

   #ifdef E4PARM_HIGH
      if ( f4 == 0 || posIn < 0 || ptr == 0  )
         return error4( 0, e4parm_null, E90608 ) ;
   #endif

   file4longAssign( pos, posIn, 0 ) ;

   return file4readAllInternal( f4, pos, ptr, lenIn ) ;
}
#endif

int file4readAllInternal( FILE4 *f4, FILE4LONG pos, void *ptr, unsigned len )
{
   unsigned urc ;

   #ifdef E4ANALYZE
      if ( f4->hand == INVALID4HANDLE )
         return error4( f4->codeBase, e4parm, E90608 ) ;
   #endif

   if ( error4code( f4->codeBase ) < 0 )
      return -1 ;

   if ( len == 0 )
      return 0 ;

   #ifndef S4OFF_OPTIMIZE
      if ( f4->doBuffer )
      {
         if ( file4longGetHi( pos ) != 0 )  /* means out of range, nothing read */
            return file4readError( f4, pos, len, "file4readAll" ) ;
         urc = opt4fileRead( f4, file4longGetLo( pos ), ptr, len )  ;
         if ( urc != len )
            return file4readError( f4, pos, len, "file4readAll" ) ;
         return 0 ;
      }
      else
      {
         if ( f4->fileCreated == 0 )   /* cannot read from non-existant file */
            return error4( f4->codeBase, e4opt, E90607 ) ;
   #endif
      urc = file4readLow( f4, pos, ptr, len ) ;
      if ( urc != len )
         return file4readError( f4, pos, len, "file4readAllLow" ) ;
   #ifndef S4OFF_OPTIMIZE
      }
   #endif

   return 0 ;
}

int S4FUNCTION file4readError( FILE4 *f4, FILE4LONG pos, unsigned len, const char *location )
{
   char posBuf[40] ;

   memset( posBuf, 0, sizeof( posBuf ) ) ;
   memset( posBuf, ' ', sizeof( posBuf ) - 1 ) ;

   c4ltoa45( file4longGetLo( pos ), posBuf, 19 ) ;
   c4ltoa45( len, posBuf + 20, 19 ) ;

   return error4describe( f4->codeBase, e4read, E90621, f4->name, posBuf, location ) ;
}

#ifdef S4FILE_EXTENDED
/*
   void file4longSubtractLongLong( FILE4LONG *f1, FILE4LONG f2 )
   {
      file4longSubtract( f1, file4longGetLo( f2 ) ) ;
      f1->piece.longHi -= f2.piece.longHi ;
   }
*/
/* this function requires that the result be contained within a long */
/*
   unsigned file4longSubtractLong( FILE4LONG f1, FILE4LONG f2 )
   {
      file4longSubtract( &f1, file4longGetLo( f2 ) ) ;
      f1.piece.longHi -= f2.piece.longHi ;
      assert5( f1.piece.longHi == 0 ) ;
      return f1.piece.longLo ;
   }
*/
/*
   void file4longSubtract( FILE4LONG *f1, unsigned long val )
   {
      if ( (unsigned long)val < f1->piece.longLo )
         f1->piece.longLo -= val ;
      else
      {
         if ( f1->piece.longHi == 0 ) */  /* nothing to subtract from... */
/*
            f1->piece.longLo = 0 ;
         else
         {
            f1->piece.longHi-- ;
            f1->piece.longLo += (ULONG_MAX-val) ;
         }
      }
   }
*/

/*
   void file4longAddLong( FILE4LONG *f1, FILE4LONG *f2 )
   {
      file4longAdd( f1, file4longGetLo( *f2 ) ) ;
      f1->piece.longHi += f2->piece.longHi ;
   }
*/

/*
   void file4longAdd( FILE4LONG *f1, unsigned long val )
   {
      unsigned long tVal = f1->piece.longLo + val ;
      if ( tVal < f1->piece.longLo ) */ /* means carry over to hiPos */
/*
         f1->piece.longHi++ ;
      f1->piece.longLo = tVal ;
   }
*/
#endif

int S4FUNCTION file4replace( FILE4 *keep, FILE4 *from )
{
   FILE4 tmp ;
   int rc ;
   char fromName[LEN4PATH] ;
   CODE4 *c4 ;
   #ifndef S4SINGLE
      char *buf ;
      unsigned bufSize ;
      #ifdef S4LOW_MEMORY
         #ifndef S4OFF_OPTIMIZE
            int hasOpt ;
         #endif
      #endif
      FILE4LONG pos ;
      FILE4LONG fLen ;
   #endif

   #ifdef E4PARM_LOW
      if ( keep == 0 || from == 0  )
         return error4( 0, e4parm_null, E90609 ) ;
   #endif

   rc = 0 ;
   c4 = from->codeBase ;
   #ifdef E4ANALYZE
      if ( from->isReadOnly || keep->isReadOnly )
         return error4( c4, e4parm, E90601 ) ;
   #endif

   #ifndef S4SINGLE
      if ( keep->lowAccessMode == OPEN4DENY_RW )
      {
   #endif
      memcpy( (void *)&tmp, (void *)keep, sizeof ( FILE4 ) ) ;  /* remember settings */

   /* 05/09/96 AS first, must flush the files to disk to avoid delay-writes
      which are based on the FILE4 pointer which is maintained, instead of
      the physical handle, which is not --> or at least unoptimize it */

      file4optimize( from, OPT4OFF, OPT4OTHER ) ;
      file4optimize( keep, OPT4OFF, OPT4OTHER ) ;
      file4flush( from ) ;
      file4flush( keep ) ;
      #ifdef S4MACINTOSH
         strcpy( fromName, keep->name ) ;
         rc = FSpExchangeFiles(&keep->macSpec, &from->macSpec) ;
         if (rc <0)
            return -1 ;
         rc = file4close(keep) ;
         from->isTemp = 1 ;
         if (file4close(from) )
            return -1 ;
         if ( file4open ( keep, c4, fromName, 0 ) )
            return -1 ;
      #else
         keep->hand = from->hand ;
         from->hand = tmp.hand ;
         keep->doAllocFree = 0 ;
         strncpy( fromName, from->name, sizeof( fromName ) ) ;
         from->name = keep->name ;
         from->isTemp = 1 ;
         if ( file4close ( from ) )
            return -1 ;
         if ( file4close ( keep ) )
            return -1 ;

         if ( u4rename( fromName, tmp.name ) < 0 )
            return -1 ;

         if ( file4open ( keep, c4, tmp.name, 0 ) )
            return -1 ;
      #endif
      keep->isTemp = tmp.isTemp ;
      keep->doAllocFree = tmp.doAllocFree ;
      if ( keep->doAllocFree == 1 )  /* AS 3/4/98 also must assign NameBuf or memory leakage */
         keep->nameBuf = (char *)tmp.name ;
      #ifndef S4OFF_OPTIMIZE
         if ( tmp.link.n != 0 )   /* file was optimized... */
            file4optimizeLow( keep, c4->optimize, tmp.type, tmp.expectedReadSize, tmp.ownerPtr ) ;
      #endif
   #ifndef S4SINGLE
      }
      else  /* can't lose the file handle if other user's have the file open, so just do a copy */
      {
         FILE4LONG len ;
         file4longAssign( len, 0, 0 ) ;

         file4lenSetLow( keep, len ) ;

         bufSize = c4->memSizeBuffer ;

         #ifdef S4LOW_MEMORY
            #ifndef S4OFF_OPTIMIZE
               hasOpt = c4->hasOpt && c4->opt.numBuffers ;
               code4optSuspend( c4 ) ;
            #endif
         #endif

         for ( ;; bufSize -= 0x800 )
         {
            if ( bufSize < 0x800 )  /* make one last try */
            {
               bufSize = 100 ;
               buf = (char *)u4allocEr( c4, (long)bufSize ) ;
               if ( buf == 0 )
                  return -1 ;
            }
            buf = (char *)u4alloc( (long)bufSize ) ;
            if ( buf )
               break ;
         }

         file4longAssign( pos, 0, 0 ) ;
         for( fLen = file4lenLow( from ) ; file4longGreaterZero( fLen ) ; file4longSubtract( &fLen, (long)bufSize ) )
         {
            if ( ( file4longLess( fLen, bufSize) ) )
               bufSize = file4longGetLo( fLen ) ;

            if ( file4readAllInternal( from, pos, buf, bufSize ) < 0 )
            {
               rc = -1 ;
               break ;
            }
            if ( file4writeInternal( keep, pos, buf, bufSize ) < 0 )
            {
               rc = -1 ;
               break ;
            }
            file4longAdd( &pos, bufSize ) ;
         }
         from->isTemp = 1 ;
         file4close( from ) ;
         u4free( buf ) ;
         #ifdef S4LOW_MEMORY
            #ifndef S4OFF_OPTIMIZE
               if ( hasOpt )
                  code4optRestart( c4 ) ;
            #endif
         #endif
      }
   #endif
   return rc ;
}

#ifdef S4NO_ECVT
   #define S4NO_ECVTFCVT
#endif
#ifdef S4NO_FCVT
   #define S4NO_ECVTFCVT
#endif

#ifdef S4NO_ECVTFCVT

#define MAXIMUM 30
#define PRECISION 17

static char valueStr[32] ;

static double minus[] =
{1e-256,1e-128,1e-64,
 1e-32,1e-16,1e-8,
 1e-4,1e-2,1e-1,1.0} ;

static double plus[] =
{1e+256,1e+128,1e+64,
 1e+32,1e+16,1e+8,
 1e+4,1e+2,1e+1} ;

#ifdef S4NO_ECVT
char *f4ecvt( double value, int numdigits, int *decPtr, int *signPtr )
{
   int dptr, count, j, k ;
   char *vPtr ;

   if ( numdigits < 0 )
      numdigits = 0 ;
   else
      if ( numdigits > MAXIMUM ) numdigits = MAXIMUM ;

   if ( value < 0.0 )
   {
      value = -value ;
      *signPtr = 1 ;
   }
   else
      *signPtr = 0 ;

   if ( value == 0.0 )
   {
      memset( valueStr, '0', numdigits ) ;
      dptr = 0 ;
   }
   else
   {
      dptr = 1 ;
      k = 256 ;
      count = 0 ;
      while ( value < 1.0 )
      {
        while ( value < minus[count+1] )
        {
           value *= plus[count] ;
           dptr -= k ;
        }
        k /= 2 ;
        count++ ;
      }
      k = 256 ;
      count = 0 ;
      while ( value >= 10.0 )
      {
        while ( value >= plus[count] )
        {
           value *= minus[count] ;
           dptr += k ;
        }
        k /= 2 ;
        count++ ;
      }

      for ( vPtr = &valueStr[0]; vPtr <= &valueStr[numdigits]; vPtr++ )
      {
         if ( vPtr >= &valueStr[PRECISION] )  *vPtr = '0' ;
         else
         {
            j = value ;
            *vPtr = j + '0' ;
            value = ( value - j + 1.0e-15 ) * 10.0 ;
         }
      }
      --vPtr ;
      if ( *vPtr >= '5' )
      {
        while (1)
        {
           if ( vPtr == &valueStr[0] )
           {
              dptr++ ;
              valueStr[0] = '1' ;
              break ;
           }
           *vPtr = 0 ;
           --vPtr ;
           if ( *vPtr != '9' )
           {
              (*vPtr)++ ;
              break ;
           }
        }
      }
   }
   *decPtr = dptr ;
   valueStr[numdigits] = 0 ;
   return valueStr ;
}
#endif

#ifdef S4NO_FCVT
char *f4fcvt( double value, int numdigits, int *decPtr, int *signPtr )
{
   int dptr, count, j, k ;
   char *vPtr ;

   if ( numdigits < 0 )
      numdigits = 0 ;
   else
      if ( numdigits > MAXIMUM ) numdigits = MAXIMUM ;

   if ( value < 0.0 )
   {
      value = -value ;
      *signPtr = 1 ;
   }
   else
      *signPtr = 0 ;

   if ( value == 0.0 )
   {
      memset( valueStr, '0', numdigits ) ;
      dptr = 0 ;
   }
   else
   {
      dptr = 1 ;
      k = 256 ;
      count = 0 ;
      while ( value < 1.0 )
      {
        while ( value < minus[count+1] )
        {
           value *= plus[count] ;
           dptr -= k ;
        }
        k /= 2 ;
        count++ ;
      }
      k = 256 ;
      count = 0 ;
      while ( value >= 10.0 )
      {
        while ( value >= plus[count] )
        {
           value *= minus[count] ;
           dptr += k ;
        }
        k /= 2 ;
        count++ ;
      }

      if ( ( numdigits += dptr ) < 0 )
        numdigits = 0 ;
      else
        if ( numdigits > MAXIMUM )  numdigits = MAXIMUM ;

      for ( vPtr = &valueStr[0]; vPtr <= &valueStr[numdigits]; vPtr++ )
      {
         if ( vPtr >= &valueStr[PRECISION] )  *vPtr = '0' ;
         else
         {
            j = value ;
            *vPtr = j + '0' ;
            value = ( value - j + 1.0e-15 ) * 10.0 ;
         }
      }
      --vPtr ;
      if ( *vPtr >= '5' )
      {
        while (1)
        {
           if ( vPtr == &valueStr[0] )
           {
              numdigits++ ;
              dptr++ ;
              valueStr[0] = '1' ;
              break ;
           }
           *vPtr = 0 ;
           --vPtr ;
           if ( *vPtr != '9' )
           {
              (*vPtr)++ ;
              break ;
           }
        }
      }
   }
   *decPtr = dptr ;
   valueStr[numdigits] = 0 ;
   return valueStr ;
}
#endif /*S4NO_FCVT */
#endif /* S4NO_ECVTFCVT*/
#undef S4NO_ECVTFCVT

#ifdef S4READ_ADVANCE

#define MEM4ADVANCE_START 10
#define MEM4ADVANCE_EXPAND 10

int S4FUNCTION file4advanceRead( FILE4 *f4, unsigned pos, void *data, const unsigned len, S4ADVANCE_FUNCTION *completionRoutine, void *completionData )
{
   FILE4ADVANCE_READ *advanceRead ;
   CODE4 *c4 ;

   c4 = f4->codeBase ;

   if ( c4->advanceReadsEnabled == 0 )  /* not enabled */
      return 0 ;

   if ( c4->advanceReadMemory == 0 )
      advanceRead = (FILE4ADVANCE_READ *)mem4createAlloc( c4, &c4->advanceReadMemory, MEM4ADVANCE_START, sizeof( FILE4ADVANCE_READ ), MEM4ADVANCE_EXPAND, 0 ) ;
   else
      advanceRead = (FILE4ADVANCE_READ *)mem4alloc( c4->advanceReadMemory ) ;

   if ( advanceRead == 0 )
      return error4( c4, e4memory, E90624 ) ;

   advanceRead->file = f4 ;
   advanceRead->data = (char *)data ;
   advanceRead->len = len ;
   advanceRead->pos = pos ;
   advanceRead->usageFlag = r4queued ;
   advanceRead->completionRoutine = completionRoutine ;
   advanceRead->completionData = completionData ;

   EnterCriticalSection( &c4->critical4advanceReadList ) ;

   l4add( &c4->advanceReadList, advanceRead ) ;
   l4add( &f4->advanceReadFileList, &advanceRead->fileLink ) ;

   LeaveCriticalSection( &c4->critical4advanceReadList ) ;

   SetEvent( c4->pendingReadEvent ) ;  /* notify the write thread */
   Sleep( 0 ) ;

   return 0 ;
}

/* cancels all advance-reads for the given file */
int file4advanceCancel( FILE4 *f4 )
{
   FILE4ADVANCE_READ *advanceRead ;
   LINK4 *advanceReadLink, *saved ;
   CODE4 *c4 ;

   c4 = f4->codeBase ;

   EnterCriticalSection( &c4->critical4advanceReadList ) ;

   for ( advanceReadLink = (LINK4 *)l4first( &f4->advanceReadFileList ) ;; )
   {
      if ( advanceReadLink == 0 )
         break ;
      advanceRead = (FILE4ADVANCE_READ *)(advanceReadLink - 1) ;
      saved = (LINK4 *)l4next( &f4->advanceReadFileList, advanceReadLink ) ;
      if ( advanceRead->usageFlag == r4queued )  /* do ourselves */
      {
         l4remove( &f4->advanceReadFileList, advanceReadLink ) ;
         l4remove( &c4->advanceReadList, advanceRead ) ;
         advanceRead->status = 0 ;
         advanceRead->usageFlag = r4finished ;
         mem4free( c4->advanceReadMemory, advanceRead ) ;
      }
      advanceReadLink = saved ;
   }

   LeaveCriticalSection( &c4->critical4advanceReadList ) ;

   for ( ;; )
   {
      /* now verify that the checkInUse read gets completed */
      if ( l4numNodes( &f4->advanceReadFileList ) == 0 )
         break ;
      #ifdef E4ANALYZE
         if ( l4numNodes( &f4->advanceReadFileList ) > 1 )   /* in theory impossible, it means delay-write has 2 files writing at same time */
            return error4( c4, e4struct, E90624 ) ;
      #endif
      SetEvent( c4->pendingReadEvent ) ;  /* notify the write thread */
      Sleep( 0 ) ;   /* give up our time slice to get the delay-write going */
   }

   #ifndef S4OFF_OPTIMIZE
      if ( c4->opt.advanceReadFile == f4 )
      {
         c4->opt.advanceLargeBufferAvail = AR4EMPTY ;
         c4->opt.advanceReadFile = 0 ;
      }
   #endif

   return 0 ;
}

#ifdef S4USE_INT_DELAY
   int file4advanceReadMain( void *data )
#else
   void file4advanceReadMain( void *data )
#endif
{
   CODE4 *c4 ;
   FILE4ADVANCE_READ *advanceRead ;
   FILE4LONG tLong ;

   c4 = (CODE4 *)data ;
   c4->advanceReadsEnabled = 1 ;

   for ( ;; )
   {
      if ( l4numNodes( &c4->advanceReadList ) == 0 )
      {
         if ( c4->uninitializeAdvanceRead == 1 )   /* shutdown */
         {
            SetEvent( c4->initUndoAdvanceRead ) ;
            #ifdef S4USE_INT_DELAY
               return 0 ;
            #else
               return ;
            #endif
         }
         else
         {
            WaitForSingleObject( c4->pendingReadEvent, INFINITE ) ;
            ResetEvent( c4->pendingReadEvent ) ;
         }
      }
      else  /* perform a read on the first available block */
      {
         EnterCriticalSection( &c4->critical4advanceReadList ) ;

         advanceRead = (FILE4ADVANCE_READ *)l4first( &c4->advanceReadList ) ;
         if ( advanceRead == 0 )   /* maybe got removed by main thread, so none to read... */
         {
            LeaveCriticalSection( &c4->critical4advanceReadList ) ;
            Sleep( 0 ) ;
            continue ;
         }
         advanceRead->usageFlag = r4inUse ;
         LeaveCriticalSection( &c4->critical4advanceReadList ) ;

         file4longAssign( tLong, advanceRead->pos, 0 ) ;
         advanceRead->status = file4readLowDo( advanceRead->file, tLong, advanceRead->data, advanceRead->len ) ;
         advanceRead->usageFlag = r4finished ;  /* outside of critical section, to allow a wait for completion while keeping the critical section */
         EnterCriticalSection( &c4->critical4advanceReadList ) ;
         l4remove( &c4->advanceReadList, advanceRead ) ;
         l4remove( &advanceRead->file->advanceReadFileList, &advanceRead->fileLink ) ;

         /* the completion routine may get reset by another routine which
            needed to call it */
         if ( advanceRead->completionRoutine != 0 )
            advanceRead->completionRoutine( advanceRead ) ;

         /* for reading, the critical section includes the completion routine
            because it checks the status flag before modifying it */
         LeaveCriticalSection( &c4->critical4advanceReadList ) ;
         mem4free( c4->advanceReadMemory, advanceRead ) ;
      }
   }
}

/* this function takes a write request, and ensures that any advance-read
   information which overlaps the write request is updated to reflect the
   changes */
/* if doCancel is true, it means that advance-reads which overlap the positions
   partially will be canceled since they are out of date (i.e. based on a write
   overlap request, not a re-request which is also serviced here) */
void file4advanceReadWriteOver( FILE4 *f4, unsigned long pos, const unsigned len, const void *data, const int doCancel )
{
   FILE4ADVANCE_READ *advanceRead ;
   LINK4 *advanceLink ;
   CODE4 *c4 ;
   #ifndef S4OFF_OPTIMIZE
      OPT4 *opt ;
   #endif

   c4 = f4->codeBase ;

   #ifndef S4OFF_OPTIMIZE
      if ( f4->fileCreated == 0 )  /* ensure file created, else no critical section, can't be advance-reads */
         return ;
   #endif

   if ( pos > ULONG_MAX - len )  /* means we will exceed - i.e. large file */
      return ;

   /* first lock out operations on the advance-read list */
   EnterCriticalSection( &c4->critical4advanceReadList ) ;

   /* first take care of the outstanding advance-reads */
   if ( l4numNodes( &f4->advanceReadFileList ) != 0 )
   {
      for ( advanceLink = (LINK4 *)l4first( &f4->advanceReadFileList ) ;; )
      {
         if ( advanceLink == 0 )
            break ;
         advanceRead = (FILE4ADVANCE_READ *)(advanceLink - 1 ) ;
         advanceLink = (LINK4 *)l4next( &f4->advanceReadFileList, advanceLink ) ;

         if ( ( advanceRead->pos + advanceRead->len ) <= pos ) /* outside of block */
            continue ;
         if ( ( pos + len ) <= advanceRead->pos )  /* outside of block */
            continue ;

         /* if the status is in-use, then just wait for it to finish */
         while ( advanceRead->usageFlag == r4inUse )
            Sleep( 0 ) ;

         /* in this case, the piece is finished, so take care of the completion
            routine ourselves */
         /* once that is finished, it is assumed that the completion routine
            will make it noticed in the handling code below for after-read
            advance-read data */
         if ( advanceRead->usageFlag == r4finished )
         {
            if ( advanceRead->completionRoutine != 0 )
               advanceRead->completionRoutine( advanceRead ) ;
            advanceRead->completionRoutine = 0 ;
            continue ;
         }

         /* now, if the advance piece belongs in the buffer, then read all
            the info before the delay piece, copy the delay piece over,
            and read all the info after the delay piece */

         /* if the entire block is within the range, then can just copy to
            it, otherwise cancel it */
         if ( ( advanceRead->pos >= pos ) && ( pos + len >= advanceRead->pos + advanceRead->len ) ) /* copy it */
         {
            advanceRead->status = 0 ;
            memcpy( advanceRead->data, (const char *)data + (advanceRead->pos - pos ), advanceRead->len ) ;
            advanceRead->usageFlag = r4finished ;  /* outside of critical section, to allow a wait for completion while keeping the critical section */
         }
         else
         {
            if ( doCancel )
               advanceRead->usageFlag = r4canceled ;  /* outside of critical section, to allow a wait for completion while keeping the critical section */
            else
               continue ;
         }

         l4remove( &f4->codeBase->advanceReadList, advanceRead ) ;
         l4remove( &f4->advanceReadFileList, &advanceRead->fileLink ) ;
         if ( advanceRead->completionRoutine != 0 )
            advanceRead->completionRoutine( advanceRead ) ;
         mem4free( f4->codeBase->advanceReadMemory, advanceRead ) ;
      }
   }

   /* now take care of the special-advance-read buffer */
   /* if it is not full, then either it is empty or else we serviced it
      already in the advance-read list */
   if ( f4->advanceReadBufStatus == AR4FULL )
   {
      if ( pos < ( f4->advanceReadBufPos + (long)f4->advanceReadBufLen ) )  /* outside of block */
         if ( ( pos + (long)len ) > f4->advanceReadBufPos )  /* outside of block */
         {
            if ( ( f4->advanceReadBufPos >= pos ) && ( pos + len >= f4->advanceReadBufPos + f4->advanceReadBufLen ) ) /* copy it */
               memcpy( f4->advanceReadBuf, (const char *)data + (f4->advanceReadBufPos - pos ), f4->advanceReadBufLen ) ;
            else
               if ( doCancel )
                  f4->advanceReadBufStatus = AR4EMPTY ;
         }
   }

   /* now take care of the optimized advance-read buffer */
   #ifndef S4OFF_OPTIMIZE
      opt = &c4->opt ;

      /* if it is not full, then either it is empty or else we serviced it
         already in the advance-read list */
      if ( opt->advanceLargeBufferAvail == AR4FULL )
      {
         if ( pos < ( opt->advanceLargePos + opt->advanceLargeLen ) )  /* outside of block */
            if ( ( pos + len ) > opt->advanceLargePos )  /* outside of block */
            {
               if ( ( opt->advanceLargePos >= pos ) && ( pos + len >= opt->advanceLargePos + opt->advanceLargeLen ) ) /* copy it */
                  memcpy( opt->advanceLargeBuffer, (const char *)data + (opt->advanceLargePos - pos ), opt->advanceLargeLen ) ;
               else
                  if ( doCancel )
                     opt->advanceLargeBufferAvail = AR4EMPTY ;
            }
      }
   #endif

   LeaveCriticalSection( &c4->critical4advanceReadList ) ;
}

void S4CALL file4advanceReadBufCompletionRoutine( void *advance )
{
   FILE4ADVANCE_READ *advanceRead ;
   int *arFlag ;

   /* to verify safety, use critical section on the arFlag (from CODE4) */

   advanceRead = (FILE4ADVANCE_READ *)advance ;
   arFlag = (int *)(advanceRead->completionData) ;

   if ( advanceRead->usageFlag == r4canceled )
      *arFlag = AR4EMPTY ;
   else
      if ( *arFlag == AR4SET )  /* if reset to empty it means the read was cancelled by the main thread, so leave as empty */
      {
         /* verify that all was read */
         if ( advanceRead->file->advanceReadBufLen == advanceRead->status )
            *arFlag = AR4FULL ;
         else
            *arFlag = AR4EMPTY ;
      }
}

void opt4advanceReadBuf( FILE4 *f4, unsigned long pos, unsigned len )
{
   CODE4 *c4 ;
   LINK4 *advanceLink ;
   FILE4ADVANCE_READ *advanceRead ;
   #ifndef S4OFF_OPTIMIZE
      long hashVal, adjustedPos ;
      unsigned int extraRead ;
      OPT4 *opt ;
   #endif

   c4 = f4->codeBase ;

   /* if optimization is enabled, then first ensure that the block is not
      already in memory */
   #ifndef S4OFF_OPTIMIZE
      opt = &f4->codeBase->opt ;
      if ( len > opt->blockSize )  /* don't do multi-block reads */
         return ;

      extraRead = (unsigned) ((unsigned long)((unsigned long)pos << opt->numShift ) >> opt->numShift ) ;
      adjustedPos = pos - extraRead ;
      hashVal = opt4fileHash( opt, f4, (unsigned long)adjustedPos ) ;
      if ( opt4fileReturnBlock( f4, pos, hashVal ) != 0 )
         return ;
   #endif

   if ( f4->advanceReadBuf == 0 )
   {
      f4->advanceReadBuf = (char *)u4alloc( len ) ;
      if ( f4->advanceReadBuf == 0 )
         return ;
      f4->advanceReadBufStatus = AR4EMPTY ;
      f4->advanceReadBufLen = len ;
   }
   else
   {
      if ( len > f4->advanceReadBufLen )   /* too large, don't bother */
         return ;
      if ( f4->advanceReadBufStatus == AR4SET || f4->advanceReadBufStatus == AR4FULL )
         if ( f4->advanceReadBufPos == pos )  /* already advanced on this read */
            return ;
   }

   #ifndef S4OFF_OPTIMIZE
      if ( f4->advanceReadBufStatus == AR4FULL )  /* finished read, so place into optimization */
      {
         /* first ensure that it is not already bufferred */
         extraRead = (unsigned) ((unsigned long)((unsigned long)f4->advanceReadBufPos << opt->numShift ) >> opt->numShift ) ;
         adjustedPos = f4->advanceReadBufPos - extraRead ;
         hashVal = opt4fileHash( opt, f4, (unsigned long)adjustedPos ) ;
         if ( opt4fileReturnBlock( f4, adjustedPos, hashVal ) == 0 )
         {
            /* the opt4fileWrite function can be used to place data into memory
               without marking as changed based on last paramater... */
            opt4fileWrite( f4, f4->advanceReadBufPos, f4->advanceReadBufLen, f4->advanceReadBuf, 0 ) ;
         }
         f4->advanceReadBufStatus = AR4EMPTY ;
      }
   #endif

   EnterCriticalSection( &c4->critical4advanceReadList ) ;
   if ( f4->advanceReadBufStatus == AR4SET )  /* must remove from list */
   {
      for ( advanceLink = (LINK4 *)l4first( &f4->advanceReadFileList ) ;; )
      {
         if ( advanceLink == 0 )
            break ;
         advanceRead = (FILE4ADVANCE_READ *)(advanceLink - 1 ) ;
         advanceLink = (LINK4 *)l4next( &f4->advanceReadFileList, advanceLink ) ;

         if ( advanceRead->completionRoutine == file4advanceReadBufCompletionRoutine )  /* spec buf */
         {
            if ( advanceRead->usageFlag != r4queued )  /* must just wait */
            {
               LeaveCriticalSection( &c4->critical4advanceReadList ) ;
               while ( f4->advanceReadBufStatus == AR4SET )
                  Sleep( 0 ) ;
               EnterCriticalSection( &c4->critical4advanceReadList ) ;
               break ;
            }

            /* is queued, so can just remove */
            advanceRead->status = 0 ;
            advanceRead->usageFlag = r4finished ;
            l4remove( &f4->codeBase->advanceReadList, advanceRead ) ;
            l4remove( &f4->advanceReadFileList, &advanceRead->fileLink ) ;
            mem4free( f4->codeBase->advanceReadMemory, advanceRead ) ;
         }
      }
   }

   #ifndef S4OFF_OPTIMIZE
      /* in optimized case, make sure the read is done on a block boundary */
      extraRead = (unsigned) ((unsigned long)((unsigned long)pos << opt->numShift ) >> opt->numShift ) ;
      adjustedPos = pos - extraRead ;
      len = f4->advanceReadBufLen ;
      f4->advanceReadBufStatus = AR4SET ;
      f4->advanceReadBufPos = adjustedPos ;

      LeaveCriticalSection( &c4->critical4advanceReadList ) ;

      file4advanceRead( f4, pos, f4->advanceReadBuf, len, file4advanceReadBufCompletionRoutine, &f4->advanceReadBufStatus ) ;
   #else
      f4->advanceReadBufStatus = AR4SET ;
      f4->advanceReadBufPos = pos ;

      LeaveCriticalSection( &c4->critical4advanceReadList ) ;

      file4advanceRead( f4, pos, f4->advanceReadBuf, len, file4advanceReadBufCompletionRoutine, &f4->advanceReadBufStatus ) ;
   #endif
}

#endif /* S4READ_ADVANCE */
