/* i4ntag.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4INDEX_OFF

int tfile4type( TAG4FILE *t4 )
{
 #ifdef S4FOX
    return t4->expr->type ;
 #endif
 #ifdef S4CLIPPER
    return t4->expr->type ;
 #endif
 #ifdef S4MDX
    return t4->header.type ;
 #endif
}

#ifdef S4CLIPPER

B4BLOCK *tfile4block( TAG4FILE *t4 )
{
   #ifdef E4PARM_LOW
      if ( t4 == 0 )
      {
         error4( 0, e4parm_null, E91642 ) ;
         return 0 ;
      }
   #endif
   #ifdef E4ANALYZE
      if ( t4->blocks.lastNode == 0 )
      {
         error4( 0, e4struct, E91642 ) ;
         return 0 ;
      }
   #endif
   return (B4BLOCK *)t4->blocks.lastNode ;
}

int tfile4rlBottom( TAG4FILE *t4 )
{
   int rc ;
   B4BLOCK *blockOn ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return -1 ;

   do
   {
      rc = tfile4upToRoot( t4 ) ;
      if ( rc < 0 )
         return -1 ;

      if ( rc != 2 )
      {
         b4goEof( tfile4block( t4 ) ) ;
         do
         {
            rc = tfile4down( t4 ) ;
            if ( rc < 0 )
               return -1 ;
            b4goEof( tfile4block( t4 ) ) ;
         } while ( rc == 0 ) ;
      }

      if ( rc == 2 )   /* failed due to read while locked */
         tfile4outOfDate( t4 ) ;
   } while ( rc == 2 ) ;

   blockOn = tfile4block(t4) ;
   if ( blockOn->keyOn > 0 )
   {
      blockOn->keyOn = blockOn->nKeys-1 ;
      assert5( blockOn->nKeys <= t4->header.keysMax && blockOn->nKeys > 0 ) ;
   }

   #ifdef E4ANALYZE
      if ( blockOn->keyOn < 0 )
         return error4( t4->codeBase, e4info, E91642 ) ;
   #endif

   return 0 ;
}

int S4FUNCTION tfile4bottom( TAG4FILE *t4 )
{
   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   if ( t4->header.descending )   /* if descending, go bottom means go top */
      return tfile4rlTop( t4 ) ;
   else
      return tfile4rlBottom( t4 ) ;
}

#ifdef S4HAS_DESCENDING
void S4FUNCTION tfile4descending( TAG4FILE *tag, const unsigned short int setting )
{
   tag->header.descending = setting ;
}
#endif

/* Returns  1 - Cannot move down; 0 - Success; -1 Error */
int tfile4down( TAG4FILE *t4 )
{
   long blockDown ;
   B4BLOCK *blockOn, *popBlock, *newBlock, *parent ;
   int rc ;
   FILE4LONG pos ;
   #ifdef S4BYTE_SWAP
      char *swapPtr ;
      int i ;
      short shortVal ;
      long longVal ;
   #endif

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   blockOn = (B4BLOCK *)t4->blocks.lastNode ;

   if ( blockOn == 0 )    /* Read the root block */
   {
      if ( t4->header.root <= 0L )
      {
         file4longAssign( pos, t4->header.headerOffset + 2*sizeof(short), 0 ) ;
         if ( file4readAllInternal( &t4->file, pos, &t4->header.root, 2*sizeof(S4LONG)) < 0 )
            return -1 ;

         #ifdef S4BYTE_SWAP
            t4->header.root = x4reverseLong( (void *)&t4->header.root ) ;
            t4->header.eof  = x4reverseLong( (void *)&t4->header.eof ) ;
         #endif
      }
      blockDown = t4->header.root ;
   }
   else
   {
      if ( b4leaf( blockOn ) )
         return 1 ;
      blockDown = b4key( blockOn, blockOn->keyOn )->pointer ;
      #ifdef E4ANALYZE
         if ( blockDown <= 0L || blockOn->nKeys -1 > t4->header.keysMax )
            error4( t4->codeBase, e4info, E81602 ) ;
      #endif
   }

   /* Get memory for the new block */
   popBlock = (B4BLOCK *)l4pop( &t4->saved ) ;
   newBlock = popBlock ;
   if ( newBlock == 0 )
      newBlock = b4alloc( t4, blockDown ) ;
   if ( newBlock == 0 )
      return -1 ;
   parent = (B4BLOCK *)l4last( &t4->blocks ) ;
   l4add( &t4->blocks, newBlock ) ;

   if ( popBlock == 0  ||  newBlock->fileBlock*I4MULTIPLY != blockDown )
   {
      #ifndef S4OFF_WRITE
         if ( newBlock->changed == 1 )
            if ( b4flush(newBlock) < 0 )
               return -1 ;
      #endif

      rc = i4readBlock( &t4->file, blockDown, parent, newBlock ) ;

      if ( rc < 0 )
         return -1 ;

      if ( rc == 1 )
      {
         l4remove( &t4->blocks, newBlock ) ;
         l4add( &t4->saved, newBlock ) ;
         return 2 ;
      }

      for ( ;; )
      {
         blockOn = (B4BLOCK *)l4pop( &t4->saved ) ;
         if ( blockOn == 0 )
            break ;
         #ifndef S4OFF_WRITE
            if ( b4flush(blockOn) < 0 )
               return -1 ;
         #endif
         b4free( blockOn ) ;
      }
   }

   newBlock->keyOn = 0 ;
   return 0 ;
}

int S4FUNCTION tfile4eof( TAG4FILE *t4 )
{
   B4BLOCK *b4 ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   b4 = tfile4block( t4 ) ;
   return ( b4->keyOn >= b4->nKeys ) ;
}

#ifndef S4OFF_WRITE
int tfile4flush( TAG4FILE *t4 )
{
   int rc ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E94901 ) ;
   #endif

   rc = tfile4update( t4 ) ;
   #ifndef S4OPTIMIZE_OFF
      if ( rc )
         rc = file4flush( &t4->file ) ;
   #endif
   return rc ;
}
#endif

int tfile4freeAll( TAG4FILE *t4 )
{
   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   while ( tfile4up( t4 ) == 0 ) ;
   return tfile4freeSaved( t4 ) ;
}

int tfile4freeSaved( TAG4FILE *t4 )
{
   B4BLOCK *blockOn ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   #ifndef S4OFF_WRITE
      if ( tfile4update( t4 ) < 0 )
         return -1 ;
   #endif

   for ( ;; )
   {
      blockOn = (B4BLOCK *)l4pop( &t4->saved ) ;
      if ( blockOn == 0 )
         return 0 ;
      #ifndef S4OFF_WRITE
         if ( b4flush( blockOn ) < 0 )
            return -1 ;
      #endif
      b4free( blockOn ) ;
   }
}

B4KEY_DATA *tfile4keyData( TAG4FILE *t4 )
{
   B4BLOCK *b4 ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
      {
         error4( 0, e4parm_null, E91642 ) ;
         return 0 ;
      }
   #endif

   b4 = (B4BLOCK *)t4->blocks.lastNode ;
   return b4key( b4, b4->keyOn ) ;
}

long S4FUNCTION tfile4recNo( TAG4FILE *t4 )
{
   B4BLOCK *blockOn ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return (long)error4( 0, e4parm_null, E91642 ) ;
   #endif

   blockOn = (B4BLOCK *)t4->blocks.lastNode ;
   if ( blockOn == 0 )
      return -2L ;
   if ( blockOn->keyOn >= blockOn->nKeys )
      return -1 ;

   return b4recNo( blockOn, blockOn->keyOn ) ;
}

int S4FUNCTION tfile4seek( TAG4FILE *t4, const void *ptr, const int lenPtrIn )
{
   int rc, lenPtr ;
   B4BLOCK *blockOn ;
   int upperFnd, upperAft, incPos, dSet ;
   unsigned char *cPtr ;
   dSet = 0 ;
   cPtr = (unsigned char *)ptr ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 || ptr == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;
   lenPtr = lenPtrIn ;

   if ( lenPtr > t4->header.keyLen )
      lenPtr = t4->header.keyLen ;

   if ( t4->header.descending )   /* look for current item less one: */
   {
      for( incPos = lenPtr-1 ; dSet == 0 && incPos >=0 ; incPos-- )
      {
         if ( cPtr[incPos] == 0xFF )
            cPtr[incPos] = 0x00 ;
         else
         {
            cPtr[incPos]++ ;
            dSet = 1 ;
         }
      }
   }

   rc = 3 ;
   for(;;) /* Repeat until found */
   {
      while ( rc >= 2 )
      {
         if ( rc == 2 )
            tfile4outOfDate( t4 ) ;
         rc = tfile4upToRoot( t4 ) ;
         upperFnd = 0 ;
         upperAft = 0 ;

         if ( rc < 0 )
            return -1 ;
      }
      blockOn = (B4BLOCK *)t4->blocks.lastNode ;
      #ifdef E4ANALYZE
         if ( blockOn == 0 )
            return error4( t4->codeBase, e4info, E91642 ) ;
      #endif

      rc = b4seek( blockOn, (char *)ptr, lenPtr ) ;
      if ( b4leaf( blockOn ) )
      {
         if ( rc == r4after && upperAft && blockOn->keyOn >= blockOn->nKeys )
             while( upperAft-- > 1 )
                tfile4up( t4 ) ;
         if ( rc == 0 || !upperFnd )
            break ;

         while( upperFnd-- > 1 )
             tfile4up( t4 ) ;
         rc = 0 ;
         break ;
      }
      if ( rc == 0 )
      {
         upperFnd = 1 ;
         upperAft = 0 ;
      }
      else
         if ( rc == r4after && !upperFnd && !( blockOn->keyOn >= blockOn->nKeys ) )
            upperAft = 1 ;

      rc = tfile4down( t4 ) ;
      if ( rc < 0 )
         return -1 ;

      if ( upperFnd )
         upperFnd++ ;
      if ( upperAft )
         upperAft++ ;
   }
   if ( t4->header.descending )   /* must go back one! */
   {
      /* reset the search_ptr ; */
      incPos++ ;
      cPtr[incPos]-- ;
      memset( cPtr + incPos + 1, 0xFF, lenPtr - incPos - 1 ) ;
      if ( dSet )
      {
         rc = (int)tfile4skip( t4, -1L ) ;
         if ( rc == 0L )  /* bof = eof condition */
         {
            b4goEof( blockOn ) ;
            rc = r4eof ;
         }
         else
         {
            if ( (u4memcmp)( b4keyKey( tfile4block( t4 ), tfile4block( t4 )->keyOn ), ptr, lenPtr ) )
               rc = r4after ;
            else
               rc = 0 ;  /* successful find */
         }
      }
      else
      {
         if ( rc == 0 )  /* the item was found, so go top, */
            tfile4top( t4 ) ;
         else  /* otherwise want an eof type condition */
         {
            b4goEof( blockOn ) ;
            rc = r4eof ;
         }
      }
   }
   return rc ;
}

long S4FUNCTION tfile4skip( TAG4FILE *t4, long numSkip )
{
   int rc, sign, seekSpecial ;
   B4BLOCK *blockOn ;

   long j ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return (long)error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   if ( numSkip < 0)
      sign = -1 ;
   else
      sign = 1 ;

   blockOn = (B4BLOCK *)t4->blocks.lastNode ;
   if ( blockOn == 0 )
   {
      rc = tfile4top( t4 ) ;
      if ( rc < 0 )
         return -numSkip ;
      blockOn = (B4BLOCK *)t4->blocks.lastNode ;
   }

   for( j = numSkip; j != 0; j -= sign )  /* skip 1 * numSkip */
   {
      if ( b4leaf(blockOn) )
      {
         if ( b4skip( blockOn, (long) sign) != sign )  /* go up */
         {
            int go_on = 1 ;
            while ( go_on )
            {
               if ( l4prev( &t4->blocks, t4->blocks.lastNode ) == 0 )  /* root block */
               {
                  if ( numSkip > 0 && t4->header.descending == 0 || numSkip <= 0 && t4->header.descending == 1 )
                  {
                     if ( tfile4bottom( t4 ) < 0 )
                        return -numSkip ;
                  }
                  else
                     if ( tfile4top( t4 ) < 0 )
                        return -numSkip ;

                  return ( numSkip - j ) ;
               }

               rc = tfile4up( t4 ) ;
               blockOn = tfile4block( t4 ) ;
               if ( rc != 0 ) return -1 ;

               if ( sign > 0 )  /* forward skipping */
               {
                  if ( !( blockOn->keyOn >= blockOn->nKeys ) )
                     go_on = 0 ;
               }
               else   /* backward skipping */
               {
                  if ( ! (blockOn->keyOn == 0) )
                  {
                     b4skip( blockOn, -1L ) ;
                     go_on = 0 ;
                  }
               }
            }
         }
      }
      else
      {
         if ( sign > 0 )
            b4skip( blockOn, 1L ) ;

         /* save the current key in case skip fails */
         if ( blockOn->keyOn >= blockOn->nKeys )  /* case where no proper key to copy exists */
         {
            if ( blockOn->nKeys == 0 )   /* invalid, so must discard this block */
            {
               /* codeBase can't recover from this gracefully, so just go back up saying
                  that couldn't skip */
               return 0 ;
            }

            /* not on a true key, so instead just go to the old position, and then try the skip again (unless r4after)*/
            memcpy( t4->codeBase->savedKey, b4keyKey( blockOn, blockOn->keyOn - 1 ), t4->header.keyLen ) ;
            seekSpecial = 1 ;
         }
         else
         {
            memcpy( t4->codeBase->savedKey, b4keyKey( blockOn, blockOn->keyOn ), t4->header.keyLen ) ;
            seekSpecial = 0 ;
         }

         while ( (rc = tfile4down( t4 ) ) == 0 )
         {
            if ( sign < 0 )
            {
               blockOn = tfile4block( t4 ) ;
               b4goEof( blockOn ) ;
               if ( b4leaf( blockOn ) )
                  blockOn->keyOn-- ;
            }
         }
         if ( rc < 0 )
            return -numSkip ;

         if ( rc == 2 )   /* failed on i/o, seek current spot to make valid */
         {
            #ifndef S4OPTIMIZE_OFF
            #ifndef S4SINGLE
               file4refresh( &t4->file ) ;
            #endif
            #endif
            rc = tfile4seek( t4, t4->codeBase->savedKey, t4->header.keyLen ) ;
            if ( rc < 0 )
               return -numSkip ;
            if ( rc == r4after )   /* means skipped 1 ahead */
               if ( j != 1 )
                  j-- ;
            if ( seekSpecial == 1 )  /* means we are actually on wrong pos, need to skip one */
               j += sign ;   /* pre-increment # of skip times */
         }

         blockOn = tfile4block( t4 ) ;
      }
   }
   return numSkip ;
}

#ifndef S4OFF_WRITE
/* NTX only needs to do a copy and adjust the index pointers */
/* if extraOld is true then the extra key is placed in old, otherwise in new */
B4BLOCK *tfile4split( TAG4FILE *t4, B4BLOCK *oldBlock, const int extraOld )
{
   long  newFileBlock ;
   B4BLOCK *newBlock ;
   int isBranch ;

   if ( error4code( t4->codeBase ) < 0 )
      return 0 ;

   #ifdef E4INDEX_VERIFY
      if ( b4verify( oldBlock ) == -1 )
         error4describe( oldBlock->tag->codeBase, e4index, E91642, oldBlock->tag->alias, 0, 0 ) ;
   #endif

   newFileBlock = tfile4extend( t4 ) ;
   newBlock = b4alloc( t4, newFileBlock ) ;
   if ( newBlock == 0 )
      return 0 ;

   newBlock->changed = 1 ;
   oldBlock->changed = 1 ;

   memcpy( newBlock->data, oldBlock->data, B4BLOCK_SIZE - ( t4->header.keysMax + 2 ) * sizeof(short) ) ;

   if ( extraOld )
   {
      newBlock->nKeys = oldBlock->nKeys / 2 ;
      oldBlock->nKeys -= newBlock->nKeys ;
      if ( oldBlock->nKeys == newBlock->nKeys )
      {
         oldBlock->nKeys-- ;
         newBlock->nKeys++ ;
      }
      // newBlock->nKeys must be non-zero because extra key goes into old block if here
      assert5( newBlock->nKeys <= t4->header.keysMax && newBlock->nKeys > 0 ) ;
      assert5( oldBlock->nKeys <= t4->header.keysMax && (oldBlock->nKeys > 0 || newBlock->nKeys == 2 )) ;
   }
   else
   {
      newBlock->nKeys = oldBlock->nKeys ;
      oldBlock->nKeys = oldBlock->nKeys / 2 ;
      newBlock->nKeys -= oldBlock->nKeys ;
      assert5( newBlock->nKeys <= t4->header.keysMax && (newBlock->nKeys > 0 || oldBlock->nKeys == 2 )) ;
      // oldBlock->nKeys must be non-zero because extra key goes into new block if here
      assert5( oldBlock->nKeys <= t4->header.keysMax && oldBlock->nKeys > 0 ) ;
   }

   isBranch = !b4leaf( oldBlock ) ;

   memcpy( newBlock->pointers, &oldBlock->pointers[oldBlock->nKeys + isBranch], newBlock->nKeys * sizeof(short) ) ;
   memcpy( &newBlock->pointers[newBlock->nKeys], oldBlock->pointers, (oldBlock->nKeys + isBranch) * sizeof(short) ) ;

   if ( isBranch == 0 )  /* leaf blocks need one more copy */
      newBlock->pointers[t4->header.keysMax] = oldBlock->pointers[t4->header.keysMax] ;

   newBlock->keyOn = oldBlock->keyOn - oldBlock->nKeys - isBranch ;
   if ( newBlock->keyOn < 0 && extraOld == 1 )  // not adding to new anyway, just set to 0 for safety
       newBlock->keyOn = 0 ;
   newBlock->nKeys -= isBranch ;
   // AS 05/05/98 is possible for newKeys to have 0 keys if new key going into newBlock
   assert5( newBlock->nKeys <= t4->header.keysMax && (newBlock->nKeys > 0 || (extraOld == 0 && (oldBlock->nKeys + isBranch ) == 2) )) ;
   assert5( newBlock->keyOn >= 0 && oldBlock->keyOn >= 0 ) ;

   return newBlock ;
}
#endif  /* S4OFF_WRITE */

int tfile4rlTop( TAG4FILE *t4 )
{
   int rc ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   do
   {
      rc = tfile4upToRoot( t4 ) ;
      if ( rc < 0 )
         return -1 ;

      if ( rc != 2 )
      {
         ((B4BLOCK *)t4->blocks.lastNode)->keyOn = 0 ;
         do
         {
            if ( (rc = tfile4down(t4)) < 0 )
               return -1 ;
            ((B4BLOCK *)t4->blocks.lastNode)->keyOn = 0 ;
         } while ( rc == 0 ) ;
      }

      if ( rc == 2 )   /* failed due to read while locked */
         tfile4outOfDate( t4 ) ;
   } while ( rc == 2 ) ;

   return 0 ;
}

int S4FUNCTION tfile4top( TAG4FILE *t4 )
{
   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   if ( t4->header.descending )   /* if descending, go top means go bottom */
      return tfile4rlBottom( t4 ) ;
   else
      return tfile4rlTop( t4 ) ;
}

int tfile4up( TAG4FILE *t4 )
{
   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( t4->blocks.lastNode == 0 )
      return 1 ;
   l4add( &t4->saved, l4pop(&t4->blocks) ) ;
   return 0 ;
}

#ifndef S4OFF_WRITE
int tfile4update( TAG4FILE *t4 )
{
   B4BLOCK *blockOn ;
   FILE4LONG pos ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   if ( tfile4updateHeader( t4 ) < 0 )
      return -1 ;

   for( blockOn = 0 ;; )
   {
      blockOn = (B4BLOCK *)l4next( &t4->saved ,blockOn ) ;
      if ( blockOn == 0 )
         break ;
      if ( b4flush(blockOn) < 0 )
         return -1 ;
   }

   for( blockOn = 0 ;; )
   {
      blockOn = (B4BLOCK *)l4next( &t4->blocks, blockOn ) ;
      if ( blockOn == 0 )
         break ;
      if ( b4flush( blockOn ) < 0 )
         return -1 ;
   }

   if ( t4->rootWrite )
   {
      #ifdef S4BYTE_SWAP
         t4->header.root = x4reverseLong( (void *)&t4->header.root ) ;
         t4->header.eof = x4reverseLong( (void *)&t4->header.eof ) ;
      #endif
      file4longAssign( pos, t4->headerOffset + 2*sizeof( short ), 0 ) ;
      if ( file4writeInternal( &t4->file, pos, &t4->header.root, 2*sizeof(S4LONG) ) < 0 )
         return -1 ;
      #ifdef S4BYTE_SWAP
         t4->header.root = x4reverseLong( (void *)&t4->header.root ) ;
         t4->header.eof = x4reverseLong( (void *)&t4->header.eof ) ;
      #endif
      t4->rootWrite = 0 ;
   }

   return 0 ;
}
#endif /* S4OFF_WRITE */

int tfile4upToRoot( TAG4FILE *t4 )
{
   LINK4 *linkOn ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   for ( ;; )
   {
      linkOn = (LINK4 *)l4pop( &t4->blocks ) ;
      if ( linkOn == 0 )
         return tfile4down(t4) ;
      l4add( &t4->saved, linkOn ) ;
   }
}

int tfile4close( TAG4FILE *t4, DATA4FILE *d4 )
{
   CODE4 *c4 ;
   int finalRc ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91638 ) ;
   #endif

   c4 = t4->codeBase ;
   finalRc = 0 ;

   #ifdef E4ANALYZE
      if ( t4->userCount <= 0 )
         return error4( c4, e4struct, E91638 ) ;
   #endif

   t4->userCount-- ;
   #ifdef S4SERVER
      if ( c4->server->keepOpen != 2 || t4->file.isTemp != 1 )
   #endif
   if ( t4->userCount == 0 )
   {
      if ( tfile4freeAll( t4 ) < 0 )
         finalRc = error4set( c4, 0) ;
      expr4free( t4->expr ) ;
      expr4free( t4->filter ) ;
      mem4release( t4->blockMemory ) ;
      t4->blockMemory = 0 ;
      if ( file4openTest( &t4->file ) )
      {
         if ( c4->doRemove == 1 )
            t4->file.isTemp = 1 ;
         if ( file4close( &t4->file ) < 0 )
            finalRc = error4set( c4, 0 ) ;
      }
      if ( t4->link.n != (LINK4 *)0 )
         l4remove( &d4->tagfiles, t4 ) ;
      mem4free( c4->tagFileMemory, t4 ) ;
      error4set( c4, finalRc ) ;
   }

   return finalRc ;
}

int S4FUNCTION t4close( TAG4 *t4 )
{
   int finalRc ;
   DATA4 *d4 ;
   CODE4 *c4 ;

   #ifdef S4VBASIC
      if ( c4parm_check( t4, 4, E91637 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91637 ) ;
   #endif

   if ( l4seek( &t4->index->tags, t4 ) == 1 )  /* i4close removes the tag, so if still there, was not called from i4closeLow() */
   {
      if ( l4numNodes( &t4->index->tags ) == 1 )   /* only the one tag, so remove index */
         return i4closeLow( t4->index ) ;
      else  /* must remove from the list manually */
         l4remove( &t4->index->tags, t4 ) ;
   }

   c4 = t4->tagFile->codeBase ;
   d4 = t4->index->data ;

   #ifndef S4OFF_TRAN
      /* May 10/96 AS added line below which otherwise caused bad indexes not to close good tags */
      if ( t4->index->isValid == 1 ) /* if invalid (failed create/open) then allow close */
         if ( t4->isValid == 1 ) /* if invalid (failed create/open) then allow close */
            if ( code4transEnabled( c4 ) )
               if ( code4trans( c4 )->currentTranStatus == r4active )  /* disallow on current active only */
                  return error4( c4, e4transViolation, E81522 ) ;
   #endif

   finalRc = error4set( c4, 0 ) ;

   #ifndef S4OFF_WRITE
      #ifndef S4OFF_TRAN
         if ( t4->isValid == 1 ) /* if invalid (failed create/open) then allow close */
      #endif
            if ( d4 )
               if ( d4update( d4 ) < 0 )
                  finalRc = error4set( c4, 0 ) ;
   #endif

   #ifndef S4SINGLE
      if ( tfile4unlock( t4->tagFile, data4serverId( t4->index->data ) ) < 0 )
         finalRc = error4set( c4, 0 ) ;
   #endif
   /* 03/11/98 AS expression data mismatch problem --> if same data4 re-allocated */
   if ( t4->tagFile->expr != NULL ) /* could be NULL if improper open or if unsupported expression (read-only) */
      if ( t4->tagFile->expr->data == t4->index->data )
         t4->tagFile->expr->data = 0 ;

   if ( tfile4close( t4->tagFile, d4->dataFile ) < 0 )
      finalRc = error4set( c4, 0 ) ;
   mem4free( c4->tagMemory, t4 ) ;

   error4set( c4, finalRc ) ;

   return finalRc ;
}

#ifndef S4OFF_WRITE
long tfile4extend( TAG4FILE *t4 )
{
   long oldEof ;
   CODE4 *c4 = t4->codeBase ;
   FILE4LONG pos ;

   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   #ifdef E4ANALYZE
      if ( t4->header.version == t4->header.oldVersion )
         return (long)error4( c4, e4info, E91636 ) ;
   #endif

   oldEof = t4->header.eof ;

   #ifdef S4SINGLE
      if ( oldEof != 0 )   /* case where free-list exists */
         t4->header.eof = 0L ;
      else
      {
   #endif
      oldEof = file4longGetLo( file4lenLow( &t4->file ) ) ;
      #ifdef E4ANALYZE
         if ( oldEof <= t4->checkEof )
            return (long)error4( c4, e4info, E91636 ) ;
         t4->checkEof = oldEof ;
      #endif

      file4longAssign( pos, file4longGetLo( file4lenLow( &t4->file ) ) + 1024, 0 ) ;
      file4lenSetLow( &t4->file, pos ) ; /* and extend the file */
   #ifdef S4SINGLE
      }
   #endif
   return oldEof/512 ;
}
#endif /* S4OFF_WRITE */

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int tfile4go2( TAG4FILE *t4, const unsigned char *ptr, const long recNum, const int goAdd )
{
   int rc ;
   long rec ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 || ptr == 0 || recNum < 1 )
         return error4( 0, e4parm, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   rc = tfile4seek( t4, ptr, t4->header.keyLen ) ;
   if ( rc )
      return rc ;

   for(;;)
   {
      rec = tfile4recNo( t4 ) ;
      if (rec == recNum )
         return 0 ;

      rc = (int)tfile4skip( t4, 1L ) ;
      if ( rc == -1 )
         return -1 ;
      if ( rc == 0 )
      {
         b4goEof( tfile4block( t4 ) ) ;
         return r4found ;
      }

      if ( (*t4->cmp)( tfile4keyData( t4 )->value, ptr, t4->header.keyLen ) )
         return r4found ;
   }
}

static TAG4FILE *tfile4open( DATA4 *d4, const char *fileName )
{
   TAG4FILE *tfile ;
   CODE4 *c4 ;
   char buf[258], buffer[1024] ;
   char exprBuf[I4MAX_EXPR_SIZE + 1] ;
   char *ptr, garbage ;
   FILE4SEQ_READ seqRead ;
   int rc, len, oldTagNameError ;
   FILE4LONG pos ;

   c4 = d4->codeBase ;

   u4ncpy( buf, fileName, sizeof( buf ) ) ;
   #ifndef S4CASE_SEN
      c4upper(buf) ;
   #endif

   #ifdef S4CASE_SEN
      u4nameExt( buf, sizeof(buf), "ntx", 0 ) ;
   #else
      u4nameExt( buf, sizeof(buf), "NTX", 0 ) ;
   #endif

   oldTagNameError = c4->errTagName ;
   c4->errTagName = 0 ;
   tfile = dfile4tag( d4->dataFile, buf ) ;
   c4->errTagName = oldTagNameError ;
   if ( tfile != 0 ) /* because t4open() verifies no duplicates, this must be a duplicate data4 instance */
   {
      /* changed 09/19/95 - test program t4skip.c */
      tfile->userCount++ ;
      return tfile ;
      /* only one instance ever allowed */
/*      error4( c4, e4instance, E94906 ) ;*/
/*      return 0 ; */

      /*
      #ifndef S4SERVER
         if ( c4->singleOpen != OPEN4SPECIAL )   only one instance allowed...
         {
            error4( c4, e4instance, E94906 ) ;
            return 0 ;
         }
      #endif

      tfile->userCount++ ;
      return tfile ;
      */
   }

   if ( c4->tagFileMemory == 0 )
   {
      c4->tagFileMemory = mem4create( c4, c4->memStartTagFile, sizeof(TAG4FILE), c4->memExpandTagFile, 0 ) ;
      if ( c4->tagFileMemory == 0 )
         return 0 ;
   }

   tfile = (TAG4FILE *)mem4alloc( c4->tagFileMemory ) ;
   if ( tfile == 0 )
      return 0 ;

   tfile->file.hand = INVALID4HANDLE ;
   tfile->codeBase = c4 ;
   tfile->userCount = 1 ;
   if ( tfile->blockMemory == 0 )
      tfile->blockMemory = mem4create( c4, c4->memStartBlock, (sizeof(B4BLOCK)) + B4BLOCK_SIZE -
                                     (sizeof(B4KEY_DATA)) - (sizeof(short)) - (sizeof(char[2])),
                                     c4->memExpandBlock, 0 ) ;

   if ( tfile->blockMemory == 0 )
   {
      tfile4close( tfile, d4->dataFile ) ;
      return 0 ;
   }

   rc = file4open( &tfile->file, c4, buf, 1 ) ;
   if ( rc != 0 )
   {
      tfile4close( tfile, d4->dataFile ) ;
      return 0 ;
   }

   #ifndef S4OPTIMIZE_OFF
      file4optimizeLow( &tfile->file, c4->optimize, OPT4INDEX, 0, tfile ) ;
   #endif

   file4longAssign( pos, 0, 0 ) ;
   file4seqReadInitDo( &seqRead, &tfile->file, pos, buffer, 1024, 1 ) ;
   #ifndef S4STRUCT_PAD
      if ( file4seqReadAll( &seqRead, &tfile->header.sign, sizeof(I4IND_HEAD_WRITE) ) < 0 )
   #else
      if ( file4seqReadAll( &seqRead, &tfile->header.sign, sizeof(I4IND_HEAD_WRITE)-2 ) < 0 )  /* Subtract 2 because sizeof is actually 22, not 24 */
   #endif
   {
      tfile4close( tfile, d4->dataFile ) ;
      return 0 ;
   }
   #ifdef S4BYTE_SWAP
      tfile->header.sign = x4reverseShort( (void *)&tfile->header.sign ) ;
      tfile->header.version = x4reverseShort( (void *)&tfile->header.version ) ;
      tfile->header.root = x4reverseLong( (void *)&tfile->header.root ) ;
      tfile->header.eof = x4reverseLong( (void *)&tfile->header.eof ) ;
      tfile->header.groupLen = x4reverseShort( (void *)&tfile->header.groupLen ) ;
      tfile->header.keyLen = x4reverseShort( (void *)&tfile->header.keyLen ) ;
      tfile->header.keyDec = x4reverseShort( (void *)&tfile->header.keyDec ) ;
      tfile->header.keysMax = x4reverseShort( (void *)&tfile->header.keysMax ) ;
      tfile->header.keysHalf = x4reverseShort( (void *)&tfile->header.keysHalf ) ;
   #endif
   tfile->header.headerOffset = 0 ;

   /* Perform some checks */
   if ( tfile->header.keyLen > I4MAX_KEY_SIZE || tfile->header.keyLen <= 0 ||
      tfile->header.keysMax != 2* tfile->header.keysHalf || tfile->header.keysHalf <= 0 ||
      tfile->header.groupLen != tfile->header.keyLen+ 8 ||
      (tfile->header.sign != 0x6 && tfile->header.sign != 0x106 ) )
   {
      error4describe( c4, e4index, E84904, buf, 0, 0 ) ;
      tfile4close( tfile, d4->dataFile ) ;
      return 0 ;
   }

   tfile->cmp = (S4CMP_FUNCTION *)u4memcmp ;
   tfile->header.root = -1 ;
   tfile->header.oldVersion = tfile->header.version ;

   u4namePiece( tfile->alias, sizeof( tfile->alias ), fileName, 0, 0 ) ;
   #ifndef S4CASE_SEN
      c4upper( tfile->alias ) ;
   #endif

   file4seqReadAll( &seqRead, exprBuf, sizeof( exprBuf ) - 1 ) ;
   c4trimN( exprBuf, sizeof( exprBuf ) ) ;
   tfile->expr = expr4parseLow( d4, exprBuf, tfile ) ;
   if ( tfile->expr == 0 )
   {
      tfile4close( tfile, d4->dataFile ) ;
      return 0 ;
   }
   tfile->expr->keyLen = tfile->header.keyLen ;
   tfile->expr->keyDec = tfile->header.keyDec ;

   if ( expr4context( tfile->expr, d4 ) < 0 )
   {
      tfile4close( tfile, d4->dataFile ) ;
      return 0 ;
   }
   len = expr4keyLen( tfile->expr ) ;
   if ( len < 0 )
   {
      error4describe( c4, e4info, 84906, exprBuf, 0, 0 ) ;
      tfile4close( tfile, d4->dataFile ) ;
      return 0 ;
   }

   file4seqReadAll( &seqRead, &tfile->header.unique, sizeof( tfile->header.unique ) ) ;
   file4seqReadAll( &seqRead, &garbage, sizeof( garbage ) ) ;
   file4seqReadAll( &seqRead, &tfile->header.descending, sizeof( tfile->header.descending ) ) ;
   #ifdef S4BYTE_SWAP
      tfile->header.unique = x4reverseShort( (void *)&tfile->header.unique ) ;
      tfile->header.descending = x4reverseShort( (void *)&tfile->header.descending ) ;
   #endif
   file4seqReadAll( &seqRead, exprBuf, sizeof( exprBuf ) - 1 ) ;
   c4trimN( exprBuf, sizeof( exprBuf ) ) ;
   if ( exprBuf[0] != 0 )
   {
      tfile->filter = expr4parseLow( d4, exprBuf, tfile ) ;
      if ( tfile->filter != 0 )
      {
         if ( expr4context( tfile->filter, d4 ) < 0 )
         {
            tfile4close( tfile, d4->dataFile ) ;
            return 0 ;
         }
         len = expr4key( tfile->filter, &ptr, 0 ) ;
         if ( len < 0 )
         {
            tfile4close( tfile, d4->dataFile ) ;
            return 0 ;
         }
         if ( expr4type( tfile->filter ) != 'L' )
         {
            tfile4close( tfile, d4->dataFile ) ;
            return 0 ;
         }
      }
   }

/*   if( tfile->header.unique )
      tfile->uniqueError = c4->errDefaultUnique ;
*/
   l4add( &d4->dataFile->tagfiles, tfile ) ;   /* add the tag to its index list */

   tfile4initSeekConv( tfile, (char)expr4type( tfile->expr ) ) ;

   if ( tfile->blockMemory == 0 )
      tfile->blockMemory = mem4create( c4, c4->memStartBlock, (sizeof(B4BLOCK)) + B4BLOCK_SIZE -
                           (sizeof(B4KEY_DATA)) - (sizeof(short)) - (sizeof(char[2])), c4->memExpandBlock, 0 ) ;

   if ( tfile->blockMemory == 0 )
   {
      #ifdef E4STACK
         error4stack( c4, e4memory, E94906 ) ;
      #endif
      tfile4close( tfile, d4->dataFile ) ;
      return 0 ;
   }

   if ( d4->dataFile->indexLocked == 1 )   /* index locked, so lock this tag as well */
      tfile4lock( tfile, data4serverId( d4 ) ) ;
   return tfile ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
TAG4 *S4FUNCTION t4openLow( DATA4 *d4, INDEX4 *i4ndx, const char *fileName, const char *dummy )
{
   CODE4 *c4 ;
   INDEX4 *i4 ;
   TAG4 *t4 ;
   int oldTagErr ;

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E94903 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( d4 == 0 || fileName == 0 )
      {
         error4( 0, e4parm_null, E94903 ) ;
         return 0 ; ;
      }
   #endif

   c4 = d4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return 0 ;

   oldTagErr = c4->errTagName ;
   c4->errTagName = 0 ;
   if ( d4tag( d4, fileName ) )
   {
      error4describe( c4, e4instance, E85308, fileName, 0, 0 ) ;
      c4->errTagName = oldTagErr ;
      return 0 ;
   }
   c4->errTagName = oldTagErr ;
   error4set( c4, 0 ) ;

   if ( i4ndx == 0 )   /* must create an index for the tag */
   {
      if ( c4->indexMemory == 0 )
      {
         c4->indexMemory = mem4create( c4, c4->memStartIndex, sizeof( INDEX4 ), c4->memExpandIndex, 0 ) ;
         if ( c4->indexMemory == 0 )
            return 0 ;
      }

      i4 = (INDEX4 *)mem4alloc( c4->indexMemory ) ;
      if ( i4 == 0 )
      {
         #ifdef E4STACK
            error4stack( c4, e4memory, E94903 ) ;
         #endif
         return 0 ;
      }

      i4->codeBase = c4 = d4->codeBase ;
      /* 09/22/95 AS - changed last parm to 1 from 0 for t4group (c/s access name needs extension if provided) */
      /* 05/21/97 AS - changed 2nd last parm to 1 from 0 -- (c/s access name needs path if provided) */
      u4namePiece( i4->accessName, sizeof( i4->accessName ), fileName, 1, 1 ) ;
    }
   else
      i4 = i4ndx ;

   if ( c4->tagMemory == 0 )
   {
      c4->tagMemory = mem4create( c4, c4->memStartTag, sizeof(TAG4), c4->memExpandTag, 0 ) ;
      if ( c4->tagMemory == 0 )
      {
         if ( i4ndx == 0 )
            mem4free( c4->indexMemory, i4 ) ;
         return 0 ;
      }
   }

   t4 = (TAG4 *)mem4alloc( c4->tagMemory ) ;
   if ( t4 == 0 )
   {
      #ifdef E4STACK
         error4stack( c4, e4memory, E94903 ) ;
      #endif
      if ( i4ndx == 0 )
         mem4free( c4->indexMemory, i4 ) ;
      return 0 ;
   }

   t4->tagFile = tfile4open( d4, fileName ) ;
   if ( t4->tagFile == 0 )
   {
      #ifdef E4STACK
         error4stack( c4, e4memory, E94903 ) ;
      #endif
      mem4free( c4->tagMemory, t4 ) ;
      if ( i4ndx == 0 )
         mem4free( c4->indexMemory, i4 ) ;
      return 0 ;
   }

   if ( t4->tagFile->header.unique )
      t4->errUnique = c4->errDefaultUnique ;
   t4->index = i4 ;
   if ( i4ndx == 0 )
   {
      i4->data = d4 ;
      l4add( &d4->indexes, i4 ) ;
   }

   l4add( &t4->index->tags, t4 ) ;
   #ifndef S4OFF_TRAN
      t4->isValid = 1 ;
   #endif
   return t4 ;
}

#ifndef S4OFF_WRITE
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int tfile4shrink( TAG4FILE *t4, long blockNo )
{
   #ifdef S4SINGLE
      t4->header.eof = blockNo * 512 ;
   #endif
   return 0 ;
}

int tfile4updateHeader( TAG4FILE *t4 )
{
   T4HEADER *header ;
   FILE4 *file ;
   FILE4LONG tLong ;

   #ifdef S4BYTE_SWAP
      I4IND_HEAD_WRITE swap ;
   #endif
   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   header = &t4->header ;
   file = &t4->file ;
/*   header->version = header->oldVersion + 2 ; */

   if ( header->oldVersion != header->version )
   {
      #ifdef S4BYTE_SWAP
         swap.sign = x4reverseShort( (void *)&header->sign ) ;
         swap.version = x4reverseShort( (void *)&header->version ) ;
         swap.root = x4reverseLong( (void *)&header->root ) ;
         swap.eof = x4reverseLong( (void *)&header->eof ) ;
         swap.groupLen = x4reverseShort( (void *)&header->groupLen ) ;
         swap.keyLen = x4reverseShort( (void *)&header->keyLen ) ;
         swap.keyDec = x4reverseShort( (void *)&header->keyDec ) ;
         swap.keysMax = x4reverseShort( (void *)&header->keysMax ) ;
         swap.keysHalf = x4reverseShort( (void *)&header->keysHalf ) ;

         if ( file4write( file, 0L, &swap, 2 * sizeof(S4LONG ) + 2 * sizeof( short ) ) < 0)
            return -1 ;

         header->unique = x4reverseShort( (void *)&header->unique ) ;
         header->descending = x4reverseShort( (void *)&header->descending ) ;
         #ifndef S4STRUCT_PAD
            if ( file4write( file, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE, &header->unique, sizeof(header->unique)) < 0 )
                    return -1 ;
            if ( file4write( file, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE + sizeof(header->unique) + 1, &header->descending, sizeof(header->descending)) < 0 )
               return -1 ;
         #else
            if ( file4write( file, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE - 2, &header->unique, sizeof(header->unique)) < 0 )
               return -1 ;
            if ( file4write( file, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE - 2 + sizeof(header->unique) + 1, &header->descending, sizeof(header->descending)) < 0 )
               return -1 ;
         #endif

         header->unique = x4reverseShort( (void *)&header->unique ) ;
         header->descending = x4reverseShort( (void *)&header->descending ) ;
      #else
         file4longAssign( tLong, 0, 0 ) ;
         if ( file4writeInternal( file, tLong, &header->sign, 2 * sizeof(S4LONG ) + 2 * sizeof( short ) ) < 0 )
            return -1 ;
         #ifndef S4STRUCT_PAD
            file4longAssign( tLong, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE, 0 ) ;
            if ( file4writeInternal( file, tLong, &header->unique, sizeof(header->unique)) < 0 )
               return -1 ;
            file4longAssign( tLong, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE + sizeof(header->unique) + 1, 0 ) ;
            if ( file4writeInternal( file, tLong, &header->descending, sizeof(header->descending)) < 0 )
               return -1 ;
         #else
            file4longAssign( tLong, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE - 2, 0 ) ;
            if ( file4writeInternal( file, tLong, &header->unique, sizeof(header->unique)) < 0 )
               return -1 ;
            file4longAssign( tLong, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE - 2 + sizeof(header->unique) + 1, 0 ) ;
            if ( file4writeInternal( file, tLong, &header->descending, sizeof(header->descending)) < 0 )
               return -1 ;
         #endif
      #endif
      header->oldVersion = header->version ;
   }
   return 0;
}
#endif  /* S4OFF_WRITE */

int tfile4goEof( TAG4FILE *t4 )
{
   int rc ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   rc = tfile4bottom( t4 ) ;
   if ( rc != 0 )
      return rc ;

   #ifdef E4ANALYZE
      if ( tfile4block( t4 ) == 0 )
         return error4( 0, e4info, E91642 ) ;
   #endif

   tfile4block(t4)->keyOn++ ;

   return 0 ;
}

int tfile4doVersionCheck( TAG4FILE *t4, int doSeek, int updateVersion )
{
   #ifndef S4SINGLE
      int rc, needSeek ;
      B4BLOCK *b4 ;
      CODE4 *c4 ;
      FILE4LONG fLong ;

      #ifdef E4PARM_LOW
         if ( t4 == 0 )
            return error4( 0, e4parm_null, E91635 ) ;
      #endif

      c4 = t4->codeBase ;

      if ( error4code( c4 ) < 0 )
         return e4codeBase ;

      if ( tfile4lockTest( t4 ) )
         return 0 ;

      #ifndef S4OPTIMIZE_OFF
         /* make sure read from disk */
         if ( t4->file.doBuffer )
            c4->opt.forceCurrent = 1 ;
      #endif

      file4longAssign( fLong, 0, 0 ) ;
      rc = file4readAllInternal( &t4->file, fLong, &t4->header.sign , 2 * sizeof( short ) + 2 * sizeof(S4LONG ) ) ;
      #ifndef S4OPTIMIZE_OFF
         if ( t4->file.doBuffer )
            c4->opt.forceCurrent = 0 ;
      #endif
      if ( rc < 0 )
         return rc ;
      #ifdef S4BYTE_SWAP
         t4->header.sign = x4reverseShort( (void *)&t4->header.sign ) ;
         t4->header.version = x4reverseShort( (void *)&t4->header.version ) ;
         t4->header.root = x4reverseLong( (void *)&t4->header.root ) ;
         t4->header.eof = x4reverseLong( (void *)&t4->header.eof ) ;
         /* t4->header.groupLen = x4reverseShort( (void *)&t4->header.groupLen ) ;  */
         /* t4->header.keyLen = x4reverseShort( (void *)&t4->header.keyLen ) ;      */
         /* t4->header.keyDec = x4reverseShort( (void *)&t4->header.keyDec ) ;      */
         /* t4->header.keysMax = x4reverseShort( (void *)&t4->header.keysMax ) ;    */
         /* t4->header.keysHalf = x4reverseShort( (void *)&t4->header.keysHalf ) ;  */
      #endif

      if ( t4->header.version == t4->header.oldVersion )
         return 0 ;

      if ( updateVersion == 1 )
         t4->header.oldVersion = t4->header.version ;
      else
         t4->header.version = t4->header.oldVersion ;

      /* remember the old position */
      needSeek = 0 ;
      if ( doSeek )
      {
         b4 = (B4BLOCK *)t4->blocks.lastNode ;  /* can't use tfile4block( t4 ) since might be null */
         if ( b4 != 0 )
         {
            if ( tfile4eof( t4 ) )
               needSeek = 2 ;
            else
/*               if ( b4leaf( b4 ) && b4->nKeys != 0 )
                 changed line 04/09/96 AS --> if on a branch, and no seek is performed, a gpf or general
                 error can later occur (in d4seek) --> this should be ok for S4CLIPPER */
               if ( b4->nKeys != 0 )
               {
                  memcpy( c4->savedKey, b4key( b4, b4->keyOn ), t4->header.keyLen + 2 * sizeof(S4LONG ) ) ;
                  needSeek = 1 ;
               }
         }
      }

      if ( tfile4freeAll( t4 ) < 0 )  /* Should be a memory operation only */
         #ifdef E4ANALYZE
            return error4( c4, e4result, E91635 ) ;
         #else
            return e4result ;
         #endif

      switch( needSeek )
      {
         case 1:
            #ifdef E4ANALYZE_ALL
               if ( tfile4go( t4, ((B4KEY_DATA *)c4->savedKey)->value, ((B4KEY_DATA *)c4->savedKey)->num, 0 ) != 0 )
                  return error4( c4, e4index, E91635 ) ;
            #else
               tfile4go( t4, ((B4KEY_DATA *)c4->savedKey)->value, ((B4KEY_DATA *)c4->savedKey)->num, 0 ) ;
            #endif
            break ;
         case 2:
            tfile4goEof( t4 ) ;
            break ;
      }

   #endif
   return 0;
}

int S4FUNCTION i4versionCheck( INDEX4 *index, const int d1, const int d2 )
{
   return error4( index->codeBase, e4notSupported, E94903 ) ;
}

int S4FUNCTION tfile4versionCheck( TAG4FILE *t4, const int doSeek, const int updateVersion )
{
   #ifndef S4OPTIMIZE_OFF
      if ( t4->file.doBuffer == 0 )
   #endif
   #ifndef S4SINGLE
      if ( tfile4lockTest( t4 ) == 0 )
   #endif
      return tfile4doVersionCheck( t4, doSeek, updateVersion ) ;
   return 0 ;
}

#endif  /* S4CLIPPER */
#endif  /* S4INDEX_OFF */

#ifdef S4VB_DOS
#ifdef S4CLIPPER

TAG4 *S4FUNCTION tfile4open_v(DATA4 *d4, char *name)
   {
      return tfile4open( d4, 0, c4str(name) ) ;
   }

#endif  /* S4CLIPPER */
#endif  /* S4VB_DOS */
