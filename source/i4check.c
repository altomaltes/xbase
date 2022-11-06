/* i4check.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_INDEX

typedef struct
{
   F4FLAG flag ;

   TAG4FILE *tag ;
   char *oldKey ;
   long oldRec ;
   long numRecs ;
   int doCompare ;  /* Do not compare the first time */
   CODE4 *codeBase ;
   DATA4 *data ;
} C4CHECK ;

static int c4checkInit( C4CHECK *check, CODE4 *cb, TAG4FILE *t4, long nRecs, DATA4 *d4 )
{
   int rc ;

   memset( (void *)check, 0, sizeof(C4CHECK) ) ;

   rc = f4flagInit( &check->flag, cb, (unsigned long)nRecs ) ;
   if ( rc < 0 )
      return rc ;

   check->codeBase = cb ;
   check->tag = t4 ;
   check->numRecs = nRecs ;
   check->data = d4 ;

   check->oldKey = (char *)u4allocFree( t4->codeBase, (long)t4->header.keyLen ) ;
   if (check->oldKey == 0)
      return e4memory ;
   return 0 ;
}

static void c4checkFree( C4CHECK *c4 )
{
   u4free( c4->flag.flags ) ;
   u4free( c4->oldKey ) ;
}

static int c4checkRecord( C4CHECK *check )
{
   B4KEY_DATA *keyData ;
   TAG4FILE *t4 ;
   unsigned char *newPtr ;
   int len, rc ;
   #ifdef S4FOX
      int i ;
   #endif

   t4 = check->tag ;

   keyData = tfile4keyData( check->tag ) ;
   if ( keyData == 0 )
      return error4( check->codeBase, e4index, E95701 ) ;

   if ( keyData->num < 1  ||  keyData->num > check->numRecs )
      return error4describe( check->codeBase, e4index, E85703, check->tag->alias, (char *)0, (char *)0 ) ;

   if ( f4flagIsSet( &check->flag, (unsigned long)keyData->num) )
      return error4describe( check->codeBase, e4index, E85703, check->tag->alias, (char *)0, (char *)0 ) ;
   else
      f4flagSet( &check->flag, (unsigned long)keyData->num ) ;

   rc = d4go( check->data, keyData->num ) ;
   if ( rc < 0 )
      return rc ;
   rc = expr4context( t4->expr, check->data ) ;
   if ( rc < 0 )
      return rc ;

   len = tfile4exprKey( t4, &newPtr ) ;

   if ( len != t4->header.keyLen )
      return error4describe( check->codeBase, e4index, E85704, t4->alias, (char *)0, (char *)0 ) ;

   if ( t4->filter )
   {
      rc = expr4context( t4->filter, check->data ) ;
      if ( rc < 0 )
         return rc ;
      if ( !expr4true( t4->filter ) )  /* means record should not be in the index, but it is... */
         return error4describe( check->codeBase, e4index, E85705, t4->alias, (char *)0, (char *)0 ) ;
   }

   #ifdef S4MDX
      if ( expr4type( t4->expr ) == r4num )
      {
         if ( c4bcdCmp( newPtr, keyData->value, 0 ) != 0 )
            return error4describe( check->codeBase, e4index, E85705, t4->alias, (char *)0, (char *)0 ) ;
      }
      else
   #endif
   if ( c4memcmp( newPtr, keyData->value, (unsigned int)t4->header.keyLen ) != 0 )
      return error4describe( check->codeBase, e4index, E85705, t4->alias, (char *)0, (char *)0  ) ;

   #ifdef S4FOX
      /* blanks at the end of a key MUST be recorded as trailing blanks, and
         not included in the key.  Some early versions of CodeBase erroneously
         put blanks as part of the key and not as trails.  Catch that here
         because it causes problems with FoxPro and CodeBase */
      /* There is an exception in one version of FoxPro (2.6) which strangely
         included trail blanks in some instances */
      for( i = t4->header.keyLen ; i > 0 ; i-- )
         if ( keyData->value[i-1] != t4->pChar )
            break ;

      if ( d4version( check->data ) != 0x30 && check->codeBase->compatibility == 26 && t4->filter != 0 )
      {
         if ( x4trailCnt( tfile4block( t4 ), tfile4block( t4 )->keyOn ) != 0 )
            error4describe( check->codeBase, e4index, E85712, t4->alias, (char *)0, (char *)0 ) ;
      }
      else
      {
         if ( x4trailCnt( tfile4block( t4 ), tfile4block( t4 )->keyOn ) != (t4->header.keyLen - i ) )
            error4describe( check->codeBase, e4index, E85712, t4->alias, (char *)0, (char *)0 ) ;
      }
   #endif

   if ( check->doCompare )
   {
      #ifdef S4FOX
         #ifdef S4VFP_KEY
            if ( tfile4type( t4 ) != r4str )
               rc = c4memcmp( check->oldKey, newPtr, (unsigned int)t4->header.keyLen ) ;
            else
         #endif
               rc = u4keycmp( check->oldKey, newPtr, (unsigned int)t4->header.keyLen, (unsigned int)t4->header.keyLen, 0, &t4->vfpInfo ) ;
      #else
         rc = (*t4->cmp)( check->oldKey, newPtr, (unsigned int)t4->header.keyLen ) ;
      #endif

      if ( rc > 0)
         error4describe( check->codeBase, e4index, E85706, t4->alias, (char *)0, (char *)0 ) ;
      #ifdef S4FOX
         if ( rc == 0  &&  keyData->num <= check->oldRec )
            error4describe( check->codeBase, e4index, E85707, t4->alias, (char *)0, (char *)0 ) ;
      #endif /* S4FOX */

      #ifdef S4FOX
         if ( t4->header.typeCode & 0x01 )
      #else
         if ( t4->header.unique )
      #endif /* S4FOX */
      if ( rc == 0 )
         error4describe( check->codeBase, e4index, E85708, t4->alias, (char *)0, (char *)0 ) ;
   }
   else
      check->doCompare = 1 ;

   memcpy( check->oldKey, newPtr, (unsigned int)t4->header.keyLen ) ;

   check->oldRec = keyData->num ;

   if ( error4code( check->codeBase ) < 0 )
      return error4code( check->codeBase )  ;
   return 0 ;
}

#ifdef S4CLIPPER
static int tfile4blockCheck( TAG4FILE *t4, int firstTime )
{
   B4BLOCK *b4 ;
   int i, bType, rc ;
   CODE4 *c4 ;

   if ( firstTime )
      tfile4upToRoot( t4 ) ;

   c4 = t4->codeBase ;
   b4 = (B4BLOCK *)t4->blocks.lastNode ;
   if ( b4 == 0 )
      return 0 ;
   if ( b4->nKeys < t4->header.keysHalf && t4->header.root / 512 != b4->fileBlock )
      return error4describe( c4, e4index, E85709, tfile4alias( t4 ), (char *)0, (char *)0 ) ;
   if ( !b4leaf( b4 ) )
   {
      for ( i = 0 ; i <= b4->nKeys ; i++ )
      {
         b4->keyOn = i ;
         rc = tfile4down( t4 ) ;
         if ( rc != 0 )
            return error4describe( c4, e4index, E81601, tfile4alias( t4 ), 0, 0 ) ;
         if ( i == 0 )
            bType = b4leaf( (B4BLOCK *)t4->blocks.lastNode ) ;
         else
            if ( bType != b4leaf( (B4BLOCK *)t4->blocks.lastNode ) )
               return error4describe( c4, e4index, E85709, tfile4alias( t4 ), (char *)0, (char *)0 ) ;
         rc = tfile4blockCheck( t4, 0 ) ;
         if ( rc != 0 )
            return rc ;
         rc = tfile4up( t4 ) ;
         if ( rc != 0 )
            return error4describe( c4, e4index, E81601, tfile4alias( t4 ), (char *)0, (char *)0 ) ;
      }
   }
   return 0 ;
}
#endif

int t4check( TAG4 *t4 )
{
   C4CHECK check ;
   int rc, isRecord, keysSkip, rc2 ;
   CODE4 *c4 ;
   TAG4 *oldSelectedTag ;
   B4BLOCK *blockOn ;
   long baseSize, onRec ;
   unsigned char *ptr ;
   DATA4 *d4 ;
   #ifdef S4FOX
      unsigned char *tempVal ;
      long tempLong ;
   #endif
   #ifndef S4CLIPPER
      B4KEY_DATA *keyBranch, *keyLeaf ;
   #endif
   #ifdef S4PRINTF_OUT
      unsigned long loop ;
   #endif

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E95703 ) ;
   #endif

   d4 = t4->index->data ;
   c4 = d4->codeBase ;

   rc = expr4context( t4->tagFile->expr, d4 ) ;
   if ( rc < 0 )
      return rc ;
   if ( t4->tagFile->filter != 0 )
   {
      rc = expr4context( t4->tagFile->filter, d4 ) ;
      if ( rc < 0 )
         return rc ;
   }

   #ifndef S4OFF_MULTI
      #ifdef S4SERVER
         rc = dfile4lockFile( d4->dataFile, data4clientId( d4 ), data4serverId( d4 ) ) ;   /* returns -1 if error4code( codeBase ) < 0 */
      #else
         rc = d4lockFile( d4 ) ;   /* returns -1 if error4code( codeBase ) < 0 */
      #endif
      if ( rc != 0 )
         return rc ;
      #ifdef S4CLIPPER
         rc = i4lock( t4->index ) ;
      #else
         rc = index4lock( t4->tagFile->indexFile, data4serverId( d4 ) ) ;
      #endif
      if ( rc != 0 )
         return rc ;
      rc = d4refresh( d4 ) ;
      if ( rc != 0 )
         return rc ;
   #endif

   #ifndef S4OFF_WRITE
      rc = d4updateRecord( d4, 0 ) ;
      if ( rc < 0 )
         return rc ;
      if ( rc )
         return error4( c4, rc, E95703 ) ;
   #endif

   oldSelectedTag = d4tagSelected( d4 ) ;
   d4tagSelect( d4, t4 ) ;

   #ifdef S4CLIPPER
      rc = tfile4blockCheck( t4->tagFile, 1 ) ;
      if ( rc != 0 )
         return rc ;
   #endif

   baseSize = d4recCount( d4 ) ;
   if ( baseSize < 0L )
      return (int)baseSize ;

   rc = d4top( d4 ) ;
   if (rc < 0 )
      return rc ;
   if ( rc == 0 )
      rc = 1 ;

   if ( baseSize == 0L )
   {
      if ( tfile4skip( t4->tagFile, 1L ) == 0 )
      {
         d4tagSelect( d4, oldSelectedTag ) ;
         return( 0 ) ;
      }
      else
         return error4describe( c4, e4index, E85710, d4alias( d4 ), tfile4alias( t4->tagFile ), (char *)0 ) ;
   }

   rc2 = c4checkInit( &check, c4, t4->tagFile, baseSize, d4 ) ;
   if ( rc2 < 0 )
      return rc2 ;

   #ifdef S4PRINTF_OUT
      loop = 0 ;
      printf( "On Rec %10ld\n", loop ) ;
   #endif
   while ( rc == 1 )
   {
      rc = c4checkRecord( &check ) ;
      if ( rc )
         break ;
       rc = (int)tfile4skip( t4->tagFile, 1L ) ;
      if ( rc < 0 )
         break ;
      #ifdef S4PRINTF_OUT
         if ( (loop++ % 100) == 0 )
            printf( "\b\b\b\b\b\b\b\b\b\b%10ld", loop ) ;
      #endif
   }

   if ( rc < 0 )
   {
      c4checkFree( &check ) ;
      return rc ;
   }

   isRecord = 1 ;

   /* Now Test for Duplication */
   for ( onRec = 1;  onRec <= baseSize; onRec++)
   {
      if ( t4->tagFile->filter != 0 )
      {
         if ( d4go( d4, onRec ) < 0 )
            break ;
         rc2 = expr4context( t4->tagFile->filter, check.data ) ;
         if ( rc2 < 0 )
            return rc2 ;

         isRecord = expr4true( t4->tagFile->filter ) ;
      }

      if ( f4flagIsSet( &check.flag, (unsigned long)onRec ) )
      {
         if ( !isRecord )
         {
            error4describe( c4, e4index, E95703, t4->tagFile->alias, (char *)0, (char *)0 ) ;
            break ;
         }
      }
      else
      {
         if ( ! isRecord )
            continue ;

         #ifdef S4FOX
            if ( t4->tagFile->header.typeCode & 0x01 )
         #else
            if ( t4->tagFile->header.unique )
         #endif
            {
               if ( d4go(d4,onRec) < 0 )
                  break ;
               if ( expr4context( t4->tagFile->expr, check.data ) < 0 )
                  break ;
               if ( tfile4exprKey( t4->tagFile, &ptr) < 0 )
                  break ;
               if ( tfile4seek( t4->tagFile, ptr, expr4keyLen( t4->tagFile->expr ) ) == 0 )
                  continue ;
            }

         error4describe( c4, e4index, E85711, t4->tagFile->alias, 0, 0 ) ;
         break ;
      }
   }

   c4checkFree( &check ) ;
   if ( error4code( c4 ) < 0 )
      return error4code( c4 ) ;

   /* Now make sure the block key pointers match the blocks they point to. */
   /* This needs to be true for d4seek to function perfectly. */

   rc = d4bottom( d4 ) ;
   if ( rc < 0 )
      return rc ;

   if ( rc == 3 )
   {
      d4tagSelect( d4, oldSelectedTag ) ;
      return 0 ;
   }

   for(;;)
   {
      #ifdef S4FOX
         keysSkip = -tfile4block(t4->tagFile)->header.nKeys ;
      #else
         keysSkip = -tfile4block(t4->tagFile)->nKeys ;
      #endif

      rc = (int)tfile4skip( t4->tagFile, (long) keysSkip ) ;
      if ( error4code( c4 ) < 0 )
         return error4code( c4 ) ;
      if ( rc != keysSkip )
      {
         d4tagSelect( d4, oldSelectedTag ) ;
         return 0 ;
      }

      blockOn = (B4BLOCK *)t4->tagFile->blocks.lastNode ;
      if ( blockOn == 0 )
         return error4describe( c4, e4index, E85712, tfile4alias( t4->tagFile ), (char *)0, (char *)0 ) ;

      #ifdef S4FOX
         tempVal = (unsigned char *)u4allocFree( c4, (long)t4->tagFile->header.keyLen ) ;
         if ( tempVal == 0 )
            return error4stack( c4, e4memory, E95703 ) ;
         memcpy( tempVal, (void *)b4keyKey( blockOn, blockOn->keyOn ), (unsigned int)t4->tagFile->header.keyLen ) ;
         tempLong = b4recNo( blockOn, blockOn->keyOn ) ;

         if ( tfile4go( t4->tagFile, tempVal, tempLong, 0 ) )
         {
            u4free( tempVal ) ;
            return error4describe( c4, e4index, E85712, tfile4alias( t4->tagFile ), (char *)0, (char *)0 ) ;
         }
         u4free( tempVal ) ;
      #endif

      #ifndef S4CLIPPER
         for ( ;; )
         {
            blockOn = (B4BLOCK *)blockOn->link.p ;
            if ( blockOn == 0 )
               break ;
            if ( blockOn == (B4BLOCK *)t4->tagFile->blocks.lastNode )
               break ;

            #ifdef S4FOX
               if ( blockOn->keyOn < blockOn->header.nKeys )
            #else
               if ( blockOn->keyOn < blockOn->nKeys )
            #endif
               {
                  keyBranch = b4key( blockOn, blockOn->keyOn ) ;
                  keyLeaf = b4key( tfile4block( t4->tagFile ), tfile4block( t4->tagFile )->keyOn ) ;

                  if ( c4memcmp( keyBranch->value, keyLeaf->value, (unsigned int)t4->tagFile->header.keyLen) != 0 )
                     return error4describe( c4, e4index, E85712, tfile4alias( t4->tagFile ), (char *)0, (char *)0 ) ;

                  break ;
               }
         }
         if ( blockOn == 0 )
            return error4describe( c4, e4index, E85712, tfile4alias( t4->tagFile ), (char *)0, (char *)0 ) ;
      #endif
   }
}

#ifdef S4FOX
static int flag4blocks( TAG4FILE *t4, F4FLAG *f4, long *node1, long *node2, long *node3 )
{
   int i, rc ;
   B4BLOCK *blockOn ;
   long flagNo ;

   rc = tfile4down( t4 ) ;
   if ( rc < 0 )
      return rc ;
   if ( rc == 2 )
      return e4index ;
   if ( rc == 1 )
      return error4( t4->codeBase, e4index, E95704 ) ;

   blockOn = tfile4block(t4) ;

   flagNo = blockOn->fileBlock / B4BLOCK_SIZE ;
   if ( *node2 != -2 )
      if ( *node2 != blockOn->fileBlock )
         return error4( t4->codeBase, e4index, E81601 ) ;
   if ( *node1 != -2 )
      if ( *node1 != blockOn->header.leftNode )
         return error4( t4->codeBase, e4index, E81601 ) ;
   if ( *node3 != -2 )
      if ( *node3 != blockOn->header.rightNode )
         return error4( t4->codeBase, e4index, E81601 ) ;

   if ( f4flagIsSet( f4, (unsigned long)flagNo ) )
      return error4( t4->codeBase, e4index, E81601 ) ;

   rc = f4flagSet( f4, (unsigned long)flagNo ) ;
   if ( rc < 0 )
      return rc ;
   if ( ! b4leaf(blockOn) )
   {
      if ( blockOn->header.leftNode == -1 )
         *node1 = -1L ;
      else
         *node1 = -2L ;
      *node2 = b4key( blockOn, 0 )->num ;

      for( i = 0; i < blockOn->header.nKeys; i++ )
      {
         b4go( blockOn, (long)i ) ;
         if ( i == blockOn->header.nKeys - 1 && blockOn->header.rightNode == -1 )
            *node3 = -1 ;
         else
            *node3 = -2 ;
         rc = flag4blocks( t4, f4, node1, node2, node3 ) ;
         if ( rc < 0 )
            return rc ;
      }
   }

   *node1 = blockOn->fileBlock ;
   *node2 = blockOn->header.rightNode ;
   tfile4up(t4) ;
   return 0 ;
}
#else
static int flag4blocks( TAG4FILE *t4, F4FLAG *f4 )
{
   int i, rc ;
   B4BLOCK *blockOn ;
   long flagNo ;

   rc = tfile4down( t4 ) ;
   if ( rc < 0 )
      return rc ;
   if ( rc == 2 )
      return -1 ;
   if ( rc == 1 )
      return error4( t4->codeBase, e4index, E95704 ) ;

   blockOn = tfile4block(t4) ;

   #ifdef S4CLIPPER
      flagNo = (blockOn->fileBlock) * I4MULTIPLY / B4BLOCK_SIZE ;
   #else
      flagNo = (blockOn->fileBlock-4) * I4MULTIPLY / t4->indexFile->header.blockRw ;
   #endif

   if ( f4flagIsSet( f4, flagNo ) )
      return error4( t4->codeBase, e4index, E81601 ) ;

   rc = f4flagSet( f4, flagNo ) ;
   if ( rc < 0 )
      return rc ;
   if ( ! b4leaf(blockOn) )
   {
      #ifdef S4MDX
         for( i = 0; i <= blockOn->nKeys; i++ )
         {
            blockOn->keyOn = i ;
            rc = flag4blocks( t4, f4 ) ;
            if ( rc < 0 )
               return rc ;
         }
      #endif
      #ifdef S4CLIPPER
         for( i = 0; i <= blockOn->nKeys; i++ )
         {
            blockOn->keyOn = i ;
            rc = flag4blocks( t4, f4 ) ;
            if ( rc < 0 )
               return rc ;
         }
      #endif
   }

   tfile4up(t4) ;
   return 0 ;
}
#endif   /*  ifdef S4FOX   */

/* checks that all blocks in the file are on free list or are being used */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
static int i4checkBlocks( INDEX4 *i4 )
{
   #ifndef S4CLIPPER
      TAG4FILE *tagOn ;
      F4FLAG flags ;
      FILE4LONG pos ;
      long flagNo ;
      S4LONG totBlocks, freeBlock, eofBlockNo, len ;
      CODE4 *c4 ;
      #ifndef S4OFF_MULTI
         int rc ;
      #endif
      #ifdef S4FOX
         long node1, node2, node3 ;
      #endif
      #ifdef S4MDX
         T4DESC  desc[48] ;
         int i ;
      #endif

      c4 = i4->codeBase ;

      #ifndef S4OFF_MULTI
         rc = d4lockIndex( i4->data ) ;
         if ( rc < 0 )
            return rc ;
      #endif

      len = file4longGetLo( file4lenLow( &i4->indexFile->file ) ) ;

      #ifdef S4MDX
         totBlocks = ( len - 2048 ) / i4->indexFile->header.blockRw ;
      #else
         totBlocks = len / B4BLOCK_SIZE ;
      #endif

      /* First Flag for the Free Chain */
      f4flagInit( &flags, i4->codeBase, (unsigned long)totBlocks ) ;

      eofBlockNo = len/I4MULTIPLY ;
      #ifdef S4FOX
         for ( freeBlock = i4->indexFile->tagIndex->header.freeList ; freeBlock ; )
      #else
         for ( freeBlock = i4->indexFile->header.freeList ; freeBlock ; )
      #endif
         {
            if ( freeBlock == eofBlockNo  ||  error4code( c4 ) < 0 )
               break ;

            #ifdef S4MDX
               flagNo = (int)((freeBlock-4)*I4MULTIPLY/i4->indexFile->header.blockRw) ;
            #else
               flagNo = (int) (freeBlock / B4BLOCK_SIZE) ;
            #endif

            if ( freeBlock >= eofBlockNo  || f4flagIsSet(&flags, (unsigned long)flagNo ) )
            {
               error4( c4, e4index, E85701 ) ;
               break ;
            }
            f4flagSet( &flags, (unsigned long)flagNo ) ;

            #ifdef S4MDX
               file4longAssign( pos, freeBlock * I4MULTIPLY + sizeof(S4LONG), 0 ) ;
               file4readAllInternal( &i4->indexFile->file, pos, &freeBlock, sizeof( freeBlock ) ) ;
            #else
            file4longAssign( pos, freeBlock * I4MULTIPLY, 0 ) ;
               file4readAllInternal( &i4->indexFile->file, pos, &freeBlock, sizeof( freeBlock ) ) ;
            #endif

            #ifdef S4BYTE_SWAP
               freeBlock = x4reverseLong( (void *)&freeBlock ) ;
            #endif
         }

      #ifdef S4FOX
         /* do the header tag */
         tagOn = i4->indexFile->tagIndex ;
         flagNo = (int)((tagOn->headerOffset) / (long)B4BLOCK_SIZE) ;
         if ( f4flagIsSet( &flags, (unsigned long)flagNo ) )
            return error4( i4->codeBase, e4index, E81601 ) ;
         f4flagSet( &flags, (unsigned long)flagNo ) ;
         f4flagSet( &flags, (unsigned long)flagNo + 1L ) ;  /* tag header is 2 blocks long */

         if ( tfile4freeAll( tagOn ) >= 0 )
         {
            #ifdef S4FOX
               node1 = -1L ;
               node2 = tagOn->header.root ;
               node3 = -1L ;
               flag4blocks( tagOn, &flags, &node1, &node2, &node3 ) ;
            #else
               flag4blocks( tagOn, &flags ) ;
            #endif

            /* Now Flag for each block in each tag */
            for ( tagOn = 0 ;; )
            {
               tagOn = (TAG4FILE *)l4next( &i4->indexFile->tags,tagOn ) ;
               if ( tagOn == 0 )
                  break ;
               flagNo = (int)( tagOn->headerOffset / (long)B4BLOCK_SIZE) ;
               if ( f4flagIsSet( &flags, (unsigned long)flagNo ) )
                  return error4( i4->codeBase, e4index, E81601 ) ;
               f4flagSet( &flags, (unsigned long)flagNo ) ;
               f4flagSet( &flags, (unsigned long)flagNo + 1L ) ;  /* tag header is 2 blocks long */

               if ( tfile4freeAll( tagOn ) < 0 )
                  break ;
               #ifdef S4FOX
                  node1 = -1L ;
                  node2 = tagOn->header.root ;
                  node3 = -1L ;
                  if ( node2 == -1 )
                  {
                     file4longAssign( pos, tagOn->headerOffset, 0 ) ;
                     if ( file4readAllInternal( &i4->indexFile->file, pos, &node2, sizeof(node2)) < 0 )
                        return error4( i4->codeBase, e4index, E81601 ) ;
                     #ifdef S4BYTE_SWAP
                        node2 = x4reverseLong( (void *)&node2 ) ;
                     #endif
                  }
                  flag4blocks( tagOn, &flags, &node1, &node2, &node3 ) ;
               #else
                  flag4blocks( tagOn, &flags ) ;
               #endif
            }
         }
      #else
         /* Read header information to flag the tag header blocks */
         file4longAssign( pos, 512, 0 ) ;
         file4readAllInternal( &i4->indexFile->file, pos, desc, sizeof(desc) ) ;

         /* Now Flag for each block in each tag */
         i = 1 ;

         for ( tagOn = 0 ;; i++ )
         {
            tagOn = (TAG4FILE *)l4next( &i4->indexFile->tags, tagOn ) ;
            if ( tagOn == 0 )
               break ;
            #ifdef S4BYTE_SWAP
               desc[i].headerPos = x4reverseLong( (void *)&desc[i].headerPos ) ;
               desc[i].x1000 = 0x1000 ;
            #endif

            flagNo = (int) ((desc[i].headerPos * I4MULTIPLY - 2048) / (long) i4->indexFile->header.blockRw) ;
            if ( f4flagIsSet( &flags, flagNo ) )
               return error4( i4->codeBase, e4index, E81601 ) ;
            f4flagSet( &flags, flagNo ) ;

            if ( tfile4freeAll(tagOn) < 0 )
               break ;
            flag4blocks( tagOn, &flags ) ;
         }
      #endif

      if ( f4flagIsAllSet( &flags, 0UL, (unsigned long)totBlocks - 1L ) == 0 )
         error4( i4->codeBase, e4index, E85702 ) ;

      u4free( flags.flags ) ;
      if ( error4code( i4->codeBase ) < 0 )
          return error4code( i4->codeBase ) ;

   #endif
   return 0 ;
}

int i4check( INDEX4 *i4 )
{
   int rc ;
   TAG4 *tagOn ;
   #ifdef S4HAS_DESCENDING
      int oldDesc ;
   #endif

   #ifdef E4PARM_HIGH
      if ( i4 == 0 )
         return error4( 0, e4parm_null, E95705 ) ;
   #endif

   if ( error4code( i4->codeBase ) < 0 )
      return e4codeBase ;

   #ifndef S4OFF_WRITE
      #ifdef S4CLIPPER
         rc = i4update( i4 ) ;
      #else
         rc = index4update( i4->indexFile ) ;
      #endif
      if ( rc < 0 )
         return rc ;
   #endif

   rc = i4checkBlocks( i4 ) ;
   if ( rc < 0 )
      return rc ;

   for( tagOn = 0 ;; )
   {
      tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
      if ( tagOn == 0 )
         return 0 ;
      #ifdef S4HAS_DESCENDING
         oldDesc = tagOn->tagFile->header.descending ;
         tagOn->tagFile->header.descending = 0 ;   /* force ascending */
         rc = t4check( tagOn ) ;
         tagOn->tagFile->header.descending = (short)oldDesc ;   /* return to previous */
         if ( rc < 0 )
            return rc ;
      #else
         rc = t4check( tagOn ) ;
         if ( rc < 0 )
            return rc ;
      #endif
   }
}

#endif  /* S4OFF_INDEX */

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION d4check( DATA4 *d4 )
{
   #ifdef S4OFF_INDEX
      return 0 ;
   #else
      INDEX4 *indexOn ;
      TAG4 *oldTag ;
      int rc ;

      #ifdef E4PARM_HIGH
         if ( d4 == 0 )
            return error4( 0, e4parm_null, E95702 ) ;
      #endif

      oldTag = d4tagSelected( d4 ) ;

      #ifndef S4OFF_WRITE
         rc = d4updateRecord( d4, 0 ) ;
         if ( rc != 0 )  /* either an error or r4unique */
            return rc ;
      #endif

      #ifndef S4OFF_MULTI
         #ifdef S4SERVER
            rc = dfile4lockFile( d4->dataFile, data4clientId( d4 ), data4serverId( d4 ) ) ;   /* returns -1 if error4code( codeBase ) < 0 */
         #else
            rc = d4lockFile( d4 ) ;   /* returns -1 if error4code( codeBase ) < 0 */
         #endif
         if ( rc )
            return rc ;
      #endif

      #ifndef S4OFF_TRAN
         rc = tran4active( d4->codeBase, d4 ) ;
         if ( rc != 0 )
            return error4( d4->codeBase, rc, E81517 ) ;
      #endif

      for( indexOn = 0 ;; )
      {
         indexOn = (INDEX4 *)l4next( &d4->indexes, indexOn ) ;
         if ( indexOn == 0 )
         {
            rc = 0 ;
            break ;
         }
         rc = i4check( indexOn ) ;
         if ( rc < 0 )
         {
            rc = -1 ;
            break ;
         }
      }

      d4tagSelect( d4, oldTag ) ;
      return rc ;
   #endif  /* S4OFF_INDEX */
}


