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

/* c4code.c  */

#include "d4all.h"


#ifdef S4WINTEL
   #ifndef S4WINCE
      #include <process.h>
   #endif
#endif  /* S4WINTEL */

#if S4VERSION != 6401
   #error Your CodeBase source version does not match your header file version.
#endif

#ifdef S4TESTING
   #include <fcntl.h>
   #include <sys\types.h>
   #include <sys\stat.h>
#endif

#ifdef S4WINTEL
   #ifdef __TURBOC__
      #ifndef __DLL__
         #ifndef S4OS2PM
            #ifndef __WIN32
               #ifndef S4WINDOWS
                  extern unsigned _stklen ;
               #endif
            #endif
         #endif
      #endif
   #endif

   #ifdef _MSC_VER
       #ifndef __DLL__
          #include <malloc.h>
       #endif
   #endif
#endif

#ifdef S4CBPP
   #ifdef __cplusplus
      #include "d4data.hpp"
   #endif
#endif

#ifdef S4TESTING
   FILE4 s4test ;
#else
   #ifdef S4TRACK_FILES
      FILE4 s4test ;
   #endif
#endif

#ifdef S4FILE_EXTENDED
   #ifdef S4FOX
      C4STAMP largeStamp = {"BBROYGBVGWE", 0, 0};
   #endif
#endif

char f4memoNullChar = '\0' ;
char *expr4buf = 0 ;

static unsigned int numCode4 = 0 ;  /* used to determine when mem4reset() should be called */
extern int resetInProgress ;

int code4numCodeBase( void )
{
   return numCode4 ;
}

#ifdef S4SEMAPHORE
   #ifdef S4OS2
      #include <time.h>

      static char sem4mem[20], sem4expr[20] ;
      static HMTX hmtx4mem, hmtx4expr ;
   #endif
#endif

#ifdef S4IBMOS2
   #ifdef S4OS2DLL
      #ifndef __MULTI__     /* no multi-thread code is being produced */
         int errno = 0 ;
      #endif
   #endif
#endif

#ifdef __DLL__

#ifdef S4PASCAL
   #ifndef __WIN32
      typedef char far* LPSTR ;
      typedef unsigned int HANDLE ;
      typedef unsigned short WORD ;
      #define PASCAL _pascal
      #define S4USE_WEP
   #endif
#endif

#ifndef S4PASCAL_DOS
   #ifdef S4DLL
      HINSTANCE cb5inst = (HINSTANCE)NULL ;
   #endif
#endif

#ifdef S4OS2
ULONG _dllmain (ULONG termflag, HMODULE modhandle)
{
   #ifdef S4SEMAPHORE
      int i ;
      APIRET rc ;
      time_t t ;
   #endif

   if ( termflag == 0 )
   {
      #ifdef S4SEMAPHORE
         strcpy( sem4expr, "\\SEM32\\S4A" ) ;
         strcpy( sem4mem, "\\SEM32\\S4B" ) ;
         for ( i = 0 ; i < 100 ; i++ )
         {
            u4delaySec() ;
            time( &t ) ;
            t %= 10000L ;

            c4ltoa45( t, sem4expr + 10, -4 ) ;
            c4ltoa45( t, sem4mem + 10, -4 ) ;

            rc = DosCreateMutexSem( sem4expr, &hmtx4expr, 0, 0 ) ;
            if ( rc != 0 )
               continue ;

            rc = DosCreateMutexSem( sem4mem, &hmtx4mem, 0, 0 ) ;
            if ( rc != 0 )
            {
               DosCloseMutexSem( hmtx4expr ) ;
               continue ;
            }
            return 1 ;
         }
      #else
         return 1 ;
      #endif
   }
   else
   {
      #ifdef S4SEMAPHORE
         DosCloseMutexSem( hmtx4mem ) ;
         DosCloseMutexSem( hmtx4expr ) ;
      #endif
   }

   return 1 ;
}
#endif

#ifndef __WIN32
#ifndef S4PASCAL_DOS
#ifdef S4WINDOWS
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int far PASCAL LibMain( HINSTANCE hInstance, WORD wDataSeg, WORD cbHeapSize, LPSTR lpCmdLine )
{
   if ( !cb5inst )
      cb5inst = hInstance ;

   return 1 ;
}
#endif   /* S4WINDOWS */
#endif   /* S4PASCAL_DOS */

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION WEP( int nParameter )
{
   return 1 ;
}

#else

BOOL APIENTRY DllMain( HANDLE hModule, DWORD reasonForCall, LPVOID reserved )
{
   if ( !cb5inst )
      cb5inst = hModule ;

   return 1 ;
}

#endif   /* __WIN32 */

#ifndef S4PASCAL_DOS
HINSTANCE S4FUNCTION c4dllInst( void )
{
   return cb5inst ;
}
#endif   /* S4PASCAL_DOS */

#endif   /* __DLL__ */

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION code4initLow( CODE4 *c4, const char *defaultProtocol, long versionId, long structSize )
{
   #ifdef S4TESTING
      int exclusiveFlag ;
      int openFlag ;
      char *envLog ;
      #ifdef S4CASE_SEN
         char *logFile = "T4TEST.log" ;
      #else
         char *logFile = "T4TEST.LOG" ;
      #endif
   #else
      #ifdef S4TRACK_FILES
         int openFlag ;
         char *envLog ;
         #ifdef S4CASE_SEN
            char *logFile = "T4TEST.log" ;
         #else
            char *logFile = "T4TEST.LOG" ;
         #endif
      #endif
   #endif
      int rc ;
   #ifndef S4OFF_OPTIMIZE
      #ifdef S4OPTIMIZE_STATS
         DATA4 *stat ;
         int oldOffErr ;
      #endif
   #endif
   #ifndef S4STAND_ALONE
      #ifdef S4WINTEL
         WSADATA WSAData ;
      #endif
   #endif

   #ifdef E4PARM_HIGH
      if ( c4 == 0 )
         return error4( 0, e4parm, E91001 ) ;
   #endif

   /***********************************************************************

      This code must reside at the beginning of the function to ensure that
      no file open/create incorrectly auto-starts the optimization process

    ***********************************************************************/

   #ifndef S4OFF_OPTIMIZE
      c4->hadOpt = 1 ;
   #endif

   #ifdef S4TESTING
      if ( numCode4 == 0 )
      {
         memset( &s4test, 0, sizeof( s4test ) ) ;
         s4test.hand = INVALID4HANDLE ;
      }
      #ifdef S4STAND_ALONE
         c4->doTransLocking = 1 ;   /* for testing, to allow one program to have multiple transactions in progress */
      #endif
   #else
      #ifdef S4TRACK_FILES
         if ( numCode4 == 0 )
         {
            memset( &s4test, 0, sizeof( s4test ) ) ;
            s4test.hand = INVALID4HANDLE ;
         }
      #endif
   #endif

   if ( versionId != S4VERSION )
   {
      char errHeader[20], errLib[20] ;
      #ifdef S4WINCE
         strcpy( errLib, "Library: ") ;
         _itoa( S4VERSION, errLib + 9, 10 ) ;
         strcpy( errHeader, "Headers: ") ;
         _itoa( versionId, errHeader + 9, 10 ) ;
      #else
         sprintf( errLib,    "Library: %ld", S4VERSION ) ;
         sprintf( errHeader, "Headers: %ld", versionId ) ;
      #endif
      return error4describe( 0, e4version, E91001, errLib, errHeader, 0 ) ;
   }

   if ( structSize )   /* bypass check if zero passed */
      if ( structSize != sizeof( CODE4 ) )  /* error, switch mismatch most likely */
      {
         char errHeader[30], errLib[30] ;
         sprintf( errLib,    "Library struct size: %d", sizeof( CODE4 ) ) ;
         sprintf( errHeader, "Header struct size: %ld", structSize ) ;
         return error4describe( 0, e4verify, E81507, errLib, errHeader, 0 ) ;
      }

   memset( (void *)c4, 0, sizeof( CODE4 ) ) ;

   #ifdef S4MACINTOSH
      c4->macDir = *(long *)0x398 ;
      c4->macVol = -(*(short *)0x214) ;
   #endif

   /* if a DLL, can't check the stack length since this is a separate executable */
   #ifndef __WIN32
      #ifndef S4WINDOWS
         #ifndef __DLL__
            #ifdef __TURBOC__
               #ifndef S4OS2PM
                  if ( _stklen < 5000U ) /* 5000 seems to be an appropriate minimum */
                        return error4( 0, e4result, E81003 ) ;
               #endif
            #endif
         #endif
         #ifdef _MSC_VER
            if ( stackavail() < 5000U )
               error4( 0, e4result, E81003 ) ;
         #endif
      #endif
   #endif

   if ( numCode4 == 0 && resetInProgress == 0 )
      mem4init() ;
   numCode4++ ;

   #ifdef E4ANALYZE
      c4->debugInt = E4DEBUG_INT ; /* Some random value for double checking. */
   #else
      #ifdef S4VBASIC
         c4->debugInt = E4DEBUG_INT ; /* Some random value for double checking. */
      #endif  /* S4VBASIC */
   #endif  /* E4MISC */

   #ifndef S4OFF_MEMO
      c4->memSizeMemoExpr = 0x400 ;        /*  1024 */
   #endif

   c4->numericStrLen = 17 ;    /* default length for clipper numeric keys is 10  */
   c4->decimals = 2 ;

   /* Flags initialization */

      c4setErrCreate( c4, 1 ) ;
   c4->errDefaultUnique = r4uniqueContinue ;
   c4->errExpr = 1 ;
   c4->errFieldName = 1 ;
   c4->errOpen = 1 ;
   #ifndef S4OFF_INDEX
      c4->errTagName = 1 ;
   #endif
   c4->autoOpen = 1 ;
   c4->safety = 1 ;

   #ifndef S4OFF_MULTI
      c4->lockDelay = 100 ;
   #endif

   c4->collatingSequence = sort4machine ;   /* default to FoxPro 2.x values */
   c4->codePage = cp0 ;

   c4->memStartDataFile = 5 ;
   c4->memExpandDataFile = 5 ;
   #ifndef S4OFF_INDEX
      c4->memStartTagFile = 10 ;
      c4->memExpandTagFile =5 ;
   #endif

   #ifndef S4SINGLE
      c4->memStartLock = 5 ;
      c4->memExpandLock = 10 ;
   #endif

   #ifndef N4OTHER
      c4->memStartIndexFile = 5 ;
      c4->memExpandIndexFile = 5 ;
   #endif

   #ifndef S4OFF_MULTI
      c4->lockAttemptsSingle = 1 ;
   #endif

      c4->errGo = 1 ;
      c4->errSkip = 1 ;
      c4->errRelate = 1 ;
      #ifndef S4OFF_MULTI
         c4->lockAttempts = WAIT4EVER ;   /* wait forever */
      #endif
      c4->singleOpen = 1 ;
      #ifndef S4OFF_ENFORCE_LOCK
         c4->lockEnforce = 0 ;
      #endif
      #ifdef S4OFF_OPTIMIZE
         #ifndef S4OFF_INDEX
            c4->memSizeBlock = 0x400 ;        /*  1024 */
         #endif
      #else
         c4->memSizeBlock = 0x400 ;        /*  1024 */
      #endif
      c4->memSizeSortPool = 0xF000 ;    /* 61440 */
      c4->memSizeSortBuffer = 0x1000 ;  /*  4096 */
      c4->memSizeBuffer = 0x8000 ;      /* 32768 */
      #ifndef S4OFF_MEMO
         c4->memSizeMemo = 0x200 ;         /*   512 */
      #endif
      c4->memStartData = 10 ;
      c4->memExpandData = 5 ;
      #ifndef S4OFF_INDEX
         c4->memExpandBlock = 10 ;
         c4->memStartBlock = 10 ;
         c4->memStartIndex = 10 ;
         c4->memStartTag = 10 ;
         c4->memExpandIndex = 5 ;
         c4->memExpandTag = 10 ;
      #endif

      c4->doIndexVerify = 1 ;

   #ifndef S4OFF_TRAN
      #ifndef S4OFF_WRITE
            c4->log = LOG4ON ;
         #ifdef S4STAND_ALONE
            c4->logOpen = 1 ;
         #endif
      #endif
   #endif

   #ifndef S4OFF_COMMUNICATIONS
      #ifdef S4TIMEOUT_HOOK
         c4->timeout = 100 ;
      #else
         c4->timeout = -1 ;
      #endif
   #endif

   #ifndef S4OPTIMIZE_OFF
      c4->doOpt = 1 ;   /* by default do optimization */
         c4->optimize = OPT4EXCLUSIVE ;   /* by default optimize non-shared files */
         c4->optimizeWrite = OPT4EXCLUSIVE ;
   #endif  /* not S4OPTIMIZE_OFF */

      rc = code4tranInit( c4 ) ;
      if ( rc < 0 )
      {
         numCode4--;
         return error4( 0, rc, E91001 ) ;
      }

      #ifndef S4OFF_COMMUNICATIONS
         c4->clientDataCount = 1 ;
      #endif

      #ifndef S4LANGUAGE
         code4dateFormatSet( c4, "MM/DD/YY" ) ;
      #else
         #ifdef S4GERMAN
            code4dateFormatSet( c4, "DD.MM.YY" ) ;
         #endif
         #ifdef S4FRENCH
            code4dateFormatSet( c4, "MM/DD/YY" ) ;
         #endif
         #ifdef S4SWEDISH
            code4dateFormatSet( c4, "YYYY-MM-DD" ) ;
         #endif
         #ifdef S4FINNISH
            code4dateFormatSet( c4, "YYYY-MM-DD" ) ;
         #endif
         #ifdef S4NORWEGIAN
            code4dateFormatSet( c4, "DD-MM-YYYY" ) ;
         #endif
      #endif

   #ifdef S4FILE_EXTENDED
      #ifdef S4FOX
         if (largeStamp.doLargeLocking)
            code4largeOn(c4);
         else
            largeStamp.previousCode4init++;
      #endif
   #endif

   c4->initialized = 1 ;

   if ( numCode4 == 1 )
   {
      #ifdef S4TESTING
         openFlag = c4->errOpen ;
         exclusiveFlag = c4->accessMode ;
         c4->errOpen = 0 ;
         c4->accessMode = OPEN4DENY_NONE ;
         envLog = getenv( "T4LOG" ) ;
         if ( envLog )           /* if env. var. exists, use it */
            logFile = envLog ;
         if ( file4open( &s4test, c4, logFile, 0 ) != r4success )
            file4create( &s4test, c4, logFile, 0 ) ;
         c4->accessMode = exclusiveFlag ;
         c4->errOpen = openFlag ;
      #else
         #ifdef S4TRACK_FILES
            openFlag = c4->errOpen ;
            c4->errOpen = 0 ;
            envLog = getenv( "CB51_LOG" ) ;
            if ( envLog )           /* if env. var. exists, use it */
               logFile = envLog ;
            if ( file4open( &s4test, c4, logFile, 0 ) != r4success )
               file4create( &s4test, c4, logFile, 0 ) ;
            c4->errOpen = openFlag ;
         #endif
      #endif
   }

      #ifndef S4SINGLE
         #ifdef S4CB51
            code4unlockAutoSet( c4, LOCK4DATA ) ;
         #else
            code4unlockAutoSet( c4, LOCK4ALL ) ;
         #endif
      #endif

   #ifndef S4OFF_OPTIMIZE
      #ifdef S4OPTIMIZE_STATS
         stat = c4->statusDbf ;
         if ( stat == 0 )  /* open the stat database */
         {
            oldOffErr = c4->errOpen ;
            c4->errOpen = 0 ;
            c4->statusDbf = d4open( c4, "OPT4STAT" ) ;
            c4->errOpen = oldOffErr ;
            stat = c4->statusDbf ;
            if ( stat == 0 )
               error4set( c4, 0 ) ;
            else
            {
               d4optimize( stat, OPT4OFF ) ;
               c4->typeFld = d4field( stat, "TYPE" ) ;
               c4->fileNameFld = d4field( stat, "FILE_NAME" ) ;
               c4->offsetFld = d4field( stat, "OFFSET" ) ;
               c4->lengthFld = d4field( stat, "LENGTH" ) ;
               if ( c4->typeFld == 0 || c4->fileNameFld == 0 || c4->offsetFld == 0 || c4->lengthFld == 0 )
               {
                  d4close( stat ) ;
                  c4->statusDbf = 0 ;
               }
            }
         }
      #endif
   #endif /* S4OFF_OPTIMIZE */

   #ifdef S4WRITE_DELAY
      /* if the begin thread fails, then delay-writes are simply not enabled,
         which is ok because it will simply be bypassed. */
      InitializeCriticalSection( &c4->critical4delayWriteList ) ;
      c4->pendingWriteEvent = CreateEvent( 0, TRUE, FALSE, 0 ) ;
      if ( c4->pendingWriteEvent != INVALID4HANDLE )
      {
         /* cast unsigned long to long for BORLAND C++ avoid warning message
            (verified with Borland) */
         if ( (long)_beginthread( file4writeDelayMain, 5000, c4 ) == -1 )
         {
            CloseHandle( c4->pendingWriteEvent ) ;
            DeleteCriticalSection( &c4->critical4delayWriteList ) ;
         }
         else
            while ( c4->delayWritesEnabled != 1 )  /* ensure thread starts to avoid initUndo thread corruptions */
               Sleep( 0 ) ;
      }
   #endif

   #ifdef S4READ_ADVANCE
      /* if the begin thread fails, then delay-writes are simply not enabled,
         which is ok because it will simply be bypassed. */
      InitializeCriticalSection( &c4->critical4advanceReadList ) ;
      c4->pendingReadEvent = CreateEvent( 0, TRUE, FALSE, 0 ) ;
      if ( c4->pendingReadEvent != INVALID4HANDLE )
      {
         /* cast unsigned long to long for BORLAND C++ avoid warning message
            (verified with Borland) */
         if ( (long)_beginthread( file4advanceReadMain, 5000, c4 ) == -1 )
         {
            CloseHandle( c4->pendingReadEvent ) ;
            DeleteCriticalSection( &c4->critical4advanceReadList ) ;
         }
         else
            while ( c4->advanceReadsEnabled != 1 )  /* ensure thread starts to avoid initUndo thread corruptions */
               Sleep( 0 ) ;
      }
   #endif

   /***********************************************************************

      This code must reside at the end of the function to ensure that no
      file open/create incorrectly auto-starts the optimization process

    ***********************************************************************/
   #ifndef S4OFF_OPTIMIZE
      c4->hadOpt = 1 ;
         c4->memMaxPercent = 25 ;   /* default use 25% of available memory */
      #ifdef S4OS2
         c4->memStartMax = 0xF0000L ;
      #else
         #ifndef S4WINTEL
            c4->memStartMax = 0xF0000L ;
         #else
            #ifdef S4WINDOWS
               c4->memStartMax = 0xF0000L ;
            #else
               c4->memStartMax = 0x50000L ;
            #endif  /* S4WINDOWS */
         #endif
      #endif
   #endif

   #ifndef S4STAND_ALONE
      #ifdef S4WINTEL
         rc = WSAStartup( MAKEWORD( 1,1 ), &WSAData ) ;
      #endif
      c4->ver4 = code4osVersion() ;
      if ( c4->ver4 <= 0 )
      {
         numCode4--;
         return error4( 0, e4result, E91001 ) ;
      }
      c4->readMessageBufferLen = READ4MESSAGE_BUFFER_LEN ;


      #ifndef S4OFF_THREAD
         list4mutexInit(&c4->writeBuffersAvail) ;
         list4mutexInit(&c4->connectBufferListMutex) ;
      #endif

      if (c4->ver4 == ver4NT )
         c4->readMessageNumBuffers = READ4MESSAGE_NUM_BUFFER ;
      else
         c4->readMessageNumBuffers = 1 ;
      c4->writeMessageBufferLen = WRITE4MESSAGE_BUFFER_LEN ;
      c4->writeMessageNumOutstanding = WRITE4MESSAGE_NUM_BUFFER ;

      #ifdef S4COMM_THREAD
         /* and now start the communications thread */
         c4->inter = (INTER4 *)u4alloc( sizeof( INTER4 ) ) ;
         if ( c4->inter == 0 )
         {
            numCode4--;
            return error4( 0, e4memory, E91001 ) ;
         }
         if ( inter4init( c4->inter, c4 ) < 0 )
         {
            numCode4--;
            return error4( 0, e4result, E91001 ) ;
         }
         c4->interThreadHandle = _beginthread( inter4, 5000, c4->inter ) ;
         if ( c4->interThreadHandle == -1 )
         {
            inter4initUndo( c4->inter ) ;
            numCode4--;
            return error4( 0, e4result, E91001 ) ;
         }
      #endif
   #endif

   #ifdef S4FOX
      c4->compatibility = 26 ;
   #endif

   return 0 ;
}

/* if doInit is set to 1, code4init() is called on the allocated CODE4 */
CODE4 *S4FUNCTION code4allocLow( int doInit, const char *defaultProtocol, long versionId )
{
   CODE4 *c4 ;
   #ifndef S4CBPP
      c4 = (CODE4 *)u4alloc( (long)sizeof( CODE4 ) ) ;
   #else
      #ifdef __cplusplus
         c4 = (CODE4 *)u4alloc( (long)sizeof( Code4 ) ) ;
      #else
         c4 = (CODE4 *)u4alloc( (long)sizeof( CODE4 ) ) ;
      #endif
   #endif

   if ( c4 == 0 )
      return 0 ;

   if ( doInit == 1 )
   {
      code4initLow( c4, defaultProtocol, versionId, sizeof( CODE4 ) ) ;
      c4->didAlloc = 1 ;
   }
   return c4 ;
}

void S4FUNCTION expr4calcDelete( EXPR4CALC *calc )
{
   CODE4 *c4;

   if( !calc || !calc->expr )
      return;

   c4 = calc->expr->codeBase ;

      if( calc->total != 0 )
      {
         expr4free( calc->total->resetExpression );
         l4remove( &c4->totalList, calc->total ) ;
         mem4free( c4->totalMemory, calc->total ) ;
      }
      l4remove( &c4->calcList, calc ) ;
   expr4free( calc->expr ) ;
   mem4free( c4->calcMemory, calc ) ;
}

void S4FUNCTION code4calcReset( CODE4 *c4 )
{
   LIST4 *list ;

   #ifdef E4PARM_HIGH
      if ( c4 == 0 )
      {
         error4( 0, e4parm_null, E90921 ) ;
         return ;
      }
   #endif

      list = &c4->calcList ;

   if( list != 0 )
   {
      while( list->nLink > 0 )
         expr4calcDelete( (EXPR4CALC *)list->lastNode ) ;

   }
}

#ifdef S4MEM_PRINT
   extern char *write4buf ;
   extern FILE4SEQ_WRITE *file4seqPtr ;
#endif

static int code4initUndo2( CODE4 *c4, int doClose )
{
      int errCode ;

   #ifdef E4PARM_HIGH
      if ( c4 == 0 )
         return e4parm_null ;
   #endif

   if ( c4->initialized == 0 )  /* already uninitialized */
      return 0 ;

   #ifdef E4MISC
      if ( numCode4 == 0 )   /* code4initUndo already called... */
      {
         if ( doClose == 1 )   /* if zero, don't error out since severe already */
            return error4( 0, e4info, E81004 ) ;
         else
            return e4info ;
      }
   #endif

   error4set(c4, 0 ) ;

   #ifdef S4SEMAPHORE
      #ifdef S4OS2
         DosCloseMutexSem( hmtx4mem ) ;
         DosCloseMutexSem( hmtx4expr ) ;
      #endif
   #endif

   #ifndef S4OPTIMIZE_OFF
      code4optSuspend( c4 ) ;
   #endif

      #ifndef S4OFF_TRAN
            if ( code4transEnabled( c4 ) )
         if ( code4tranStatus( c4 ) == r4active )
            code4tranRollback( c4 ) ;
      #endif

      if ( doClose )
         code4close( c4 ) ;

      code4calcReset( c4 ) ;
      #ifndef S4OFF_COMMUNICATIONS
            /* if ( c4->defaultServer != 0 ) */
            /* { */
               connection4initUndo( &c4->defaultServer ) ;
            /*    connection4free( c4->defaultServer ) ;*/
            /* } */
         c4->defaultServer.connected = 0 ;
      #endif

      #ifndef S4OFF_TRAN
         code4tranInitUndo( c4 ) ;
      #endif

   #ifdef S4OPTIMIZE_STATS
      if ( c4->statusDbf != 0 )
      {
         d4close( c4->statusDbf ) ;
         c4->statusDbf = 0 ;
      }
   #endif

   #ifdef S4MNDX
      if ( c4->memoUseBuffer != 0 )
      {
         u4free( c4->memoUseBuffer ) ;
         c4->memoUseBuffer = 0 ;
      }
   #endif

   #ifdef S4WRITE_DELAY
      /* set the event semaphore to terminate the delay-write thread */
      if ( c4->delayWritesEnabled )
      {
         c4->initUndoDelayWrite = CreateEvent( 0, TRUE, FALSE, 0 ) ;
         if ( c4->initUndoDelayWrite != INVALID_HANDLE_VALUE )
         {
            InterlockedIncrement( &c4->uninitializeDelayWrite ) ;
            /* notify the write thread */
            if ( SetEvent( c4->pendingWriteEvent ) ) /* wait for the delay-write thread to uninitialize */
            {
               WaitForSingleObject( c4->initUndoDelayWrite, INFINITE ) ;
               Sleep( 0 ) ;
            }
            CloseHandle( c4->initUndoDelayWrite ) ;
            CloseHandle( c4->pendingWriteEvent ) ;
            DeleteCriticalSection( &c4->critical4delayWriteList ) ;
         }
      }
   #endif

   #ifdef S4READ_ADVANCE
      /* set the event semaphore to terminate the advance-read thread */
      if ( c4->advanceReadsEnabled )
      {
         c4->initUndoAdvanceRead = CreateEvent( 0, TRUE, FALSE, 0 ) ;
         if ( c4->initUndoAdvanceRead != INVALID_HANDLE_VALUE )
         {
            InterlockedIncrement( &c4->uninitializeAdvanceRead ) ;
            /* notify the write thread */
            if ( SetEvent( c4->pendingReadEvent ) ) /* wait for the delay-write thread to uninitialize */
            {
               WaitForSingleObject( c4->initUndoAdvanceRead, INFINITE ) ;
               Sleep( 0 ) ;
            }
            CloseHandle( c4->initUndoAdvanceRead ) ;
            CloseHandle( c4->pendingReadEvent ) ;
            DeleteCriticalSection( &c4->critical4advanceReadList ) ;
         }
      }
   #endif

   #ifndef S4OFF_INDEX
      mem4release( c4->indexMemory ) ;
      c4->indexMemory = 0 ;

      mem4release( c4->index4fileMemory ) ;
      c4->index4fileMemory = 0 ;
   #endif

   mem4release( c4->dataMemory ) ;
   c4->dataMemory = 0 ;

   mem4release( c4->data4fileMemory ) ;
   c4->data4fileMemory = 0 ;

   #ifndef S4OFF_MULTI
      mem4release( c4->lockMemory ) ;
      c4->lockMemory = 0 ;

      mem4release( c4->lockLinkMemory ) ;
      c4->lockLinkMemory = 0 ;
   #endif

   #ifndef S4OFF_INDEX
      mem4release( c4->tagMemory ) ;
      c4->tagMemory = 0 ;

      mem4release( c4->tagFileMemory ) ;
      c4->tagFileMemory = 0 ;
   #endif

   mem4release( c4->dataListMemory ) ;
   c4->dataListMemory = 0 ;
   mem4release( c4->relateDataListMemory ) ;
   c4->relateDataListMemory = 0 ;
   mem4release( c4->relateMemory ) ;
   c4->relateMemory = 0 ;
   mem4release( c4->relateListMemory ) ;
   c4->relateListMemory = 0 ;
   mem4release( c4->relationMemory ) ;
   c4->relationMemory = 0 ;

   mem4release( c4->calcMemory ) ;
   c4->calcMemory = 0 ;

   mem4release( c4->bitmapMemory ) ;
   c4->bitmapMemory = 0 ;

   if ( c4->fieldBuffer != 0 )
   {
      u4free( c4->fieldBuffer ) ;
      c4->fieldBuffer = 0 ;
      c4->bufLen = 0 ;
   }

   if ( c4->exprWorkBuf != 0 )
   {
      u4free( c4->exprWorkBuf ) ;
      c4->exprWorkBuf = 0 ;
      c4->exprBufLen = 0 ;
   }

   if ( c4->storedKey != 0 )
   {
      u4free( c4->storedKey ) ;
      c4->storedKey = 0 ;
      c4->storedKeyLen = 0 ;
   }

   if ( c4->errorLog != 0 )
   {
      if ( c4->errorLog->hand != INVALID4HANDLE )
         file4close( c4->errorLog ) ;
      u4free( c4->errorLog ) ;
      c4->errorLog = 0 ;
   }

   #ifdef E4ANALYZE
      c4->debugInt = 0 ; /* Some random value for double checking. */
   #endif


   #ifndef S4OFF_TRAN
      if ( c4->tranData != 0 )
      {
         u4free( c4->tranData ) ;
         c4->tranData = 0 ;
         c4->tranDataLen = 0 ;
      }
   #endif

   #ifndef S4OFF_COMMUNICATIONS
      mem4release( c4->connectLowMemory ) ;
      c4->connectLowMemory = 0 ;
      mem4release( c4->writeMemory ) ;
      c4->writeMemory = 0 ;
   #endif

   #ifdef S4TESTING
      if ( s4test.codeBase == c4 )
      {
         if ( s4test.hand != INVALID4HANDLE )
         {
            #ifdef S4MEM_PRINT
               if ( write4buf != 0 )
               {
                  file4seqWriteFlush( file4seqPtr ) ;
                  u4free( write4buf ) ;
                  write4buf = 0 ;
               }
            #endif
            file4close( &s4test ) ;
         }
      }
   #else
      #ifdef S4TRACK_FILES
         if ( s4test.codeBase == c4 )
            if ( s4test.hand != INVALID4HANDLE)
               file4close( &s4test ) ;
      #endif
   #endif

   #ifndef S4STAND_ALONE
      #ifndef S4OFF_THREAD
         list4mutexInitUndo(&c4->writeBuffersAvail) ;
         list4mutexInitUndo(&c4->connectBufferListMutex) ;
      #endif
      #ifdef S4COMM_THREAD
         if ( c4->inter != 0 )
         {
            inter4shutdownRequest( c4->inter ) ;
            u4free( c4->inter ) ;
            c4->inter = 0 ;
         }
      #endif
      #ifdef S4WINTEL
         WSACleanup() ;
      #endif
   #endif

      /* the server has no current SERVER4CLIENT to get error code of */
      errCode = error4code( c4 ) ;
   c4->initialized = 0 ;
   if ( c4->didAlloc == 1 )
      u4free( c4 ) ;

   numCode4-- ;
   #ifdef S4FILE_EXTENDED
      #ifdef S4FOX
         if (numCode4 == 0 && largeStamp.previousCode4init)
         {
            largeStamp.previousCode4init=0;
            largeStamp.doLargeLocking=0;
         }
      #endif
   #endif
   if ( numCode4 == 0 && resetInProgress == 0)   /* reset memory */
      mem4reset() ;

      return errCode ;
}

void S4FUNCTION code4exit( CODE4 *c4 )
{
   int rc ;

   if ( c4 == 0 )
      rc = -1 ;
   else
   {
      rc = error4code( c4 ) ;
      code4initUndo2( c4, 0 ) ;
   }

   #ifndef S4WINDOWS
      exit( rc ) ;
   #else
      #ifdef S4TESTING
         u4terminate() ;
      #else
         #ifdef S4WINCE
            ExitThread(0) ;
         #else
            FatalAppExit( 0, E4_MESSAG_EXI ) ;
         #endif
      #endif
   #endif
}

int S4FUNCTION code4initUndo( CODE4 *c4 )
{
      return code4initUndo2( c4, 1 ) ;
}

int S4FUNCTION code4close( CODE4 *c4 )
{
   DATA4 *dataOn, *dataNext ;
   LIST4 *list ;

   #ifdef S4VBASIC
      if ( c4parm_check( c4, 1, E91003 ) )
         return -1 ;
   #endif  /* S4VBASIC */

   #ifdef E4PARM_HIGH
      if ( c4 == 0 )
         return error4( 0, e4parm_null, E91003 ) ;
   #endif

   list = tran4dataList( (&(c4->c4trans.trans)) ) ;
   for ( dataNext = (DATA4 *)l4first( list ) ; ; )
   {
      dataOn = dataNext ;
      if ( dataOn == 0 )
         break ;
      dataNext = (DATA4 *)l4next( list, dataNext ) ;
      if ( dataOn == dataNext )   /* error -- stuck in endless loop */
         return -1 ;
      #ifndef S4OFF_OPTIMIZE
         #ifdef S4OPTIMIZE_STATS
            /* don't close the internal opt tracking dbf here. */
            if ( dataOn != dataOn->codeBase->statusDbf )
         #endif
      #endif
         d4close( dataOn ) ;
   }

   if ( error4code( c4 ) < 0 )
      return error4code( c4 ) ;
   return 0 ;
}

/* input must be in capital letters */
DATA4FILE *dfile4data( CODE4 *c4, const char *aliasName )
{
   DATA4FILE *dataOn ;
   #ifdef E4MISC
      DATA4FILE *dataResult ;
   #endif
   #ifdef S4MACINTOSH
      FSSpec macSpec ;
      Str255 macStr ;
      char len ;
   #endif

   #ifdef E4PARM_LOW
      if ( c4 == 0 || aliasName == 0 )
      {
         error4( c4, e4parm_null, E91102 ) ;
         return 0 ;
      }
   #endif

   dataOn = 0 ;
   #ifdef E4MISC
      dataResult = 0 ;
   #endif

   for(;;)
   {
      dataOn = (DATA4FILE *)l4next( &c4->dataFileList, dataOn ) ;
      if ( !dataOn )
         break ;

      if ( strcmp( aliasName, dfile4name( dataOn ) ) == 0 )
      {
      #ifdef S4MACINTOSH
         strcpy( (char *)&macStr, aliasName ) ;
         CtoPstr( (char *)&macStr ) ;
         FSMakeFSSpec( c4->macVol, c4->macDir, macStr, &macSpec);
         len = (*macSpec.name) + 1 ;    /*What is the length of the pascal string*/
         if (( macSpec.vRefNum == dataOn->file.macSpec.vRefNum ) && ( macSpec.parID == dataOn->file.macSpec.parID ) && ( memcmp(macSpec.name, dataOn->file.macSpec.name, (size_t)len)== 0 ) )
         {
      #endif
         #ifdef E4MISC
            if ( dataResult != 0 )
            {
               error4( c4, e4info, E83501 ) ;
               return 0 ;
            }
            dataResult = dataOn ;
         #else
            return dataOn ;
         #endif  /* E4MISC */
      #ifdef S4MACINTOSH
         }
      #endif
      }
   }

   #ifdef E4MISC
      return dataResult ;
   #else
      return dataOn ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
DATA4 *tran4dataName( TRAN4 *trans, const char *name, const long clientId, const int doPath )
{
   DATA4 *dataOn ;
   #ifdef E4MISC
      DATA4 *dataResult ;
   #endif
      char name1[258] ;

   #ifdef E4ANALYZE
      if ( tran4verify( trans, 1 ) < 0 )
         return 0 ;
   #endif

   dataOn = 0 ;
   #ifdef E4MISC
      dataResult = 0 ;
   #endif

      u4nameCurrent( name1, sizeof( name1 ), name ) ;
      #ifdef S4CASE_SEN
         u4nameExt( name1, sizeof( name1 ), ".dbf", 0 ) ;
      #else
         u4nameExt( name1, sizeof( name1 ), ".DBF", 0 ) ;
      #endif

   for (;; )
   {
      dataOn = (DATA4 *)l4next( tran4dataList( trans ), dataOn ) ;
      if ( dataOn == 0 )
         break ;
      if ( strcmp( name, d4alias( dataOn ) ) == 0 )  /* try simple alias check */
      {
         #ifdef E4MISC
            if ( dataResult != 0 )
            {
               error4( trans->c4trans->c4, e4info, E83501 ) ;
               return 0 ;
            }
            dataResult = dataOn ;
         #else
            return dataOn ;
         #endif
      }
         #ifdef E4MISC
            else
         #endif
         /* otherwise, try file name check */
         if ( doPath )
            if ( strcmp( name1, dataOn->dataFile->file.name ) == 0 )
            {
               #ifdef E4MISC
                  if ( dataResult != 0 )
                  {
                     error4( trans->c4trans->c4, e4info, E83501 ) ;
                     return 0 ;
                  }
                  dataResult = dataOn ;
               #else
                  return dataOn ;
               #endif
            }
   }
   #ifdef E4MISC
      return dataResult ;
   #else
      return dataOn ;
   #endif
}

/* gets the data4 corresponding to the serverId, and sets the current clientId to clientId */
DATA4 *tran4data( TRAN4 *trans, const long serverId, const long clientId )
{
   DATA4 *dataOn ;
   #ifdef E4MISC
      DATA4 *dataResult ;
   #endif

   #ifdef E4ANALYZE
      if ( tran4verify( trans, 1 ) < 0 )
         return 0 ;
   #endif

   dataOn = 0 ;
   #ifdef E4MISC
      dataResult = 0 ;
   #endif

   if ( serverId == 0 )  /* invalid, just return failure - clientId is 0 means we don't care, any data4 will do */
      return 0 ;

   for (;; )
   {
      dataOn = (DATA4 *)l4next( tran4dataList( trans ), dataOn ) ;
      if ( dataOn == 0 )
         break ;
      if ( data4serverId( dataOn ) == serverId )
            if ( data4clientId( dataOn ) == clientId )
         {
            #ifdef E4MISC
               if ( dataResult != 0 )
               {
                  error4( trans->c4trans->c4, e4info, E83501 ) ;
                  return 0 ;
               }
               dataResult = dataOn ;
            #else
               break ;
            #endif
         }
   }

   #ifdef E4MISC
      return dataResult ;
   #else
      return dataOn ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
DATA4 *S4FUNCTION code4data( CODE4 *c4, const char *aliasName )
{
   char buf[LEN4DATA_ALIAS+2] ;

   #ifdef S4VBASIC
      if ( c4parm_check( c4, 1, E91102 ) )
         return 0 ;
   #endif  /* S4VBASIC */

   #ifdef E4PARM_HIGH
      if ( c4 == 0 || aliasName == 0 )
      {
         error4( c4, e4parm_null, E91102 ) ;
         return 0 ;
      }
   #endif

   u4ncpy( buf, aliasName, sizeof( buf ) ) ;
   #ifndef S4CASE_SEN
      c4upper( buf ) ;
   #endif

      return tran4dataName( code4trans( c4 ), buf, 0L, 0 ) ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
long S4FUNCTION code4version( CODE4 *c4 )
{
   /* this function must reside in the DLL in order to correctly return the
      DLL'S version of this value */
   return (long)S4VERSION ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION code4indexFormat( CODE4 *c4 )
{
   #ifdef S4OFF_INDEX
      return 0 ;
   #else
      #ifdef E4PARM_HIGH
         if ( c4 == 0 )
            return error4( 0, e4parm_null, E91110 ) ;
      #endif

      #ifdef S4MDX
         return r4mdx ;
      #endif
      #ifdef S4FOX
         return r4cdx ;
      #endif
      #ifdef S4CLIPPER
         return r4ntx ;
      #endif
      #ifdef S4NDX
         return r4ndx ;
      #endif
   #endif
}

void S4FUNCTION code4largeOn( CODE4 *c4 )
{
#ifdef S4STAND_ALONE
#ifndef S4OFF_WRITE
#ifndef S4OFF_TRAN
   #ifdef S4FILE_EXTENDED
      #ifdef S4FOX
         if (largeStamp.doLargeLocking != 1 && code4numCodeBase() > 1)
            error4( c4, e4parm, E91013 ) ;
         if ( l4numNodes( &c4->dataFileList ) != 0 )  /* only allow when no data files open */
            error4( c4, e4parm, E91013 ) ;
         else
         {
            if (largeStamp.doLargeLocking == 0)
            {
               largeStamp.doLargeLocking=1;
               largeStamp.previousCode4init++;
            }
            c4->largeFileOffset = S4LARGE_FILE_OFFSET ;
         }
      #else
         error4( c4, e4notSupported, E91013 ) ;
      #endif
   #else
      error4( c4, e4notSupported, E91013 ) ;
   #endif
#endif
#endif
#endif
}

#ifdef S4STAND_ALONE
#ifndef S4OFF_WRITE
#ifndef S4OFF_TRAN
int S4FUNCTION code4logCreate( CODE4 *c4, const char *fileName, const char *userId )
{
   int rc ;
   static char defaultUser[] = "PUBLIC" ;

   #ifdef E4PARM_HIGH
      if ( c4 == 0 )
         return error4( c4, e4parm_null, E91013 ) ;
   #endif

   if ( userId == 0 )
      userId = defaultUser ;
   else
      if ( userId[0] == 0 )  /* null string */
         userId = defaultUser ;

   if ( c4->c4trans.transFile != 0 )
      return r4logOpen ;

   if ( fileName == 0 )
      #ifdef S4CASE_SEN
         fileName = "C4.log" ;
      #else
         fileName = "C4.LOG" ;
      #endif
   else
      if ( fileName[0] == 0 )  /* null string */
         #ifdef S4CASE_SEN
            fileName = "C4.log" ;
         #else
            fileName = "C4.LOG" ;
         #endif

   rc = code4transFileEnable( &c4->c4trans, fileName, 1 ) ;

   #ifndef S4UTILS
      tran4addUser( &c4->c4trans.trans, 0L, userId, (unsigned short int)strlen( userId ) ) ;
   #endif

   return rc ;
}

const char *S4FUNCTION code4logFileName( CODE4 *c4 )
{
   #ifdef E4PARM_HIGH
      if ( c4 == 0 )
      {
         error4( c4, e4parm_null, E91014 ) ;
         return 0 ;
      }
   #endif

   if ( c4->c4trans.transFile == 0 )
      return 0 ;
   return c4->c4trans.transFile->file.name ;
}

int S4FUNCTION code4logOpen( CODE4 *c4, const char *fileName, const char *userId )
{
   int rc ;
   static char defaultUser[] = "PUBLIC" ;

   #ifdef E4PARM_HIGH
      if ( c4 == 0 )
         return error4( c4, e4parm_null, E91015 ) ;
   #endif

   if ( userId == 0 )
      userId = defaultUser ;
   else
      if ( userId[0] == 0 )  /* null string */
         userId = defaultUser ;

   if ( c4->c4trans.transFile != 0 )
      return r4logOpen ;

   if ( fileName == 0 )
      #ifdef S4CASE_SEN
         fileName = "C4.log" ;
      #else
         fileName = "C4.LOG" ;
      #endif
   else
      if ( fileName[0] == 0 )  /* null string */
         #ifdef S4CASE_SEN
            fileName = "C4.log" ;
         #else
            fileName = "C4.LOG" ;
         #endif

   rc = code4transFileEnable( code4trans( c4 )->c4trans, fileName, 0 ) ;

   #ifndef S4UTILS
      tran4addUser( &c4->c4trans.trans, 0L, userId, (unsigned short int)strlen( userId ) ) ;
   #endif
   return rc ;
}

void S4FUNCTION code4logOpenOff( CODE4 *c4 )
{
   #ifdef E4PARM_HIGH
      if ( c4 == 0 )
      {
         error4( c4, e4parm_null, E91016 ) ;
         return ;
      }
   #endif

   c4->logOpen = 0 ;
}
#endif /* S4OFF_TRAN */
#endif /* S4OFF_WRITE */
#endif /* S4STAND_ALONE */

const char *S4FUNCTION code4indexExtension( CODE4 *c4 )
{
   if ( c4->indexExtension[0] == 0 )
      switch ( code4indexFormat( c4 ) )
      {
         case r4mdx:
            #ifdef S4CASE_SEN
               memcpy( c4->indexExtension, "mdx", 3 ) ;
            #else
               memcpy( c4->indexExtension, "MDX", 3 ) ;
            #endif
            break ;
         case r4cdx:
            #ifdef S4CASE_SEN
               memcpy( c4->indexExtension, "cdx", 3 ) ;
            #else
               memcpy( c4->indexExtension, "CDX", 3 ) ;
            #endif
            break ;
         case r4ntx:
            #ifdef S4CASE_SEN
               memcpy( c4->indexExtension, "ntx", 3 ) ;
            #else
               memcpy( c4->indexExtension, "NTX", 3 ) ;
            #endif
            break ;
         case r4ndx:
            #ifdef S4CASE_SEN
               memcpy( c4->indexExtension, "ndx", 3 ) ;
            #else
               memcpy( c4->indexExtension, "NDX", 3 ) ;
            #endif
            break ;
         default:
            return 0 ;
      }

   return c4->indexExtension ;
}

#ifdef S4FOX
   #define S4FORMAT 1
#endif

#ifdef S4CLIPPER
   #ifdef S4FORMAT
      #error Choose only one CodeBase index file compatibility option.
   #endif
   #define S4FORMAT 2
#endif

#ifdef S4MDX
   #ifdef S4FORMAT
      #error Choose only one CodeBase index file compatibility option.
   #endif
   #define S4FORMAT 4
#endif

   #ifndef S4FORMAT
      #error You must define either S4FOX, S4CLIPPER or S4MDX
   #endif

#ifdef S4DOS
   #define S4OPERATING 0x10
#endif

#ifdef S4WIN16
   #ifdef S4OPERATING
      #error Choose only one of CodeBase switches S4DOS, S4WIN16, __WIN32, S4OS2, __unix__, S4MACINTOSH or S4PASCAL_WIN
   #endif
   #define S4OPERATING 0x20
#endif

#ifdef __WIN32
   #ifdef S4OPERATING
      #error Choose only one of CodeBase switches S4DOS, S4WIN16, __WIN32, S4OS2, __unix__, S4MACINTOSH or S4PASCAL_WIN
   #endif
   #define S4OPERATING 0x40
#endif

#ifdef S4STAND_ALONE
   #define S4STAND_ALONE_VAL 0x80
#else
   #define S4STAND_ALONE_VAL 0x0
#endif

#ifdef S4OS2
   #ifdef S4OPERATING
      #error Choose only one of CodeBase switches S4DOS, S4WIN16, __WIN32, S4OS2, __unix__, S4MACINTOSH or S4PASCAL_WIN
   #endif
   #define S4OPERATING 0x100
#endif

#ifdef __unix__
   #ifdef S4OPERATING
      #error Choose only one of CodeBase switches S4DOS, S4WIN16, __WIN32, S4OS2, __unix__, S4MACINTOSH or S4PASCAL_WIN
   #endif
   #define S4OPERATING 0x200
#endif

#ifdef S4MACINTOSH
   #ifdef S4OPERATING
      #error Choose only one of CodeBase switches S4DOS, S4WIN16, __WIN32, S4OS2, __unix__, S4MACINTOSH or S4PASCAL_WIN
   #endif
   #define S4OPERATING 0x400
#endif

#ifdef S4PASCAL_WIN
   #ifdef S4OPERATING
      #error Choose only one of CodeBase switches S4DOS, S4WIN16, __WIN32, S4OS2, __unix__, S4MACINTOSH or S4PASCAL_WIN
   #endif
   #define S4OPERATING 0x800
#endif

#ifndef S4OPERATING
   #error Must choose one of CodeBase switches S4DOS, S4WIN16, __WIN32, S4OS2, __unix__, S4MACINTOSH or S4PASCAL_WIN
#endif

#ifdef S4CB51
   #define S4CB51_VAL  0x1000
#else
   #define S4CB51_VAL  0
#endif

#ifdef S4SAFE
   #define S4SAFE_VAL  0x2000
#else
   #define S4SAFE_VAL  0
#endif

#ifdef S4LOCK_HOOK
   #define S4LOCK_HOOK_VAL 0x4000
#else
   #define S4LOCK_HOOK_VAL 0
#endif

#ifdef S4MAX
   #define S4MAX_VAL 0x8000
#else
   #define S4MAX_VAL 0
#endif

#ifdef S4TIMEOUT_HOOK
   #define S4TIMEOUT_HOOK_VAL 0x10000L
#else
   #define S4TIMEOUT_HOOK_VAL 0
#endif

#ifdef E4ANALYZE
   #define E4ANALYZE_VAL 0x20000L
#else
   #define E4ANALYZE_VAL 0
#endif

#ifdef E4DEBUG
   #define E4DEBUG_VAL 0x40000L
#else
   #define E4DEBUG_VAL 0
#endif

#ifdef E4HOOK
   #define E4HOOK_VAL 0x80000L
#else
   #define E4HOOK_VAL 0
#endif

#ifdef E4LINK
   #define E4LINK_VAL 0x100000L
#else
   #define E4LINK_VAL 0
#endif

#ifdef E4MISC
   #define E4MISC_VAL 0x200000L
#else
   #define E4MISC_VAL 0
#endif

#ifdef E4OFF
   #define E4OFF_VAL 0x400000L
#else
   #define E4OFF_VAL 0
#endif

#ifdef E4OFF_STRING
   #define E4OFF_STRING_VAL 0x800000L
#else
   #define E4OFF_STRING_VAL 0
#endif

#ifdef E4PARM_HIGH
   #define E4PARM_HIGH_VAL 0x1000000L
#else
   #define E4PARM_HIGH_VAL 0
#endif

#ifdef E4PAUSE
   #define E4PAUSE_VAL 0x2000000L
#else
   #define E4PAUSE_VAL 0
#endif

#ifdef E4STOP
   #define E4STOP_VAL 0x4000000L
#else
   #define E4STOP_VAL 0
#endif

#ifdef E4STOP_CRITICAL
   #define E4STOP_CRITICAL_VAL 0x8000000L
#else
   #define E4STOP_CRITICAL_VAL 0
#endif

#ifdef S4OFF_INDEX
   #define S4OFF_INDEX_VAL 0x10000000L
#else
   #define S4OFF_INDEX_VAL 0
#endif

#ifdef S4OFF_MEMO
   #define S4OFF_MEMO_VAL 0x20000000L
#else
   #define S4OFF_MEMO_VAL 0
#endif

#ifdef S4OFF_MULTI
   #define S4OFF_MULTI_VAL 0x40000000L
#else
   #define S4OFF_MULTI_VAL 0
#endif

#ifdef S4OFF_OPTIMIZE
   #define S4OFF_OPTIMIZE_VAL 0x80000000L
#else
   #define S4OFF_OPTIMIZE_VAL 0
#endif

/*
   no room for these switches

#ifdef S4OFF_REPORT
   #define S4OFF_REPORT_VAL 0x100000000
#else
   #define S4OFF_REPORT_VAL 0
#endif

#ifdef S4OFF_TRAN
   #define S4OFF_TRAN_VAL 0x200000000
#else
   #define S4OFF_TRAN_VAL 0
#endif

#ifdef S4OFF_WRITE
   #define S4OFF_WRITE_VAL 0x400000000
#else
   #define S4OFF_WRITE_VAL 0
#endif
*/

long S4FUNCTION u4switch()
{
   return (long)
          ( S4FORMAT + S4OPERATING + S4CB51_VAL + S4SAFE_VAL
          + S4LOCK_HOOK_VAL + S4MAX_VAL + S4TIMEOUT_HOOK_VAL + E4ANALYZE_VAL
          + E4DEBUG_VAL + E4HOOK_VAL + E4LINK_VAL + E4MISC_VAL + E4OFF_VAL
          + E4OFF_STRING_VAL + E4PARM_HIGH_VAL + E4PAUSE_VAL + E4STOP_VAL +
          E4STOP_CRITICAL_VAL + S4OFF_INDEX_VAL + S4OFF_MEMO_VAL +
          S4OFF_MULTI_VAL + S4OFF_OPTIMIZE_VAL +
/*          S4OFF_REPORT_VAL + S4OFF_TRAN_VAL + S4OFF_WRITE_VAL +  S4CLIEN T_VAL +  no room */


           S4STAND_ALONE_VAL ) ;
}

#ifdef S4VB_DOS

DATA4 *d4data_v( CODE4 *c4, char *alias )
{
   return code4data( c4, c4str(alias) ) ;
}

#endif

/*#ifdef S4CB51*/
#ifdef S4DLL_BUILD
#ifndef S4WINCE
#ifndef S4PASCAL_DOS

#define CTRL4SERVERNAMESIZE 260

/*Structures for CodeControls Functions */

typedef   struct ctrl4code_tag
{
   LINK4       link ;
   HINSTANCE   hInst ;
   CODE4       *code ;
   int         alloc ;
   LIST4       form ;


}CTRL4CODE ;

/***************************************************************\
\***************************************************************/

#ifdef __cplusplus
   extern "C" {
#endif
int         S4FUNCTION ctrl4addCode( HINSTANCE hInst ) ;                                 /*450*/
void        S4FUNCTION ctrl4codeListInit( void ) ;                                        /*451*/
void        S4FUNCTION ctrl4freeCtrlNode( CTRL4CODE *node ) ;
void        S4FUNCTION ctrl4freeCodeList( void ) ;                                        /*452*/
CTRL4CODE * S4FUNCTION ctrl4getCtrlCode( HINSTANCE hInst ) ;                              /*453*/
void        S4FUNCTION ctrl4getServerName( HINSTANCE hInst,char *serverName,int strLen ) ; /* 5 oh something */
void        S4FUNCTION ctrl4setServerName( HINSTANCE hInst,char *serverName ) ; /* 5 oh something */
void        S4FUNCTION ctrl4initVBX( CODE4 *code,HINSTANCE hInstance,int initialize ) ;   /*454*/
void        S4FUNCTION ctrl4initVBXUndo( CODE4 *code,HINSTANCE hInstance ) ;              /*455*/
#ifdef __cplusplus
   }
#endif

/***************************************************************\
*  List Containing CODE4 and hInstance
*  structures for CodeControls
\***************************************************************/

LIST4    ctrl4codeListVBX ;


/***************************************************************\
\***************************************************************/

void S4FUNCTION ctrl4codeListInit( void )
{
      /*memset( &ctrl4codeListVBX,0,sizeof( LIST4 ) );*/
}

/***************************************************************\
\***************************************************************/

int S4FUNCTION ctrl4addCode( HINSTANCE hInst )
{
   CTRL4CODE   *node ;

   if ( !hInst )
      return -1 ;

   node = ( CTRL4CODE * ) u4alloc( sizeof( CTRL4CODE ) ) ;
   if ( node )
   {
      node->hInst = hInst ;
           node->alloc = 1 ;
      node->code = NULL ;
      l4add( &ctrl4codeListVBX,node ) ;
   }
   return 0 ;
}

/***************************************************************\
\***************************************************************/

void S4FUNCTION ctrl4freeCtrlNode( CTRL4CODE *node )
{
   if ( node )
   {
      l4remove( &ctrl4codeListVBX,node ) ;
      if ( node->code )
      {
         code4initUndo( node->code ) ;
         u4free( node->code ) ;
      }
      u4free( node ) ;
   }
}

/***************************************************************\
\***************************************************************/

void S4FUNCTION ctrl4freeCodeList( void )
{
   CTRL4CODE   *node ;

   for( node = ( CTRL4CODE * ) l4first( &ctrl4codeListVBX );node != NULL; )
   {
      l4remove( &ctrl4codeListVBX,node ) ;
                if ( node->code && node->alloc )
                {
         code4initUndo( node->code ) ;
         u4free( node->code ) ;
                }
      u4free( node ) ;
      node = ( CTRL4CODE * ) l4next( &ctrl4codeListVBX,node ) ;
   }
}

/***************************************************************\
\***************************************************************/

CTRL4CODE * S4FUNCTION ctrl4getCtrlCode( HINSTANCE hInst )
{
   CTRL4CODE   *node,*returnnode = NULL ;

   if ( hInst == 0 )
      return NULL ;

   for( node = ( CTRL4CODE * ) l4first( &ctrl4codeListVBX );node != NULL; )
   {
      if ( node->hInst == hInst )
      {
         returnnode = node ;
         node = NULL ;
      }
      else
         node = ( CTRL4CODE * ) l4next( &ctrl4codeListVBX,node ) ;
   }
   return returnnode ;
}

/***************************************************************\
\***************************************************************/

void S4FUNCTION ctrl4initVBXUndo( CODE4 *code,HINSTANCE hInstance )
{
   CTRL4CODE   *node ;

   node = ctrl4getCtrlCode( hInstance ) ;
   if ( node )
   {
      if ( code != node->code )
         code4initUndo( code ) ;

      code4initUndo( node->code ) ;
      if ( node->alloc )
      {
         u4free( node->code ) ;
      }
      l4remove( &ctrl4codeListVBX,node ) ;
      u4free( node ) ;
   }
}

/***************************************************************\
\***************************************************************/

void S4FUNCTION ctrl4initVBX( CODE4 *code, HINSTANCE hInstance, int initialize )
{
   int dealloc ;
   CODE4 *oldcode ;
   CTRL4CODE *node ;

   if ( code )
   {
      node = ctrl4getCtrlCode( hInstance ) ;
      if ( !node )
      {
         ctrl4addCode( hInstance ) ;
         node = ctrl4getCtrlCode( hInstance ) ;
         if ( node )
            node->alloc = 0 ;
      }

      if ( node )
      {
         if ( node->code )
         {
            dealloc = 0 ;
            if ( node->alloc )
               dealloc = IDYES ;
            else
               dealloc = MessageBox( 0,
               "Warning! Detected two calls to ctrl4init\nwithout intervening call to ctrl4initUndo!\n\nOk to free previously allocated CODE4 memory ?","Multiple CODE4 Detected"
               ,MB_YESNO|MB_TASKMODAL|MB_ICONEXCLAMATION ) ;

            if ( dealloc == IDYES )
            {
               oldcode = node->code ;
               ctrl4initVBXUndo( node->code,hInstance ) ;
               u4free( oldcode ) ;
               node->code = NULL ;
            }
         }
         node->alloc = 0 ;
         node->code = code ;

         if ( initialize )
         {
               code4init( code ) ;
         }
      }
   }
}


#ifdef S4STAND_ALONE
#ifdef P4ARGS_USED
   #pragma argsused
#endif
void  S4FUNCTION ctrl4getServerName( HINSTANCE hInst,char *serverName,int strLen ) /*5 oh something*/
{
   return ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
void S4FUNCTION ctrl4setServerName( HINSTANCE hInst,char *serverName ) /*5 oh something*/
{
   return ;
}
#endif /* S4STAND_ALONE */

#endif /* S4PASCAL_DOS */
#endif /* S4WINCE */
#endif /* S4DLL_BUILD */

/* functions used to set and get CODE4 flags from outside of a DLL in cases
   where the structures are unknown (eg. index independent program) */
/* cannot be defines */
#ifndef S4CLIPPER
INDEX4FILE *S4FUNCTION i4getIndexFileDo( const INDEX4 *i4 )
{
   return i4->indexFile ;
}
#endif

int S4FUNCTION c4getAccessMode( const CODE4 *c4 )
{
   return c4->accessMode ;
}

int S4FUNCTION c4getAutoOpen( const CODE4 *c4 )
{
   return c4->autoOpen ;
}

short S4FUNCTION c4getCompatibility( const CODE4 *c4 )
{
   return c4->compatibility ;
}

int S4FUNCTION c4getErrorCode( const CODE4 *c4 )
{
   return error4code( c4 ) ;
}

int S4FUNCTION c4getErrDefaultUnique( const CODE4 *c4 )
{
   return c4->errDefaultUnique ;
}

int S4FUNCTION c4getErrExpr( const CODE4 *c4 )
{
   return c4->errExpr ;
}

int S4FUNCTION c4getErrFieldName( const CODE4 *c4 )
{
   return c4->errFieldName ;
}

int S4FUNCTION c4getErrGo( const CODE4 *c4 )
{
   return c4->errGo ;
}

int S4FUNCTION c4getErrOpen( const CODE4 *c4 )
{
   return c4->errOpen ;
}

int S4FUNCTION c4getErrRelate( const CODE4 *c4 )
{
   return c4->errRelate ;
}

int S4FUNCTION c4getErrSkip( const CODE4 *c4 )
{
   return c4->errSkip ;
}

int S4FUNCTION c4getErrTagName( const CODE4 *c4 )
{
   return c4->errTagName ;
}

int S4FUNCTION c4getLockAttempts( const CODE4 *c4 )
{
   return c4->lockAttempts ;
}

int S4FUNCTION c4getLockEnforce( const CODE4 *c4 )
{
   return c4->lockEnforce ;
}

int S4FUNCTION c4getOptimize( const CODE4 *c4 )
{
   return c4->optimize ;
}

int S4FUNCTION c4getOptimizeWrite( const CODE4 *c4 )
{
   return c4->optimizeWrite ;
}

int S4FUNCTION c4getReadLockDo( const CODE4 *c4 )
{
   return c4->readLock ;
}

int S4FUNCTION c4getReadOnlyDo( const CODE4 *c4 )
{
   return c4->readOnly ;
}

int S4FUNCTION c4getSafety( const CODE4 *c4 )
{
   return c4->safety ;
}

int S4FUNCTION c4getSingleOpen( const CODE4 *c4 )
{
   return c4->singleOpen ;
}

const char *S4FUNCTION t4getExprSource( TAG4 *t4 )
{
   return t4expr( t4 ) ;
}

void S4FUNCTION c4setAccessMode( CODE4 *c4, char val )
{
   c4->accessMode = val ;
}

void S4FUNCTION c4setAutoOpen( CODE4 *c4, int val )
{
   c4->autoOpen = val ;
}

void S4FUNCTION c4setCompatibility( CODE4 *c4, short val )
{
   c4->compatibility = val ;
}

void S4FUNCTION c4setErrorCode( CODE4 *c4, int val )
{
   error4set( c4, val ) ;
}

void S4FUNCTION c4setErrExpr( CODE4 *c4, int val )
{
   c4->errExpr = val ;
}

void S4FUNCTION c4setErrFieldName( CODE4 *c4, int val )
{
   c4->errFieldName = val ;
}

void S4FUNCTION c4setErrGo( CODE4 *c4, int val )
{
   c4->errGo = val ;
}

void S4FUNCTION c4setErrOpen( CODE4 *c4, int val )
{
   c4->errOpen = val ;
}

void S4FUNCTION c4setErrRelate( CODE4 *c4, int val )
{
   c4->errRelate = val ;
}

void S4FUNCTION c4setErrSkip( CODE4 *c4, int val )
{
   c4->errSkip = val ;
}

void S4FUNCTION c4setErrTagName( CODE4 *c4, int val )
{
   c4->errTagName = val ;
}

void S4FUNCTION c4setLockAttempts( CODE4 *c4, int val )
{
   c4->lockAttempts = val ;
}

void S4FUNCTION c4setLockEnforce( CODE4 *c4, int val )
{
   c4->lockEnforce = val ;
}

void S4FUNCTION c4setOptimize( CODE4 *c4, int val )
{
   c4->optimize = val ;
}

void S4FUNCTION c4setOptimizeWrite( CODE4 *c4, int val )
{
   c4->optimizeWrite = val ;
}

void S4FUNCTION c4setReadLockDo( CODE4 *c4, char val )
{
   c4->readLock = val ;
}

void S4FUNCTION c4setReadOnlyDo( CODE4 *c4, int val )
{
   c4->readOnly = val ;
}

void S4FUNCTION c4setSafety( CODE4 *c4, char val )
{
   c4->safety = val ;
}

void S4FUNCTION c4setSingleOpen( CODE4 *c4, short val )
{
   c4->singleOpen = val ;
}

void S4FUNCTION c4setErrDefaultUnique( CODE4 *c4, short val )
{
   c4->errDefaultUnique = val ;
}

#ifndef S4STAND_ALONE
int S4FUNCTION code4osVersion(void)
{
   #ifdef S4WINTEL
      int rc ;
      OSVERSIONINFO verInfo ;

      verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
      rc = GetVersionEx(&verInfo) ;
      if (!rc)
         return -1 ;
      switch ( verInfo.dwPlatformId )
      {
         case VER_PLATFORM_WIN32s:
            return ver431 ;
         case VER_PLATFORM_WIN32_WINDOWS:
            return ver495 ;
         case VER_PLATFORM_WIN32_NT:
            return ver4NT ;
         default:
            return -1 ;
      }
   #else
      return ver4Unix ;
   #endif
}
#endif


#ifndef S4OFF_TRAN
#ifndef S4OFF_WRITE

int code4tranInitUndoLow( TRAN4 *t4, const long clientId )
{
   int rc ;

   if ( t4 == 0 )
      return 0 ;

   if ( t4->c4trans->enabled == 1 && t4->userId[0] != 0 )  /* if it has been initialized */
   {
      rc = tran4set( t4, t4->currentTranStatus, -1L, clientId, TRAN4INIT_UNDO, 0, 0L, 0L ) ;
      if ( rc < 0 )
         return rc ;
      if ( tran4lowAppend( t4, 0, 0 ) != 0 )
         return e4transAppend ;
      memset( t4->userId, 0, sizeof( t4->userId ) ) ;
   }

   t4->dataPos = 0 ;

   return 0 ;
}

#ifdef S4STAND_ALONE
void code4tranInitUndo( CODE4 *c4 )
{
   #ifdef E4PARM_LOW
      if ( c4 == 0 )
      {
         error4( 0, e4parm_null, E93804 ) ;
         return ;
      }
   #endif

   if ( code4transEnabled( c4 ) )
      code4transInitUndo( &c4->c4trans ) ;
}

int S4FUNCTION code4tranInit2( CODE4 *c4, const char *fileName, const char *charId )
{
   int rc ;

   #ifdef E4PARM_LOW
      if ( c4 == 0 || fileName == 0 )
         return error4( 0, e4parm_null, E93801 ) ;
   #endif

   c4->c4trans.c4 = c4 ;
   rc = code4transFileEnable( &c4->c4trans, fileName, 0 ) ;
   if ( rc < 0 )
      return rc ;
   c4->c4trans.trans.c4trans = &c4->c4trans ;

   if ( charId != 0 )
      return tran4addUser( &c4->c4trans.trans, 0L, charId, ( unsigned short )strlen( charId ) ) ;

   return 0 ;
}
#endif /* S4STAND_ALONE */

#endif /* S4OFF_WRITE */
#endif /* S4OFF_TRAN */

#ifdef S4STAND_ALONE
int S4FUNCTION code4tranInit( CODE4 *c4 )
{
   #ifdef E4PARM_LOW
      if ( c4 == 0 )
         return error4( 0, e4parm_null, E93802 ) ;
   #endif

   c4->c4trans.c4 = c4 ;
   c4->c4trans.trans.c4trans = &c4->c4trans ;
   tran4dataListSet( &c4->c4trans.trans, &c4->c4trans.trans.localDataList ) ;
   return 0 ;
}
#endif /* S4STAND_ALONE */
