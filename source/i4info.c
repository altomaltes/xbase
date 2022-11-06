/* i4info.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_INDEX
char *S4FUNCTION t4alias( TAG4 *t4 )
{
   #ifdef S4VBASIC
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

unsigned short int S4FUNCTION tfile4isDescending( TAG4FILE *tag )
{
      #ifdef S4NDX
         return 0 ;
      #endif
      #ifdef S4FOX
         if ( tag->header.descending )
            return r4descending ;
         else
            return 0 ;
      #endif
      #ifdef S4CLIPPER
         if ( tag->header.descending )
            return r4descending ;
         else
            return 0 ;
      #endif
      #ifdef S4MDX
         if ( tag->header.typeCode & 8 )
            return r4descending ;
         else
            return 0 ;
      #endif
}

TAG4INFO *S4FUNCTION i4tagInfo( INDEX4 *i4 )
{
   TAG4INFO *tagInfo ;
   TAG4 *tagOn ;
   int numTags, i ;

   #ifdef E4PARM_HIGH
      if ( i4 == 0 )
      {
         error4( 0, e4parm_null, E95501 ) ;
         return 0 ;
      }
   #endif

   if ( error4code( i4->codeBase ) < 0 )
      return 0 ;

   numTags = 0 ;
   for( tagOn = 0 ;; )
   {
      tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
      if ( tagOn == 0 )
         break ;
      numTags++ ;
   }

   if ( error4code( i4->codeBase ) < 0 )
      return 0 ;
   tagInfo = (TAG4INFO *)u4allocFree( i4->codeBase, ( (long)numTags + 1L ) * sizeof( TAG4INFO ) ) ;
   if ( tagInfo == 0 )
      return 0 ;

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
