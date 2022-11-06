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

/* revisited by altomaltes@gmail.com
 */

/* u4util.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"


#ifdef S4TESTING
   #ifdef S4WINDOWS
      #ifndef __WIN32
         #include <dos.h>
      #endif
   #endif
   extern FILE4 s4test ;
#endif

#ifdef S4WINTEL
   #ifndef __WIN32
      #include <sys\timeb.h>
   #endif
#endif
#ifdef __unix__
   #include <sys/times.h>
   #ifndef __linux__
      #ifdef S4NO_USLEEP
         #ifdef S4NO_SELECT
            #include <stropts.h>
            #include <poll.h>
         #else
            #include <sys/select.h>
         #endif
      #endif
   #endif
#endif
#ifdef S4MACINTOSH
   #include <Timer.h>
#endif

#ifdef S4WINDOWS
   #ifdef S4VBASIC
      #ifdef __cplusplus
         extern "C" {
      #endif

      long S4FUNCTION v4Cstring(char *) ;
      void S4FUNCTION v4Cstringfree(char *) ;

      #ifdef __cplusplus
         }
      #endif
   #endif
#endif

/* '*wasOldLen' contains the old len and will contain the new.
   'newLen' contains the new length.
   memory is only allocated if the  'newLen' paramater is greater than the
   wasOldLen paramater.
*/
int S4FUNCTION u4allocAgainDefault( CODE4 *c4, char **ptrPtr, unsigned *wasOldLen, const unsigned newLen )
{
   char *newPtr ;

   #ifdef E4PARM_HIGH
      if ( *ptrPtr == 0 && *wasOldLen != 0 )
         return error4( c4, e4parm, E94501 ) ;
   #endif

   if ( newLen <= *wasOldLen )
      return 0 ;

   newPtr = (char *)u4allocFreeDefault( c4, (long)newLen ) ;

   if ( newPtr == 0 )
      return error4stack( c4, e4memory, E94501 ) ;

   if ( *ptrPtr != 0 )
   {
      memcpy( newPtr, *ptrPtr, (size_t) *wasOldLen ) ;
      u4free( *ptrPtr ) ;
   }
   *ptrPtr = newPtr ;
   *wasOldLen = newLen ;
   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif

void *S4FUNCTION u4allocFreeDefault( CODE4 *c4, long n )
{
   void *ptr ;

   #ifdef S4SEMAPHORE
      #ifdef E4MISC
         /* the debug u4alloc uses global tracking pointers, so semaphore */
         if ( mem4start( c4 ) != 0 )
            return 0 ;
      #endif
   #endif

   ptr = (void *)u4allocDefault( n ) ;

   #ifndef S4OPTIMIZE_OFF
      if ( ptr == 0 && c4 )
         if ( c4->hasOpt )
         {
            code4optSuspend( c4 ) ;
            ptr = (void *)u4allocDefault( n ) ;
            code4optRestart( c4 ) ;
         }
   #endif

   #ifdef S4SEMAPHORE
      #ifdef E4MISC
         mem4stop( c4 ) ;
      #endif
   #endif

   return ptr ;
}

void S4FUNCTION u4delayHundredth( const unsigned int numHundredths )
{
   #ifndef __WIN32
      #ifndef __unix__
         int sec ;
         unsigned int hundredths ;
      #endif
      #ifdef S4WINTEL
         struct timeb oldTime, newTime ;
      #endif
      #ifdef S4WIN16
            MSG msg;
      #endif
      #ifdef __unix__
         #ifdef S4NO_USLEEP
            #ifndef S4NO_SELECT
               struct timeval waitTime ;
            #endif
         #else
            int sec ;
            u_int hundredths ;
         #endif
      #endif
      #ifdef S4MACINTOSH
          UnsignedWide oldTime, newTime ;
      #endif
   #endif

   #ifdef __unix__
      #ifdef S4NO_USLEEP
         #ifndef S4NO_SELECT
            waitTime.tv_sec = numHundredths / 100 ;
            waitTime.tv_usec = (numHundredths % 100 ) * 10000 ;
         #endif
      #endif
   #endif
   #ifdef S4WINTEL
      #ifndef __WIN32
         ftime( &oldTime) ;
         sec = numHundredths / 100 ;
         hundredths = numHundredths % 100 ;
      #endif
   #endif
   #ifdef S4MACINTOSH
      Microseconds( &oldTime ) ;
      sec = numHundredths / 429497 ;
      hundredths = numHundredths % 429497 ;
   #endif

   #ifdef S4WIN16
      for ( ;; )
      {
         /* Give some other application a chance to run. */
            if ( PeekMessage( &msg, (HWND)NULL, (UINT)0, (UINT)0, PM_REMOVE ) )
            {
               /* can't allow re-entrancy, so if not a paint message, just
                  discard.  Allow paints messages to give up control to other
                  apps, but client's calling CodeBase calls on WM_PAINT
                  messages may fail */
               if ( msg.message == WM_PAINT )
               {
                  TranslateMessage((LPMSG)&msg) ;
                  DispatchMessage((LPMSG)&msg) ;
               }
            }

         /* allow device drivers to run... */
         _asm mov ax, 0x1689
         _asm mov bl, 1
         _asm int 0x2f

         ftime( &newTime ) ;

         if ( newTime.time - oldTime.time > ( sec + 1 ) )  /* > 1 secord over */
            break ;

         if ( newTime.time - oldTime.time >= sec )
            if ( ( (long)(newTime.millitm - oldTime.millitm ) / 10 ) > (long)hundredths )
               break ;
     }
   #else
      #ifdef __unix__
         #ifdef S4NO_USLEEP
            #ifdef S4NO_SELECT
               poll(0, NULL, numHundredths * 10 ) ;
            #else
               select(0, NULL, NULL, NULL, &waitTime ) ;
            #endif
         #else
            if (numHundredths >= 100)
            {
               sec = numHundredths / 100 ;
               hundredths = (numHundredths % 100)*10000 ;
               sleep(sec) ;
            }
            else
               hundredths = numHundredths * 10000 ;
            usleep(hundredths ) ;
         #endif
      #else
         #ifdef __WIN32
            Sleep( 10 * numHundredths ) ;
         #else
            for ( ;; )
            {
               #ifdef S4WINTEL
                  ftime( &newTime ) ;

                  if ( newTime.time - oldTime.time >= sec )
                     if ( (unsigned int)(( (newTime.millitm - oldTime.millitm ) / 10 )) > hundredths )
                        break ;
               #endif
               #ifdef S4MACINTOSH
                   Microseconds(&newTime ) ;
                   if (newTime.hi - oldTime.hi > (sec + 1) )
                      break ;
                   if (newTime.hi - oldTime.hi >= sec )
                      if( (newTime.lo - oldTime.lo ) > hundredths )
                          break;
               #endif
            }
         #endif
      #endif
   #endif
   return ;
}

#ifndef S4INLINE
/* delays one second, allows other windows applications to run, etc. */
void u4delaySec()
{
   u4delayHundredth( 100 ) ;
}
#endif
#ifndef S4WINCE
char *u4environ( char *find, const int doErr )
{
   char *env ;

   #ifdef E4PARM_HIGH
      if ( find == 0 )
      {
         error4( 0, e4parm_null, E94503 ) ;
         return 0 ;
      }
   #endif

   env = getenv( find ) ;

   if ( env == 0 && doErr )
   {
      error4describe( 0, e4info, E84501, find, 0, 0 ) ;
      return 0 ;
   }

   return env ;
}
#endif
unsigned int S4FUNCTION u4ncpy( char *to, const char *from, const unsigned int l )
{
   unsigned i, len ;

   #ifdef E4PARM_HIGH
      if ( l == 0 )
      {
         error4( 0, e4parm, E94502 ) ;
         return 0 ;
      }
   #endif

   len = l - 1 ;
   for ( i = 0;; i++ )
   {
      if ( i >= len )
      {
         to[len] = 0 ;
         return len ;
      }
      to[i] = from[i] ;
      if ( from[i] == 0 )
         return i ;
   }
}

#ifndef S4INLINE
int S4FUNCTION u4ptrEqual( const void *p1, const void *p2 )
{
   return( p1 == p2 ) ;
}
#endif

#ifdef S4TESTING
#ifdef S4WINDOWS
PUBLIC void S4FUNCTION u4terminate( void )
{
   #ifdef __WIN32
      ExitProcess(1) ;
   #else
      union REGS terminate ;

      terminate.h.ah = 0x4C ;

      intdos( &terminate, &terminate ) ;
   #endif
}
#endif  /* S4WINDOWS */

void u4setField( const char *serverId, const char *database, const long recno, const char *fieldName, const char *newValue )
{
   #ifndef S4OFF_WRITE
   CODE4 cb ;
   DATA4 *data ;
   FIELD4 *field ;
   int openAttempts ;

   if ( database == 0 || recno == 0 || fieldName == 0 || newValue == 0 )
      return ;

   code4init( &cb ) ;
   cb.errOff = 1 ;

      code4logOpenOff( &cb ) ;

   #ifndef S4OFF_MULTI
      cb.accessMode = OPEN4DENY_RW ;       /* open exclusively */
   #endif
   cb.errOpen = 0 ;
   cb.autoOpen = 0 ;

   data = d4open( &cb, database ) ;
   for ( openAttempts = 0 ; ( data == NULL ) && ( openAttempts < 300 ) ; openAttempts++ )
   {
      u4delayHundredth( 100 ) ;   /* wait one second and try again */
      data = d4open( &cb, database ) ;
   }
   if ( data == NULL )
   {
      code4close(&cb) ;
      code4initUndo(&cb) ;
      return ;   /* should return an error or something  */
   }

   #ifndef S4OFF_MULTI
      cb.lockAttempts = 300 ;
      cb.lockDelay = 100 ;     /* shouldn't have to wait--just in case */

      if ( d4lockFile( data ) != 0 )
      {
         d4close( data ) ;
         code4close(&cb) ;
         code4initUndo(&cb) ;
         return ;
      }
   #endif

   if ( (field = d4field( data, fieldName )) == 0 )
   {
      d4close( data ) ;
      code4close(&cb) ;
      code4initUndo(&cb) ;
      return ;
   }

   if ( d4go( data, recno ) != 0 )
   {
      d4close( data ) ;
      code4close(&cb) ;
      code4initUndo(&cb) ;
      return ;
   }

   f4assign( field, newValue ) ;

   if ( d4flushData( data ) != 0 )
   {
      d4close( data ) ;
      code4close(&cb) ;
      code4initUndo(&cb) ;
      return ;
   }

   d4close( data ) ;
   code4close(&cb) ;
   code4initUndo(&cb) ;
   #endif /* S4OFF_WRITE */
}

int ats4readInfo( char *filename, void *info, const unsigned int len )
{
   CODE4 cb ;
   FILE4 f ;
   FILE4LONG fpos ;

   code4init( &cb ) ;
      code4logOpenOff( &cb ) ;

   cb.errOff = 1 ;
   cb.errOpen = 0 ;   /* avoid endless loop due to error calling this function... */
   if ( file4open( &f, &cb, filename, 1 ) )
   {
      code4initUndo( &cb ) ;
      return 0 ;   /* info could not be read */
   }

   file4longAssign( fpos, 0, 0 ) ;

   file4readInternal( &f, fpos, info, len ) ;
   file4close( &f ) ;

   code4close( &cb ) ;
   code4initUndo( &cb ) ;

   return 1 ;   /* info successfully read */
}

PUBLIC void S4FUNCTION ats4setSuiteStatus( const char *newValue )
{
   ATS4RECINFO info ;
   const char *fieldStatus = "STATUS" ;
   const char *fieldRDate = "RDATE" ;
   char *serverId, rdate[8] ;

   if ( newValue == 0 )
      return ;

   serverId = 0 ;


   date4today( rdate ) ;

   if( getenv( "T4FLAG" ) )
   {
      if ( strcmp( getenv( "T4FLAG" ), "T" ) == 0 )
      {
         u4setField( serverId, getenv( "T4SUITE" ), atol( getenv( "T4RECNO" ) ), fieldStatus, newValue ) ;
         u4setField( serverId, getenv( "T4SUITE" ), atol( getenv( "T4RECNO" ) ), fieldRDate, rdate ) ;
      }
   }
   else
   {
      if ( ats4readInfo( ATS_FILENAME_REC, &info, sizeof( info ) ) )
      {
         u4setField( serverId, (char *)info.T4SUITE, atol((char *)info.T4RECNO), fieldStatus, newValue ) ;
         u4setField( serverId, (char *)info.T4SUITE, atol((char *)info.T4RECNO), fieldRDate, rdate ) ;
      }
   }
}
#endif /* S4TESTING */

#ifdef S4MEM_PRINT
   int in4temp = 0 ;
   char *write4buf = 0 ;
   FILE4SEQ_WRITE file4seq ;
   FILE4SEQ_WRITE *file4seqPtr ;
#endif

#ifdef S4TESTING

void u4writeOut( char *out, int newLine, long lenIn )
{
   int errCode, flushCode, loop ;
   unsigned char val, intVal ;
   FILE4LONG len ;
   FILE4LONG pos ;
   char buf[10] ;
   #ifdef S4TESTING
         int undoLocal = 0 ;
   #endif


   #ifdef S4MEM_PRINT
      if ( in4temp == 1 )
         return ;
   #endif

   if( s4test.hand == INVALID4HANDLE )  /* even for non-testing just ensure else return */
      return ;


   if ( s4test.codeBase != 0 )
   {
      errCode = error4code( s4test.codeBase ) ;
      error4set( s4test.codeBase, 0 ) ;
   }

   len = file4lenLow( &s4test ) ;

   if ( file4longError( len ) != 0 )
   {
      #ifdef S4MEM_PRINT
         if ( write4buf == 0 )
         {
            in4temp = 1 ;
            write4buf = u4alloc( 40000L ) ;
            in4temp = 0 ;
            if ( write4buf != 0 )
            {
               file4seqWriteInit( &file4seq, &s4test, len, write4buf, (unsigned int)40000 ) ;
               file4seqPtr = &file4seq ;
            }
         }
      #endif

      if ( newLine )
      {
         #ifdef S4MEM_PRINT
            if ( write4buf != 0 )
            {
               file4seqWrite( &file4seq, "\r\n", 2 ) ;
               len += 2 ;
            }
            else
            {
         #endif
            file4longAssignLong( pos, len ) ;
            file4writeInternal( &s4test, pos, "\r\n", 2 ) ;
            file4longAdd( &len, 2 ) ;
         #ifdef S4MEM_PRINT
            }
         #endif
      }
      #ifdef S4MEM_PRINT
         if ( write4buf != 0 )
         {
            for ( loop = 0 ; loop < lenIn ; loop++ )
            {
               val = out[loop] ;
               if ( val >= '0' && val <= '9' )
               {
                  file4seqWrite( &file4seq, &val, 1 ) ;
                  len++ ;
               }
               else
               {
                  buf[0] = '0' ;
                  buf[1] = 'x' ;
                  intVal = val / 100 ;
                  buf[2] = intVal + '0' ;
                  val -= intVal * 100 ;
                  intVal = val / 10 ;
                  buf[3] = intVal + '0' ;
                  val -= intVal * 10 ;
                  buf[4] = val + '0' ;
                  buf[5] = ' ' ;
                  file4seqWrite( &file4seq, buf, 6 ) ) ;
                  len += 6 ;
               }
            }
         }
         else
         {
      #endif
         flushCode = s4test.codeBase->fileFlush ;
         s4test.codeBase->fileFlush = 1 ;
         for ( loop = 0 ; loop < lenIn ; loop++ )
         {
            val = out[loop] ;
            if ( val >= '0' && val <= '9' )
            {
               file4longAssignLong( pos, len ) ;
               file4writeInternal( &s4test, pos, &val, 1 ) ;
               file4longAdd( &len, 1 ) ;
            }
            else
            {
               intVal = val / 100 ;
               buf[0] = '0' ;
               buf[1] = 'x' ;
               buf[2] = intVal + '0' ;
               val -= intVal * 100 ;
               intVal = val / 10 ;
               buf[3] = intVal + '0' ;
               val -= intVal * 10 ;
               buf[4] = val + '0' ;
               buf[4] = ' ' ;
               file4longAssignLong( pos, len ) ;
               file4writeInternal( &s4test, pos, (void *)buf, 6 ) ;
               file4longAdd( &len, 6 ) ;
            }

         }
         file4flush( &s4test ) ;
         s4test.codeBase->fileFlush = flushCode ;
      #ifdef S4MEM_PRINT
         }
      #endif
   }

   error4set( s4test.codeBase, errCode ) ;

}
#endif

#ifdef S4TESTING
   int ats4errorFlag = 0 ;   /* used to prevent infinite loops */

   #define INC_U4WRITEERR
   void S4FUNCTION u4writeErr( const char S4PTR *errStr, int newLine )
#else
   #ifdef S4TRACK_FILES
      #define INC_U4WRITEERR
      void u4writeErr( const char *errStr, int newLine )
   #endif
#endif
#ifdef INC_U4WRITEERR
{
   int errCode, flushCode ;
   FILE4LONG len, len2 ;
   #ifdef S4TESTING
         CODE4 cb ;
         int undoLocal = 0 ;
   #endif


   #ifdef S4TESTING
         if ( ats4errorFlag == 0 )
         {
            ats4errorFlag = 1 ;
            ats4setSuiteStatus( "E" ) ;
            ats4errorFlag = 0 ;
         }
   #endif

   #ifdef S4MEM_PRINT
      if ( in4temp == 1 )
         return ;
   #endif


   if ( s4test.hand == INVALID4HANDLE )
   {
      #ifdef S4TESTING
            /* reopen the log if it has been previously closed:   */
            code4init(&cb) ;
            /* the code4init() call should have opened or created s4test  */
               code4logOpenOff( &cb ) ;
            if( s4test.hand == INVALID4HANDLE )
            {
               code4close(&cb) ;
               code4initUndo(&cb) ;
               return ;
            }
            undoLocal = 1 ;
      #endif
   }

   if( s4test.hand == INVALID4HANDLE )  /* even for non-testing just ensure else return */
   {
      return ;
   }

   if ( s4test.codeBase != 0 )
   {
      errCode = error4code( s4test.codeBase ) ;
      error4set( s4test.codeBase, 0 ) ;
   }

   len = file4lenLow( &s4test ) ;

   if( file4longError( len ) >= 0 )
   {
      #ifdef S4MEM_PRINT
         if ( write4buf == 0 )
         {
            in4temp = 1 ;
            write4buf = u4alloc( 40000L ) ;
            in4temp = 0 ;
            if ( write4buf != 0 )
            {
               file4seqWriteInit( &file4seq, &s4test, len, write4buf, (unsigned int)40000 ) ;
               file4seqPtr = &file4seq ;
            }
         }
      #endif

      if ( newLine )
      {
         #ifdef S4MEM_PRINT
            if ( write4buf != 0 )
            {
               file4seqWrite( &file4seq, "\r\n", 2 ) ;
               len += 2 ;
            }
            else
            {
         #endif
            file4writeInternal( &s4test, len, "\r\n", 2 ) ;
            file4longAdd( &len, 2 ) ;
         #ifdef S4MEM_PRINT
            }
         #endif
      }
      #ifdef S4MEM_PRINT
         if ( write4buf != 0 )
         {
            file4seqWrite( &file4seq, errStr, strlen( errStr ) ) ;
         }
         else
         {
      #endif
         flushCode = s4test.codeBase->fileFlush ;
         s4test.codeBase->fileFlush = 1 ;
         file4longAssignLong( len2, len ) ;
         file4longAdd( &len2, strlen( errStr ) ) ;
         file4lenSetLow( &s4test, len2 ) ;
         file4writeInternal( &s4test, len, (void *)errStr, strlen( errStr ) ) ;
         file4flush( &s4test ) ;
         s4test.codeBase->fileFlush = flushCode ;
      #ifdef S4MEM_PRINT
         }
      #endif
   }

   error4set( s4test.codeBase, errCode ) ;

   #ifdef S4TESTING
         if ( undoLocal )
         {
            code4close(&cb) ;
            code4initUndo(&cb) ;
         }

   #endif
}
#undef INC_U4WRITEERR
#endif

void S4FUNCTION u4yymmdd( char *yymmdd )
{
#ifdef S4WINCE
   SYSTEMTIME st ;
   GetLocalTime( &st ) ;
   // AS 04/30/98 year 2000 issue, fox uses '0x00', dBASE '0x64'
   #ifdef S4FOX
      yymmdd[0] = (char)st.wYear % 100 ;
   #else
      yymmdd[0] = (char)st.wYear -1900 ;
   #endif
   yymmdd[1] = (char)st.wMonth ;
   yymmdd[2] = (char)st.wDay ;
#else
   #ifdef __unix___THREADS
      time_t timeVal ;
      struct tm result ;

      time( (time_t *) &timeVal ) ;
      localtime_r( (time_t *) &timeVal, &result) ;
      // AS 04/30/98 year 2000 issue, fox uses '0x00', dBASE '0x64'
      #ifdef S4FOX
         yymmdd[0] = (char)(result.tm_year % 100) ;
      #else
         yymmdd[0] = (char)result.tm_year ;
      #endif
      yymmdd[1] = (char)(result.tm_mon + 1) ;
      yymmdd[2] = (char)result.tm_mday ;
   #else
      time_t timeVal ;
      struct tm *tmPtr ;

      time( (time_t *) &timeVal ) ;
      tmPtr =  localtime( (time_t *) &timeVal ) ;
      /* AS 04/30/98 year 2000 issue, fox uses '0x00', dBASE '0x64' */
      #ifdef S4FOX
         yymmdd[0] = (char)(tmPtr->tm_year % 100) ;
      #else
         yymmdd[0] = (char)tmPtr->tm_year ;
      #endif
      yymmdd[1] = (char)(tmPtr->tm_mon + 1) ;
      yymmdd[2] = (char)tmPtr->tm_mday ;
   #endif
#endif
}

int S4FUNCTION u4remove( const char *ptr )
{
   #ifdef S4UNICODE
      unsigned short name[256] ;
   #endif

   #ifdef E4PARM_LOW
      if ( ptr == 0 )
         return error4( 0, e4parm_null, E94504 ) ;
   #endif

   #ifdef S4UNICODE
      c4atou(ptr, name, 256) ;
      return DeleteFile(name) ;
   #else
      #ifdef S4NO_REMOVE
         return( unlink( ptr ) ) ;
      #else
         return remove( ptr ) ;
      #endif
   #endif
}

int u4rename( const char *oldName, const char *newName )
{
   #ifdef S4UNICODE
      unsigned short from[256], to[256] ;
   #endif
   #ifdef S4NO_RENAME
      char  buf[250] ;

      memset( (void *)buf, 0, sizeof(buf) ) ;
   #endif

   #ifdef E4PARM_LOW
      if ( oldName == 0 || newName == 0 )
         return error4( 0, e4parm_null, E94505 ) ;
   #endif

   #ifdef S4UNICODE
      c4atou(oldName, from, 256) ;
      c4atou(newName, to, 256 ) ;
      return MoveFile(from, to ) ;
   #else
      #ifdef S4NO_RENAME
         #ifdef __unix__
            memcpy( (void *)buf, "mv ", 3 ) ; /* system rename or move call */
         #else
            memcpy( (void *)buf, "rename ", 7 ) ; /* system copy call */
         #endif
         strcat( buf, oldName ) ;
         strcat( buf, " " ) ;
         strcat( buf, newName ) ;
         return system( buf ) ;
      #else
         return rename( oldName, newName ) ;
      #endif
   #endif
}

#ifdef S4VB_DOS

int S4FUNCTION u4ncpy_v( char *to, char *from, int len )
{
   return u4ncpy( StringAddress(to), from, (unsigned)len ) ;
}

int S4FUNCTION u4ncpy_v2( char *to, char *from, int len )
{
   return u4ncpy( to, c4str(from), len ) ;
}

#endif


#ifdef S4WINDOWS
   #ifdef S4VBASIC
      long S4FUNCTION v4Cstring(char *s)
      {
         char *d = 0 ;

         if( s )
         {
            d= (char *) u4alloc(1L + (long)strlen(s));
            strcpy(d,s);
         }
         return (long) d;
      }

      void S4FUNCTION v4Cstringfree(char *s)
      {
         if( s )
         {
           u4free(s);
         }
      }
   #endif   /* S4VBASIC */
#endif   /* S4WINDOWS */

#ifndef S4FOX
   /*   memory comparison routines for foreign languages */

   /*  v4map structure :  There are three lines of numbers in each row.
                          The upper line (commented out) represents ascii
                          characters.  The middle line represents the
                          ascii value.  The lower line is the precedence value
                          of the corresponding ascii value as compared to
                          the other ascii values. There are two v4map
                          structures: one uses the ANSI (Windows) char set,
                          which will handle English, French and German.
                          The second is an OEM (ASCII) set for German and
                          French character sets.
   */
   #ifdef S4ANSI
      unsigned char v4map[256] =
      {
         /*     ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤ */
         /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
              194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,

         /*     ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤ */
         /*    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 */
              210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,

         /*         !   "   #   $   %   &   '   (   )   *   +   ,   -   .   / */
         /*    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 */
                1,156,162,170,157,135,154,163,123,124,136,132,167,133,152,153,

         /*     0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ? */
         /*    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 */
                3,  4,  5,  6,  7,  8,  9, 10, 11, 12,168,169,129,131,130,171,

         /*     @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O */
         /*    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79 */
              174, 13, 20, 21, 23, 24, 29, 30, 31, 32, 37, 39, 40, 41, 42, 44,

         /*     P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _ */
         /*    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95 */
               50, 51, 52, 53, 54, 55, 60, 61, 62, 63, 66,125,172,126,151,173,

         /*     `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o */
         /*    96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111 */
              164, 67, 74, 75, 77, 78, 83, 84, 85, 86, 91, 93, 94, 95, 96, 98,

         /*     p   q   r   s   t   u   v   w   x   y   z   {   |   }       ≤ */
         /*   112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127 */
              104,105,106,107,109,110,115,116,117,118,122,127,155,128,150,226,

         /*     ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤ */
         /*   128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143 */
              227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,

         /*     ≤   `   '   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤   ≤ */
         /*   144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159 */
              243,165,166,244,245,246,247,248,249,250,251,252,253,254,255,255,

         /*         ≠   õ   ú XXX  ù    |      " XXX   ¶   Æ   ™   - XXX XXX */
         /*   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175 */
                2,175,158,159,160,161,176,177,178,179,181,147,149,134,180,183,

         /*     ¯   Ò   ˝ XXX   '   Ê    XXX   , XXX   ß   Ø   ¨   ´ XXX   ® */
         /*   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191 */
              140,139,142,143,184,185,186,187,188,141,182,148,144,145,146,189,

         /*     A   A   A   A   é   è   í   Ä   E   ê   E   E   I   I   I   I */
         /*   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207 */
               16, 15, 17, 19, 14, 18,192, 22, 27, 26, 28, 25, 35, 34, 36, 33,

         /*     D   •   O   O   O   O   ô   X   0   U   U   U   ö   Y   b   · */
         /*   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223 */
               38, 43, 47, 46, 48, 49, 45,137,190, 58, 57, 59, 56, 64, 65,108,

         /*     Ö   †   É   a   Ñ   Ü   ë   á   ä   Ç   à   â   ç   °   å   ã */
         /*   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239 */
               70, 69, 71, 73, 68, 72,193, 76, 81, 80, 82, 79, 89, 88, 90, 87,

         /*     Â   §   ï   ¢   ì   o   î   ˆ   0   ó   £   ñ   Å   y   b   ò */
         /*   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 */
               92, 97,101,100,102,103, 99,138,191,113,112,114,111,120,121,119,
      } ;

      #ifndef S4LANGUAGE
         int S4CALL u4memcmp( S4CMP_PARM s1, S4CMP_PARM s2, size_t len )
         {
            unsigned int i ;

            for (i=0; i<len; i++)
               if ( ((unsigned char *)s1)[i] != ((unsigned char *)s2)[i] )
               {
                  if ( v4map[((unsigned char *)s1)[i]] < v4map[((unsigned char *)s2)[i]] )
                     return -1 ;
                  return 1 ;
               }

            return 0 ;
         }
      #endif
   #endif

   #ifdef S4LANGUAGE
      #ifdef S4GERMAN
         #ifdef S4ANSI
            typedef struct
            {
               unsigned char extChar ;
               char expChars[3] ;
            } LANGUAGE_CONVERT ;

            LANGUAGE_CONVERT v4table[] =
            {
               { 246, "oe" },  /* î */
               { 223, "ss" },  /* · */
               { 228, "ae" },  /* Ñ */
               { 252, "ue" },  /* Å */
               { 235, "ee" },  /* â */
               #ifdef S4CLIPPER
                  { 196, "AE" },  /* é */
                  { 214, "OE" },  /* ô */
                  { 220, "UE" },  /* ö */
               #else
                  { 196, "Ae" },
                  { 214, "Oe" },
                  { 220, "Ue" },
               #endif
               { 233, "e " },  /* Ç */
               { 226, "a " },  /* É */
               { 224, "a " },  /* Ö */
               { 229, "a " },  /* Ü */
               { 234, "e " },  /* à */
               { 232, "e " },  /* ä */
               { 238, "i " },  /* å */
               { 236, "i " },  /* ç */
               { 197, "A " },  /* è */
               /* The angstrom is not indexed in German correctly, so this is used instead*/
               { 244, "o " },  /* ì */
               { 242, "o " },  /* ï */
               { 251, "u " },  /* ñ */
               { 249, "u " },  /* ó */
               { 255, "y " },  /* ò */
               { 225, "a " },  /* † */
               { 237, "i " },  /* ° */
               { 243, "o " },  /* ¢ */
               { 250, "u " },  /* £ */
               { 241, "n " },  /* § */
               {   0, "  " },   /* A blank entry to make the u4valid work better */
            };
         #else  /* ifndef S4ANSI  */
            unsigned char v4map[256] =
            {
               #ifdef S4CLIPPER
                  /*      ,  ,  ,  ,  ,  ,  ,  ,  ,   ,   ,  ,  ,   ,  ,   */
                  /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
                         0,  1,  2,  3,  4,  5,  6,  7,  8, 30,  9, 10, 11, 31, 12, 13,

                  /*     ,  ,  ,  ,  ,  ,  ,  ,  ,  ,EOF,  ,  ,  ,  ,   */
                  /*    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 */
                        14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,

                  /*      ,  !,  ",  #,  $,  %,  &,  ',  (,  ),  *,  +,  ,,  -,  .,  / */
                  /*    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 */
                        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,

                  /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  :,  ;,  <,  =,  >,  ? */
                  /*    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 */
                        48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,

                  /*     @,  A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O */
                  /*    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79 */
                        64, 65, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,

                  /*     P,  Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z,  [,  \,  ],  ^,  _ */
                  /*    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95 */
                        82, 83, 84, 85, 86, 87, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98,

                  /*     `,  a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o */
                  /*    96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111 */
                        99,100,102,103,104,105,106,107,108,109,110,111,112,113,114,115,

                  /*     p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z,  {,  |,  },   ,   */
                  /*   112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127 */
                       117,118,119,120,122,123,125,126,127,128,129,130,131,132,133,134,

                  /*     Ä,  Å,  Ç,  É,  Ñ,  Ö,  Ü,  á,  à,  â,  ä,  ã,  å,  ç,  é,  è */
                  /*   128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143 */
                       135,124,136,137,101,138,139,140,141,142,143,144,145,146, 66,147,

                  /*     ê,  ë,  í,  ì,  î,  ï,  ñ,  ó,  ò,  ô,  ö,  õ,  ú,  ù,  û,  ü */
                  /*   144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159 */
                       148,149,150,151,116,152,153,154,155, 81, 88,156,157,158,159,160,

                  /*     †,  °,  ¢,  £,  §,  •,  ¶,  ß,  ®,  ©,  ™,  ´,  ¨,  ≠,  Æ,  Ø */
                  /*   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175 */
                       161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,

                  /*     ∞,  ±,  ≤,  ≥,  ¥,  µ,  ∂,  ∑,  ∏,  π,  ∫,  ª,  º,  Ω,  æ,  ø */
                  /*   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191 */
                       177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,

                  /*     ¿,  ¡,  ¬,  √,  ƒ,  ≈,  ∆,  «,  »,  …,   ,  À,  Ã,  Õ,  Œ,  œ */
                  /*   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207 */
                       193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,

                  /*     –,  —,  “,  ”,  ‘,  ’,  ÷,  ◊,  ÿ,  Ÿ,  ⁄,  €,  ‹,  ›,  ﬁ,  ﬂ */
                  /*   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223 */
                       209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,

                  /*     ‡,  ·,  ‚,  „,  ‰,  Â,  Ê,  Á,  Ë,  È,  Í,  Î,  Ï,  Ì,  Ó,  Ô */
                  /*   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239 */
                       225,121,226,227,228,229,230,231,232,233,234,235,236,237,238,239,

                  /*     ,  Ò,  Ú,  Û,  Ù,  ı,  ˆ,  ˜,  ¯,  ˘,  ˙,  ˚,  ¸,  ˝,  ˛,    */
                  /*   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 */
                       240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
               #else
                  /*      ,  ,  ,  ,  ,  ,  ,  ,  ,   ,   ,  ,  ,   ,  ,   */
                  /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
                         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,

                  /*     ,  ,  ,  ,  ,  ,  ,  ,  ,  ,EOF,  ,  ,  ,  ,   */
                  /*    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 */
                        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,

                  /*      ,  !,  ",  #,  $,  %,  &,  ',  (,  ),  *,  +,  ,,  -,  .,  / */
                  /*    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 */
                        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,

                  /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  :,  ;,  <,  =,  >,  ? */
                  /*    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 */
                       158,159,160,161,162,163,164,165,166,167, 48, 49, 50, 51, 52, 53,

                  /*     @,  A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O */
                  /*    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79 */
                        54,168,172,173,175,176,178,179,180,181,182,183,184,185,186,188,

                  /*     P,  Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z,  [,  \,  ],  ^,  _ */
                  /*    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95 */
                       190,191,192,193,194,195,197,198,199,200,201, 55, 56, 57, 58, 59,

                  /*     `,  a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o */
                  /*    96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111 */
                        60,202,210,211,213,214,219,220,221,222,227,228,229,230,231,233,

                  /*     p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z,  {,  |,  },   ,   */
                  /*   112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127 */
                       239,240,241,242,244,245,250,251,252,253,255, 61, 62, 63, 64, 65,

                  /*     Ä,  Å,  Ç,  É,  Ñ,  Ö,  Ü,  á,  à,  â,  ä,  ã,  å,  ç,  é,  è */
                  /*   128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143 */
                       174,249,215,205,206,204,207,212,217,218,216,226,225,224,169,170,

                  /*     ê,  ë,  í,  ì,  î,  ï,  ñ,  ó,  ò,  ô,  ö,  õ,  ú,  ù,  û,  ü */
                  /*   144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159 */
                       177,208,171,236,237,235,248,247,254,189,196, 66, 67, 68, 69, 70,

                  /*     †,  °,  ¢,  £,  §,  •,  ¶,  ß,  ®,  ©,  ™,  ´,  ¨,  ≠,  Æ,  Ø */
                  /*   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175 */
                       203,223,234,246,232,187,209,238, 71, 72, 73, 74, 75, 76, 77, 78,

                  /*     ∞,  ±,  ≤,  ≥,  ¥,  µ,  ∂,  ∑,  ∏,  π,  ∫,  ª,  º,  Ω,  æ,  ø */
                  /*   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191 */
                        79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94,

                  /*     ¿,  ¡,  ¬,  √,  ƒ,  ≈,  ∆,  «,  »,  …,   ,  À,  Ã,  Õ,  Œ,  œ */
                  /*   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207 */
                        95, 96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,

                  /*     –,  —,  “,  ”,  ‘,  ’,  ÷,  ◊,  ÿ,  Ÿ,  ⁄,  €,  ‹,  ›,  ﬁ,  ﬂ */
                  /*   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223 */
                       111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,

                  /*     ‡,  ·,  ‚,  „,  ‰,  Â,  Ê,  Á,  Ë,  È,  Í,  Î,  Ï,  Ì,  Ó,  Ô */
                  /*   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239 */
                       127,243,128,129,130,131,132,133,134,135,136,137,138,139,140,141,

                  /*     ,  Ò,  Ú,  Û,  Ù,  ı,  ˆ,  ˜,  ¯,  ˘,  ˙,  ˚,  ¸,  ˝,  ˛,    */
                  /*   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 */
                       142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157
               #endif /* S4CLIPPER */
            } ;

            typedef struct
            {
               unsigned char extChar ;
               char expChars[3] ;
            } LANGUAGE_CONVERT ;

            LANGUAGE_CONVERT v4table[] =
            {
               { 148, "oe" },
               { 225, "ss" },
               { 132, "ae" },
               { 129, "ue" },
               { 130, "ee" },
               #ifdef S4CLIPPER
                  { 142, "AE" },
                  { 153, "OE" },
                  { 154, "UE" },
               #else
                  { 142, "Ae" },
                  { 153, "Oe" },
                  { 154, "Ue" },
               #endif
               { 131, "a " },
               { 133, "a " },
               { 134, "a " },
               { 136, "e " },
               { 137, "e " },
               { 138, "e " },
               { 140, "i " },
               { 141, "i " },
               { 143, "A " },
               /* The angstrom is not indexed in German correctly, so this is used instead*/
               { 147, "o " },
               { 149, "o " },
               { 150, "u " },
               { 151, "u " },
               { 152, "y " },
               { 160, "a " },
               { 161, "i " },
               { 162, "o " },
               { 163, "u " },
               { 164, "n " },
               {   0, "  " },   /* A blank entry to make the u4valid work better */
            } ;
         #endif /* S4ANSI */

         #ifdef S4DICTIONARY
            /* sort method uses the dictionary sort order */
            int S4CALL u4memcmp( S4CMP_PARM s1, S4CMP_PARM s2, size_t len )
            {
               int i ;

               for ( i = 0 ; i < len ; i++ )
                  if ( ((unsigned char *)s1)[i] != ((unsigned char *)s2)[i] )
                  {
                     if ( v4map[((unsigned char *)s1)[i]] < v4map[((unsigned char *)s2)[i]] )
                        return -1 ;
                     return 1 ;
                  }

               return 0 ;
            }
         #else   /* if PHONEBOOK SORT (default) */
            /* sort method uses the phone book sort order */
            static unsigned char *u4valid( unsigned char * parm )
            {
               int x = 0 ;

               while ( v4table[x].extChar != 0 )
               {
                  if ( v4table[x].extChar == *(unsigned char *)parm )
                     return (unsigned char *)v4table[x].expChars ;
                  x++ ;
               }
               return parm ;
            }

            int S4CALL u4memcmp( S4CMP_PARM s1, S4CMP_PARM s2, size_t len )
            {
               short s1Ext, s2Ext ;
               unsigned char *compare1, *compare2 ;
               unsigned char *string1Ptr, *string2Ptr ;

               string1Ptr = (unsigned char *)s1 ;
               string2Ptr = (unsigned char *)s2 ;

               while( len-- )
               {
                  if( *string1Ptr != *string2Ptr )
                  {
                     /* The characters are not equal.  Check for extended characters
                        as in German the extended characters are equivalent to
                        expanded characters  */

                     s1Ext = ( (short)*string1Ptr > 127 ) ;
                     s2Ext = ( (short)*string2Ptr > 127 ) ;

                     if( s1Ext ^ s2Ext )
                     {
                        /* Only one is an extended character. Check to see if valid and
                        expand to full length */

                        compare1 = (s1Ext) ? u4valid(string1Ptr) : string1Ptr ;
                        compare2 = (s2Ext) ? u4valid(string2Ptr) : string2Ptr ;

                        /* Expansion has been done to one string (maximum 2 chars in expansion) */
                        /* now compare the two characters */

                        if ( compare1[0] == compare2[0] )    /* do if first chars equal */
                        {
                           /* if there are not two valid second chars to check */
                           if ( compare1[1] == ' ' || compare2[1] == ' ' ||
                                compare1[1] == 0   || compare2[1] == 0 )
                           {
                              if (v4map[*string1Ptr] < v4map[*string2Ptr])  return -1 ;
                              return 1 ;
                           }

                           if ( compare1[1] == compare2[1] )
                           {
                              (s1Ext) ? string2Ptr++ : string1Ptr++ ;
                              if (len) len-- ;
                           }
                           else
                           {
                              if (v4map[*(compare1+1)] < v4map[*(compare2+1)])  return -1 ;
                              return 1 ;
                           }
                        }
                        else
                        {
                           if (v4map[*compare1] < v4map[*compare2])  return -1 ;
                           return 1 ;
                        }
                     }
                     else
                     {
                        /* Neither character is extended so return according to
                           v4map[].  */
                        if (v4map[*string1Ptr] < v4map[*string2Ptr])  return -1 ;
                        return 1 ;
                     }
                  }
                  /* Characters are equal. Increment the pointers and loop again. */

                  string1Ptr++ ;
                  string2Ptr++ ;
               }
               return 0 ;
            }
         #endif  /* S4DICTIONARY */
      #endif  /* S4GERMAN  */

      #ifdef S4FRENCH
         #ifndef S4ANSI
            /* This mapping is for French. */
            unsigned char v4map[256] =
            {
               /*      ,  ,  ,  ,  ,  ,  ,  ,  ,   ,   ,  ,  ,   ,  ,   */
               /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
                      0,140,141,142,143,144,145,146,147,  1,148,149,150,  2,151,152,

               /*     ,  ,  ,  ,  ,  ,  ,  ,  ,  ,EOF,  ,  ,  ,  ,   */
               /*    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 */
                    153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,

               /*      ,  !,  ",  #,  $,  %,  &,  ',  (,  ),  *,  +,  ,,  -,  .,  / */
               /*    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 */
                      3,117,132,123,124,109,122,134, 98, 99,110,107,112,108,111,118,

               /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  :,  ;,  <,  =,  >,  ? */
               /*    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 */
                      4,  5,  6,  7,  8,  9, 10, 11, 12, 13,114,113,104,106,105,116,

               /*     @,  A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O */
               /*    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79 */
                    121, 14, 17, 18, 20, 21, 23, 24, 25, 26, 27, 28, 29, 30, 31, 33,

               /*     P,  Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z,  [,  \,  ],  ^,  _ */
               /*    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95 */
                     35, 36, 37, 38, 39, 40, 42, 43, 44, 45, 46,100,119,101,135,137,

               /*     `,  a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o */
               /*    96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111 */
                    133, 47, 53, 54, 56, 57, 62, 63, 64, 65, 70, 71, 72, 73, 74, 76,

               /*     p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z,  {,  |,  },   ,   */
               /*   112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127 */
                     81, 82, 83, 84, 86, 87, 92, 93, 94, 95, 97,102,120,103,136,169,

               /*     Ä,  Å,  Ç,  É,  Ñ,  Ö,  Ü,  á,  à,  â,  ä,  ã,  å,  ç,  é,  è */
               /*   128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143 */
                     19, 88, 58, 49, 48, 50, 51, 55, 59, 60, 61, 66, 67, 68, 15, 16,

               /*     ê,  ë,  í,  ì,  î,  ï,  ñ,  ó,  ò,  ô,  ö,  õ,  ú,  ù,  û,  ü */
               /*   144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159 */
                     22,130,131, 78, 77, 79, 89, 90, 96, 34, 41,125,126,127,129,128,

               /*     †,  °,  ¢,  £,  §,  •,  ¶,  ß,  ®,  ©,  ™,  ´,  ¨,  ≠,  Æ,  Ø */
               /*   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175 */
                     52, 69, 80, 91, 75, 32,138,139,115,170,171,172,173,174,175,176,

               /*     ∞,  ±,  ≤,  ≥,  ¥,  µ,  ∂,  ∑,  ∏,  π,  ∫,  ª,  º,  Ω,  æ,  ø */
               /*   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191 */
                    177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,

               /*     ¿,  ¡,  ¬,  √,  ƒ,  ≈,  ∆,  «,  »,  …,   ,  À,  Ã,  Õ,  Œ,  œ */
               /*   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207 */
                    193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,

               /*     –,  —,  “,  ”,  ‘,  ’,  ÷,  ◊,  ÿ,  Ÿ,  ⁄,  €,  ‹,  ›,  ﬁ,  ﬂ */
               /*   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223 */
                    209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,

               /*     ‡,  ·,  ‚,  „,  ‰,  Â,  Ê,  Á,  Ë,  È,  Í,  Î,  Ï,  Ì,  Ó,  Ô */
               /*   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239 */
                    225, 85,226,227,228,229,230,231,232,233,234,235,236,237,238,239,

               /*     ,  Ò,  Ú,  Û,  Ù,  ı,  ˆ,  ˜,  ¯,  ˘,  ˙,  ˚,  ¸,  ˝,  ˛,    */
               /*   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 */
                    240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
            } ;

         #endif /* ifndef S4ANSI */

         int S4CALL u4memcmp( S4CMP_PARM s1, S4CMP_PARM s2, size_t len )
         {
            int i ;

            for ( i = 0 ; i < len ; i++ )
               if ( ((unsigned char *)s1)[i] != ((unsigned char *)s2)[i] )
               {
                  if ( v4map[((unsigned char *)s1)[i]] < v4map[((unsigned char *)s2)[i]] )
                     return -1 ;
                  return 1 ;
               }

            return 0 ;
         }
      #endif  /* S4FRENCH */

      #ifdef S4SWEDISH
         #ifndef S4ANSI
            /* This mapping is for Swedish. */
            unsigned char v4map[256] =
            {
               /*      ,  ,  ,  ,  ,  ,  ,  ,  ,   ,   ,  ,  ,   ,  ,   */
               /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
                      0,140,141,142,143,144,145,146,147,  1,148,149,150,  2,151,152,

               /*     ,  ,  ,  ,  ,  ,  ,  ,  ,  ,EOF,  ,  ,  ,  ,   */
               /*    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 */
                    153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,

               /*      ,  !,  ",  #,  $,  %,  &,  ',  (,  ),  *,  +,  ,,  -,  .,  / */
               /*    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 */
                      3,119,132,125,126,111,124,134,100,101,112,109,114,110,113,120,

               /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  :,  ;,  <,  =,  >,  ? */
               /*    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 */
                      4,  5,  6,  7,  8,  9, 10, 11, 12, 13,116,115,106,108,107,118,

               /*     @,  A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O */
               /*    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79 */
                    123, 14, 15, 16, 18, 19, 21, 22, 23, 24, 25, 26, 27, 28, 29, 31,

               /*     P,  Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z,  [,  \,  ],  ^,  _ */
               /*    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95 */
                     32, 33, 34, 35, 36, 37, 39, 40, 41, 42, 43,102,121,103,135,137,

               /*     `,  a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o */
               /*    96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111 */
                    133, 48, 52, 53, 55, 56, 61, 62, 63, 64, 69, 70, 71, 72, 73, 75,

               /*     p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z,  {,  |,  },   ,   */
               /*   112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127 */
                     79, 80, 81, 82, 84, 85, 90, 91, 92, 93, 95,104,122,105,136,169,

               /*     Ä,  Å,  Ç,  É,  Ñ,  Ö,  Ü,  á,  à,  â,  ä,  ã,  å,  ç,  é,  è */
               /*   128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143 */
                     17, 86, 57, 49, 97, 50, 96, 54, 58, 59, 60, 65, 66, 67, 45, 44,

               /*     ê,  ë,  í,  ì,  î,  ï,  ñ,  ó,  ò,  ô,  ö,  õ,  ú,  ù,  û,  ü */
               /*   144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159 */
                     20, 98, 46, 76, 99, 77, 87, 88, 94, 47, 38,127,128,129,131,130,

               /*     †,  °,  ¢,  £,  §,  •,  ¶,  ß,  ®,  ©,  ™,  ´,  ¨,  ≠,  Æ,  Ø */
               /*   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175 */
                     51, 68, 78, 89, 74, 30,138,139,117,170,171,172,173,174,175,176,

               /*     ∞,  ±,  ≤,  ≥,  ¥,  µ,  ∂,  ∑,  ∏,  π,  ∫,  ª,  º,  Ω,  æ,  ø */
               /*   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191 */
                    177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,

               /*     ¿,  ¡,  ¬,  √,  ƒ,  ≈,  ∆,  «,  »,  …,   ,  À,  Ã,  Õ,  Œ,  œ */
               /*   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207 */
                    193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,

               /*     –,  —,  “,  ”,  ‘,  ’,  ÷,  ◊,  ÿ,  Ÿ,  ⁄,  €,  ‹,  ›,  ﬁ,  ﬂ */
               /*   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223 */
                    209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,

               /*     ‡,  ·,  ‚,  „,  ‰,  Â,  Ê,  Á,  Ë,  È,  Í,  Î,  Ï,  Ì,  Ó,  Ô */
               /*   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239 */
                    225, 83,226,227,228,229,230,231,232,233,234,235,236,237,238,239,

               /*     ,  Ò,  Ú,  Û,  Ù,  ı,  ˆ,  ˜,  ¯,  ˘,  ˙,  ˚,  ¸,  ˝,  ˛,    */
               /*   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 */
                    240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
            } ;
         #endif /* S4ANSI */

         int S4CALL u4memcmp( S4CMP_PARM s1, S4CMP_PARM s2, size_t len )
         {
            int i ;

            for ( i = 0 ; i < len ; i++ )
               if ( ((unsigned char *)s1)[i] != ((unsigned char *)s2)[i] )
               {
                  if ( v4map[((unsigned char *)s1)[i]] < v4map[((unsigned char *)s2)[i]] )
                     return -1 ;
                  return 1 ;
               }

            return 0 ;
         }
      #endif  /* S4SWEDISH */

      #ifdef S4FINNISH
         #ifndef S4ANSI
            /* This mapping is for Finnish. */
            unsigned char v4map[256] =
            {
               /*      ,  ,  ,  ,  ,  ,  ,  ,  ,   ,   ,  ,  ,   ,  ,   */
               /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
                      0,140,141,142,143,144,145,146,147,  1,148,149,150,  2,151,152,

               /*     ,  ,  ,  ,  ,  ,  ,  ,  ,  ,EOF,  ,  ,  ,  ,   */
               /*    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 */
                    153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,

               /*      ,  !,  ",  #,  $,  %,  &,  ',  (,  ),  *,  +,  ,,  -,  .,  / */
               /*    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 */
                      3,119,132,125,126,111,124,134,100,101,112,109,114,110,113,120,

               /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  :,  ;,  <,  =,  >,  ? */
               /*    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 */
                      4,  5,  6,  7,  8,  9, 10, 11, 12, 13,116,115,106,108,107,118,

               /*     @,  A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O */
               /*    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79 */
                    123, 14, 15, 16, 18, 19, 21, 22, 23, 24, 25, 26, 27, 28, 29, 31,

               /*     P,  Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z,  [,  \,  ],  ^,  _ */
               /*    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95 */
                     32, 33, 34, 35, 36, 37, 39, 40, 41, 42, 43,102,121,103,135,137,

               /*     `,  a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o */
               /*    96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111 */
                    133, 48, 52, 53, 55, 56, 61, 62, 63, 64, 69, 70, 71, 72, 73, 75,

               /*     p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z,  {,  |,  },   ,   */
               /*   112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127 */
                     79, 80, 81, 82, 84, 85, 90, 91, 92, 93, 95,104,122,105,136,169,

               /*     Ä,  Å,  Ç,  É,  Ñ,  Ö,  Ü,  á,  à,  â,  ä,  ã,  å,  ç,  é,  è */
               /*   128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143 */
                     17, 86, 57, 49, 97, 50, 96, 54, 58, 59, 60, 65, 66, 67, 45, 44,

               /*     ê,  ë,  í,  ì,  î,  ï,  ñ,  ó,  ò,  ô,  ö,  õ,  ú,  ù,  û,  ü */
               /*   144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159 */
                     20, 98, 46, 76, 99, 77, 87, 88, 94, 47, 38,127,128,129,131,130,

               /*     †,  °,  ¢,  £,  §,  •,  ¶,  ß,  ®,  ©,  ™,  ´,  ¨,  ≠,  Æ,  Ø */
               /*   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175 */
                     51, 68, 78, 89, 74, 30,138,139,117,170,171,172,173,174,175,176,

               /*     ∞,  ±,  ≤,  ≥,  ¥,  µ,  ∂,  ∑,  ∏,  π,  ∫,  ª,  º,  Ω,  æ,  ø */
               /*   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191 */
                    177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,

               /*     ¿,  ¡,  ¬,  √,  ƒ,  ≈,  ∆,  «,  »,  …,   ,  À,  Ã,  Õ,  Œ,  œ */
               /*   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207 */
                    193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,

               /*     –,  —,  “,  ”,  ‘,  ’,  ÷,  ◊,  ÿ,  Ÿ,  ⁄,  €,  ‹,  ›,  ﬁ,  ﬂ */
               /*   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223 */
                    209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,

               /*     ‡,  ·,  ‚,  „,  ‰,  Â,  Ê,  Á,  Ë,  È,  Í,  Î,  Ï,  Ì,  Ó,  Ô */
               /*   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239 */
                    225, 83,226,227,228,229,230,231,232,233,234,235,236,237,238,239,

               /*     ,  Ò,  Ú,  Û,  Ù,  ı,  ˆ,  ˜,  ¯,  ˘,  ˙,  ˚,  ¸,  ˝,  ˛,    */
               /*   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 */
                    240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
            } ;

         #endif /* S4ANSI */

         int S4CALL u4memcmp( S4CMP_PARM s1, S4CMP_PARM s2, size_t len )
         {
            int i ;

            for ( i = 0 ; i < len ; i++ )
               if ( ((unsigned char *)s1)[i] != ((unsigned char *)s2)[i] )
               {
                  if ( v4map[((unsigned char *)s1)[i]] < v4map[((unsigned char *)s2)[i]] )
                     return -1 ;
                  return 1 ;
               }

            return 0 ;
         }
      #endif  /* S4FINNISH */

      #ifdef S4NORWEGIAN
         #ifndef S4ANSI
            /* This mapping is for Norwegian. */
            unsigned char v4map[256] =
            {
               /*      ,  ,  ,  ,  ,  ,  ,  ,  ,   ,   ,  ,  ,   ,  ,   */
               /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
                      0,140,141,142,143,144,145,146,147,  1,148,149,150,  2,151,152,

               /*     ,  ,  ,  ,  ,  ,  ,  ,  ,  ,EOF,  ,  ,  ,  ,   */
               /*    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 */
                    153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,

               /*      ,  !,  ",  #,  $,  %,  &,  ',  (,  ),  *,  +,  ,,  -,  .,  / */
               /*    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 */
                      3,119,132,125,126,111,124,134,100,101,112,109,114,110,113,120,

               /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  :,  ;,  <,  =,  >,  ? */
               /*    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 */
                      4,  5,  6,  7,  8,  9, 10, 11, 12, 13,116,115,106,108,107,118,

               /*     @,  A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O */
               /*    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79 */
                    123, 14, 15, 16, 18, 19, 21, 22, 23, 24, 25, 26, 27, 28, 29, 31,

               /*     P,  Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z,  [,  \,  ],  ^,  _ */
               /*    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95 */
                     32, 33, 34, 35, 36, 37, 39, 40, 41, 42, 43,102,121,103,135,137,

               /*     `,  a,  b,  c,  d,  e,  f,  g,  h,  i,  j,  k,  l,  m,  n,  o */
               /*    96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111 */
                    133, 48, 52, 53, 55, 56, 61, 62, 63, 64, 69, 70, 71, 72, 73, 75,

               /*     p,  q,  r,  s,  t,  u,  v,  w,  x,  y,  z,  {,  |,  },   ,   */
               /*   112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127 */
                     79, 80, 81, 82, 84, 85, 90, 91, 92, 93, 95,104,122,105,136,169,

               /*     Ä,  Å,  Ç,  É,  Ñ,  Ö,  Ü,  á,  à,  â,  ä,  ã,  å,  ç,  é,  è */
               /*   128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143 */
                     17, 86, 57, 49, 97, 50, 96, 54, 58, 59, 60, 65, 66, 67, 45, 44,

               /*     ê,  ë,  í,  ì,  î,  ï,  ñ,  ó,  ò,  ô,  ö,  õ,  ú,  ù,  û,  ü */
               /*   144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159 */
                     20, 98, 46, 76, 99, 77, 87, 88, 94, 47, 38,127,128,129,131,130,

               /*     †,  °,  ¢,  £,  §,  •,  ¶,  ß,  ®,  ©,  ™,  ´,  ¨,  ≠,  Æ,  Ø */
               /*   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175 */
                     51, 68, 78, 89, 74, 30,138,139,117,170,171,172,173,174,175,176,

               /*     ∞,  ±,  ≤,  ≥,  ¥,  µ,  ∂,  ∑,  ∏,  π,  ∫,  ª,  º,  Ω,  æ,  ø */
               /*   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191 */
                    177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,

               /*     ¿,  ¡,  ¬,  √,  ƒ,  ≈,  ∆,  «,  »,  …,   ,  À,  Ã,  Õ,  Œ,  œ */
               /*   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207 */
                    193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,

               /*     –,  —,  “,  ”,  ‘,  ’,  ÷,  ◊,  ÿ,  Ÿ,  ⁄,  €,  ‹,  ›,  ﬁ,  ﬂ */
               /*   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223 */
                    209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,

               /*     ‡,  ·,  ‚,  „,  ‰,  Â,  Ê,  Á,  Ë,  È,  Í,  Î,  Ï,  Ì,  Ó,  Ô */
               /*   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239 */
                    225, 83,226,227,228,229,230,231,232,233,234,235,236,237,238,239,

               /*     ,  Ò,  Ú,  Û,  Ù,  ı,  ˆ,  ˜,  ¯,  ˘,  ˙,  ˚,  ¸,  ˝,  ˛,    */
               /*   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 */
                    240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
            } ;
         #endif /* S4ANSI */

         int S4CALL u4memcmp( S4CMP_PARM s1, S4CMP_PARM s2, size_t len )
         {
            int i ;

            for ( i = 0 ; i < len ; i++ )
               if ( ((unsigned char *)s1)[i] != ((unsigned char *)s2)[i] )
               {
                  if ( v4map[((unsigned char *)s1)[i]] < v4map[((unsigned char *)s2)[i]] )
                     return -1 ;
                  return 1 ;
               }

            return 0 ;
         }

      #endif /* S4NORWEGIAN */
   #endif /* S4LANGUAGE  */
#else /* S4FOX */
   #ifdef S4VFP_KEY
      int t4strToVFPKey( char *dest, const char *src, const int src_l, const int max_l, T4VFP *vfp )
      {
         int i, j, head, tail, len ;
         unsigned char index ;
         translatedChars *v4translatedChar = vfp->tablePtr ;
         compressedChars *v4compressedChar = vfp->compPtr ;
         unsigned char *v4codePage = vfp->cpPtr ;

         #ifdef E4PARM_LOW
            if ( max_l < 1 || src_l > max_l )
               return error4( 0, e4parm, E91643 ) ;
         #endif

         if ( src_l*2 < max_l )   /* partial key: don't need to worry about tail characters */
         {
            tail = len = src_l ;

            head = 0 ;
            for ( i = 0 ; (i < len && head < max_l) ; i++ )
            {
               index = ( (unsigned char)src[i] < 128 ) ? (unsigned char)src[i] : v4codePage[(unsigned char)src[i]-128] ;
               if ( v4translatedChar[0][index] != 255 )
                  dest[head++] = v4translatedChar[0][index];
               else
               {
                  /* translate both characters */
                  for ( j = 0 ; (j < 2 && head < max_l) ; j++ )
                     dest[head++] = v4translatedChar[0][v4compressedChar[v4translatedChar[1][index]][j]] ;
               }
            }

            return head ;
         }
         else
         {
            len = src_l ;

            for ( i = len-1 ; (i >= 0 && src[i] == ' ') ; i-- )   /* remove trailing spaces */
               len-- ;

            tail = len ;
            for ( i = 0 ; (i < len && tail < max_l) ; i++ )   /* determine where to start tail characters */
            {
               index = ( (unsigned char)src[i] < 128 ) ? (unsigned char)src[i] : v4codePage[(unsigned char)src[i]-128] ;
               if ( v4translatedChar[0][index] == 255 )
                  tail++ ;
            }

            head = 0 ;
            for ( i = 0 ; (i < len && head < max_l) ; i++ )
            {
               index = ( (unsigned char)src[i] < 128 ) ? (unsigned char)src[i] : v4codePage[(unsigned char)src[i]-128] ;
               if ( v4translatedChar[0][index] != 255 )
               {
                  dest[head++] = v4translatedChar[0][index];
                  if ( ( v4translatedChar[1][index] != 255 ) && ( tail < max_l ) )
                     dest[tail++] = v4translatedChar[1][index];
               }
               else
               {
                  /* translate both characters */
                  for ( j = 0 ; (j < 2 && head < max_l) ; j++ )
                  {
                     dest[head++] = v4translatedChar[0][v4compressedChar[v4translatedChar[1][index]][j]] ;
                     if ( ( v4translatedChar[1][v4compressedChar[v4translatedChar[1][index]][j]] != 255 ) && ( tail < max_l ) )
                        dest[tail++] = v4translatedChar[1][v4compressedChar[v4translatedChar[1][index]][j]] ;
                  }
               }
            }

            memset( (void *)&dest[tail], 0, max_l-tail ) ;   /* pad out the remaining (should be same as tag's pChar) */
            return max_l ;
         }
      }
   #endif /* S4VFP_KEY */

   int S4CALL u4keycmp( S4CMP_PARM dataPtr, S4CMP_PARM searchPtr, size_t sLen, size_t dLen, size_t trailCnt, T4VFP *vfp )
   {
      unsigned char *data = (unsigned char *)dataPtr ;
      unsigned char *search = (unsigned char *)searchPtr ;
      unsigned s, d ;
      int skippedFlag = 0 ;

      if ( vfp->sortType == sort4machine )   /* machine doesn't need to handle translated keys */
         return c4memcmp( dataPtr, searchPtr, sLen ) ;

      /* no special double characters at this point so don't worry about them */
      d = 0 ;
      for ( s = 0 ; s < sLen && search[s] >= 10 && d < dLen ; s++ )   /* compare the heads */
      {
         if ( data[d] != search[s] )
         {
            if ( search[s] == 17 )
            {
               if ( data[d] >= 10 ) return 1 ;
                  continue ;
            }
            if ( data[d] > search[s] ) return 1 ;
               return -1 ;
         }
         d++ ;
      }

      if ( d < dLen && data[d] >= 10 )   /* remember we skipped some characters */
           skippedFlag = 1 ;
      while ( d < dLen && data[d] >= 10 )   /* advance data to it's tail */
         d++ ;

      for ( ; s < sLen && search[s] < 10 ; s++ )   /* compare the tails */
      {
         if ( d < dLen )
         {
            if ( data[d] != search[s] )
            {
               if ( skippedFlag || data[d] > search[s] ) return 1 ;
               return -1 ;
            }
         }
         else
         {
            if ( trailCnt <= 0 || search[s] != 0 )
            {
               if ( !skippedFlag ) return -1 ;
               return 1 ;
            }
            trailCnt-- ;
         }
         d++ ;
      }

      return 0 ;
   }

   /* Visual FoxPro uses collating sequences and codepages to support International Languages */

   #ifdef S4GENERAL
      /* v4general - supports the general collating sequence */

      translatedChars v4general[2] =
      {
         {
            /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
               32, 16, 16, 16, 16, 16, 16, 16, 16, 17, 16, 16, 16, 16, 16, 16,

            /*    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 */
                  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,

            /*    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 */
                  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,

            /*    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 */
                  86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 33, 34, 35, 36, 37, 38,

            /*    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79 */
                  39, 96, 97, 98,100,102,103,104,105,106,107,108,109,111,112,114,

            /*    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95 */
                 115,116,117,118,119,120,122,123,124,125,126, 40, 41, 42, 43, 44,

            /*    96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111 */
                  45, 96, 97, 98,100,102,103,104,105,106,107,108,109,111,112,114,

            /*   112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127 */
                 115,116,117,118,119,120,122,123,124,125,126, 46, 47, 48, 49, 16,

            /*   128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143 */
                       16, 16, 24, 50, 19, 51, 52, 53, 54, 55,118, 19,255, 16, 16, 16,

            /*   144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159 */
                  16, 24, 24, 19, 19, 56, 30, 30, 57, 58,118, 24,255, 16, 16,125,

            /*   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175 */
                  32, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 19, 69, 30, 70, 71,

            /*   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191 */
                  72, 73, 88, 89, 74, 75, 76, 77, 78, 87, 79, 19, 80, 81, 82, 83,

            /*   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207 */
                  96, 96, 96, 96, 96, 96,255, 98,102,102,102,102,106,106,106,106,

            /*   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223 */
                 101,112,114,114,114,114,114, 84,129,120,120,120,120,125,255,255,

            /*   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239 */
                  96, 96, 96, 96, 96, 96,255, 98,102,102,102,102,106,106,106,106,

            /*   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 */
                 101,112,114,114,114,114,114,125,129,120,120,120,120,125,255,118,
          },
          {
            /*     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 */
                 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,

            /*    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 */
                 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,

            /*    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 */
                      255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,

            /*    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 */
                      255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,

            /*    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79 */
                      255,  0,255,  0,255,  0,255,255,255,  0,255,255,255,255,  0,  0,

            /*    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95 */
                      255,255,255,  0,255,  0,255,255,255,  0,255,255,255,255,255,255,

            /*    96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111 */
                      255,  0,255,  0,255,  0,255,255,255,  0,255,255,255,255,  0,  0,

            /*   112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127 */
                      255,255,255,  0,255,  0,255,255,255,  0,255,255,255,255,255,255,

            /*   128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143 */
                      255,255,255,255,255,255,255,255,255,255,  8,255, 0,255,255,255,

            /*   144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159 */
                      255,255,255,255,255,255,255,255,255,255,  8,255,  0,255,255,  4,

            /*   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175 */
                        1,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,

            /*   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191 */
                      255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,

            /*   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207 */
                        2,  1,  3,  5,  4,  6,  1,  7,  2,  1,  3,  4,  2,  1,  3,  4,

            /*   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223 */
                      255,  5,  2,  1,  3,  5,  4,255,255,  2,  1,  3,  4,  1,  2,  3,

            /*   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239 */
                        2,  1,  3,  5,  4,  6,  1,  7,  2,  1,  3,  4,  2,  1,  3,  4,

            /*   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 */
                      255,  5,  2,  1,  3,  5,  4,255,255,  2,  1,  3,  4,  1,  2,  4,
          }
       } ;

      compressedChars v4generalComp[4] =
      {
         { 79, 69 },   /* "oe" */
         { 65, 69 },   /* "ae" */
         { 84, 72 },   /* "th" */
         { 83, 83 },   /* "ss" */
      } ;
   #endif /* S4GENERAL */

   /* 1252 - Supports The WINDOWS ANSI CodePage */
   /* Note: all collating sequence translation tables use CodePage 1252 directly;
            therefore, this CodePage table has a one-to-one mapping */
   /* Note: this is the default CodePage if no CodePage is selected (i.e., cp0 ) */

   unsigned char CodePage_1252[128] =
   {
      /*   128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143 */
           128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,

      /*   144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159 */
           144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,

      /*   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175 */
           160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,

      /*   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191 */
           176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,

      /*   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207 */
           192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,

      /*   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223 */
           208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,

      /*   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239 */
           224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,

      /*   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 */
           240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
   } ;

   #ifdef S4CODEPAGE_437
      /* 437 - Supports The U.S. MS-DOS CodePage */

      unsigned char CodePage_437[] =
      {
         /*   128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143 */
              199,252,233,226,228,224,229,231,234,235,232,239,238,236,196,197,

         /*   144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159 */
              201,230,198,244,246,242,251,249,255,214,220,  1,  1,  1,  1,  1,

         /*   160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175 */
              225,237,243,250,241,209,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,

         /*   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191 */
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,

         /*   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207 */
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,

         /*   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223 */
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,

         /*   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239 */
                1,223,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,

         /*   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255 */
                1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
      } ;
   #endif  /* S4CODEPAGE_437 */
#endif  /* S4FOX */
