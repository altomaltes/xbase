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

/* c4const.c */

#include "d4all.h"



#ifndef S4INDEX_OFF

/* This function creates a branch out of the input constant, and combines it with the input map */
static int bitmap4constantCombine( BITMAP4 *parent, BITMAP4 *oldAndMap, CONST4 *con, int conType )
{
   BITMAP4 *tempLeaf, *andMap, *newBranch ;
   CONST4 *temp ;

   if ( con->len == 0 || error4code( parent->log->codeBase ) == e4memory )
      return 0 ;

   newBranch = bitmap4create( parent->log, parent->relate, 1, 1 ) ;
   if ( newBranch == 0 )
      return 0 ;

   andMap = bitmap4create( parent->log, parent->relate, 0, 0 ) ;
   if ( andMap == 0 )
      return 0 ;
   bitmap4copy( andMap, oldAndMap ) ;
   l4add( &newBranch->children, andMap ) ;

   tempLeaf = bitmap4create( parent->log, parent->relate, 1, 0 ) ;
   if ( tempLeaf == 0 )
      return 0 ;
   tempLeaf->type = andMap->type ;
   tempLeaf->tag = andMap->tag ;
   l4add( &newBranch->children, tempLeaf ) ;

   switch( conType )
   {
      case 1:
         memcpy( (void *)&tempLeaf->lt, (void *)con, sizeof( CONST4 ) ) ;
         break ;
      case 2:
         memcpy( (void *)&tempLeaf->le, (void *)con, sizeof( CONST4 ) ) ;
         break ;
      case 3:
         memcpy( (void *)&tempLeaf->gt, (void *)con, sizeof( CONST4 ) ) ;
         break ;
      case 4:
         memcpy( (void *)&tempLeaf->ge, (void *)con, sizeof( CONST4 ) ) ;
         break ;
      case 5:
         memcpy( (void *)&tempLeaf->eq, (void *)con, sizeof( CONST4 ) ) ;
         break ;
      case 6:
         temp = (CONST4 *)u4alloc( (long)sizeof( CONST4 ) ) ;
         if ( temp == 0 )
            return 0 ;
         memcpy( (void *)temp, (void *)con, sizeof( CONST4 ) ) ;
         l4add( &tempLeaf->ne, temp ) ;
         break ;
      default:
         return error4( parent->log->codeBase, e4info, E93701 ) ;
   }
   memset( (void *)con, 0 ,sizeof( CONST4 ) ) ;
   newBranch = bitmap4redistribute( 0, newBranch, 0 ) ;

   if ( error4code( parent->log->codeBase ) < 0 )
      return error4code( parent->log->codeBase ) ;

   if ( newBranch->children.nLink == 0 )
      bitmap4destroy( newBranch ) ;
   else
      l4add( &parent->children, newBranch ) ;

   return 0 ;
}

/* this function redistributes (splits and combines) and/and, or/or block sequences */
BITMAP4 * bitmap4redistribute( BITMAP4 *parent, BITMAP4 *map, const char doShrink )
{
   BITMAP4 *childMap, *childOn, *parent2map ;
   char split ;

   if ( map->branch == 0 )
      return map ;

   /* first combine all the children of this map */
   childOn = childMap = (BITMAP4 *)l4first( &map->children ) ;
   for( ;; )
   {
      if ( childOn == 0 )
         break ;
      childOn = bitmap4redistribute( map, childOn, 0 ) ;
      childOn = (BITMAP4 *)l4next( &map->children, childOn ) ;
   }

   /* now combine all leaf children where possible */
   if ( parent != 0 )
      if ( parent->andOr != map->andOr )  /* case where no combos possible */
         return map ;

   parent2map = 0 ;
   childMap = (BITMAP4 *)l4first( &map->children ) ;
   for( ; childMap != 0 ; )
   {
      childOn = (BITMAP4 *)l4next( &map->children, childMap ) ;
      if ( childOn == 0 )
         break ;

      split = 0 ;
      if ( childOn->tag != childMap->tag || childOn->andOr != childMap->andOr )
        split = 1 ;
      else
      {
        if ( map != 0 )
           if ( map->andOr != childOn->andOr )
              split = 1 ;
      }

      if ( split == 1 )
      {
         if ( parent2map == 0 )
         {
            parent2map = bitmap4create( map->log, map->relate, map->andOr, 1 ) ;
            if ( parent2map == 0 )  /* must handle by freeing... */
               return 0 ;
            if ( parent == 0 )
            {
               parent = bitmap4create( map->log, map->relate, map->andOr, 1 ) ;
               if ( parent == 0 )  /* must handle by freeing... */
                  return 0 ;
               l4add( &parent->children, map ) ;
            }
            l4add( &parent->children, parent2map ) ;
         }
         l4remove( &map->children, childOn ) ;
         l4add( &parent2map->children, childOn ) ;
      }
      else
      {
         childMap = bitmap4combineLeafs( map, childMap, childOn ) ;
         if ( error4code( map->log->codeBase ) < 0 )
            return 0 ;
      }
   }

   if ( parent2map != 0 )
   {
      #ifdef E4ANALYZE
         if ( parent == 0 )
         {
            error4( map->log->codeBase, e4info, E93701 ) ;
            return 0 ;
         }
      #endif
      bitmap4redistribute( parent, parent2map, 1 ) ;
   }

   if ( doShrink )
   {
      if ( map->children.nLink == 1 )   /* just a child, so remove myself */
      {
         childMap = (BITMAP4 *)l4first( &map->children ) ;
         l4remove( &map->children, childMap ) ;
         if ( parent != 0 )
         {
            #ifdef E4ANALYZE
               if ( childMap->tag == 0 && childMap->children.nLink == 0 )
               {
                  error4( childMap->log->codeBase, e4info, E93701 ) ;
                  return 0 ;
               }
            #endif
            if ( parent->tag == 0 && childMap->tag != 0 )
               parent->tag = childMap->tag ;
            l4addAfter( &parent->children, map, childMap ) ;
            l4remove( &parent->children, map ) ;
         }
         bitmap4destroy( map ) ;
         map = childMap ;
      }
   }

   if ( parent2map != 0 && parent != 0 )
      return parent ;

   return map ;
}

/* this function redistributes the input maps by breaking the one up into constants and creating maps for each */
BITMAP4 *bitmap4redistributeLeaf( BITMAP4 *parent, BITMAP4 *map1, BITMAP4 *map2 )
{
   BITMAP4 *newBranch, *orMap, *place, *andMap, *temp ;
   CONST4 *cOn ;

   newBranch = bitmap4create( parent->log, parent->relate, 1, 1 ) ;
   if ( newBranch == 0 )
      return 0 ;

   place = bitmap4create( parent->log, parent->relate, 0, 0 ) ;
   if ( place == 0 )
      return 0 ;
   l4addAfter( &parent->children, map1, place ) ;

   l4remove( &parent->children, map1 ) ;
   l4remove( &parent->children, map2 ) ;

   if ( map1->andOr == 1 )
   {
      andMap = map1 ;
      orMap = map2 ;
   }
   else
   {
      andMap = map2 ;
      orMap = map1 ;
   }

   bitmap4constantCombine( newBranch, andMap, &orMap->lt, 1 ) ;
   bitmap4constantCombine( newBranch, andMap, &orMap->le, 2 ) ;
   bitmap4constantCombine( newBranch, andMap, &orMap->gt, 3 ) ;
   bitmap4constantCombine( newBranch, andMap, &orMap->ge, 4 ) ;
   bitmap4constantCombine( newBranch, andMap, &orMap->eq, 5 ) ;
   for( ;; )
   {
      cOn = (CONST4 *)l4first( &orMap->ne ) ;
      if ( cOn == 0 )
         break ;
      bitmap4constantCombine( newBranch, andMap, cOn, 6 ) ;
   }

   if ( error4code( parent->log->codeBase ) == e4memory )
      return 0 ;

   if ( newBranch->children.nLink == 0 )   /* collapsed */
   {
      if ( parent->tag == 0 && andMap->tag != 0 )
         parent->tag = andMap->tag ;
      bitmap4destroy( newBranch ) ;
      newBranch = 0 ;
   }
   else
   {
      while( newBranch->branch == 1 && newBranch->children.nLink == 1 )
      {
         temp = (BITMAP4 *)l4first( &newBranch->children ) ;
         bitmap4destroy( newBranch ) ;
         newBranch = temp ;
      }
      l4addAfter( &parent->children, place, newBranch ) ;
   }

   l4remove( &parent->children, place ) ;
   bitmap4destroy( place ) ;
   bitmap4destroy( orMap ) ;
   bitmap4destroy( andMap ) ;

   return newBranch ;
}

/* this function splits and combines and/or, or/and block sequences */
/* all bitmaps must be in standard bitmap4redistribute format prior to call */
BITMAP4 * bitmap4redistributeBranch( BITMAP4 *parent, BITMAP4 *map )
{
   BITMAP4 *childOn2, *childOn, *childNext2 ;

   if ( map->branch == 0 )
      return map ;

   childOn = (BITMAP4 *)l4first( &map->children ) ;

   for( ;; )
   {
      if ( childOn == 0 )
         break ;
      if ( childOn->branch )
      {
         childOn = bitmap4redistributeBranch( map, childOn ) ;
         if ( childOn == 0 && error4code( parent->log->codeBase ) == e4memory )
            return 0 ;
      }
      if ( childOn->branch == 0 )
      {
         childOn2 = (BITMAP4 *)l4next( &map->children, childOn ) ;
         while( childOn2 != 0 )
         {
            if ( childOn2->branch )
            {
               childOn2 = bitmap4redistributeBranch( map, childOn2 ) ;
               if ( childOn2 == 0 && error4code( parent->log->codeBase ) == e4memory )
                  return 0 ;
            }
            childNext2 = (BITMAP4 *)l4next( &map->children, childOn2 ) ;
            if ( childOn->branch == 0 && map->andOr == 1 && childOn->tag == childOn2->tag &&  childOn->andOr != childOn2->andOr )
            {
               childOn = bitmap4redistributeLeaf( map, childOn, childOn2 ) ;
               if ( childOn == 0 && error4code( parent->log->codeBase ) == e4memory )
                  return 0 ;
            }
            childOn2 = childNext2 ;
         }
      }
      childOn = (BITMAP4 *)l4next( &map->children, childOn ) ;
   }

   if ( map->branch == 1 )
   {
      if ( map->children.nLink == 0 )   /* mark ourselves as a leaf with no match */
      {
         map->branch = 0 ;
         map->noMatch = 1 ;
      }
      else
         if ( map->children.nLink == 1 )   /* just a child, so remove myself */
         {
            childOn = (BITMAP4 *)l4first( &map->children ) ;
            l4remove( &map->children, childOn ) ;
            if ( parent != 0 )
            {
               l4addAfter( &parent->children, map, childOn ) ;
               l4remove( &parent->children, map ) ;
            }
            bitmap4destroy( map ) ;
            map = childOn ;
         }
   }

   return map ;
}

/* location = 0 if seek_before, 1 if seek 1st, 2 if seek last, 3 if seek_after,  */
/* add 10 if it is to be an approximate seek */
/* returns record number */
#ifdef S4HAS_DESCENDING
long bitmap4seek( BITMAP4 *map, const CONST4 *con, const char location, const long check, const int doCheck )
{
   int len, rc ;
   TAG4FILE *tag ;
   char *result ;
   #ifdef S4CLIPPER
      char holdResult[20] ;  /* enough space for a numerical key */
   #endif

   #ifdef S4VFP_KEY
      char buf[I4MAX_KEY_SIZE] ;
   #endif

   tag = map->tag ;
   result = (char *)const4return( map->log, con ) ;

   /* must convert to a proper key */
   if ( map->type != r4str )
   {
      #ifdef S4CLIPPER
         memcpy( holdResult, result, con->len ) ;
         result = holdResult ;
      #endif
      #ifdef E4ANALYZE
         if ( expr4len( tag->expr ) == -1 )
            return error4( map->log->codeBase, e4info, E83701 ) ;
      #endif
      len = expr4keyConvert( tag->expr, (char **)&result, con->len, map->type, tag ) ;
   }
   else
   {
      len = con->len ;
      #ifdef S4VFP_KEY
      if ( tfile4vfpKey( tag ) )
      {
         if ( len*2 > sizeof(buf) )
            return error4( map->log->codeBase, e4info, E82102 ) ;
         len = t4strToVFPKey( buf, result, len, len*2, &tag->vfpInfo ) ;
         if ( len < 0 )
            return error4( map->log->codeBase, e4info, E85404 ) ;
         result = buf ;
      }
      #endif
   }

   if ( location > 1 )
      tfile4descending( tag, 1 ) ;
   else
      tfile4descending( tag, 0 ) ;
   tfile4seek( tag, result, len ) ;
   tfile4descending( tag, 0 ) ;

   if ( !tfile4eof( tag ) )
      if ( doCheck == 1 )
        if ( check == tfile4recNo( tag ) )
           return -1 ;

   switch ( location )
   {
      case 0:
         if ( tfile4skip( tag, -1L ) != -1L )   /* at top already */
            return -1 ;
         break ;
      case 1:
         if ( tfile4eof( tag ) )
            return -1 ;
         break ;
      case 2:
         if ( !tfile4eof( tag ) )
         #ifdef S4FOX
            if( u4keycmp( tfile4keyData(tag)->value, result, (unsigned int)len, (unsigned int)tag->header.keyLen, 0, &tag->vfpInfo ) != 0 )
          #else
            if( (*tag->cmp)( tfile4keyData(tag)->value, result, (unsigned int)len ) != 0 )  /* last one is too far, go back one for a closure */
         #endif
         {
            if ( doCheck == 1 )
               if ( check == tfile4recNo( tag ) )   /* case where none belong, so break now */
                  return -1 ;
         }
         break ;
      case 3:
         if ( tfile4eof( tag ) )
         {
            rc = tfile4top( tag ) ;
            if ( rc != 0 )  /* no records */
               return -1 ;
         }
         else
         {
            rc = (int)tfile4skip( tag, 1L ) ;
            if ( rc == 0L )
               return -1 ;
         }
         break ;
      default:
         return error4( map->log->codeBase, e4info, E93701 ) ;
   }

   return tfile4recNo( tag ) ;
}
#endif
#ifdef S4MDX
long bitmap4seek( BITMAP4 *map, const CONST4 *con, const char location, const long check, const int doCheck )
{
   int len, rc, seekRc, isDesc ;
   TAG4FILE *tag ;
   char *result ;
   char didSkip ;

   tag = map->tag ;
   result = (char *)const4return( map->log, con ) ;
   isDesc = ( tag->header.typeCode & 8 ) ? 1 : 0 ;

   if ( map->type != r4str )   /* must convert to a proper key */
   {
      #ifdef E4ANALYZE
         if ( expr4len( tag->expr ) == -1 )
            return error4( map->log->codeBase, e4info, E83701 ) ;
      #endif
      #ifdef S4VFP_KEY
         len = expr4keyConvert( tag->expr, (char **)&result, con->len, map->type, tag->expr->vfpInfo->sortType ) ;
      #else
         len = expr4keyConvert( tag->expr, (char **)&result, con->len, map->type, 0 ) ;
      #endif
   }
   else
      len = con->len ;

   seekRc = tfile4seek( tag, result, len ) ;

   if ( !tfile4eof( tag ) )
      if ( doCheck && location < 2 )
         if ( check == tfile4recNo( tag ) )
            return -1 ;

   switch ( location )
   {
      case 0:
         if ( isDesc )
         {
            if ( tfile4eof( tag ) )
               return -1 ;
            for(; (*tag->cmp)( tfile4keyData(tag)->value, result, len ) == 0; )
            {
               rc = (int)tfile4skip( tag, 1L ) ;
               if ( rc < 0 )
                 return -1 ;
               if ( rc != 1 )
               {
                 if ( rc == 0 )
                    return -1 ;
                 break ;
               }
            }
         }
         else
            if ( tfile4skip( tag, -1L ) != -1L )   /* at top already */
               return -1 ;
         break ;
       case 1:
         if ( isDesc )
         {
            if ( seekRc == 2 )
            {
               if ( !tfile4eof( tag ) )
                 if ( check == tfile4recNo( tag ) )   /* case where none belong, so break now */
                    return -1 ;
               if ( tfile4skip( tag, -1L ) != -1L )
                 return -1 ;
            }
            else
            {
               rc = -1 ;
               for(; (*tag->cmp)( tfile4keyData(tag)->value, result, len ) == 0; )
               {
                  rc = (int)tfile4skip( tag, 1L ) ;
                  if ( rc < 0 )
                     return -1 ;
                  if ( rc != 1 )
                     break ;
               }
               if ( rc == 0 )
                  tfile4bottom( tag ) ;
               else
                  tfile4skip( tag, -1L ) ;
            }
         }
         else
         {
            if ( tfile4eof( tag ) )
               return -1 ;
         }
         break ;
      case 2:
         if ( isDesc )
         {
            if ( tfile4eof( tag ) )
               return -1 ;
            if ( seekRc == 2 )
               if ( check == tfile4recNo( tag ) )   /* case where none belong, so break now */
                  return -1 ;
            break ;
         }
         else
         {
            if( (*tag->cmp)( tfile4keyData(tag)->value, result, len ) != 0 )  /* last one is too far, go back one for a closure */
            {
               if ( !tfile4eof( tag ) )
                  if ( check == tfile4recNo( tag ) )   /* case where none belong, so break now */
                     return -1 ;
               if ( tfile4skip( tag, -1L ) != -1L )
                  return -1 ;
            }
         }
      case 3:
         if ( isDesc )
         {
            if ( tfile4skip( tag, -1L ) != -1L )   /* at top already */
               return -1 ;
         }
         else
         {
            didSkip = 0 ;

            for(; (*tag->cmp)( tfile4keyData(tag)->value, result, len ) == 0; )
            {
               rc = (int)tfile4skip( tag, 1L ) ;
               if ( rc < 0 )
                  return -1 ;
               if ( rc != 1 )
               {
                  if ( location == 2 )   /* on last record, but it still belongs, so don't skip back */
                     didSkip = 0 ;
                  if ( location == 3 )   /* on last record not far enough, so none match */
                     return -1 ;
                  break ;
               }
               didSkip = 1 ;
            }

            if ( location == 3 )
            {
               if ( didSkip == 0 && seekRc != 2 )
                  if ( tfile4skip( tag, 1L ) != 1L )
                     return -1 ;
            }
            else
               if ( didSkip == 1 )
                 if ( tfile4skip( tag, -1L ) != -1L )
                     return -1 ;
         }
         break ;
      default:
         return error4( map->log->codeBase, e4info, E93701 ) ;
   }

   return tfile4recNo( tag ) ;
}
#endif

/* returns a pointer to the constant value */
void *const4return( L4LOGICAL *log, const CONST4 *c1 )
{
   return (void *)( log->buf + c1->offset ) ;
}

/* updates the log's constant memory buffer, re-allocating memory if required */
int const4memAlloc( L4LOGICAL *log, const unsigned len )
{
   if ( ( log->bufPos + len ) > log->bufLen )
   {
      #ifdef E4ANALYZE
         if ( (long)len + (long)log->bufLen != (long)(len + log->bufLen) )
            return error4( log->codeBase, e4memory, E83702 ) ;
      #endif
      if ( u4allocAgain( log->codeBase, &log->buf, &log->bufLen, log->bufPos + len ) != 0 )
         return error4( log->codeBase, e4memory, E93704 ) ;
   }
   log->bufPos += len ;
   return 0 ;
}

/* duplicate an existing constant */
int const4duplicate( CONST4 *to, const CONST4 *from, L4LOGICAL *log )
{
   unsigned int len ;

   len = (unsigned int)from->len ;

   if ( len == 0 )
      memset( (void *)to, 0, (unsigned int)sizeof( CONST4 ) ) ;
   else
   {
      if ( const4memAlloc( log, len ) < 0 )
         return -1 ;
      memcpy( log->buf + log->bufPos - len, const4return( log, from ), len ) ;
      to->offset = log->bufLen - len ;
      to->len = len ;
   }

   return 0 ;
}

/* get a constant from an expr. info structure */
int const4get( CONST4 *con, BITMAP4 *map, L4LOGICAL *log, const int pos )
{
   unsigned int len ;
   char *result ;
   int rc ;

   if ( expr4execute( log->expr, pos, (void **)&result ) < 0 )
      return -1 ;
   len = (unsigned int)log->expr->info[pos].len ;

   #ifdef E4ANALYZE
      if ( map->type != 0 && map->type != v4functions[log->expr->info[pos].functionI].returnType )
         return error4( map->log->codeBase, e4info, E83703 ) ;
   #endif

   rc = const4memAlloc( log, len ) ;
   if ( rc < 0 )
      return error4stack( map->log->codeBase, rc, E93704 ) ;

   memcpy( log->buf + log->bufPos - len, result, len ) ;
   map->type = v4functions[log->expr->info[pos].functionI].returnType ;
   con->offset = log->bufLen - len ;
   con->len = len ;

   return 0 ;
}

int const4less( CONST4 *p1, CONST4 *p2, BITMAP4 *map )
{
   switch( map->type )
   {
      case r4numDoub:
      case r4dateDoub:
         #ifdef E4ANALYZE
            if ( p1->len != p2->len )
               return error4( map->log->codeBase, e4struct, E93704 ) ;
         #endif
         if ( *(double *)const4return( map->log, p1 ) < *(double *)const4return( map->log, p2 ) )
            return 1 ;
         break ;
      case r4num:
      case r4str:
         if ( p1->len < p2->len )
         {
            if ( c4memcmp( const4return( map->log, p1 ), const4return( map->log, p2 ), (unsigned int)p1->len ) <= 0 )
               return 1 ;
         }
         else
            if ( c4memcmp( const4return( map->log, p1 ), const4return( map->log, p2 ), (unsigned int)p2->len ) < 0 )
               return 1 ;
         break ;
      default:
         return error4( map->log->codeBase, e4info, E93704 ) ;
   }

   return 0 ;
}

int const4eq( CONST4 *p1, CONST4 *p2, BITMAP4 *map )
{
   if ( p1->len < p2->len )
   {
      #ifdef E4ANALYZE
         if ( map->type == r4numDoub || map->type == r4dateDoub )
            return error4( map->log->codeBase, e4struct, E93704 ) ;
      #endif
      return 0 ;
   }

   #ifdef E4ANALYZE
      switch( map->type )
      {
         case r4numDoub:
         case r4dateDoub:
         case r4num:
         case r4str:
         case r4log:
            break ;
         default:
            return error4( map->log->codeBase, e4info, E93704 ) ;
      }
   #endif

   if ( c4memcmp( const4return( map->log, p1 ), const4return( map->log, p2 ), (unsigned int)p1->len ) == 0 )
      return 1 ;

   return 0 ;
}

int const4lessEq( CONST4 *p1, CONST4 *p2, BITMAP4 *map )
{
   switch( map->type )
   {
      case r4numDoub:
      case r4dateDoub:
         #ifdef E4ANALYZE
            if ( p1->len != p2->len )
               return error4( map->log->codeBase, e4struct, E93704 ) ;
         #endif
         if ( *(double *)const4return( map->log, p1 ) <= *(double *)const4return( map->log, p2 ) )
            return 1 ;
         break ;
      case r4num:
      case r4str:
         if ( p1->len <= p2->len )
         {
            if ( c4memcmp( const4return( map->log, p1 ), const4return( map->log, p2 ), (unsigned int)p1->len ) <= 0 )
               return 1 ;
         }
         else
            if ( c4memcmp( const4return( map->log, p1 ), const4return( map->log, p2 ), (unsigned int)p2->len ) < 0 )
               return 1 ;
         break ;
      default:
         return error4( map->log->codeBase, e4info, E93704 ) ;
   }

   return 0 ;
}

void const4addNe( BITMAP4 *map, CONST4 *con )
{
   CONST4 *cOn ;

   cOn = (CONST4 *)l4first( &map->ne ) ;
   while ( cOn != 0 )
   {
      if ( const4eq( con, cOn, map ) )  /* ne already exists, so ignore */
         return ;
      cOn = (CONST4 *)l4next( &map->ne, cOn ) ;
   }
   cOn = (CONST4 *) u4alloc( (long)sizeof( CONST4 ) ) ;
   if ( cOn == 0 )
      return ;
   memcpy( (void *)cOn, (void *)con, (unsigned int)sizeof( CONST4 ) ) ;
   l4add( &map->ne, cOn ) ;
   memset( (void *)con, 0, (unsigned int)sizeof( CONST4 ) ) ;
}

void const4deleteNe( LIST4 *list, CONST4 *con )
{
   l4remove( list, con ) ;
   u4free( con ) ;
}

#endif   /* S4INDEX_OFF */

