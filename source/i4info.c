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
   #if defined( S4CLIENT ) || defined( S4OFF_INDEX )
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
      #ifdef S4CLIENT
         if ( tag->keyLen == 0 )
         {
            TAG4 *t4 = d4tag( tag->refData, tag->alias ) ;
            if ( t4 == 0 )
               return -1 ;
            TAG4INFO *temp = i4tagInfo( t4->index ) ;
            if ( temp == 0 )
               return -1 ;
            u4free( temp ) ;
            temp = 0 ;
         }

         assert5( tag->keyLen != 0 ) ;
         return tag->keyLen ;
      #else
         return tfile4keyLen( tag ) ;
      #endif
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
      #ifndef S4CLIENT
         return tfile4isDescending( tag->tagFile ) ;
      #else
         /* CS 1999/04/22 In C/S, can't directly get
            descending info on tag so scan TAG4INFO array. */
         int rc ;
         TAG4INFO *tagInfo = i4tagInfo( tag->index );
         char *tagName = t4alias( tag ) ;

         for ( int i = 0 ; tagInfo[i].name ; i++ )
         {
            #ifdef S4CLIPPER
               rc = u4namecmp(tagName,tagInfo[i].name) ;
            #else
               #ifdef __unix__
                  rc = strcasecmp(tagName,tagInfo[i].name) ;
               #else
                  rc = stricmp(tagName,tagInfo[i].name) ;
               #endif
            #endif
            if ( rc == 0 )
            {
               short desc = tagInfo[i].descending ;
               u4free(tagInfo) ;
               return desc ;
            }
         }

         u4free(tagInfo) ;
         return error4( tag->index->codeBase, e4result, 0 ) ;  // The tag was not found in the array.
      #endif
   }

   /*
   #ifdef S4CLIENT
      int t4type( TAG4 *t4 )
      {
         if ( t4->tagFile->keyType != 0 )
            return t4->tagFile->keyType ;

         TAG4INFO *tagInfo = i4tagInfo( tag->index );
         char *tagName = t4alias( t4 ) ;

         for ( int i = 0 ; tagInfo[i].name ; i++ )
         {
            if ( stricmp(tagName,tagInfo[i].name) == 0 )
            {
               t4->tagFile->keyType = tagInfo[i].descending ;
               u4free(tagInfo) ;
               return t4->tagFile->keyType ;
            }
         }

         u4free(tagInfo) ;
         return error4( t4->index->codeBase, e4result, 0 ) ;  // The tag was not found in the array.
      }
   #endif
   */


   #ifdef S4CLIENT
      S4CONST char *S4FUNCTION t4exprLow( TAG4 *t4 )
      {
         #ifdef E4VBASIC
            #ifdef S4CB51
               if ( c4parm_check( t4, 4, E40148 ) ) return 0 ;
            #else
               if ( c4parm_check( t4, 4, E91641 ) ) return 0 ;
            #endif
         #endif

         #ifdef E4PARM_HIGH
            if ( t4 == 0 )
            {
               error4( 0, e4parm_null, E91641 ) ;
               return 0 ;
            }
            if ( t4->tagFile == 0 )
            {
               error4( 0, e4parm, E91641 ) ;
               return 0 ;
            }
         #endif

         if ( t4->tagFile->exprPtr == 0 )
         {
            TAG4INFO *temp = i4tagInfo( t4->index ) ;
            if ( temp == 0 )
               return 0 ;
            u4free( temp ) ;
            temp = 0 ;
            #ifdef E4ANALYZE
               if ( t4->tagFile->exprPtr == 0 )
               {
                  error4( 0, e4info, E91641 ) ;
                  return 0 ;
               }
            #endif
         }

         return t4->tagFile->exprPtr ;
      }


      int S4FUNCTION t4keyLenExported( TAG4 *tag )
      {
         assert5( tag != 0 ) ;

         if ( tag->tagFile->keyLen == 0 )
         {
            TAG4INFO *temp = i4tagInfo( tag->index ) ;
            if ( temp == 0 )
               return -1 ;
            u4free( temp ) ;
            temp = 0 ;
         }

         assert5( tag->tagFile->keyLen != 0 ) ;
         return tag->tagFile->keyLen ;
      }
   #endif /* S4CLIENT */

   #ifdef S4CLIENT
      extern unsigned short f4memoNullChar ;

      S4CONST char *S4FUNCTION t4filterLow( TAG4 *t4 )
      {
         TAG4INFO *temp ;

         #ifdef E4VBASIC
            #ifdef S4CB51
               if ( c4parm_check( t4, 4, E40149 ) )
                  return 0 ;
            #else
               if ( c4parm_check( t4, 4, E91641 ) )
                  return 0 ;
            #endif
         #endif

         #ifdef E4PARM_HIGH
            if ( t4 == 0 )
            {
               error4( 0, e4parm_null, E91641 ) ;
               return 0 ;
            }

            if ( t4->tagFile == 0 )
            {
               error4( 0, e4parm, E91641 ) ;
               return 0 ;
            }
         #endif

         if ( t4->tagFile->filterPtr == 0 )
         {
            temp = i4tagInfo( t4->index ) ;
            if ( temp == 0 )
            {
               error4( 0, e4memory, E91641 ) ;
               return 0 ;
            }
            u4free( temp ) ;
            temp = 0 ;
         }

         if ( t4->tagFile->filterPtr == 0 )
            return (char *)(&f4memoNullChar) ;   /* pointer to empty string */
         else
            return t4->tagFile->filterPtr ;
      }
   #endif /* S4CLIENT */



   #ifndef S4CLIENT
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
   #endif  /* S4CLIENT */
#endif  /* S4OFF_INDEX */
