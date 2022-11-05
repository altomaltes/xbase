/* i4info.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

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

#ifdef S4CLIENT
S4CONST char *S4FUNCTION t4exprLow( TAG4 *t4 )
{
   TAG4INFO *temp ;

   #ifdef S4VBASIC
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
      temp = i4tagInfo( t4->index ) ;
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

extern char f4memoNullChar ;

S4CONST char *S4FUNCTION t4filterLow( TAG4 *t4 )
{
   TAG4INFO *temp ;

   #ifdef S4VBASIC
   #ifdef S4CB51
      if ( c4parm_check( t4, 4, E40149 ) ) return 0 ;
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
      return &f4memoNullChar ;   /* pointer to empty string */
   else
      return t4->tagFile->filterPtr ;
}

TAG4INFO *S4FUNCTION i4tagInfo( INDEX4 *i4 )
{
   CONNECTION4 *connection ;
   CONNECTION4INDEX_INFO_OUT *out ;
   CODE4 *c4 ;
   int rc, offset, len ;
   CONNECTION4TAG_INFO *tagInfo ;
   TAG4FILE *tagFile ;
   unsigned int i ;

   #ifdef E4PARM_HIGH
      if ( i4 == 0 )
      {
         error4( 0, e4parm_null, E95501 ) ;
         return 0 ;
      }
   #endif

   c4 = i4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return 0 ;

   connection = i4->data->dataFile->connection ;
   connection4assign( connection, CON4INDEX_INFO, data4clientId( i4->data ), data4serverId( i4->data ) ) ;
   connection4addData( connection, i4->indexFile->accessName, strlen( i4->indexFile->accessName ) + 1, NULL ) ;
   connection4sendMessage( connection ) ;
   rc = connection4receiveMessage( connection ) ;
   if ( rc < 0 )
   {
      #ifdef E4STACK
         error4stack( c4, rc, E95501 ) ;
      #endif
      return 0 ;
   }

   rc = connection4status( connection ) ;
   if ( rc < 0 )
   {
      connection4error( connection, c4, rc, E95501 ) ;
      return 0 ;
   }

   if ( connection4len( connection ) < sizeof( CONNECTION4INDEX_INFO_OUT ) )
   {
      error4( c4, e4packetLen, E95501 ) ;
      return 0 ;
   }

   out = (CONNECTION4INDEX_INFO_OUT *)connection4data( connection ) ;
   out->numTags = ntohs(out->numTags) ;
   if ( connection4len( connection ) < (long)(sizeof( CONNECTION4INDEX_INFO_OUT ) + (unsigned long)(out->numTags * sizeof( CONNECTION4TAG_INFO )) ))
   {
      error4( c4, e4packetLen, E95501 ) ;
      return 0 ;
   }

   tagInfo = (CONNECTION4TAG_INFO *)u4allocFree( c4, ( out->numTags + 1 ) * sizeof( CONNECTION4TAG_INFO ) ) ;
   if ( tagInfo == 0 )
      return 0 ;

   offset = sizeof( CONNECTION4INDEX_INFO_OUT ) ;
   for ( i = 0 ; i < out->numTags ; i++ )
   {
      memcpy( &tagInfo[i], (char *)out + offset, sizeof( CONNECTION4TAG_INFO ) ) ;
      tagInfo[i].name.ptr = (char *)out + ntohs(tagInfo[i].name.offset) ;
      tagInfo[i].expression.ptr = (char *)out + ntohs(tagInfo[i].expression.offset) ;
      offset += ( sizeof( CONNECTION4TAG_INFO ) + strlen( tagInfo[i].name.ptr )
                + strlen( tagInfo[i].expression.ptr ) + 2 ) ;
      if ( tagInfo[i].filter.offset == 0 )
         tagInfo[i].filter.ptr = 0 ;
      else
      {
         tagInfo[i].filter.ptr = (char *)out + ntohs(tagInfo[i].filter.offset) ;
         offset += ( strlen( tagInfo[i].filter.ptr ) + 1 ) ;
      }
      tagInfo[i].unique = ntohs(tagInfo[i].unique) ;
      tagInfo[i].descending = ntohs(tagInfo[i].descending) ;
      tagFile = d4tag( i4->data, tagInfo[i].name.ptr )->tagFile ;
      if ( tagFile == 0 )   /* just skip this tag */
      {
         out->numTags-- ;
         i-- ;
         continue ;
      }
      tagInfo[i].name.ptr = tagFile->alias ;   /* remove pointer to communication memory, and point to tag name */
      if ( tagFile->exprPtr == 0 )
      {
         len = strlen( tagInfo[i].expression.ptr ) ;
         if ( len != 0 )
         {
            tagFile->exprPtr = (char *)u4allocFree( c4, len + 1 ) ;
            if ( len == 0 )
            {
               error4( c4, e4memory, E95501 ) ;
               u4free( tagInfo ) ;
               return 0 ;
            }
            memcpy( tagFile->exprPtr, tagInfo[i].expression.ptr, len ) ;
            tagFile->exprPtr[len] = 0 ;
            tagInfo[i].expression.ptr = tagFile->exprPtr ;
         }
      }
      if ( tagFile->filterPtr == 0 )
      {
         if ( tagInfo[i].filter.ptr != 0 )
         {
            len = strlen( tagInfo[i].filter.ptr ) ;
            if ( len != 0 )
            {
               tagFile->filterPtr = (char *)u4allocFree( c4, len + 1 ) ;
               if ( len == 0 )
               {
                  error4( c4, e4memory, E95501 ) ;
                  u4free( tagInfo ) ;
                  return 0 ;
               }
               memcpy( tagFile->filterPtr, tagInfo[i].filter.ptr, len ) ;
               tagFile->filterPtr[len] = 0 ;
               tagInfo[i].filter.ptr = tagFile->filterPtr ;
            }
         }
      }

   }

   return (TAG4INFO *)tagInfo ;
}
#endif

unsigned short int S4FUNCTION tfile4isDescending( TAG4FILE *tag )
{
   #ifdef S4CLIENT
      error4( tag->codeBase, e4notSupported, 0 ) ;
      return 0 ;
   #else
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
   #endif
}

#ifndef S4CLIENT
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

#endif  /* S4CLIENT */
#endif  /* S4OFF_INDEX */
