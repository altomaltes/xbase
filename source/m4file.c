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

/* m4file.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved.  */

#include "d4all.h"

#ifndef S4MEMO_OFF

#ifndef S4OFF_MULTI
  /* ndx/clipper versions have no free chain--do not lock memo file */
#ifndef N4OTHER
/* the lock is forced since a memo file lock only lasts if the .dbf file is locked */
int memo4fileLock( MEMO4FILE *f4memo )
{
   int rc, oldAttempts ;
   CODE4 *c4 ;

   c4 = f4memo->file.codeBase ;

   if ( f4memo->fileLock == 1 )
      return 0 ;

   if ( f4memo->file.hand == INVALID4HANDLE )
      return -1 ;

   oldAttempts = c4->lockAttempts ;
   c4->lockAttempts = WAIT4EVER ;

   #ifdef S4MDX
      rc = file4lockInternal( &f4memo->file, L4LOCK_POS - 1L, 0, 2L, 0 ) ;
   #endif

   #ifdef S4FOX
      rc = file4lockInternal( &f4memo->file, L4LOCK_POS_OLD, c4->largeFileOffset, 1L, 0 ) ;
   #endif

   c4->lockAttempts = oldAttempts ;
   if ( rc == 0 )
      f4memo->fileLock = 1 ;
   #ifndef S4OPTIMIZE_OFF
      file4refresh( &f4memo->file ) ;   /* make sure all up to date */
   #endif
   return rc ;
}

int memo4fileUnlock( MEMO4FILE *f4memo )
{
   int rc ;

   if ( f4memo->fileLock == 0 )
      return 0 ;
   #ifdef S4MDX
      rc = file4unlockInternal( &f4memo->file, L4LOCK_POS - 1L, 0, 2L, 0 ) ;
   #endif
   #ifdef S4FOX
      rc = file4unlockInternal( &f4memo->file, L4LOCK_POS_OLD, f4memo->file.codeBase->largeFileOffset, 1L, 0 ) ;
   #endif
   if ( rc == 0 )
      f4memo->fileLock = 0 ;
   return rc ;
}
#endif
#endif

int memo4fileOpen( MEMO4FILE *f4memo, DATA4FILE *d4, char *name )
{
   MEMO4HEADER  header ;
   int rc ;
   FILE4LONG pos ;

   f4memo->data = d4 ;

   if ( file4open( &f4memo->file, d4->c4, name, 1 ) )
      return -1 ;

   #ifndef S4OPTIMIZE_OFF
      file4optimize( &f4memo->file, d4->c4->optimize, OPT4OTHER ) ;
   #endif

   file4longAssign( pos, 0, 0 ) ;
   if ( (rc = file4readAllInternal(&f4memo->file, pos, &header, sizeof(header))) < 0 )
      return -1 ;

   #ifdef WORDS_BIGENDIAN
      #ifdef S4MFOX
         f4memo->blockSize = header.blockSize  ;
      #else
         #ifdef S4MNDX
            f4memo->blockSize = MEMO4SIZE ;
         #else
            f4memo->blockSize = x4reverseShort( (void *)&header.blockSize) ;
         #endif
      #endif
   #else
      #ifdef S4MFOX
         f4memo->blockSize = x4reverseShort( (void *)&header.blockSize ) ;
      #else
         #ifdef S4MNDX
            f4memo->blockSize = MEMO4SIZE ;
         #else
            f4memo->blockSize = header.blockSize ;
         #endif
      #endif
   #endif
   return rc ;
}

#ifdef S4MFOX
/* offset is # bytes from start of memo that reading should begin, readMax is
   the maximum possible that can be read (limited to an unsigned int, so is
   16-bit/32-bit compiler dependent.
*/
int memo4fileReadPart( MEMO4FILE *f4memo, long memoId, char **ptrPtr, unsigned *lenPtr, unsigned long offset, const unsigned readMax, long *type )
{
   unsigned long avail ;
   MEMO4BLOCK memoBlock ;
   FILE4LONG pos ;

   if ( memoId <= 0L )
   {
      *lenPtr = 0 ;
      return 0 ;
   }

   file4longAssign( pos, (unsigned long)memoId * f4memo->blockSize, 0 ) ;

   if ( file4readAllInternal( &f4memo->file, pos, &memoBlock, sizeof( MEMO4BLOCK ) ) < 0)
      return -1 ;

   #ifndef WORDS_BIGENDIAN
      memoBlock.type = x4reverseLong( (void *)&memoBlock.type ) ;
      memoBlock.numChars = x4reverseLong( (void *)&memoBlock.numChars ) ;
   #endif

   avail = memoBlock.numChars - offset ;
   if ( avail > (unsigned long)readMax )
      avail = readMax ;

   if ( avail > (unsigned long)*lenPtr )
   {
      if ( *lenPtr > 0 )
         u4free( *ptrPtr ) ;
      *ptrPtr = (char *)u4allocEr( f4memo->file.codeBase, avail + 1 ) ;
      if ( *ptrPtr == 0 )
         return e4memory ;
   }

   *lenPtr = (unsigned)avail ;
   *type = memoBlock.type ;

   if ( avail == 0 )
      return 0 ;
   file4longAdd( &pos, offset + 2 * sizeof( S4LONG ) ) ;
   return file4readAllInternal( &f4memo->file, pos, *ptrPtr, *lenPtr ) ;
}
#endif

#ifdef S4MNDX
   extern char f4memoNullChar ;
#endif

#ifdef S4MFOX
int memo4fileRead( MEMO4FILE *f4memo, long memoId, char **ptrPtr, unsigned int *ptrLen, long *mType )
#else
int memo4fileRead( MEMO4FILE *f4memo, long memoId, char **ptrPtr, unsigned int *ptrLen )
#endif
{
   #ifdef S4MFOX
      return memo4fileReadPart( f4memo, memoId, ptrPtr, ptrLen, 0L, UINT_MAX -100, mType ) ;
   #else
      FILE4LONG pos ;
      CODE4 *c4 ;
      #ifdef S4MNDX
         unsigned int amtRead, lenRead, loop ;
         char *tPtr ;
         FILE4LONG p2 ;
      #else
         MEMO4BLOCK  memoBlock ;
         unsigned finalLen ;
      #endif

      c4 = f4memo->file.codeBase ;

      #ifdef S4MNDX
         if ( memoId <= 0L )
         {
            if ( *ptrPtr != &f4memoNullChar )
               u4free( *ptrPtr ) ;
            *ptrPtr = 0 ;
            *ptrLen = 0 ;
            return 0 ;
         }

         file4longAssign( pos, memoId * f4memo->blockSize, 0 ) ;

         amtRead = 0 ;

         if ( c4->memoUseBuffer == 0 )
         {
            c4->memoUseBuffer = (char*)u4allocEr( c4, MEMO4SIZE ) ;
            if ( c4->memoUseBuffer == 0 )
               return e4memory ;
         }

         for( amtRead = 0 ;; )
         {
            file4longAssign( p2, file4longGetLo( pos ) + amtRead, 0 ) ;
            lenRead = file4readInternal( &f4memo->file, p2, c4->memoUseBuffer, MEMO4SIZE ) ;
            if ( lenRead <= 0 )
               return -1 ;

            for ( loop = 0 ; lenRead > 0 ; loop++, lenRead-- )
            {
               if ( c4->memoUseBuffer[loop] == 0x1A ) /* if done */
               {
                  if ( loop + amtRead > 0 )
                  {
                     if ( *ptrLen < amtRead + loop )
                     {
                        tPtr = (char *)u4allocEr( c4, amtRead + loop + 1 ) ;
                        if ( tPtr == 0 )
                           return e4memory ;
                        memcpy( tPtr, *ptrPtr, (int)amtRead ) ;
                        if ( *ptrPtr != &f4memoNullChar )
                           u4free( *ptrPtr ) ;
                        *ptrPtr = tPtr ;
                     }
                     *ptrLen = amtRead + loop ;
                     memcpy( *ptrPtr + amtRead, c4->memoUseBuffer, loop ) ;
                     (*ptrPtr)[amtRead+loop] = 0 ;
                     return 0 ;
                  }
                  else
                  {
                     tPtr = 0 ;
                     if ( *ptrPtr != &f4memoNullChar )
                        u4free( *ptrPtr ) ;
                     *ptrPtr = 0 ;
                     *ptrLen = 0 ;
                  }
                  return 0 ;
               }
            }

            lenRead = loop ;

            if ( *ptrLen < amtRead + lenRead )
            {
               tPtr = (char *)u4allocEr( c4, amtRead + lenRead + 1 ) ;
               if ( tPtr == 0 )
                  return e4memory ;
               if ( *ptrLen > 0 )
               {
                  memcpy( tPtr, *ptrPtr, *ptrLen ) ;
                  u4free( *ptrPtr ) ;
               }
               *ptrPtr = tPtr ;
               *ptrLen = (unsigned)( amtRead + lenRead ) ;
            }
            memcpy( *ptrPtr + amtRead, c4->memoUseBuffer, lenRead ) ;

            amtRead += lenRead ;
         }
      #else
         if ( memoId <= 0L )
         {
            *ptrLen = 0 ;
            return 0 ;
         }

         file4longAssign( pos, memoId * f4memo->blockSize, 0 ) ;
         if ( file4readAllInternal( &f4memo->file, pos, &memoBlock, sizeof( MEMO4BLOCK ) ) < 0)
            return -1 ;

         #ifdef WORDS_BIGENDIAN
            memoBlock.startPos = x4reverseShort( (void *)&memoBlock.startPos ) ;
            memoBlock.numChars = x4reverseLong( (void *)&memoBlock.numChars ) ;
         #endif

         if ( memoBlock.minusOne != -1 )  /* must be an invalid entry, so return an empty entry */
         {
            *ptrLen = 0 ;
            return 0 ;
         }
         else
         {
            if ( memoBlock.numChars >= UINT_MAX )
               return error4( c4, e4info, E95210 ) ;

            finalLen = (unsigned)memoBlock.numChars - 2 * ( sizeof(short) ) - ( sizeof(S4LONG) ) ;
            if ( finalLen > *ptrLen )
            {
               if ( *ptrLen > 0 )
                  u4free( *ptrPtr ) ;
               *ptrPtr = (char *)u4allocEr( c4, finalLen + 1 ) ;
               if ( *ptrPtr == 0 )
                  return e4memory ;
            }
            *ptrLen = finalLen ;

            file4longAdd( &pos, memoBlock.startPos ) ;
            return file4readAllInternal( &f4memo->file, pos, *ptrPtr, finalLen ) ;
         }
      #endif
   #endif
}

#ifndef S4OFF_WRITE
#ifdef S4MFOX
/* Writes partial data to a memo record.
   Usage rules:
      Must call this function with an offset == 0 to write 1st part of block
      before any additional writing.  In addition, the memoLen must be
      accurately set during the first call in order to reserve the correct
      amount of memo space ahead of time.  Later calls just fill in data to
      the reserved disk space.
      lenWrite is the amount of data to write, offset is the number of
      bytes from the beginning of the memo in which to write the data
      Secondary calls to this function assume that everything has been
      previously set up, and merely performs a file write to the reserved
      space.  The space is not checked to see whether or not it actually
      is in the bounds specified, so use with care.
*/
int memo4fileWritePart( MEMO4FILE *f4memo, long *memoIdPtr, const char *ptr, const long memoLen, const long offset, const unsigned lenWrite, const long type )
{
   int strNumBlocks ;
   #ifndef S4OFF_MULTI
      #ifndef N4OTHER
         int rc, lockCond ;
      #endif
   #endif
   MEMO4BLOCK oldMemoBlock ;
   MEMO4HEADER mh ;
   unsigned lenRead ;
   long blockNo ;
   unsigned nEntryBlks = 0 ;
   FILE4LONG pos ;

   #ifdef E4PARM_LOW
      if ( memoIdPtr == 0 )
         return error4( 0, e4parmNull, E95208 ) ;
      if ( f4memo->file.hand == INVALID4HANDLE ) /* file closed! */
         return error4( 0, e4parm, E95208 ) ;
   #endif

   if ( offset == 0 )   /* must do the set-up work */
   {
      if ( memoLen == 0 )
      {
         *memoIdPtr = 0L ;
         return 0 ;
      }

      #ifdef E4MISC
         if ( f4memo->blockSize <= 1 )
            return error4( f4memo->data->c4, e4info, E85202 ) ;
      #endif

      strNumBlocks = (int) ((memoLen + sizeof(MEMO4BLOCK) + f4memo->blockSize-1) / f4memo->blockSize) ;
      if ( *memoIdPtr <= 0L )
         blockNo = 0L ;
      else
      {
         blockNo = *memoIdPtr ;
         file4longAssign( pos, blockNo * f4memo->blockSize, 0 ) ;

         file4readAllInternal( &f4memo->file, pos, (char *)&oldMemoBlock, sizeof(oldMemoBlock) ) ;

         #ifndef WORDS_BIGENDIAN
            oldMemoBlock.type = x4reverseLong( (void *)&oldMemoBlock.type ) ;
            oldMemoBlock.numChars = x4reverseLong( (void *)&oldMemoBlock.numChars ) ;
         #endif
         nEntryBlks = (unsigned) ((oldMemoBlock.numChars + f4memo->blockSize-1)/ f4memo->blockSize ) ;
      }
      if ( nEntryBlks >= ((unsigned)strNumBlocks) && blockNo )  /* write to existing position */
         *memoIdPtr = blockNo ;
      else  /* read in header record */
      {
         #ifndef S4OFF_MULTI
            #ifndef N4OTHER
               lockCond = f4memo->data->memoFile.fileLock ;
               rc = memo4fileLock( &f4memo->data->memoFile ) ;
               if ( rc )
                  return rc ;
            #endif
         #endif

         file4longAssign( pos, 0, 0 ) ;
         lenRead = file4readInternal( &f4memo->file, pos, &mh, sizeof( mh ) ) ;
         #ifndef WORDS_BIGENDIAN
            mh.nextBlock = x4reverseLong( (void *)&mh.nextBlock ) ;
            /* block size isn't needed */
         #endif

         if ( error4code( f4memo->data->c4 ) < 0 )
         {
            #ifndef S4OFF_MULTI
            #ifndef N4OTHER
               if ( !lockCond )
                  memo4fileUnlock( &f4memo->data->memoFile ) ;
            #endif
            #endif
            return -1 ;
         }

         if ( lenRead != sizeof( mh ) )
         {
            #ifndef S4OFF_MULTI
            #ifndef N4OTHER
               if ( !lockCond )
                  memo4fileUnlock( &f4memo->data->memoFile ) ;
            #endif
            #endif
            return file4readError( &f4memo->file, pos, sizeof( mh ), "memo4fileWritePart" ) ;
         }

         *memoIdPtr = mh.nextBlock ;
         mh.nextBlock = *memoIdPtr + strNumBlocks ;

         #ifndef WORDS_BIGENDIAN
            mh.nextBlock = x4reverseLong( (void *)&mh.nextBlock ) ;
         #endif

         file4writeInternal( &f4memo->file, pos, &mh, sizeof( mh ) ) ;

         #ifndef S4OFF_MULTI
         #ifndef N4OTHER
            if ( !lockCond )
               memo4fileUnlock( &f4memo->data->memoFile ) ;
         #endif
         #endif
      }
      if ( memo4fileDump( f4memo, *memoIdPtr, ptr, lenWrite, memoLen, type ) < 0 )
         return -1 ;
   }
   else
   {
      file4longAssign( pos, *memoIdPtr * f4memo->blockSize + offset + sizeof( oldMemoBlock ), 0 ) ;
      return file4writeInternal( &f4memo->file, pos, ptr, lenWrite ) ;
   }

   return 0 ;
}
#endif /* S4MFOX */

int memo4fileWrite( MEMO4FILE *f4memo, long *memoIdPtr, const char *ptr, const unsigned ptrLen )
{
   #ifdef S4MFOX
      return memo4fileWritePart( f4memo, memoIdPtr, ptr, (long)ptrLen, 0L, ptrLen, 1L ) ;
   #else
      int strNumBlocks ;
      FILE4LONG pos ;
      #ifndef S4MDX
         #ifndef S4OFF_MULTI
            #ifndef N4OTHER
               int rc, lockCond ;
            #endif
         #endif
      #endif
      #ifdef S4MNDX
         MEMO4HEADER mh ;
         long lenRead ;
         char buf[MEMO4SIZE] ;
         int readSize, i ;
      #else
         MEMO4BLOCK oldMemoBlock ;
         MEMO4CHAIN_ENTRY newEntry, cur, prev ;
         int strWritten ;
         long prevPrevEntry, prevPrevNum ;
         long fileLen, extraLen ;
      #endif

      #ifdef E4PARM_LOW
         if ( memoIdPtr == 0 )
            return error4( 0, e4parmNull, E95209 ) ;
      #endif

      #ifdef S4MNDX
         if ( ptrLen == 0 )
         {
            *memoIdPtr = 0L ;
            return 0 ;
         }

         strNumBlocks = (int)(((long)ptrLen + f4memo->blockSize-1) / f4memo->blockSize) ;

         if ( *memoIdPtr <= 0L )
            *memoIdPtr = 0L ;
         else    /* read in old record to see if new entry can fit */
         {
            readSize = 0 ;
            file4longAssign( pos, *memoIdPtr * f4memo->blockSize, 0 ) ;

            do
            {
               readSize += MEMO4SIZE ;

               lenRead = file4readInternal( &f4memo->file, pos, buf, MEMO4SIZE ) ;
               if ( lenRead <= 0 )
                  return file4readError( &f4memo->file, pos, MEMO4SIZE, "memo4fileWrite()" ) ;

               for ( i=0 ; ((long) i) < lenRead ; i++ )
                  if ( buf[i] == (char)0x1A )  break ;

               #ifdef E4MISC
                  if ( buf[i] != (char)0x1A && lenRead != MEMO4SIZE )
                     return error4( f4memo->file.codeBase, e4info, E85203 ) ;
               #endif

               file4longAssign( pos, file4longGetLo( pos ) + MEMO4SIZE, 0 ) ;
            } while ( i >= MEMO4SIZE && buf[i] != (char) 0x1A ) ;  /* Continue if Esc is not located */

            if ( ((unsigned)readSize) <= ptrLen )   /* there is not room */
               *memoIdPtr = 0 ;
         }

         if ( *memoIdPtr == 0 )   /* add entry at eof */
         {
            #ifndef S4OFF_MULTI
               #ifndef N4OTHER
                  lockCond = f4memo->data->memoFile.fileLock ;
                  rc = memo4fileLock( &f4memo->data->memoFile ) ;
                  if ( rc )
                     return rc ;
               #endif
            #endif

            file4longAssign( pos, 0, 0 ) ;
            lenRead = file4readInternal( &f4memo->file, pos, &mh, sizeof( mh ) ) ;
            #ifdef WORDS_BIGENDIAN
               mh.nextBlock = x4reverseLong( (void *)&mh.nextBlock ) ;
            #endif
            if ( error4code( f4memo->data->c4 ) < 0 )
            {
               #ifndef S4OFF_MULTI
                  #ifndef N4OTHER
                     if ( !lockCond )
                        memo4fileUnlock( &f4memo->data->memoFile ) ;
                  #endif
               #endif
               return -1 ;
            }

            if ( lenRead != sizeof( mh ) )
            {
               #ifndef S4OFF_MULTI
                  #ifndef N4OTHER
                     if ( !lockCond )
                        memo4fileUnlock( &f4memo->data->memoFile ) ;
                  #endif
               #endif
               file4longAssign( pos, 0, 0 ) ;
               return file4readError( &f4memo->file, pos, sizeof( mh ), "memo4fileWrite()" ) ;
            }

            *memoIdPtr = mh.nextBlock ;
            mh.nextBlock = *memoIdPtr + strNumBlocks ;
            #ifdef WORDS_BIGENDIAN
               mh.nextBlock = x4reverseLong( (void *)&mh.nextBlock ) ;
            #endif

            file4longAssign( pos, 0, 0 ) ;
            file4writeInternal( &f4memo->file, pos, &mh, sizeof( mh ) ) ;

            #ifndef S4OFF_MULTI
               #ifndef N4OTHER
                  if ( !lockCond )
                     memo4fileUnlock( &f4memo->data->memoFile ) ;
               #endif
            #endif

            #ifdef WORDS_BIGENDIAN
               mh.nextBlock = x4reverseLong( (void *)&mh.nextBlock ) ;
            #endif
         }


         if ( memo4fileDump( f4memo, *memoIdPtr, ptr, ptrLen, ptrLen ) < 0 )
            return -1 ;

         return 0 ;
      #else
         /* S4MMDX */
         memset( (void *)&newEntry, 0, sizeof(newEntry) ) ;
         newEntry.blockNo = *memoIdPtr ;

         strWritten = 0 ;
         if ( ptrLen == 0 )
         {
            strWritten = 1 ;
            *memoIdPtr = 0 ;
         }

         /* Initialize information about the old memo entry */
         if ( newEntry.blockNo <= 0L )
         {
            if ( strWritten )
            {
               *memoIdPtr = 0L ;
               return 0 ;
            }
            newEntry.num = 0 ;
         }
         else
         {
            file4longAssign( pos, newEntry.blockNo * f4memo->blockSize, 0 ) ;

            file4readAllInternal( &f4memo->file, pos, (char *)&oldMemoBlock, sizeof(oldMemoBlock) ) ;
            #ifdef WORDS_BIGENDIAN
               oldMemoBlock.startPos = x4reverseShort( (void *)&oldMemoBlock.startPos ) ;
               oldMemoBlock.numChars = x4reverseLong( (void *)&oldMemoBlock.numChars ) ;
            #endif

            newEntry.num = (unsigned)(((long)oldMemoBlock.numChars + (long)f4memo->blockSize-1)/ f4memo->blockSize ) ;
         }

         strNumBlocks = (int)(((long)ptrLen+2*(sizeof(short))+(sizeof(S4LONG))+ f4memo->blockSize-1) / f4memo->blockSize ) ;

         if ( newEntry.num >= strNumBlocks  &&  !strWritten )
         {
            *memoIdPtr = newEntry.blockNo + newEntry.num - strNumBlocks ;
            if ( memo4fileDump( f4memo, *memoIdPtr, ptr, ptrLen, ptrLen ) < 0 )
               return -1 ;

            strWritten = 1 ;
            if ( newEntry.num == strNumBlocks )
               return 0 ;

            newEntry.num -= strNumBlocks ;
         }

         /* Initialize 'chain' */
         memset( (void *)&cur, 0, sizeof(cur) ) ;
         memset( (void *)&prev, 0, sizeof(prev) ) ;

         for(;;)
         {
            if ( error4code( f4memo->data->c4 ) < 0 )
               return -1 ;

            memo4fileChainFlush( f4memo, &prev ) ;
            prevPrevEntry = prev.blockNo ;
            prevPrevNum = prev.num ;

            memcpy( (void *)&prev, (void *)&cur, sizeof(prev) ) ;

            if ( newEntry.blockNo > 0  &&  prev.next > newEntry.blockNo )
            {
               /* See if the new entry fits in here */
               memcpy( (void *)&cur, (void *)&newEntry, sizeof(cur) ) ;
               newEntry.blockNo = 0 ;
               cur.next  = prev.next ;
               prev.next = cur.blockNo ;
               cur.toDisk = prev.toDisk = 1 ;
            }
            else
               memo4fileChainSkip( f4memo, &cur ) ;

            /* See if the entries can be combined. */
            if ( prev.blockNo + prev.num == cur.blockNo && prev.num )
            {
               /* 'cur' becomes the combined groups. */
               prev.toDisk = 0 ;
               cur.toDisk  = 1 ;

               cur.blockNo = prev.blockNo ;
               if ( cur.num >= 0 )
                  cur.num  += prev.num ;
               prev.blockNo = prevPrevEntry ;
               prev.num = prevPrevNum ;
            }

            if ( strWritten )
            {
               if ( newEntry.blockNo == 0 )
               {
                  memo4fileChainFlush( f4memo, &prev ) ;
                  memo4fileChainFlush( f4memo, &cur ) ;
                  return 0 ;
               }
            }
            else  /* 'str' is not yet written, try the current entry */
            {
               if ( cur.next == -1 )  /* End of file */
                  cur.num = strNumBlocks ;

               if ( cur.num >= strNumBlocks )
               {
                  cur.num -= strNumBlocks ;
                  *memoIdPtr = cur.blockNo + cur.num ;
                  memo4fileDump( f4memo, *memoIdPtr, ptr, ptrLen, ptrLen ) ;
                  if ( cur.next == -1 ) /* if end of file */
                  {
                     /* For dBASE IV compatibility */
                     fileLen  = file4longGetLo( file4lenLow( &f4memo->file ) ) ;
                     extraLen = f4memo->blockSize -  fileLen % f4memo->blockSize ;
                     if ( extraLen != f4memo->blockSize )
                     {
                        file4longAssign( pos, fileLen + extraLen, 0 ) ;
                        file4lenSetLow( &f4memo->file, pos ) ;
                     }
                  }

                  strWritten = 1 ;

                  if ( cur.num == 0 )
                  {
                     if ( cur.next == -1 ) /* End of file */
                        prev.next = cur.blockNo + strNumBlocks ;
                     else
                        prev.next = cur.next ;
                     prev.toDisk = 1 ;
                     cur.toDisk = 0 ;
                  }
                  else
                     cur.toDisk = 1 ;
               }
            }
         }
      #endif
   #endif
}
#endif /* S4OFF_WRITE */
#endif /* S4MEMO_OFF */

