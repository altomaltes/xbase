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

/* i4info.c   (c)Copyright Sequiter Software Inc., 1988-2001.  All rights reserved. */

#include "d4all.h"

unsigned short S4FUNCTION tfile4isDescending( TAG4FILE *tag )
{
   #if  defined( S4OFF_INDEX )
      error4( tag->codeBase, e4notSupported, 0 ) ;
      return 0 ;
   #else
      #if defined( S4FOX )
         if ( tag->header.descending )
            return r4descending ;
         else
            return 0 ;
      #elif defined( S4CLIPPER )
         if ( tag->header.descending )
            return r4descending ;
         else
            return 0 ;
      #elif defined( S4MDX )
         if ( tag->header.typeCode & 8 )
            return r4descending ;
         else
            return 0 ;
      #endif
   #endif
}


// AS Jan 30/07 - exported for off-index...
int S4FUNCTION tfile4keyLenExport( TAG4FILE *tag )
{
   assert5( tag != 0 ) ;
   #ifdef S4OFF_INDEX
      return e4notSupported ;
   #else
         return tfile4keyLen( tag ) ;
   #endif
}



#ifndef S4OFF_INDEX
   char *S4FUNCTION t4alias( TAG4 *t4 )
   {
      #ifdef E4VBASIC
         #ifdef S4CB51
            if ( c4parm_check( t4, 4, E40146 ) ) return 0 ;
         #else
            if ( c4parm_check( t4, 4, E91640 ) ) return 0 ;
         #endif
      #endif

      #ifdef E4PARM_HIGH
         if ( t4 == 0 )
         {
            error4( 0, e4parm_null, E91640 ) ;
            return 0 ;
         }
         if ( t4->tagFile == 0 )
         {
            error4( 0, e4parm, E91640 ) ;
            return 0 ;
         }
      #endif

      return t4->tagFile->alias ;
   }

   short S4FUNCTION t4descending( TAG4 *tag )
   {
         return tfile4isDescending( tag->tagFile ) ;
   }



      TAG4INFO *S4FUNCTION i4tagInfo( INDEX4 *i4 )
      {
         #ifdef E4PARM_HIGH
            if ( i4 == 0 )
            {
               error4( 0, e4parm_null, E95501 ) ;
               return 0 ;
            }
         #endif

         if ( error4code( i4->codeBase ) < 0 )
            return 0 ;

         int numTags = 0 ;
         TAG4 *tagOn ;
         for( tagOn = 0 ;; )
         {
            tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
            if ( tagOn == 0 )
               break ;
            numTags++ ;
         }

         if ( error4code( i4->codeBase ) < 0 )
            return 0 ;
         TAG4INFO *tagInfo = (TAG4INFO *)u4allocFree( i4->codeBase, ( (long)numTags + 1L ) * sizeof( TAG4INFO ) ) ;
         if ( tagInfo == 0 )
            return 0 ;

         int i ;

         for( tagOn = 0, i = 0 ;; i++ )
         {
            tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
            if ( tagOn == 0 )
               return ( tagInfo ) ;
            tagInfo[i].name = tfile4alias( tagOn->tagFile ) ;
            tagInfo[i].expression = expr4source( tagOn->tagFile->expr ) ;
            tagInfo[i].filter = expr4source( tagOn->tagFile->filter ) ;
            tagInfo[i].unique = t4unique( tagOn ) ;
            tagInfo[i].descending = tfile4isDescending( tagOn->tagFile ) ;
         }
      }
#endif  /* S4OFF_INDEX */
