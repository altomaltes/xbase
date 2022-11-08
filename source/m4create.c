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

/* m4create.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved.  */

#include "d4all.h"

#ifndef S4MEMO_OFF
#ifndef S4OFF_WRITE

/* if name is null, a temporary memo file is created, if d4 is null name must exist for create to work */
int memo4fileCreate( MEMO4FILE *m4file, CODE4 *c4, DATA4FILE *d4, const char *name )
{
   MEMO4HEADER *headerPtr ;
   char buf[258] ;
   int i, rc, saveFlag, safety, pos ;
   FILE4LONG fPos ;
   #ifdef S4WINCE
      SYSTEMTIME st ;
      WORD t ;
   #else
      time_t t ;
   #endif
   #ifndef S4MFOX
      #ifndef S4MNDX
         char memoName[9] ;
      #endif
   #endif
   #ifdef S4MACINTOSH
      long macDirTemp ;
      int macVolTemp ;
   #endif

   #ifdef E4PARM_LOW
      if ( c4 == 0 )
         return error4( c4, e4parmNull, E95207 ) ;
      if ( d4 == 0 && name == 0 )
         return error4( c4, e4parmNull, E95207 ) ;
   #endif

   m4file->data = d4 ;

   #ifndef S4MNDX
      #ifdef S4MFOX
         if ( c4->memSizeMemo < 33 )
            c4->memSizeMemo = 33 ;
      #else
         if ( c4->memSizeMemo % 512 != 0 || c4->memSizeMemo == 0 )
            c4->memSizeMemo = ((c4->memSizeMemo / 512) + 1) * 512 ;
      #endif

      #ifdef E4MISC
         if ( sizeof( MEMO4HEADER ) > c4->memSizeMemo )
            return error4( c4, e4memoCreate, E95207 ) ;

         #ifdef S4MFOX
            if ( c4->memSizeMemo > 512 * 32 )
         #else
            if ( c4->memSizeMemo > 512 * 63 )
         #endif
               return error4( 0, e4memoCreate, E85201 ) ;
      #endif
   #endif

   headerPtr = (MEMO4HEADER *)u4allocEr( c4, (long)sizeof( MEMO4HEADER ) ) ;
   if ( headerPtr == 0 )
      return -1 ;

   #ifdef S4MFOX
      m4file->blockSize = (short)c4->memSizeMemo ;
      if ( m4file->blockSize > 512 )
         headerPtr->nextBlock = 0x01000000 ;
      else
      {
         headerPtr->nextBlock = B4BLOCK_SIZE/m4file->blockSize ;
         #ifndef S4BYTE_SWAP
            headerPtr->nextBlock = x4reverseLong( (void *)&headerPtr->nextBlock) ;
         #endif
      }
      #ifndef S4BYTE_SWAP
         headerPtr->blockSize = x4reverseShort( (void *)&m4file->blockSize ) ;
      #else
         headerPtr->blockSize = m4file->blockSize ;
      #endif
   #else
      #ifdef S4MNDX
         m4file->blockSize = MEMO4SIZE ;
         headerPtr->nextBlock = 1 ;
      #else
         headerPtr->nextBlock = 1 ;
         headerPtr->x102 = 0x102 ;
         m4file->blockSize = headerPtr->blockSize = (short)c4->memSizeMemo ;
      #endif
   #endif

   if ( name == 0 )
   {
      saveFlag = c4getErrCreate( c4 ) ;
      safety = c4->safety ;
      c4setErrCreate( c4, 0 ) ;
      c4->safety = 1 ;   /* AS 03/21/97 safety should be on to avoid file overwrite, not false! */

      /* use the d4 path to ensure the file gets created in the correct location */
      #ifndef S4MACINTOSH
         pos = u4namePath( buf, sizeof( buf ) - 14, d4->file.name ) ;
      #else
         pos = 0 ;
         macDirTemp = c4->macDir ;
         macVolTemp = c4->macVol ;
         c4->macDir = d4->file.macSpec.parID ;
         c4->macVol = d4->file.macSpec.vRefNum ;
      #endif
      strcpy( buf + pos, "TEMP" ) ;
      for ( i = 0 ;; )
      {
         if ( i >= 100 )
         {
            rc = error4( c4, e4create, E80605 ) ;
            break ;
         }
         i++ ;

         u4delayHundredth( 50 ) ;
         #ifndef S4WINCE
            time( &t );                    /* changed back from time from clock due to size and linking considerations*/
         #else
            GetLocalTime(&st) ;
            t = st.wMilliseconds ;
         #endif
         t %= 10000L ;

         c4ltoa45( t, buf + pos + 4, -4 ) ;
         strcpy( buf + pos + 8, ".tmp" ) ;
         rc = file4create( &m4file->file, c4, buf, 1 ) ;
         if ( rc <= 0 )
            break ;
      }
      #ifdef S4MACINTOSH
         c4->macDir = macDirTemp ;
         c4->macVol = macVolTemp ;
      #endif

      c4setErrCreate( c4, saveFlag ) ;
      c4->safety = safety ;

      if ( rc < 0 )
      {
         u4free( headerPtr ) ;
         return error4( 0, rc, E95207 ) ;
      }
   }
   else
   {
      u4ncpy( buf, name, sizeof( buf ) ) ;
      #ifndef S4MFOX
         #ifndef S4MNDX
            memset( memoName, 0, sizeof( memoName ) ) ;
            u4namePiece( memoName, sizeof(memoName), name, 0, 0 ) ;
            memcpy( headerPtr->fileName, memoName, 8 ) ;
         #endif
      #endif
      #ifdef S4MFOX
         #ifdef S4CASE_SEN
            u4nameExt( buf, sizeof(buf), "fpt", 1 ) ;
         #else
            u4nameExt( buf, sizeof(buf), "FPT", 1 ) ;
         #endif
      #else
         #ifdef S4CASE_SEN
            u4nameExt( buf, sizeof(buf), "dbt", 1 ) ;
         #else
            u4nameExt( buf, sizeof(buf), "DBT", 1 ) ;
         #endif
      #endif

      rc = file4create( &m4file->file, c4, buf, 1 ) ;
      if ( rc != 0 )
      {
         u4free( headerPtr ) ;
         return error4( c4, (short)rc, E85201 ) ;
      }
   }
   #ifdef S4BYTE_SWAP
      #ifndef S4MFOX
         headerPtr->nextBlock = x4reverseLong( (void *)&headerPtr->nextBlock ) ;
         #ifndef S4MNDX
            headerPtr->x102 = 0x201 ;
            headerPtr->blockSize = x4reverseShort( (void *)&headerPtr->blockSize ) ;
         #endif
      #endif
   #endif

   file4longAssign( fPos, 0, 0 ) ;
   rc = file4writeInternal( &m4file->file, fPos, headerPtr, sizeof(  MEMO4HEADER ) ) ;
   #ifdef S4MFOX
      file4longAssign( fPos, 512, 0 ) ;
      file4lenSetLow( &m4file->file, fPos ) ;
   #endif
   u4free( headerPtr ) ;
   return rc ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
#ifdef S4MFOX
   int memo4fileDump( MEMO4FILE *f4memo, const long memoId, const char *ptr, const unsigned len, const long memoLen, const long type )
#else
   int memo4fileDump( MEMO4FILE *f4memo, const long memoId, const char *ptr, const unsigned len, const long memoLen )
#endif
{
   long pos, finalLen, tp ;
   int rc, doLenSet ;
   FILE4LONG fLen ;

   #ifdef S4MNDX
      char oneA = 0x1A ;
   #else
      MEMO4BLOCK  memoBlock ;

      #ifdef S4MFOX
         #ifdef S4BYTE_SWAP
            memoBlock.type = type ;
            memoBlock.numChars = memoLen ;
         #else
            memoBlock.type = x4reverseLong( (void *)&type ) ;
            memoBlock.numChars = x4reverseLong( (void *)&memoLen ) ;
         #endif
      #else
         memoBlock.minusOne = -1 ;
         memoBlock.startPos = sizeof(S4LONG) + 2 * sizeof( short ) ;
         memoBlock.numChars = memoBlock.startPos + memoLen ;
         #ifdef S4BYTE_SWAP
            memoBlock.startPos = x4reverseShort( (void *)&memoBlock.startPos ) ;
            memoBlock.numChars = x4reverseLong( (void *)&memoBlock.numChars ) ;
         #endif
      #endif
   #endif

   pos = memoId * f4memo->blockSize ;

   doLenSet = ( file4longGetLo( file4lenLow( &f4memo->file ) ) < pos + len ) ;

   #ifndef S4MNDX
      file4longAssign( fLen, pos, 0 ) ;
      rc = file4writeInternal( &f4memo->file, fLen, &memoBlock, sizeof( MEMO4BLOCK ) ) ;
      if ( rc != 0 )
         return rc ;
      #ifdef S4MFOX
         pos += sizeof( MEMO4BLOCK) ;
      #else
         pos += sizeof(S4LONG) + 2 * sizeof( short ) ;
      #endif
   #endif

   #ifdef S4MNDX
      file4longAssign( fLen, pos, 0 ) ;
      rc = file4writeInternal( &f4memo->file, fLen, ptr, len ) ;
      if ( rc != 0 )
         return rc ;
      file4longAssign( fLen, pos + len, 0 ) ;
      rc = file4writeInternal( &f4memo->file, fLen, &oneA, 1 ) ;
   #else
      file4longAssign( fLen, pos, 0 ) ;
      rc = file4writeInternal( &f4memo->file, fLen, ptr, len ) ;
   #endif

   if ( rc < 0 )
      return rc ;

   if ( doLenSet )
   {
      finalLen = file4longGetLo( file4lenLow( &f4memo->file ) ) ;
      tp = finalLen / f4memo->blockSize ;
      if ( ( tp * f4memo->blockSize ) != finalLen )
      {
         #ifdef E4ANALYZE
            if ( (unsigned long)(tp+1)*f4memo->blockSize <= file4longGetLo( file4lenLow( &f4memo->file ) ) ||
                 (unsigned long)(tp+1)*f4memo->blockSize <= pos + len )
              return error4( f4memo->data->c4, e4info, E95207 ) ;
         #endif
         file4longAssign( fLen, (tp+1) * f4memo->blockSize, 0 ) ;
         file4lenSetLow( &f4memo->file, fLen ) ;
      }
   }

   return 0 ;
}

#endif  /* S4OFF_WRITE */
#endif  /* S4MEMO_OFF */
