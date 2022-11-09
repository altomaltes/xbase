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

/* s4init.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

/* no spools */
int S4FUNCTION sort4getMemInit( SORT4 *s4 )
{
   E4PARMLOW( s4, E91901 ) ;

   s4quick( (void **)s4->pointers, s4->pointersUsed, s4->cmp, s4->sortLen) ;
   #ifdef  E4ANALYZE
      if ( s4->seqwriteBuffer == 0 )
         return error4( s4->codeBase, e4info, E91901 ) ;
   #endif
   u4free( s4->seqwriteBuffer ) ;
   s4->seqwriteBuffer = 0 ;

   return 0 ;
}

int S4FUNCTION sort4getInit( SORT4 *s4 )
{
   int rc ;

   #ifdef E4PARM_HIGH
      if ( s4 == 0 )
         return error4( 0, e4parmNull, E91902 ) ;
   #endif

   if ( error4code( s4->codeBase ) < 0 )
      return e4codeBase ;

   if ( s4->spoolsMax > 0 )
   {
      rc = sort4spoolsInit( s4, 0 ) ;
      if ( rc == e4memory )
      {
         sort4free( s4 ) ;
         return error4( s4->codeBase, e4memory, E91902 ) ;
      }
   }
   else
   {
      sort4getMemInit( s4 ) ;
      return 0 ;
   }

   return rc ;
}

int S4FUNCTION sort4spoolsInit( SORT4 *s4, const int prevCall )
{
   unsigned int entriesPerSpool, spoolsPerPool, entriesUsed ;
   void *ptr, *memFree ;
   int rc ;
   char *poolEntry, *poolEntryIterate ;
   S4LONG spoolDiskI ;

   E4PARM_TEST( s4 == 0 || prevCall < 0 || prevCall > 1, E91903 ) ;

   poolEntry = (char *) 0 ;
   if ( !prevCall )
   {
      rc = s4flush( s4 ) ;
      if ( rc < 0 )
         return error4stack( s4->codeBase, (short)rc, E91903 ) ;
      rc = file4seqWriteFlush(&s4->seqwrite ) ;
      if ( rc < 0 )
         return error4stack( s4->codeBase, (short)rc, E91903 ) ;
      #ifdef E4ANALYZE
         if (s4->seqwriteBuffer == 0 )
            return error4( s4->codeBase, e4info, E91903 ) ;
      #endif
      u4free( s4->seqwriteBuffer ) ;
      s4->seqwriteBuffer = 0 ;

      u4free( s4->pointers ) ;
      s4->pointers = 0 ;
   }

   for (;;)
   {
      s4->spoolPointer = (S4SPOOL *)u4alloc( (long)sizeof(S4SPOOL) * s4->spoolsMax ) ;
      if ( s4->spoolPointer )
         break ;

      if ( l4last( &s4->pool ) == 0 )
         return e4memory ;

      memFree = l4pop( &s4->pool ) ;
      mem4free( s4->poolMemory, memFree ) ;
      s4->poolN-- ;
   }

   for(;;)
   {
      /* Split up the pools between the spools */
      if ( s4->poolN == 0 )
         spoolsPerPool = s4->spoolsMax ;
      else
         spoolsPerPool = (s4->spoolsMax+s4->poolN-1) / s4->poolN ;

      entriesPerSpool = s4->poolEntries / spoolsPerPool ;

      if ( entriesPerSpool == 0 )
         return e4memory ;

      if ( s4->poolN != 0 )
         break ;

      ptr = mem4alloc( s4->poolMemory ) ;
      if ( ptr != 0 )
      {
         l4add( &s4->pool, ptr ) ;
         s4->poolN++ ;
      }
      else
      {
         s4->poolEntries /= 2 ;
         for ( ;; )
         {
            ptr = l4pop(&s4->pool) ;
            if ( ptr == 0 )
               break ;
            mem4free( s4->poolMemory, ptr ) ;
         }
         mem4release( s4->poolMemory ) ;
         s4->poolMemory = mem4create( s4->codeBase, 1, (unsigned) s4->poolEntries*s4->totLen+sizeof(LINK4), 1, 1 ) ;
      }
   }

   s4->spoolMemLen = entriesPerSpool * s4->totLen ;
   s4->spoolDiskLen = (S4LONG)s4->pointersInit * s4->totLen ;

   entriesUsed = s4->poolEntries+1 ;  /* Entries used in current pool. */
   poolEntryIterate = 0 ;

   for ( spoolDiskI = 0L; s4->spoolsN < s4->spoolsMax; )
   {
      c4memmove( s4->spoolPointer+1, s4->spoolPointer, sizeof(S4SPOOL)*s4->spoolsN ) ;
      if ( entriesUsed + entriesPerSpool > s4->poolEntries )
      {
         entriesUsed = 0 ;
         poolEntryIterate= (char *) l4next( &s4->pool, poolEntryIterate);
         poolEntry = poolEntryIterate + sizeof(LINK4) ;
      }
      s4->spoolPointer->ptr = poolEntry ;
      poolEntry   += (s4->totLen*entriesPerSpool) ;
      entriesUsed += entriesPerSpool ;

      s4->spoolPointer->spoolI = s4->spoolsN++ ;
      s4->spoolPointer->disk = spoolDiskI ;
      spoolDiskI += s4->spoolDiskLen ;

      s4->spoolPointer->len = 0 ;
         if ( s4->spoolsN < s4->spoolsMax )
            s4nextSpoolEntry(s4) ;
   }
   return 0 ;
}

int S4FUNCTION sort4init( SORT4 *s4, CODE4 *c4, const int sortL, const int infoL )
{
   #ifdef E4PARM_HIGH
      if ( s4 == 0 || c4 == 0 )
         return error4( c4, e4parmNull, E91904 ) ;
   #endif

   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   sort4initSet( s4, c4, sortL, infoL ) ;
   if ( sort4initAlloc( s4 ) == e4memory )
   {
      sort4free( s4 ) ;
      return error4( c4, e4memory, E91904 ) ;
   }
   return 0 ;
}

int S4FUNCTION sort4initSet( SORT4 *s4, CODE4 *c4, const int sortL, const int infoL )
{
   #ifdef E4PARM_LOW
      if ( s4 == 0 || c4 == 0 || sortL < 0 || infoL < 0 )
         return error4( 0, e4parm, E91905 ) ;
   #endif

   memset( (void *)s4, 0, sizeof(SORT4) ) ;
   s4->file.hand = INVALID4HANDLE ;

   s4->codeBase = c4 ;
   s4->cmp = (S4CMP_FUNCTION *)u4memcmp ;

   s4->sortLen = (unsigned int)sortL ;
   s4->infoLen = (unsigned int)infoL ;
   s4->infoOffset = s4->sortLen + sizeof(S4LONG) ;
   s4->totLen = s4->infoOffset + s4->infoLen ;
   s4->poolEntries = ( c4->memSizeSortPool - sizeof( LINK4 ) ) / s4->totLen ;
   s4->pointersMax = c4->memSizeSortPool / sizeof( char * ) ;
   s4->isMemAvail = 1 ;
   return 0 ;
}

int S4FUNCTION sort4initAlloc( SORT4 *s4 )
{
   FILE4LONG start ;

   E4PARMLOW( s4, E91906 ) ;

   if ( s4->seqwriteBuffer == 0 )
   {
      s4->seqwriteBuffer = (char *)u4alloc( (long)s4->codeBase->memSizeSortBuffer ) ;
      if ( s4->seqwriteBuffer == 0 )
         return e4memory ;
      file4longAssign( start, 0, 0 ) ;
      file4seqWriteInitLow( &s4->seqwrite, &s4->file, start, s4->seqwriteBuffer, s4->codeBase->memSizeSortBuffer ) ;
   }

   if ( s4->pointers == 0 )
      for(;;)
      {
         s4->pointers = (char **)u4alloc( (long)s4->pointersMax * sizeof(char *) ) ;
         if ( s4->pointers != 0 )
            break ;

         s4->pointersMax /= 2 ;
         if ( s4->pointersMax < 256 )
            return e4memory ;
      }

   #ifdef E4ANALYZE
      if ( s4->poolMemory )
         return error4( s4->codeBase, e4info, E81901 ) ;
   #endif

   s4->poolMemory = mem4create( s4->codeBase, 1, s4->codeBase->memSizeSortPool,1,1);
   if ( s4->poolMemory == 0 )
         return e4memory ;

   return 0 ;
}

void sort4initPointers( SORT4 *s4, char *availMem, unsigned int len )
{
   /* Assign 'pointers' */
   unsigned int n, i ;

   E4PARM_TRET( s4 == 0 || availMem == 0, E91907, NULL ) ;

   n = len / s4->totLen ;
   i = s4->pointersInit ;

   s4->pointersInit += n ;
   if ( s4->pointersInit > s4->pointersMax )
      s4->pointersInit = s4->pointersMax ;

   for ( ; i < s4->pointersInit ; i++, availMem += s4->totLen )
      s4->pointers[i] = availMem ;
}
