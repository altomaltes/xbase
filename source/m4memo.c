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

/* m4memo.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved.  */

#include "d4all.h"


#ifndef S4OFF_MEMO
#ifdef S4MFOX
long memo4lenPart( MEMO4FILE *f4memo, long memoId )
{
   FILE4LONG pos ;
   int rc ;
   MEMO4BLOCK memoBlock ;

   if ( memoId <= 0L )
      return 0 ;

   file4longAssign( pos, memoId * f4memo->blockSize, 0 ) ;
   rc = file4readAllInternal( &f4memo->file, pos, &memoBlock, sizeof( MEMO4BLOCK ) ) ;
   if ( rc < 0 )
      return error4stack( f4memo->file.codeBase, rc, E95204 ) ;
   #ifndef S4BYTE_SWAP
      return x4reverseLong( (void *)&memoBlock.numChars ) ;
   #else
      return memoBlock.numChars ;
   #endif
}
#endif

#ifndef S4OFF_WRITE
int S4FUNCTION d4memoCompress( DATA4 *data )
{
   CODE4 *c4 ;
   int rc ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E95201 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E95201 ) ;
   #endif

   c4 = data->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   if ( data->dataFile->nFieldsMemo == 0 )
      return 0 ;

   rc = d4update( data ) ;
   if ( rc )
      return rc ;

   #ifndef S4OFF_MULTI
/*
      #ifdef S4SERVER
         rc = dfile4lockFile( data->dataFile, data4clientId( data ), data4serverId( data ) ) ;
      #else
*/
         rc = d4lockFile( data ) ;
/*
      #endif
*/
      if ( rc )
         return rc ;
   #endif

   #ifndef S4OFF_TRAN
      if ( code4transEnabled( c4 ) )
         if ( tran4active( c4, data ) != 0 )
            return error4( c4, e4transViolation, E81502 ) ;
   #endif

   return dfile4memoCompress( data->dataFile, data ) ;
}

int dfile4memoCompress( DATA4FILE *data, DATA4 *d4 )
{
   char *rdBuf, *wrBuf, *ptr ;
   FILE4SEQ_READ rd ;
   FILE4SEQ_WRITE wr ;
   MEMO4FILE newFile ;
   CODE4 *c4 ;
   unsigned int bufSize, ptrLen, ptrMax, saveFlag ;
   long curCount, iRec, newId ;
   int  rc, i ;
   FILE4LONG fLong ;
   FIELD4 *field ;
   #ifdef S4MFOX
      long memoLen, memoType ;
      unsigned long pos ;

      memoType = 1 ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E95201 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E95201 ) ;
   #endif

   c4 = data->c4 ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   if ( data->memoFile.file.hand == INVALID4HANDLE )
      return 0 ;

   #ifndef S4OFF_MULTI
      #ifndef N4OTHER
         #ifdef S4SERVER
            rc = dfile4lockMemo( data ) ;
         #else
            rc = dfile4lockMemo( data ) ;
         #endif
         if ( rc )
            return rc ;
      #endif
   #endif

   saveFlag = c4->memSizeMemo ;
   c4->memSizeMemo = (unsigned int)data->memoFile.blockSize ;

   rc = memo4fileCreate( &newFile, c4, data, 0 ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E91102 ) ;

   c4->memSizeMemo = saveFlag ;
   newFile.blockSize = data->memoFile.blockSize ;

   rdBuf = wrBuf = 0 ;
   bufSize = c4->memSizeBuffer ;

   for (; bufSize > data->recWidth; bufSize -= 0x800 )
   {
      rdBuf = (char *)u4allocFree( c4, (long)bufSize ) ;
      if ( rdBuf == 0 )
         continue ;

      wrBuf = (char *)u4allocFree( c4, (long)bufSize ) ;
      if ( wrBuf )
         break ;

      u4free( rdBuf ) ;
      rdBuf = 0 ;
   }

   #ifdef S4ADVANCE_READ
      file4seqReadInitDo( &rd, &data->file, dfile4recordPosition( data, 1L ), rdBuf, bufSize, 1 ) ;
   #else
      file4seqReadInitDo( &rd, &data->file, dfile4recordPosition( data, 1L ), rdBuf, bufSize, 0 ) ;
   #endif
   file4seqWriteInitLow( &wr, &data->file, dfile4recordPosition( data, 1L ), wrBuf, bufSize ) ;

   curCount = dfile4recCount( data, data4serverId( d4 )  ) ;

   ptr = 0 ;
   ptrLen = ptrMax = 0 ;

   for ( iRec= 1L ; iRec <= curCount && rc == 0 ; iRec++ )
   {
      if ( file4seqReadAll( &rd, d4->record, data->recWidth ) < 0 )
         break ;

      for ( i = 0 ; i < d4->dataFile->nFieldsMemo ; i++ )
      {
         field = d4->fieldsMemo[i].field ;

         #ifdef S4FOX
            pos = 0L ;

            if ( f4null( field ) == 1 )
               memoLen = 0 ;
            else
               memoLen = memo4lenPart( &data->memoFile, f4long( field ) ) ;


            if ( memoLen > 0L )
            {
               newId = 0L ;
               do
               {
                  #ifdef E4ANALYZE
                     if ( pos > (unsigned long)memoLen )
                     {
                        rc = error4( c4, e4memoCorrupt, E91102 ) ;
                        break ;
                     }
                  #endif

                  ptrLen = ptrMax ;

                  if ( memo4fileReadPart( &data->memoFile, f4long( field ), &ptr, &ptrLen, pos, UINT_MAX - 100, &memoType ) < 0 )
                  {
                     rc = -1 ;
                     break ;
                  }

                  if ( ptrLen > ptrMax )
                     ptrMax = ptrLen ;

                  if ( memo4fileWritePart( &newFile, &newId, ptr, memoLen, pos, ptrLen, memoType ) < 0 )
                  {
                     rc = -1 ;
                     break ;
                  }
                  pos += ptrLen ;
               } while( pos != (unsigned long)memoLen ) ;
               f4assignLong( field, newId ) ;
            }
            else
               f4assignLong( field, 0L ) ;
         #else
            ptrLen = ptrMax ;
            if ( memo4fileRead( &data->memoFile, f4long(field), &ptr, &ptrLen ) < 0 )
            {
               rc = -1 ;
               break ;
            }

            if ( ptrLen > ptrMax )
               ptrMax = ptrLen ;
            else
               if ( ptrLen == 0 && ptr == 0 )
                  ptrMax = 0 ;

            newId = 0L ;
            if ( memo4fileWrite( &newFile, &newId, ptr, ptrLen ) < 0 )
            {
               rc = -1 ;
               break ;
            }

            c4ltoa45( newId, f4ptr(field), - ((int) field->len) ) ;
         #endif
      }
      file4seqWrite( &wr, d4->record, data->recWidth ) ;
   }

   if ( rc < 0 )
      file4close( &newFile.file ) ;  /* error occurred */
   else
      file4seqWriteFlush(&wr) ;

   #ifdef S4ADVANCE_READ
      file4seqReadInitUndo( &rd ) ;
   #endif

   u4free( ptr ) ;
   u4free( rdBuf ) ;
   u4free( wrBuf ) ;

   if ( rc == 0 )
   {
      rc = file4replace( &data->memoFile.file, &newFile.file ) ;
      if ( rc == 0 )
         #ifdef S4MMDX
            if ( file4longGetLo( file4lenLow( &data->memoFile.file ) ) < (unsigned long)data->memoFile.blockSize )
            {
               file4longAssign( fLong, data->memoFile.blockSize, 0 ) ;
               rc = file4lenSetLow( &data->memoFile.file, fLong ) ;
            }
         #else
            if ( file4longGetLo( file4lenLow( &data->memoFile.file ) ) < 512 )
            {
               file4longAssign( fLong, 512, 0 ) ;
               rc = file4lenSetLow( &data->memoFile.file, fLong ) ;
            }
         #endif
   }

   return rc ;
}
#endif /* S4OFF_WRITE */

#ifndef S4MFOX
#ifndef S4MNDX

int memo4fileChainFlush( MEMO4FILE *f4memo, MEMO4CHAIN_ENTRY *chain )
{
   #ifdef S4BYTE_SWAP
      MEMO4CHAIN_ENTRY swap ;
   #endif
   FILE4LONG pos ;

   if ( chain->toDisk )
   {
      chain->toDisk = 0 ;
      file4longAssign( pos, chain->blockNo * f4memo->blockSize, 0 ) ;
      #ifdef S4BYTE_SWAP
         memcpy( (void *)&swap, (void *)chain, sizeof( MEMO4CHAIN_ENTRY ) ) ;
         swap.next = x4reverseLong( (void *)&swap.next ) ;
         swap.num = x4reverseLong( (void *)&swap.num ) ;

         return file4writeInternal( &f4memo->file, pos, &swap, 2 * sizeof(S4LONG) ) ;
      #else
         return file4writeInternal( &f4memo->file, pos, chain, 2 * sizeof(S4LONG) ) ;
      #endif
   }
   return 0 ;
}

int memo4fileChainSkip( MEMO4FILE *f4memo, MEMO4CHAIN_ENTRY *chain )
{
   unsigned lenRead ;
   FILE4LONG pos ;

   chain->toDisk = 0 ;
   chain->blockNo = chain->next ;

   file4longAssign( pos, chain->next * f4memo->blockSize, 0 ) ;

   if ( chain->next < 0 )
      lenRead = 0 ;
   else
   {
      lenRead = file4readInternal( &f4memo->file, pos, chain, sizeof( chain->next ) + sizeof( chain->num ) ) ;
      #ifdef S4BYTE_SWAP
         chain->next = x4reverseLong( (void *)&chain->next ) ;
         chain->num = x4reverseLong( (void *)&chain->num ) ;
      #endif
   }
   if ( error4code( f4memo->data->c4 ) < 0 )
      return -1 ;
   if ( lenRead == 0 )
   {
      chain->num = -1 ;
      chain->next = -1 ;
      return 0 ;
   }
   if ( lenRead != sizeof( chain->next ) + sizeof( chain->num ) )
      return file4readError( &f4memo->file, pos, sizeof(chain->next)+sizeof(chain->num), "memo4fileChainSkip" ) ;
   return 0 ;
}
#endif
#endif

#endif  /* S4OFF_MEMO */

/* Make the memo file entries current */
#ifndef S4OFF_MEMO
#ifndef S4OFF_MULTI
int d4validateMemoIds( DATA4 *data )
{
   int i, rc ;
   char *fromPtr ;

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E95203 ) ;
   #endif

   if ( data->memoValidated )
      return 0 ;

   if ( data->recNum > 0 )
   {
      #ifdef S4SERVER
         rc = dfile4lock( data->dataFile, data4clientId( data ), data4serverId( data ), data->recNum ) ;
      #else
         rc = d4lock( data, data->recNum ) ;
      #endif
      if ( rc )
         return rc ;
   }

   rc = d4readOld( data, data->recNum ) ;
   if ( rc < 0 )
      return error4stack( data->codeBase, rc, E95203 ) ;

   if ( data->recordChanged == 0 )   /* if the record has changed, leave intact */
      for ( i = 0 ; i < data->dataFile->nFieldsMemo ; i++ )
      {
         if ( data->fieldsMemo[i].isChanged == 0 )
         {
            fromPtr = data->recordOld + data->fieldsMemo[i].field->offset ;
            memcpy( f4ptr( data->fieldsMemo[i].field ), fromPtr, f4len( data->fieldsMemo[i].field ) ) ;  /* need f4len() because S4FOX 3.0 has 4 byte memos, not 10 byte */
         }
      }

   data->memoValidated = 1 ;
   return 0 ;
}
#endif  /* S4OFF_MULTI */
#endif  /* S4OFF_MEMO */
