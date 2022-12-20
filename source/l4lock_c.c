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

/* l4lock_c.c (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifdef S4LOCK_CHECK

/* Saves information about all current locks  */
typedef struct
{
   LINK4  link ;
   int     next ;
   int     prev ;
   int     hand ;
   long    startPos ;
   long    len ;
   long    endPos ;
} L4LOCK_INFO ;

static LIST4 lockList ;
static int first =  1 ;
static MEM4 *lockAlloc ;

int S4FUNCTION l4lock_check()
{
   return lockList.nLink ;
}

/* Returns true or false; true means found */
static L4LOCK_INFO *l4lockFind( int h, long p, long l )
{
   L4LOCK_INFO *lPtr ;

   if ( first )
   {
      first =  0 ;
      memset( &lockList, 0, sizeof(lockList) ) ;
      lockAlloc =  mem4create( 0, 20, sizeof(L4LOCK_INFO), 20, 0 ) ;
   }

   for ( lPtr = 0; lPtr = (L4LOCK_INFO *) l4next(&lockList, lPtr); )
   {
      if ( lPtr->hand == h  && lPtr->startPos == p && lPtr->len == l )
         return lPtr ;
      if ( lPtr->hand == h )
      {
         /* Check for Overlap. */
         if ( lPtr->startPos >= p && lPtr->startPos <= p+l-1  ||
              lPtr->endPos >= p   && lPtr->endPos   <= p+l-1  ||
              p >= lPtr->startPos && p <= lPtr->endPos        ||
              p+l-1 >= lPtr->startPos && p+l-1 <= lPtr->endPos )
              {
                 error4( 0, e4result, E86101 ) ;
                 return 0 ; ;
              }
      }
   }
   return 0 ;
}

void S4FUNCTION l4lockRemove( int h, long p, long l )
{
   L4LOCK_INFO *lPtr ;

   lPtr =  l4lockFind( h,p,l ) ;
   if ( lPtr == 0 )
      error4( 0, e4result, E86102 ) ;
   else
      l4remove( &lockList, lPtr ) ;
}

void S4FUNCTION l4lockSave( int h, long p, long l )
{
   L4LOCK_INFO *lPtr ;

   if ( l4lockFind(h,p,l) != 0 )
   {
      error4( 0, e4result, E86101 ) ;
      return ;
   }

   lPtr =  (L4LOCK_INFO *) mem4alloc( lockAlloc ) ;
   if ( lPtr == 0 )
   {
      error4( 0, e4memory, E96102 ) ;
      return ;
   }
   l4add( &lockList, lPtr ) ;

   lPtr->hand = h ;
   lPtr->startPos = p ;
   lPtr->len  = l ;
   lPtr->endPos =  p+l-1 ;
}

#endif
