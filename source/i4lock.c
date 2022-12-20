/* i4lock.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4INDEX_OFF

#ifdef S4CLIPPER
   #include <time.h>
#endif

/*
#ifdef E4ANALYZE_ALL
static int blockFileCompare( B4BLOCK *b4 )
{
   B4BLOCK *deb4block ;
   long blockNo ;
   int rc ;

   deb4block = (B4BLOCK *)mem4alloc2( b4->tag->index->block_memory, b4->tag->codeBase ) ;
   if ( deb4block == 0 )
      return -1 ;

   blockNo = b4->file_block ;
   rc = 0 ;

   #ifdef S4MDX
      if ( file4readAll( &b4->tag->index->file, I4MULTIPLY * blockNo, &deb4block->n_keys, deb4block->tag->index->header.blockRw ) < 0 )
         rc = -1 ;
      else
         rc = c4memcmp( deb4block->n_keys, b4->n_keys, deb4block->tag->index->header.blockRw ) ;
   #endif
   #ifdef S4NDX
      if ( file4readAll( &b4->tag->file, I4MULTIPLY * blockNo, &deb4block->n_keys, B4BLOCK_SIZE ) < 0 )
         rc = -1 ;
      else
         rc = c4memcmp( deb4block->n_keys, b4->n_keys, B4BLOCK_SIZE ) ;
   #endif
   #ifdef S4FOX
      if ( file4readAll( &b4->tag->index->file, I4MULTIPLY * blockNo, &deb4block->header, B4BLOCK_SIZE) < 0 )
         rc = -1 ;
      else
         rc = c4memcmp( &deb4block->header, &b4->header, B4BLOCK_SIZE ) ;
   #endif
   #ifdef S4CLIPPER
      if ( file4readAll( &b4->tag->file, blockNo, &deb4block->n_keys, B4BLOCK_SIZE ) < 0 )
         rc = -1 ;
      else
         rc = c4memcmp( deb4block->n_keys, b4->n_keys, B4BLOCK_SIZE ) ;
   #endif

   mem4free( b4->tag->index->block_memory, deb4block ) ;
   return rc ;
}

static int indexVersionVerify( INDEX4 *i4 )
{
   TAG4 *tagOn ;
   B4BLOCK *blockOn ;

   for ( tagOn = 0 ;; )
   {
      tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
      if ( tagOn == 0 )
         break ;
      for ( blockOn = 0 ;; )
      {
         blockOn = (B4BLOCK *)l4next( &tagOn->blocks, blockOn ) ;
         if ( blockOn == 0 )
            break ;
         if ( blockFileCompare( blockOn ) != 0 )
            return -1 ;
      }
      for ( blockOn = 0 ;; )
      {
         blockOn = (B4BLOCK *)l4next( &tagOn->saved, blockOn ) ;
         if ( blockOn == 0 )
            break ;
         if ( blockFileCompare( blockOn ) != 0 )
            return -1 ;
      }
   }

   return 0 ;
}
#endif
*/
#ifndef S4CLIPPER
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int index4lockTest( INDEX4FILE *i4 )
{
   #ifdef S4OFF_MULTI
      return 1 ;
   #else
      if ( i4->file.lowAccessMode != OPEN4DENY_NONE )
         return 1 ;
      return ( i4->fileLocked ? 1 : 0 ) ;
   #endif
}
#endif /* S4CLIPPER */

#ifdef S4CLIPPER
int tfile4lockTest( TAG4FILE *t4 )
{
   #ifdef S4OFF_MULTI
      return 1 ;
   #else
      if ( t4->file.lowAccessMode != OPEN4DENY_NONE )
         return 1 ;
      return ( t4->fileLocked > 0 ? 1 : 0 ) ;
   #endif
}
#endif /* S4CLIPPER */

#ifdef S4CLIPPER
int i4lock( INDEX4 *index )
{
   #ifdef S4OFF_MULTI
      return 1 ;
   #else
      return dfile4lockIndex( index->data->dataFile, data4serverId( index->data ) ) ;
   #endif
}

#endif /* S4CLIPPER */

int S4FUNCTION tfile4lock( TAG4FILE *t4, const long serverId )
{
   #ifdef S4CLIPPER
      #ifndef S4OFF_MULTI
         int rc ;

         #ifdef E4PARM_LOW
            if ( t4 == 0 )
               return error4( 0, e4parmNull, E95607 ) ;
            if ( serverId == 0 )
               return error4( t4->codeBase, e4parmNull, E95607 ) ;
         #endif

         if ( error4code( t4->codeBase ) < 0 )
            return e4codeBase ;

         if ( t4->fileLocked == serverId )    /* Already locked */
            return 0 ;

         if ( t4->fileLocked != 0 )    /* Already locked by another client */
            return r4locked ;

         rc = file4lockInternal( &t4->file, L4LOCK_POS, 0, L4LOCK_POS, 0 ) ;
         if ( rc )
            return rc ;

         #ifndef S4OPTIMIZE_OFF
            file4refresh( &t4->file ) ;   /* make sure all up to date */
         #endif

         if ( file4longGetLo( file4lenLow( &t4->file ) ) != 0 )  /* if the file exists (not being created), update header */
            if ( tfile4versionCheck( t4, 1, 1 ) < 0 )
               return -1 ;

         t4->fileLocked = serverId ;  /* this flag must be set after the version_check is complete */
      #endif /* S4OFF_MULTI */

      return 0 ;
   #else
      return -1 ;
   #endif
}

int S4FUNCTION tfile4unlock( TAG4FILE *t4, const long serverId )
{
   #ifdef S4CLIPPER
      #ifndef S4OFF_MULTI
        int rc ;
        #ifdef E4PARM_LOW
           if ( t4 == 0 )
              return error4( 0, e4parm, E95606 ) ;
        #endif

        if ( t4->fileLocked == 0 )
           return 0 ;

        if ( t4->fileLocked == serverId || ( t4->fileLocked != 0 && serverId == 0 ) )
        {
           #ifndef S4OFF_WRITE
              if ( tfile4update( t4 ) < 0 )
                 return -1 ;
           #endif

           rc = file4unlockInternal( &t4->file, L4LOCK_POS, 0, L4LOCK_POS, 0 ) ;
           if ( rc < 0 )
              return rc ;
           t4->fileLocked = 0 ;
           return rc ;
        }
      #endif
      return 0 ;
   #else
     return -1 ;
   #endif
}

#ifndef S4CLIPPER
#ifndef S4OFF_MULTI
static INDEX4 *code4index( CODE4 *cb, const long id, const char *name )
{
   DATA4 *data ;
   INDEX4 *index ;
   LIST4 *list ;

   if ( cb == 0 || id == 0 )
   {
      error4( cb, e4parmNull, E95601 ) ;
      return 0 ;
   }

   #ifdef S4MEM_DBF
      if ( cb->server == 0 )
      {
         return mem4index() ;
      }
      else
   #endif

      list = tran4dataList( &cb->c4trans.trans ) ;
      for( data = 0 ;; )
      {
         data = (DATA4 *)l4next( list, data ) ;
         if ( data == 0 )
            break ;
         if ( data4serverId( data ) == id )
         {
            for( index = 0 ;; )
            {
               index = (INDEX4 *)l4next( &data->indexes, index ) ;
               if ( index == 0 )
               {
                  return 0 ;
               }
               if ( strcmp( name, index->indexFile->file.name ) == 0 )
               {
                  return index ;
               }
            }
         }
      }

   #ifndef S4OFF_CATALOG
      if ( cb->catalog != 0 )
      {
         data = cb->catalog->data ;
         if ( data != 0 )
            if ( data4serverId( data ) == id )   /* maybe the catalog file */
               for( index = 0 ;; )
               {
                  index = (INDEX4 *)l4next( &data->indexes, index ) ;
                  if ( index == 0 )
                     return 0 ;
                  if ( strcmp( name, index->indexFile->file.name ) == 0 )
                     return index ;
               }
      }
   #endif

   return 0 ;
}
#endif  /* S4OFF_MULTI */

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int index4lock( INDEX4FILE *i4, const long serverId )
{
   #ifndef S4OFF_MULTI
      int rc ;
      INDEX4 *index ;

      #ifdef E4PARM_LOW
         if ( i4 == 0 )
            return error4( 0, e4parmNull, E95602 ) ;
         if ( serverId == 0 )
            return error4( i4->codeBase, e4parmNull, E95602 ) ;
      #endif

      if ( error4code( i4->codeBase ) < 0 )
         return e4codeBase ;

      if ( i4->fileLocked == serverId )    /* Already locked */
         return 0 ;

      if ( i4->fileLocked != 0 )    /* Already locked by another client */
         return r4locked ;

      #ifdef S4MDX
         rc = file4lockInternal( &i4->file, L4LOCK_POS - 1L, 0, 1L, 0 ) ;
      #endif
      #ifdef S4FOX
         rc = file4lockInternal( &i4->file, L4LOCK_POS, i4->codeBase->largeFileOffset, 1L, 0 ) ;
      #endif

      if ( rc )
         return rc ;

      #ifndef S4OPTIMIZE_OFF
         file4refresh( &i4->file ) ;   /* make sure all up to date */
      #endif

      index = code4index( i4->codeBase, serverId, i4->file.name ) ;

      if ( index != 0 && file4longGetLo( file4lenLow( &i4->file ) ) != 0 )  /* if the file exists (not being created), update header */
         if ( i4versionCheck( index, 1, 1 ) < 0 )
         {
            #ifdef S4MDX
               file4unlockInternal( &i4->file, L4LOCK_POS - 1L, 0, 1L, 0 ) ;
            #endif
            #ifdef S4FOX
               file4unlockInternal( &i4->file, L4LOCK_POS, i4->codeBase->largeFileOffset, 1L, 0 ) ;
            #endif
            return -1 ;
         }

      #ifdef S4FOX
         i4->eof = file4longGetLo( file4lenLow( &i4->file ) ) ;
      #endif
      i4->fileLocked = serverId ;  /* this flag must be set after the version_check is complete */

   #endif

   return 0 ;
}

int i4unlock( INDEX4 *i4 )
{
   return index4unlock( i4->indexFile, data4serverId( i4->data ) ) ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int index4unlock( INDEX4FILE *i4, const long serverId )
{
   #ifndef S4OFF_MULTI
      int rc ;

      #ifdef E4PARM_LOW
         if ( i4 == 0 )
            return error4( 0, e4parm, E95603 ) ;
      #endif

      if ( i4->fileLocked == 0 )
         return 0 ;

      if ( i4->fileLocked == serverId || ( i4->fileLocked != 0 && serverId == 0 ) )
      {
         #ifndef S4OFF_WRITE
            if ( index4update( i4 ) < 0 )
               return -1 ;
         #endif

         #ifdef S4MDX
            rc = file4unlockInternal( &i4->file, L4LOCK_POS - 1L, 0, 1L, 0 ) ;
         #else
            #ifdef S4FOX
               rc = file4unlockInternal( &i4->file, L4LOCK_POS, i4->codeBase->largeFileOffset, 1L, 0 ) ;
            #else
               #error index format missing
            #endif
         #endif
         if ( rc < 0 )
            return rc ;
         i4->fileLocked = 0 ;
         return 0 ;
      }
   #endif
   return 0 ;
}
#endif /* S4CLIPPER */

#ifdef S4CLIPPER
/* unlocks all the corresponding tag files */
int i4unlock( INDEX4 *i4 )
{
   #ifdef S4OFF_MULTI
      return 0 ;
   #else
      int rc, save_rc = 0, setUnlock ;
      TAG4 *tagOn ;
      TAG4FILE *tagFileOn ;

      #ifdef S4VBASIC
         if ( c4parm_check( i4, 0, E95605 ) )
            return -1 ;
      #endif

      #ifdef E4PARM_HIGH
         if ( i4 == 0 )
            return error4( i4->codeBase, e4parm, E95605 ) ;
      #endif

      if ( i4->data->dataFile->indexLocked )
      {
         #ifndef S4OFF_WRITE
            if ( i4update( i4 ) < 0 )
               return -1 ;
         #endif

         for( tagOn = 0 ;; )
         {
            tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
            if ( tagOn == 0 )
               break ;
            rc = 0 ;
            if ( tagOn->tagFile->fileLocked == data4serverId( i4->data ) )
            {
               rc = file4unlockInternal( &tagOn->tagFile->file, L4LOCK_POS, 0, L4LOCK_POS, 0 ) ;
               if ( rc )
                  save_rc = rc ;
               else
                  tagOn->tagFile->fileLocked = 0 ;
            }
         }

         if ( save_rc == 0 )
         {
            setUnlock = 1 ;
            for ( tagFileOn = 0 ;; )
            {
               tagFileOn = (TAG4FILE *)l4next( &i4->data->dataFile->tagfiles, tagFileOn ) ;
               if ( tagFileOn == 0 )
                  break ;
               if ( tagFileOn->fileLocked != 0 )  /* positive value gives serverId */
               {
                  setUnlock = 0 ;
                  break ;
               }
            }
            if ( setUnlock == 1 )
               i4->data->dataFile->indexLocked = 0 ;
         }
      }

      return save_rc ;
   #endif
}

#endif  /* S4CLIPPER */

#endif  /* S4INDEX_OFF */
