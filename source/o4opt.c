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

/* o4opt.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"


#ifndef S4OPTIMIZE_OFF

static int opt4blockAdd( OPT4BLOCK *, FILE4 *, unsigned, long, long ) ;

static OPT4BLOCK *opt4fileChooseBlock( FILE4 * ) ;
/* puts block onto bottom of lru-list and assigns the optlist to the block */
#define opt4listLruBottomPlace( o4, b4 ) ( l4add( &((o4)->list), &((b4)->lruLink) ), ( (b4)->optList = (o4) ) )
static void opt4listLruBottomShift( OPT4BLOCK * ) ;
static void opt4fileReadSpBuffer( FILE4 *, const unsigned long, int, int ) ;
static void opt4timeReset( OPT4 *, int, int ) ;
#ifdef S4ADVANCE_TEST
   PUBLIC unsigned S4FUNCTION opt4fileReadFile( FILE4 S4PTR *, const unsigned long, char S4PTR * ) ;
#else
   static unsigned opt4fileReadFile( FILE4 *, const unsigned long, char * ) ;
#endif

/* moves the block from the current position to the lru position (l4last)
   of the lru-list it is currently on.  Also updates the accessCount */
static void opt4listLruBottomShift( OPT4BLOCK *block )
{
   OPT4 *opt ;
   LINK4 *link ;
   LIST4 *list ;

   opt = &block->file->codeBase->opt ;

   list = &block->optList->list ;
   link = (LINK4 *)&block->lruLink ;
   if ( link != list->lastNode )
   {
      if ( list->selected == link )
         list->selected = (LINK4 *)l4prev( list, link ) ;
      l4remove( list, link ) ;
      l4add( list, link ) ;
   }

   if ( (unsigned int)(opt->accessTimeCount - block->accessTime) > opt->minAccessTimeVariation )
      block->accessTime = opt->accessTimeCount++ ;
   if ( opt->accessTimeCount == 0 )  /* time count got reset, so reset for everyone */
      opt4timeReset( opt, 0, 1 ) ;

   block->hitCount += block->file->hitCountAdd ;
}

static int opt4blockAdd( OPT4BLOCK *block, FILE4 *file, unsigned blockLen, long hashVal, long position )
{
   #ifdef E4PARM_LOW
      if ( file == 0 || block == 0 || hashVal < 0 || position < 0 )
         return error4( 0, e4parm, E92508 ) ;
      if ( (unsigned long)hashVal >= file->codeBase->opt.numLists )
         return error4( 0, e4parm, E92508 ) ;
   #endif
   #ifdef E4ANALYZE
      if ( block->changed != 0 )
         return error4( file->codeBase, e4struct, E92508 ) ;
   #endif

   l4add( &file->codeBase->opt.lists[hashVal], block ) ;
   block->len = blockLen ;
   block->pos = position ;
   block->file = file ;

   #ifdef E4ANALYZE_ALL
      /* AS 02/24/97 - it is possible for this to be ok, eg. writing tags to disk, later
         tags blocks written before earlier ones.
      if ( file->len != -1 )  ensure we are not beyond file length - impossible...
         if ( (long)(block->pos + block->len) > file->len )
            error4( file->codeBase, e4opt, E92508 ) ;
      */
   #endif

   return 0 ;
}

int opt4blockClear( OPT4BLOCK *block )
{
   #ifdef E4PARM_LOW
      if ( block == 0 )
         return error4( 0, e4parm_null, E92508 ) ;
   #endif

   block->changed = 0 ;
   block->len = 0 ;
   block->pos = 0 ;
   block->file = 0 ;

   return 0 ;
}

#ifdef S4WRITE_DELAY
void S4CALL opt4writeCompletionRoutine( void *inDelay )
{
   FILE4WRITE_DELAY *delay ;

   delay = (FILE4WRITE_DELAY *)inDelay ;
   EnterCriticalSection( &delay->file->codeBase->opt.critical4optWrite ) ;
   l4add( &delay->file->codeBase->opt.delayAvail, (LINK4 *)delay->completionData ) ;
   LeaveCriticalSection( &delay->file->codeBase->opt.critical4optWrite ) ;
}
#endif

#ifndef _MSC_VER
   #ifdef P4ARGS_USED
   #pragma argsused
#endif
#endif
static int opt4blockFlush( OPT4 *opt, OPT4BLOCK *block, char buffer, int doDelay )
{
   int rc, flushBuffer ;
   #ifdef S4WRITE_DELAY
      OPT4BLOCK *delayBlock ;
      LINK4 *delayLink ;
   #endif
   #ifdef __SC__   /* compiler bug - cpp */
      unsigned long long_val ;
   #endif

   #ifdef E4PARM_LOW
      if ( block == 0 )
         return error4( 0, e4parm_null, E92508 ) ;
   #endif

   #ifdef E4ANALYZE_ALL
      if ( block->file->hasDup == 1 )
         if ( file4cmpPart( block->file->codeBase, block->data, block->file, block->pos, block->len ) != 0 )
            return error4( block->file->codeBase, e4opt, E82503 ) ;
   #endif

   if ( opt->readFile == block->file )  /* check to see if block has been temporarily read-buffered */
      if ( ( (unsigned long)block->pos < opt->readStartPos + opt->bufferSize ) && ( (unsigned long)block->pos >= opt->readStartPos ) )
         memcpy( opt->readBuffer + block->pos - opt->readStartPos, block->data, block->len ) ;

   if ( buffer && block->file->type == OPT4DBF )
   {
      if ( opt->writeFile != block->file )
         flushBuffer = 1 ;
      else
      {
         if ( opt->writeBlockCount == opt->maxBlocks )
            flushBuffer = 1 ;
         else
         {
            if ( opt->writeCurPos != (unsigned long)block->pos )   /* not a consecutive write, so check out possibilities */
            {
               if ( opt->writeCurPos < (unsigned long)block->pos )
               {
                  if ( (unsigned long)block->pos - opt->writeCurPos < opt->blockSize )   /* partially filled block, can just extend */
                  {
                     opt->writeCurPos = (unsigned long)block->pos ;
                     flushBuffer = 0 ;
                  }
                  else
                     flushBuffer = 1 ;
               }
               else
                  flushBuffer = 1 ;
            }
            else   /* just want to write at current spot, so go ahead */
               flushBuffer = 0 ;
         }

      }

      #ifdef E4ANALYZE
         if ( flushBuffer < 0 || flushBuffer > 1 )
            return error4( block->file->codeBase, e4info, E92508 ) ;
      #endif

      if ( flushBuffer == 1 )
      {
         if ( ( rc = opt4flushWriteBuffer( opt ) ) != 0 )
            return rc ;
         opt->writeFile = block->file ;
         opt->writeStartPos = opt->writeCurPos = (unsigned long)block->pos ;
      }
      memcpy( opt->writeBuffer + (opt->writeCurPos - opt->writeStartPos), block->data, block->len ) ;
      opt->writeCurPos += block->len ;
      opt->writeBlockCount++ ;

      #ifdef E4ANALYZE_ALL
         if ( block->file->hasDup == 1 )
            if ( file4cmpPart( opt->writeFile->codeBase, opt->writeBuffer, opt->writeFile, opt->writeStartPos, (unsigned)(opt->writeCurPos - opt->writeStartPos) ) != 0 )
               return error4( block->file->codeBase, e4opt, E82503 ) ;
      #endif
   }
   else
   {
      #ifdef __SC__   /* compiler bug - cpp */
         if ( opt->writeFile == block->file )
         {
            long_val = opt->writeBlockCount ;
            long_val *= opt->blockSize ;

            if ( opt->writeCurPos >= (unsigned long) block->pos &&
                 (opt->writeCurPos - long_val ) <= (unsigned long)block->pos )
               if ( ( rc = opt4flushWriteBuffer( opt ) ) != 0 )
                  return rc ;
         }
      #else
         if ( opt->writeFile == block->file )
            if ( opt->writeCurPos >= (unsigned long) block->pos &&
                 (opt->writeCurPos - opt->writeBlockCount * opt->blockSize ) <= (unsigned long)block->pos )
               if ( ( rc = opt4flushWriteBuffer( opt ) ) != 0 )
                  return rc ;
      #endif
      block->file->doBuffer = 0 ;
      #ifdef S4WRITE_DELAY
         if ( doDelay == 1 && opt->delayWriteBuffer != 0 )
         {
            for ( ;; )  /* wait until a block is available */
            {
               EnterCriticalSection( &opt->critical4optWrite ) ;
               delayLink = (LINK4 *)l4first( &opt->delayAvail ) ;
               if ( delayLink == 0 )
               {
                  LeaveCriticalSection( &opt->critical4optWrite ) ;
                  Sleep( 0 ) ;
               }
               else
                  break ;
            }

            delayBlock = (OPT4BLOCK *)(delayLink - 1) ;
            l4remove( &opt->delayAvail, delayLink ) ;
            LeaveCriticalSection( &opt->critical4optWrite ) ;
            memcpy( delayBlock->data, block->data, block->len ) ;

            if ( file4writeOpt( block->file, block->pos, delayBlock->data, block->len, 1, opt4writeCompletionRoutine, &delayBlock->lruLink ) != r4delay )
            {
               /* the delay links were not used, so put onto list */
               EnterCriticalSection( &opt->critical4optWrite ) ;
               l4add( &opt->delayAvail, (LINK4 *)&delayBlock->lruLink ) ;
               LeaveCriticalSection( &opt->critical4optWrite ) ;
            }
         }
         else
      #endif
      {
         if ( ( rc = file4writeOpt( block->file, block->pos, block->data, block->len, 0, 0, 0 ) ) != 0 )
            return rc ;
      }
      block->file->doBuffer = 1 ;
   }

   block->changed = 0 ;

   return 0 ;
}

/* esp. in delay-write, may return a different block back to user for
   use, instead of the current one */
/*OPT4BLOCK *opt4blockRemove( OPT4 *opt, OPT4BLOCK *block, int doFlush )*/
int opt4blockRemove( OPT4 *opt, OPT4BLOCK *block, int doFlush )
{
   #ifdef E4PARM_LOW
      if ( block == 0 )
      {
         error4( 0, e4parm_null, E92508 ) ;
         return 0 ;
      }
   #endif

   if ( doFlush && block->changed )
   {
      block->file->doBuffer = 0 ;
      opt4blockFlush( opt, block, 1, 1 ) ;
      block->file->doBuffer = 1 ;
   }

   if ( block->file != 0 )
   {
      l4remove( &opt->lists[opt4fileHash( opt, block->file, (unsigned long)block->pos )], block ) ;
      block->file = 0 ;
   }

   opt4blockClear( block ) ;

/*   return block ;  */
   return 0 ;
}

/* function to remove, without saving, parts of an optimized file */
int opt4fileDelete( FILE4 *file, unsigned long lowPos, unsigned long hiPos )
{
   long hashVal ;
   unsigned long onPos, endDeletePos ;
   OPT4BLOCK *blockOn ;
   OPT4 *opt ;
   FILE4LONG fLong ;

   #ifdef E4PARM_LOW
      if ( file == 0 || lowPos < 0 || hiPos < 0 )
         return error4( 0, e4parm, E92508 ) ;
   #endif

   opt = &file->codeBase->opt ;

   hashVal = opt4fileHash( opt, file, (unsigned long)lowPos ) ;
   if ( hashVal != opt4fileHash( opt, file, lowPos + file->codeBase->opt.blockSize - 1L ) )  /* not on a block boundary, so delete partial */
   {
      /* lowPos / blockSize because position must be on a block boundary */
      blockOn = opt4fileReturnBlock( file, lowPos / file->codeBase->opt.blockSize, hashVal ) ;
      if ( blockOn )  /* block in memory, so partially delete */
      {
         if ( file4longGetLo( file->len ) <= hiPos )   /* just removing all file, so merely delete */
            blockOn->len = (unsigned)(lowPos - blockOn->pos) ;
         else  /* read the old data into the block - ignore errors since could be eof, else catch later */
         {
            file4longAssign( fLong, lowPos, 0 ) ;
            file4readInternal( file, fLong, (char *)blockOn->data + blockOn->pos - lowPos, (unsigned)(opt->blockSize - (lowPos - blockOn->pos)) ) ;
         }
      }
      onPos = ( (lowPos + opt->blockSize) >> opt->blockPower ) << opt->blockPower ;
   }
   else
      onPos = lowPos ;

   endDeletePos = hiPos + opt->blockSize - 1 ;
   while( onPos < endDeletePos )
   {
      /* onPos / blockSize because position must be on a block boundary */
      blockOn = opt4fileReturnBlock( file, onPos / file->codeBase->opt.blockSize, opt4fileHash( opt, file, (unsigned long)onPos ) ) ;
      if ( blockOn )  /* block in memory, so delete */
      {
         opt4blockRemove( opt, blockOn, 0 ) ;
         opt4blockLruTop( blockOn ) ;
         l4addBefore( &opt->avail, l4first( &opt->avail ), &blockOn->lruLink ) ;
      }
      onPos += opt->blockSize ;
   }

   onPos -= opt->blockSize ;
   if ( onPos < hiPos )
   {
      blockOn = opt4fileReturnBlock( file, (unsigned long)onPos, opt4fileHash( opt, file, (unsigned long)onPos ) ) ;
      if ( blockOn )
      {
         if ( file4longGetLo( file->len ) <= hiPos )
         {
            opt4blockRemove( opt, blockOn, 0 ) ;
            opt4blockLruTop( blockOn ) ;
            l4addBefore( &opt->avail, l4first( &opt->avail ), &blockOn->lruLink ) ;
         }
         else
         {
            file4longAssign( fLong, onPos, 0 ) ;
            file4readInternal( file, fLong, blockOn->data, hiPos - blockOn->pos ) ;
         }
      }
   }
   return 0 ;
}

int opt4flushAll( OPT4 *opt, char doFree )
{
   OPT4BLOCK *blockOn ;
   LINK4 *linkOn, *nextLink ;
   LIST4 *flushList ;
   char i ;
   int rc, saveRc ;

   #ifdef E4PARM_LOW
      if ( opt == 0 )
         return error4( 0, e4parm_null, E92508 ) ;
   #endif

   saveRc = opt4flushWriteBuffer( opt ) ;

   for ( i = 0 ; i < OPT4NUM_LISTS ; i++ )
   {
      flushList = &opt->prio[i]->list ;

      for( linkOn = (LINK4 *)l4first( flushList ) ; linkOn != 0; )
      {
         blockOn = (OPT4BLOCK *)( linkOn - 1 ) ;
         if( blockOn->changed )
         {
            rc = opt4blockFlush( opt, blockOn, 1, 0 ) ;
            if ( rc != 0 )
               saveRc = rc ;
         }

         if ( doFree )
         {
            nextLink = (LINK4 *)l4next( flushList, linkOn ) ;
            l4remove( &opt->lists[ opt4fileHash( opt, blockOn->file, (unsigned long)blockOn->pos )], blockOn ) ;
            opt4blockLruTop( blockOn ) ;
            l4add( &opt->avail, linkOn ) ;
            linkOn = nextLink ;
            opt4blockClear( blockOn ) ;
         }
         else
            linkOn = (LINK4 *)l4next( flushList, linkOn ) ;
      }
   }
   return saveRc ;
}

int opt4fileFlushList( OPT4 *opt, FILE4 *file, LIST4 *flushList, int doFree )
{
   OPT4BLOCK *blockOn ;
   LINK4 *linkOn, *nextLink ;

   #ifdef E4PARM_LOW
      if ( file == 0 || flushList == 0 )
         return error4( 0, e4parm, E92508 ) ;
   #endif

   for( linkOn = (LINK4 *)l4first( flushList ) ; linkOn != 0; )
   {
      blockOn = (OPT4BLOCK *)( linkOn - 1 ) ;
      if( blockOn->file == file )
      {
         if( blockOn->changed )
            if ( opt4blockFlush( opt, blockOn, 1, 0 ) < 0 )
               return -1 ;

         if ( doFree )
         {
            nextLink = (LINK4 *)l4next( flushList, linkOn ) ;
            l4remove( &opt->lists[ opt4fileHash( opt, file, (unsigned long)blockOn->pos )], blockOn ) ;
            opt4blockLruTop( blockOn ) ;
            l4add( &opt->avail, linkOn ) ;
            linkOn = nextLink ;
            opt4blockClear( blockOn ) ;
         }
         else
            linkOn = (LINK4 *)l4next( flushList, linkOn ) ;
      }
      else
         linkOn = (LINK4 *)l4next( flushList, linkOn ) ;
   }
   return 0 ;
}

#ifdef S4WRITE_DELAY
void S4CALL opt4largeWriteCompletionRoutine( void *delay )
{
   /* just resets the avail flag */
   *((int *)(((FILE4WRITE_DELAY *)(delay))->completionData)) = 1 ;
}
#endif

int opt4flushWriteBuffer( OPT4 *opt )
{
   int rc, oldDoBuffer, oldBufferWrites ;
   FILE4 *file ;

   #ifdef E4PARM_LOW
      if ( opt == 0 )
         return error4( 0, e4parm_null, E92508 ) ;
   #endif

   if ( opt->writeBlockCount != 0 )
   {
      file = opt->writeFile ;
      #ifdef E4ANALYZE_ALL
         if ( file->hasDup == 1 )
            if ( file4cmpPart( file->codeBase, opt->writeBuffer, file, opt->writeStartPos, (unsigned)(opt->writeCurPos - opt->writeStartPos) ) != 0 )
               return error4( file->codeBase, e4opt, E80602 ) ;
      #endif

      oldDoBuffer = file->doBuffer ;
      oldBufferWrites = file->bufferWrites ;
      file->doBuffer = 0 ;
      file->bufferWrites = 0 ;

      #ifdef S4WRITE_DELAY
         if ( opt->writeBuffer == opt->writeBufferActual )
         {
            opt->writeBufferActualAvail = 0 ;
            rc = file4writeDelay( file, opt->writeStartPos, opt->writeBuffer,
               (unsigned)(opt->writeCurPos - opt->writeStartPos), opt4largeWriteCompletionRoutine, &opt->writeBufferActualAvail ) ;
            while ( opt->delayLargeBufferAvail != 1 )  /* wait so we can use this buffer */
               Sleep( 0 ) ;
            opt->writeBuffer = opt->delayLargeBuffer ;
         }
         else
         {
            #ifdef E4ANALYZE
               if ( opt->writeBuffer != opt->delayLargeBuffer )
                  return error4( 0, e4struct, E92508 ) ;
            #endif
            opt->delayLargeBufferAvail = 0 ;
            rc = file4writeDelay( file, opt->writeStartPos, opt->writeBuffer, (unsigned)(opt->writeCurPos - opt->writeStartPos), opt4largeWriteCompletionRoutine, &opt->delayLargeBufferAvail ) ;
            while ( opt->writeBufferActualAvail != 1 )  /* wait so we can use this buffer */
               Sleep( 0 ) ;
            opt->writeBuffer = opt->writeBufferActual ;
         }
      #else
         rc = file4write( file, opt->writeStartPos, opt->writeBuffer, (unsigned)(opt->writeCurPos - opt->writeStartPos) ) ;
      #endif
      file->doBuffer = oldDoBuffer ;
      file->bufferWrites = oldBufferWrites ;
      if ( rc < 0 )
         return rc ;
      opt->writeStartPos = opt->writeCurPos = opt->writeBlockCount = 0 ;
      opt->writeFile = 0 ;
   }

   return 0 ;
}

/* this function sets the 'selected' member of the lru-list to show which
   links are considered available. */
static void opt4fileMarkAvailable( OPT4 *opt, int useCount )
{
   OPT4LIST *optList ;
   OPT4BLOCK *block ;
   LINK4 *link ;
   LIST4 *list ;
   int i ;
   unsigned short int linksLeft ;
   unsigned long elapsedTime ;

   for ( i = 0 ; i < OPT4NUM_LISTS ; i++ )
   {
      optList = opt->prio[i] ;

      if ( useCount )
      {
         optList->currentPrioCount-- ;
         if ( optList->currentPrioCount > 0 )
            continue ;
         optList->currentPrioCount = i ;  /* reset priority */
      }

      list = &optList->list ;

      for ( linksLeft = l4numNodes( list ), link = (LINK4 *)l4first( list ) ;; linksLeft--, link = (LINK4 *)l4next( list, link ) )
      {
         if ( link == 0 )
            break ;
         block = (OPT4BLOCK *)(link - 1) ;

         elapsedTime = opt->accessTimeCount - block->accessTime ;
         if ( elapsedTime < optList->minTime )  /* all blocks this new must remain on list */
            break ;

         /* other wise determine based on minimum links */

         elapsedTime = opt->readTimeCount - block->readTime ;
         if ( elapsedTime > optList->maxTime )  /* candidate for removal */
         {
            list->selected = link ;
            continue ;
         }

         /* else use maximum link information */
         if ( linksLeft > optList->minLink )
         {
            list->selected = link ;
            continue ;
         }

         /* else at or below the minimum allowed links, so stop */
         break ;
      }

      if ( list->selected == 0 )   /* there were none available, so increase the time before a re-scan */
         optList->currentPrioCount++ ;
   }
}

/* also sets the priority scheme */
static OPT4BLOCK *opt4fileChooseBlock( FILE4 *file )
{
   LINK4 *lruLink ;
   LIST4 *chooseList ;
   OPT4 *opt ;
   int i, listAvg, l1, l2, l3 ;
   OPT4BLOCK *block ;

   #ifdef E4PARM_LOW
      if ( file == 0 )
      {
         error4( 0, e4parm_null, E92508 ) ;
         return 0 ;
      }
   #endif

   opt = &file->codeBase->opt ;

   if ( opt->avail.nLink )
      chooseList = &opt->avail ;
   else
   {
      for( i = 0 ; i < OPT4NUM_LISTS ; i++ )
      {
         chooseList = &opt->prio[i]->list ;
         if ( chooseList->selected != 0 )   /* we have the desired link */
            break ;
      }

      if ( i >= OPT4NUM_LISTS )
      {
         /* no candidate lists were found, so try finding something available */
         opt4fileMarkAvailable( opt, 1 ) ;

         for( i = 0 ; i < OPT4NUM_LISTS ; i++ )
         {
            chooseList = &opt->prio[i]->list ;
            if ( chooseList->selected != 0 )   /* we have the desired link */
               break ;
         }

         if ( i >= OPT4NUM_LISTS )
         {
            /* no candidate lists were found, mark available for ALL lists */
            opt4fileMarkAvailable( opt, 0 ) ;

            for( i = 0 ; i < OPT4NUM_LISTS ; i++ )
            {
               chooseList = &opt->prio[i]->list ;
               if ( chooseList->selected != 0 )   /* we have the desired link */
                  break ;
            }

            if ( i >= OPT4NUM_LISTS )
            {
               /* no candidate lists were found, so try list normalization */
               l1 = l4numNodes( &opt->prio[0]->list ) ;
               l2 = l4numNodes( &opt->prio[1]->list ) ;
               l3 = l4numNodes( &opt->prio[2]->list ) ;
               listAvg = (l1 + l2 + l3) / 3  ;

               chooseList = 0 ;

               if ( listAvg > 0 )
               {
                  if ( l1 > listAvg )
                     chooseList = &opt->prio[0]->list ;
                  else
                  {
                     if ( l2 > listAvg )
                        chooseList = &opt->prio[1]->list ;
                     else
                        chooseList = &opt->prio[2]->list ;
                  }
               }
               else
               {
                  /* just try to get a link */
                  chooseList = &opt->prio[3]->list ;
                  if ( l4numNodes( chooseList ) == 0 )
                  {
                     chooseList = &opt->prio[4]->list ;
                     if ( l4numNodes( chooseList ) == 0 )
                        chooseList = &opt->prio[5]->list ;
                  }

               }
            }
         }
      }
   }

   lruLink = (LINK4 *)l4first( chooseList ) ;
   if ( lruLink == 0 )   /* none were available, this is an error */
   {
      error4( 0, e4struct, E92508 ) ;
      return 0 ;
   }
   if ( lruLink == chooseList->selected )  /* lru link is selected, set selected to null */
      chooseList->selected = (LINK4 *)0 ;
   l4remove( chooseList, lruLink ) ;
   block = (OPT4BLOCK *)( lruLink - 1 ) ;
   return block ;
}

static void opt4blockUpgradePriorityCheck( OPT4BLOCK *block, OPT4 *opt )
{
   #ifndef S4OFF_INDEX
      #ifdef N4OTHER
         TAG4FILE *t4file ;
      #else
         INDEX4FILE *i4file ;
      #endif
   #endif
   DATA4FILE *d4file ;

   if ( block->optList == &opt->dbfLo )  /* maybe move to hi */
   {
      d4file = ((DATA4FILE *)block->file->ownerPtr) ;
      /* NOT moved to hi-priority if record size > 4k, to avoid memory congestion (except header pos 0 block) */
      if ( d4file != 0 )
         if ( block->pos == 0 || (d4file->hiPrio == 1 && dfile4recWidth( d4file ) > 4096 ) )
         {
            l4remove( &block->optList->list, &block->lruLink ) ;
            opt4listLruBottomPlace( &opt->dbfHi, block ) ;
         }
      return ;
   }

   #ifndef S4OFF_INDEX
      if ( block->optList == &opt->indexLo )  /* maybe move to hi */
      {
         #ifdef N4OTHER
            t4file = ((TAG4FILE *)block->file->ownerPtr) ;

            if ( t4file != 0 )
            {
               if ( t4file->readBlockTag == 0 || block->len != opt->blockSize )  /* not reading a block, so leave at low priority */
                  return ;

               /* first ensure that it is on a real index boundary, and that we read the whole block */
               if ( opt->blockSize / 2 == B4BLOCK_SIZE )  /* must check both blocks */
               {
                  if ( b4dataLeaf( block->data, t4file->readBlockTag ) == 0 )
                  {
                     l4remove( &block->optList->list, &block->lruLink ) ;
                     opt4listLruBottomPlace( &opt->indexHi, block ) ;
                     return ;
                  }
                  if ( b4dataLeaf( (char *)block->data + B4BLOCK_SIZE, t4file->readBlockTag ) == 0 )
                  {
                     l4remove( &block->optList->list, &block->lruLink ) ;
                     opt4listLruBottomPlace( &opt->indexHi, block ) ;
                  }
               }

               if ( opt->blockSize == B4BLOCK_SIZE )
                  if ( b4dataLeaf( block->data, t4file->readBlockTag ) == 0 )
                  {
                     l4remove( &block->optList->list, &block->lruLink ) ;
                     opt4listLruBottomPlace( &opt->indexHi, block ) ;
                  }
            }
         #else
            i4file = ((INDEX4FILE *)block->file->ownerPtr) ;

            if ( i4file != 0 )
            {
               if ( i4file->readBlockTag == 0 || block->len != opt->blockSize )  /* not reading a block, so leave at low priority */
                  return ;

               /* first ensure that it is on a real index boundary, and that we read the whole block */
               #ifdef S4MDX
                  if ( (unsigned long)i4file->header.blockRw != opt->blockSize )
                     return ;

                  /* if it is a branch block, then upgrade it's priority */
                  if ( b4dataLeaf( block->data, i4file->readBlockTag ) == 0 )
                  {
                     l4remove( &block->optList->list, &block->lruLink ) ;
                     opt4listLruBottomPlace( &opt->indexHi, block ) ;
                  }
               #else
                  if ( opt->blockSize / 2 == B4BLOCK_SIZE )  /* must check both blocks */
                  {
                     if ( b4dataLeaf( block->data, i4file->readBlockTag ) == 0 )
                     {
                        l4remove( &block->optList->list, &block->lruLink ) ;
                        opt4listLruBottomPlace( &opt->indexHi, block ) ;
                        return ;
                     }
                     if ( b4dataLeaf( (char *)block->data + B4BLOCK_SIZE, i4file->readBlockTag ) == 0 )
                     {
                        l4remove( &block->optList->list, &block->lruLink ) ;
                        opt4listLruBottomPlace( &opt->indexHi, block ) ;
                     }
                  }

                  if ( opt->blockSize == B4BLOCK_SIZE )
                     if ( b4dataLeaf( block->data, i4file->readBlockTag ) == 0 )
                     {
                        l4remove( &block->optList->list, &block->lruLink ) ;
                        opt4listLruBottomPlace( &opt->indexHi, block ) ;
                     }
               #endif
            }
         #endif
      }
   #endif /* S4OFF_INDEX */
}

static OPT4LIST *opt4listDetermine( OPT4 *opt, FILE4 *file, int hiPrio )
{
   switch ( file->type )
   {
      case OPT4DBF:
         if ( hiPrio == 1 )
            return &opt->dbfHi ;
         else
            return &opt->dbfLo ;
      case OPT4INDEX:
         if ( hiPrio == 1 )
            return &opt->indexHi ;
         else
            return &opt->indexLo ;
      default:
         return &opt->other ;
   }
}

/* gets an available block, and places it on the appropriate lru-list */
/* also sets the readTime and accessTime for the block */
static OPT4BLOCK *opt4fileGetBlock( OPT4 *opt, FILE4 *file, int hiPrio )
{
   OPT4BLOCK *block ;
   OPT4LIST *optList ;

   #ifdef E4PARM_LOW
      if ( file == 0 )
      {
         error4( 0, e4parm_null, E92508 ) ;
         return 0 ;
      }

   #endif

   block = opt4fileChooseBlock( file ) ;
   opt4blockRemove( opt, block, 1 ) ;

   optList = opt4listDetermine( opt, file, hiPrio ) ;
   opt4listLruBottomPlace( optList, block ) ;

   block->readTime = opt->readTimeCount++ ;
   if ( opt->readTimeCount == 0 )  /* time count got reset, so reset for everyone */
      opt4timeReset( opt, 1, 0 ) ;
   block->accessTime = ++opt->accessTimeCount ;

   return block ;
}

static void opt4timeReset( OPT4 *opt, int doReadTime, int doAccessTime )
{
   OPT4BLOCK *blockOn ;
   LIST4 *listOn ;
   int i ;

   /* for simplicity, just reset all blocks read-time to 0 (i.e. just read) */
   /* in general this function will rarely be called, thus the impact should be minor */

   for ( i = 0 ; i < OPT4NUM_LISTS ; i++ )
   {
      listOn = &opt->prio[i]->list ;
      blockOn = (OPT4BLOCK *)l4first( listOn ) ;
      for( ;; )
      {
         if ( blockOn == 0 )
            break ;
         if ( doReadTime )
            blockOn->readTime = 0 ;
         if ( doAccessTime )
            blockOn->accessTime = 0 ;
         blockOn = (OPT4BLOCK *)l4next( listOn, blockOn ) ;
      }
   }
}

/* inlined now
long opt4fileHash( OPT4 *opt, FILE4 *file, unsigned long pos )
{
   return ( (( file->hashInit + pos ) >> opt->blockPower ) & opt->mask ) ;
}
*/

/* 06/28/96 AS --> 16-bit, required len may be > sizeof unsigned because the
   value is added to around line 880 below; therefore must use longs */
int opt4fileWrite( FILE4 *file, unsigned long pos, unsigned len, const void *data, char changed )
{
   S4LONG hashVal ;
   S4UNSIGNED_LONG lenWritten, adjustedPos ;
   unsigned readLen, lenRead, extraRead ;
   OPT4BLOCK *blockOn ;
   OPT4 *opt ;
   int doUpgradeCheck ;
   unsigned int a, b ; /* Used for min() reproduction */

   #ifdef E4PARM_LOW
      if ( file == 0 || pos < 0 || data == 0 )
         return error4( 0, e4parm, E92508 ) ;
   #endif

   #ifdef S4ADVANCE_READ
      /* ensure that the information to be written replaces any advanced-read
         information in order to ensure that the advance-read data is up to
         date */

      if ( changed == 1 )
         file4advanceReadWriteOver( file, pos, len, data, 1 ) ;
   #endif

   #ifdef S4FILE_EXTENDED
      #ifdef E4ANALYZE
         if ( (pos + len) < pos )  /* means overflow, we have exceeded 4 gig file limit */
            return error4( 0, e4parm, E92508 ) ;
      #endif
   #endif

   opt = &file->codeBase->opt ;
   lenWritten = 0 ;
   extraRead = (unsigned) ((unsigned S4LONG)((unsigned S4LONG)pos << opt->numShift ) >> opt->numShift ) ;
   adjustedPos = pos - extraRead ;
   if( len > ( (unsigned long)opt->numBlocks * (unsigned long)opt->blockSize ) )
   {
      /* case where amount to write > total bufferred length - do a piece at a time */
      adjustedPos = (S4LONG)( ((S4LONG)opt->numBlocks - 1) * (S4LONG)opt->blockSize ) ;
      for ( lenWritten = 0L ; len > lenWritten ; lenWritten += adjustedPos )
      {
         if ( ( len - lenWritten ) < adjustedPos )
            adjustedPos = len - lenWritten ;
         if ( opt4fileWrite( file, pos + lenWritten, (unsigned)adjustedPos, (char *)data + lenWritten, changed ) != (int)adjustedPos )
            return (int)lenWritten ;
      }
      return (int)lenWritten ;
   }
   len += extraRead ;

   do
   {
      hashVal = opt4fileHash( opt, file, (unsigned S4LONG)adjustedPos ) ;
      readLen = (unsigned) ((len / (unsigned)opt->blockSize) ? opt->blockSize : len) ;
      blockOn = opt4fileReturnBlock( file, adjustedPos, hashVal ) ;
      if ( blockOn == 0 )
      {
         blockOn = opt4fileGetBlock( opt, file, 0 ) ;
         #ifdef E4ANALYZE
            if ( blockOn == 0 )
               return error4( 0, e4info, E92508 ) ;
         #endif
         if ( (unsigned S4LONG)( readLen - extraRead ) < opt->blockSize && adjustedPos < file4longGetLo( file4lenLow( file ) ) )
            lenRead = opt4fileReadFile( file, adjustedPos, (char *)blockOn->data ) ;
         else
         {
            if ( file4longGetLo( file4lenLow( file ) ) >= (S4UNSIGNED_LONG) (pos + lenWritten + opt->blockSize) )   /* mark block as full to avoid file len set below... */
               lenRead = (unsigned)opt->blockSize ;
            else
               lenRead = 0 ;
         }
         opt4blockAdd( blockOn, file, lenRead, hashVal, adjustedPos ) ;
         doUpgradeCheck = 1 ;
      }

      if ( readLen < (unsigned) extraRead )
         readLen = (unsigned) extraRead ;
      memcpy( (char *)blockOn->data + extraRead, (char *)data + lenWritten, (unsigned)(readLen - extraRead) ) ;
      if ( doUpgradeCheck == 1 )
         opt4blockUpgradePriorityCheck( blockOn, opt ) ;

      opt4listLruBottomShift( blockOn ) ;
      blockOn->changed |= changed ;
      len -= readLen ;
      lenWritten += readLen - extraRead ;
      extraRead = 0 ;
      adjustedPos += opt->blockSize ;
      if ( pos + lenWritten > blockOn->pos + (S4UNSIGNED_LONG)blockOn->len )
      {
         if ( file4longError( file->len ) == -1L || ( file4longGetLo( file->len ) < ( pos + lenWritten ) ) ) /* file size has grown */
         {
            if ( blockOn->pos + (S4UNSIGNED_LONG)blockOn->len < pos )
               memset( ((char *)blockOn->data) + blockOn->len, 0,(unsigned)( pos - ( blockOn->pos + blockOn->len ) ) ) ;
            blockOn->len = (unsigned)( pos - blockOn->pos + lenWritten ) ;
            if ( file->bufferWrites == 1 )
            {
               file4longAssign( file->len, pos + lenWritten, 0 ) ;
               #ifdef E4ANALYZE
                  /* make sure the file length really has grown */
                  if ( file->fileCreated )
                     if ( file4longGetLo( u4filelength( file->hand ) ) > file4longGetLo( file->len ) )
                        return error4( file->codeBase, e4info, E92508 ) ;
               #endif
            }
         }
         else   /* we have written past our end of block, but not our block, update our block len, dummy info */
         {
            a = (unsigned) opt->blockSize ;
            b = (unsigned)( file4longGetLo( file->len ) - blockOn->pos ) ;
            blockOn->len = (a>b)?b:a ; /* minimum of a or b */
             /* AS 02/16/97 changed, else c/s MDX t5row5 failed */
         }
      }
   } while( len && blockOn->len == (unsigned)opt->blockSize ) ;

   return (unsigned) lenWritten ;
}

/* 06/28/96 AS --> 16-bit, required len may be > sizeof unsigned because the
   value is added to around line 880 below; therefore must use longs */
unsigned opt4fileRead( FILE4 *f4, unsigned long pos, void *data, unsigned len )
{
   S4LONG hashVal ;
   S4UNSIGNED_LONG adjustedPos, lenRead ;
   unsigned readLen, extraRead ;
   int blocksNeeded ;
   OPT4BLOCK *blockOn ;
   OPT4 *opt ;
   DATA4FILE *d4file ;

   #ifdef E4PARM_LOW
      if ( f4 == 0 || pos < 0 || data == 0 )
      {
         error4( 0, e4parm, E92508 ) ;
         return 0 ;
      }
   #endif

   opt = &f4->codeBase->opt ;
   lenRead = 0 ;
   extraRead = (unsigned)((unsigned S4LONG)((unsigned S4LONG)pos << opt->numShift ) >> opt->numShift ) ;
   adjustedPos = pos - extraRead ;
   if( (unsigned S4LONG)len > ( opt->numBlocks * opt->blockSize ))
   {
      /* case where amount to read > total bufferred length - do a piece at a time */
      adjustedPos = (S4LONG)( ((S4LONG)opt->numBlocks - 1) * (S4LONG)opt->blockSize ) ;
      for ( lenRead = 0 ; len > lenRead ; lenRead += adjustedPos )
      {
         if ( (len - lenRead ) < adjustedPos )
            adjustedPos = len - lenRead ;
         if ( opt4fileRead( f4, pos + lenRead, (char *)data + lenRead, (unsigned)adjustedPos ) != (unsigned)adjustedPos )
            return (unsigned)lenRead ;
      }
      #ifdef E4ANALYZE_ALL
         if ( f4->hasDup == 1 )
            if ( file4cmpPart( f4->codeBase, data, f4, pos, (unsigned)lenRead ) != 0 )
            {
               error4( f4->codeBase, e4opt, E80602 ) ;
               return 0 ;
            }
      #endif

      return (unsigned)lenRead ;
   }
   len += extraRead ;

   do
   {
      hashVal = opt4fileHash( opt, f4, (unsigned long)adjustedPos ) ;
      readLen = (unsigned) ((len / opt->blockSize) ? opt->blockSize : len ) ;
      blockOn = opt4fileReturnBlock( f4, adjustedPos, hashVal ) ;
      if ( blockOn == 0 )  /* read from disk */
      {
         /* first ensure that the file is not a temporary file that is empty */
         if ( f4->fileCreated == 0 )  /* not created, so if not in memory then empty */
            return (unsigned)0 ;

         /* if force current is on, then pre-reading will be wasted, so just skip */
         if ( opt->forceCurrent != 1 )
         {
            if ( f4->type == OPT4DBF )
            {
               d4file = (DATA4FILE *)f4->ownerPtr ;
               if ( d4file != 0 )
                  if ( d4file->hiPrio == -1 )
                  {
                     opt4fileReadSpBuffer( f4, (unsigned long)adjustedPos, -1, -1 ) ;
                     blockOn = opt4fileReturnBlock( f4, adjustedPos, hashVal ) ;
                  }
            }

            if ( blockOn == 0 )
            {
               /* depending on the amount to read, it may still make sense to
                  do advance reading -- quicker to read 2 blocks now than to
                  read 1 block twice */
               if ( len - readLen > 0 )  /* need to read more than 1 block */
               {
                  /* if the 2nd block is already in memory, don't bother */
                  blockOn = opt4fileReturnBlock( f4, adjustedPos + opt->blockSize, opt4fileHash( opt, f4, (unsigned long)adjustedPos + opt->blockSize ) ) ;
                  if ( blockOn == 0 )  /* not there, so get */
                  {
                     /* check to see how many blocks we need */
                     blocksNeeded = 1 + (int)(len / readLen) ;  /* 1st block, plus any extras */
                     opt4fileReadSpBuffer( f4, (unsigned long)adjustedPos, blocksNeeded, 1 ) ;
                     blockOn = opt4fileReturnBlock( f4, adjustedPos, hashVal ) ;
                  }
                  else  /* the 2nd block is there, so reset blockOn and read the first one */
                     blockOn = 0 ;
               }
            }
         }

         if ( blockOn == 0 )
         {
            blockOn = opt4fileGetBlock( opt, f4, 0 ) ;
            opt4blockAdd( blockOn, f4, opt4fileReadFile( f4, adjustedPos, (char *)blockOn->data ), hashVal, adjustedPos ) ;
            opt4blockUpgradePriorityCheck( blockOn, opt ) ;
         }
      }
      else
         if ( opt->forceCurrent == 1 )
         {
            if ( blockOn->changed == 0 && f4->bufferWrites == 0
               #ifndef S4OFF_MULTI
                  && f4->lowAccessMode == OPEN4DENY_NONE
               #endif
               )
               opt4fileReadFile( f4, adjustedPos, (char *)blockOn->data ) ;
         }

      opt4listLruBottomShift( blockOn ) ;

      if ( blockOn->len < readLen )
         readLen = blockOn->len ;
      if ( readLen < (unsigned) extraRead )
         readLen = (unsigned) extraRead ;
      memcpy( (char *)data + lenRead, (char *)blockOn->data + extraRead, (unsigned)(readLen - extraRead) ) ;
      len -= readLen ;
      lenRead += readLen - extraRead ;
      extraRead = 0 ;
      adjustedPos += opt->blockSize ;
   } while( len && blockOn->len == (unsigned) opt->blockSize ) ;
   #ifdef E4ANALYZE_ALL
      if ( f4->hasDup == 1 )
         if ( file4cmpPart( f4->codeBase, data, f4, pos, (unsigned)lenRead ) != 0 )
         {
            error4( f4->codeBase, e4opt, E80602 ) ;
            return 0 ;
         }
   #endif

   return (unsigned)lenRead ;
}

void opt4blockLruTop( OPT4BLOCK *block )
{
   LIST4 *list ;
   LINK4 *l4link ;

   #ifdef E4PARM_LOW
      if ( block == 0 )
      {
         error4( 0, e4parm_null, E92508 ) ;
         return ;
      }
   #endif

   list = &block->optList->list ;
   l4link = &block->lruLink ;

   if ( list->selected == l4link )
      list->selected = (LINK4 *)l4prev( list, l4link ) ;

   l4remove( list, l4link ) ;
}

#ifdef S4ADVANCE_TEST
   unsigned S4FUNCTION opt4fileReadFile( FILE4 *file, const unsigned long posIn, char *buf )
#else
   static unsigned opt4fileReadFile( FILE4 *file, const unsigned long posIn, char *buf )
#endif
{
   OPT4 *opt ;
   unsigned long pos ;
   FILE4LONG fPos ;
   #ifdef S4ADVANCE_READ
      unsigned int compareLen, copyOffset, len ;
   #endif

   #ifdef E4PARM_LOW
      if ( file == 0 || posIn < 0 || buf == 0 )
      {
         error4( 0, e4parm, E92508 ) ;
         return 0 ;
      }
   #endif

   pos = posIn ;
   opt = &file->codeBase->opt ;

   #ifdef S4ADVANCE_READ
      /* first check the advance-read buffer.  If the data is available there,
         then copy it out.  Can't do more than that here because this function
         is reading into a block already (don't want to overwrite that one by
         mistake */

      /* only copy if can get the entire block from advance-read (in theory
         must always be able to get none or all since on block boundaries */

      if ( opt->advanceLargeBufferAvail != AR4EMPTY )
         if ( file == opt->advanceReadFile )
         {
            len = (unsigned)(opt->blockSize) ;

            /* if an overlap of any type occurs, then copy that part out first,
               and then take care of the remaining pieces */
            if ( pos >= opt->advanceLargePos && pos < opt->advanceLargePos + opt->advanceLargeLen )
            {
               if ( pos == opt->advanceLargePos )  /* easy case */
               {
                  if ( opt->advanceLargeLen >= len )
                  {
                     while( opt->advanceLargeBufferAvail == AR4SET )  /* wait for the buffer to complete read, if reqd. */
                        Sleep( 0 ) ;
                     if ( opt->advanceLargeBufferAvail == AR4FULL ) /* successful read */
                     {
                        memcpy( buf, opt->advanceLargeBuffer, len ) ;
                        /* reset the position of the data for easier handling later */
                        opt->advanceLargePos += len ;
                        opt->advanceLargeLen -= len ;
                        memcpy( opt->advanceLargeBuffer, opt->advanceLargeBuffer + len, opt->advanceLargeLen ) ;
                        if ( opt->advanceLargeLen == 0 )  /* just remove */
                        {
                           opt->advanceLargeBufferAvail = AR4EMPTY ;
                           opt->advanceReadFile = 0 ;
                        }
                        return len ;
                     }
                  }
               }
               else
                  if ( pos > opt->advanceLargePos )  /* full copy only */
                  {
                     copyOffset = pos - opt->advanceLargePos ;
                     compareLen = opt->advanceLargeLen - copyOffset ;
                     if ( compareLen >= len )  /* then ok */
                     {
                        while( opt->advanceLargeBufferAvail == AR4SET )  /* wait for the buffer to complete read, if reqd. */
                           Sleep( 0 ) ;
                        if ( opt->advanceLargeBufferAvail == AR4FULL ) /* successful read */
                        {
                           if ( compareLen == len )
                              opt->advanceLargeLen -= compareLen ;   /* remove trailer, since will get copied into buffers anyway */
                           memcpy( buf, opt->advanceLargeBuffer + copyOffset, len ) ;
                           return len ;
                        }
                     }
                  }
            }
         }
   #endif

   if ( opt->writeFile == file )
      if ( (unsigned long)pos + opt->blockSize >= opt->writeStartPos && (unsigned long) pos < opt->writeCurPos )
         opt4flushWriteBuffer( opt ) ;

   /* 07/01/96 AS --> t4excl.c c/s failure after d4zap()
      do not exceed reading the file size if delay-write is true because
      it may still be on disk (i.e. file size may not yet be updated)
      len = file4readLow( file, pos, buf, (unsigned)(opt->blockSize) ) ;
   */

   file4longAssign( fPos, pos, 0 ) ;
   if ( file->bufferWrites != 0 )
      if ( file4longError( file->len ) != -1 )
         if ( file4longGetLo( file->len ) < pos + opt->blockSize )
            return file4readLow( file, fPos, buf, (unsigned)(file4longGetLo( file->len ) - pos) ) ;

   return file4readLow( file, fPos, buf, (unsigned)(opt->blockSize) ) ;
}

/* this function performs larger reads into the given buffer with the
   desired length to read.   This is used for advance reads and when
   larger amounts of information are required
   returned is the length read */
static unsigned long opt4fileReadToBuffer( FILE4 *file, char *buf, unsigned long pos, unsigned int readLen )
{
   unsigned long len, saveBlockSize ;
   OPT4 *opt ;

   opt = &file->codeBase->opt ;
   saveBlockSize = opt->blockSize ;
   opt->blockSize = readLen ;
   len = opt4fileReadFile( file, pos, buf ) ;
   opt->blockSize = saveBlockSize ;

   return len ;
}

#ifdef S4ADVANCE_READ
void S4CALL opt4readCompletionRoutine( void *advance )
{
   FILE4ADVANCE_READ *advanceRead ;
   int *arFlag ;

   /* to verify safety, use critical section on the arFlag (from CODE4) */

   advanceRead = (FILE4ADVANCE_READ *)advance ;
   arFlag = (int *)(advanceRead->completionData) ;
   if ( advanceRead->usageFlag == r4canceled )
      *arFlag = AR4EMPTY ;
   else
      if ( *arFlag == AR4SET )  /* if reset to empty it means the read was cancelled by the main thread, so leave as empty */
      {
         /* verify that all was read */
         if ( advanceRead->file->codeBase->opt.advanceLargeLen == advanceRead->status )
            *arFlag = AR4FULL ;
         else
            *arFlag = AR4EMPTY ;
      }
}

static void opt4fileReadAdvanceBuffer( OPT4 *opt, FILE4 *f4, void *buffer, long pos, unsigned int len )
{
   long hashVal ;

   hashVal = opt4fileHash( opt, f4, (unsigned long)pos ) ;
   if ( opt4fileReturnBlock( f4, pos, hashVal ) != 0 )  /* in memory, so don't perform the advance-read */
      return ;

   opt->advanceLargeBufferAvail = AR4SET ;
   opt->advanceReadFile = f4 ;
   file4advanceRead( f4, pos, buffer, len, opt4readCompletionRoutine, &opt->advanceLargeBufferAvail ) ;
}
#endif

/* this function does advance reads by reading in a whole buffer at
   a time, instead of just one block worth
   will advance-read numBlocks blocks.  If numBlocks == -1, then will advance
   read the whole read buffer, if direction == -1, then it self-detects it
   if either direction or numBlocks is -1, both must be -1 */
static void opt4fileReadSpBuffer( FILE4 *file, const unsigned long posIn, int numBlocks, int direction )
{
   unsigned long len, curPos, saveBlockSize, endPos, pos ;
   unsigned short copyPos ;
   OPT4BLOCK *blockOn ;
   OPT4 *opt ;
   long hashVal ;
   int readBlocks ;
   unsigned int curBlocks ;
   #ifdef S4ADVANCE_READ
      int posInAdvance ;  /* position to read was within the advance-read area */
      int blocksWithin ;  /* the number of blocks within the advance read once position-adjusted (i.e. the length relevanet to quantity reqd. - if numBlocks not -1 */
      int advanceNextRead, doAdvanceRead ;
      long advPos ;
   #endif

   #ifdef E4PARM_LOW
      if ( file == 0 )
      {
         error4( 0, e4parm_null, E92508 ) ;
         return ;
      }
      if ( ( direction == -1 && numBlocks != -1 ) || ( direction != -1 && numBlocks == -1 ) )
      {
         error4( file->codeBase, e4parm, E92508 ) ;
         return ;
      }
   #endif

   pos = posIn ;
   opt = &file->codeBase->opt ;
   saveBlockSize = opt->blockSize ;

   #ifdef S4ADVANCE_READ
      if ( numBlocks == -1 )
         doAdvanceRead = 1 ;
      else
         doAdvanceRead = 0 ;
      advanceNextRead = 0 ;

      /* first check the advance read-buffer, and see if it matches the
         information we want.  Even if it doesn't, if numBlocks is -1, then
         it gets copied out (to perform more advance-reading */
      if ( opt->advanceLargeBufferAvail == AR4SET ) /* wait on read if file matches */
         if ( file == opt->advanceReadFile )
            while( opt->advanceLargeBufferAvail == AR4SET )
               Sleep( 0 ) ;

      if ( opt->advanceLargeBufferAvail == AR4FULL )
      {
         if ( opt->advanceLargeLen == 0 )
         {
            opt->advanceLargeBufferAvail = AR4EMPTY ;
            opt->advanceReadFile = 0 ;
         }
         else
         {
            blocksWithin = 0 ;
            posInAdvance = 0 ;

            if ( file == opt->advanceReadFile )
            {
               if ( pos >= (unsigned long)opt->advanceLargePos && pos <= opt->advanceLargePos + opt->advanceLargeLen - saveBlockSize )
               {
                  posInAdvance = 1 ;
                  if ( numBlocks != -1 )
                     blocksWithin = ( opt->advanceLargeLen + opt->advanceLargePos - pos  ) / saveBlockSize ;
               }
               else
                  posInAdvance = 0 ;
            }

            if ( file == opt->advanceReadFile || numBlocks == -1 )  /* extract the blocks from the advance-read buffer */
            {
               advPos = curPos = opt->advanceLargePos ;
               hashVal = opt4fileHash( opt, file, curPos ) ;
               opt->advanceLargeBufferAvail = AR4EMPTY ;
               opt->advanceReadFile = 0 ;

               for ( ;; )
               {
                  blockOn = opt4fileReturnBlock( file, curPos, hashVal ) ;

                  if ( blockOn == 0 )
                  {
                     blockOn = opt4fileGetBlock( opt, file, 0 ) ;
                     memcpy( blockOn->data, opt->advanceLargeBuffer + ( curPos - advPos ), (unsigned)(saveBlockSize) ) ;
                     opt4blockAdd( blockOn, file, (unsigned)(saveBlockSize), hashVal, curPos ) ;
                     opt4blockUpgradePriorityCheck( blockOn, opt ) ;
                  }
                  else   /* update the lru status */
                     opt4listLruBottomShift( blockOn ) ;

                  curPos += saveBlockSize ;

                  #ifdef E4DEBUG
                     if ( curPos > opt->advanceLargePos + opt->advanceLargeLen )
                     {
                        error4( file->codeBase, e4struct, E92508 ) ;
                        return ;
                     }
                  #endif

                  if ( curPos == opt->advanceLargePos + opt->advanceLargeLen )
                     break ;

                  hashVal++ ;
                  if ( (unsigned long)hashVal >= opt->numLists )
                     hashVal = opt4fileHash( opt, file, curPos ) ;
               }

               if ( file == opt->advanceReadFile )
               {
                  if ( numBlocks == -1 )
                  {
                     if ( posInAdvance == 1 )
                        return ;
                  }
                  else
                     if ( blocksWithin >= numBlocks )
                        return ;
               }
            }
         }
      }
   #endif

   if ( direction == -1 )
   {
      /* first do a sample to see which way to read */
      blockOn = opt4fileReturnBlock( file, pos+saveBlockSize, opt4fileHash( opt, file, pos+saveBlockSize ) ) ;
      if ( blockOn != 0 ) /* read previous blocks, not next blocks */
      {
         if ( pos > ( opt->bufferSize - saveBlockSize ) )
            pos -= ( opt->bufferSize - saveBlockSize ) ;
         else
            pos = 0 ;
      }
   }

   readBlocks = 0 ;
   curBlocks = opt->maxBlocks ;

   for ( ;; )  /* may need to read more than the read-buffer worth if a large read */
   {
      if ( numBlocks != -1 )
      {
         if ( curBlocks > (unsigned int)( numBlocks - readBlocks ) )  /* don't over-read */
         {
            curBlocks = numBlocks - readBlocks ;
            numBlocks = -1 ;   /* will be done */
         }
      }
      len = opt4fileReadToBuffer( file, opt->readBuffer, pos, (unsigned int)( curBlocks * saveBlockSize ) ) ;
      if ( len == 0 )
         return ;

      opt->readStartPos = pos ;
      opt->readFile = file ;

      /* first do the last block, in case there is a length issue */
      curPos = pos + saveBlockSize * ( ( len - 1 ) >> opt->blockPower ) ;
      hashVal = opt4fileHash( opt, file, curPos ) ;
      blockOn = opt4fileReturnBlock( file, curPos, hashVal ) ;
      if ( blockOn == 0 )
      {
         blockOn = opt4fileGetBlock( opt, file, 0 ) ;
         endPos = curPos - pos ;
         copyPos = (unsigned short)(len - endPos) ;
         memcpy( blockOn->data, opt->readBuffer + endPos, copyPos ) ;
         opt4blockAdd( blockOn, file, copyPos, hashVal, curPos ) ;
         opt4blockUpgradePriorityCheck( blockOn, opt ) ;
      }

      if ( curPos != pos )
      {
         curPos -= saveBlockSize ;

         /* can just subtract one from hash value since it goes in order */
         for ( ;; curPos -= saveBlockSize )
         {
            hashVal-- ;
            if ( hashVal < 0 )
               hashVal = opt4fileHash( opt, file, curPos ) ;
            blockOn = opt4fileReturnBlock( file, curPos, hashVal ) ;
            if ( blockOn == 0 )
            {
               blockOn = opt4fileGetBlock( opt, file, 0 ) ;
               memcpy( blockOn->data, opt->readBuffer + ( curPos - pos ), (unsigned)(saveBlockSize) ) ;
               opt4blockAdd( blockOn, file, (unsigned)(saveBlockSize), hashVal, curPos ) ;
               opt4blockUpgradePriorityCheck( blockOn, opt ) ;
            }
            else   /* update the lru status */
               opt4listLruBottomShift( blockOn ) ;
            if ( curPos == pos )
            {
               #ifdef S4ADVANCE_READ
                  advanceNextRead = 1 ;
               #endif
               break ;
            }
         }
      }

      opt->readFile = 0 ;

      if ( numBlocks == -1 )
         break ;
      if ( len < opt->bufferSize )
         break ;

      readBlocks += curBlocks ;
      pos += opt->bufferSize ;
   }

   #ifdef S4ADVANCE_READ
      if ( opt->advanceLargeBufferAvail == AR4EMPTY && advanceNextRead == 1 && doAdvanceRead == -1 )   /* request an advance read for the next section */
      {
         if ( direction == 1 )
            advPos = ((unsigned long)(posIn >> opt->blockPower ) >> opt->blockPower ) + opt->bufferSize ;
         else
            advPos = ((unsigned long)(posIn << opt->blockPower ) >> opt->blockPower ) - 2 * opt->bufferSize ;

         if ( advPos > 0 )
         {
            opt->advanceLargePos = advPos ;
            opt->advanceLargeLen = opt->bufferSize ;
            opt4fileReadAdvanceBuffer( opt, file, opt->advanceLargeBuffer, advPos, opt->bufferSize ) ;
         }
      }
   #endif

   return ;
}

OPT4BLOCK *opt4fileReturnBlock( FILE4 *file, unsigned long pos, long hashVal )
{
   OPT4CMP compare ;
   OPT4BLOCK *blockOn ;
   LIST4 *list ;

   #ifdef E4PARM_LOW
      if ( file->codeBase->opt.numBuffers == 0 || file == 0  || hashVal < 0 || pos < 0 )
      {
         error4( 0, e4parm, E92508 ) ;
         return 0 ;
      }
      if ( (unsigned long)hashVal >= file->codeBase->opt.numLists )
      {
         error4( 0, e4parm, E92508 ) ;
         return 0 ;
      }
   #endif

   if ( file->doBuffer == 0 )
      return 0 ;

   list = &file->codeBase->opt.lists[hashVal] ;
   blockOn = (OPT4BLOCK *)l4first( list ) ;
   if ( blockOn != 0 )
   {
      compare.file = file ;
      memcpy( (void *)&compare.pos, (void *)&pos, sizeof( compare.pos ) ) ;

      for( ;; )
      {
         if ( !c4memcmp( (void *)&blockOn->file, (void *)&compare, sizeof( compare ) ) )
         {
            #ifdef E4ANALYZE_ALL
               if ( file4longGetHi( file->len ) != -1 )  /* ensure we are not beyond file length - impossible... */
                  if ( (unsigned long)(blockOn->pos + blockOn->len) > file4longGetLo( file->len ) )
                     error4( file->codeBase, e4opt, E92508 ) ;
            #endif
            return blockOn ;
         }
         blockOn = (OPT4BLOCK *)l4next( list, blockOn ) ;
         if ( blockOn == 0 )
            return 0 ;
      }
   }
   return 0 ;
}

#endif   /* not S4OPTIMIZE_OFF */
