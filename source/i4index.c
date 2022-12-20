/* i4index.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4INDEX_OFF

#ifdef S4CLIPPER
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int i4setup( CODE4 *c4, DATA4 *d4, const char *name, int autoOpened )
{
   TAG4FILE *tagFile ;
   TAG4 *tag ;
   DATA4FILE *dfile ;
   INDEX4 *i4 = 0 ;

   dfile = d4->dataFile ;

   for ( tagFile = 0 ;; )
   {
      tagFile = (TAG4FILE *)l4next( &dfile->tagfiles, tagFile ) ;
      if ( tagFile == 0 )
         break ;
      if ( i4 == 0 )
      {
         i4 = (INDEX4 *)mem4alloc( c4->indexMemory ) ;
         i4->codeBase = c4 ;
         i4->data = d4 ;
         l4add( &d4->indexes, i4 ) ;
         u4ncpy( i4->accessName, name, sizeof( i4->accessName ) - 1 ) ;
      }
      tag = (TAG4 *)mem4alloc( c4->tagMemory ) ;
      if ( tag == 0 )
         return 0 ;
      tag->tagFile = tagFile ;
      tag->tagFile->userCount++ ;
      tag->index = i4 ;
      l4add( &i4->tags, tag ) ;
   }

   return 0 ;
}
#endif

int S4FUNCTION i4close( INDEX4 *i4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E91701 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( i4 == 0 )
         return error4( 0, e4parmNull, E91701 ) ;
      if ( i4->codeBase == 0 )
         return error4( 0, e4parm, E91701 ) ;
      #ifndef S4CLIPPER
            /* users are not allowed to close production index files */
            if ( index4isProduction( i4->indexFile ) )
               return error4( i4->codeBase, e4close, E81721 ) ;
      #endif
   #endif

   return i4closeLow( i4 ) ;
}

const char *S4FUNCTION t4fileName( TAG4 *t4 )
{
   #ifdef E4PARM_HIGH
      if ( t4 == 0 )
      {
         error4( 0, e4parmNull, E91720 ) ;
         return 0 ;
      }
   #endif

      #ifdef S4CLIPPER
         return t4->tagFile->file.name ;
      #else
         return i4fileName( t4->index ) ;
      #endif
}

const char *S4FUNCTION i4fileName( INDEX4 *i4 )
{
      #ifdef S4CLIPPER
         TAG4FILE *tag ;
      #endif

   #ifdef E4PARM_HIGH
      if ( i4 == 0 )
      {
         error4( 0, e4parmNull, E91720 ) ;
         return 0 ;
      }
   #endif

      #ifdef S4CLIPPER
         if ( i4->path != 0 )
            return i4->path ;
         if ( l4numNodes( &i4->tags ) == 1 )
         {
            tag = (TAG4FILE *)(((TAG4 *)(i4->tags.lastNode))->tagFile) ;
            return tag->alias ;
         }
         else
            return d4fileName( i4->data ) ;
      #else
         return i4->indexFile->file.name ;
      #endif /* S4CLIPPER */
}

#ifndef S4CLIPPER

int S4FUNCTION i4closeLow( INDEX4 *i4 )
{
   int rc, finalRc ;
   TAG4 *tagOn ;
   CODE4 *c4 ;

   c4 = i4->codeBase ;

   #ifndef S4OFF_TRAN
      if ( i4->isValid == 1 ) /* if invalid (failed create/open) then allow close */
         if ( code4transEnabled( c4 ) )
            if ( code4trans( c4 )->currentTranStatus == r4active )  /* disallow on current active only */
               return error4( c4, e4transViolation, E81522 ) ;
   #endif

   finalRc = 0 ;

   #ifndef S4OFF_WRITE
      #ifndef S4OFF_TRAN
         if ( i4->isValid == 1 ) /* if invalid (failed create/open) then allow close */
      #endif
      if ( i4->data )
         if ( d4update( i4->data ) < 0 )
            finalRc = error4set( c4, 0 ) ;
   #endif

   for( ;; )
   {
      tagOn = (TAG4 *)l4pop( &i4->tags ) ;
      if ( tagOn == 0 )
         break ;
      #ifndef S4OFF_TRAN
         #ifdef E4ANALYZE
            if ( tagOn->removedKeys.nLink != 0 )
               return error4( c4, e4info, E91701 ) ;
         #endif
      #endif
      if ( i4->data->tagSelected == tagOn )  /* can't have a tag selected from a closed index */
         i4->data->tagSelected = 0 ;
      /* 06/10/96 AS expression data mismatch problem --> if same data4 re-allocated */
      if ( tagOn->tagFile->expr != NULL ) /* could be NULL if improper open or if unsupported expression (read-only) */
         if ( tagOn->tagFile->expr->data == i4->data )
            tagOn->tagFile->expr->data = 0 ;
      mem4free( c4->tagMemory, tagOn ) ;
      tagOn = 0 ;
   }
   if ( i4->indexFile != 0 )
   {
      /* 05/30/96 AS --> t4code4.c failure if creating production index file with keepopen true (mdx c/s) */
      index4unlock( i4->indexFile, data4serverId( i4->data ) ) ;
      rc = index4close( i4->indexFile ) ;
      if ( rc != 0 )
         finalRc = rc ;
   }
   if ( i4->link.n != 0 )
      l4remove( &i4->data->indexes, i4 ) ;
   mem4free( c4->indexMemory, i4 ) ;
   i4 = 0 ;

   if ( finalRc != 0 )
   {
      error4set( c4, finalRc ) ;
      return finalRc ;
   }

   return 0 ;
}

int index4close( INDEX4FILE *i4 )
{
   int finalRc, isProduction ;
   CODE4 *c4 ;
   TAG4FILE *tagOn ;
   #ifndef S4SINGLE
      int saveAttempts ;
   #endif

   #ifdef E4PARM_LOW
      if ( i4 == 0 )
         return error4( 0, e4parmNull, E91702 ) ;
   #endif
   #ifdef E4ANALYZE
      if ( i4->userCount < 0 )
         return error4( i4->codeBase, e4struct, E81702 ) ;
   #endif

   isProduction = index4isProduction( i4 ) ;
   if ( i4->isValid == 0 || ( i4->userCount <= 1 && isProduction == 0 ) || ( i4->userCount == 0 && i4->dataFile->userCount == 0 ) )
   {
      c4 = i4->codeBase ;

      finalRc = error4code( c4 ) ;
      #ifndef S4SINGLE
         saveAttempts = c4->lockAttempts ;
         c4->lockAttempts = WAIT4EVER ;
      #endif

      #ifndef S4OFF_WRITE
         if ( index4update( i4 ) < 0 )
            finalRc = error4set( c4, 0 ) ;
      #endif

      #ifndef S4SINGLE
         if ( index4unlock( i4, 0UL ) < 0 )
            finalRc = error4set( c4, 0 ) ;
      #endif

      #ifdef S4FOX
         if ( i4->tagIndex )
            if ( i4->tagIndex->header.typeCode >= 64 )  /* compound index */
      #endif
      for( ;; )
      {
         tagOn = (TAG4FILE *)l4pop( &i4->tags ) ;
         if ( tagOn == 0 )
            break ;
         if ( tfile4freeAll( tagOn ) < 0 )
         {
            finalRc = error4set( c4, 0 ) ;
            break ;
         }
         expr4free( tagOn->expr ) ;
         expr4free( tagOn->filter ) ;
         #ifdef S4FOX
            mem4release( tagOn->builtKeyMemory ) ;
         #endif
         mem4free( c4->tagFileMemory, tagOn ) ;
         tagOn = 0 ;
      }

      #ifdef S4FOX
         if ( i4->tagIndex != 0 )
         {
            if ( tfile4freeAll( i4->tagIndex ) < 0 )
               finalRc = error4set( c4, 0 ) ;
            else
            {
               expr4free( i4->tagIndex->expr ) ;
               expr4free( i4->tagIndex->filter ) ;
               #ifdef S4FOX
                  mem4release( i4->tagIndex->builtKeyMemory ) ;
               #endif
               mem4free( c4->tagFileMemory, i4->tagIndex ) ;
               i4->tagIndex = 0 ;
            }
         }
      #endif

      mem4release( i4->blockMemory ) ;

      if ( file4openTest( &i4->file ) )
      {
         if ( i4->dataFile )
            l4remove( &i4->dataFile->indexes, i4 ) ;
         #ifndef S4CLIPPER
            if ( c4->doRemove == 1 )
               i4->file.isTemp = 1 ;
         #endif
         if ( file4close( &i4->file ) < 0 )
            finalRc = error4set( c4, 0 ) ;
      }

      mem4free( c4->index4fileMemory, i4 ) ;
      i4 = 0 ;
      #ifndef S4SINGLE
         c4->lockAttempts = saveAttempts ;
      #endif
      error4set( c4, (short)finalRc ) ;
      return finalRc ;
   }
   else
   {
      i4->userCount-- ;
      return 0 ;
   }
}

#ifndef S4OFF_WRITE
long index4extend( INDEX4FILE *i4 )
{
   long oldEof ;
   unsigned len ;
   CODE4 *c4 ;
   FILE4LONG pos ;
   #ifdef S4FOX
      TAG4FILE *tagIndex ;
   #endif

   #ifdef E4PARM_LOW
      if ( i4 == 0 )
         return error4( 0, e4parmNull, E91703 ) ;
   #endif

   c4 = i4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   #ifdef S4FOX
      tagIndex= i4->tagIndex ;
      #ifdef E4ANALYZE
         if ( tagIndex->header.version == i4->versionOld )
            return error4( c4, e4index, E91703 ) ;
      #endif

      oldEof = tagIndex->header.freeList ;

      if( oldEof == 0L )  /* case where no free list */
      {
         oldEof = i4->eof ;
         i4->eof += B4BLOCK_SIZE ;
      }
      else
      {
         file4longAssign( pos, tagIndex->header.freeList * I4MULTIPLY, 0 ) ;
         len = file4readInternal( &i4->file, pos, (char *)&tagIndex->header.freeList, sizeof( tagIndex->header.freeList ) ) ;

         #ifdef WORDS_BIGENDIAN
            tagIndex->header.freeList = x4reverseLong( &tagIndex->header.freeList ) ;
         #endif

         if ( error4code( c4 ) < 0 )
            return -1 ;

         switch( len )
         {
            case 0:
               #ifdef E4ANALYZE
                  return error4( c4, e4index, E91703 ) ;
               #else  /* else fix up */
                  tagIndex->header.freeList = 0L ;
                  oldEof = i4->eof ;
                  i4->eof += B4BLOCK_SIZE ;
                  break ;
               #endif
            case sizeof(tagIndex->header.freeList):
               break ;
            default:
               return file4readError( &i4->file, pos, sizeof(tagIndex->header.freeList), "index4extend" ) ;
         }
      }
   #else
      oldEof = i4->header.freeList ;

      if( i4->header.freeList == 0L )  /* case where no free list */
      {
         oldEof = i4->header.eof ;
         i4->header.eof = i4->header.eof + i4->header.blockRw/I4MULTIPLY ;
      }
      else
      {
         file4longAssign( pos, i4->header.freeList*I4MULTIPLY + sizeof(S4LONG), 0 ) ;
         len = file4readInternal( &i4->file, pos, (char *)&i4->header.freeList, sizeof(i4->header.freeList)) ;

         #ifdef WORDS_BIGENDIAN
            i4->header.freeList = x4reverseLong( (void *)&i4->header.freeList ) ;
         #endif

         if ( error4code( c4 ) < 0 )
            return -1 ;

         switch( len )
         {
            case 0:
               #ifdef E4ANALYZE
                  return error4( c4, e4index, E91703 ) ;
               #else   /* try to fix up */
                  i4->header.freeList = 0L ;
                  oldEof = i4->header.eof ;
                  i4->header.eof = i4->header.eof + i4->header.blockRw/I4MULTIPLY ;
                  break ;
               #endif
            case sizeof(i4->header.freeList):
               break ;
            default:
               return file4readError( &i4->file, pos, sizeof(i4->header.freeList), "index4extend" ) ;
         }
      }
   #endif /* S4FOX */

   return oldEof ;
}

int index4flush( INDEX4FILE *i4 )
{
   int rc ;

   #ifdef E4PARM_LOW
      if ( i4 == 0 )
         return error4( 0, e4parmNull, E91704 ) ;
   #endif

   rc = index4update( i4 ) ;
   if ( file4flush( &i4->file ) < 0 )
      rc = -1 ;

   return rc ;
}

int index4update( INDEX4FILE *i4 )
{
   TAG4FILE *tagOn ;
   int rc ;

   #ifdef E4PARM_LOW
      if ( i4 == 0 )
         return error4( 0, e4parmNull, E91705 ) ;
   #endif

   if ( error4code( i4->codeBase ) < 0 )
      return e4codeBase ;

   #ifndef S4SINGLE
      if ( i4->fileLocked != 0 )
      {
   #endif
   rc = index4updateHeader( i4 ) ;
   if ( rc < 0 )
      return error4stack( i4->codeBase, (short)rc, E91705 ) ;
   #ifdef S4FOX
      rc = tfile4update(i4->tagIndex) ;
      if ( rc < 0 )
         return error4stack( i4->codeBase, (short)rc, E91705 ) ;
      if ( i4->tagIndex->header.typeCode >= 64 )  /* compound index */
   #endif

   for ( tagOn = 0 ;; )
   {
      tagOn = (TAG4FILE *)l4next( &i4->tags, tagOn ) ;
      if ( tagOn == 0 )
         break ;
      rc = tfile4update( tagOn ) ;
      if ( rc < 0 )
         return error4stack( i4->codeBase, (short)rc, E91705 ) ;
      tagOn->header.root = -1L ;
   }

   #ifndef S4SINGLE
      }
   #endif

   return 0 ;
}
#endif  /* S4OFF_WRITE */


INDEX4 *S4FUNCTION i4open( DATA4 *d4, const char *fileName )
{
   CODE4  *c4 ;
   INDEX4 *i4 ;
      TAG4 *tag ;
      TAG4FILE *tagFile ;

   #ifdef E4PARM_HIGH
      if ( d4 == 0 )
      {
         error4( 0, e4parmNull, E91706 ) ;
         return 0 ;
      }
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E91706 ) )
         return 0 ;
   #endif

   c4 = d4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return 0 ;

      if ( c4->indexMemory == 0 )
      {
         c4->indexMemory = mem4create( c4, c4->memStartIndex, sizeof(INDEX4), c4->memExpandIndex, 0 ) ;
         if ( c4->indexMemory == 0 )
            return 0 ;
      }

      i4 = (INDEX4 *)mem4alloc( c4->indexMemory ) ;
      if ( i4 == 0 )
      {
         #ifdef ERROR4STACK
            error4stack( c4, e4memory, E91706 ) ;
         #endif
         return 0 ;
      }

      i4->data = d4 ;
      i4->codeBase = c4 ;

      if ( fileName != 0 )
      {
         #ifdef E4MISC
            if ( strlen( fileName ) > sizeof( i4->accessName ) )
            {
               error4describe( c4, e4name, E91706, fileName, 0, 0 ) ;
               i4closeLow( i4 ) ;
               return 0 ;
            }
         #endif
         u4ncpy( i4->accessName, fileName, sizeof( i4->accessName ) - 1 ) ;
      }
         else
         {
            u4namePiece( i4->accessName, sizeof( i4->accessName ), d4->alias, 0, 0 ) ;
         }
         #ifndef S4CASE_SEN
            c4upper( i4->accessName ) ;
         #endif

      i4->indexFile = index4open( d4, fileName, i4 ) ;
      if ( i4->indexFile == 0 )
      {
         i4closeLow( i4 ) ;
         return 0 ;
      }

      l4add( &d4->indexes, i4 ) ;

      for ( tagFile = 0 ;; )
      {
         tagFile = (TAG4FILE *)l4next( &i4->indexFile->tags, tagFile ) ;
         if ( tagFile == 0 )
            break ;

         tag = (TAG4 *)mem4alloc( c4->tagMemory ) ;
         if ( tag == 0 )
         {
            i4closeLow( i4 ) ;
            error4( c4, e4memory, E91706 ) ;
            return 0 ;
         }
         tag->index = i4 ;
         tag->tagFile = tagFile ;

         #ifdef S4FOX
            if ( tag->tagFile->header.typeCode & 0x04 )  /* r4/e4 candidate */
            {
               if ( c4->errDefaultUnique == e4unique )
                  tag->errUnique = e4candidate ;
               else
                  tag->errUnique = r4candidate ;
            }
            else
               if ( tag->tagFile->header.typeCode & 0x01 )
         #else
            if ( tag->tagFile->header.unique )
         #endif
               tag->errUnique = c4->errDefaultUnique ;


         l4add( &i4->tags, tag ) ;
      }

      #ifndef S4OFF_TRAN
         i4->isValid = 1 ;
      #endif
      return i4 ;
}

INDEX4FILE *index4open( DATA4 *d4, const char *fileName, INDEX4 *index )
{
   CODE4  *c4 ;
   int    rc ;
   INDEX4FILE *i4 ;
   char buf[258] ;
   TAG4FILE *tagFile ;
   DATA4FILE *dfile ;
   #ifdef E4MISC
      INDEX4FILE *i4ptr ;
   #endif
   #ifdef S4FOX
      B4BLOCK *b4 ;
      int oldErrExpr ;
      #ifndef S4SINGLE
         long oldFileLock ;
      #endif
   #else
      T4DESC tagInfo[47] ;
      int iTag ;
      FILE4LONG pos ;
   #endif
      INDEX4 *indexLoop ;

   #ifdef E4PARM_LOW
      if ( d4 == 0 )
      {
         error4( 0, e4parmNull, E91707 ) ;
         return 0 ;
      }
         if ( index == 0 )
         {
            error4( 0, e4parmNull, E91707 ) ;
            return 0 ;
         }
   #endif

   c4 = d4->codeBase ;

   dfile = d4->dataFile ;
   #ifdef E4ANALYZE
      if ( code4indexExtension( c4 ) == 0 )
      {
         error4( c4, e4struct, E91707 ) ;
         return 0 ;
      }
   #endif
   if ( fileName == 0 )
      u4ncpy( (char *)buf, dfile->file.name, sizeof( buf ) - 1 ) ;
   else
   {
      rc = u4nameCurrent( (char *)buf, sizeof( buf ), fileName ) ;
      if ( rc < 0 )
      {
         error4( c4, rc, E94509 ) ;  /* from u4nameCurrent */
         return 0 ;
      }
   }
   u4nameExt( (char *)buf, sizeof( buf ), code4indexExtension( c4 ), ( fileName == 0 ? 1 : 0 ) ) ;
   #ifndef S4CASE_SEN
      c4upper( (char *)buf ) ;
   #endif
   i4 = dfile4index( dfile, (char *)buf ) ;
   if ( i4 != 0 )
   {
         /* allowed if current data4 does not have a pointer to index */
         for ( indexLoop = 0 ;; )
         {
            indexLoop = (INDEX4 *)l4next( &d4->indexes, indexLoop ) ;
            if ( indexLoop == 0 )
               break ;
            if ( indexLoop->indexFile == i4 )
            {
               error4( c4, e4instance, E91707 ) ;
               return 0 ;
            }
         }
      i4->userCount++ ;
      i4->isValid = 1 ;
      return i4 ;
   }

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E91707 ) )
         return 0 ;
   #endif

   c4 = dfile->c4 ;
   if ( error4code( c4 ) < 0 )
      return 0 ;

   if ( c4->index4fileMemory == 0 )
      c4->index4fileMemory = mem4create( c4, c4->memStartIndexFile, sizeof(INDEX4FILE), c4->memExpandIndexFile, 0 ) ;
   if ( c4->index4fileMemory == 0 )
       return 0 ;

   if ( c4->tagMemory == 0 )
   {
      c4->tagMemory = mem4create( c4, c4->memStartTag, sizeof(TAG4), c4->memExpandTag, 0 ) ;
      if ( c4->tagMemory == 0 )
         return 0 ;
   }

   if ( c4->tagFileMemory == 0 )
   {
      c4->tagFileMemory = mem4create( c4, c4->memStartTagFile, sizeof(TAG4FILE), c4->memExpandTagFile, 0 ) ;
      if ( c4->tagFileMemory == 0 )
         return 0 ;
   }

   i4 = (INDEX4FILE *)mem4alloc( c4->index4fileMemory ) ;
   if ( i4 == 0 )
   {
      #ifdef ERROR4STACK
         error4stack( c4, e4memory, E91707 ) ;
      #endif
      return 0 ;
   }

   /* next line for this function duration only since the return of this
      function is assigned into index->indexFile but upper level.
      Therefore, we still must clean up ourselves if failure */
   index->indexFile = i4 ;
   i4->dataFile = dfile ;
   i4->codeBase = c4 ;
   #ifdef S4FOX
      #ifdef E4MISC
         for ( i4ptr = 0 ;; )
         {
            i4ptr = (INDEX4FILE *)l4next( &dfile->indexes, i4ptr ) ;
            if ( i4ptr == 0 )
               break ;
            if ( !c4memcmp( i4ptr->file.name, buf, (size_t)strlen( (char *)buf ) ) )
            {
               mem4free( c4->index4fileMemory, i4 ) ;
               index->indexFile = 0 ;
               error4( c4, e4parm, E81703 ) ;
               return 0 ;
            }
         }
      #endif

      rc = file4open( &i4->file, c4, (char *)buf, 1 ) ;
      if ( rc )
      {
         index->indexFile = 0 ;
         mem4free( c4->index4fileMemory, i4 ) ;
         return 0 ;
      }

      i4->eof = file4longGetLo( file4lenLow( &i4->file ) ) ;

      i4->tagIndex = (TAG4FILE *)mem4alloc( c4->tagFileMemory ) ;
      if ( i4->tagIndex == 0 )
      {
         file4close( &i4->file ) ;
         index->indexFile = 0 ;
         mem4free( c4->index4fileMemory, i4 ) ;
         error4( c4, e4memory, E91707 ) ;
         return 0 ;
      }

      l4add( &dfile->indexes, i4 ) ;
      if ( fileName == 0 )
      {
         if ( tfile4init( i4->tagIndex, index, 0L, (unsigned char *)"" ) < 0 )
         {
            index4close( i4 ) ;
            return 0 ;
         }
      }
      else
      {
         u4namePiece( (char *)buf, 258, fileName, 0, 0 ) ;  /* get the tagName based on the fileName */
         if ( tfile4init( i4->tagIndex, index, 0L, (unsigned char *)buf) < 0 )
         {
            index4close( i4 ) ;
            return 0 ;
         }
      }
      if ( tfile4setCodePage( i4->tagIndex, d4->codePage ) < 0 )
      {
         error4( c4, e4index, E91642 ) ;
         index4close( i4 ) ;
         return 0 ;
      }

      /* Perform some checks */
      if ( i4->tagIndex->header.root <= 0L || i4->tagIndex->header.typeCode < 32 )
      {
         #ifdef E4ANALYZE_ALL
            error4describe( c4, e4index, E81714, buf, 0, 0 ) ;
         #endif
         index4close( i4 ) ;
         #ifndef E4ANALYZE_ALL
            error4describe( c4, e4index, E81714, (char *)buf, 0, 0 ) ;
         #endif
         return 0 ;
      }

      i4->versionOld = i4->tagIndex->header.version ;
      i4->versionReadUnlocked = i4->versionOld ;
      i4->blockMemory = mem4create( c4, c4->memStartBlock, sizeof(B4BLOCK) + B4BLOCK_SIZE
                      - sizeof(B4STD_HEADER) - sizeof(B4NODE_HEADER), c4->memExpandBlock, 0 ) ;
      if ( i4->blockMemory == 0 )
      {
         index4close( i4 ) ;
         return 0 ;
      }

      /* do an initial block allocation to make sure minimal is allocated while optimization is suspended */
      b4 = (B4BLOCK *)mem4alloc2( i4->blockMemory, c4 ) ;
      if ( b4 == 0 )
      {
         index4close( i4  ) ;
         error4( c4, e4memory, E91707 ) ;
         return 0 ;
      }
      else
      {
         mem4free( i4->blockMemory, b4 ) ;
         b4 = 0 ;
      }

      #ifndef S4SINGLE
         /* disable locking */
         oldFileLock = i4->fileLocked ;
         i4->fileLocked = data4serverId( d4 ) ;
      #endif

      rc = tfile4top( i4->tagIndex ) ;
      if ( rc < 0 )
      {
         #ifndef S4SINGLE
            i4->fileLocked = oldFileLock ;
         #endif
         index4close( i4 ) ;
         return 0 ;
      }

      /* if we have a compound index, then load the tags, otherwise this is the only tag */
      if ( i4->tagIndex->header.typeCode >= 64 )
      {
         if ( b4numKeys( tfile4block( i4->tagIndex ) ) )
            do
            {
               tagFile = (TAG4FILE *)mem4alloc( c4->tagFileMemory ) ;
               if ( tagFile == 0 )
               {
                  index4close( i4 ) ;
                  error4( c4, e4memory, E91707 ) ;
                  #ifndef S4SINGLE
                     i4->fileLocked = oldFileLock ;
                  #endif
                  return 0 ;
               }

               if ( i4->file.isReadOnly == 1 ) /* special case, allow to open invalid expression tags, since not appending */
               {
                  oldErrExpr = c4->errExpr ;
                  c4->errExpr = 0 ;
               }
               if ( tfile4init( tagFile, index, b4recNo( tfile4block(i4->tagIndex), tfile4block(i4->tagIndex)->keyOn ), tfile4keyData( i4->tagIndex )->value ) < 0 )
               {
                  if ( i4->file.isReadOnly == 1 ) /* special case, allow to open invalid expression tags, since not appending */
                     error4set( c4, 0 ) ;
                  else
                  {
                     #ifndef S4SINGLE
                        i4->fileLocked = oldFileLock ;
                     #endif
                     index4close( i4 ) ;
                     return 0 ;
                  }
               }
               if ( i4->file.isReadOnly == 1 ) /* special case, allow to open invalid expression tags, since not appending */
                  c4->errExpr = oldErrExpr ;
               if ( tfile4setCodePage( tagFile, d4->codePage ) < 0 )
               {
                  error4( c4, e4index, E91642 ) ;
                  index4close( i4 ) ;
                  return 0 ;
               }

               l4add( &i4->tags, tagFile ) ;
            } while ( tfile4skip( i4->tagIndex, 1L ) == 1L ) ;
      }
      else
      {
         #ifdef E4MISC
            if ( fileName == 0 )
            {
               index4close( i4 ) ;
               error4( c4, e4index, E81715 ) ;
            }
         #endif
         tagFile = i4->tagIndex ;
         l4add( &i4->tags, i4->tagIndex ) ;   /* if an .idx, add single tag */
      }
      #ifndef S4SINGLE
         i4->fileLocked = oldFileLock ;
      #endif
   #endif

   #ifndef S4FOX
      #ifdef E4MISC
         for ( i4ptr = 0 ;; )
         {
            i4ptr = (INDEX4FILE *)l4next(&dfile->indexes, i4ptr) ;
            if ( i4ptr == 0 )
               break ;
            if ( !c4memcmp( i4ptr->file.name, buf, (size_t) strlen(buf) ) )
            {
               error4( c4, e4parm, E81703 ) ;
               return 0 ;
            }
         }
      #endif

      rc = file4open( &i4->file, c4, buf, 1 ) ;
      if ( rc )
      {
         index->indexFile = 0 ;
         mem4free( c4->index4fileMemory, i4 ) ;
         return 0 ;
      }

      file4longAssign( pos, 0, 0 ) ;
      if ( file4readAllInternal( &i4->file, pos, &i4->header, sizeof(I4HEADER) ) < 0 )
      {
         file4close( &i4->file ) ;
         index->indexFile = 0 ;
         mem4free( c4->index4fileMemory, i4 ) ;
         return 0 ;
      }

      #ifdef WORDS_BIGENDIAN
         i4->header.blockChunks = x4reverseShort( (void *)&i4->header.blockChunks ) ;
         i4->header.blockRw = x4reverseShort( (void *)&i4->header.blockRw ) ;
         i4->header.slotSize = x4reverseShort( (void *)&i4->header.slotSize ) ;
         i4->header.numTags = x4reverseShort( (void *)&i4->header.numTags ) ;
         i4->header.eof = x4reverseLong( (void *)&i4->header.eof ) ;
         i4->header.freeList = x4reverseLong( (void *)&i4->header.freeList ) ;
      #endif

      l4add( &dfile->indexes, i4 ) ;

      /* Perform some checks */
      if ( i4->header.blockRw != i4->header.blockChunks*512  ||
           i4->header.blockChunks <= 0 ||
           i4->header.blockChunks > 63 ||
           i4->header.numTags < 0  || i4->header.numTags > 47 ||
           i4->header.eof <= 0L )
      {
         index4close( i4 ) ;
         error4describe( c4, e4index, E81716, buf, 0, 0 ) ;
         return 0 ;
      }

      file4longAssign( pos, 544, 0 ) ;
      if ( file4readAllInternal( &i4->file, pos, tagInfo, sizeof(tagInfo)) < 0 )
      {
         index4close( i4 ) ;
         return 0 ;
      }

      for ( iTag = 0; iTag < (int) i4->header.numTags; iTag++ )
      {
         tagFile = (TAG4FILE *)mem4alloc( c4->tagFileMemory ) ;
         if ( tagFile == 0 )
         {
            index4close( i4 ) ;
            #ifdef E4STACK
               error4stack( c4, e4memory, E91707 ) ;
            #endif
            return 0 ;
         }

         #ifdef WORDS_BIGENDIAN
            tagInfo[iTag].headerPos = x4reverseLong( (void *)&tagInfo[iTag].headerPos ) ;
            tagInfo[iTag].x1000 = 0x1000 ;
         #endif

         if ( tfile4init( tagFile, index, tagInfo + iTag ) < 0 )
         {
            index4close( i4 ) ;
            return 0 ;
         }

         l4add( &i4->tags, tagFile ) ;
      }

      i4->blockMemory = mem4create( c4, c4->memStartBlock, sizeof(B4BLOCK) + i4->header.blockRw -
                        sizeof(B4KEY_DATA) - sizeof(short) - sizeof(char[6]), c4->memExpandBlock, 0 ) ;
      if ( i4->blockMemory == 0 )
      {
         index4close( i4 ) ;
         return 0 ;
      }
   #endif

   #ifndef S4OPTIMIZE_OFF
      file4optimize( &i4->file, c4->optimize, OPT4INDEX ) ;
   #endif

   i4->userCount++ ;
   i4->isValid = 1 ;
   return i4 ;
}

TAG4 *S4FUNCTION i4tag( INDEX4 *i4, const char *tagName )
{
   char tagLookup[LEN4TAG_ALIAS+1] ;
   TAG4 *tagOn ;

   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E91709 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( i4 == 0 )
      {
         error4( 0, e4parmNull, E91709 ) ;
         return 0 ;
      }
      if ( tagName == 0 )
      {
         error4( i4->codeBase, e4parmNull, E91709 ) ;
         return 0 ;
      }
   #endif

   u4ncpy( tagLookup, tagName, sizeof( tagLookup )) ;
   c4upper( tagLookup ) ;

   for( tagOn = 0 ;; )
   {
      tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
      if ( tagOn == 0 )
         break ;
      if ( strcmp( tagOn->tagFile->alias, tagLookup ) == 0 )
         return tagOn ;
   }

   if ( i4->codeBase->errTagName )
      error4describe( i4->codeBase, e4tagName, E91709, tagName, 0, 0 ) ;
   return 0 ;
}

#ifndef S4OFF_WRITE
int index4shrink( INDEX4FILE *i4, long blockNo )
{
   int rc ;
   FILE4LONG pos ;

   #ifdef E4PARM_LOW
      if ( i4 == 0 )
         return error4( 0, e4parmNull, E91708 ) ;
      if ( blockNo < 0 )
         return error4( i4->codeBase, e4parm, E91708 ) ;
   #endif

   if ( error4code( i4->codeBase ) < 0 )
      return e4codeBase ;

   #ifdef S4FOX
      #ifdef WORDS_BIGENDIAN
         i4->tagIndex->header.freeList = x4reverseLong( (void *)&i4->tagIndex->header.freeList ) ;
      #endif
      file4longAssign( pos, blockNo, 0 ) ;
      rc = file4writeInternal( &i4->file, pos, (char *)&i4->tagIndex->header.freeList,
           sizeof(i4->tagIndex->header.freeList) ) ;
      if ( rc < 0 )
         return error4stack( i4->codeBase, (short)rc, E91708 ) ;
      i4->tagIndex->header.freeList = blockNo ;
   #else
      #ifdef WORDS_BIGENDIAN
         i4->header.freeList = x4reverseLong( (void *)&i4->header.freeList ) ;
      #endif
      file4longAssign( pos, blockNo*I4MULTIPLY + sizeof(S4LONG), 0 ) ;
      rc = file4writeInternal( &i4->file, pos, (char *)&i4->header.freeList, sizeof(i4->header.freeList) ) ;
      if ( rc < 0 )
         return error4stack( i4->codeBase, rc, E91708 ) ;
      i4->header.freeList = blockNo ;
   #endif

   return 0 ;
}
#endif  /* S4OFF_WRITE */

#ifndef S4OFF_WRITE
/* Updates the header if the version has changed */
int i4updateHeader( INDEX4 *i4 )
{
   return index4updateHeader( i4->indexFile ) ;
}

int index4updateHeader( INDEX4FILE *i4 )
{
   FILE4LONG pos ;
   #ifdef S4MDX
      int rc ;
      TAG4FILE *tagOn ;
      #ifdef WORDS_BIGENDIAN
         I4HEADER swap ;
      #endif
   #endif

   #ifdef E4PARM_LOW
      if ( i4 == 0 )
         return error4( 0, e4parmNull, E91710 ) ;
   #endif

   if ( error4code( i4->codeBase ) < 0 )
      return e4codeBase ;

   #ifdef S4FOX
      if ( i4->tagIndex == 0 )   /* index file not complete */
         return 0 ;

      if ( i4->versionOld != i4->tagIndex->header.version )
      {
         #ifdef WORDS_BIGENDIAN
            i4->tagIndex->header.root = x4reverseLong( (void *)&i4->tagIndex->header.root ) ;
            i4->tagIndex->header.freeList = x4reverseLong( (void *)&i4->tagIndex->header.freeList ) ;
            i4->tagIndex->header.keyLen = x4reverseShort( (void *)&i4->tagIndex->header.keyLen ) ;
         #else
            i4->tagIndex->header.version = (unsigned long)x4reverseLong( (void *)&i4->tagIndex->header.version ) ;
         #endif

         file4longAssign( pos, 0, 0 ) ;
         if ( file4writeInternal( &i4->file, pos, (char *)&i4->tagIndex->header, LEN4HEADER_WR ) < 0 )
            return -1;

         #ifdef WORDS_BIGENDIAN
            i4->tagIndex->header.root = x4reverseLong( (void *)&i4->tagIndex->header.root ) ;
            i4->tagIndex->header.freeList = x4reverseLong( (void *)&i4->tagIndex->header.freeList ) ;
            i4->tagIndex->header.keyLen = x4reverseShort( (void *)&i4->tagIndex->header.keyLen ) ;
         #else
            i4->tagIndex->header.version = (unsigned long)x4reverseLong( (void *)&i4->tagIndex->header.version ) ;
         #endif
         i4->versionOld = i4->tagIndex->header.version ;
         i4->versionReadUnlocked = i4->versionOld ;
      }
   #else
      if ( i4->changed )
      {
         #ifdef WORDS_BIGENDIAN
            memcpy( (void *)&swap, (void *)&i4->header, sizeof(I4HEADER) ) ;

            swap.blockChunks = x4reverseShort( (void *)&swap.blockChunks ) ;
            swap.blockRw = x4reverseShort( (void *)&swap.blockRw ) ;
            swap.slotSize = x4reverseShort( (void *)&swap.slotSize ) ;
            swap.numTags = x4reverseShort( (void *)&swap.numTags ) ;
            swap.eof = x4reverseLong( (void *)&swap.eof ) ;
            swap.freeList = x4reverseLong( (void *)&swap.freeList ) ;

            rc = file4write( &i4->file, 0L, (char *)&swap, sizeof( I4HEADER ) ) ;
            if ( rc < 0 )
               return error4stack( i4->codeBase, rc, E91710 ) ;
         #else
            file4longAssign( pos, 0, 0 ) ;
            rc = file4writeInternal( &i4->file, pos, (char *)&i4->header, sizeof( I4HEADER ) ) ;
            if ( rc < 0 )
               return error4stack( i4->codeBase, rc, E91710 ) ;
         #endif
         i4->changed = 0 ;
         for( tagOn = 0 ;; )
         {
            tagOn = (TAG4FILE *)l4next( &i4->tags, tagOn ) ;
            if ( tagOn == 0 )
               break ;
            if ( tagOn->changed == 1 )
            {
               tagOn->header.version++ ;
               tagOn->changed = 0 ;
               file4longAssign( pos, tagOn->headerOffset + 20L, 0 ) ;
               rc = file4writeInternal( &i4->file, pos, &tagOn->header.version, sizeof(char) ) ;
               if ( rc < 0 )
                  return error4stack( i4->codeBase, rc, E91710 ) ;
               if ( tagOn->hadKeys != tagOn->hasKeys )
               {
                  /* just update the tag to record that it has keys (no other update yet) */
                  file4longAssign( pos, tagOn->headerOffset + 222 + sizeof( T4HEADER ), 0 ) ;
                  rc = file4writeInternal( &i4->file, pos, &tagOn->hasKeys, sizeof(char) ) ;
                  if ( rc < 0 )
                     return error4stack( i4->codeBase, rc, E91710 ) ;
                  tagOn->hadKeys = tagOn->hasKeys ;
               }
            }
         }
      }
   #endif

   return 0 ;
}
#endif  /* S4OFF_WRITE */

int S4FUNCTION t4versionCheck( TAG4 *t4, const int doSeek, const int updateVersion )
{
   #ifndef S4OPTIMIZE_OFF
      if ( t4->index->indexFile->file.doBuffer == 0 )
         return i4versionCheck( t4->index, doSeek, updateVersion ) ;
      else
         return 0 ;
   #else
      return i4versionCheck( t4->index, doSeek, updateVersion ) ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION i4versionCheck( INDEX4 *i4, const int doSeek, const int updateVersion )
{
   #ifndef S4SINGLE
      TAG4 *saveTag ;
      TAG4FILE *tagOn ;
      int needSeek ;
      B4BLOCK *b4 ;
      int rc ;

      #ifdef E4PARM_LOW
         if ( i4 == 0 )
            return error4( 0, e4parmNull, E91711 ) ;
      #endif

      if ( error4code( i4->codeBase ) < 0 )
         return e4codeBase ;

      rc = index4versionCheck( i4->indexFile, updateVersion ) ;
      if ( rc < 0 )
         return error4stack( 0, (short)rc, E91711 ) ;
      if ( rc == 0 )   /* version did not change */
         return 0 ;

      needSeek = 0 ;
      saveTag = d4tagSelected( i4->data ) ;
      if ( saveTag != 0 )
      {
         /* remember the old position */
         if ( doSeek )
         {
            b4 = (B4BLOCK *)saveTag->tagFile->blocks.lastNode ;
            if ( b4 != 0 )
            {
               if ( tfile4eof( saveTag->tagFile ) )
                  needSeek = 2 ;
               else
                  #ifdef S4FOX
                     if ( b4leaf( b4 ) && b4numKeys( b4 ) != 0 )
                  #else
                     if ( b4leaf( b4 ) && b4numKeys( b4 ) != 0 && b4->keyOn < b4numKeys( b4 ) )
                  #endif
                     {
                        #ifdef S4FOX
                           memcpy( i4->codeBase->savedKey, (void *)(b4key( b4, b4->keyOn )), saveTag->tagFile->header.keyLen + sizeof(S4LONG) ) ;
                        #else
                           memcpy( i4->codeBase->savedKey, (void *)(b4key( b4, b4->keyOn )), saveTag->tagFile->header.keyLen + 2 * sizeof(S4LONG) ) ;
                        #endif
                        needSeek = 1 ;
                     }
            }
         }
      }

      for ( tagOn = 0 ;; )
      {
         tagOn = (TAG4FILE *)l4next( &i4->indexFile->tags, tagOn ) ;
         if ( tagOn == 0 )
            break ;
         if ( tfile4freeAll( tagOn ) < 0 )  /* Should be a memory operation only */
            return error4( i4->codeBase, e4result, E91711 ) ;
      }

      switch ( needSeek )
      {
         case 1:
            tfile4go( saveTag->tagFile, ((B4KEY_DATA *)i4->codeBase->savedKey)->value, ((B4KEY_DATA *)i4->codeBase->savedKey)->num, 0 ) ;
            break ;
         case 2:
            tfile4goEof( saveTag->tagFile ) ;
            break ;
         default:
            break ;
      }
   #endif

   return 0 ;
}

/* Reads the header, checks the version to see if the blocks need to be freed. */
/* returns 1 if version needs to be updated, else 0 */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int index4versionCheck( INDEX4FILE *i4, const int updateVersion )
{
   #ifdef S4SINGLE
      return 0 ;
   #else
      #ifndef S4FOX
         TAG4FILE *tagOn ;
      #endif
      int rc ;
      FILE4LONG pos ;

      #ifdef E4PARM_LOW
         if ( i4 == 0 )
            return error4( 0, e4parmNull, E91712 ) ;
      #endif

      if ( error4code( i4->codeBase ) < 0 )
         return e4codeBase ;

      if ( index4lockTest( i4 ) )
         return 0 ;

      #ifndef S4OPTIMIZE_OFF
         /* make sure read from disk unless file locked, etc. */
         if ( i4->file.doBuffer )  /* also makes sure 'opt' should exist */
            i4->codeBase->opt.forceCurrent = 1 ;
      #endif

      file4longAssign( pos, 0, 0 ) ;
      #ifdef S4FOX
         rc = file4readAllInternal( &i4->file, pos, &i4->tagIndex->header, LEN4HEADER_WR ) ;
         #ifndef S4OPTIMIZE_OFF
            if ( i4->file.doBuffer )
               i4->codeBase->opt.forceCurrent = 0 ;
         #endif
         if ( rc < 0 )
            return error4stack( i4->codeBase, (short)rc, E91712 ) ;

         #ifdef WORDS_BIGENDIAN
            i4->tagIndex->header.root = x4reverseLong( (void *)&i4->tagIndex->header.root ) ;
            i4->tagIndex->header.freeList = x4reverseLong( (void *)&i4->tagIndex->header.freeList ) ;
            /* version is already in non-intel form */
            i4->tagIndex->header.keyLen = x4reverseShort( (void *)&i4->tagIndex->header.keyLen ) ;
         #else
            i4->tagIndex->header.version = (unsigned long)x4reverseLong( (void *)&i4->tagIndex->header.version ) ;
         #endif

         if ( i4->tagIndex->header.version == i4->versionOld )
            return 0 ;

         if ( updateVersion == 0 )
         {
            /* AS 06/13/97 if updateVersion false, then reading, so if version
               == to read value, allow to continue, just reset regular version */
            if ( i4->tagIndex->header.version == i4->versionReadUnlocked )
            {
               i4->tagIndex->header.version = i4->versionOld ;
               return 0 ;
            }

            /* AS 06/13/97 Keep a read version which is based on last read here, but not
               guaranteed for write operation since the write-order of the index
               file is not guaranteed (version updated first or after index file),
               which can only be verified if the index file is locked */
            i4->versionReadUnlocked = i4->tagIndex->header.version ;
            i4->tagIndex->header.version = i4->versionOld ;
         }
         else
         {
            i4->versionOld = i4->tagIndex->header.version ;
            i4->versionReadUnlocked = i4->versionOld ;
         }
      #else
         rc = file4readAllInternal( &i4->file, pos, &i4->header, sizeof(I4HEADER) ) ;
         #ifndef S4OPTIMIZE_OFF
            if ( i4->file.doBuffer )
               i4->codeBase->opt.forceCurrent = 0 ;
         #endif
         if ( rc < 0 )
            return error4stack( i4->codeBase, rc, E91712 ) ;

         #ifdef WORDS_BIGENDIAN
            i4->header.blockChunks = x4reverseShort( (void *)&i4->header.blockChunks ) ;
            i4->header.blockRw = x4reverseShort( (void *)&i4->header.blockRw ) ;
            i4->header.slotSize = x4reverseShort( (void *)&i4->header.slotSize ) ;
            i4->header.numTags = x4reverseShort( (void *)&i4->header.numTags ) ;
            i4->header.eof = x4reverseLong( (void *)&i4->header.eof ) ;
            i4->header.freeList = x4reverseLong( (void *)&i4->header.freeList ) ;
         #endif
         #ifndef S4OPTIMIZE_OFF
            /* make sure read from disk unless file locked, etc. */
            if ( i4->file.doBuffer )  /* also makes sure 'opt' should exist */
              i4->codeBase->opt.forceCurrent = 1 ;
         #endif
         for( tagOn = 0 ;; )
         {
            tagOn = (TAG4FILE *)l4next( &i4->tags, tagOn ) ;
            if ( tagOn == 0 )
               break ;
            file4longAssign( pos, tagOn->headerOffset + 20L, 0 ) ;
            rc = file4readAllInternal( &i4->file, pos, &tagOn->header.version, sizeof(char) ) ;
            if ( rc < 0 )
            {
               #ifndef S4OPTIMIZE_OFF
                  if ( i4->file.doBuffer )
                     i4->codeBase->opt.forceCurrent = 0 ;
               #endif
               return error4stack( i4->codeBase, rc, E91712 ) ;
            }
            file4longAssign( pos, tagOn->headerOffset + 222 + sizeof( T4HEADER ), 0 ) ;
            rc = file4readAllInternal( &i4->file, pos, &tagOn->hasKeys, sizeof(char) ) ;
            if ( rc < 0 )
            {
               #ifndef S4OPTIMIZE_OFF
                  if ( i4->file.doBuffer )
                     i4->codeBase->opt.forceCurrent = 0 ;
               #endif
               return error4stack( i4->codeBase, rc, E91712 ) ;
            }
            tagOn->hadKeys = tagOn->hasKeys ;
         }

         #ifndef S4OPTIMIZE_OFF
            if ( i4->file.doBuffer )
               i4->codeBase->opt.forceCurrent = 0 ;
         #endif
      #endif
      return 1 ;
   #endif
}
#endif /* S4CLIPPER */

/* need a special version check */
int S4FUNCTION tfile4versionCheckFree( TAG4FILE *t4 )
{
   #ifdef S4CLIPPER
      return tfile4doVersionCheck( t4, 0, 0 ) ;
   #else
      int rc ;
      TAG4FILE *tagOn ;
      INDEX4FILE *i4;

      i4 = t4->indexFile ;
      rc = index4versionCheck( i4, 0 ) ;
      if ( rc <= 0 )
        return rc ;
      /* else means version changed */

      for ( tagOn = 0 ;; )
      {
        tagOn = (TAG4FILE *)l4next( &i4->tags, tagOn ) ;
        if ( tagOn == 0 )
           break ;
        if ( tfile4freeAll( tagOn ) < 0 )  /* Should be a memory operation only */
           return error4( i4->codeBase, e4result, E91711 ) ;
      }
   #endif
}


#ifdef S4CLIPPER

/* This function closes all the tags corresponding with the index */
int S4FUNCTION i4closeLow( INDEX4 *i4 )
{
   int finalRc ;
   CODE4 *c4 ;
   TAG4 *tagOn ;
   int oldOpenErr, rc ;
   char buf[258] ;
   #ifndef S4SINGLE
      int saveAttempts ;
   #endif

   c4 = i4->codeBase ;

   finalRc = error4code( c4 ) ;
   #ifndef S4OFF_WRITE
      if ( i4->data )
         if ( d4update( i4->data ) < 0 )
            finalRc = error4set( c4, 0 ) ;
   #endif

   #ifndef S4SINGLE
      saveAttempts = c4->lockAttempts ;
      c4->lockAttempts = WAIT4EVER ;
      if ( i4unlock(i4) < 0 )
         finalRc = error4set( c4, 0 ) ;
   #endif

   for( ;; )
   {
      tagOn = (TAG4 *)l4pop( &i4->tags ) ;
      if ( tagOn == 0 )
         break ;
      if ( i4->data->tagSelected == tagOn )   /* can't have a non-existant tag selected */
         i4->data->tagSelected = 0 ;
      t4close( tagOn ) ;
      tagOn = 0 ;
   }

   if ( c4->doRemove == 1 )
   {
      oldOpenErr = c4->errOpen ;
      c4->errOpen = 0 ;
      u4ncpy( buf, i4->accessName, sizeof( buf ) - 1 ) ;
      #ifndef S4CASE_SEN
         u4nameExt( buf, sizeof( buf ), "CGP", 1 ) ;
      #else
         u4nameExt( buf, sizeof( buf ), "cgp", 1 ) ;
      #endif
      rc = file4open( &i4->file, c4, buf, 0 ) ;
      if ( rc == 0 )
      {
         i4->file.isTemp = 1 ;
         file4close( &i4->file ) ;
      }
      c4->errOpen = oldOpenErr ;
   }

   if ( i4->data )
      l4remove( &i4->data->indexes, i4 ) ;

   mem4free( c4->indexMemory, i4 ) ;
   i4 = 0 ;
   #ifndef S4SINGLE
      c4->lockAttempts = saveAttempts ;
   #endif
   error4set( c4, finalRc ) ;
   return  finalRc ;
}

#ifndef S4OFF_WRITE
/* This function flushes all the tags corresponding with the index */
int i4flush( INDEX4 *i4 )
{
   TAG4 *tagOn ;
   int rc ;

   #ifdef E4PARM_LOW
      if ( i4 == 0 )
         return error4( 0, e4parmNull, E91713 ) ;
   #endif

   #ifndef S4SINGLE
      if ( i4->data->dataFile->indexLocked == 1 )
   #endif
      for ( tagOn = 0 ;; )
      {
         tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
         if ( tagOn == 0 )
            break ;
         rc = tfile4flush( tagOn->tagFile ) ;
         if ( rc < 0 )
            return error4stack( i4->codeBase, rc, E91713 ) ;
         tagOn->tagFile->header.root = -1L ;
      }
      return 0 ;
}

int i4update( INDEX4 *i4 )
{
   TAG4 *tagOn ;
   int rc ;

   #ifdef E4PARM_LOW
      if ( i4 == 0 )
         return error4( 0, e4parmNull, E91705 ) ;
   #endif

   if ( error4code( i4->codeBase ) < 0 )
      return e4codeBase ;

   #ifndef S4SINGLE
      if ( i4->data->dataFile->indexLocked == 1 )
   #endif
      for ( tagOn = 0 ;; )
      {
         tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
         if ( tagOn == 0 )
            break ;
         rc = tfile4update( tagOn->tagFile ) ;
         if ( rc < 0 )
            return error4stack( i4->codeBase, rc, E91705 ) ;
         tagOn->tagFile->header.root = -1L ;
      }
      return 0 ;
}
#endif

INDEX4 *S4FUNCTION i4open( DATA4 *d4, const char *fileName )
{
   INDEX4 *i4 ;
   CODE4 *c4 ;
   TAG4 *tag ;
   int len, rc, i ;
   char buf[258], tagBuf[258], ext[3] ;
   int numFiles ;
   FILE4SEQ_READ seqread ;
   char buffer[1024], tNames[258], firstByte ;
   int pos, iPos, saveLen, tempLen ;
      FILE4LONG tPos ;

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E91706 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( d4 == 0 )
      {
         error4( 0, e4parmNull, E91706 ) ;
         return 0 ;
      }
   #endif

   c4 = d4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return 0 ;

      if ( fileName == 0 )
      {
         u4ncpy( buf, d4->dataFile->file.name, sizeof( buf ) - 1 ) ;
         #ifndef S4CASE_SEN
            u4nameExt( buf, sizeof( buf ), "CGP", 1 ) ;
         #else
            u4nameExt( buf, sizeof( buf ), "cgp", 1 ) ;
         #endif
      }
      else
      {
         u4ncpy( buf, fileName, sizeof( buf ) - 1 ) ;
         #ifndef S4CASE_SEN
            c4upper( buf ) ;
         #endif
         rc = u4nameRetExt( ext, 3, buf ) ;
         if ( rc )  /* extension provided */
         {
            if ( rc == 3 )
               #ifndef S4CASE_SEN
                  if ( c4memcmp( ext, "NTX", 3 ) == 0 )
               #else
                  if ( c4memcmp( ext, "ntx", 3 ) == 0 )
               #endif
               {
                  tag = t4open( d4, (INDEX4 *)0, fileName ) ;
                  if ( tag == 0 )
                     return 0 ;
                  #ifndef S4OFF_TRAN
                     tag->index->isValid = 1 ;
                  #endif
                  return tag->index ;
               }
         }
         else
            #ifndef S4CASE_SEN
               u4nameExt( buf, sizeof(buf), "CGP", 0 ) ;
            #else
               u4nameExt( buf, sizeof(buf), "cgp", 0 ) ;
            #endif
      }

      if ( c4->indexMemory == 0 )
      {
         c4->indexMemory = mem4create( c4, c4->memStartIndex, sizeof( INDEX4 ), c4->memExpandIndex, 0 ) ;
         if ( c4->indexMemory == 0 )
            return 0 ;
      }

      if ( c4->tagMemory == 0 )
      {
         c4->tagMemory = mem4create( c4, c4->memStartTag, sizeof( TAG4 ), c4->memExpandTag, 0 ) ;
         if ( c4->tagMemory == 0 )
            return 0 ;
      }

      if ( c4->tagFileMemory == 0 )
      {
         c4->tagFileMemory = mem4create( c4, c4->memStartTagFile, sizeof( TAG4FILE ), c4->memExpandTagFile, 0 ) ;
         if ( c4->tagFileMemory == 0 )
            return 0 ;
      }

      i4 = (INDEX4 *)mem4alloc( c4->indexMemory ) ;
      if ( i4 == 0 )
         return 0 ;

      i4->codeBase = c4 ;
      i4->data = d4 ;
      l4add( &d4->indexes, i4 ) ;
      if ( fileName == 0 )
         u4namePiece( i4->accessName, sizeof( i4->accessName ), buf, 0, 0 ) ;
      else
         strcpy( i4->accessName, fileName ) ;
      c4upper( i4->accessName ) ;

      if ( file4open( &i4->file, c4, buf, 1 ) )
      {
         i4closeLow( i4 ) ;
         if ( c4->errOpen )
            error4( c4, e4open, E81708 ) ;
         return 0 ;
      }

      file4longAssign( tPos, 0, 0 ) ;
      file4seqReadInitDo( &seqread, &i4->file, tPos, buffer, sizeof(buffer), 0 ) ;

      pos = 0L ;
      rc = file4seqReadAll( &seqread, &firstByte, sizeof( firstByte ) ) ;
      if ( rc )
      {
         i4closeLow( i4 ) ;
         if ( c4->errOpen )
            error4( c4, e4info, E81709 ) ;
         return 0 ;
      }

      if ( firstByte < 65 )   /* old format - potential problem if >= 65 files in an old format file. */
      {
         numFiles = firstByte ;
         if ( file4seqReadAll( &seqread, &firstByte, sizeof( firstByte ) ) )
         {
            i4closeLow( i4 ) ;
            if ( c4->errOpen )
               error4( c4, e4info, E81709 ) ;
            return 0 ;
         }

         for ( i = 0 ; i < numFiles ; i++ )
         {
            if ( file4seqReadAll( &seqread, &len, sizeof( len ) ) )
            {
               file4close( &i4->file ) ;
               i4closeLow( i4 ) ;
               if ( c4->errOpen )
                  error4( c4, e4info, E81709 ) ;
               return 0 ;
            }
            pos += sizeof( len ) ;
            rc = u4namePath( tagBuf, sizeof( tagBuf ), buf ) ;
            tagBuf[rc+len] = '\0' ;
            if ( sizeof( tagBuf ) > rc + len ) /* make sure room to read */
               rc = file4seqReadAll( &seqread, tagBuf+rc, len ) ;
            else
               rc = -1 ;

            if ( rc )
            {
               if ( c4->errOpen )
                  error4( c4, e4info, E81709 ) ;
               file4close( &i4->file ) ;
               i4closeLow( i4 ) ;
               return 0 ;
            }

            pos += len ;
            if  ( t4open( d4, i4, tagBuf ) == 0 )
            {
               file4close( &i4->file ) ;
               i4closeLow( i4 ) ;
               return 0 ;
            }
         }

         file4close( &i4->file ) ;
      }
      else
      {
         file4longAssign( tPos, 0, 0 ) ;
         file4seqReadInitDo( &seqread, &i4->file, tPos, buffer, sizeof(buffer), 0 ) ;
         saveLen = 0 ;

         for( len = sizeof( tNames ) ; len == sizeof( tNames ) ; )
         {
            len = file4seqRead( &seqread, tNames, sizeof( tNames )) ;
            if ( len < sizeof( tNames ) ) /* case where all read in now - free up this file handle for use */
            {
               if ( file4close ( &i4->file ) )
               {
                  i4closeLow( i4 ) ;
                  if ( c4->errOpen )
                     error4( c4, e4info, E81710 ) ;
                  return 0 ;
               }
               if ( len == 0 )
                  break ;
            }
            for( iPos = 0, pos = 0 ; pos < len ; )
            {
               switch( tNames[pos] )
               {
                  /* cases where the values are ignored, or found name */
                  case ' ':
                  case '\r':
                  case '\n':
                  case '\t':
                  case '\x1A':
                     if ( iPos < pos )  /* try to open the file */
                     {
                        tempLen = pos - iPos ;
                        if ( saveLen == 0 )
                        {
                           rc = u4namePath( tagBuf, sizeof( tagBuf ), buf ) ;
                           tagBuf[rc + tempLen] = '\0' ;
                        }
                        else
                           rc = saveLen ;
                        memcpy( tagBuf + rc, &tNames[iPos], tempLen ) ;

                        if  ( t4open( d4, i4, tagBuf ) == 0 )
                        {
                           file4close( &i4->file ) ;
                           i4closeLow( i4 ) ;
                           return 0 ;
                        }
                     }
                     iPos = ++pos ;
                     break ;

                  /* case where a name is attempted to be read in */
                  default:
                     pos++ ;
               }
            }
            tempLen = pos - iPos ;
            rc = u4namePath( tagBuf, sizeof( tagBuf ), buf ) ;
            tagBuf[rc+tempLen] = '\0' ;
            memcpy( tagBuf + rc, &tNames[iPos], tempLen ) ;
            saveLen = rc + tempLen ;
         }

         if ( ( saveLen - rc ) > 0 )  /* try to open the file */
         {
            if  ( t4open( d4, i4, tagBuf ) == 0 )
            {
               file4close( &i4->file ) ;
               i4closeLow( i4 ) ;
               return 0 ;
            }
         }
      }
   #ifndef S4OFF_TRAN
      i4->isValid = 1 ;
   #endif

   return i4 ;
}

TAG4 *S4FUNCTION i4tag( INDEX4 *i4, const char *tagName )
{
   char tagLookup[LEN4TAG_ALIAS+1] ;
   TAG4 *tagOn ;

   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E91709 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( i4 == 0 || tagName == 0 )
      {
         error4( 0, e4parmNull, E91709 ) ;
         return 0 ;
      }
   #endif

   u4ncpy( tagLookup, tagName, sizeof( tagLookup ) - 1 ) ;
   c4upper( tagLookup ) ;

   for( tagOn = 0 ;; )
   {
      tagOn = (TAG4 *)l4next( &i4->tags, tagOn) ;
      if ( tagOn == 0 )
         break ;
      if ( strcmp( tagOn->tagFile->alias, tagLookup) == 0 )
         return tagOn ;
   }

   if ( i4->codeBase->errTagName )
      error4describe( i4->codeBase, e4tagName, E91709, tagName, 0, 0 ) ;
   return 0 ;
}
#endif   /*  ifdef S4CLIPPER  */

#ifndef S4CLIPPER
int index4isProduction( INDEX4FILE *i4 )
{
   #ifdef S4FOX
      char ext[3] ;
      int l1, l2, count ;
   #endif
   #ifdef E4PARM_LOW
      if ( i4 == 0 )
         return error4( 0, e4parmNull, E91714 ) ;
   #endif

   #ifdef S4MDX
         return i4->dataFile->openMdx ;
   #endif

   #ifdef S4FOX
         if ( i4->dataFile->openMdx )
      {
         /* AS 08/04/97 fix #94 */
         /* first check extension */
         if ( u4nameRetExt( ext, sizeof( ext ), i4->file.name ) != 3 )  /* does not match */
            return 0 ;
         if ( memcmp( code4indexExtension( i4->codeBase ), ext, 3 ) != 0 )
            return 0 ;

         if ( i4->file.name == 0 )   /* most likely a failure during open/create now closing */
            return 0 ;
         l1 = strlen( i4->file.name ) ;
         if ( l1 == 0 )
            return 0 ;
         for ( count = l1 - 1 ;; count-- )
         {
            if ( count <= 0 )
               break ;
            if ( i4->file.name[count] == '.' )
            {
               l1 = count ;
               break ;
            }
         }
         l2 = strlen( i4->dataFile->file.name ) ;
         if ( l2 == 0 )
            return 0 ;
         for ( count = l2 - 1 ;; count-- )
         {
            if ( count <= 0 )
               break ;
            if ( i4->dataFile->file.name[count] == '.' )
            {
               l2 = count ;
               break ;
            }
         }
         if ( l1 == l2 )
            return( c4memcmp( i4->file.name, i4->dataFile->file.name, (unsigned)l1 ) ? 0 : 1 ) ;
      }
      return 0 ;
   #endif
}
#endif /* S4CLIPPER */
#endif /* S4INDEX_OFF */

#ifdef S4VB_DOS
INDEX4 * i4open_v( DATA4 *d4, char *name )
{
   char *namePtr ;

   namePtr = c4str(name) ;

   if (namePtr[0] == '\0' )
      return i4open( d4, 0 ) ;

   return i4open( d4, namePtr ) ;
}

TAG4 *i4tag_v( INDEX4 *ind, char *tagName )
{
 return i4tag( ind, c4str(tagName) ) ;
}

#endif  /* S4VB_DOS */

#ifdef WORDS_BIGENDIAN
#ifdef S4CLIPPER
void index4swapBlockClipper(void *swap, int keysmax, int grouplen )
{
   char *swapPtr = swap ;
   int j ;
   #ifdef S4DATA_ALIGN
      S4LONG longVal[2] ;
   #endif

   for ( j = -1 ; j <= keysmax ; j++ ) /*-1 to handle nKeys */
   {
      *(short *)swapPtr = x4reverseShort( (void *)swapPtr ) ;
      swapPtr += sizeof(short) ;
   }

   #ifdef S4DATA_ALIGN
      if (   ( keysmax%2) || (grouplen%4) )
         for ( j = 0 ; j <= keysmax ; j++ )
         {
            memcpy( (void *)longVal, swapPtr, sizeof(longVal) ) ;
            longVal[0] = x4reverseLong( (void *)&longVal[0] ) ;
            longVal[1] = x4reverseLong( (void *)&longVal[1] ) ;
            memcpy( swapPtr, (void *)longVal, sizeof(longVal));
         }
      else
   #endif
   for ( j = 0 ; j <= keysmax ; j++ )
   {
      *(S4LONG *)swapPtr = x4reverseLong( (void *)swapPtr ) ;
      *(S4LONG *)(swapPtr + sizeof(S4LONG))= x4reverseLong( (void *)(swapPtr+sizeof(S4LONG)) ) ;
      swapPtr += grouplen ;
   }
}
#endif /*S4CLIPPER*/
#endif /*WORDS_BIGENDIAN*/


int S4FUNCTION expr4getReturnType( EXPR4 *expr, int infoOn )
{
   return v4functions[expr->info[infoOn].functionI].returnType ;
}
