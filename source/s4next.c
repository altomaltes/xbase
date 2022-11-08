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

/* s4next.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"


int s4nextSpoolEntry( SORT4 *s4 )
{
   S4LONG lastDiskPos, diskDataLeft ;
   S4LONG newRec, spoolRec ;
   unsigned lenRead, maxRead ;
   FILE4LONG filePos ;
   int low, high, pos, rc ;
   char *newData ;
   S4SPOOL saveSpool ;

   E4PARMLOW( s4, E91908 ) ;

   s4->spoolPointer->pos += s4->totLen ;
   if ( s4->spoolPointer->pos >= s4->spoolPointer->len )
   {
      s4->spoolPointer->pos = 0 ;
      if ( s4->spoolPointer->disk >= 0 )
      {
         lastDiskPos = (s4->spoolPointer->spoolI+1) * s4->spoolDiskLen  ;
         diskDataLeft = lastDiskPos - s4->spoolPointer->disk ;
         maxRead = s4->spoolMemLen ;
         if ( (S4LONG) s4->spoolMemLen > diskDataLeft )
            maxRead = diskDataLeft ;
         file4longAssign( filePos, s4->spoolPointer->disk, 0 ) ;
         lenRead = file4readInternal( &s4->file, filePos, s4->spoolPointer->ptr, maxRead ) ;

         if ( error4code( s4->codeBase ) < 0 )
         {
            sort4free( s4 ) ;
            return error4stack( s4->codeBase, (short)error4code( s4->codeBase ), E91908 ) ;
         }

         s4->spoolPointer->len = lenRead ;
         s4->spoolPointer->disk += lenRead ;
         if ( lenRead != maxRead || lenRead == 0 )
         {
            if ( lenRead % s4->totLen )
            {
               sort4free( s4 ) ;
               return error4describe( s4->codeBase, e4read, E91908, s4->file.name, 0, 0 ) ;
            }
            s4->spoolPointer->disk = -1 ;
            if ( lenRead == 0 )
            {
               s4deleteSpoolEntry( s4 ) ;
               return 0 ;
            }
            else
               s4->spoolPointer->len = lenRead ;
         }
         else  /* Check if we are out of disk entries for the spool */
            if ( s4->spoolPointer->disk >= lastDiskPos )
               s4->spoolPointer->disk = -1L ;
      }
      else
      {
         s4deleteSpoolEntry(s4) ;
         return 0 ;
      }
   }

   /* Position the new entry to the sorted location using a binary search */
   /* New entry is placed before 'result':  int pos >= 1  when complete */
   low = 1 ;
   high = s4->spoolsN ;
   newData = s4->spoolPointer->ptr + s4->spoolPointer->pos ;
   memcpy( (void *)&newRec, newData + s4->sortLen, sizeof(newRec) ) ;

   for(;;)
   {
      pos = ( low + high ) / 2 ;
      if ( pos == low && pos == high )  /* then found */
      {
         memcpy( (void *)&saveSpool, (void *)s4->spoolPointer, sizeof(S4SPOOL) ) ;
         c4memmove( s4->spoolPointer, s4->spoolPointer+1, sizeof(S4SPOOL)*(pos-1) ) ;
         memcpy( (void *)(s4->spoolPointer+pos-1), (void *)&saveSpool, sizeof(S4SPOOL) ) ;
         return 0 ;
      }
      rc = (*s4->cmp)(newData, s4->spoolPointer[pos].ptr + s4->spoolPointer[pos].pos, s4->sortLen) ;
      if ( rc == 0 )
      {
         memcpy( (void *)&spoolRec, s4->spoolPointer[pos].ptr + s4->spoolPointer[pos].pos + s4->sortLen, sizeof(spoolRec) ) ;
         if ( newRec > spoolRec )
            rc = 1 ;
      }

      if ( rc > 0 )
         low = pos+1 ;
      else
         high = pos ;
      #ifdef E4ANALYZE
         if ( high < low )
            return error4( s4->codeBase, e4result, E91908 ) ;
      #endif
   }
}

