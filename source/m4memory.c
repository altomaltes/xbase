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

/* m4memory.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#ifdef S4OS2
   #ifdef __DLL__
      #define  INCL_DOSMEMMGR
   #endif
#endif

#include "d4all.h"

#ifdef S4VB_DOS
   #include "malloc.h"
#endif

#ifdef S4OS2
   #ifdef __DLL__
      #include <bsememf.h>
   #endif
#endif

#ifdef S4WINDOWS
   #define S4WIN_ALLOC
#else
   #ifdef __WIN32
      #define S4WIN_ALLOC
   #endif
#endif

/* for 32 bit, allow more than 20000 memory allocations... */

#ifdef __WIN32
   #define MEM4MAX_POINTERS 200000
#else
   #define MEM4MAX_POINTERS 20000
#endif

#define mem4numTypes 10

#ifdef S4MEM_PRINT
   #ifdef S4WIN_ALLOC
      #ifdef S4TESTING
         #include "t4test.h"
      #endif
   #endif
   int v4print = 0 ; /* if v4print == 0 then stdout, else stdprn, no output if -1 */
   /* use globals to hold the current file name and line number */
   const char *m4fileName = 0 ;
   int m4lineNo = 0 ;
   #ifdef S4WINDOWS
      int  S4FUNCTION code4memFileName( void )
      {
         return m4lineNo ;
      }

      const char *S4FUNCTION code4memFileNo( void )
      {
         return m4fileName ;
      }

      void S4FUNCTION code4memLineNoSet( int val )
      {
         m4lineNo = val ;
      }

      void S4FUNCTION code4memFileNameSet( const char *ptr )
      {
         m4fileName = ptr ;
      }
   #endif
#endif

typedef struct
{
   LINK4  link ;
   MEM4  types[mem4numTypes] ;
} MEMORY4GROUP ;

#ifdef __WIN32
   #ifdef S4SEMAPHORE
      /* multi-thread support */
      CRITICAL_SECTION critical4memory, critical4expression ;
   #endif
#endif

#ifndef S4CBPP
   static LIST4  avail = { 0, 0, 0 } ;   /* A list of available MEM4 entries */
   static LIST4  used = { 0, 0, 0 } ;   /* A list of used MEM4 entries */
   static LIST4  groups = { 0, 0, 0 } ; /* A list of Allocated MEM4 groups */
#else
   /* initialization with class and union structures */
   static LIST4  avail  = { {0}, 0, {0} } ;   /* A list of available MEM4 entries */
   static LIST4  used   = { {0}, {0}, {0} } ;   /* A list of used MEM4 entries */
   static LIST4  groups = { {0}, 0, {0} } ;   /* A list of Allocated MEM4 groups */
#endif
#ifdef S4SEMAPHORE
   #ifdef __WIN32
      static int memoryInitialized = 0 ;
   #endif
#endif

int resetInProgress = 0 ;

#ifdef S4SEMAPHORE
#ifdef __WIN32
int mem4start( CODE4 *c4 )
{
   if ( memoryInitialized == 0 )
      return -1 ;

   EnterCriticalSection( &critical4memory ) ;

   return 0 ;
}

void mem4stop( CODE4 *c4 )
{
   if ( memoryInitialized == 0 )
      return ;

   LeaveCriticalSection( &critical4memory ) ;
}
#else
static int mem4start( CODE4 *c4 )
{
   APIRET rc ;

   #ifdef E4ANALYZE
      if ( c4 == 0 )
         return error4( c4, e4info, "OS/2 Semaphore Failure" ) ;
   #endif

   rc = DosRequestMutexSem( c4->hmtxMem, -1 ) ;
   if ( rc != 0 )
      return error4( c4, e4info, "OS/2 Semaphore Failure" ) ;
   return 0 ;
}

static void mem4stop( CODE4 *c4 )
{
   #ifdef E4ANALYZE
      if ( c4 == 0 )
      {
         error4( c4, e4info, "OS/2 Semaphore Failure" ) ;
         return ;
      }
   #endif

   DosReleaseMutexSem( c4->hmtxMem ) ;
}
#endif /* __WIN32 */
#endif /* S4SEMAPHORE */

#ifdef E4MISC

static char **mem4testPointers ;
static int mem4numPointer = -1 ;
static int mem4numUsed = 0 ;

#ifdef S4DATA_ALIGN
   #define mem4extraChars 12
   #define mem4extraTot (2*(mem4extraChars + sizeof(S4LONG)))
#else
   #define mem4extraChars 10
   #define mem4extraTot (mem4extraChars*2 + sizeof(unsigned))
#endif
#define mem4checkChar 0x55

#define MEM4INC 500

/* Returns the pointer to be returned; is passed the pointer allocated by malloc ... */
static char *mem4fixPointer( char *startPtr, unsigned largeLen )
{
   char *returnPtr ;
   unsigned pos ;

   memset( startPtr, mem4checkChar, mem4extraChars ) ;
   returnPtr = startPtr + mem4extraChars ;

   memcpy( returnPtr, (void *)&largeLen, sizeof(largeLen) ) ;
   pos = largeLen - mem4extraChars ;
   memset( startPtr+ pos, mem4checkChar, mem4extraChars ) ;

   return returnPtr + sizeof(unsigned) ;
}

/* Returns the pointer allocated by malloc; */
/* passed by pointer returned by 'mem4fixPointer' */
static char *mem4checkPointer( char *returnPtr, int clear )
{
   unsigned *largeLenPtr ;
   char *mallocPtr, *testPtr ;
   int i, j ;

   largeLenPtr = (unsigned *)(returnPtr - sizeof(unsigned)) ;
   mallocPtr = returnPtr - sizeof(unsigned) - mem4extraChars ;

   for ( j = 0; j < 2; j++ )
   {
      if (j == 0)
         testPtr = mallocPtr ;
      else
         testPtr = mallocPtr + *largeLenPtr - mem4extraChars ;

      for ( i = 0 ; i < mem4extraChars ; i++ )
         if ( testPtr[i] != mem4checkChar )
         {
            #ifdef S4MEM_PRINT
               if ( v4print != -1 )
               {
                  #ifdef S4WIN_ALLOC
                     #ifdef S4TESTING
                        d4displayStr( mem4displayPtr, "corrupt memory: ", 1 ) ;
                        d4displayPtr( mem4displayPtr, mallocPtr- sizeof(unsigned) - mem4extraChars, 0 ) ;
                     #endif
                  #else
                     if ( v4print )
                        fprintf( stdprn, "\r\ncorrupt memory: %p", mallocPtr- sizeof(unsigned) - mem4extraChars ) ;
                     else
                        printf( "\r\ncorrupt memory: %p", mallocPtr- sizeof(unsigned) - mem4extraChars ) ;
                  #endif
               }
            #endif
            error4( 0, e4result, E85901 ) ;
            return 0 ;
         }
   }
   if ( clear == 1 ) /* null the memory to potentially detect re-use, including clearing check chars */
      memset( mallocPtr, 0, *largeLenPtr ) ;
   return mallocPtr ;
}

static int mem4pushPointer( char *ptr )
{
   #ifdef S4WIN_ALLOC
      HANDLE handle, *hPtr, *oldHPtr ;
      hPtr = (HANDLE *)0 ;
   #endif

   if ( mem4numPointer < 0 )
   {
      #ifdef S4WIN_ALLOC
         #ifdef __DLL__
            handle = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, (DWORD) sizeof(char *) * MEM4INC + sizeof(HANDLE) ) ;
         #else
            handle = GlobalAlloc( GMEM_MOVEABLE | GMEM_ZEROINIT, (DWORD) sizeof(char *) * MEM4INC + sizeof(HANDLE) ) ;
         #endif

         if ( handle == (HANDLE) 0 )
            return error4( 0, e4memory, E95901 ) ;

         hPtr = (HANDLE *)GlobalLock( handle ) ;
         *hPtr++ = handle ;
         mem4testPointers = (char **)hPtr ;
      #else
         mem4testPointers = (char **)malloc( sizeof(char *) * MEM4INC ) ;
      #endif
      mem4numPointer = MEM4INC ;
   }
   if ( mem4numPointer == mem4numUsed )
   {
      mem4numPointer += MEM4INC ;
      if ( mem4numPointer > MEM4MAX_POINTERS )
         return error4( 0, e4result, E95901 ) ;

      #ifdef S4WIN_ALLOC
         oldHPtr = (HANDLE *)(mem4testPointers) ;
         oldHPtr-- ;  /* get the actual handle */

         GlobalUnlock( *oldHPtr ) ;

         #ifdef __DLL__
            handle = GlobalReAlloc( *oldHPtr, (DWORD)sizeof(char *) * mem4numPointer + sizeof( HANDLE ), GMEM_MOVEABLE ) ;
         #else
            handle = GlobalReAlloc( *oldHPtr, (DWORD)sizeof(char *) * mem4numPointer + sizeof( HANDLE ), GMEM_MOVEABLE ) ;
         #endif

         if ( handle == (HANDLE) 0 )
            return error4( 0, e4memory, E95901 ) ;

         hPtr = (HANDLE *)GlobalLock( handle ) ;
         *hPtr++ = handle ;
         mem4testPointers = (char **)hPtr ;
      #else
         mem4testPointers = (char **)realloc( (void *)mem4testPointers, sizeof(char *)*mem4numPointer ) ;
      #endif
   }

   if ( mem4testPointers == 0 )
      return error4( 0, e4memory, E95901 ) ;

   mem4testPointers[mem4numUsed++] = ptr ;

   return 0 ;
}

static int mem4popPointer( char *ptr )
{
   int i ;

   if ( mem4testPointers == 0 )   /* failure in allocating tracking memory, so allow shutdown */
   {
      mem4numUsed-- ;
      return 0 ;
   }

   for ( i = mem4numUsed - 1 ; i >= 0 ; i-- )
      if ( mem4testPointers[i] == ptr )
      {
         /* This 'memmove' may create compile warning */
         c4memmove( mem4testPointers+i, mem4testPointers+i+1, (size_t) (sizeof(char *) * (mem4numUsed-i-1))) ;
         mem4numUsed-- ;
         return 0 ;
      }

   return error4( 0, e4result, E95902 ) ;
}

int S4FUNCTION mem4checkMemory()
{
   int i ;
   char *ptr ;

   if ( code4numCodeBase() == 0 && resetInProgress == 0 )
      return 0 ;

   #ifdef S4SEMAPHORE
      mem4start( 0 ) ;
   #endif

   for ( i = 0; i < mem4numUsed; i++ )
   {
      ptr = mem4checkPointer( mem4testPointers[i], 0 ) ;
      if ( ptr == 0 )
         return error4( 0, e4result, E95903 ) ;
   }

   #ifdef S4SEMAPHORE
      mem4stop( 0 ) ;
   #endif

   return 0 ;
}

int S4FUNCTION mem4freeCheck( const int maxLeft )
{
   #ifdef S4MEM_PRINT
      int i ;
      if ( v4print != -1 )
      {
         #ifdef S4WIN_ALLOC
            #ifdef S4TESTING
               for ( i = 0; i < mem4numUsed; i++ )
               {
                  d4displayStr( mem4displayPtr, "mem4freeCheck: ", 1 ) ;
                  d4displayPtr( mem4displayPtr, mem4testPointers[i], 0 ) ;
               }
            #endif
         #else
            if ( v4print )
               for ( i = 0; i < mem4numUsed; i++ )
                  fprintf( stdprn, "\r\nmem4freeCheck: %p", mem4testPointers[i] ) ;
            else
               for ( i = 0; i < mem4numUsed; i++ )
                  printf( "\r\nmem4freeCheck: %p", mem4testPointers[i] ) ;
         #endif
      }
   #endif

   if ( mem4numUsed > maxLeft )
      return error4( 0, e4result, 95904 ) ;

   return ( mem4numUsed ) ;
}
#endif

void *S4FUNCTION mem4allocDefault( MEM4 *memoryType )
{
   #ifdef E4PARM_HIGH
      if ( memoryType == 0 )
      {
         error4( 0, e4parmNull, E95905 ) ;
         return 0 ;
      }
   #endif

   return mem4alloc2Default( memoryType, 0 ) ;
}

static Y4CHUNK *mem4allocChunkDefault( MEM4 *typePtr )
{
   Y4CHUNK *chunkPtr ;
   int  nAllocate, i ;
   char *ptr ;
   long allocSize ;

   nAllocate = typePtr->unitExpand ;
   if ( l4last( &typePtr->chunks ) == 0 )
      nAllocate = typePtr->unitStart ;

   for ( ;; )
   {
      allocSize = (long)sizeof( LINK4 ) + (long)nAllocate * typePtr->unitSize ;
      if ( allocSize < UINT_MAX )
         break ;
      if ( nAllocate <= 1 )
         return 0 ;
      nAllocate = nAllocate / 2 ;
   }

   #ifdef S4DOS    /* no memory sharing under DOS, so can use a CODE4 for limited memory */
      chunkPtr = (Y4CHUNK *)u4allocFreeDefault( typePtr->codeBase, allocSize ) ;
   #else
      chunkPtr = (Y4CHUNK *)u4alloc( allocSize ) ;
   #endif
   if ( chunkPtr == 0 )
      return 0 ;
   ptr = (char *)&chunkPtr->data ;
   for ( i = 0 ; i < nAllocate ; i++ )
      l4add( &typePtr->pieces, (LINK4 *)( ptr + i * typePtr->unitSize ) ) ;

   return  chunkPtr ;
}

static void *mem4allocLow( MEM4 *memoryType )
{
   LINK4 *nextPiece ;
   Y4CHUNK *newChunk ;
   #ifdef E4MISC
      char *ptr ;
   #endif
   #ifdef S4OS2_SEMAPHORE
      #ifdef __DLL__
         ULONG flags ;
      #endif
   #endif

   if ( memoryType == 0 )
      return 0 ;
   nextPiece = (LINK4 *)l4pop( &memoryType->pieces ) ;

   if ( nextPiece != 0 )
   {
      #ifdef S4SEMAPHORE
         #ifdef S4OS2
            #ifdef __DLL__
               /* get access to the memory */
               flags = PAG_WRITE | PAG_READ  ;
               if ( DosGetSharedMem( nextPiece, flags ) != 0 )
                  return 0 ;
            #endif
         #endif
      #endif
      #ifdef E4MISC
         memoryType->nUsed++ ;
         ptr = mem4fixPointer( (char *)nextPiece, memoryType->unitSize ) ;

         if ( mem4pushPointer( ptr ) == e4memory )
         {
            memoryType->nUsed-- ;
            l4add( &memoryType->pieces, nextPiece ) ;
            return 0 ;
         }

         #ifdef S4MEM_PRINT
            if ( v4print != -1 )
            {
               #ifdef S4WIN_ALLOC
                  #ifdef S4TESTING
                     {
                        d4displayStr( mem4displayPtr, "  Y4ALLOC:  ", 1 ) ;
                        d4displayPtr( mem4displayPtr, ptr, 0 ) ;
                        d4displayStr( mem4displayPtr, "  file:  ", 0 ) ;
                        d4displayStr( mem4displayPtr, m4fileName, 0 ) ;
                        d4displayStr( mem4displayPtr, "  line:  ", 0 ) ;
                        d4displayNum( mem4displayPtr, m4lineNo, 0 ) ;
                     }
                  #endif
               #else
                  if ( v4print )
                     fprintf( stdprn, "\r\n  Y4ALLOC:  %lx  file:  %s  line:  %12d", ptr, m4fileName, m4lineNo ) ;
                  else
                     printf( "\r\n  Y4ALLOC:  %lx  file:  %s  line:  %12d", ptr, m4fileName, m4lineNo  ) ;
               #endif
            }
         #endif

         return (void *)ptr ;
      #else
         return nextPiece ;
      #endif
   }

   if ( (newChunk = mem4allocChunkDefault( memoryType )) == 0 )
      return 0 ;
   l4add( &memoryType->chunks, &newChunk->link ) ;

   memoryType->nUsed++ ;
   #ifdef E4MISC
      ptr = mem4fixPointer( (char *)l4pop(&memoryType->pieces),
                            memoryType->unitSize ) ;
      #ifdef S4MEM_PRINT
         if ( v4print != -1 )
         {
            #ifdef S4WIN_ALLOC
               #ifdef S4TESTING
                  {
                     d4displayStr( mem4displayPtr, "  Y4ALLOC:  ", 1 ) ;
                     d4displayPtr( mem4displayPtr, ptr, 0 ) ;
                     d4displayStr( mem4displayPtr, "  file:  ", 0 ) ;
                     d4displayStr( mem4displayPtr, m4fileName, 0 ) ;
                     d4displayStr( mem4displayPtr, "  line:  ", 0 ) ;
                     d4displayNum( mem4displayPtr, m4lineNo, 0 ) ;
                  }
               #endif
            #else
               if ( v4print )
                  fprintf( stdprn, "\r\n  Y4ALLOC:  %lx  file:  %s  line:  %12d", ptr, m4fileName, m4lineNo ) ;
               else
                  printf( "\r\n  Y4ALLOC:  %lx  file:  %s  line:  %12d", ptr, m4fileName, m4lineNo  ) ;
            #endif
         }
      #endif

      mem4pushPointer( ptr ) ;
      return (void *)ptr ;
   #else
      return l4pop( &memoryType->pieces ) ;
   #endif
}

void *S4FUNCTION mem4alloc2Default( MEM4 *memoryType, CODE4 *c4 )
{
   void *ptr ;

   if ( code4numCodeBase() == 0 && resetInProgress == 0 )
      return 0 ;

   if ( c4 )
      if ( error4code( c4 ) < 0 )
         return 0 ;

   #ifdef S4SEMAPHORE
      mem4start( c4 ) ;
   #endif

   ptr = mem4allocLow( memoryType ) ;

   #ifdef S4SEMAPHORE
      #ifdef __WIN32
         mem4stop( c4 ) ;
      #else
         mem4stop( memoryType->codeBase ) ;
      #endif
   #endif

   if ( ptr == 0 )
   {
      if ( c4 )
         error4set( c4, e4memory ) ;
      return 0 ;
   }
   #ifdef E4MISC
      memset( ptr, 0, memoryType->unitSize - mem4extraTot ) ;
   #else
      memset( ptr, 0, memoryType->unitSize ) ;
   #endif
   return ptr ;
}

MEM4 *S4FUNCTION mem4createDefault( CODE4 *c4, int start, const unsigned int uSize, int expand, const int isTemp )
{
   MEM4 *onType ;
   unsigned int unitSize ;
   #ifdef S4SEMAPHORE
      #ifdef S4OS2
         #ifdef __DLL__
            ULONG flags ;
         #endif
      #endif
   #endif

   #ifdef E4PARM_HIGH
      /* c4 == 0 is allowable */
      if (  start < 1 || expand < 1 )
      {
         error4( c4, e4parm, E95906 ) ;
         return 0 ;
      }
   #endif

   if ( code4numCodeBase() == 0 && resetInProgress == 0 )
      return 0 ;

   if ( uSize < sizeof( LINK4 ) )   /* ensure enough so that we can keep track of the memory */
      unitSize = sizeof( LINK4 ) ;
   else
      unitSize = uSize ;

   #ifdef E4MISC
      unitSize += mem4extraTot ;
   #endif

   if ( c4 )
      if ( error4code( c4 ) < 0 )
         return 0 ;

   #ifdef S4SEMAPHORE
      mem4start( c4 ) ;
   #endif

   if ( !isTemp )
      for( onType = 0 ;; )
      {
         onType = (MEM4 *)l4next( &used, onType ) ;
         if ( onType == 0 )
            break ;

         #ifdef S4SEMAPHORE
            #ifdef S4OS2
               #ifdef __DLL__
                  /* get access to the memory */
                  flags = PAG_WRITE | PAG_READ  ;
                  if ( DosGetSharedMem( onType, flags ) != 0 )
                     return 0 ;
               #endif
            #endif
         #endif

         #ifdef S4DOS    /* no memory sharing under DOS, so can use a CODE4 for limited memory */
            if ( onType->codeBase == c4 || onType->codeBase == 0 )
         #endif
         if ( onType->unitSize == unitSize && onType->nRepeat > 0 )
         {
            /* Match */
            if ( start > onType->unitStart )
               onType->unitStart = start ;
            if ( expand > onType->unitExpand)
               onType->unitExpand = expand ;
            onType->nRepeat++ ;
            #ifdef S4SEMAPHORE
               mem4stop( c4 ) ;
            #endif
            #ifdef S4DOS
               if ( onType->codeBase == 0 )  /* set to avoid code4initUndo() errors */
                  onType->codeBase = c4 ;
            #endif
            return onType ;
         }
      }

   /* Allocate memory for another MEM4 */

   onType = (MEM4 *)l4last( &avail ) ;
   if ( onType == 0 )
   {
      MEMORY4GROUP *group ;
      int i ;

      #ifdef S4MEM_PRINT
         char *nullPtr = 0 ;

         if ( v4print != -1 )
         {
            #ifdef S4WIN_ALLOC
               #ifdef S4TESTING
                  {
                     d4displayStr( mem4displayPtr, "  MEM4:  ", 1 ) ;
                     d4displayPtr( mem4displayPtr, nullPtr, 0 ) ;
                     d4displayStr( mem4displayPtr, "  file:  ", 0 ) ;
                     d4displayStr( mem4displayPtr, "ignoreNextLine", 0 ) ;
                     d4displayStr( mem4displayPtr, "  line:  ", 0 ) ;
                     d4displayNum( mem4displayPtr, 0, 0 ) ;
                     d4displayStr( mem4displayPtr, "  num bytes:  ", 0 ) ;
                     d4displayNum( mem4displayPtr, 0, 0 ) ;
                  }
               #endif
            #else
               if ( v4print )
                  fprintf( stdprn, "\r\n  MEM4:  %lx  file:  ignoreNextLine  line:  %12d  num bytes:  %12d", nullPtr, 0, 0 ) ;
               else
                  printf("\r\n  MEM4:  %lx  file:  ignoreNextLine  line:  %12d  num bytes:  %12d", nullPtr, 0, 0 ) ;
            #endif
         }
      #endif

      group = (MEMORY4GROUP *)u4allocFreeDefault( c4, (long)sizeof( MEMORY4GROUP ) ) ;
      if ( group == 0 )
      {
         if ( c4 )
            error4set( c4, e4memory ) ;
         #ifdef S4SEMAPHORE
            mem4stop( c4 ) ;
         #endif
         return 0 ;
      }

      for ( i = 0 ; i < mem4numTypes ; i++ )
         l4add( &avail, group->types + i ) ;
      onType = (MEM4 *)l4last( &avail ) ;
      l4add( &groups, group ) ;
   }

   l4remove( &avail, onType ) ;
   memset( (void *)onType, 0, sizeof( MEM4 ) ) ;
   l4add( &used, onType ) ;

   onType->unitStart = start ;
   onType->unitSize = unitSize ;
   onType->unitExpand= expand ;
   onType->nRepeat = 1 ;
   onType->nUsed = 0 ;
   if ( isTemp )
      onType->nRepeat = -1 ;

   #ifdef S4SEMAPHORE
      mem4stop( c4 ) ;
   #endif

   #ifdef S4MEM_PRINT
      if ( v4print != -1 )
      {
         #ifdef S4WIN_ALLOC
            #ifdef S4TESTING
               {
                  d4displayStr( mem4displayPtr, "  MEM4:     ", 1 ) ;
                  d4displayPtr( mem4displayPtr, onType, 0 ) ;
                  d4displayStr( mem4displayPtr, "  file:  ", 0 ) ;
                  d4displayStr( mem4displayPtr, m4fileName, 0 ) ;
                  d4displayStr( mem4displayPtr, "  line:  ", 0 ) ;
                  d4displayNum( mem4displayPtr, m4lineNo, 0 ) ;
               }
            #endif
         #else
            if ( v4print )
               fprintf( stdprn, "\r\n  MEM4:     %lx  file:  %s  line:  %12d", onType, m4fileName, m4lineNo ) ;
            else
               printf( "\r\n  MEM4:     %lx  file:  %s  line:  %12d", onType, m4fileName, m4lineNo ) ;
         #endif
      }
   #endif

   #ifdef S4DOS    /* no memory sharing under DOS, so can use a CODE4 for limited memory */
      onType->codeBase = c4 ;
   #endif
   return onType ;
}

void *S4FUNCTION mem4createAllocDefault( CODE4 *c4, MEM4 **typePtrPtr, int start, const unsigned int unitSize, int expand, const int isTemp)
{
   if ( *typePtrPtr == 0 )
   {
      *typePtrPtr = mem4createDefault( c4, start, unitSize, expand, isTemp ) ;
      if ( *typePtrPtr == 0 )
         return 0 ;
   }

   return mem4alloc2Default( *typePtrPtr, c4 ) ;
}

int S4FUNCTION mem4freeDefault( MEM4 *memoryType, void *freePtr )
{
   #ifdef E4MISC
      int rc ;
      LINK4 *holdPtr ;
   #endif

   if ( freePtr == 0 )  /* as documented */
      return 0 ;

   if ( memoryType == 0 )
      return error4( 0, e4parmNull, E95907 ) ;

   if ( code4numCodeBase() == 0 && resetInProgress == 0 )
      return 0 ;

   #ifdef S4SEMAPHORE
      mem4start( 0 ) ;
   #endif

   memoryType->nUsed-- ;
   #ifdef E4MISC
      if ( memoryType->nUsed < 0 )
      {
         #ifdef S4SEMAPHORE
            mem4stop( 0 ) ;
         #endif
         return error4( 0, e4result, E95907 ) ;
      }

      rc = mem4popPointer( (char *)freePtr ) ;
      holdPtr = (LINK4 *)mem4checkPointer( (char *)freePtr, 0 ) ;
      if ( holdPtr != 0 )
      {
         l4add( &memoryType->pieces, holdPtr ) ;
         #ifdef S4MEM_PRINT
            if ( v4print != -1 )
            {
               #ifdef S4WIN_ALLOC
                  #ifdef S4TESTING
                     {
                        d4displayStr( mem4displayPtr, "  Y4FREE:   ", 1 ) ;
                        d4displayPtr( mem4displayPtr, freePtr, 0 ) ;
                        d4displayStr( mem4displayPtr, "  file:  ", 0 ) ;
                        d4displayStr( mem4displayPtr, m4fileName, 0 ) ;
                        d4displayStr( mem4displayPtr, "  line:  ", 0 ) ;
                        d4displayNum( mem4displayPtr, m4lineNo, 0 ) ;
                     }
                  #endif
               #else
                  if ( v4print )
                     fprintf(stdprn, "\r\n  Y4FREE:   %lx  file:  %s  line:  %12d", freePtr, m4fileName, m4lineNo ) ;
                  else
                     printf( "\r\n  Y4FREE:   %lx  file:  %s  line:  %12d", freePtr, m4fileName, m4lineNo ) ;
               #endif
            }
         #endif
      }

      memset( ((char *)freePtr), 0, memoryType->unitSize - 2 * mem4extraChars - sizeof( unsigned ) ) ;
      #ifdef S4SEMAPHORE
         mem4stop( 0 ) ;
      #endif
      return rc ;
   #else
      l4add( &memoryType->pieces, (LINK4 *)freePtr ) ;
      #ifdef S4SEMAPHORE
         mem4stop( 0 ) ;
      #endif
      return 0 ;
   #endif
}

void S4FUNCTION mem4release( MEM4 *memoryType )
{
   void *ptr ;

   if ( memoryType == 0 )
      return ;

   if ( code4numCodeBase() == 0 && resetInProgress == 0 )
      return ;

   #ifdef S4SEMAPHORE
      mem4start( 0 ) ;
   #endif

   memoryType->nRepeat-- ;
   if ( memoryType->nRepeat <= 0 )
   {
      for(;;)
      {
         ptr = l4pop( &memoryType->chunks) ;
         if ( ptr == 0 )
            break ;
         u4freeDefault( ptr ) ;
      }

      l4remove( &used, memoryType ) ;
      l4add( &avail, memoryType ) ;

      #ifdef S4MEM_PRINT
         if ( v4print != -1 )
         {
            #ifdef S4WIN_ALLOC
               #ifdef S4TESTING
                  {
                     d4displayStr( mem4displayPtr, "  Y4RELEASE:  ", 1 ) ;
                     d4displayPtr( mem4displayPtr, memoryType, 0 ) ;
                  }
               #endif
            #else
               if ( v4print )
                  fprintf( stdprn, "\r\n  Y4RELEASE:  %lx", memoryType ) ;
               else
                  printf( "\r\n  Y4RELEASE:  %lx", memoryType ) ;
            #endif
         }
      #endif
   }

   #ifdef S4SEMAPHORE
      mem4stop( 0 ) ;
   #endif
}

#ifdef S4TESTING
   int s4allocOff = 0 ;
#endif

#ifdef S4MAX
   long mem4allocated = 0L ;

   long u4allocated( void )
   {
      return mem4allocated ;
   }
#endif

#ifdef S4MAX
   static long mem4maxMemory = 16384 ;

   long u4max( void )
   {
      return mem4maxMemory ;
   }
#endif

void *S4FUNCTION u4allocDefault( long n )
{
   size_t s ;
   char *ptr ;
   #ifdef S4SEMAPHORE
      #ifdef S4OS2
         #ifdef __DLL__
            ULONG    flags;
            APIRET   rc;
         #endif
      #endif
   #endif
   #ifdef S4WIN_ALLOC
      HANDLE  handle, *hPtr ;
   #endif

   #ifdef S4TESTING
      if ( s4allocOff == 1 )   /* for testing, emulate out of memory conditions */
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( n == 0L )
      {
         error4( 0, e4parm, E85903 ) ;
         return 0 ;
      }
   #endif

   #ifdef E4MISC
      n += mem4extraChars * 2 + sizeof( unsigned ) ;
   #endif

   #ifdef S4MAX
      n += sizeof( S4LONG ) ;   /* room for length */
      if ( mem4allocated + n > mem4maxMemory )
         return 0 ;
   #endif

   s = (size_t) n ;
   if ( n > (long) s )
      return 0 ;

   #ifdef S4WIN_ALLOC
      #ifdef S4WINCE
         handle = LocalAlloc( LMEM_FIXED|LMEM_ZEROINIT, (UINT)s+sizeof(HANDLE) ) ;  /* modified 4/22/98 L.Y. */
      #else
         #ifdef __DLL__
            handle = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, (DWORD) s+ sizeof(HANDLE) ) ;
         #else
            handle = GlobalAlloc( GMEM_MOVEABLE | GMEM_ZEROINIT, (DWORD) s+ sizeof(HANDLE) ) ;
         #endif
      #endif

      if ( handle == (HANDLE)0 )
      {
         #ifdef E4DEBUG
            #ifdef __WIN32
               /* for debugger purposes */
               DWORD err = GetLastError() ;
            #endif
         #endif
         return 0 ;
      }

      #ifndef S4WINCE
         hPtr = (HANDLE *)GlobalLock( handle ) ;
      #else
         hPtr = (HANDLE *)LocalLock( handle) ;
      #endif
      *hPtr++ = handle ;
      ptr = (char *)hPtr ;
   #else
      #ifdef __DLL__
         #ifdef S4OS2_SEMAPHORE
            flags = PAG_WRITE | PAG_READ | OBJ_GETTABLE ;
            rc = DosAllocSharedMem( (void *)&ptr, 0, s, flags ) ;

            if (rc != 0)
               return 0 ;
            flags = PAG_WRITE | PAG_READ ;
            rc = DosGetSharedMem( ptr, flags ) ;
            if ( rc != 0 )
               return 0 ;
         #else
            ptr = (char *)malloc( s ) ;

            if ( ptr == 0 )
               return 0 ;

            #ifndef S4PASCAL_WIN
               memset( ptr, 0, s ) ;
            #endif
         #endif
      #else
         ptr = (char *)malloc( s ) ;

         if ( ptr == 0 )
            return 0 ;

                              /* Borland malloc of 64K (or close to) will */
         #ifdef __TURBOC__    /* result in corruption of segment memory due */
            if ( (ptr+s-1 <= ptr) && (s > 1 ))    /* to wrap-around problems  */
            {
               free( ptr ) ;
               return 0 ;
            }
         #endif

         #ifndef S4PASCAL_WIN
            memset( ptr, 0, s ) ;
         #endif
      #endif
   #endif

   #ifdef S4MEM_PRINT
      if ( v4print != -1 )
      {
         #ifdef S4WIN_ALLOC
            #ifdef S4TESTING
               {
                  d4displayStr( mem4displayPtr, "  U4ALLOC:  ", 1 ) ;
                  d4displayPtr( mem4displayPtr, ptr, 0 ) ;
                  d4displayStr( mem4displayPtr, "  file:  ", 0 ) ;
                  d4displayStr( mem4displayPtr, m4fileName, 0 ) ;
                  d4displayStr( mem4displayPtr, "  line:  ", 0 ) ;
                  d4displayNum( mem4displayPtr, m4lineNo, 0 ) ;
                  d4displayStr( mem4displayPtr, "  num bytes:  ", 0 ) ;
                  d4displayNum( mem4displayPtr, n, 0 ) ;
               }
            #endif
         #else
            if ( v4print )
               fprintf( stdprn, "\r\n  U4ALLOC:  %lx  file:  %s  line:  %12d  num bytes:  %12ld", ptr, m4fileName, m4lineNo, n ) ;
            else
               printf("\r\n  U4ALLOC:  %lx  file:  %s  line:  %12d  num bytes:  %12ld", ptr, m4fileName, m4lineNo, n ) ;
         #endif
      }
   #endif

   #ifdef E4MISC
      #ifdef S4SEMAPHORE
         mem4start( 0 ) ;
      #endif
      ptr = mem4fixPointer( ptr, s ) ;
      mem4pushPointer( ptr ) ;
      memset( ptr, 0, s-mem4extraChars*2 - sizeof(unsigned) ) ;
      #ifdef S4SEMAPHORE
         mem4stop( 0 ) ;
      #endif
   #endif

   #ifdef S4MAX
      #ifdef S4SEMAPHORE
         mem4start( 0 ) ;
      #endif
      mem4allocated += n ;
      #ifdef S4SEMAPHORE
         mem4stop( 0 ) ;
      #endif
      *((long *)ptr) = n ;
      ptr += sizeof(S4LONG ) ;
   #endif

   return (void *)ptr ;
}

void S4FUNCTION u4freeFixedDefault( FIXED4MEM fixedMem )
{
   #ifdef S4USE_LOW_MEMORY
      if ( ( GetWinFlags() & WF_ENHANCED ) == WF_ENHANCED )
         GlobalPageUnlock( (HGLOBAL)s4protected( fixedMem ) ) ;
      GlobalDosFree( (HGLOBAL)s4protected( fixedMem ) ) ;
   #else
      u4freeDefault( fixedMem ) ;
   #endif
}

/* u4alloc_fixed() allocates fixed memory blocks in the lower 1 MEG of ram.
   It is used for allocating memory that is to be used with DOS interrupts.
*/
FIXED4MEM S4FUNCTION u4allocFixedDefault( CODE4 *c4, long n )
{
   #ifdef S4USE_LOW_MEMORY
      size_t s ;
      FIXED4MEM fixedMem ;

      memset( &fixedMem, 0, sizeof( FIXED4MEM ) ) ;

      #ifdef E4PARM_LOW
         if ( c4 == 0 )
         {
            error4( 0, e4parmNull, E95908 ) ;
            return fixedMem ;
         }

         if ( n == 0L )
         {
            error4( c4, e4parm, E85903 ) ;
            return fixedMem ;
         }
         n += mem4extraChars*2 + sizeof(unsigned) ;
      #endif

      s = (size_t)n ;
      if ( n > (long)s )
         return fixedMem ;

      #ifdef __DLL__
         fixedMem.block = GlobalDosAlloc( (DWORD)s + sizeof(HANDLE) ) ;
      #else
         fixedMem.block = GlobalDosAlloc( (DWORD)s + sizeof(HANDLE) ) ;
      #endif

      if ( fixedMem.block == 0 )
         return fixedMem ;

      if ( ( GetWinFlags() & WF_ENHANCED ) == WF_ENHANCED )
         GlobalPageLock( (HGLOBAL)s4protected( fixedMem ) ) ;

      memset( (void *)s4protected( fixedMem ), 0, (size_t)n ) ;
      return fixedMem ;
   #else
      return u4allocFreeDefault( c4, n ) ;
   #endif
}

void *S4FUNCTION u4allocErDefault( CODE4 *c4, long n )
{
   void *ptr = (void *)u4allocFreeDefault( c4, n ) ;
   if ( ptr == 0 && c4 )
      error4( c4, e4memory, E95909 ) ;

   return ptr ;
}

int S4FUNCTION u4freeDefault( void *ptr )
{
   int rc ;
   #ifdef S4WIN_ALLOC
      HANDLE  hand ;
   #endif

   #ifdef S4MAX
      long amount ;
   #endif

   rc = 0 ;

   /* if we need to access the semaphores, better ensure that the memory
      area is initialized; otherwise allow general freeing.  Change for
      CodeControls, means occasional memory leaks, but not common.
      AS 02/10/96.
   */
   #ifdef S4MAX
      if ( code4numCodeBase() == 0 && resetInProgress == 0 )
         return 0 ;
   #else
      #ifdef E4MISC
         if ( code4numCodeBase() == 0 && resetInProgress == 0 )
            return 0 ;
      #endif
   #endif

   if ( ptr == 0 )
      return 0 ;

   #ifdef S4MAX
      ptr = ((char *)ptr) - sizeof(S4LONG) ;
      amount = *((long *)ptr) ;
      #ifdef S4SEMAPHORE
         mem4start( 0 ) ;
      #endif
      mem4allocated -= amount ;
      #ifdef S4SEMAPHORE
         mem4stop( 0 ) ;
      #endif
   #endif

   #ifdef S4WIN_ALLOC
      #ifdef E4MISC
         #ifdef S4SEMAPHORE
            mem4start( 0 ) ;
         #endif
         rc = mem4popPointer( (char *)ptr ) ;
         ptr = mem4checkPointer( (char *)ptr, 1 ) ;
         #ifdef S4SEMAPHORE
            mem4stop( 0 ) ;
         #endif
         if ( (char *)ptr == 0 )  /* error during mem check */
            return -1 ;
         hand = ((HANDLE *)(char *)ptr)[-1] ;
      #else
         hand = ((HANDLE *)ptr)[-1] ;
      #endif

      #ifdef S4MEM_PRINT
         if ( v4print != -1 )
         {
            #ifdef S4WIN_ALLOC
               #ifdef S4TESTING
                  d4displayStr( mem4displayPtr, "  U4FREE:   ", 1 ) ;
                  d4displayPtr( mem4displayPtr, ptr, 0 ) ;
                  d4displayStr( mem4displayPtr, "  file:  ", 0 ) ;
                  d4displayStr( mem4displayPtr, m4fileName, 0 ) ;
                  d4displayStr( mem4displayPtr, "  line:  ", 0 ) ;
                  d4displayNum( mem4displayPtr, m4lineNo, 0 ) ;
               #endif
            #endif
         }
      #endif
      #ifndef S4WINCE
         GlobalUnlock( hand ) ;
         hand = GlobalFree( hand ) ;
      #else
         LocalUnlock( hand ) ;
         hand = LocalFree( hand ) ;
      #endif
      if ( hand != (HANDLE) 0 )
         return error4( 0, e4memory, E95910 ) ;
   #else
      #ifdef E4MISC
         #ifdef S4SEMAPHORE
            mem4start( 0 ) ;
         #endif
         rc = mem4popPointer( (char *)ptr ) ;
         ptr = mem4checkPointer( (char *)ptr, 1 ) ;
         #ifdef S4SEMAPHORE
            mem4stop( 0 ) ;
         #endif

         #ifdef S4MEM_PRINT
            if ( v4print != -1 )
            {
               #ifdef S4WIN_ALLOC
                  #ifdef S4TESTING
                     {
                        d4displayStr( mem4displayPtr, "  U4FREE:  ", 1 ) ;
                        d4displayPtr( mem4displayPtr, ptr, 0 ) ;
                        d4displayStr( mem4displayPtr, "  file:  ", 0 ) ;
                        d4displayStr( mem4displayPtr, m4fileName, 0 ) ;
                        d4displayStr( mem4displayPtr, "  line:  ", 0 ) ;
                        d4displayNum( mem4displayPtr, m4lineNo, 0 ) ;
                     }
                  #endif
               #else
                  if ( v4print )
                     fprintf( stdprn, "\r\n  U4FREE:  %lx  file:  %s  line:  %12d", ptr, m4fileName, m4lineNo  ) ;
                  else
                     printf( "\r\n U4FREE:  %lx  file:  %s  line:  %12d", ptr, m4fileName, m4lineNo  );
               #endif
            }
         #endif

         free( ptr ) ;
      #else
         #ifdef S4MEM_PRINT
            if ( v4print != -1 )
            {
               #ifdef S4WIN_ALLOC
                  #ifdef S4TESTING
                     {
                        d4displayStr( mem4displayPtr, "  U4FREE:  ", 1 ) ;
                        d4displayPtr( mem4displayPtr, ptr, 0 ) ;
                        d4displayStr( mem4displayPtr, "  file:  ", 0 ) ;
                        d4displayStr( mem4displayPtr, m4fileName, 0 ) ;
                        d4displayStr( mem4displayPtr, "  line:  ", 0 ) ;
                        d4displayNum( mem4displayPtr, m4lineNo, 0 ) ;
                     }
                  #endif
               #else
                  if ( v4print )
                     fprintf( stdprn, "\r\n  U4FREE:  %lx  file:  %s  line:  %12d", ptr, m4fileName, m4lineNo  ) ;
                  else
                     printf( "\r\n  U4FREE:  %lx  file:  %s  line:  %12d", ptr, m4fileName, m4lineNo  );
               #endif
            }
         #endif

         free( (char *)ptr ) ;
      #endif
   #endif

   return rc ;
}


int mem4reset( void )
{
   MEM4 *onType ;
   LINK4 *onChunk, *onGroup ;
   #ifdef S4WIN_ALLOC
      #ifdef E4MISC
         HANDLE hand ;
      #endif
   #endif
   #ifdef S4LOCK_CHECK
      return error4( 0, e4result, E85904 ) ;
   #endif

   resetInProgress = 1 ;   /* suspend the disallowment of u4free when no code4's */

   for( onType = 0 ;; )
   {
      onType = (MEM4 *)l4next(&used,onType) ;
      if ( onType == 0 )
         break ;
      do
      {
         onChunk = (LINK4 *)l4pop( &onType->chunks) ;
         u4freeDefault( onChunk ) ;  /* free of 0 still succeeds */
      } while ( onChunk ) ;
   }

/*
   for( onType = 0 ;; )
   {
      onType = (MEM4 *)l4next(&avail,onType) ;
      if ( onType == 0 )
         break ;
      do
      {
         onChunk = (LINK4 *)l4pop( &onType->chunks) ;
         u4freeDefault( onChunk ) ;   free of 0 still succeeds f
      } while ( onChunk ) ;
   }
*/

   for( ;; )
   {
      onGroup = (LINK4 *)l4pop( &groups ) ;
      if ( onGroup == 0 )
         break ;
      u4freeDefault( onGroup ) ;
   }

   #ifdef E4MISC
      if ( mem4numPointer > 0 )
      {
         #ifdef S4WIN_ALLOC
            hand = ((HANDLE *)mem4testPointers)[-1] ;

            GlobalUnlock( hand ) ;
            hand = GlobalFree( hand ) ;

            if ( hand != (HANDLE)0 )
               return error4( 0, e4memory, E95911 ) ;
         #else
            #ifdef S4MEM_PRINT
               if ( v4print != -1 )
               {
                  #ifdef S4WIN_ALLOC
                     #ifdef S4TESTING
                        {
                           d4displayStr( mem4displayPtr, "  MEM4RESET:  ", 1 ) ;
                           d4displayPtr( mem4displayPtr, mem4testPointers, 0 ) ;
                     #endif
                  #else
                     if ( v4print )
                        fprintf( stdprn, "\r\n  MEM4RESET:  %lx", mem4testPointers ) ;
                     else
                        printf( "\r\n  MEM4RESET:  %lx", mem4testPointers );
                  #endif
               }
            #endif

            free( (void *)mem4testPointers ) ;
         #endif

         mem4testPointers = 0 ;
         mem4numPointer = -1 ;
         mem4numUsed = 0 ;
      }
   #endif

   mem4init() ;

   #ifdef S4SEMAPHORE
      #ifdef __WIN32
         DeleteCriticalSection( &critical4memory ) ;
         DeleteCriticalSection( &critical4expression ) ;
      #endif
   #endif

   #ifdef S4SEMAPHORE
      #ifdef __WIN32
         memoryInitialized = 0 ;
      #endif
   #endif

   resetInProgress = 0 ;   /* suspend the disallowment of u4free when no code4's */

   return 0 ;
}

void mem4init( void )
{
   if ( code4numCodeBase() != 0 )
      return ;

   #ifdef S4SEMAPHORE
      #ifdef __WIN32
         memoryInitialized = 1 ;
      #endif
   #endif

   #ifdef S4SEMAPHORE
      #ifdef __WIN32
         if ( resetInProgress == 0 )
         {
            InitializeCriticalSection( &critical4memory ) ;
            InitializeCriticalSection( &critical4expression ) ;
         }
      #endif
      mem4start( 0 ) ;
   #endif
   memset( (void *)&avail, 0, sizeof( avail ) ) ;
   memset( (void *)&used, 0, sizeof( used ) ) ;
   memset( (void *)&groups, 0, sizeof( groups ) ) ;
   #ifdef S4SEMAPHORE
      mem4stop( 0 ) ;
   #endif
}

#ifdef S4WIN_ALLOC
   #undef S4WIN_ALLOC
#endif
