/* d4tag.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifdef P4ARGS_USED
   #pragma argsused
#endif
TAG4 *S4FUNCTION d4tag( DATA4 *d4, const char* const tagName )
{
   #ifndef S4INDEX_OFF
      char tagLookup[LEN4TAG_ALIAS+1] ;
      TAG4 *tagOn ;

   C4PARMDATA( d4, E92401, NULL ); 

      #ifdef E4PARM_HIGH
         if ( tagName == 0 )
         {
            error4( 0, e4parmNull, E92401 ) ;
            return 0 ;
         }
      #endif

      u4ncpy( tagLookup, tagName, sizeof( tagLookup ) ) ;
      c4upper( tagLookup ) ;

      for( tagOn = 0 ;; )
      {
         tagOn = d4tagNext( d4, tagOn ) ;
         if ( tagOn == 0 )
            break ;
         if ( strcmp( tagOn->tagFile->alias, tagLookup ) == 0 )
            return tagOn ;
      }

      if ( d4->codeBase->errTagName )
         error4describe( d4->codeBase, e4tagName, E92401, tagName, 0, 0 ) ;
   #endif
   return 0 ;
}

/* for N4OTHER should compare full name, adding extension if required */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
TAG4FILE *dfile4tag( DATA4FILE *d4, const char * const tagName )
{
   #ifndef S4INDEX_OFF
      char tagLookup[LEN4TAG_ALIAS+1] ;
      TAG4FILE *tagOn ;
      #ifdef N4OTHER
            char tagLookup2[258], ext1[3], ext2[3] ;
            int l1, l2 ;
      #endif

      E4PARMLOW( d4 && tagName, E91102 ) ;

      #ifdef N4OTHER
            u4nameCurrent( tagLookup2, sizeof( tagLookup2 ), tagName ) ;
            u4nameExt( tagLookup2, sizeof( tagLookup2 ), code4indexExtension( d4->c4 ), 0 ) ;
            c4upper( tagLookup2 ) ;
      #endif

      u4namePiece( tagLookup, sizeof( tagLookup ), tagName, 0, 0 ) ;
      #ifndef S4CASE_SEN
         c4upper( tagLookup ) ;
      #endif

      for( tagOn = 0 ;; )
      {
         tagOn = dfile4tagNext( d4, tagOn ) ;
         if ( tagOn == 0 )
            break ;
         #ifdef N4OTHER
               if ( strcmp( tagOn->file.name, tagLookup2 ) == 0 )
                  return tagOn ;
            if ( strcmp( tagOn->alias, tagLookup ) == 0 )  /* also need to verify extension*/
            {
                  l1 = u4nameRetExt( ext1, sizeof( ext1 ), tagOn->file.name ) ;
                  l2 = u4nameRetExt( ext2, sizeof( ext1 ), tagName ) ;
                  if ( ( l2 == 0 ) || ( ( l1 == l2 ) && ( memcmp( ext1, ext2, l1 ) == 0 ) ) )
                  return tagOn ;
            }
         #else
            if ( strcmp( tagOn->alias, tagLookup ) == 0 )
               return tagOn ;
         #endif
      }

      if ( d4->c4->errTagName )
         error4describe( d4->c4, e4tagName, E91102, tagName, 0, 0 ) ;
   #endif
   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
TAG4 *S4FUNCTION d4tagDefault( DATA4 *d4 )
{
   #ifndef S4INDEX_OFF
      TAG4 *tag ;
      INDEX4 *index ;

   C4PARMDATA( d4, E92403, NULL ); 

      tag = d4->tagSelected ;
      if ( tag )
         return tag ;

      index = (INDEX4 *)l4first( &d4->indexes ) ;
      if ( index )
      {
         tag = (TAG4 *)l4first( &index->tags ) ;
         if ( tag )
            return tag ;
      }
   #endif
   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
TAG4FILE *dfile4tagDefault( DATA4FILE *d4 )
{
   #ifndef S4INDEX_OFF
      TAG4FILE *tag ;

      E4PARMLOW( d4, E91102 ) ;

      tag = dfile4tagSelected( d4 ) ;
      if ( tag )
         return tag ;

      tag = dfile4tagNext( d4, 0 ) ;
      return tag ;
   #else
      return 0 ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
TAG4 *S4FUNCTION d4tagNext( DATA4 *d4, TAG4 * tag )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      INDEX4 *i4 ;
      TAG4 * tagOn ;

      tagOn = tag ;

   C4PARMDATA( d4, E92405, NULL ); 

      if ( tagOn == 0 )
      {
         i4 = (INDEX4 *)l4first( &d4->indexes ) ;
         if ( i4 == 0 )
            return 0 ;
      }
      else
      {
         for ( i4 = 0 ;; )
         {
            i4 = (INDEX4 *)l4next( &d4->indexes, i4 ) ;
            if ( i4 == 0 )
               return 0 ;
            if ( i4 == tagOn->index )
               break ;
         }
      }

      tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
      if ( tagOn == 0 )
      {
         i4 = (INDEX4 *)l4next( &d4->indexes, i4 )  ;
         if ( i4 == 0 )
            return 0 ;
         return (TAG4 *)l4first( &i4->tags ) ;
      }
      return tagOn ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
TAG4FILE *dfile4tagNext( DATA4FILE *d4, TAG4FILE *tagOn )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      #ifndef N4OTHER
         INDEX4FILE *i4 ;
      #endif

      E4PARMLOW( d4, E91102 ) ;

      #ifdef N4OTHER
         return (TAG4FILE *)l4next( &d4->tagfiles, tagOn ) ;
      #else
         if ( tagOn == 0 )
         {
            i4 = (INDEX4FILE *)l4first( &d4->indexes ) ;
            if ( i4 == 0 )
               return 0 ;
         }
         else
            i4 = tagOn->indexFile ;

         tagOn = (TAG4FILE *)l4next( &i4->tags, tagOn ) ;
         if ( tagOn )
            return tagOn ;

         i4 = (INDEX4FILE *)l4next( &d4->indexes, i4 ) ;
         if ( i4 == 0 )
            return 0 ;

         return (TAG4FILE *)l4first( &i4->tags ) ;
      #endif /* N4OTHER */
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
TAG4 *S4FUNCTION d4tagPrev( DATA4 *d4, TAG4 *tag )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      INDEX4 *i4 ;
      TAG4 * tagOn ;

      tagOn = tag ;

   C4PARMDATA( d4, E92407, -1 ); 

      if ( tagOn == 0 )
      {
         i4 = (INDEX4 *)l4last( &d4->indexes ) ;
         if ( i4 == 0 )
            return 0 ;
      }
      else
      {
         for ( i4 = 0 ;; )
         {
            i4 = (INDEX4 *)l4prev( &d4->indexes, i4 ) ;
            if ( i4 == 0 )
               return 0 ;
            if ( i4 == tagOn->index )
               break ;
         }
      }

      tagOn = (TAG4 *)l4prev( &i4->tags, tagOn ) ;
      if ( tagOn == 0 )
      {
         i4 = (INDEX4 *)l4prev( &d4->indexes, i4 )  ;
         if ( i4 == 0 )
            return 0 ;
         return (TAG4 *)l4last( &i4->tags ) ;
      }
      return tagOn ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
TAG4FILE *dfile4tagPrev( DATA4FILE *d4, TAG4FILE *tagOn )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      #ifndef N4OTHER
         INDEX4FILE *i4 ;
      #endif

      E4PARMLOW( d4, E91102 ) ;

      #ifdef N4OTHER
         return (TAG4FILE *)l4prev( &d4->tagfiles, tagOn ) ;
      #else
         if ( tagOn == 0 )
         {
            i4 = (INDEX4FILE *)l4last( &d4->indexes ) ;
            if ( i4 == 0 )
               return 0 ;
         }
         else
            i4 = tagOn->indexFile ;

         tagOn = (TAG4FILE *)l4prev( &i4->tags, tagOn ) ;
         if ( tagOn )
            return tagOn ;

         i4 = (INDEX4FILE *)l4prev( &d4->indexes, i4 ) ;
         if ( i4 == 0 )
            return 0 ;

         return (TAG4FILE *)l4last( &i4->tags ) ;
      #endif
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
void S4FUNCTION d4tagSelect( DATA4 *d4, TAG4 *t4 )
{
   C4PARMDATA( d4, E92409, -1 ); 

   #ifdef S4INDEX_OFF
      if ( t4 != 0 )
      {
         error4( d4->codeBase, e4notIndex, E92409 ) ;
         return ;
      }
   #else
      d4->tagSelected = t4 ;
   #endif

   return ;
}

int dfile4tagSelect( DATA4FILE *d4, TAG4FILE *t4 )
{
   E4PARMLOW( d4, E91102 ) ;

   #ifdef S4INDEX_OFF
      if ( t4 != 0 )
         error4( d4->c4, e4notIndex, E91102 ) ;
   #else
      #ifdef N4OTHER
         d4->tagfiles.selected = (LINK4 *)t4 ;
      #else
         #ifdef E4ANALYZE
            if ( t4 )
               if ( t4->indexFile->dataFile != d4 )
                  return error4( t4->codeBase, e4struct, E91102 ) ;
         #endif

         if ( t4 == 0 )
            d4->indexes.selected = 0 ;
         else
         {
            d4->indexes.selected = (LINK4 *)t4->indexFile ;
            t4->indexFile->tags.selected = (LINK4 *)t4 ;
         }
      #endif
   #endif

   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
TAG4 *S4FUNCTION d4tagSelected( DATA4 *d4 )
{
   #ifndef S4INDEX_OFF
      #ifdef E4PARM_HIGH
         if ( d4 == 0 )
         {
            error4( 0, e4parmNull, E92411 ) ;
            return 0 ;
         }
      #endif

      return d4->tagSelected ;
   #else
      return 0 ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
TAG4FILE *dfile4tagSelected( DATA4FILE *d4 )
{
   #ifndef S4INDEX_OFF
      #ifndef N4OTHER
         INDEX4FILE *index ;
         TAG4FILE *tag ;
      #endif

      E4PARMLOW( d4, E91102 ) ;

      #ifdef N4OTHER
         return (TAG4FILE *)d4->tagfiles.selected ;
      #else
         index = (INDEX4FILE *)d4->indexes.selected ;
         if ( index )
         {
            tag = (TAG4FILE *)index->tags.selected ;
            if ( tag )
               return tag ;
         }
         return (TAG4FILE *)0 ;
      #endif
   #else
      return (TAG4FILE *)0 ;
   #endif
}

#ifdef S4VB_DOS

TAG4 *d4tag_v( DATA4 *d4, char *name )
{
   return d4tag( d4, c4str(name) ) ;
}

#endif

int S4FUNCTION d4numTags(DATA4 *data)
{
  int numTags;

  #ifdef S4CLIPPER
  numTags = l4numNodes( &data->dataFile->tagfiles ) ;
  #else
  INDEX4FILE *i4fileOn ;
  for ( numTags = 0, i4fileOn = 0 ;; )
  {
     i4fileOn = (INDEX4FILE *)l4next( &data->dataFile->indexes, i4fileOn ) ;
     if ( i4fileOn == 0 )
        break ;
     numTags += l4numNodes( &i4fileOn->tags ) ;
  }
  #endif
  return (numTags);
}
