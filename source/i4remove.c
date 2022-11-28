/* i4remove.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_WRITE
#ifndef S4OFF_INDEX

int tfile4remove( TAG4FILE *t4, const unsigned char *ptr, const long rec )
{
   int rc ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parmNull, E95404 ) ;
   #endif

   rc = tfile4go( t4, ptr, rec, 0 ) ;   /* returns -1 if error4code( codeBase ) < 0 */
   if ( rc < 0 )
       return error4stack( t4->codeBase, rc, E95404 ) ;
   if ( rc )
      return r4entry ;

   return tfile4removeCurrent( t4 ) ;
}

int tfile4removeCalc( TAG4FILE *t4, long rec )
{
   unsigned char *ptr ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parmNull, E95405 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   tfile4exprKey( t4, &ptr ) ;
   return tfile4remove( t4, ptr, rec ) ;
}

#ifndef S4CLIPPER

#ifdef S4FOX
/* remove the (current) branch block */
int tfile4removeBranch( TAG4FILE *t4, B4BLOCK *blockOn )
{
   long lNode, rNode ;
   INDEX4FILE *i4 ;
   FILE4LONG pos ;
   #ifdef WORDS_BIGENDIAN
      char *swapPtr ;
      S4LONG longVal ;
      short shortVal ;
      int i ;
   #endif

   i4 = t4->indexFile ;

   if ( blockOn == (B4BLOCK *)l4first( &t4->blocks ) )
   {
      /* Root Block, do not delete */
      #ifdef E4ANALYZE
         if ( blockOn->header.leftNode != -1 || blockOn->header.rightNode != -1 || blockOn->header.nKeys != 1 )
            return error4describe( t4->codeBase, e4info, E80401, tfile4alias( t4 ), i4->file.name, 0 ) ;
      #endif
      memset( blockOn->data, 0, B4BLOCK_SIZE - sizeof(B4STD_HEADER) - sizeof(B4NODE_HEADER) ) ;
      if ( !b4leaf( blockOn ) )   /* if not a leaf, then reset nodeHdr too */
      {
         memset( (void *)&blockOn->nodeHdr, 0, sizeof(B4NODE_HEADER) ) ;
         b4leafInit( blockOn ) ;
      }
      else
         blockOn->nodeHdr.freeSpace = B4BLOCK_SIZE - sizeof( B4STD_HEADER ) - sizeof( B4NODE_HEADER ) ;
      blockOn->header.nKeys = 0 ;
      blockOn->header.leftNode = -1 ;
      blockOn->header.rightNode = -1 ;
      blockOn->keyOn = -1 ;
      blockOn->builtOn = -1 ;
      blockOn->header.nodeAttribute = 3 ;  /* root and leaf */
      blockOn->changed = 1 ;
   }
   else /* This block is to be deleted */
   {
      lNode = blockOn->header.leftNode ;
      rNode = blockOn->header.rightNode ;
      l4remove( &t4->blocks, blockOn ) ;
      if ( index4shrink( i4, blockOn->fileBlock ) < 0 )
         return 0 ;
      blockOn->changed = 0 ;

      if ( lNode != -1L )
      {
         file4longAssign( pos, I4MULTIPLY * lNode, 0 ) ;
         if ( file4readAllInternal( &i4->file, pos, &blockOn->header, B4BLOCK_SIZE) < 0 )
            return 0 ;

         #ifdef WORDS_BIGENDIAN
            blockOn->header.nodeAttribute = x4reverseShort( (void *)&blockOn->header.nodeAttribute ) ;
            blockOn->header.nKeys = x4reverseShort( (void *)&blockOn->header.nKeys ) ;
            blockOn->header.leftNode = x4reverseLong( (void *)&blockOn->header.leftNode ) ;
            blockOn->header.rightNode = x4reverseLong( (void *)&blockOn->header.rightNode ) ;

            /* if block is a leaf */
            if (blockOn->header.nodeAttribute >= 2 )
            {
               blockOn->nodeHdr.freeSpace = x4reverseShort( (void *)&blockOn->nodeHdr.freeSpace ) ;
               longVal = x4reverseLong( (void *)&blockOn->nodeHdr.recNumMask[0] ) ;
               memcpy( (void *)&blockOn->nodeHdr.recNumMask[0], (void *)&longVal, sizeof(S4LONG) ) ;
            }
            else /* if block is a branch */
            {
               shortVal = blockOn->tag->header.keyLen + sizeof(S4LONG) ;
               /* position swapPtr to end of first key expression */
               swapPtr = (char *) &blockOn->nodeHdr.freeSpace + blockOn->tag->header.keyLen ;

               /* move through all B4KEY's to swap 'long's */
               for ( i = 0 ; i < (int) blockOn->header.nKeys ; i++ )
               {
                  longVal = x4reverseLong( (void *)swapPtr ) ;
                  memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
                  swapPtr += sizeof(S4LONG) ;
                  longVal = x4reverseLong( (void *)swapPtr ) ;
                  memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
                  swapPtr += shortVal ;
               }
            }
         #endif

         blockOn->fileBlock = lNode ;
         blockOn->header.rightNode = rNode ;
         blockOn->changed = 1 ;
         b4flush( blockOn ) ;
      }

      if ( rNode != -1L )
      {
         file4longAssign( pos, I4MULTIPLY * rNode, 0 ) ;
         if ( file4readAllInternal( &i4->file, pos, &blockOn->header, B4BLOCK_SIZE) < 0 )
            return 0 ;

         #ifdef WORDS_BIGENDIAN
            blockOn->header.nodeAttribute = x4reverseShort( (void *)&blockOn->header.nodeAttribute ) ;
            blockOn->header.nKeys = x4reverseShort( (void *)&blockOn->header.nKeys ) ;
            blockOn->header.leftNode = x4reverseLong( (void *)&blockOn->header.leftNode ) ;
            blockOn->header.rightNode = x4reverseLong( (void *)&blockOn->header.rightNode ) ;

            /* if block is a leaf */
            if (blockOn->header.nodeAttribute >= 2 )
            {
               blockOn->nodeHdr.freeSpace = x4reverseShort( (void *)&blockOn->nodeHdr.freeSpace ) ;
               longVal = x4reverseLong( (void *)blockOn->nodeHdr.recNumMask ) ;
               memcpy( (void *)blockOn->nodeHdr.recNumMask, (void *)&longVal, sizeof(S4LONG) ) ;
            }
            else /* if block is a branch */
            {
               shortVal = blockOn->tag->header.keyLen + sizeof(S4LONG) ;
               /* position swapPtr to end of first key expression */
               swapPtr = (char *) &blockOn->nodeHdr.freeSpace + blockOn->tag->header.keyLen ;

               /* move through all B4KEY's to swap 'long's */
               for ( i = 0 ; i < (int) blockOn->header.nKeys ; i++ )
               {
                  longVal = x4reverseLong( (void *)swapPtr ) ;
                  memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
                  swapPtr += sizeof(S4LONG) ;
                  longVal = x4reverseLong( (void *)swapPtr ) ;
                  memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
                  swapPtr += shortVal ;
               }
            }
         #endif

         blockOn->fileBlock = rNode ;
         blockOn->header.leftNode = lNode ;
         blockOn->changed = 1 ;
         b4flush( blockOn ) ;
      }

      return b4free( blockOn ) ;
   }

   return 0 ;
}
#endif

/* Remove the current key */
int tfile4removeCurrent( TAG4FILE *t4 )
{
   B4BLOCK *b4 ;
   INDEX4FILE *i4 ;
   #ifdef S4FOX
      void *newKeyInfo ;
      long rec = 0L ;
      int blRemoved, lessThanLast ;
   #else
      FILE4LONG pos ;
      int removeDone, updateReqd ;
      B4BLOCK *blockIterate ;
   #endif

   i4 = t4->indexFile ;

   #ifdef S4FOX
      i4->tagIndex->header.version = i4->versionOld+1 ;
      newKeyInfo = 0 ;

      for ( b4 = (B4BLOCK *)t4->blocks.lastNode ; b4 ; )
      {
         blRemoved = 0 ;
         if ( newKeyInfo == 0 )  /* then delete entry */
         {
            if ( b4lastpos( b4 ) == 0 )
            {
               if ( b4 != (B4BLOCK *)l4first( &t4->blocks ) )
                  blRemoved = 1 ;
               tfile4removeBranch( t4, b4 ) ;
               b4 = (B4BLOCK *)t4->blocks.lastNode ;
            }
            else
            {
               lessThanLast = 0 ;
               if ( b4->keyOn < b4lastpos( b4 ) )
                  lessThanLast = 1 ;
               b4remove( b4 ) ;
               if ( lessThanLast )
                  return 0 ;

               /* On last entry */
               b4goEof( b4 ) ;
               b4->keyOn-- ;  /* update keyOn for goint to last spot */
               newKeyInfo = b4keyKey( b4, b4->keyOn ) ;
               rec = b4recNo( b4, b4->keyOn ) ;
            }
         }
         else  /* Adjust entry */
         {
            if ( b4brReplace( b4, (unsigned char *)newKeyInfo, rec ) < 0 )
               return -1 ;
            if ( b4->keyOn != b4lastpos( b4 ) )  /* not on end key, so exit, else continue */
               return 0 ;
         }

         if ( !blRemoved )
         {
            b4 = (B4BLOCK *)b4->link.p ;
            if ( b4 == (B4BLOCK *)t4->blocks.lastNode )
               break ;
         }
      }
   #endif

   #ifdef S4MDX
      removeDone = 0 ;
      updateReqd = 0 ;
      i4->changed = 1 ;
      t4->changed = 1 ;
      t4->header.version++ ;

      blockIterate = (B4BLOCK *)l4last( &t4->blocks ) ;
      for ( ;; )
      {
         b4 = blockIterate ;
         if ( b4 == 0 )
            break ;
         blockIterate = (B4BLOCK *)l4prev( &t4->blocks, blockIterate ) ;  /* Calculate the previous block while the current block exists. */

         if ( !removeDone )  /* either removing or else updating */
         {
            if ( b4lastpos( b4 ) == 0 )  /* delete block */
            {
               if ( b4 == (B4BLOCK *)l4first( &t4->blocks ) )  /* root block, don't delete */
               {
                  b4->changed = 1 ;
                  b4->keyOn = 0 ;
                  memset( (void *)&b4->nKeys, 0, i4->header.blockRw ) ;
                  if (t4->filter )  /* must modify the filter setting for dBASE IV compatibility */
                  {
                     file4longAssign( pos, t4->headerOffset + sizeof( t4->header ) + 222, 0 ) ;
                     file4writeInternal( &t4->indexFile->file, pos, (char *)"\0", (int)1 ) ;
                     t4->hasKeys = 0 ;
                     #ifdef S4MDX
                        t4->hadKeys = 1 ;
                     #endif
                  }
                  return 0 ;
               }
               else
               {
                  l4remove( &t4->blocks, b4 ) ;
                  if ( index4shrink( i4, b4->fileBlock) < 0 )
                     return -1 ;
                  b4free( b4 ) ;
                  b4 = 0 ;
               }
            }
            else  /* just remove entry */
            {
               if ( blockIterate && b4->keyOn == b4lastpos( b4 ) && b4lastpos( b4 ) > 0 )  /* save to become last key of the block for parent update */
               {
                  memcpy( t4->codeBase->savedKey, b4keyKey( b4, b4->nKeys - 1 - b4leaf(b4) ), t4->header.keyLen ) ;
                  updateReqd = 1 ;
               }
               else
                  updateReqd = 0 ;

               b4remove( b4 ) ;
               if ( !b4leaf( b4 ) && b4lastpos( b4 ) == 0 )  /* branch with only one entry, so have the entry take place of this block */
               {
                  if (b4 == (B4BLOCK *)l4first( &t4->blocks ) )   /* the entry becomes the new root */
                  {
                     /* first update the tags root */
                     t4->header.root = b4key( b4, 0 )->num ;
                     #ifdef WORDS_BIGENDIAN
                        t4->header.root = x4reverseLong( (void *)&t4->header.root ) ;
                     #endif
                     file4longAssign( pos, t4->headerOffset, 0 ) ;
                     file4writeInternal( &t4->indexFile->file, pos, (void *)&t4->header.root, sizeof(t4->header.root) ) ;
                     #ifdef WORDS_BIGENDIAN
                        t4->header.root = x4reverseLong( (void *)&t4->header.root ) ;
                     #endif
                     updateReqd = 0 ;
                  }
                  else  /* remove this branch block */
                  {
                     blockIterate->changed = 1 ;
                     b4key( blockIterate, blockIterate->keyOn )->num = b4key( b4, 0 )->num ;
                  }
                  l4remove( &t4->blocks, b4 ) ;
                  if ( index4shrink( t4->indexFile, b4->fileBlock) < 0 )
                      return -1 ;
                  b4->changed = 0 ;
                  b4free( b4 ) ;
                  b4 = 0 ;
               }
               else
                  b4->keyOn = b4lastpos( b4 ) ;

               if ( !updateReqd )
                  return 0 ;
               removeDone = 1 ;
            }
         }
         else  /* Adjust entry - at most one update will be required in MDX */
         {
            if ( b4->keyOn < b4lastpos( b4 ) )
            {
               b4->changed = 1 ;
               memcpy( b4keyKey( b4, b4->keyOn), t4->codeBase->savedKey, t4->header.keyLen ) ;
               return 0 ;
            }
         }
      }
   #endif

   return 0 ;
}

#endif   /* ifndef S4CLIPPER  */

#ifdef S4CLIPPER
int tfile4balanceBranch( TAG4FILE *, B4BLOCK * ) ;

static int tfile4removeRef( TAG4FILE *t4 )
{
   B4KEY_DATA *myKeyData ;
   B4BLOCK *blockOn, *blockUp, *neighborBlock ;
   long reference, neighborRef ;
   int i, rc ;

   blockOn = (B4BLOCK *)t4->blocks.lastNode ;

   tfile4up( t4 ) ;
   blockUp = (B4BLOCK *)t4->blocks.lastNode ;

   if ( blockUp == 0 )  /* root block only, so reference doesn't exist */
   {
      /* reset the block */
      short offset = ( t4->header.keysMax + 2 + ( (t4->header.keysMax/2)*2 != t4->header.keysMax ) ) * sizeof(short) ;
      for ( i = 0 ; i <= t4->header.keysMax ; i++ )
         blockOn->pointers[i] = (short)( t4->header.groupLen * i ) + offset ;
      return 0 ;
   }
   else  /* delete the block */
   {
      tfile4shrink( t4, blockOn->fileBlock ) ;
      l4pop( &t4->saved ) ;
      blockOn->changed = 0 ;
      b4free( blockOn ) ;
      blockOn = 0 ;
   }
   blockOn = blockUp ;

   myKeyData = (B4KEY_DATA *)u4allocEr( t4->codeBase, t4->header.groupLen ) ;
   if ( myKeyData == 0 )
      return e4memory ;
   if ( blockOn->keyOn >= blockOn->nKeys )
      memcpy( &(myKeyData->num), &( b4key( blockOn, blockOn->keyOn - 1 )->num), sizeof(S4LONG) + t4->header.keyLen ) ;
   else
      memcpy( &(myKeyData->num), &( b4key( blockOn, blockOn->keyOn )->num), sizeof(S4LONG) + t4->header.keyLen ) ;

   if ( blockOn->nKeys == 1 )
   {
      /* take the branch, and put in place of me, then delete and add me */
      if ( blockOn->keyOn >= blockOn->nKeys )
         reference = (long)b4key( blockOn, 0 )->pointer ;
      else
         reference = (long)b4key( blockOn, 1 )->pointer ;
      b4remove( blockOn ) ;
      if ( t4->blocks.nLink == 1 )  /* root block, so reference will take over */
      {
         tfile4shrink( t4, blockOn->fileBlock ) ;
         tfile4up( t4 ) ;
         l4pop(&t4->saved ) ;
         blockOn->changed = 0 ;
         b4free( blockOn ) ;
         blockOn = (B4BLOCK *)t4->blocks.lastNode ;
         t4->header.root = reference ;
      }
      else   /* special case -- blocks contain very large keys.  try balancing */
      {
         tfile4up( t4 ) ;
         blockUp = (B4BLOCK *)t4->blocks.lastNode ;
         if ( blockUp->keyOn == 0 )
         {
            /* move one right neighbour into left */
            neighborRef = (long)b4key( blockUp, 1 )->pointer ;
            neighborBlock = b4alloc( t4, neighborRef ) ;
            if ( neighborBlock == 0 )
            {
               u4free( myKeyData ) ;
               return e4memory ;
            }
            if ( i4readBlock( &t4->file, neighborRef, 0, neighborBlock ) < 0 )
            {
               b4free( neighborBlock ) ;
               neighborBlock = 0 ;
               u4free( myKeyData ) ;
               return -1 ;
            }
            if ( neighborBlock->nKeys == t4->header.keysMax )  /* already maxed out, must shift */
            {
               /* borrow for the current block */
               reference = b4key( blockOn, 0 )->pointer ;
               blockOn->keyOn = 0 ;
               b4insert( blockOn, b4keyKey( blockUp, 0 ), b4recNo( blockUp, 0 ), reference / 512 ) ;
               memcpy( b4keyKey( blockUp, 0 ), b4keyKey( neighborBlock, 0 ), t4->header.keyLen ) ;
               b4key( blockUp, 0 )->num = b4key( neighborBlock, 0 )->num ;
               b4key( blockOn, 1 )->pointer = b4key( neighborBlock, 0 )->pointer ;
               neighborBlock->keyOn = 0 ;
               b4remove( neighborBlock ) ;
               blockUp->changed = 1 ;
               blockOn->changed = 1 ;
               neighborBlock->changed = 1 ;
               b4flush( neighborBlock ) ;
               b4free( neighborBlock ) ;
               neighborBlock = 0 ;
            }
            else
            {
               /* move current reference over, and delete from parent */
               neighborBlock->keyOn = 0 ;
               b4insert( neighborBlock, b4keyKey( blockUp, blockUp->keyOn ), b4key( blockUp, blockUp->keyOn )->num, b4key( blockOn, 0 )->pointer / 512 ) ;
               b4remove( blockUp ) ;
               #ifdef E4INDEX_VERIFY
                  if ( (B4BLOCK *)t4->saved.lastNode != blockOn )
                     return error4( t4->codeBase, e4index, E81601 ) ;
               #endif
               l4pop( &t4->saved ) ;
               blockOn->changed = 0 ;
               tfile4shrink( t4, blockOn->fileBlock ) ;
               b4free( blockOn ) ;
               blockOn = 0 ;
               blockUp->changed = 1 ;
               neighborBlock->changed = 1 ;

               /* now check that parent is still valid */
               if ( blockUp->nKeys == 0 ) /* just removed last entry... */
               {
                  if ( t4->header.root == blockUp->fileBlock * 512 )
                  {
                     tfile4shrink( t4, blockUp->fileBlock ) ;
                     l4pop(&t4->blocks ) ;  /* remov ourselves */
                     reference = (long)b4key( blockUp, 0 )->pointer ;
                     blockUp->changed = 0 ;
                     b4free( blockUp ) ;
                     blockOn = 0 ;
                     l4add( &t4->blocks, neighborBlock ) ;
                     t4->header.root = reference ;
                  }
                  else
                  {
                     b4flush( neighborBlock ) ;
                     b4free( neighborBlock ) ;
                     neighborBlock = 0 ;
                     tfile4balanceBranch( t4, blockUp ) ;
                  }
               }
               else  /* save and free */
               {
                  b4flush( neighborBlock ) ;
                  b4free( neighborBlock ) ;
                  neighborBlock = 0 ;
               }
            }
         }
         else
         {
            /* move to left neighbor */
            neighborRef = (long)b4key( blockUp, blockUp->keyOn - 1 )->pointer ;
            neighborBlock = b4alloc( t4, neighborRef ) ;
            if ( neighborBlock == 0 )
            {
               u4free( myKeyData ) ;
               return e4memory ;
            }
            if ( i4readBlock( &t4->file, neighborRef, 0, neighborBlock ) < 0 )
            {
               b4free( neighborBlock ) ;
               neighborBlock = 0 ;
               u4free( myKeyData ) ;
               return -1 ;
            }
            if ( neighborBlock->nKeys == t4->header.keysMax )  /* already maxed out */
            {
               /* borrow for the current block */
               blockOn->keyOn = 0 ;
               reference = b4key( blockOn, 0 )->pointer ;
               b4insert( blockOn, b4keyKey( blockUp, blockUp->keyOn - 1 ), b4key( blockUp, blockUp->keyOn - 1 )->num, b4key( neighborBlock, neighborBlock->nKeys )->pointer / 512 ) ;
               memcpy( b4keyKey( blockUp, blockUp->keyOn - 1 ), b4keyKey( neighborBlock, neighborBlock->nKeys - 1 ), t4->header.keyLen ) ;
               b4key( blockUp, blockUp->keyOn - 1 )->num = b4key( neighborBlock, neighborBlock->nKeys - 1 )->num ;
               b4key( blockOn, 1 )->pointer = reference ;
               neighborBlock->nKeys-- ;
               blockUp->changed = 1 ;
               blockOn->changed = 1 ;
               neighborBlock->changed = 1 ;
               b4flush( neighborBlock ) ;
               b4free( neighborBlock ) ;
               neighborBlock = 0 ;
            }
            else
            {
               /* move current reference over, and delete from parent */
               neighborBlock->keyOn = neighborBlock->nKeys ;
               /* just re-insert the last key, then change the values */
               b4insert( neighborBlock, b4keyKey( neighborBlock, neighborBlock->keyOn - 1 ),
                         b4key( neighborBlock, neighborBlock->keyOn - 1 )->num,
                         b4key( neighborBlock, neighborBlock->keyOn )->pointer / 512 ) ;

               memcpy( b4keyKey( neighborBlock, neighborBlock->nKeys - 1 ), b4keyKey( blockUp, blockUp->keyOn - 1 ), t4->header.keyLen ) ;
               b4key( neighborBlock, neighborBlock->nKeys - 1 )->num = b4key( blockUp, blockUp->keyOn - 1 )->num ;
               b4key( neighborBlock, neighborBlock->nKeys )->pointer = b4key( blockOn, 0 )->pointer ;
               #ifdef E4INDEX_VERIFY
                  if ( (B4BLOCK *)t4->saved.lastNode != blockOn )
                     return error4( t4->codeBase, e4index, E81601 ) ;
               #endif
               l4pop( &t4->saved ) ;
               blockOn->changed = 0 ;
               tfile4shrink( t4, blockOn->fileBlock ) ;
               b4free( blockOn ) ;
               blockOn = 0 ;
               b4key( blockUp, blockUp->keyOn )->pointer = b4key( blockUp, blockUp->keyOn - 1 )->pointer ;
               blockUp->keyOn-- ;
               b4remove( blockUp ) ;
               blockUp->changed = 1 ;
               neighborBlock->changed = 1 ;

               /* now check that parent is still valid */
               if ( blockUp->nKeys == 0 ) /* just removed last entry... */
               {
                  if ( t4->header.root == blockUp->fileBlock * 512 )
                  {
                     reference = (long)b4key( blockUp, 0 )->pointer ;
                     tfile4shrink( t4, blockUp->fileBlock ) ;
                     l4pop(&t4->blocks ) ;  /* remov ourselves */
                     blockUp->changed = 0 ;
                     b4free( blockUp ) ;
                     blockUp = 0 ;
                     l4add( &t4->blocks, neighborBlock ) ;
                     t4->header.root = reference ;
                  }
                  else
                  {
                     b4flush( neighborBlock ) ;
                     b4free( neighborBlock ) ;
                     neighborBlock = 0 ;
                     tfile4balanceBranch( t4, blockUp ) ;
                  }
               }
               else  /* save and free */
               {
                  b4flush( neighborBlock ) ;
                  b4free( neighborBlock ) ;
                  neighborBlock = 0 ;
               }
            }
         }
      }
   }
   else    /* just remove myself, add later */
   {
      b4remove( blockOn ) ;
      if ( blockOn->nKeys < t4->header.keysHalf && blockOn->fileBlock != t4->header.root )  /* if not root may have to balance the tree */
      {
         if ( tfile4balanceBranch( t4, blockOn ) < 0 )
         {
            u4free( myKeyData ) ;
            return -1 ;
         }
      }
   }

   /* now add the removed reference back into the index */
   rc = tfile4add( t4, (unsigned char *)myKeyData->value, myKeyData->num, 0 ) ;

   u4free( myKeyData ) ;

   return rc ;
}

static int tfile4balanceBlock( TAG4FILE *t4, B4BLOCK *parent, B4BLOCK *b4, B4BLOCK *b4temp, char isRight, char balanceMode )
{
   B4KEY_DATA *bdata, *bdata2 ;
   int avg, i ;
   long tempFb, reference ;

   if ( balanceMode == 2 && b4leaf( b4temp ) )
      return error4( t4->codeBase, e4index, E81601 ) ;
   else
   {
      if ( b4temp->nKeys + b4->nKeys < t4->header.keysMax )
      {
         if ( isRight )
         {
            if ( balanceMode == 2 )
            {
               b4->keyOn = 0 ;
               parent->keyOn-- ;
               bdata = b4key( parent, parent->keyOn ) ;
               if ( b4->keyOn == 0 && b4->nKeys == 0 )  /* must do reference as well */
               {
                  reference = b4key( b4, 0 )->pointer ;
                  b4insert( b4, bdata->value, bdata->num, b4key( b4temp, b4temp->nKeys )->pointer / 512 ) ;
                  b4key( b4, 1 ) ->pointer = reference ;
               }
               else
                  b4insert( b4, bdata->value, bdata->num, b4key( b4temp, b4temp->nKeys )->pointer / 512 ) ;
               for ( i = b4temp->nKeys - 1 ; i >= 0 ; i-- )
               {
                  b4->keyOn = 0 ;
                  bdata = b4key( b4temp, i ) ;
                  b4insert( b4, bdata->value, bdata->num, bdata->pointer / 512 ) ;
               }
               b4temp->changed = 0 ;
               tfile4shrink( t4, b4temp->fileBlock ) ;
               b4remove( parent ) ;
               if ( parent->nKeys < t4->header.keysHalf )
                  tfile4balanceBranch( t4, parent ) ;
            }
            else
            {
               for ( i = 0 ; i < b4->nKeys ; i++ )
               {
                  b4goEof( b4temp ) ;
                  bdata = b4key( b4, i ) ;
                  b4insert( b4temp, bdata->value, bdata->num, 0L ) ;
               }

               t4->codeBase->doIndexVerify = 0 ;  /* avoid verify errors due to our partial removal */
               do
               {
                  tfile4down( t4 ) ;
                  if ( t4->blocks.lastNode == 0 )
                     break ;
               } while( !b4leaf( (B4BLOCK *)t4->blocks.lastNode ) ) ;  /* don't use tfile4block() due to borland problems */
               t4->codeBase->doIndexVerify = 1 ;

               b4flush( b4temp ) ;
               if ( tfile4removeRef( t4 ) != 0 )  /* will delete b4 and will remove and re-add the reference if required */
                  return -1 ;
            }
         }
         else
         {
            /* put parent entry, then all but one child, then last child to parent */
            b4goEof( b4 ) ;
            if ( balanceMode == 2 )
            {
               b4temp->keyOn = 0 ;
               bdata = b4key( parent, parent->keyOn ) ;
               b4insert( b4temp, bdata->value, bdata->num, b4key( b4, b4->nKeys )->pointer / 512 ) ;
               for ( i = b4->nKeys - 1 ; i >= 0 ; i-- )
               {
                  b4temp->keyOn = 0 ;
                  bdata = b4key( b4, i ) ;
                  b4insert( b4temp, bdata->value, bdata->num, bdata->pointer / 512 ) ;
               }
               tfile4shrink( t4, b4->fileBlock ) ;
               l4pop( &t4->saved ) ;
               b4->changed = 0 ;
               b4free( b4 ) ;
               b4 = 0 ;
               b4flush( b4temp ) ;
               b4remove( parent ) ;
               if ( parent->nKeys < t4->header.keysHalf )
                  tfile4balanceBranch( t4, parent ) ;
            }
            else
            {
               bdata = b4key( parent, parent->keyOn ) ;
               if ( b4->keyOn == 0 && b4->nKeys == 0 )  /* must do reference as well */
               {
                  reference = b4key( b4, 0 )->pointer ;
                  b4insert( b4, bdata->value, bdata->num, 0L ) ;
                  b4key( b4, 1 ) ->pointer = reference ;
               }
               else
                  b4insert( b4, bdata->value, bdata->num, 0L ) ;
               for ( i = 0 ; i < b4temp->nKeys - 1 ; i++ )
               {
                  b4goEof( b4 ) ;
                  bdata = b4key( b4temp, i ) ;
                  b4insert( b4, bdata->value, bdata->num, 0L ) ;
               }
               bdata = b4key( parent, parent->keyOn ) ;
               bdata2 = b4key( b4temp, b4temp->nKeys - 1 ) ;
               memcpy( bdata->value, bdata2->value, t4->header.keyLen )  ;
               memcpy( &( bdata->num ), &bdata2->num, sizeof(S4LONG ) )  ;

               parent->keyOn++ ;
               parent->changed = 1 ;

               t4->codeBase->doIndexVerify = 0 ;  /* avoid verify errors due to our partial removal */
               b4flush( b4 ) ;
               do
               {
                  if ( tfile4down( t4 ) == 1 )
                     break ;
               } while( ((B4BLOCK *)t4->blocks.lastNode)->fileBlock != b4temp->fileBlock ) ;
               t4->codeBase->doIndexVerify = 1 ;
               if ( tfile4removeRef( t4 ) != 0 )  /* will delete b4temp and will remove and re-add the reference if required */
                  return -1 ;
               b4temp->changed = 0 ;
            }
         }
      }
      else  /* will have to redistribute keys */
      {
         avg = ( b4->nKeys + b4temp->nKeys + 1 ) / 2 ;
         if ( avg < t4->header.keysHalf )
            avg = t4->header.keysHalf ;
         if ( isRight )
         {
            b4->keyOn = 0 ;
            parent->keyOn-- ;
            bdata = b4key( parent, parent->keyOn ) ;

            if ( balanceMode == 2 )
            {
               if ( b4->nKeys == 0 )
               {
                  tempFb = b4key( b4, 0 )->pointer ;
                  b4insert( b4, bdata->value, bdata->num, b4key( b4temp, b4temp->nKeys )->pointer / 512 ) ;
                  b4append( b4, tempFb / 512 ) ;
               }
               else
                  b4insert( b4, bdata->value, bdata->num, b4key( b4temp, b4temp->nKeys )->pointer / 512 ) ;
            }
            else
               b4insert( b4, bdata->value, bdata->num, 0L ) ;

            while ( b4temp->nKeys > avg && b4->nKeys < avg )
            {
               b4->keyOn = 0 ;
               b4temp->keyOn = b4temp->nKeys - 1 ;
               bdata = b4key( b4temp, b4temp->keyOn ) ;
               if ( balanceMode == 2 )
                  b4insert( b4, bdata->value, bdata->num, bdata->pointer / 512 ) ;
               else
                  b4insert( b4, bdata->value, bdata->num, 0L ) ;
               b4remove( b4temp ) ;
            }
            b4temp->keyOn = b4temp->nKeys - 1 ;
         }
         else
         {
            b4goEof( b4 ) ;
            if ( balanceMode == 2 )
            {
               bdata = b4key( b4, b4->nKeys ) ;
               bdata2 = b4key( parent, parent->keyOn ) ;
               memcpy( bdata->value, bdata2->value, t4->header.keyLen )  ;
               memcpy( &(bdata->num), &bdata2->num, sizeof(S4LONG ) )  ;
               b4->nKeys++ ;
            }
            else
            {
               bdata = b4key( parent, parent->keyOn ) ;
               b4insert( b4, bdata->value, bdata->num, 0L ) ;
            }

            while ( b4->nKeys - (balanceMode == 2) < avg )
            {
               b4goEof( b4 ) ;
               b4temp->keyOn = 0 ;
               bdata = b4key( b4temp, b4temp->keyOn ) ;
               if ( balanceMode == 2 )
                  b4insert( b4, bdata->value, bdata->num, bdata->pointer / 512 ) ;
               else
                  b4insert( b4, bdata->value, bdata->num, 0L ) ;
               b4remove( b4temp ) ;
            }

            if ( balanceMode == 2 )
            {
               b4->nKeys-- ;
               b4->keyOn-- ;
            }
            b4temp->keyOn = 0 ;
         }
         /* and add a key back to the parent */
         bdata = b4key( parent, parent->keyOn ) ;
         if ( balanceMode == 2 )
         {
            if ( isRight )
               bdata2 = b4key( b4temp, b4temp->nKeys - 1 ) ;
            else
               bdata2 = b4key( b4, b4->nKeys ) ;
            memcpy( bdata->value, bdata2->value, t4->header.keyLen )  ;
            memcpy( &(bdata->num), &bdata2->num, sizeof(S4LONG ) )  ;
            if ( isRight )
            {
               b4temp->keyOn = b4temp->nKeys ;
               b4remove( b4temp ) ;
            }
         }
         else
         {
            bdata2 = b4key( b4temp, b4temp->keyOn ) ;
            memcpy( bdata->value, bdata2->value, t4->header.keyLen )  ;
            memcpy( &(bdata->num), &bdata2->num, sizeof(S4LONG ) )  ;
            b4remove( b4temp ) ;
         }
         parent->changed = 1 ;
         b4flush( b4temp ) ;
      }
   }
   return 0 ;
}

static int tfile4balanceBranchLeaf( TAG4FILE *t4, B4BLOCK *parent, B4BLOCK *branch, B4BLOCK *leaf )
{
   B4BLOCK *b4temp ;
   long newFileBlock ;
   B4KEY_DATA *key ;

   t4->header.version++ ;
   newFileBlock = tfile4extend( t4 ) ;
   b4temp = b4alloc( t4, newFileBlock ) ;
   if ( b4temp == 0 )
      return -1 ;

   key = b4key( b4temp, 0 ) ;
   key->pointer =  leaf->fileBlock * I4MULTIPLY ;
   b4temp->changed = 1 ;
   key = b4key( parent, parent->keyOn ) ;
   #ifdef E4ANALYZE_ALL
      if ( key->pointer != leaf->fileBlock * I4MULTIPLY )
         return error4( t4->codeBase, e4index, E95407 ) ;
   #endif
   key->pointer = newFileBlock * I4MULTIPLY ;
   parent->changed = 1 ;
   l4add( &t4->blocks, b4temp ) ;

   return tfile4balanceBlock( t4, parent, b4temp, branch, 1, 2 ) ;
}

/* if doFull is true, the whole branch set will be balanced, not just the current leaf block and reqd.  Also, this will balance top down instead of the other way. */
int tfile4balance( TAG4FILE *t4, B4BLOCK *b4, int doFull )
{
   B4BLOCK *b4temp, *parent ;
   long tempFb ;
   int rc, noKeys, doRightSpecial ;
   char isRight, balanceMode ;  /* balanceMode = 0 for done, 1 for leaf, and 2 for branch */
   B4KEY_DATA *myKeyData ;

   if ( !b4leaf( b4 ) )
      return 0 ;

   b4temp = b4alloc( t4, 0L ) ;
   if ( b4temp == 0 )
      return -1 ;

   if ( doFull )
   {
      tfile4upToRoot( t4 ) ;
      balanceMode = 2 ;  /* branch */
   }
   else
      balanceMode = 1 ;  /* leaf */

   noKeys = 0 ;
   while ( balanceMode != 0 || doFull )
   {
      if ( doFull )
      {
         parent = (B4BLOCK *)t4->blocks.lastNode ;
         if ( noKeys == 0 )
            parent->keyOn = parent->nKeys ;
         rc = tfile4down( t4 ) ;
         if ( rc == 1 )  /* make sure siblings are also leafs */
         {
            tfile4up( t4 ) ;
            parent = (B4BLOCK *)t4->blocks.lastNode ;
            if ( parent != 0 )
            {
               if ( parent->keyOn > 0 )
               {
                  tempFb = (long)b4key( parent, parent->keyOn - 1 )->pointer ;
                  if ( i4readBlock( &t4->file, tempFb, 0, b4temp ) < 0 )
                  {
                     b4free( b4temp ) ;
                     return -1 ;
                  }
                  if ( b4leaf( b4temp ) == 0 &&  b4leaf( b4 ) == 1 )
                  {
                     rc = tfile4balanceBranchLeaf( t4, parent, b4temp, b4 ) ;
                     b4free( b4temp ) ;
                     return rc ;
                  }
               }
            }

            break ;
         }
         else
            if ( noKeys == 1 )  /* need to check the siblings to make sure also branches */
            {
               tfile4up( t4 ) ;
               parent = (B4BLOCK *)t4->blocks.lastNode ;
               if ( parent != 0 )
               {
                  if ( parent->keyOn > 0 )
                  {
                     tempFb = (long)b4key( parent, parent->keyOn - 1 )->pointer ;
                     if ( i4readBlock( &t4->file, tempFb, 0, b4temp ) < 0 )
                     {
                        b4free( b4temp ) ;
                        return -1 ;
                     }
                     if ( b4leaf( b4temp ) == 0 &&  b4leaf( b4 ) == 1 )
                     {
                        rc = tfile4balanceBranchLeaf( t4, parent, b4temp, b4 ) ;
                        b4free( b4temp ) ;
                        return rc ;
                     }
                     if ( b4leaf( b4temp ) == 1 &&  b4leaf( b4 ) == 0 )
                     {
                        rc = tfile4balanceBranchLeaf( t4, parent, b4, b4temp ) ;
                        b4free( b4temp ) ;
                        return rc ;
                     }
                  }
                  else
                  {
                     parent->keyOn = 0 ;
                     tempFb = (long)b4key( parent, parent->keyOn + 1 )->pointer ;
                     if ( i4readBlock( &t4->file, tempFb, 0, b4temp ) < 0 )
                     {
                        b4free( b4temp ) ;
                        return -1 ;
                     }
                     if ( b4leaf( b4temp ) == 0 &&  b4leaf( b4 ) == 1 )
                     {
                        rc = tfile4balanceBranchLeaf( t4, parent, b4temp, b4 ) ;
                        b4free( b4temp ) ;
                        return rc ;
                     }
                     if ( b4leaf( b4temp ) == 1 &&  b4leaf( b4 ) == 0 )
                     {
                        rc = tfile4balanceBranchLeaf( t4, parent, b4, b4temp ) ;
                        b4free( b4temp ) ;
                        return rc ;
                     }
                  }
               }

               if ( b4leaf( b4 ) )
                  break ;
               tfile4down( t4 ) ;
               tfile4down( t4 ) ;
               b4 = (B4BLOCK *)t4->blocks.lastNode ;
               tfile4up( t4 ) ;
               continue ;
            }

         if ( rc < 0 )
            return -1 ;

         b4 = (B4BLOCK *)t4->blocks.lastNode ;
         b4->keyOn = b4->nKeys ;

         if ( b4->nKeys >= t4->header.keysHalf )
            continue ;

         if ( b4leaf( b4 ) )
            balanceMode = 1 ;
         tempFb = b4key( parent, parent->keyOn - 1 )->pointer ;
         isRight = 1 ;
      }
      else
      {
         if ( balanceMode == 2 )  /* branch */
         {
            b4 = parent ;
            while( t4->blocks.lastNode != (LINK4 *)b4 )  /* re-align ourselves */
               tfile4up( t4 ) ;
         }
         tfile4up( t4 ) ;
         parent = (B4BLOCK *)t4->blocks.lastNode ;

         if ( parent == 0 )  /* root block */
         {
            tfile4down( t4 ) ;
            parent = (B4BLOCK *)t4->blocks.lastNode ;
            if ( !b4leaf( parent ) && parent->nKeys == 0 )  /* remove */
            {
               tempFb = (long)b4key( parent, 0 )->pointer ;
               tfile4shrink( t4, parent->fileBlock ) ;
               tfile4up( t4 ) ;
               l4pop( &t4->saved ) ;
               parent->changed = 0 ;
               b4free( parent ) ;
               parent = 0 ;
               t4->header.root = tempFb ;
               tfile4down( t4 ) ;
            }
            break ;
         }

         if ( parent->nKeys == 0 )  /* try to replace parent with ourself */
            return error4describe( t4->codeBase, e4index, E81601, tfile4alias( t4 ), 0, 0 ) ;

         if ( b4->nKeys >= t4->header.keysHalf )
         {
            balanceMode = 0 ;
            continue ;
         }
         isRight = (parent->keyOn == parent->nKeys ) ;
         if ( isRight )
            tempFb = b4key( parent, parent->keyOn - 1 )->pointer ;
         else
            tempFb = b4key( parent, parent->keyOn + 1 )->pointer ;
      }

      b4temp->fileBlock = tempFb ;
      for ( ;; )
      {
         if ( i4readBlock( &t4->file, tempFb, 0, b4temp ) < 0 )
         {
            b4free( b4temp ) ;
            return -1 ;
         }

         if ( balanceMode == 2 || b4leaf( b4temp ) )  /* if branch mode or leaf mode and leaf block found */
            break ;

         if ( isRight )
            tempFb = b4key( b4temp, b4temp->nKeys )->pointer ;
         else
            tempFb = b4key( b4temp, 0 )->pointer ;
      }

      /* save a key position for after-seek */
      myKeyData = (B4KEY_DATA *)u4allocEr( t4->codeBase, t4->header.groupLen ) ;
      if ( myKeyData == 0 )
          return e4memory ;

      doRightSpecial = 0 ;
      if ( b4->nKeys == 0 )   /* can't do a seek on, so after just go to far left */
      {
         noKeys = 1 ;
         if ( isRight == 1 && parent != 0 )  /* get parents key, since performing a right-op */
         {
            memcpy( &(myKeyData->num), &( b4key( parent, parent->nKeys - 1 )->num), sizeof(S4LONG) + t4->header.keyLen ) ;
            doRightSpecial = 1 ;
         }
         else
            memcpy( &(myKeyData->num), &( b4key( b4temp, 0 )->num), sizeof(S4LONG) + t4->header.keyLen ) ;
         assert5( myKeyData->num > 0 ) ;
      }
      else
      {
         noKeys = 0 ;
         /* AS 02/17/98 t5samp1.cpp - if only 1 key, else section fails because key '1' doesn't exist for leaf */
/*         if ( b4->keyOn > 0 ) */
         if ( b4->keyOn > 0 || (b4->nKeys == 1 && balanceMode == 1 ) )
            memcpy( &(myKeyData->num), &( b4key( b4, 0 )->num), sizeof(S4LONG) + t4->header.keyLen ) ;
         else
            memcpy( &(myKeyData->num), &( b4key( b4, 1 )->num), sizeof(S4LONG) + t4->header.keyLen ) ;
         assert5( myKeyData->num > 0 ) ;
      }

      if ( tfile4balanceBlock( t4, parent, b4, b4temp, isRight, balanceMode ) < 0 )
      {
         u4free( myKeyData ) ;
         return -1 ;
      }

      /* need to re-find position */
      assert5( myKeyData->num > 0 ) ;
      rc = tfile4go( t4, myKeyData->value, myKeyData->num, 0 ) ;   /* returns -1 if error4code( codeBase ) < 0 */
      if ( noKeys == 1 ) /* can't find position, so just go to far left */
      {
         for ( ;; )
         {
            b4 = tfile4block( t4 ) ;
            if ( b4->nKeys < t4->header.keysHalf || b4leaf( b4 ) )
            {
               if ( doRightSpecial == 1 )   /* in this case, don't reset position, just go up */
               {
                  /* AS 11/27/97 below fix didn't work - tkay.c - change to just break without moving */
                  tfile4upToRoot( t4 ) ;
                  noKeys = 0 ;   /* forces from the top again */
               }
               else
                  tfile4upToRoot( t4 ) ;
               balanceMode = 2 ;
               doFull = 1 ;
               break ;
            }
            if ( doRightSpecial == 1 )
            {
               b4->keyOn = b4->nKeys ;
               tfile4down( t4 ) ;
            }
            else
               if ( tfile4skip( t4, 1L ) != 1L )
                  break ;
         }
      }
      if ( doRightSpecial == 1 && t4->blocks.lastNode != 0 )   /* in this case, don't reset position, just go up */
         b4 = tfile4block( t4 ) ;
      u4free( myKeyData ) ;
      if ( rc < 0 )
         return rc ;

      if ( b4 == 0 )  /* in theory should never happen, but would maybe mean done */
         return 0 ;

      parent = (B4BLOCK *)b4->link.p ;
      if ( parent == 0 || b4leaf( parent ) )  /* no parent */
         break ;

      if ( parent == b4 )  /* move down one to avoid problems (b4 should never be root) */
      {
         if ( tfile4down( t4 ) == 0 )
         {
            b4 = (B4BLOCK *)t4->blocks.lastNode ;
            tfile4up( t4 ) ;
         }
      }

      if ( balanceMode != 2 || noKeys != 1 )
      {
         if ( doFull )
         {
            if ( b4leaf( b4 ) )
               balanceMode = 0 ;
         }
         else
         {
            if ( parent->nKeys < t4->header.keysHalf )  /* do parent as well */
               balanceMode = 2 ;
            else
               balanceMode = 0 ;
         }
      }
    }

   b4free( b4temp ) ;
   return 0 ;
}

int tfile4getReplaceEntry( TAG4FILE *t4, B4KEY_DATA *insertSpot, B4BLOCK *saveBlock )
{
   int rc ;
   B4BLOCK *blockOn ;

   blockOn = saveBlock ;

   if ( b4leaf( blockOn ) )
      return 0 ;

   blockOn->keyOn = blockOn->keyOn + 1 ;
   while ( !b4leaf( blockOn ) )
   {
      rc = tfile4down( t4 ) ;
      if ( rc < 0 || rc == 2 )
         return -1 ;
      blockOn = (B4BLOCK *)t4->blocks.lastNode ;
   }
   memcpy( &insertSpot->num, &(b4key( blockOn, blockOn->keyOn )->num), sizeof(S4LONG) + t4->header.keyLen ) ;
   saveBlock->changed = 1 ;
   b4remove( blockOn ) ;
   if ( blockOn->nKeys < t4->header.keysHalf && blockOn->fileBlock != t4->header.root )  /* if not root may have to balance the tree */
      if ( tfile4balance( t4, blockOn, 0 ) < 0 )
         return -1 ;
   return 1 ;
}

int tfile4balanceBranch( TAG4FILE *t4, B4BLOCK *b4 )
{
   B4BLOCK *parent, *b4temp ;
   int rc ;
   char isRight ;
   long tempFb, reference ;

   #ifdef E4ANALYZE
      if ( b4leaf( b4 ) )
         return error4describe( t4->codeBase, e4index, E95406, tfile4alias( t4 ), 0, 0 ) ;
   #endif

   if ( b4 == (B4BLOCK *)l4first( &t4->blocks ) )
   {
      if ( b4->nKeys == 0 )   /* empty, so just remove */
      {
         reference = (long)b4key( b4, 0 )->pointer ;
         b4->changed = 0 ;
         tfile4shrink( t4, b4->fileBlock ) ;
         tfile4up( t4 ) ;
         l4pop(&t4->saved ) ;
         b4free( b4 ) ;
         b4 = 0 ;
         t4->header.root = reference ;
      }
      return 0 ;
   }

   tfile4up( t4 ) ;
   parent = (B4BLOCK *)t4->blocks.lastNode ;

   #ifdef E4ANALYZE
      if ( b4key( parent, parent->keyOn )->pointer != I4MULTIPLY * b4->fileBlock )
         return error4describe( t4->codeBase, e4index, E95406, tfile4alias( t4 ), 0, 0 ) ;
   #endif

   if ( parent->keyOn == parent->nKeys )
   {
      tempFb = b4key( parent, parent->keyOn - 1 )->pointer ;
      isRight = 1 ;
   }
   else
   {
      tempFb = b4key( parent, parent->keyOn + 1 )->pointer ;
      isRight = 0 ;
   }

   b4temp = b4alloc( t4, 0L ) ;
   if ( b4temp == 0 )
      return error4stack( t4->codeBase, e4memory, E95406 ) ;

   rc = i4readBlock( &t4->file, tempFb, 0, b4temp ) ;
   if ( rc < 0 )
   {
      b4free( b4temp ) ;
      return error4stack( t4->codeBase, rc, E95406 ) ;
   }

   rc = tfile4balanceBlock( t4, parent, b4, b4temp, isRight, 2 ) ;
   b4free( b4temp ) ;
   if ( rc < 0 )
      return error4stack( t4->codeBase, rc, E95406 ) ;

   return 0 ;
}

int tfile4removeCurrent( TAG4FILE *t4 )
{
   B4BLOCK *blockOn ;

   t4->header.version = (short)( t4->header.oldVersion + 1L ) ;

   blockOn = (B4BLOCK *)t4->blocks.lastNode ;

   #ifdef E4ANALYZE
      if ( b4lastpos( blockOn ) == (b4leaf( blockOn ) ? -1 : 0) )
         return error4describe( t4->codeBase, e4index, E85401, tfile4alias( t4 ), 0, 0 ) ;
   #endif

   switch( tfile4getReplaceEntry( t4, b4key( blockOn, blockOn->keyOn ), blockOn ) )
   {
      case 0 :   /* leaf delete */
         b4remove( blockOn ) ;
         if ( blockOn->nKeys == 0 )  /* last entry deleted! -- remove upward reference */
         {
            if ( tfile4removeRef( t4 ) != 0 )
               return -1 ;
         }
         else
         {
            if ( blockOn->nKeys < t4->header.keysHalf && blockOn->fileBlock != t4->header.root )  /* if not root may have to balance the tree */
               return tfile4balance( t4, blockOn, 0 ) ;
         }
         break ;

      case 1 :   /* branch delete */
         if( tfile4block( t4 )->nKeys == 0 )    /* removed the last key of */
            if ( tfile4removeRef( t4 ) != 0 )
               return -1 ;
         break ;

      default:
         return error4describe( t4->codeBase, e4index, E81601, tfile4alias( t4 ), 0, 0 ) ;
   }
   return 0 ;
}
#endif  /* S4CLIPPER */

#endif  /* S4OFF_INDEX */
#endif  /* S4OFF_WRITE */
