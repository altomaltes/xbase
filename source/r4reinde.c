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

/* r4reinde.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"


#ifndef S4OFF_WRITE
#ifndef S4INDEX_OFF

#ifndef S4OFF_TRAN
void i4deleteRemoveKeys( INDEX4 *index )
{
   TAG4 *tagOn ;
   TAG4KEY_REMOVED *removed ;

   for( tagOn = 0 ;; )
   {
      tagOn = (TAG4 *)l4next( &index->tags, tagOn ) ;
      if ( tagOn == 0 )
         break ;

      for ( ;; )
      {
         removed =(TAG4KEY_REMOVED *)l4first( &tagOn->removedKeys ) ;
         if ( removed == 0 )
            break ;
         l4remove( &tagOn->removedKeys, removed ) ;
         u4free( removed ) ;
      }
   }
}
#endif /* S4OFF_TRAN */

#ifndef N4OTHER
#include "r4reinde.h"

int S4FUNCTION i4reindex( INDEX4 *i4 )
{
   R4REINDEX reindex ;
   INDEX4FILE *indexFile ;
   CODE4 *c4 ;
   TAG4 *tagOn ;
   int rc ;
   DATA4 *data ;
   #ifndef S4MDX
      FILE4LONG fPos ;
   #endif
   #ifndef S4OPTIMIZE_OFF
      #ifdef S4LOW_MEMORY
         int hasOpt ;
      #endif
   #endif
   #ifdef S4FOX
      B4BLOCK *block ;
      S4LONG rNode, goTo ;
      char tagName[LEN4TAG_ALIAS + 1] ;
      S4LONG i ;
      int len ;
      #ifdef S4BYTE_SWAP
         char *swapPtr ;
         S4LONG longVal ;
         short shortVal ;
      #endif
   #endif  /* S4FOX */

   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E92101 ) )
         return -1 ;
   #endif  /* S4VBASIC */

   #ifdef E4PARM_HIGH
      if ( i4 == 0  )
         return error4( 0, e4parm_null, E92101 ) ;
   #endif

   c4 = i4->codeBase ;
   #ifdef E4ANALYZE
      if ( c4 == 0 )
         return error4( 0, e4struct, E92101 ) ;
   #endif

   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   indexFile = i4->indexFile ;
   data = i4->data ;

   #ifndef S4OPTIMIZE_OFF
      #ifdef S4LOW_MEMORY
         hasOpt = c4->hasOpt ;
         code4optSuspend( c4 ) ;
      #endif
   #endif

   #ifndef S4SINGLE
      rc = d4lockAll( data ) ;
      if ( rc )
         return rc ;
   #endif  /* S4SINGLE */

   #ifndef S4OFF_TRAN
      /* reindex is allowed, but need to fix-up any unique settings */
      i4deleteRemoveKeys( i4 ) ;
   #endif

   for( ;; )
   {
      rc = r4reindexInit( &reindex, i4, i4->indexFile ) ;
      if ( rc < 0 )
         break ;
      rc = r4reindexTagHeadersCalc( &reindex ) ;
      if ( rc < 0 )
         break ;
      rc = r4reindexBlocksAlloc(&reindex ) ;
      if ( rc < 0 )
         break ;

      #ifndef S4SINGLE
         #ifdef S4FOX
            if ( indexFile->file.lowAccessMode != OPEN4DENY_RW )
               indexFile->tagIndex->header.version = indexFile->versionOld + 1 ;
         #endif
      #endif

      #ifdef E4ANALYZE
         if ( i4->tags.nLink != indexFile->tags.nLink )
         {
            rc = e4struct ;
            break ;
         }
      #endif

      #ifdef S4FOX
         reindex.nBlocksUsed = 0 ;
         tagName[LEN4TAG_ALIAS] = '\0' ;

         if ( indexFile->tagIndex->header.typeCode >= 64 )  /* if .cdx */
         {
            reindex.tag = indexFile->tagIndex ;
            rc = sort4init( &reindex.sort, c4, indexFile->tagIndex->header.keyLen, 0 ) ;
            if ( rc < 0 )
               break ;
            reindex.sort.cmp = (S4CMP_FUNCTION *)u4memcmp ;

            for( tagOn = 0, i = 1 ; ; i++ )
            {
               tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
               if ( tagOn == 0 )
                  break ;
               len = strlen( tfile4alias( tagOn->tagFile ) ) ;
               memset( tagName, ' ', LEN4TAG_ALIAS ) ;
               memcpy( tagName, tfile4alias( tagOn->tagFile ), (unsigned int)len ) ;
               rc = sort4put( &reindex.sort, 2 * B4BLOCK_SIZE * i, tagName, "" ) ;
               if ( rc < 0 )
                  break ;
               #ifdef E4MISC
                  reindex.keyCount++ ;
               #endif /* E4MISC */
            }

            if ( rc < 0 )
               break ;

            rc = r4reindexWriteKeys( &reindex, e4unique ) ;  /* tag index should have no uniques */
            if ( rc != 0 )
            {
               r4reindexFree( &reindex ) ;
               break ;
            }
         }
         else
            if ( reindex.nTags > 1 )   /* should only be 1 tag in an .idx */
            {
               rc = e4index ;
               break ;
            }
      #endif  /* S4FOX */

      for( tagOn = 0 ; ; )
      {
         tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
         if ( tagOn == 0 )
            break ;
         reindex.tag = tagOn->tagFile ;
         #ifdef S4FOX
            reindex.nBlocksUsed = 0 ;
         #else  /* S4MDX */
            reindex.tag->header.version++ ;
         #endif  /* S4FOX */

         rc = expr4context( tagOn->tagFile->expr, data ) ;
         if ( rc < 0 )
            break ;
         if ( tagOn->tagFile->filter != 0 )
         {
            rc = expr4context( tagOn->tagFile->filter, data ) ;
            if ( rc < 0 )
               break ;
         }

         rc = r4reindexSupplyKeys( &reindex ) ;
         if ( rc )
         {
            r4reindexFree( &reindex ) ;
            break ;
         }

         rc = r4reindexWriteKeys( &reindex, t4unique( tagOn ) ) ;
         if ( rc )
         {
            r4reindexFree( &reindex ) ;
            break ;
         }
      }

      if ( rc != 0 )   /* r4unique or error */
         break ;

      rc = r4reindexTagHeadersWrite( &reindex ) ;
      if ( rc < 0 )
         break ;

      #ifdef S4FOX
         /* now must fix the right node branches for all blocks by moving leftwards */
         for( tagOn = 0 ; ; )
         {
            tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
            if ( tagOn == 0 )
               break ;

            for( tfile4rlBottom( tagOn->tagFile ) ; tagOn->tagFile->blocks.lastNode ; tfile4up( tagOn->tagFile ) )
            {
               block = tfile4block( tagOn->tagFile ) ;
               goTo = block->header.leftNode ;

               while ( goTo != -1 )
               {
                  #ifdef E4DEBUG
                     if ( goTo <= 0 )  /* invalid value, esp. zero */
                        return error4( c4, e4struct, E92101 ) ;
                  #endif

                  rNode = block->fileBlock ;
                  if ( block->changed )
                  {
                     rc = b4flush( block ) ;
                     if ( rc < 0 )
                        break ;
                  }

                  file4longAssign( fPos, I4MULTIPLY * goTo, 0 ) ;
                  rc = file4readAllInternal( &indexFile->file, fPos, &block->header, B4BLOCK_SIZE ) ;
                  if ( rc < 0 )
                     break ;

                  #ifdef S4BYTE_SWAP
                     block->header.nodeAttribute = x4reverseShort( (void *)&block->header.nodeAttribute ) ;
                     block->header.nKeys = x4reverseShort( (void *)&block->header.nKeys ) ;
                     block->header.leftNode = x4reverseLong( (void *)&block->header.leftNode ) ;
                     block->header.rightNode = x4reverseLong( (void *)&block->header.rightNode ) ;

                     if (block->header.nodeAttribute >= 2 ) /* if block is a leaf */
                     {
                        block->nodeHdr.freeSpace = x4reverseShort( (void *)&block->nodeHdr.freeSpace ) ;
                        longVal = x4reverseLong( (void *)&block->nodeHdr.recNumMask[0] ) ;
                        memcpy( (void *)&block->nodeHdr.recNumMask[0], (void *)&longVal, sizeof(S4LONG) ) ;
                     }
                     else /* if block is a branch */
                     {
                        shortVal = block->tag->header.keyLen + sizeof(S4LONG) ;
                        /* position swapPtr to end of first key expression */
                        swapPtr = (char *) &block->nodeHdr.freeSpace + block->tag->header.keyLen ;

                        /* move through all B4KEY's to swap 'long's */
                        for ( i = 0 ; i < (int) block->header.nKeys ; i++ )
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

                  block->fileBlock = goTo ;
                  if ( block->header.rightNode != rNode )  /* if a bad value */
                  {
                     block->header.rightNode = rNode ;
                     block->changed = 1 ;
                  }
                  goTo = block->header.leftNode ;
               }
               if ( rc < 0 )
                  break ;
               block->builtOn = -1 ;
               rc = b4top( block ) ;
               if ( rc < 0 )
                  break ;
            }
            if ( rc < 0 )
               break ;
         }
         if ( rc < 0 )
            break ;
      #endif  /* S4FOX */

      r4reindexFree( &reindex ) ;
      #ifndef S4OPTIMIZE_OFF
         #ifdef S4LOW_MEMORY
            if ( hasOpt )
               code4optRestart( c4 ) ;
         #endif
      #endif

      break ;
   }
   if ( rc < 0 )
      return error4stack( c4, (short)rc, E92101 ) ;

   data->recNum = -1 ;
   data->recNumOld = -1 ;
   d4blankLow( data, data->record ) ;

   return rc ;
}

int r4reindexInit( R4REINDEX *r4, INDEX4 *i4, INDEX4FILE *indexFile )
{
   #ifdef E4PARM_LOW
      if ( r4 == 0 || i4 == 0 || indexFile == 0 )
         return error4( 0, e4parm_null, E92102 ) ;
   #endif

   memset( (void *)r4, 0, sizeof( R4REINDEX ) ) ;

   r4->indexFile = indexFile ;
   r4->data = i4->data ;
   r4->dataFile = indexFile->dataFile ;
   r4->codeBase = i4->codeBase ;

   r4->minKeysmax = INT_MAX ;
   r4->startBlock = 0 ;
   r4->sort.file.hand = INVALID4HANDLE ;

   #ifndef S4FOX
      r4->blocklen = indexFile->header.blockRw ;
   #endif  /* S4FOX */

   r4->bufferLen = i4->codeBase->memSizeSortBuffer ;
   if ( r4->bufferLen < 1024 )
      r4->bufferLen = 1024 ;

   r4->buffer = (char *)u4allocEr( i4->codeBase, (S4LONG)r4->bufferLen ) ;
   if ( r4->buffer == 0 )
      return e4memory ;

   #ifdef S4FOX
      r4->lastblock = 1024 ;  /* leave space for the index header block  */
   #endif  /* S4FOX */

   return 0 ;
}

void r4reindexFree( R4REINDEX *r4 )
{
   u4free( r4->buffer ) ;
   u4free( r4->startBlock ) ;
   sort4free( &r4->sort ) ;
}

int r4reindexBlocksAlloc( R4REINDEX *r4 )
{
   S4LONG onCount ;

   #ifdef E4PARM_LOW
      if ( r4 == 0 )
         return error4( 0, e4parm_null, E92102 ) ;
   #endif

   #ifdef E4MISC
      if ( (unsigned)r4->minKeysmax > INT_MAX )
         return error4( r4->codeBase, e4info, E92102 ) ;
   #endif

   /* Calculate the block stack height */
   onCount = dfile4recCount( r4->dataFile, -2 ) ;
   if ( onCount < 0 || r4->minKeysmax <= 1 )
      return error4stack( r4->codeBase, (short)onCount, E92102 ) ;
   for ( r4->nBlocks = 2; onCount != 0L; r4->nBlocks++ )
      onCount /= r4->minKeysmax ;

   if ( r4->startBlock == 0 )
   {
      #ifdef S4FOX
         r4->startBlock = (R4BLOCK_DATA *)u4allocEr( r4->codeBase, (S4LONG)B4BLOCK_SIZE * r4->nBlocks ) ;
      #endif  /* S4FOX */

      #ifdef S4MDX
         r4->startBlock = (R4BLOCK_DATA *)u4allocEr( r4->codeBase, (S4LONG)r4->blocklen * r4->nBlocks ) ;
      #endif  /* S4MDX */
   }

   if ( r4->startBlock == 0 )
      return e4memory ;

   return 0 ;
}

int r4reindexSupplyKeys( R4REINDEX *r4 )
{
   FILE4SEQ_READ seqRead ;
   EXPR4 *filter ;
   unsigned char *keyResult ;
   int rc, *filterResult ;
   S4LONG  count, iRec ;
   DATA4FILE *dataFile ;
   TAG4FILE *t4 ;
   #ifndef S4MEMO_OFF
      int i ;
   #endif

   #ifdef E4PARM_LOW
      if ( r4 == 0 )
         return error4( 0, e4parm_null, E92102 ) ;
   #endif

   dataFile = r4->dataFile ;
   t4 = r4->tag ;
   #ifdef E4MISC
      r4->keyCount = 0L ;
   #endif  /* E4MISC */
   rc =sort4init( &r4->sort, r4->codeBase, t4->header.keyLen, 0 ) ;
   if ( rc < 0 )
      return error4stack( r4->codeBase, (short)rc, E92102 ) ;

   #ifdef S4FOX
      r4->sort.cmp = (S4CMP_FUNCTION *)u4memcmp ;
   #endif  /* S4FOX */

   #ifdef S4MDX
      r4->sort.cmp = (S4CMP_FUNCTION *)t4->cmp ;
   #endif  /* S4MDX */

   filter = t4->filter ;
   count = dfile4recCount( dataFile, -2L ) ;
   if ( count < 0 )
      return error4stack( r4->codeBase, (short)rc, E92102 ) ;

   rc = expr4context( r4->tag->expr, r4->data ) ;
   if ( rc < 0 )
      return rc ;
   if ( r4->tag->filter != 0 )
   {
      rc = expr4context( r4->tag->filter, r4->data ) ;
      if ( rc < 0 )
         return rc ;
   }

   file4seqReadInitDo( &seqRead, &dataFile->file, dfile4recordPosition( dataFile, 1L ), r4->buffer, r4->bufferLen, 1 ) ;

   for ( iRec = 1L; iRec <= count; iRec++ )
   {
      rc = file4seqReadAll( &seqRead, dataFile->record, dfile4recWidth( dataFile ) ) ;
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;
      r4->data->recNum = iRec ;

      #ifndef S4MEMO_OFF
         for ( i = 0; i < dataFile->nFieldsMemo; i++ )
            f4memoReset( r4->data->fieldsMemo[i].field ) ;
      #endif  /* S4MEMO_OFF */

      if ( filter )
      {
         rc = expr4vary( filter, (char **)&filterResult ) ;
         if ( rc < 0 )
         {
            #ifdef S4ADVANCE_READ
               file4seqReadInitUndo( &seqRead ) ;
            #endif
            return error4stack( r4->codeBase, (short)rc, E92102 ) ;
         }
         #ifdef E4MISC
            if ( expr4type( filter ) != r4log )
            {
               #ifdef S4ADVANCE_READ
                  file4seqReadInitUndo( &seqRead ) ;
               #endif
               return error4( r4->codeBase, e4result, E92102 ) ;
            }
         #endif  /* E4MISC */
         if ( ! *filterResult )
            continue ;
         t4->hasKeys = 1 ;
         #ifdef S4MDX
            t4->hadKeys = 0 ;
         #endif
      }

      tfile4exprKey( t4, &keyResult ) ;
      rc = sort4put( &r4->sort, iRec, keyResult, "" ) ;
      if ( rc < 0 )
      {
         #ifdef S4ADVANCE_READ
            file4seqReadInitUndo( &seqRead ) ;
         #endif
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;
      }
      #ifdef E4MISC
         r4->keyCount++ ;
      #endif  /* E4MISC */
   }

   #ifdef S4ADVANCE_READ
      file4seqReadInitUndo( &seqRead ) ;
   #endif

   return 0 ;
}

int r4reindexTagHeadersCalc( R4REINDEX *r4 )
{
   TAG4FILE *tag ;
   int rc ;
   CODE4 *c4 ;
   #ifdef S4FOX
      #ifdef S4DATA_ALIGN
         unsigned int size, delta ;
      #endif
      int keysmax, exprType ;
   #endif  /* S4FOX */

   #ifdef E4PARM_LOW
      if ( r4 == 0 )
         return error4( 0, e4parm_null, E92102 ) ;
   #endif

   c4 = r4->codeBase ;
   r4->nTags = 0 ;
   for( tag = 0 ;; )
   {
      tag = (TAG4FILE *)l4next( &r4->indexFile->tags, tag ) ;
      if ( tag == 0 )
         break ;
      rc = tfile4freeAll( tag ) ;
      if ( rc < 0 )
         return error4stack( c4, (short)rc, E92102 ) ;

      expr4context( tag->expr, r4->data ) ;

      tag->header.keyLen = (short)expr4keyLen( tag->expr ) ;
      #ifdef S4FOX
         #ifdef S4DATA_ALIGN
            size = (unsigned int)sizeof(S4LONG) + tag->header.keyLen ;
            delta = sizeof(void *) - size % sizeof(void *) ;
            tag->builtKeyMemory = mem4create( c4, 3, size + delta, 2, 0 ) ;
         #else
            tag->builtKeyMemory = mem4create( c4, 3, (unsigned int)sizeof(S4LONG) + tag->header.keyLen + 1, 2, 0 ) ;
         #endif
         exprType = expr4type( tag->expr ) ;
         if ( exprType < 0 )
            return error4stack( c4, (short)rc, E92102 ) ;
         tfile4initSeekConv( tag, exprType ) ;
         if ( tag->header.keyLen < 0 )
            return error4( c4, e4index, E92102 ) ;

         keysmax = ( B4BLOCK_SIZE - sizeof(B4STD_HEADER) ) / ( tag->header.keyLen + 2*sizeof(S4LONG) ) ;

         if ( keysmax < r4->minKeysmax )
            r4->minKeysmax = keysmax ;
      #endif  /* S4FOX */

      #ifdef S4MDX
         if ( tag->header.keyLen < 0 )
            return error4( c4, e4index, E92102 ) ;

         rc = expr4type( tag->expr ) ;
         if ( rc < 0 )
            return error4( c4, rc, E92102 ) ;
         tag->header.type = (char)rc ;
         if ( tag->header.type == r4dateDoub )
            tag->header.type = r4date ;
         if ( tag->header.type == r4numDoub )
            tag->header.type = r4num ;

         tfile4initSeekConv( tag, tag->header.type ) ;
         tag->header.groupLen = (short)(tag->header.keyLen+ 2*sizeof(S4LONG)-1) ;
         tag->header.groupLen-= (short)(tag->header.groupLen % sizeof(S4LONG)) ;
         tag->header.isDate = ( tag->header.type == r4date ) ? (short) 1 : (short) 0 ;

         tag->header.keysMax = (short)((r4->indexFile->header.blockRw - sizeof(short)
                       - 6 - sizeof(S4LONG)) / tag->header.groupLen) ;
         if ( tag->header.keysMax < r4->minKeysmax )
            r4->minKeysmax = tag->header.keysMax ;
         tag->hasKeys = 0 ;
         tag->hadKeys = 1 ;
      #endif  /* S4MDX */

      #ifdef E4DEBUG
         if ( I4MAX_KEY_SIZE_COMPATIBLE < tag->header.keyLen )
         {
            if ( r4->codeBase->oledbSchemaCreate == 1 )  /* ensure via real max length (not-compatible) */
            {
               if ( I4MAX_KEY_SIZE < tag->header.keyLen )
                  return error4( r4->codeBase, e4index, E82102 ) ;
            }
            else
               return error4( r4->codeBase, e4index, E82102 ) ;
         }
      #endif

      r4->nTags++ ;
   }

   #ifdef S4FOX
      if ( r4->indexFile->tagIndex->header.typeCode >= 64 )
      {
         r4->lastblock += ((S4LONG)r4->nTags-1)*2*B4BLOCK_SIZE + B4BLOCK_SIZE ;
         tfile4initSeekConv( r4->indexFile->tagIndex, r4str ) ;
      }
      else
         r4->lastblock -= B4BLOCK_SIZE ;
   #endif  /* S4FOX */

   #ifdef S4MDX
      r4->lastblockInc = r4->indexFile->header.blockRw/ 512 ;
      r4->lastblock = 4 + (r4->nTags-1)*r4->lastblockInc ;
   #endif  /* S4MDX */

   return 0 ;
}

TAG4FILE *r4reindexFindITag( R4REINDEX *r4, const int tagNum )
{
   /* First 'iTag' starts at '1' for this specific routine */
   TAG4FILE *tagOn ;
   int iTag ;

   #ifdef E4PARM_LOW
      if ( r4 == 0 || tagNum <= 0 )
      {
         error4( 0, e4parm, E92102 ) ;
         return 0 ;
      }
   #endif

   iTag = tagNum ;

   tagOn = (TAG4FILE *)l4first( &r4->indexFile->tags ) ;

   while ( --iTag >= 1 )
   {
      tagOn = (TAG4FILE *) l4next( &r4->indexFile->tags, tagOn ) ;
      if ( tagOn == 0 )
         return 0 ;
   }
   return tagOn ;
}

#ifdef S4MDX

#define GARBAGE_LEN 518

int r4reindexTagHeadersWrite( R4REINDEX *r4 )
{
   /* First, calculate the T4DESC.leftChld, T4DESC.rightChld values, T4DESC.parent values */
   int rc, higher[49], lower[49], parent[49] ;
   TAG4FILE *tagOn, *tagPtr ;
   INDEX4FILE *i4 ;
   DATA4 *d4 ;
   CODE4 *c4 ;
   int nTag, iTag, jField, len, saveCode ;
   T4DESC tagInfo ;
   const char *ptr ;
   FILE4LONG fPos ;
   #ifdef S4BYTE_SWAP
      I4HEADER swapHeader ;
      T4HEADER swapTagHeader ;
   #endif  /* S4BYTE_SWAP */

   #ifdef E4PARM_LOW
      if ( r4 == 0 )
         return error4( 0, e4parm_null, E92102 ) ;
   #endif

   memset( (void *)higher, 0, sizeof( higher ) ) ;
   memset( (void *)lower,  0, sizeof( lower ) ) ;
   memset( (void *)parent, 0, sizeof( parent ) ) ;

   i4 = r4->indexFile ;
   d4 = r4->data ;
   c4 = r4->codeBase ;

   #ifdef E4ANALYZE
      if ( i4 == 0 || d4 == 0 )
         return error4( c4, e4struct, E92102 ) ;
   #endif

   tagOn = (TAG4FILE *) l4first( &i4->tags ) ;
   if ( tagOn != 0 )
   {
      nTag = 1 ;

      for ( ;; )
      {
         tagOn = (TAG4FILE *)l4next( &i4->tags, tagOn) ;
         if ( tagOn == 0 )
            break ;
         nTag++ ;
         iTag = 1 ;
         for (;;)
         {
            tagPtr = r4reindexFindITag( r4, iTag ) ;
            #ifdef E4MISC
               if ( tagPtr == 0 || iTag < 0 || iTag >= 48 || nTag > 48 )
                  return error4( c4, e4result, E92102 ) ;
            #endif  /* E4MISC */
            if ( u4memcmp( tagOn->alias, tagPtr->alias, sizeof(tagOn->alias)) < 0)
            {
               if ( lower[iTag] == 0 )
               {
                  lower[iTag] = nTag ;
                  parent[nTag] = iTag ;
                  break ;
               }
               else
                  iTag = lower[iTag] ;
            }
            else
            {
               if ( higher[iTag] == 0 )
               {
                  higher[iTag] = nTag ;
                  parent[nTag] = iTag ;
                  break ;
               }
               else
                  iTag = higher[iTag] ;
            }
         }
      }
   }

   /* Now write the headers */
   file4longAssign( fPos, 0, 0 ) ;
   file4seqWriteInitLow( &r4->seqwrite, &i4->file, fPos, r4->buffer, r4->bufferLen ) ;

   i4->header.eof = r4->lastblock + r4->lastblockInc ;
   i4->header.freeList = 0L ;
   u4yymmdd( i4->header.yymmdd ) ;

   #ifdef S4BYTE_SWAP
      memcpy( (void *)&swapHeader, (void *)&i4->header, sizeof(I4HEADER) ) ;

      swapHeader.blockChunks = x4reverseShort( (void *)&swapHeader.blockChunks ) ;
      swapHeader.blockRw = x4reverseShort( (void *)&swapHeader.blockRw ) ;
      swapHeader.slotSize = x4reverseShort( (void *)&swapHeader.slotSize ) ;
      swapHeader.numTags = x4reverseShort( (void *)&swapHeader.numTags ) ;
      swapHeader.eof = x4reverseLong( (void *)&swapHeader.eof ) ;
      swapHeader.freeList = x4reverseLong( (void *)&swapHeader.freeList ) ;

      rc = file4seqWrite( &r4->seqwrite, &swapHeader, sizeof(I4HEADER) ) ;
   #else
      rc = file4seqWrite( &r4->seqwrite, &i4->header, sizeof(I4HEADER) ) ;
   #endif  /* S4BYTE_SWAP */

   if ( rc < 0 )
      return error4stack( c4, rc, E92102 ) ;

   rc = file4seqWriteRepeat( &r4->seqwrite, 512-sizeof(I4HEADER)+17, 0 ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E92102 ) ;

   /* There is a 0x01 on byte 17 of the first 32 bytes. */
   rc = file4seqWrite( &r4->seqwrite, "\001", 1 ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E92102 ) ;

   rc = file4seqWriteRepeat( &r4->seqwrite, 14, 0 ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E92102 ) ;

   tagOn = (TAG4FILE *) l4first( &i4->tags ) ;

   for ( iTag = 0; iTag < 47; iTag++ )
   {
      memset( (void *)&tagInfo, 0, sizeof(tagInfo) ) ;

      if ( iTag < r4->nTags )
      {
         tagInfo.headerPos = 4 + (S4LONG) iTag * r4->lastblockInc ;
         tagOn->headerOffset = tagInfo.headerPos * 512 ;

         memcpy( (void *)tagInfo.tag, tagOn->alias, sizeof(tagInfo.tag) ) ;

         tagInfo.indexType = tagOn->header.type ;

         #ifdef S4BYTE_SWAP
            tagInfo.headerPos = x4reverseLong( (void *)&tagInfo.headerPos ) ;
            tagInfo.x1000 = 0x0010 ;
         #else
            tagInfo.x1000 = 0x1000 ;
         #endif  /* S4BYTE_SWAP */

         tagInfo.x2 = 2 ;
         tagInfo.leftChld = (char) lower[iTag+1] ;
         tagInfo.rightChld = (char) higher[iTag+1] ;
         tagInfo.parent = (char) parent[iTag+1] ;

         if ( i4->header.isProduction )
         {
            saveCode = c4->errFieldName ;
            c4->errFieldName = 0 ;
            jField = d4fieldNumber( d4, tagOn->expr->source ) ;
            c4->errFieldName = saveCode ;
            if ( jField > 0 )
            {
               file4longAssign( fPos, ( jField + 1 ) * sizeof( FIELD4IMAGE ) - 1, 0 ) ;
               rc = file4writeInternal( &r4->dataFile->file, fPos, "\001", 1 ) ;
               if ( rc < 0 )
                  return error4stack( c4, rc, E92102 ) ;
            }
         }
         tagOn = (TAG4FILE *)l4next( &i4->tags, tagOn ) ;
      }
      rc = file4seqWrite( &r4->seqwrite, &tagInfo, sizeof( T4DESC ) ) ;
      if ( rc < 0 )
         return error4stack( c4, rc, E92102 ) ;
   }

   for (tagOn = 0 ;; )
   {
      tagOn = (TAG4FILE *)l4next( &i4->tags, tagOn ) ;
      if ( tagOn == 0 )
         break ;
      #ifdef S4BYTE_SWAP
         memcpy( (void *)&swapTagHeader, (void *)&tagOn->header, sizeof(T4HEADER) ) ;

         swapTagHeader.root = x4reverseLong( (void *)&swapTagHeader.root ) ;
         swapTagHeader.keyLen = x4reverseShort( (void *)&swapTagHeader.keyLen ) ;
         swapTagHeader.keysMax = x4reverseShort( (void *)&swapTagHeader.keysMax ) ;
         swapTagHeader.groupLen = x4reverseShort( (void *)&swapTagHeader.groupLen ) ;
         swapTagHeader.isDate = x4reverseShort( (void *)&swapTagHeader.isDate ) ;
         swapTagHeader.unique = x4reverseShort( (void *)&swapTagHeader.unique ) ;

         rc = file4seqWrite( &r4->seqwrite, &swapTagHeader, sizeof( T4HEADER ) ) ;
      #else
         rc = file4seqWrite( &r4->seqwrite, &tagOn->header, sizeof( T4HEADER ) ) ;
      #endif  /* S4BYTE_SWAP */
      if ( rc < 0 )
         return error4stack( c4, rc, E92102 ) ;

      ptr = tagOn->expr->source ;
      len = strlen( ptr ) ;
      rc = file4seqWrite( &r4->seqwrite, ptr, len) ;
      if ( rc < 0 )
         return error4stack( c4, rc, E92102 ) ;

      rc = file4seqWriteRepeat( &r4->seqwrite, 221-len, 0 ) ;
      if ( rc < 0 )
         return error4stack( c4, rc, E92102 ) ;

      if( tagOn->filter != 0 )
      {
         rc = file4seqWriteRepeat( &r4->seqwrite, 1, 1 ) ;
         if ( rc < 0 )
            return error4stack( c4, rc, E92102 ) ;
         if ( tagOn->hasKeys )
            rc = file4seqWriteRepeat( &r4->seqwrite, 1, 1 ) ;
         else
            rc = file4seqWriteRepeat( &r4->seqwrite, 1, 0 ) ;
      }
      else
         rc = file4seqWriteRepeat( &r4->seqwrite, 2, 0 ) ;

      if ( rc < 0 )
         return error4stack( c4, rc, E92102 ) ;

      /* write extra space up to filter write point */
      rc = file4seqWriteRepeat( &r4->seqwrite, GARBAGE_LEN-3 , 0 ) ;
      if ( rc < 0 )
         return error4stack( c4, rc, E92102 ) ;

      if ( tagOn->filter == 0 )
         len = 0 ;
      else
      {
         ptr = tagOn->filter->source ;
         len = strlen(ptr) ;
         rc = file4seqWrite( &r4->seqwrite, ptr, len ) ;
         if ( rc < 0 )
            return error4stack( c4, rc, E92102 ) ;
      }
      rc = file4seqWriteRepeat( &r4->seqwrite, r4->blocklen - GARBAGE_LEN - len - 220 - sizeof(tagOn->header), 0 ) ;
      if ( rc < 0 )
         return error4stack( c4, rc, E92102 ) ;
   }
   file4longAssign( fPos, i4->header.eof * 512, 0 ) ;
   rc = file4lenSetLow( &i4->file, fPos ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E92102 ) ;

   rc =  file4seqWriteFlush( &r4->seqwrite ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E92102 ) ;
   return 0 ;
}

int r4reindexWriteKeys( R4REINDEX *r4, short int errUnique )
{
   TAG4FILE *t4 ;
   char  lastKey[I4MAX_KEY_SIZE], *keyData ;
   int   isUnique, rc, isFirst ;
   void *dummyPtr ;
   S4LONG  keyRec ;
   FILE4LONG pos ;

   #ifdef E4PARM_LOW
      if ( r4 == 0 )
         return error4( 0, e4parm_null, E92102 ) ;
   #endif

   t4 = r4->tag ;

   r4->grouplen = t4->header.groupLen ;
   r4->valuelen = t4->header.keyLen ;
   r4->keysmax = t4->header.keysMax ;
   memset( (void *)r4->startBlock, 0, r4->nBlocks*r4->blocklen ) ;
   rc = sort4getInit( &r4->sort ) ;
   if ( rc < 0 )
      return error4stack( r4->codeBase, rc, E92102 ) ;

   file4longAssign( pos, ( r4->lastblock + r4->lastblockInc ) * 512, 0 ) ;
   file4seqWriteInitLow( &r4->seqwrite, &r4->indexFile->file, pos, r4->buffer,r4->bufferLen) ;

   #ifdef E4MISC
      if ( I4MAX_KEY_SIZE_COMPATIBLE < r4->sort.sortLen )
      {
         if ( r4->codeBase->oledbSchemaCreate == 1 )  /* ensure via real max length (not-compatible) */
         {
            if ( I4MAX_KEY_SIZE < r4->sort.sortLen )
               return error4( r4->codeBase, e4index, E82102 ) ;
         }
         else
            return error4( r4->codeBase, e4index, E82102 ) ;
      }
   #endif

   isFirst = 1 ;
   isUnique = t4->header.unique ;

   for(;;)  /* For each key to write */
   {
      rc = sort4get( &r4->sort, &keyRec, (void **) &keyData, &dummyPtr) ;
      if ( rc < 0 )
         return error4stack( r4->codeBase, rc, E92102 ) ;

      #ifdef E4MISC
         if ( r4->keyCount < 0L || r4->keyCount == 0L && rc != r4done || r4->keyCount > 0L && rc == r4done )
            return error4( r4->codeBase, e4info, E92102 ) ;
         r4->keyCount-- ;
      #endif

      if ( rc == r4done )  /* No more keys */
      {
         rc = r4reindexFinish( r4 ) ;
         if ( rc < 0 )
            return error4stack( r4->codeBase, rc, E92102 ) ;
         rc = file4seqWriteFlush( &r4->seqwrite ) ;
         if ( rc < 0 )
            return error4stack( r4->codeBase, rc, E92102 ) ;
         break ;
      }

      if ( isUnique )
      {
         if( isFirst )
            isFirst = 0 ;
         else
            if ( (*t4->cmp)( keyData, lastKey, r4->sort.sortLen) == 0 )
            {
               switch( errUnique )
               {
                  case e4unique:
                     return error4describe( r4->codeBase, e4unique, E82103, t4->alias, (char *)0, (char *)0 ) ;
                  case r4unique:
                     return r4unique ;
                  default:
                     continue ;
               }
            }
         memcpy( lastKey, keyData, r4->sort.sortLen ) ;
      }

      /* Add the key */
      rc = r4reindexAdd( r4, keyRec, (unsigned char *)keyData ) ;
      if ( rc < 0 )
         return error4stack( r4->codeBase, rc, E92102 ) ;
   }

   /* Now complete the tag header info. */
   t4->header.root = r4->lastblock ;
   return 0 ;
}

static int r4reindexToDisk( R4REINDEX *r4 )
{
   R4BLOCK_DATA *block ;
   int iBlock, rc ;
   B4KEY_DATA *keyOn, *keyTo ;
   #ifdef E4ANALYZE
      S4LONG dif ;
   #endif

   #ifdef S4BYTE_SWAP
      char *swap, *swapPtr ;
      int i ;
      S4LONG longVal ;
      short shortVal ;
   #endif  /* S4BYTE_SWAP */

   #ifdef E4PARM_LOW
      if ( r4 == 0 )
         return error4( 0, e4parm_null, E92102 ) ;
   #endif

   /* Writes out the current block and adds references to higher blocks */
   block = r4->startBlock ;
   iBlock= 0 ;

   keyOn = (B4KEY_DATA *) (block->info + (block->nKeys-1) * r4->grouplen) ;

   #ifdef E4ANALYZE
      dif = (char *) keyOn -  (char *) block ;
      if ( ( (unsigned)dif + r4->grouplen ) > r4->blocklen || dif < 0 )
         return error4( r4->codeBase, e4result, E92102 ) ;
   #endif /* E4ANALYZE */

   for(;;)
   {
      #ifdef S4BYTE_SWAP
         swap = (char *) u4allocEr( r4->codeBase, r4->blocklen + sizeof(S4LONG) ) ;
         if ( swap == 0 )
            return error4stack( r4->codeBase, e4memory, E92102 ) ;

         memcpy( (void *)swap, (void *)block, r4->blocklen ) ;
         /* position swapPtr at beginning of B4KEY's */
         swapPtr = swap ;
         swapPtr += 6 + sizeof(short) ;
         /* move through all B4KEY's to swap 'long' */
         for ( i = 0 ; i < (*(short *)swap) ; i++ )
         {
            longVal = x4reverseLong( (void *)swapPtr ) ;
            memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
            swapPtr += r4->grouplen ;
         }

         longVal = x4reverseLong( (void *)swapPtr ) ;
         memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;

         /* swap the numKeys value */
         shortVal = x4reverseShort( (void *)swap ) ;
         memcpy( swap, (void *) &shortVal, sizeof(short) ) ;

         rc = file4seqWrite( &r4->seqwrite, swap, r4->blocklen) ;
         u4free( swap ) ;
      #else
         rc = file4seqWrite( &r4->seqwrite, block, r4->blocklen ) ;
      #endif  /* S4BYTE_SWAP */
      if ( rc < 0 )
         return error4stack( r4->codeBase, rc, E92102 ) ;

      if ( iBlock )
         memset( (void *)block, 0, r4->blocklen ) ;
      r4->lastblock += r4->lastblockInc ;

      block = (R4BLOCK_DATA *) ((char *)block + r4->blocklen) ;
      iBlock++ ;
      #ifdef E4ANALYZE
         if ( iBlock >= r4->nBlocks )
            return error4( r4->codeBase, e4info, E92102 ) ;
      #endif  /* E4ANALYZE */

      keyTo = (B4KEY_DATA *) (block->info +  block->nKeys * r4->grouplen) ;
      #ifdef E4ANALYZE
         dif = (char *) keyTo -  (char *) block  ;
         if ( ( (unsigned)dif + sizeof(S4LONG ) ) > r4->blocklen || dif < 0 )
            return error4( r4->codeBase, e4result, E92102 ) ;
      #endif  /* E4ANALYZE */
      keyTo->num = r4->lastblock ;

      if ( block->nKeys < r4->keysmax )
      {
         block->nKeys++ ;
         #ifdef E4ANALYZE
            if ( ((unsigned)dif+r4->grouplen) > r4->blocklen )
               return error4( r4->codeBase, e4result, E92102 ) ;
         #endif  /* E4ANALYZE */
         memcpy( keyTo->value, keyOn->value, r4->valuelen ) ;
         return 0 ;
      }
   }
}

int r4reindexAdd( R4REINDEX *r4, const S4LONG rec, const unsigned char *keyValue )
{
   B4KEY_DATA *keyTo ;
   R4BLOCK_DATA *startBlock ;
   int rc ;
   #ifdef E4ANALYZE
      S4LONG dif ;
   #endif

   #ifdef E4PARM_LOW
      if ( r4 == 0 || rec < 0 || keyValue == 0 )
         return error4( 0, e4parm, E92102 ) ;
   #endif

   startBlock = r4->startBlock ;
   if ( startBlock->nKeys >= r4->keysmax )
   {
      rc = r4reindexToDisk( r4 ) ;
      if ( rc < 0 )
         return error4stack( r4->codeBase, rc, E92102 ) ;
      memset( (void *)startBlock, 0, r4->blocklen ) ;
   }

   keyTo = (B4KEY_DATA *)( startBlock->info + (startBlock->nKeys++) * r4->grouplen ) ;

   #ifdef E4ANALYZE
      dif = (char *) keyTo -  (char *) startBlock ;
      if ( ((unsigned)dif + r4->grouplen) > r4->blocklen || dif < 0 )
         return error4( r4->codeBase, e4index, E92102 ) ;
   #endif
   keyTo->num = rec ;
   memcpy( (void *)keyTo->value, keyValue, r4->valuelen ) ;

   return 0 ;
}

int r4reindexFinish( R4REINDEX *r4 )
{
   R4BLOCK_DATA *block ;
   int iBlock, rc ;
   B4KEY_DATA *keyTo ;
   #ifdef E4ANALYZE
      S4LONG dif ;
   #endif

   #ifdef E4PARM_LOW
      if ( r4 == 0 )
         return error4( 0, e4parm_null, E92102 ) ;
   #endif

   #ifdef S4BYTE_SWAP
      char *swap, *swapPtr ;
      int i ;
      S4LONG longVal ;
      short shortVal ;

      swap = (char *)u4allocEr( r4->codeBase, r4->blocklen ) ;
      if ( swap == 0 )
         return error4stack( r4->codeBase, e4memory, E92102 ) ;

      memcpy( (void *)swap, (void *)r4->startBlock, r4->blocklen ) ;
      /* position swapPtr at beginning of B4KEY's */
      swapPtr = swap ;
      swapPtr += 6 + sizeof(short) ;
      /* move through all B4KEY's to swap 'long' */
      for ( i = 0 ; i < (* (short *)swap) ; i++ )
      {
         longVal = x4reverseLong( (void *)swapPtr ) ;
         memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
         swapPtr += r4->grouplen ;
      }
      /* swap the numKeys value */
      shortVal = x4reverseShort( (void *)swap ) ;
      memcpy( swap, (void *)&shortVal, sizeof(short) ) ;

      rc = file4seqWrite( &r4->seqwrite, swap ,r4->blocklen ) ;
      if ( rc < 0 )
      {
         u4free( swap ) ;
         return error4stack( r4->codeBase, rc, E92102 ) ;
      }
   #else
      rc = file4seqWrite( &r4->seqwrite, r4->startBlock, r4->blocklen ) ;
      if ( rc < 0 )
         return error4stack( r4->codeBase, rc, E92102 ) ;
   #endif  /* S4BYTE_SWAP */

   r4->lastblock += r4->lastblockInc ;
   block = r4->startBlock ;

   for( iBlock=1; iBlock < r4->nBlocks; iBlock++ )
   {
      block = (R4BLOCK_DATA *)( (char *)block + r4->blocklen ) ;
      if ( block->nKeys >= 1 )
      {
         keyTo = (B4KEY_DATA *) (block->info + block->nKeys*r4->grouplen) ;
         #ifdef E4ANALYZE
            dif = (char *) keyTo  -  (char *) block ;
            if ( ( (unsigned)dif + sizeof(S4LONG ) ) > r4->blocklen  ||  dif < 0 )
               return error4( r4->codeBase, e4index, E92102 ) ;
         #endif
         keyTo->num = r4->lastblock ;

         #ifdef S4BYTE_SWAP
            memcpy( (void *)swap, (void *)block, r4->blocklen ) ;
            /* position swapPtr at beginning of B4KEY's */
            swapPtr = swap ;
            swapPtr += 6 + sizeof(short) ;
            /* move through all B4KEY's to swap 'long' */
            for ( i = 0 ; i < (*(short *)swap) ; i++ )
            {
               longVal = x4reverseLong( (void *)swapPtr ) ;
               memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
               swapPtr += r4->grouplen ;
            }
            /* this is a branch */
            longVal = x4reverseLong( (void *)swapPtr ) ;
            memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;

            /* swap the numKeys value */
            shortVal = x4reverseShort( (void *)swap ) ;
            memcpy( swap, (void *) &shortVal, sizeof(short) ) ;

            rc = file4seqWrite( &r4->seqwrite, swap, r4->blocklen ) ;
            if ( rc < 0 )
            {
               u4free( swap ) ;
               return error4stack( r4->codeBase, rc, E92102 ) ;
            }
         #else
            rc = file4seqWrite( &r4->seqwrite, block, r4->blocklen ) ;
            if ( rc < 0 )
               return error4stack( r4->codeBase, rc, E92102 ) ;
         #endif  /* S4BYTE_SWAP */

         r4->lastblock += r4->lastblockInc ;
      }
   }
   #ifdef S4BYTE_SWAP
      u4free( swap ) ;
   #endif  /* S4BYTE_SWAP */
   return 0 ;
}
#endif  /* ifdef S4MDX */

#ifdef S4FOX
/* Writes out the current block and adds references to higher blocks */
static int r4reindexToDisk( R4REINDEX *r4, const char *keyValue )
{
   R4BLOCK_DATA *block ;
   S4LONG lRecno, revLb ;
   int tnUsed, rc ;
   char *keyTo ;
   #ifdef S4DATA_ALIGN
      S4LONG longTemp ;
   #endif
   #ifdef S4BYTE_SWAP
      char swap[B4BLOCK_SIZE] ;
      char *swapPtr ;
      int i ;
      S4LONG longVal ;
      short shortVal ;
   #endif
   #ifdef E4ANALYZE
      int iBlock ;
      iBlock = 0 ;
   #endif

   #ifdef E4PARM_LOW
      if ( r4 == 0 )
         return error4( 0, e4parm_null, E92102 ) ;
   #endif

   block = r4->startBlock ;
   tnUsed = 1 ;

   memcpy( (void *)&lRecno, ((unsigned char *) (&block->header)) + sizeof(B4STD_HEADER)
           + sizeof(B4NODE_HEADER) + (block->header.nKeys - 1) * r4->nodeHdr.infoLen, sizeof(S4LONG ) ) ;
   #ifdef S4DO_BYTEORDER
      lRecno = x4reverseLong( (void *)&lRecno ) ;
   #endif
   #ifdef S4DATA_ALIGN
      memcpy( (void *)&longTemp, (void *)&r4->nodeHdr.recNumMask[0], sizeof(S4LONG) ) ;
      lRecno &= longTemp ;
   #else
      lRecno &= *(S4LONG *)&r4->nodeHdr.recNumMask[0] ;
   #endif
   lRecno = x4reverseLong( (void *)&lRecno ) ;

   for(;;)
   {
      tnUsed++ ;
      r4->lastblock += B4BLOCK_SIZE ;
      /* next line only works when on leaf branches... */
      block->header.rightNode = r4->lastblock + B4BLOCK_SIZE ;
      if ( block->header.nodeAttribute >= 2 )  /* if leaf, record freeSpace */
         memcpy( ((char *)block) + sizeof( B4STD_HEADER ), (void *)&r4->nodeHdr.freeSpace, sizeof( r4->nodeHdr.freeSpace ) ) ;

      #ifdef S4BYTE_SWAP
         memcpy( (void *)swap, (void *)block, B4BLOCK_SIZE ) ;

         /* position at either B4NODE_HEADER (leaf) or data (branch) */
         swapPtr = swap + 2 * sizeof( short) + 2 * sizeof(S4LONG) ;

         /* if block is a leaf */
         if (r4->startBlock->header.nodeAttribute >= 2 )
         {
            /* swap B4NODE_HEADER members */
            shortVal = x4reverseShort( (void *)swapPtr ) ; /* freeSpace */
            memcpy( swapPtr, (void *) &shortVal, sizeof(short) ) ;
            swapPtr += sizeof(short) ;

            longVal = x4reverseLong( (void *)swapPtr ) ;   /* recNumMask */
            memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
         }
         else /* if block is a branch */
         {
            shortVal = r4->tag->header.keyLen + sizeof(S4LONG) ;

            /* position swapPtr to end of first key expression */
            swapPtr += r4->tag->header.keyLen ;

            /* move through all B4KEY's to swap 'long's */
            for ( i = 0 ; i < (int) block->header.nKeys ; i++ )
            {
               longVal = x4reverseLong( (void *)swapPtr ) ;
               memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
               swapPtr += sizeof(S4LONG) ;
               longVal = x4reverseLong( (void *)swapPtr ) ;
               memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
               swapPtr += shortVal ;
            }
         }

         /* reposition to B4STD_HEADER and swap members */
         swapPtr = swap ;

         shortVal = x4reverseShort( (void *)swapPtr ) ; /* nodeAttribute */
         memcpy( swapPtr, (void *) &shortVal, sizeof(short) ) ;
         swapPtr += sizeof(short) ;

         shortVal = x4reverseShort( (void *)swapPtr ) ; /* nKeys */
         memcpy( swapPtr, (void *) &shortVal, sizeof(short) ) ;
         swapPtr += sizeof(short) ;

         longVal = x4reverseLong( (void *)swapPtr ) ;   /* leftNode */
         memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
         swapPtr += sizeof(S4LONG) ;

         longVal = x4reverseLong( (void *)swapPtr ) ;   /* rightNode */
         memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
         swapPtr += sizeof(S4LONG) ;

         rc = file4seqWrite( &r4->seqwrite, swap, B4BLOCK_SIZE ) ;
      #else
         rc = file4seqWrite( &r4->seqwrite, block, B4BLOCK_SIZE ) ;
      #endif
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;

      memset( (void *)block, 0, B4BLOCK_SIZE ) ;
      block->header.leftNode = r4->lastblock ;
      block->header.rightNode = -1 ;

      block = (R4BLOCK_DATA *) ((char *)block + B4BLOCK_SIZE) ;
      #ifdef E4ANALYZE
         iBlock++ ;
         if ( iBlock >= r4->nBlocks )
            return error4( r4->codeBase, e4info, E92102 ) ;
      #endif  /* E4ANALYZE */

      if ( block->header.nKeys < r4->keysmax )
      {
         keyTo = ((char *) (&block->header)) + sizeof(B4STD_HEADER) + block->header.nKeys * r4->grouplen ;
         block->header.nKeys++ ;
         #ifdef E4ANALYZE
            if ( (char *) keyTo -  (char *) block + r4->grouplen > B4BLOCK_SIZE || (char *) keyTo -  (char *) block < 0 )
               return error4( r4->codeBase, e4result, E92102 ) ;
         #endif  /* E4ANALYZE */
         memcpy( keyTo, (void *)keyValue, (unsigned int)r4->valuelen ) ;
         keyTo += r4->valuelen ;
         memcpy( keyTo, (void *)&lRecno, sizeof(S4LONG ) ) ;
         revLb = x4reverseLong( (void *)&r4->lastblock ) ;
         memcpy( keyTo + sizeof(S4LONG ), (void *)&revLb, sizeof(S4LONG ) ) ;

         if ( block->header.nKeys < r4->keysmax )  /* then done, else do next one up */
         {
            if ( tnUsed > r4->nBlocksUsed )
               r4->nBlocksUsed = tnUsed ;
            return 0 ;
         }
      }
      #ifdef E4ANALYZE
         else  /* should never occur */
            return error4( r4->codeBase, e4result, E92102 ) ;
      #endif  /* E4ANALYZE */
   }
}

int r4reindexTagHeadersWrite( R4REINDEX *r4 )
{
   TAG4FILE *tagOn ;
   INDEX4FILE *i4file ;
   int rc, totLen, iTag ;
   unsigned int exprHdrLen ;
   const char *ptr ;
   FILE4LONG pos ;
   #ifdef S4BYTE_SWAP
      T4HEADER swapTagHeader ;
   #endif

   #ifdef E4PARM_LOW
      if ( r4 == 0 )
         return error4( 0, e4parm_null, E92102 ) ;
   #endif

   iTag = 2 ;
   i4file = r4->indexFile ;

   /* Now write the headers  */
   file4longAssign( pos, 0, 0 ) ;
   file4seqWriteInitLow( &r4->seqwrite, &i4file->file, pos, r4->buffer, r4->bufferLen ) ;

   i4file->tagIndex->header.freeList = 0L ;
   exprHdrLen = 5 * sizeof(short) ;
   i4file->eof = r4->lastblock + B4BLOCK_SIZE ;

   if ( i4file->tagIndex->header.typeCode >= 64 )
   {
      #ifdef S4BYTE_SWAP
         memcpy( (void *)&swapTagHeader, (void *)&i4file->tagIndex->header, sizeof(T4HEADER) ) ;

         swapTagHeader.root = x4reverseLong( (void *)&swapTagHeader.root ) ;
         swapTagHeader.freeList = x4reverseLong( (void *)&swapTagHeader.freeList ) ;
         /* version is written in non-intel order */
         swapTagHeader.keyLen = x4reverseShort( (void *)&swapTagHeader.keyLen ) ;
         swapTagHeader.descending = x4reverseShort( (void *)&swapTagHeader.descending ) ;
         swapTagHeader.filterPos = x4reverseShort( (void *)&swapTagHeader.filterPos ) ;
         swapTagHeader.filterLen = x4reverseShort( (void *)&swapTagHeader.filterLen ) ;
         swapTagHeader.exprPos = x4reverseShort( (void *)&swapTagHeader.exprPos ) ;
         swapTagHeader.exprLen = x4reverseShort( (void *)&swapTagHeader.exprLen ) ;

         rc = file4seqWrite( &r4->seqwrite, &swapTagHeader, LEN4HEADER_WR ) ;  /* write first header part */
      #else
         i4file->tagIndex->header.version = x4reverseLong( (void *)&i4file->tagIndex->header.version ) ;
         rc = file4seqWrite( &r4->seqwrite, &i4file->tagIndex->header, LEN4HEADER_WR ) ;  /* write first header part */
         i4file->tagIndex->header.version = x4reverseLong( (void *)&i4file->tagIndex->header.version ) ;
      #endif
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;

      rc = file4seqWriteRepeat( &r4->seqwrite, 478L, 0 ) ;
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;
      rc = file4seqWrite( &r4->seqwrite, &i4file->tagIndex->header.sortSeq, 8 ) ;
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;

      #ifdef S4BYTE_SWAP
         rc = file4seqWrite( &r4->seqwrite, &swapTagHeader.descending, exprHdrLen ) ;
      #else
         rc = file4seqWrite( &r4->seqwrite, &i4file->tagIndex->header.descending, exprHdrLen ) ;
      #endif
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;

      rc = file4seqWriteRepeat( &r4->seqwrite, 512L, 0 ) ;  /* no expression */
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;
   }

   for ( tagOn = 0 ;; )
   {
      tagOn = (TAG4FILE *)l4next( &i4file->tags, tagOn ) ;
      if ( tagOn == 0 )
         break ;
      if ( i4file->tagIndex->header.typeCode >= 64 )
         tagOn->headerOffset = ((S4LONG)iTag) * B4BLOCK_SIZE ;
      else
         tagOn->headerOffset = 0L ;

      #ifdef S4BYTE_SWAP
         memcpy( (void *)&swapTagHeader, (void *)&tagOn->header, sizeof(T4HEADER) ) ;

         swapTagHeader.root = x4reverseLong( (void *)&swapTagHeader.root ) ;
         swapTagHeader.freeList = x4reverseLong( (void *)&swapTagHeader.freeList ) ;
         /* version is in non-intel ordering */
         swapTagHeader.keyLen = x4reverseShort( (void *)&swapTagHeader.keyLen ) ;
         swapTagHeader.descending = x4reverseShort( (void *)&swapTagHeader.descending ) ;
         swapTagHeader.filterPos = x4reverseShort( (void *)&swapTagHeader.filterPos ) ;
         swapTagHeader.filterLen = x4reverseShort( (void *)&swapTagHeader.filterLen ) ;
         swapTagHeader.exprPos = x4reverseShort( (void *)&swapTagHeader.exprPos ) ;
         swapTagHeader.exprLen = x4reverseShort( (void *)&swapTagHeader.exprLen ) ;

         rc = file4seqWrite( &r4->seqwrite, &swapTagHeader, LEN4HEADER_WR ) ;
      #else
         tagOn->header.version = x4reverseLong( (void *)&tagOn->header.version ) ;
         rc = file4seqWrite( &r4->seqwrite, &tagOn->header, LEN4HEADER_WR ) ;
         tagOn->header.version = x4reverseLong( (void *)&tagOn->header.version ) ;
      #endif
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;

      rc = file4seqWriteRepeat( &r4->seqwrite, 478L, 0 ) ;
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;
      rc = file4seqWrite( &r4->seqwrite, &tagOn->header.sortSeq, 8 ) ;
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;

      #ifdef S4BYTE_SWAP
         rc = file4seqWrite( &r4->seqwrite, &swapTagHeader.descending, exprHdrLen ) ;
      #else
         rc = file4seqWrite( &r4->seqwrite, &tagOn->header.descending, exprHdrLen ) ;
      #endif
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;

      ptr = tagOn->expr->source ;
      totLen = tagOn->header.exprLen ;
      rc = file4seqWrite( &r4->seqwrite, ptr, (unsigned int)tagOn->header.exprLen ) ;
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;

      if ( tagOn->filter != 0 )
      {
         ptr = tagOn->filter->source ;
         file4seqWrite( &r4->seqwrite, ptr, (unsigned int)tagOn->header.filterLen ) ;
         totLen += tagOn->header.filterLen ;
      }
      rc = file4seqWriteRepeat( &r4->seqwrite, (S4LONG)B4BLOCK_SIZE - totLen, 0 );
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;
      iTag += 2 ;
   }
   file4longAssign( pos, i4file->eof, 0 ) ;
   rc = file4lenSetLow( &i4file->file, pos ) ;
   if ( rc < 0 )
      return error4stack( r4->codeBase, (short)rc, E92102 ) ;

   rc = file4seqWriteFlush( &r4->seqwrite ) ;
   if ( rc < 0 )
      return error4stack( r4->codeBase, rc, E92102 ) ;
   return 0 ;

}

int r4reindexWriteKeys( R4REINDEX *r4, short int errUnique )
{
   char lastKey[I4MAX_KEY_SIZE] ;
   unsigned char *keyData ;
   int isUnique, rc, cLen, tLen, lastTrail ;
   int onCount, isFirst ;
   unsigned short int kLen ;
   void *dummyPtr ;
   S4LONG keyRec, recCount ;
   TAG4FILE *t4 ;
   unsigned S4LONG ff ;
   unsigned S4LONG rLen ;
   R4BLOCK_DATA *r4block ;
   FILE4LONG pos ;

   #ifdef E4PARM_LOW
      if ( r4 == 0 )
         return error4( 0, e4parm_null, E92102 ) ;
   #endif

   t4 = r4->tag ;
   kLen = (unsigned short int)t4->header.keyLen ;
   ff = 0xFFFFFFFFL ;
   isFirst = 1 ;

   #ifdef E4MISC
      if ( I4MAX_KEY_SIZE_COMPATIBLE < r4->sort.sortLen )
      {
         if ( r4->codeBase->oledbSchemaCreate == 1 )  /* ensure via real max length (not-compatible) */
         {
            if ( I4MAX_KEY_SIZE < r4->sort.sortLen )
               return error4( r4->codeBase, e4index, E82102 ) ;
         }
         else
            return error4( r4->codeBase, e4index, E82102 ) ;
      }
   #endif  /* E4MISC */

   memset( lastKey, r4->tag->pChar, kLen ) ;

   for ( cLen = 0 ; kLen ; kLen >>= 1, cLen++ ) ;
   kLen = (unsigned short int)t4->header.keyLen ;  /* reset the key length */
   r4->nodeHdr.trailCntLen = r4->nodeHdr.dupCntLen = (unsigned char)cLen ;

   r4->nodeHdr.trailByteCnt = (unsigned char)(0xFF >> ( 8 - ((cLen / 8) * 8 + cLen % 8))) ;
   r4->nodeHdr.dupByteCnt = r4->nodeHdr.trailByteCnt ;

   if ( t4 == r4->indexFile->tagIndex ) /* the tag of tags, don't use record count, instead base on 1024*numTags, which is true value */
       recCount = r4->nTags * 1024 ;
   else
   {
      recCount = dfile4recCount( r4->dataFile, -2L ) ;
      if ( recCount < 0 )
         return error4stack( r4->codeBase, (short)recCount, E92102 ) ;
   }
   rLen = (unsigned S4LONG)recCount ;

   for ( cLen = 0 ; rLen ; rLen>>=1, cLen++ ) ;
   r4->nodeHdr.recNumLen = (unsigned char) cLen ;
   if ( r4->nodeHdr.recNumLen < 12 )
      r4->nodeHdr.recNumLen = 12 ;

   for( tLen = r4->nodeHdr.recNumLen + r4->nodeHdr.trailCntLen + r4->nodeHdr.dupCntLen ;
        (tLen / 8)*8 != tLen ; tLen++, r4->nodeHdr.recNumLen++ ) ;  /* make at an 8-bit offset */

   rLen = ff >> ( sizeof(S4LONG)*8 - r4->nodeHdr.recNumLen ) ;
   memcpy( (void *)&r4->nodeHdr.recNumMask[0], (void *)&rLen, sizeof(S4LONG) ) ;

   r4->nodeHdr.infoLen = (unsigned char)((unsigned int)(r4->nodeHdr.recNumLen + r4->nodeHdr.trailCntLen + r4->nodeHdr.dupCntLen) / 8) ;
   r4->valuelen = t4->header.keyLen ;
   r4->grouplen = t4->header.keyLen + 2*sizeof(S4LONG) ;

   memset( (void *)r4->startBlock, 0, (unsigned int)r4->nBlocks * B4BLOCK_SIZE ) ;

   for ( r4block = r4->startBlock, onCount = 0 ; onCount < r4->nBlocks;
         r4block = (R4BLOCK_DATA *) ( (char *)r4block + B4BLOCK_SIZE), onCount++ )
   {
      memset( (void *)r4block, 0, B4BLOCK_SIZE ) ;
      r4block->header.leftNode = -1 ;
      r4block->header.rightNode = -1 ;
   }

   r4->nodeHdr.freeSpace = B4BLOCK_SIZE - sizeof( B4STD_HEADER ) - sizeof( B4NODE_HEADER ) ;

   r4->keysmax = (B4BLOCK_SIZE - sizeof( B4STD_HEADER ) ) / (unsigned)r4->grouplen ;

   #ifdef E4ANALYZE
      if ( r4->nodeHdr.freeSpace <= 0 || r4->keysmax <= 0 )
         return error4( r4->codeBase, e4index, E92102 ) ;
   #endif
   rc = sort4getInit( &r4->sort ) ;
   if ( rc < 0 )
      return error4stack( r4->codeBase, (short)rc, E92102 ) ;

   file4longAssign( pos, r4->lastblock+B4BLOCK_SIZE, 0 ) ;
   file4seqWriteInitLow( &r4->seqwrite, &r4->indexFile->file, pos, r4->buffer, r4->bufferLen ) ;

   lastTrail = kLen ;   /* default is no available duplicates */
   isUnique = t4->header.typeCode & 0x01 ;

   for( ;; )  /* For each key to write */
   {
      rc = sort4get( &r4->sort, &keyRec, (void **) &keyData, &dummyPtr ) ;
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;
      #ifdef E4MISC
         if ( r4->keyCount < 0L || r4->keyCount == 0L && rc != r4done || r4->keyCount > 0L && rc == r4done )
            return error4( r4->codeBase, e4info, E92102 ) ;
         r4->keyCount-- ;
      #endif  /* E4MISC */

      if ( rc == r4done )  /* No more keys */
      {
         rc = r4reindexFinish( r4, lastKey ) ;
         if ( rc < 0 )
            return error4stack( r4->codeBase, (short)rc, E92102 ) ;
         rc = file4seqWriteFlush( &r4->seqwrite ) ;
         if ( rc < 0 )
            return error4stack( r4->codeBase, (short)rc, E92102 ) ;
         break ;
      }

      if ( isUnique )
      {
         if( isFirst )
            isFirst = 0 ;
         else
            if ( u4memcmp( keyData, lastKey, r4->sort.sortLen) == 0 )
            {
               switch( errUnique )
               {
                  case e4unique:
                  case e4candidate:
                     return error4describe( r4->codeBase, e4unique, E82103, t4->alias, (char *)0, (char *)0 ) ;
                  case r4unique:
                  case r4candidate:
                     return r4unique ;
                  default:
                     continue ;
               }
            }
      }

      /* Add the key */
      rc = r4reindexAdd( r4, keyRec, keyData, lastKey, &lastTrail ) ;
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;
      memcpy( lastKey, keyData, r4->sort.sortLen ) ;
   }

   /* Now complete the tag header info. */
   t4->header.root = r4->lastblock ;
   return 0 ;
}

/* for compact leaf nodes only */
int r4reindexAdd( R4REINDEX *r4, const S4LONG rec, const unsigned char *keyValue, const char *lastKey, int *lastTrail )
{
   R4BLOCK_DATA *startBlock ;
   int rc, dupCnt, trail, kLen, iLen, len ;
   unsigned char buffer[6] ;
   char *infoPos ;

   #ifdef E4PARM_LOW
      if ( r4 == 0 || rec < 0 || keyValue == 0 )
         return error4( 0, e4parm, E92102 ) ;
   #endif

   startBlock = r4->startBlock ;
   kLen = r4->valuelen ;
   iLen = r4->nodeHdr.infoLen ;

   if ( startBlock->header.nKeys == 0 )   /* reset */
   {
      dupCnt = 0 ;
      r4->curPos = ((char *)startBlock) + B4BLOCK_SIZE ;
      memcpy( ((char *)startBlock) + sizeof( B4STD_HEADER ), (void *)&r4->nodeHdr, sizeof( B4NODE_HEADER ) ) ;
      startBlock->header.nodeAttribute |= 2 ;   /* leaf block */
      *lastTrail = kLen ;
   }
   else
      dupCnt = b4calcDups( keyValue,(const unsigned char*)lastKey, kLen ) ;

   if ( dupCnt > kLen - *lastTrail )  /* don't allow duplicating trail bytes */
      dupCnt = kLen - *lastTrail ;

   if ( dupCnt == kLen ) /* duplicate key */
      trail = 0 ;
   else
   {
      if ( d4version( r4->data ) != 0x30 && r4->codeBase->compatibility == 26 && r4->tag->filter != 0 )
         trail = 0 ;
      else
         trail = b4calcBlanks( keyValue, kLen, r4->tag->pChar ) ;
   }

   *lastTrail = trail ;

   if ( dupCnt > kLen - *lastTrail )  /* watch for case where < ' ' exissts */
      dupCnt = kLen - *lastTrail ;

   len = kLen - dupCnt - trail ;
   if ( r4->nodeHdr.freeSpace < iLen + len )
   {
      rc = r4reindexToDisk(r4, lastKey) ;
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;
      r4->nodeHdr.freeSpace = B4BLOCK_SIZE - sizeof( B4STD_HEADER ) - sizeof( B4NODE_HEADER ) ;
      dupCnt = 0 ;
      r4->curPos = ((char *)startBlock) + B4BLOCK_SIZE ;
      memcpy( ((char *)&startBlock->header) + sizeof( B4STD_HEADER ), (void *)&r4->nodeHdr, sizeof( B4NODE_HEADER ) ) ;
      startBlock->header.nodeAttribute |= 2 ;   /* leaf block */
      if ( d4version( r4->data ) != 0x30 && r4->codeBase->compatibility == 26 && r4->tag->filter != 0 )
         trail = 0 ;
      else
         trail = b4calcBlanks( keyValue, kLen, r4->tag->pChar ) ;
      len = kLen - trail ;
   }

   r4->curPos -= len ;
   memcpy( r4->curPos, keyValue + dupCnt, (unsigned int)len ) ;
   infoPos = ((char *)&startBlock->header) + sizeof(B4STD_HEADER) + sizeof(B4NODE_HEADER) + startBlock->header.nKeys * iLen ;
   x4putInfo( &r4->nodeHdr, buffer, rec, trail, dupCnt ) ;
   memcpy( infoPos, (void *)buffer, (unsigned int)iLen ) ;

   r4->nodeHdr.freeSpace -= (unsigned char) ( len + iLen ) ;
   startBlock->header.nKeys++ ;
   return 0 ;
}

int r4reindexFinish( R4REINDEX *r4, char *keyValue )
{
   R4BLOCK_DATA *block ;
   int rc, iBlock ;
   S4LONG revLb, lRecno ;
   char *keyTo ;
   #ifdef S4DATA_ALIGN
      S4LONG longTemp ;
   #endif
   #ifdef S4BYTE_SWAP
      char swap[B4BLOCK_SIZE] ;
      char *swapPtr ;
      int i ;
      S4LONG longVal ;
      short shortVal ;
   #endif

   #ifdef E4PARM_LOW
      if ( r4 == 0 )
         return error4( 0, e4parm_null, E92102 ) ;
   #endif

   block = r4->startBlock ;

   if ( r4->nBlocksUsed <= 1 ) /* just output first block */
   {
      memcpy( ((char *)block) + sizeof( B4STD_HEADER ), (void *)&r4->nodeHdr, sizeof( B4NODE_HEADER ) ) ;

      block->header.nodeAttribute |= (short)3 ;   /* leaf and root block */

      #ifdef S4BYTE_SWAP
         memcpy( (void *)swap, (void *)block, B4BLOCK_SIZE ) ;

         /* position at either B4NODE_HEADER (leaf) or data (branch) */
         swapPtr = swap + 2 * sizeof( short) + 2 * sizeof(S4LONG) ;

         if (block->header.nodeAttribute >= 2 ) /* if block is a leaf */
         {
            /* swap B4NODE_HEADER members */
            shortVal = x4reverseShort( (void *)swapPtr ) ; /* freeSpace */
            memcpy( swapPtr, (void *) &shortVal, sizeof(short) ) ;
            swapPtr += sizeof(short) ;

            longVal = x4reverseLong( (void *)swapPtr ) ;   /* recNumMask */
            memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
         }
         else /* if block is a branch */
         {
            shortVal = r4->tag->header.keyLen + sizeof(S4LONG) ;

            /* position swapPtr to end of first key expression */
            swapPtr += r4->tag->header.keyLen ;

            /* move through all B4KEY's to swap 'long's */
            for ( i = 0 ; i < (int)block->header.nKeys ; i++ )
            {
               longVal = x4reverseLong( (void *)swapPtr ) ;
               memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
               swapPtr += sizeof(S4LONG) ;
               longVal = x4reverseLong( (void *)swapPtr ) ;
               memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
               swapPtr += shortVal ;
            }
         }

         /* reposition to B4STD_HEADER and swap members */
         swapPtr = swap ;

         shortVal = x4reverseShort( (void *)swapPtr ) ; /* nodeAttribute */
         memcpy( swapPtr, (void *) &shortVal, sizeof(short) ) ;
         swapPtr += sizeof(short) ;

         shortVal = x4reverseShort( (void *)swapPtr ) ; /* nKeys */
         memcpy( swapPtr, (void *) &shortVal, sizeof(short) ) ;
         swapPtr += sizeof(short) ;

         longVal = x4reverseLong( (void *)swapPtr ) ;   /* leftNode */
         memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
         swapPtr += sizeof(S4LONG) ;

         longVal = x4reverseLong( (void *)swapPtr ) ;   /* rightNode */
         memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
         swapPtr += sizeof(S4LONG) ;

         rc = file4seqWrite( &r4->seqwrite, swap,B4BLOCK_SIZE ) ;
      #else
         rc = file4seqWrite( &r4->seqwrite, block,B4BLOCK_SIZE ) ;
      #endif
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;
      r4->lastblock += B4BLOCK_SIZE ;
   }
   else
   {
      memcpy( (void *)&lRecno, (void *) (((char *)(&block->header)) + sizeof(B4STD_HEADER)
              + sizeof(B4NODE_HEADER) + (block->header.nKeys - 1) * r4->nodeHdr.infoLen), sizeof(S4LONG ) ) ;
      #ifdef S4DO_BYTEORDER
         lRecno = x4reverseLong( (void *)&lRecno ) ;
      #endif
      #ifdef S4DATA_ALIGN
         memcpy( (void *)&longTemp, (void *)&r4->nodeHdr.recNumMask[0], sizeof(S4LONG) ) ;
         lRecno &= longTemp ;
      #else
         lRecno &= *(S4LONG *)&r4->nodeHdr.recNumMask[0] ;
      #endif

      if ( block->header.nodeAttribute >= 2 )  /* if leaf, record freeSpace */
         memcpy( ((char *)block) + sizeof( B4STD_HEADER ), (void *)&r4->nodeHdr, sizeof( B4NODE_HEADER ) ) ;

      #ifdef S4BYTE_SWAP
         memcpy( (void *)swap, (void *)r4->startBlock, B4BLOCK_SIZE ) ;

         /* position at either B4NODE_HEADER (leaf) or data (branch) */
         swapPtr = swap + 2 * sizeof( short) + 2 * sizeof(S4LONG) ;

         if (block->header.nodeAttribute >= 2 ) /* if block is a leaf */
         {
            /* swap B4NODE_HEADER members */
            shortVal = x4reverseShort( (void *)swapPtr ) ; /* freeSpace */
            memcpy( swapPtr, (void *) &shortVal, sizeof(short) ) ;
            swapPtr += sizeof(short) ;

            longVal = x4reverseLong( (void *)swapPtr ) ;   /* recNumMask */
            memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
         }
         else /* if block is a branch */
         {
            shortVal = r4->tag->header.keyLen + sizeof(S4LONG) ;

            /* position swapPtr to end of first key expression */
            swapPtr += r4->tag->header.keyLen ;

            /* move through all B4KEY's to swap 'long's */
            for ( i = 0 ; i < (int) block->header.nKeys ; i++ )
            {
               longVal = x4reverseLong( (void *)swapPtr ) ;
               memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
               swapPtr += sizeof(S4LONG) ;
               longVal = x4reverseLong( (void *)swapPtr ) ;
               memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
               swapPtr += shortVal ;
            }
         }

         /* reposition to B4STD_HEADER and swap members */
         swapPtr = swap ;

         shortVal = x4reverseShort( (void *)swapPtr ) ; /* nodeAttribute */
         memcpy( swapPtr, (void *) &shortVal, sizeof(short) ) ;
         swapPtr += sizeof(short) ;

         shortVal = x4reverseShort( (void *)swapPtr ) ; /* nKeys */
         memcpy( swapPtr, (void *) &shortVal, sizeof(short) ) ;
         swapPtr += sizeof(short) ;

         longVal = x4reverseLong( (void *)swapPtr ) ;   /* leftNode */
         memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
         swapPtr += sizeof(S4LONG) ;

         longVal = x4reverseLong( (void *)swapPtr ) ;   /* rightNode */
         memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
         swapPtr += sizeof(S4LONG) ;

         rc = file4seqWrite( &r4->seqwrite, swap,B4BLOCK_SIZE ) ;
      #else
         rc = file4seqWrite(&r4->seqwrite, r4->startBlock,B4BLOCK_SIZE ) ;
      #endif
      if ( rc < 0 )
         return error4stack( r4->codeBase, (short)rc, E92102 ) ;

      r4->lastblock += B4BLOCK_SIZE ;

      lRecno = x4reverseLong( (void *)&lRecno ) ;

      for( iBlock=1; iBlock < r4->nBlocksUsed ; iBlock++ )
      {
         block = (R4BLOCK_DATA *) ((char *)block + B4BLOCK_SIZE) ;
         keyTo = ((char *) (&block->header)) + sizeof(B4STD_HEADER) + block->header.nKeys * r4->grouplen ;
         block->header.nKeys++ ;
         #ifdef E4MISC
            if ( (char *) keyTo -  (char *) block + r4->grouplen > B4BLOCK_SIZE ||
                 (char *) keyTo -  (char *) block < 0 )
               return error4( r4->codeBase, e4result, E92102 ) ;
         #endif  /* E4MISC */
         memcpy( keyTo, (void *)keyValue, (unsigned int)r4->valuelen ) ;
         keyTo += r4->valuelen ;
         memcpy( keyTo, (void *)&lRecno, sizeof(S4LONG ) ) ;
         revLb = x4reverseLong( (void *)&r4->lastblock ) ;
         memcpy( keyTo + sizeof(S4LONG ), (void *)&revLb, sizeof(S4LONG ) ) ;

         if ( iBlock == r4->nBlocksUsed - 1 )
            block->header.nodeAttribute = 1 ;  /* root block */

         #ifdef S4BYTE_SWAP
            memcpy( (void *)swap, (void *)block, B4BLOCK_SIZE ) ;

            /* position at either B4NODE_HEADER (leaf) or data (branch) */
            swapPtr = swap + 2 * sizeof( short) + 2 * sizeof(S4LONG) ;

            if (block->header.nodeAttribute >= 2 ) /* if block is a leaf */
            {
               /* swap B4NODE_HEADER members */
               shortVal = x4reverseShort( (void *)swapPtr ) ; /* freeSpace */
               memcpy( swapPtr, (void *) &shortVal, sizeof(short) ) ;
               swapPtr += sizeof(short) ;

               longVal = x4reverseLong( (void *)swapPtr ) ;   /* recNumMask */
               memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
            }
            else /* if block is a branch */
            {
               shortVal = r4->tag->header.keyLen + sizeof(S4LONG) ;

               /* position swapPtr to end of first key expression */
               swapPtr += r4->tag->header.keyLen ;

               /* move through all B4KEY's to swap 'long's */
               for ( i = 0 ; i < (int) block->header.nKeys ; i++ )
               {
                  longVal = x4reverseLong( (void *)swapPtr ) ;
                  memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
                  swapPtr += sizeof(S4LONG) ;
                  longVal = x4reverseLong( (void *)swapPtr ) ;
                  memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
                  swapPtr += shortVal ;
               }
            }

            /* reposition to B4STD_HEADER and swap members */
            swapPtr = swap ;

            shortVal = x4reverseShort( (void *)swapPtr ) ; /* nodeAttribute */
            memcpy( swapPtr, (void *) &shortVal, sizeof(short) ) ;
            swapPtr += sizeof(short) ;

            shortVal = x4reverseShort( (void *)swapPtr ) ; /* nKeys */
            memcpy( swapPtr, (void *) &shortVal, sizeof(short) ) ;
            swapPtr += sizeof(short) ;

            longVal = x4reverseLong( (void *)swapPtr ) ;   /* leftNode */
            memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
            swapPtr += sizeof(S4LONG) ;

            longVal = x4reverseLong( (void *)swapPtr ) ;   /* rightNode */
            memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
            swapPtr += sizeof(S4LONG) ;

            rc = file4seqWrite( &r4->seqwrite, swap, B4BLOCK_SIZE ) ;
         #else
            rc = file4seqWrite( &r4->seqwrite, block, B4BLOCK_SIZE )  ;
         #endif
         if ( rc < 0 )
            return error4stack( r4->codeBase, (short)rc, E92102 ) ;

         r4->lastblock += B4BLOCK_SIZE ;
      }
   }

   return 0 ;
}

#endif  /* S4FOX */
#endif  /* N4OTHER  */
#endif  /* S4INDEX_OFF */
#endif  /* S4WRITE_OFF */
