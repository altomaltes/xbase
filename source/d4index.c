/* d4index.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4freeBlocks( DATA4 *data )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
         int rc ;
         TAG4 *tagOn ;

      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parmNull, E93001 ) ;
      #endif


         rc = 0 ;
         for( tagOn = 0 ;; )
         {
            tagOn = (TAG4 *)d4tagNext( data, tagOn ) ;
            if ( tagOn == 0 )
               return rc ;
            if ( tfile4freeAll( tagOn->tagFile ) < 0 )
               rc = -1 ;
         }
   #endif
}

/* should compare full name, adding extension if required */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
INDEX4 *S4FUNCTION d4index( DATA4 *data, const char *indexName )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      char *current ;
      char indexLookup[258], ext[4] ;
         char indexLookup2[258] ;
      INDEX4 *indexOn ;
      unsigned int i, extIndex ;
      int doAlias, hasExt, hasPath ;

      #ifdef S4VBASIC
         if ( c4parm_check( data, 2, E93002 ) )
            return 0 ;
      #endif  /* S4VBASIC */

      #ifdef E4PARM_HIGH
         if ( data == 0 )
         {
            error4( 0, e4parmNull, E93002 ) ;
            return 0 ;
         }
      #endif

      if ( indexName == 0 )
         doAlias = 1 ;
      else
      {
         if ( indexName[0] == 0 )
            doAlias = 1 ;
         else
            doAlias = 0 ;
      }

      if ( doAlias == 1 )
         u4ncpy( indexLookup, d4alias( data ), sizeof(indexLookup) ) ;
      else
         u4ncpy( indexLookup, indexName, sizeof(indexLookup) ) ;

      #ifndef S4CASE_SEN
         c4upper( indexLookup ) ;
      #endif
      u4nameExt( indexLookup, sizeof(indexLookup), code4indexExtension( data->codeBase ), 0 ) ;

      for( hasPath = 0, i = 0, hasExt = 0 ; i < sizeof( indexLookup ) ; i++ )
      {
         switch( indexLookup[i] )
         {
            case 0:  /* done */
               i = sizeof( indexLookup ) ;
               break ;
         #ifndef S4MACINTOSH
            case ':':
         #endif
            case S4DIR:
               i = sizeof( indexLookup ) ;
               hasPath = 1 ;
               break ;
            case '.':
               hasExt = 1 ;
               memset( ext, 0, sizeof( ext ) ) ;
               extIndex = 0 ;
               break ;
            default:
               if ( hasExt == 1 )  /* part of extension, so copy */
                  if ( extIndex < 3 )  /* ignore rest */
                  {
                     ext[extIndex] = indexLookup[i] ;
                     extIndex++ ;
                  }
               break ;
         }
      }

      for( indexOn = 0 ;; )
      {
         indexOn = (INDEX4 *)l4next( &data->indexes, indexOn) ;
         if ( indexOn == 0 )
            return 0 ;
            current = indexOn->accessName ;
            if ( current[0] == 0 )  /* use data file name */
            {
               u4namePiece( indexLookup2, sizeof(indexLookup2), data->dataFile->file.name, hasPath, 0 ) ;
               current = indexLookup2 ;
            }
         if ( !strcmp( current, indexLookup ) )    /* check out data->alias? */
            return indexOn ;
         if ( doAlias == 1 )   /* check with just alias (no extension) */
         {
            if ( !strcmp( current, d4alias( data ) ) )
               return indexOn ;
         }
         else     /* also check in case of extension specific */
         {
            /* do by not examining the extension (last 4 bytes) of copied memory
               this is to avoid the non-upper-case possible input scenario */

            /* but first ensure that the extension is normalized (if it exists) */
            /* 03/25/96 AS */
            if ( hasExt )
            {
               #ifndef S4CASE_SEN
                  c4upper( ext ) ;
               #endif
               if ( memcmp( code4indexExtension( data->codeBase ), ext, 3 ) != 0 )
                  continue ;   /* means extension is different, so must include in examination */
            }

            if ( !memcmp( current, indexLookup, strlen( indexLookup ) - 4 ) )
               if ( current[strlen(indexLookup) - 4] == '.' || current[strlen(indexLookup) - 4] == '\0' )
                  return indexOn ;
         }
      }
   #endif
}

#ifndef N4OTHER
#ifdef P4ARGS_USED
   #pragma argsused
#endif
INDEX4FILE *dfile4index( DATA4FILE *data, const char *indexName )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      char indexLookup[258] ;
      INDEX4FILE *indexOn ;

      #ifdef E4PARM_LOW
         if ( data == 0 || indexName == 0 )
         {
            error4( 0, e4parmNull, E91102 ) ;
            return 0 ;
         }
      #endif

         u4nameCurrent( indexLookup, sizeof( indexLookup ), indexName ) ;
      #ifndef S4CASE_SEN
         c4upper( indexLookup ) ;
      #endif
      for( indexOn = 0 ;; )
      {
         indexOn = (INDEX4FILE *)l4next( &data->indexes, indexOn ) ;
         if ( indexOn == 0 )
            return 0 ;
            if ( !strcmp( indexLookup, indexOn->file.name ) )    /* check out data->alias? */
               return indexOn ;
      }
   #endif
}
#endif

#ifndef S4OFF_WRITE
int S4FUNCTION d4reindex( DATA4 *data )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      int rc ;
      CODE4 *c4 ;
         INDEX4 *indexOn ;
         int oldSchemaCreate ;
         #ifdef S4LOW_MEMORY
            #ifndef S4OFF_OPTIMIZE
               int hasOpt ;
            #endif
         #endif

      #ifdef S4VBASIC
         if ( c4parm_check( data, 2, E93004 ) )
            return -1 ;
      #endif  /* S4VBASIC */

      #ifdef E4PARM_HIGH
         if ( data == 0 )
            return error4( 0, e4parmNull, E93004 ) ;
      #endif

      c4 = data->codeBase ;
      if ( error4code( c4 ) < 0 )
         return e4codeBase ;

      #ifndef S4OFF_WRITE
         rc = d4updateRecord( data, 0 ) ;
         if ( rc )
            return rc ;
      #endif

      if ( data->readOnly == 1 )
         return error4describe( c4, e4write, E80606, d4alias( data ), 0, 0 ) ;

      rc = 0 ;

         #ifdef S4LOW_MEMORY
            #ifndef S4OFF_OPTIMIZE
               hasOpt = c4->hasOpt && c4->opt.numBuffers ;
               if ( hasOpt )
                  code4optSuspend( c4 ) ;
            #endif
         #endif

         oldSchemaCreate = c4->oledbSchemaCreate ;
         c4->oledbSchemaCreate = 1 ;   /* for d4reindex() don't check lengths.  Only check on create */
         for ( indexOn = 0 ;; )
         {
            indexOn = (INDEX4 *)l4next( &data->indexes, indexOn ) ;
            if ( indexOn == 0 )
               break ;
            rc = i4reindex( indexOn ) ;
            if ( rc != 0 )  /* error or r4unique */
               break ;
         }
         c4->oledbSchemaCreate = oldSchemaCreate ;

         #ifdef S4LOW_MEMORY
            #ifndef S4OFF_OPTIMIZE
               if ( hasOpt )
                  code4optRestart( c4 ) ;
            #endif
         #endif
         return rc ;
   #endif
}
#endif  /* S4OFF_WRITE */

#ifdef S4VB_DOS

INDEX4 *d4index_v( DATA4 *d4, char *indexName )
{
   return d4index( d4, c4str(indexName) ) ;
}

#endif
