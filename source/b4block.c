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

/* b4block.c forced rebuild mod */

#include "d4all.h"

short S4FUNCTION x4reverseShort( const void *val )
{
   unsigned char out[2] ;
   const unsigned char *atVal ;

   atVal = (const unsigned char *)val ;

   E4PARMLOW( val, E90439 ) ;

   out[0] = atVal[1] ;
   out[1] = atVal[0] ;

   return (*(short *)out ) ;
}

S4LONG S4FUNCTION x4reverseLong( const void *val )
{
   unsigned char out[4] ;

   unsigned const char *atVal ;
   atVal = (const unsigned char *)val ;

   E4PARMLOW( val, E90439 ) ;

   #ifdef S4BYTEORDER_2301
      out[0] = atVal[2] ;
      out[1] = atVal[3] ;
      out[2] = atVal[0] ;
      out[3] = atVal[1] ;
   #else
      out[0] = atVal[3] ;
      out[1] = atVal[2] ;
      out[2] = atVal[1] ;
      out[3] = atVal[0] ;
   #endif

   return *(S4LONG *)out ;
}

double S4FUNCTION x4reverseDouble( const void *val )
{
   char returnValue[8] ;
   char *from, *to ;

   to = returnValue ;
   from = ((char *)val)+8 ;
   *to++ = *--from ;
   *to++ = *--from ;
   *to++ = *--from ;
   *to++ = *--from ;
   *to++ = *--from ;
   *to++ = *--from ;
   *to++ = *--from ;
   *to = *--from ;
   return (*(double *)returnValue) ;
}


int b4calcBlanks( const unsigned char *keyVal, const int len, const unsigned char pChar )
{
   int a ;

   E4PARM_TEST( keyVal == 0 || len < 0, E90438 );

   for ( a = len ; a > 0; a-- )
      if ( keyVal[a-1] != pChar )
         return ( len - a ) ;
   return len ;  /* all blanks */
}

#ifndef S4INDEX_OFF

B4BLOCK *b4alloc( TAG4FILE *t4, const long fb )
{
   B4BLOCK *b4 ;
   #ifdef S4CLIPPER
      short offset ;
      int i ;
   #endif  /* S4CLIPPER */

   E4PARMLOW( t4, E90438 ) ;

   #ifdef S4CLIPPER
      b4 = (B4BLOCK *)mem4alloc2( t4->blockMemory, t4->codeBase ) ;
   #else
      b4 = (B4BLOCK *)mem4alloc2( t4->indexFile->blockMemory, t4->codeBase ) ;
   #endif
   if ( b4 == 0 )
   {
      error4( t4->codeBase, e4memory, E90438 ) ;
      return 0 ;
   }

   b4->tag = t4 ;
   b4->fileBlock = fb ;

   #ifdef S4CLIPPER
      offset = ( b4->tag->header.keysMax + 2 +
         (( b4->tag->header.keysMax / 2) * 2 != b4->tag->header.keysMax ) )
         * sizeof(short) ;
      for ( i = 0 ; i <= b4->tag->header.keysMax ; i++ )
         b4->pointers[i] = (short)(( b4->tag->header.groupLen * i )) + offset ;
      b4->data = (B4KEY_DATA *) ((char *)&b4->nKeys + b4->pointers[0]) ;  /* first entry */
   #endif  /* S4CLIPPER */

   #ifdef S4FOX
/*      b4->builtKey = (B4KEY_DATA *)u4allocEr( t4->codeBase, (long)sizeof(S4LONG) + t4->header.keyLen + 1 ) ; */
      b4->builtKey = (B4KEY_DATA *)mem4alloc2( t4->builtKeyMemory, t4->codeBase ) ;
      b4->builtOn = -1 ;
   #endif  /* S4FOX */

   return b4 ;
}

int b4free( B4BLOCK *b4 )
{
   E4PARMLOW( b4, E90438 ) ;

   #ifdef S4FOX
      #ifdef E4ANALYZE
         if ( b4->changed )
            return error4( b4->tag->codeBase, e4info, E90438 ) ;
      #endif
      mem4free( b4->tag->builtKeyMemory, b4->builtKey ) ;
   #endif /* S4FOX */

   #ifdef S4CLIPPER
      mem4free( b4->tag->blockMemory, b4 ) ;
   #else
      mem4free( b4->tag->indexFile->blockMemory, b4 ) ;
   #endif
   return 0 ;
}

#ifdef S4MDX

/* S4MDX */
#ifndef S4OFF_WRITE
int b4flush( B4BLOCK *b4 )
{
   int rc ;
   INDEX4FILE *i4 ;
   TAG4FILE *t4file ;
   FILE4LONG fPos ;
   #ifdef S4BYTE_SWAP
      char *swap, *swapPtr ;
      int i ;
      S4LONG longVal ;
      short shortVal ;
   #endif  /* S4BYTE_SWAP */

   E4PARMLOW( b4, E90438 );

   if ( b4->changed )
   {
      #ifdef E4INDEX_VERIFY
         if ( b4verify( b4 ) == -1 )
            error4describe( b4->tag->codeBase, e4index, E90438, b4->tag->alias, 0, 0 ) ;
      #endif
      t4file = b4->tag ;
      i4 = t4file->indexFile ;
      file4longAssign( fPos, b4->fileBlock*I4MULTIPLY, 0 ) ;

      #ifdef S4BYTE_SWAP
         swap = (char *)u4allocEr( t4file->codeBase, t4file->indexFile->header.blockRw ) ;
         if ( swap == 0 )
            return error4stack( t4file->codeBase, e4memory, E90438 ) ;

         memcpy( (void *)swap, (void *)&b4->nKeys, t4file->indexFile->header.blockRw ) ;

         /* position swapPtr at beginning of B4KEY's */
         swapPtr = swap ;
         swapPtr += 6 + sizeof(short) ;

         /* move through all B4KEY's to swap 'long' */
         for ( i = 0 ; i < (*(short *)swap) ; i++ )
         {
            longVal = x4reverseLong( (void *)swapPtr ) ;
            memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
            swapPtr += t4file->header.groupLen ;
         }

         /* mark lastPointer */
         if ( !b4leaf( b4 ) )
         {
            longVal = x4reverseLong( (void *)swapPtr ) ;
            memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
         }

         /* swap the numKeys value */
         shortVal = x4reverseShort( (void *)swap ) ;
         memcpy( swap, (void *) &shortVal, sizeof(short) ) ;

         #ifndef S4OFF_OPTIMIZE
            i4->readBlockTag = t4file ;
         #endif

         rc = file4writeInternal( &i4->file, fPos, swap, i4->header.blockRw ) ;
         #ifndef S4OFF_OPTIMIZE
            i4->readBlockTag = 0 ;
         #endif
         u4free( swap ) ;
      #else
         #ifndef S4OFF_OPTIMIZE
            i4->readBlockTag = t4file ;
         #endif
         rc = file4writeInternal( &i4->file, fPos, &b4->nKeys, i4->header.blockRw ) ;
         #ifndef S4OFF_OPTIMIZE
            i4->readBlockTag = 0 ;
         #endif
      #endif  /* S4BYTE_SWAP */

      if ( rc < 0 )
         return error4stack( t4file->codeBase, rc, E90438 ) ;
      b4->changed = 0 ;
   }
   return 0 ;
}
#endif /* S4OFF_WRITE */

/* S4MDX */
#ifndef S4INLINE
void b4goEof( B4BLOCK *b4 )
{
   E4PARMLOW( b4, E90438 ) ;

   b4->keyOn = b4numKeys( b4 ) ;
}
#endif /* S4INLINE */

/* S4MDX */
#ifndef S4OFF_WRITE
int b4insert( B4BLOCK *b4, const void *k, const long r )
{
   #ifdef S464BIT
      S4LONG tempR ;
   #endif
   int leftLen ;
   B4KEY_DATA *dataPtr, *nextPtr ;

   #ifdef E4PARM_LOW
      if ( b4 == 0 || k == 0 || r <= 0L )
         return error4( b4->tag->codeBase, e4parm, E90438 ) ;
   #endif

   dataPtr = b4key( b4, b4->keyOn ) ;
   nextPtr = b4key( b4, b4->keyOn+1 ) ;
   leftLen = b4->tag->indexFile->header.blockRw - ( b4->keyOn + 1 ) * b4->tag->header.groupLen - sizeof(short) - sizeof(char[6]) ;

   #ifdef E4ANALYZE
      if ( b4->keyOn < 0 || b4->keyOn > b4numKeys( b4 ) || leftLen < 0 )
         return error4( b4->tag->codeBase, e4info, E90438 ) ;
   #endif

   c4memmove( nextPtr, dataPtr, leftLen ) ;
   b4->nKeys++ ;
   memcpy( dataPtr->value, k, b4->tag->header.keyLen ) ;
   #ifdef S464BIT
      tempR = (S4LONG) r ;
      memcpy( (void *)&dataPtr->num, (void *)&tempR, sizeof(tempR) ) ;
   #else
      memcpy( (void *)&dataPtr->num, (void *)&r, sizeof(r) ) ;
   #endif
   b4->changed = 1 ;

   return 0 ;
}
#endif /* S4OFF_WRITE */

/* S4MDX */
/* based on the actual stored-data (from file) only, determine whether or
   not a leaf.  Used by file optimization routines */
int b4dataLeaf( void *data, TAG4FILE *tag )
{
   short nKeys ;
   char *info ;

   E4PARMLOW( data && tag, E90438 ) ;

   nKeys = * ((short *)data) ;
   info = (char *)data + ( sizeof( short ) + 6 ) ;

   return ( ((B4KEY_DATA *)((char *)info + tag->header.groupLen * nKeys))->num == 0L ) ;
}

/* S4MDX */
#ifndef S4INLINE
B4KEY_DATA *b4key( const B4BLOCK *b4, const int iKey )
{
   E4PARMLOW( b4 && iKey, E90438 ) ;

   return (B4KEY_DATA *)((char *)&b4->info.num + b4->tag->header.groupLen * iKey) ;
}

/* S4MDX */
unsigned char *b4keyKey( B4BLOCK *b4, const int iKey )
{
   E4PARMLOW( b4 && iKey, E90438 ) ;

   return (unsigned char *)(((B4KEY_DATA *)((char *)&b4->info.num + b4->tag->header.groupLen * iKey ))->value ) ;
}

/* S4MDX */
int b4lastpos( const B4BLOCK *b4 )
{
   E4PARMLOW( b4, E90438 ) ;

   return ( ( b4leaf( b4 ) ) ? ( b4numKeys( b4 ) - 1 ) : ( b4numKeys( b4 ) ) ) ;
}

/* S4MDX */
int b4leaf( const B4BLOCK *b4 )
{
   E4PARMLOW( b4, E90438 ) ;


   return( b4key( b4, b4numKeys( b4 ) )->num == 0L ) ;
}

/* S4MDX */
long b4recNo( const B4BLOCK *b4, const int i )
{
   E4PARM_TEST(  b4 == 0 || i < 0, E90438 );

   return b4key( b4, i )->num ;
}
#endif /* S4INLINE */

/* S4MDX */
#ifndef S4OFF_WRITE
int b4remove( B4BLOCK *b4 )
{
   B4KEY_DATA *keyCur, *keyNext ;
   int leftLen ;

   E4PARMLOW( b4, E90438 ) ;


   keyCur = b4key( b4, b4->keyOn ) ;
   keyNext = b4key( b4, b4->keyOn + 1 ) ;

   leftLen = b4->tag->indexFile->header.blockRw - sizeof( b4->nKeys ) - sizeof( b4->dummy )
            - ( b4->keyOn + 1 ) * b4->tag->header.groupLen ;

   #ifdef E4ANALYZE
      if ( b4->keyOn < 0 || b4->keyOn > b4lastpos( b4 ) )
         return error4( b4->tag->codeBase, e4info, E90438 ) ;
   #endif

   if ( leftLen > 0 )
      c4memmove( keyCur, keyNext, leftLen ) ;

   b4->nKeys-- ;
   b4->changed = 1 ;

   if ( b4leaf( b4 ) )
      memset( b4keyKey( b4, b4numKeys( b4 ) ), 0, b4->tag->header.keyLen ) ;
   #ifdef E4ANALYZE
      else
         if ( b4numKeys( b4 ) < b4->tag->header.keysMax )
            memset( b4keyKey( b4, b4numKeys( b4 ) ), 0, b4->tag->header.keyLen ) ;
   #endif

   return 0 ;
}
#endif /* S4OFF_WRITE */

/* S4MDX */
int b4seek( B4BLOCK *b4, const char *searchValue, const int len )
{
   int rc, keyLower, keyUpper, saveRc, keyCur ;
   S4CMP_FUNCTION *cmp;

   E4PARM_TEST(  b4 == 0 || searchValue == 0 || len < 0, E90438 );

   /* keyCur must be between  keyLower and  keyUpper */
   keyLower = -1 ;
   keyUpper = b4numKeys( b4 ) ;

   cmp = b4->tag->cmp ;

   if ( keyUpper == 0 )
   {
      b4->keyOn = 0 ;
      return r4after ;
   }

   saveRc = 1 ;

   for( ;; )  /* Repeat until the key is found */
   {
      keyCur  = (keyUpper + keyLower) / 2  ;
      rc = (*cmp)( b4keyKey(b4,keyCur), searchValue, len ) ;

      if ( rc >= 0 )
      {
         keyUpper = keyCur ;
         saveRc = rc ;
      }
      else
         keyLower = keyCur ;

      if ( keyLower >= (keyUpper-1) )  /* then there is no exact match */
      {
         b4->keyOn =  keyUpper ;
         if ( saveRc )
            return r4after ;
         return 0 ;
      }
   }
}

/* S4MDX */
int b4skip( B4BLOCK *b4, const long n )
{
   int numLeft ;

   E4PARMLOW( b4, E90438 ) ;

   if ( n > 0 )
   {
      numLeft = b4numKeys( b4 ) - b4->keyOn ;
      if ( b4leaf( b4 ) )
         if ( numLeft != 0 )
            numLeft -- ;
   }
   else
      numLeft = -b4->keyOn ;

   if ( ( n <= 0L ) ? ( (long)numLeft <= n ) : ( (long)numLeft >= n) )
   {
      b4->keyOn = b4->keyOn + (int)n ;
      return (int)n ;
   }
   else
   {
      b4->keyOn = b4->keyOn + numLeft ;
      return numLeft ;
   }
}
#endif  /* S4MDX */

#ifdef S4FOX

/* S4FOX */
void b4leafInit( B4BLOCK *b4 )
{
   TAG4FILE *t4 ;
   int tLen ;
   unsigned int cLen, keyLen ;
   unsigned S4LONG ff, rLen ;

   E4PARMLOW( b4, E90438 ) ;

   t4 = b4->tag ;
   keyLen = (unsigned int)t4->header.keyLen ;
   ff = 0xFFFFFFFFL ;

   for ( cLen = 0 ; keyLen ; keyLen >>= 1, cLen++ ) ;

   b4->nodeHdr.trailCntLen = b4->nodeHdr.dupCntLen = (unsigned char)cLen ;

   b4->nodeHdr.trailByteCnt = (unsigned char)((unsigned char)0xFF >> ( 8 - ((cLen / 8) * 8 + cLen % 8))) ;
   b4->nodeHdr.dupByteCnt = b4->nodeHdr.trailByteCnt ;

   rLen = (unsigned long)dfile4recCount( b4->tag->indexFile->dataFile, -2L ) ;

   for ( cLen = 0 ; rLen ; rLen>>=1, cLen++ ) ;

   b4->nodeHdr.recNumLen = (unsigned char) (cLen + (( 8 - ( 2 * (unsigned int)b4->nodeHdr.trailCntLen % 8 )) % 8)) ;
   if ( b4->nodeHdr.recNumLen < 12 )
      b4->nodeHdr.recNumLen = 12 ;

   for( tLen = b4->nodeHdr.recNumLen + b4->nodeHdr.trailCntLen + b4->nodeHdr.dupCntLen ;
        (tLen / 8)*8 != tLen ; tLen++, b4->nodeHdr.recNumLen++ ) ;  /* make at an 8-bit offset */

   rLen = ff >> ( sizeof(S4LONG)*8 - b4->nodeHdr.recNumLen ) ;
   memcpy( (void *)&b4->nodeHdr.recNumMask[0], (void *)&rLen, sizeof(S4LONG) ) ;

   b4->nodeHdr.infoLen = (unsigned char)((unsigned int)(b4->nodeHdr.recNumLen + b4->nodeHdr.trailCntLen + b4->nodeHdr.dupCntLen) / 8) ;
   b4->nodeHdr.freeSpace = B4BLOCK_SIZE - sizeof( B4STD_HEADER ) - sizeof( B4NODE_HEADER ) ;
}

/* S4FOX */
int b4calcDups( const unsigned char *ptr1, const unsigned char *ptr2, const int len )
{
   int a ;

   E4PARM_TEST(  ptr1 == 0 || ptr2 == 0 || len < 0, E90438 );

   for ( a = 0 ; a < len; a++ )
      if ( ptr1[a] != ptr2[a] )
         return a ;
   return len ;  /* all duplicates */
}

/* S4FOX */
S4LONG x4recNo( const B4BLOCK *b4, const int numInBlock )
{
   #ifdef S4DATA_ALIGN
      unsigned S4LONG longPtr ;
      unsigned S4LONG longVal ;
   #else
      #ifdef S4DO_BYTEORDER
         unsigned S4LONG longTemp ;
      #else
         unsigned S4LONG *lPtr ;
      #endif
   #endif

   E4PARM_TEST( b4 == 0 || numInBlock < 0, E90439 ) ;

   #ifdef S4DATA_ALIGN
      memcpy( (void *)&longPtr , b4->data + numInBlock * b4->nodeHdr.infoLen , sizeof(S4LONG) ) ;
      #ifdef S4DO_BYTEORDER
         longPtr = (unsigned S4LONG)x4reverseLong( (void *)&longPtr ) ;
      #endif
      memcpy( (void *)&longVal , (void *)&b4->nodeHdr.recNumMask[0], sizeof(unsigned S4LONG) ) ;
      return ( longPtr & longVal ) ;
   #else
      #ifdef S4DO_BYTEORDER
         longTemp = *(unsigned S4LONG *)( b4->data + numInBlock * b4->nodeHdr.infoLen ) ;
         longTemp = (unsigned S4LONG)x4reverseLong( (void *)&longTemp ) ;
         return ( longTemp & *(unsigned S4LONG *)&b4->nodeHdr.recNumMask[0] ) ;
      #else
         lPtr = (unsigned S4LONG *)( b4->data + numInBlock * b4->nodeHdr.infoLen ) ;
         return ( *lPtr & *(unsigned S4LONG *)&b4->nodeHdr.recNumMask[0] ) ;
      #endif
   #endif
}

/* S4FOX */
int x4dupCnt( const B4BLOCK *b4, const int numInBlock )
{
   int pos ;
   #ifdef S4DATA_ALIGN
      unsigned S4LONG longPtr ;
   #else
      unsigned long *lPtr ;
      #ifdef S4DO_BYTEORDER
         unsigned long longVal ;
      #endif
   #endif

   E4PARM_TEST( b4 == 0 || numInBlock < 0, E90439 ) ;

   if ( b4->nodeHdr.infoLen > 4 )  /* > size of long, so must do careful shifting and copying */
   {
      #ifdef E4ANALYZE
         if ( b4->nodeHdr.recNumLen <= 16 )
            return error4( b4->tag->codeBase, e4info, E80401 ) ;
      #endif
      #ifdef S4DATA_ALIGN
         memcpy( (void *)&longPtr , b4->data + numInBlock * b4->nodeHdr.infoLen + 2, sizeof(S4LONG) ) ;
      #else
         lPtr = (unsigned long *)( b4->data + numInBlock * b4->nodeHdr.infoLen + 2 ) ;
      #endif
      pos = b4->nodeHdr.recNumLen - 16 ;
   }
   else
   {
      #ifdef S4DATA_ALIGN
         memcpy( (void *)&longPtr , b4->data + numInBlock * b4->nodeHdr.infoLen , sizeof(S4LONG) ) ;
      #else
         lPtr = (unsigned long *)( b4->data + numInBlock * b4->nodeHdr.infoLen ) ;
      #endif
      pos = b4->nodeHdr.recNumLen ;
   }

   #ifdef S4DATA_ALIGN
      #ifdef S4DO_BYTEORDER
         longPtr = x4reverseLong( (void *)&longPtr ) ;
      #endif
      return (int)( ( longPtr >> pos ) & b4->nodeHdr.dupByteCnt ) ;
   #else
      #ifdef S4DO_BYTEORDER
         longVal = *lPtr ;
         longVal = x4reverseLong( (void *)&longVal) ;
         return (int)( ( longVal >> pos ) & b4->nodeHdr.dupByteCnt ) ;
      #else
         return (int)( ( *lPtr >> pos ) & b4->nodeHdr.dupByteCnt ) ;
      #endif
   #endif
}

/* S4FOX */
int x4trailCnt( const B4BLOCK *b4, const int numInBlock )
{
   int pos ;
   #ifdef S4DATA_ALIGN
      unsigned S4LONG longPtr ;
   #else
      unsigned long *lPtr ;
      #ifdef S4DO_BYTEORDER
         unsigned long longVal ;
      #endif
   #endif

   E4PARM_TEST( b4 == 0 || numInBlock < 0, E90439 ) ;

   if ( b4->nodeHdr.infoLen > 4 )  /* > size of long, so must do careful shifting and copying */
   {
      #ifdef E4ANALYZE
         if ( b4->nodeHdr.recNumLen <= 16 )
            return error4( b4->tag->codeBase, e4info, E90438 ) ;
      #endif
      #ifdef S4DATA_ALIGN
         memcpy( (void *)&longPtr , b4->data + numInBlock * b4->nodeHdr.infoLen + 2, sizeof(S4LONG) ) ;
      #else
         lPtr = (unsigned long *)( b4->data + numInBlock * b4->nodeHdr.infoLen + 2 ) ;
      #endif
      pos = b4->nodeHdr.recNumLen - 16 + b4->nodeHdr.dupCntLen ;
   }
   else
   {
      #ifdef S4DATA_ALIGN
         memcpy( (void *)&longPtr , b4->data + numInBlock * b4->nodeHdr.infoLen , sizeof(S4LONG) ) ;
      #else
         lPtr = (unsigned long *)( b4->data + numInBlock * b4->nodeHdr.infoLen ) ;
      #endif
      pos = b4->nodeHdr.recNumLen + b4->nodeHdr.dupCntLen;
   }

   #ifdef S4DATA_ALIGN
      #ifdef S4DO_BYTEORDER
         longPtr = x4reverseLong( (void *)&longPtr ) ;
      #endif
      return (int)( ( longPtr >> pos ) & b4->nodeHdr.trailByteCnt ) ;
   #else
      #ifdef S4DO_BYTEORDER
         longVal = *lPtr ;
         longVal = x4reverseLong( (void *)&longVal) ;
         return (int)( ( longVal >> pos ) & b4->nodeHdr.trailByteCnt ) ;
      #endif
      return (int)( ( *lPtr >> pos ) & b4->nodeHdr.trailByteCnt ) ;
   #endif
}

/* S4FOX */
#ifndef S4OFF_WRITE

int x4putInfo( const B4NODE_HEADER *b4nodeHdr, void *buffer, const S4LONG rec, const int trail, const int dupCnt )
{
   int pos ;
   #ifdef S4DO_BYTEORDER
      #ifndef S4DATA_ALIGN
         S4LONG longTemp ;
      #endif
   #endif
   #ifdef S4DATA_ALIGN
      char unixBuf[6] ;
      char unixBuf2[4] ;
      S4LONG longVal, longTemp ;
      int doShift = 0 ;
   #else
      unsigned char *buf ;
      unsigned long *lPtr ;
   #endif

   E4PARM_TEST(  b4nodeHdr == 0 || buffer == 0 || rec < 0 || trail < 0 || dupCnt < 0, E90439 ) ;

   #ifdef S4DATA_ALIGN
      memset( unixBuf, 0, 6 ) ;
      memcpy( (void *)&longTemp , (void *)&b4nodeHdr->recNumMask[0], sizeof(S4LONG) ) ;
      longVal = rec & longTemp ;
      #ifdef S4DO_BYTEORDER
         longVal = x4reverseLong( (void *)&longVal ) ;
      #endif
      memcpy( unixBuf, (void *)&longVal, sizeof(S4LONG) ) ;
   #else
      buf = (unsigned char *) buffer ;
      lPtr = (unsigned long *)buf ;
      memset( buf, 0, 6 ) ;
      *lPtr = rec & *(long *)&b4nodeHdr->recNumMask[0] ;
      #ifdef S4DO_BYTEORDER
         *lPtr = x4reverseLong( (void *)lPtr ) ;
      #endif
   #endif

   if ( b4nodeHdr->infoLen > 4 )  /* > size of long, so must do careful shifting and copying */
   {
      #ifdef E4ANALYZE
         if ( b4nodeHdr->recNumLen <= 16 )
            return error4( 0, e4info, E80401 ) ;
      #endif
      #ifndef S4DATA_ALIGN
         lPtr = (unsigned long *)( buf + 2 ) ;  /* start at new pos */
      #else
         doShift = 1 ;
      #endif
      pos = b4nodeHdr->recNumLen - 16 ;
   }
   else
      pos = b4nodeHdr->recNumLen ;

   #ifdef S4DATA_ALIGN
      longVal = ((unsigned long)dupCnt) << pos ;
      pos += b4nodeHdr->dupCntLen ;
      longVal |= ((unsigned long)trail) << pos ;
      #ifdef S4DO_BYTEORDER
         longVal = x4reverseLong( (void *)&longVal ) ;
      #endif
      memcpy( unixBuf2, (void *)&longVal, sizeof(S4LONG) ) ;
      if (doShift)
      {
         unixBuf[2] |= unixBuf2[0] ;  /* must OR bytes 2 and 3 of 'buf' */
         unixBuf[3] |= unixBuf2[1] ;
         unixBuf[4] = unixBuf2[2] ;
         unixBuf[5] = unixBuf2[3] ;
      }
      else
      {
         unixBuf[0] |= unixBuf2[0] ;
         unixBuf[1] |= unixBuf2[1] ;
         unixBuf[2] |= unixBuf2[2] ;
         unixBuf[3] |= unixBuf2[3] ;
      }
      memcpy( (void *)buffer, unixBuf, 6 ) ;
   #else
      #ifdef S4DO_BYTEORDER
         longTemp = ((unsigned long)dupCnt) << pos ;
         pos += b4nodeHdr->dupCntLen ;
         longTemp |= ((unsigned long)trail) << pos ;
         longTemp = x4reverseLong( (void *)&longTemp ) ;
         *lPtr |= longTemp ;
      #else
         *lPtr |= ((unsigned long)dupCnt) << pos ;
         pos += b4nodeHdr->dupCntLen ;
         *lPtr |= ((unsigned long)trail) << pos ;
      #endif
   #endif

   return 0 ;
}

/* S4FOX */
int b4insertLeaf( B4BLOCK *b4, const void *vkeyData, const S4LONG rec )
{
   int leftBlanks, rightBlanks, leftDups, rightDups, leftLen, rightLen, reqdLen, movLen, oldRec, rc, oldRightDups, extraDups, saveDups ;
   unsigned char buffer[6], iLen ;
   unsigned char *keyData ;
   char *infoPos, a, b ;
   int keyLen, cLen ;
   unsigned int recLen ;
   unsigned S4LONG mask ;
   #ifdef S4DATA_ALIGN
      S4LONG *rec2;
      S4LONG *mask2;
   #endif

   E4PARM_TEST( b4 == 0 || vkeyData == 0 || rec < 0L, E90438 ) ;

   iLen = b4->nodeHdr.infoLen ;
   keyData = (unsigned char *)vkeyData ;
   keyLen = b4->tag->header.keyLen ;
   b4->builtOn = -1 ;

   /* 04/22/97 AS add if statement to reduce code, see if need to check mask first */
   #ifdef S4DATA_ALIGN
      rec2=(S4LONG *)u4alloc(sizeof(S4LONG)) ;
      memcpy(rec2, &rec, sizeof(S4LONG)) ;
      mask2=(S4LONG *)u4alloc(4) ;
      memcpy(mask2,(void *)&b4->nodeHdr.recNumMask[0], 4) ;

      if ((*rec2 & *mask2 ) != *rec2 )
   #else
      if ( (rec & (*((S4LONG *)(&b4->nodeHdr.recNumMask[0]))) ) != rec )
   #endif
   {
      /* if the record is > than the mask, must reset the block with new parameters: */
      b = sizeof(S4LONG) * 8 ;

      mask = 0x01L << (b-1) ;    /* set leftmost bit  */
      for( recLen = 0, a = 0 ; a < b ; a++ )
      {
         if ( rec & mask )
         {
            recLen = (unsigned int)((int)b - a) ;
            break ;
         }
         mask >>= 1 ;
      }

      while ( recLen > b4->nodeHdr.recNumLen )
      {
         oldRec = b4->keyOn ;
         saveDups = b4->curDupCnt ;
         rc = b4reindex( b4 ) ;
         if (rc )
            return rc ;
         iLen = b4->nodeHdr.infoLen ;
         b4top( b4 ) ;
         b4skip( b4, (long)oldRec ) ;
         b4->curDupCnt = saveDups ;
      }
   }

   /* AS 04/22/97 t4seek.c FoxPro 2.6 always has no blanks if filters exist (at least for character tags */
   if ( b4->tag->indexFile->dataFile->version != 0x30 && b4->tag->codeBase->compatibility == 26 && b4->tag->filter != 0 )
      leftBlanks = 0 ;
   else
      leftBlanks = b4calcBlanks( keyData, keyLen, b4->tag->pChar ) ;

   if ( b4numKeys( b4 ) == 0 )
   {
      if( b4->nodeHdr.freeSpace == 0 )  /* block needs initialization */
      {
         b4leafInit( b4 ) ;
         iLen = b4->nodeHdr.infoLen ;
      }
      leftDups = 0 ;
      reqdLen = keyLen - leftBlanks ;
      b4->keyOn = 0 ;
      b4->curPos = ((char *)&b4->header) + B4BLOCK_SIZE - reqdLen ;
      memcpy( b4->curPos, keyData, (unsigned int)reqdLen ) ;  /* key */
      x4putInfo( &b4->nodeHdr, buffer, rec, leftBlanks, 0 ) ;
      memcpy( b4->data, (void *)buffer, iLen ) ;
   }
   else
   {
      if ( b4->keyOn == b4numKeys( b4 ) )  /* at end */
      {
         leftDups = b4->curDupCnt ;
         reqdLen = keyLen - leftBlanks - b4->curDupCnt ;
         if ( (int)b4->nodeHdr.freeSpace < ( reqdLen + (int)iLen ) )  /* no room to add */
            return 1 ;
         b4->curPos -= reqdLen ;
         memcpy( b4->curPos, keyData + b4->curDupCnt, (unsigned int)reqdLen ) ;  /* key */
         x4putInfo( &b4->nodeHdr, buffer, rec, leftBlanks, leftDups ) ;
         memcpy( b4->data + b4->keyOn * iLen , (void *)buffer, iLen ) ;
      }
      else
      {
         rightBlanks = x4trailCnt( b4, b4->keyOn ) ;

         if ( b4->keyOn == 0 )   /* insert at top */
         {
            oldRightDups = 0 ;
            rightDups = b4calcDups( keyData, (unsigned char *)b4->curPos, keyLen - ( rightBlanks > leftBlanks ? rightBlanks : leftBlanks ) ) ;
            extraDups = rightDups ;
            leftDups = 0 ;
         }
         else /* insert in middle of block */
         {
            oldRightDups = x4dupCnt( b4, b4->keyOn) ;
            cLen = keyLen - ( rightBlanks > leftBlanks ? rightBlanks : leftBlanks ) - oldRightDups ;
            /* AS 02/20/98 t5cbug2.cpp fails if not coded here */
            if ( cLen < 0 )  /* happens when we have values less than blank but inserting blank characters */
            {
               /* modify the left-blanks to disenclude the right dups */
               leftBlanks += cLen ;
               extraDups = 0 ;
            }
            else
               extraDups = b4calcDups( keyData + oldRightDups, (unsigned char *)b4->curPos, cLen ) ;
            rightDups = oldRightDups + extraDups ;
            leftDups = b4->curDupCnt ;
         }

         #ifdef E4ANALYZE_ALL
            if ( b4->tag->header.typeCode & 0x01 )
               if ( leftDups == b4->tag->header.keyLen || rightDups == b4->tag->header.keyLen )
                  return error4( b4->tag->codeBase, e4info, E80402 ) ;
         #endif

         rightLen = keyLen - rightBlanks - rightDups ;
         leftLen = keyLen - leftDups - leftBlanks ;
         reqdLen = leftLen - extraDups ;

         #ifdef E4ANALYZE
            if ( reqdLen < 0 )
               return error4( b4->tag->codeBase, e4info, E80401 ) ;
         #endif

         if ( (int)b4->nodeHdr.freeSpace < (reqdLen + (int)iLen) )  /* no room to add */
            return 1 ;

         if ( reqdLen != 0 )  /* shift data over */
         {
            movLen = B4BLOCK_SIZE - sizeof( B4STD_HEADER ) - iLen * b4numKeys( b4 )
                      - sizeof( B4NODE_HEADER ) - b4->nodeHdr.freeSpace
                      - ( ( (char *)&b4->header ) + B4BLOCK_SIZE - b4->curPos ) ;

            #ifdef E4ANALYZE
               if ( movLen < 0 )
                  return error4( b4->tag->codeBase, e4info, E80401 ) ;
            #endif

            /* move and put keys */
            c4memmove( b4->curPos - movLen - reqdLen, b4->curPos - movLen, (unsigned int)movLen ) ;
         }
         b4->curPos += ( keyLen - rightBlanks - oldRightDups ) ;
         memcpy( b4->curPos - leftLen - rightLen, b4->curPos - ( keyLen - rightDups - rightBlanks ), (unsigned int)rightLen ) ;
         b4->curPos -= leftLen ;
         memcpy( b4->curPos, keyData + leftDups, (unsigned int)leftLen ) ;

         /* move and put info */
         infoPos = b4->data + ( b4->keyOn ) * iLen ;
         c4memmove( infoPos + iLen, infoPos, (unsigned int)(iLen * ( b4numKeys( b4 ) - b4->keyOn ) ) ) ;
         x4putInfo( &b4->nodeHdr, buffer, rec, leftBlanks, leftDups ) ;
         memcpy( infoPos, (void *)buffer, iLen ) ;
         x4putInfo( &b4->nodeHdr, buffer, x4recNo( b4, b4->keyOn + 1 ), rightBlanks, rightDups ) ;
         memcpy( infoPos + iLen, (void *)buffer, iLen ) ;
      }
   }

   b4->changed = 1 ;
   b4->header.nKeys++ ;
   b4->nodeHdr.freeSpace -= (short) (reqdLen + (int)iLen ) ;
   b4->curDupCnt = leftDups ;
   b4->curTrailCnt = leftBlanks ;
   return 0 ;
}

/* S4FOX */
/* returns 1 if unable to move keys around to make room */
static int b4insertBranchBalance( B4BLOCK *b4 )
{
   B4BLOCK *left, *right, *parent ;
   char *oPos, *nPos ;
   int gLen = b4->tag->header.keyLen + 2 * sizeof(S4LONG ) ;
   int rc ;

   if ( b4numKeys( b4 ) != 2 )   /* only perform in tight situations */
      return 1;

   parent = (B4BLOCK *)l4prev( &b4->tag->blocks, b4 ) ;
   if ( parent == 0 )   /* no parent, so working on a root block, allow split */
      return 1 ;

   if ( b4->keyOn == 0L )  /* try adding to right neighbor */
   {
      if ( parent->keyOn == b4numKeys( parent ) - 1 )  /* parent doesn't share blocks */
         return 1 ;

      if ( b4->header.rightNode == 0 )  /* no neighbor */
         return 1 ;

      right = b4alloc( b4->tag, b4->header.rightNode ) ;
      if ( right == 0 )
         return 1 ;
      rc = i4readBlock( &b4->tag->indexFile->file, b4->header.rightNode, 0, right ) ;
      if ( rc < 0 )
      {
         b4free( right ) ;
         return 1 ;
      }

      if ( b4numKeys( right ) >= 2 )  /* already full */
      {
         b4free( right ) ;
         return 1 ;
      }

      nPos = ((char *)&right->nodeHdr) + gLen * b4numKeys( right ) ;
      oPos = ((char *)&right->nodeHdr) ;
      memcpy( nPos, oPos, gLen * b4numKeys( right ) ) ;

      nPos = ((char *)&right->nodeHdr)  ;
      oPos = ((char *)&b4->nodeHdr) + gLen * ( b4numKeys( b4 ) - 1 ) ;
      memcpy( nPos, oPos, gLen ) ;
      b4->header.nKeys-- ;
      right->header.nKeys++ ;

      nPos = ((char *)&parent->nodeHdr) + gLen * ( parent->keyOn ) ;
      oPos = ((char *)&b4->nodeHdr ) + gLen * ( b4numKeys( b4 ) - 1 ) ;
      memcpy( nPos, oPos, b4->tag->header.keyLen + sizeof(S4LONG ) ) ;
      parent->changed = 1 ;
      right->changed = 1 ;
      b4flush( right ) ;
      b4free( right ) ;
   }
   else  /* try adding to left neighbor */
   {
      if ( parent->keyOn == 0 )  /* parent doesn't share blocks */
         return 1 ;

      if ( b4->header.leftNode == 0 )  /* no neighbor */
         return 1 ;

      left = b4alloc( b4->tag, b4->header.leftNode ) ;
      if ( left == 0 )
         return 1 ;
      rc = i4readBlock( &b4->tag->indexFile->file, b4->header.leftNode, 0, left ) ;
      if ( rc < 0 )
      {
         b4free( left ) ;
         return 1 ;
      }

      if ( b4numKeys( left ) >= 2 )  /* already full */
      {
         b4free( left ) ;
         return 1 ;
      }

      nPos = ((char *)&left->nodeHdr) + gLen * b4numKeys( left ) ;
      oPos = ((char *)&b4->nodeHdr) ;
      memcpy( nPos, oPos, gLen ) ;

      b4->header.nKeys-- ;

      nPos = ((char *)&b4->nodeHdr) ;
      oPos = ((char *)&b4->nodeHdr) + gLen * b4numKeys( b4 ) ;
      memcpy( nPos, oPos, gLen * b4numKeys( b4 ) ) ;

      b4->keyOn-- ;
      left->header.nKeys++ ;

      nPos = ((char *)&parent->nodeHdr) + gLen * ( parent->keyOn - 1 ) ;
      oPos = ((char *)&left->nodeHdr ) + gLen * ( b4numKeys( left ) - 1 ) ;
      memcpy( nPos, oPos, b4->tag->header.keyLen + sizeof(S4LONG ) ) ;
      parent->changed = 1 ;
      left->changed = 1 ;
      b4flush( left ) ;
      b4free( left ) ;
   }

   return 0 ;
}

/* S4FOX */
int b4insertBranch( B4BLOCK *b4, const void *k, const S4LONG r1, const S4LONG rin2, const char newFlag )
{
   int leftLen, moveLen, gLen ;
   char *dataPtr, *nextPtr ;
   S4LONG r, r2 ;

   gLen = b4->tag->header.keyLen + 2 * sizeof(S4LONG) ;

   E4PARM_TEST( b4 == 0 || k == 0 || r1 <= 0L || rin2 < 0, E90438 ) ;

   leftLen = B4BLOCK_SIZE - sizeof( b4->header ) - gLen * b4numKeys( b4 ) ;
   if ( leftLen < gLen )  /* not enough room */
   {
      /* if only 2 keys in branch, try to do some swapping with neighbor */
      if ( b4insertBranchBalance( b4 ) == 1 )
         return 1 ;
   }

   dataPtr = ((char *)&b4->nodeHdr) + b4->keyOn * gLen ;
   nextPtr = dataPtr + gLen ;

   #ifdef E4ANALYZE
      if ( b4->keyOn < 0 || b4->keyOn > b4numKeys( b4 ) )
         return error4( 0, e4info, E90438 ) ;
   #endif

   moveLen = gLen * ( b4numKeys( b4 ) - b4->keyOn ) ;

   c4memmove( nextPtr, dataPtr, (unsigned int)moveLen ) ;
   b4->header.nKeys++ ;
   memcpy( dataPtr, k, (unsigned int)b4->tag->header.keyLen ) ;
   memset( dataPtr + gLen - 2*sizeof(S4LONG), 0, sizeof(S4LONG) ) ;

   r2 = x4reverseLong( (void *)&rin2 ) ;
   memcpy( dataPtr + gLen - 2*sizeof(S4LONG), (void *)&r2, sizeof(S4LONG) ) ;

   r = x4reverseLong( (void *)&r1 ) ;

   if ( !newFlag &&  (b4->keyOn + 1) != b4numKeys( b4 ) )
      memcpy(  nextPtr + gLen - sizeof(S4LONG), (void *)&r, sizeof(S4LONG) ) ;
   else
      memcpy( dataPtr + gLen - sizeof(S4LONG), (void *)&r, sizeof(S4LONG) ) ;

   b4->changed = 1 ;

   return 0 ;
}

/* S4FOX */
#ifndef S4INLINE
int b4insert( B4BLOCK *b4, const void *keyData, const long rec, const long rec2, const char newFlag )
{
   E4PARM_TEST( b4 == 0 || keyData == 0 || rec <= 0L || rec2 < 0, E90438 ) ;

   return ( b4leaf( b4 ) ? b4insertLeaf( b4, keyData, rec ) : b4insertBranch( b4, keyData, rec, rec2, newFlag ) ) ;
}
#endif

/* S4FOX */
int b4reindex( B4BLOCK *b4 )
{
   int i, dupCnt, trail ;
   int niLen = b4->nodeHdr.infoLen + 1 ;
   unsigned char buffer[6] ;
   S4LONG rec ;
   int spaceReqd ;

   E4PARMLOW( b4, E90438 ) ;

   spaceReqd = b4numKeys( b4 ) ;   /* 1 byte extra for each record */
   if ( spaceReqd > b4->nodeHdr.freeSpace )  /* not enough room */
      return 1 ;

   for ( i = b4numKeys( b4 ) - 1 ; i >= 0 ; i-- )
   {
      dupCnt = x4dupCnt( b4, i ) ;
      trail = x4trailCnt( b4, i ) ;
      rec = x4recNo( b4, i ) ;
      memset( b4->data + i * niLen, 0, (unsigned int)niLen ) ;

      b4->nodeHdr.recNumLen += 8 ;  /* for the new info */
      b4->nodeHdr.infoLen++ ;
      x4putInfo( &b4->nodeHdr, buffer, rec, trail, dupCnt ) ;
      b4->nodeHdr.recNumLen -= 8 ;  /* to get the old info */
      b4->nodeHdr.infoLen-- ;
      memcpy( b4->data + i * niLen, (void *)buffer, (unsigned int)niLen ) ;
   }

   memcpy( (void *)&rec, (void *)&b4->nodeHdr.recNumMask[0], sizeof(S4LONG) ) ;
   rec |= (0x000000FFL << b4->nodeHdr.recNumLen ) ;
   memcpy( (void *)&b4->nodeHdr.recNumMask[0], (void *)&rec, sizeof(S4LONG) ) ;

   b4->nodeHdr.infoLen++ ;
   b4->nodeHdr.recNumLen += 8 ;
   b4->nodeHdr.freeSpace -= b4numKeys( b4 ) ;
   return 0 ;
}

/* S4FOX */
int b4flush( B4BLOCK *b4 )
{
   int rc ;
   INDEX4FILE *i4 ;
   TAG4FILE *t4file ;
   FILE4LONG fPos ;
   #ifdef S4BYTE_SWAP
      char swap[B4BLOCK_SIZE] ;
      char *swapPtr ;
      int i ;
      S4LONG longVal ;
      short shortVal ;
   #endif

   E4PARMLOW( b4, E90438 ) ;

   if ( b4->changed )
   {
      t4file = b4->tag ;
      i4 = t4file->indexFile ;

      file4longAssign( fPos, b4->fileBlock*I4MULTIPLY, 0 ) ;
      #ifndef S4OFF_OPTIMIZE
         i4->readBlockTag = t4file ;
      #endif
      #ifdef S4BYTE_SWAP
         memcpy( (void *)swap, (void *)&b4->header.nodeAttribute, B4BLOCK_SIZE ) ;

         /* position at either B4NODE_HEADER (leaf) or data (branch) */
         swapPtr = swap + 2 * sizeof( short) + 2 * sizeof(S4LONG) ;

         /* if block is a leaf */
         if (b4->header.nodeAttribute >= 2 )
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
            shortVal = t4file->header.keyLen + sizeof(S4LONG) ;

            /* position swapPtr to end of first key expression */
            swapPtr += t4file->header.keyLen ;

            /* move through all B4KEY's to swap 'long's */
            for ( i = 0 ; i < (int)b4numKeys( b4 ) ; i++ )
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

         rc = file4writeInternal( &i4->file, fPos, swap, B4BLOCK_SIZE ) ;
      #else
         rc = file4writeInternal( &i4->file, fPos, &b4->header.nodeAttribute, B4BLOCK_SIZE ) ;
      #endif
      #ifndef S4OFF_OPTIMIZE
         i4->readBlockTag = 0 ;
      #endif

      if ( rc < 0 )
         return rc ;
      b4->changed = 0 ;
   }
   return 0 ;
}
#endif /* S4OFF_WRITE */

#ifndef S4INLINE
/* S4FOX */
int b4go( B4BLOCK *b4, const int iKey )
{
   E4PARM_TEST( b4 == 0 || iKey < 0, E90438 ) ;

   return b4skip( b4, iKey - b4->keyOn ) ;
}
#endif /* S4INLINE */

/* S4FOX */
int b4top( B4BLOCK *b4 )
{
   E4PARMLOW( b4, E90438 ) ;

   b4->keyOn = 0 ;
   if ( b4leaf( b4 ) )
   {
      b4->curDupCnt = 0 ;
      b4->curPos = ((char *)&b4->header) + B4BLOCK_SIZE - b4->tag->header.keyLen + x4trailCnt( b4, 0 ) ;
   }

   return 0 ;
}

/* S4FOX */
void b4goEof( B4BLOCK *b4 )
{
   E4PARMLOW( b4, E90438 ) ;

   b4->keyOn = b4numKeys( b4 ) ;
   b4->curPos = ((char *)&b4->header) + sizeof( B4STD_HEADER ) + sizeof( B4NODE_HEADER )
                 + b4numKeys( b4 ) * b4->nodeHdr.infoLen + b4->nodeHdr.freeSpace ;
}

/* S4FOX */
B4KEY_DATA *b4key( B4BLOCK *b4, const int iKey )
{
   int len, kLen ;
   char *val ;

   E4PARM_TRET( b4 == 0 || iKey > b4numKeys( b4 ) || iKey < 0, E90438, NULL ) ;

   if ( iKey == b4->builtOn )   /* already there! */
      return b4->builtKey ;

   val = (char *)(&b4->builtKey->value[0]) ;
   kLen = b4->tag->header.keyLen ;

   if ( b4->header.nodeAttribute >= 2 ) /* leaf */
   {
      if ( b4->builtOn > iKey || b4->builtOn == -1 )
      {
         b4->builtOn = -1 ;
         b4->builtPos = ((char *)&b4->header) + B4BLOCK_SIZE ;
      }

      for ( ; b4->builtOn != iKey ; )
      {
         b4->builtOn++ ;
         b4->curDupCnt = x4dupCnt( b4, b4->builtOn ) ;
         b4->curTrailCnt = x4trailCnt( b4, b4->builtOn ) ;
         len = kLen - b4->curDupCnt - b4->curTrailCnt ;
         #ifdef E4ANALYZE
            if (len < 0 || len > kLen || ( b4->builtPos - len ) < b4->data )
            {
               error4( b4->tag->codeBase, e4info, E80401 ) ;
               return 0 ;
            }
         #endif
         b4->builtPos -= len ;
         memcpy( val + b4->curDupCnt, b4->builtPos, (unsigned int)len ) ;
         memset( val + kLen - b4->curTrailCnt, b4->tag->pChar, (unsigned int)b4->curTrailCnt ) ;
      }
      b4->builtKey->num = x4recNo( b4, iKey ) ;
   }
   else /* branch */
   {
      memcpy( val, (((char *)&b4->nodeHdr) + iKey*(2*sizeof(S4LONG ) + kLen ) ), (unsigned int)kLen ) ;
      b4->builtKey->num = x4reverseLong( (void *)( ((unsigned char *)&b4->nodeHdr)
                          + (iKey+1)*(2*sizeof(S4LONG) + kLen) - sizeof(S4LONG) ) ) ;
   }
   return b4->builtKey ;
}

/* S4FOX */
/* based on the actual stored-data (from file) only, determine whether or
   not a leaf.  Used by file optimization routines */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int b4dataLeaf( void *data, TAG4FILE *tag )
{
   B4STD_HEADER *header ;
   E4PARMLOW( data && tag, E90438 ) ;

   header = (B4STD_HEADER *)data ;

   return( header->nodeAttribute >= 2 ) ;
}

#ifndef S4INLINE
/* S4FOX */
unsigned char *b4keyKey( B4BLOCK *b4, const int iKey )
{
   E4PARMLOW( b4, E90438 ) ;

   return (unsigned char *)b4key( b4, iKey )->value ;
}

/* S4FOX */
int b4lastpos( const B4BLOCK *b4 )
{
   E4PARMLOW( b4, E90438 ) ;

   return b4numKeys( b4 ) - 1 ;
}

/* S4FOX */
int b4leaf( const B4BLOCK *b4 )
{
   E4PARMLOW( b4, E90438 ) ;

   return( b4->header.nodeAttribute >= 2 ) ;
}
#endif /* S4INLINE */

/* S4FOX */
long b4recNo( const B4BLOCK *b4, const int i )
{
   E4PARM_TEST( b4 == 0 || i < 0, E90438 ) ;

   if ( b4->header.nodeAttribute >= 2 ) /* leaf */
      return x4recNo( b4, i ) ;
   else /* branch */
       return (unsigned long)x4reverseLong( (void *)(((unsigned char *)&b4->nodeHdr) +
          i * (2*sizeof(S4LONG) + b4->tag->header.keyLen) + b4->tag->header.keyLen ) ) ;
}

/* S4FOX */
#ifndef S4OFF_WRITE
int b4brReplace( B4BLOCK *b4, const unsigned char *str, const long r )
{
   int keyLen ;
   char *putPl ;
   long rec ;

   E4PARM_TEST( b4 == 0 || str == 0 || r < 0, E90438 ) ;

   keyLen = b4->tag->header.keyLen ;
   putPl = ( (char *)&b4->nodeHdr ) + b4->keyOn * ( 2 * sizeof(S4LONG ) + keyLen ) ;
   memcpy( putPl, str, (unsigned int)keyLen ) ;
   memcpy( b4->builtKey->value, str, (unsigned int)keyLen ) ;
   memset( putPl + keyLen, 0, sizeof(S4LONG) ) ;
   rec = x4reverseLong( (void *)&r ) ;
   memcpy( putPl + keyLen, (void *)&rec, sizeof(S4LONG) ) ;

   b4->changed = 1 ;

   return 0 ;
}
#endif /* S4OFF_WRITE */

/* S4FOX */
int b4rBrseek( B4BLOCK *b4, const char *searchValue, const int len, const long recNo )
{
   int rc, keyLen ;
   #ifdef S4DO_BYTEORDER
      long longVal, lrecNo ;
   #endif
   #ifdef E4ANALYZE_ALL
      long swapped ;
   #endif

   E4PARM_TEST( b4 == 0 || searchValue == 0 || len < 0, E90438 ) ;

   #ifdef E4ANALYZE_ALL
      /* incoming rec is swapped, so unswap it to see if it is valid */
      swapped = recNo ;
      swapped = x4reverseShort( &swapped ) ;
      if ( swapped < 0 )
         return error4( 0, e4parm, E90438 ) ;
   #endif

   rc = b4seek( b4, searchValue, len ) ;

   keyLen = b4->tag->header.keyLen ;
   if ( rc == 0 )   /* string matches, so search on recNo */
      for(;;)
      {
         #ifdef S4DO_BYTEORDER
            lrecNo = x4reverseLong( (void *)&recNo ) ;
            longVal = x4reverseLong( (void *)(((char *)&b4->nodeHdr) + b4->keyOn * ( 2 * sizeof(S4LONG) + keyLen) + keyLen ) ) ;
            if ( c4memcmp( (void *)&lrecNo, (void *)&longVal, sizeof(S4LONG) ) <= 0
         #else
            if ( c4memcmp( (void *)&recNo, ((char *)&b4->nodeHdr) + b4->keyOn * ( 2 * sizeof(S4LONG) + keyLen) + keyLen, sizeof(S4LONG) ) <= 0
          #endif
            || b4->keyOn >= ( b4numKeys( b4 ) - 1 ) )  /* best match, so done */
               break ;
         else
            if( b4->keyOn < b4numKeys( b4 ) )
               b4skip( b4, 1L ) ;

         if ( u4memcmp( (void *)b4keyKey(b4,b4->keyOn), searchValue, (unsigned int)len ) )   /* the key has changed, so stop here */
            break ;
      }

   return rc ;
}

/* S4FOX */
int b4seek( B4BLOCK *b4, const char *searchValue, const int len )
{
   int rc, keyLower, keyUpper, saveRc, keyCur, groupVal ;

   E4PARM_TEST( b4 == 0 || searchValue == 0 || len < 0, E90438 ) ;

   if ( b4numKeys( b4 ) == 0 )
   {
      b4->keyOn = 0 ;
      return r4after ;
   }

   if ( b4leaf( b4 ) )
      return b4leafSeek( b4, searchValue, len ) ;

   /* keyCur must be between  keyLower and  keyUpper */
   keyLower = -1 ;
   keyUpper = b4numKeys( b4 ) - 1 ;

   saveRc = 1 ;
   groupVal = 2 * sizeof(S4LONG ) + b4->tag->header.keyLen ;

   for(;;)  /* Repeat until the key is found */
   {
      keyCur = (keyUpper + keyLower) / 2 ;
      rc = u4memcmp( (((char *)&b4->nodeHdr) + keyCur * groupVal ), searchValue, (unsigned int)len ) ;

      if ( rc >= 0 )
      {
         keyUpper = keyCur ;
         saveRc = rc ;
      }
      else
         keyLower = keyCur ;

      if ( keyLower >= (keyUpper-1) )  /* then there is no exact match */
      {
         b4->keyOn = keyUpper ;   /* branch block, just change keyOn */
         if ( saveRc )
            return r4after ;
         return 0 ;
      }
   }
}

#ifdef S4VFP_KEY
int u4tailCmp( S4CMP_PARM dataPtr, S4CMP_PARM searchPtr, size_t sLen )
{
   unsigned char *data = (unsigned char *)dataPtr ;
   unsigned char *search = (unsigned char *)searchPtr ;
   unsigned int on ;

   for ( on = 0 ; on < sLen ; on++ )
      if ( search[on] !=17 || ( data[on] >= 10 && data[on] != 17 ) )
         break ;

   return on ;
}
#endif

/* S4FOX */
int b4leafSeek( B4BLOCK *b4, const char *searchValue, const int l )
{
   int done, len, trailCnt, duplicates, bytesSame, compareLen, keyLen, originalLen, significantBytes, cLen, loop ;
   #ifdef S4VFP_KEY
      int skippedSpaces ;
   #endif
   char allBlank ;

   E4PARM_TEST( b4 == 0 || searchValue == 0 || l < 0, E90438 ) ;

   originalLen = len = l ;
   keyLen = b4->tag->header.keyLen ;
   #ifdef S4VFP_KEY
      if ( tfile4vfpKey( b4->tag ) && b4->tag->expr->type == r4str )
         for ( ; len > 0 && searchValue[len-1] == 17 ; len-- ) ;
      skippedSpaces = ( len == originalLen ) ? 0 : 1 ;
   #endif
   /* FoxPro 2.6 has no blanks if filters exist */
   if (! ( b4->tag->indexFile->dataFile->version != 0x30 && b4->tag->codeBase->compatibility == 26 && b4->tag->filter != 0 ) )
      len -= b4calcBlanks( (unsigned char *)searchValue, len, b4->tag->pChar ) ;  /* don't compare blank bytes */
   if ( len == 0 )   /* if all blanks, watch for records < blank */
   {
      len = originalLen ;
      allBlank = 1 ;
   }
   else
      allBlank = 0 ;
   duplicates = 0 ;
   b4top( b4 ) ;

   for(;;)
   {
      if ( b4->curDupCnt == duplicates )
      {
         significantBytes = keyLen - x4trailCnt( b4, b4->keyOn ) ;
         if ( allBlank && significantBytes == 0 )   /* found a blank record */
            len = 0 ;
         compareLen = ( len < significantBytes ? len : significantBytes ) - b4->curDupCnt ;
         bytesSame = (*b4->tag->cmp)( b4->curPos, searchValue + b4->curDupCnt, (unsigned int)compareLen ) ;

         if ( bytesSame == -1 )
            return r4after ;
         if ( bytesSame == compareLen )
         {
            if ( (b4->curDupCnt + bytesSame == len))  /* unless binary character, for sure done */
            {
               if ( len == originalLen )
                  done = 1 ;
               else  /* in case of binary, we may not be done if values < blank */
               {
                   trailCnt = x4trailCnt( b4, b4->keyOn ) ;
                   if ( significantBytes > trailCnt + len )  /* significant bytes beyond the trail, so check binary */
                   {                                         /* for a VFP_KEY this should never happen as key functions should guarantee l <= keyLen */
                      cLen = keyLen - trailCnt ;
                      if ( originalLen - len < cLen )
                         cLen = originalLen - len ;
                      if ( c4memcmp( b4->curPos + compareLen, searchValue + len, (unsigned int)cLen ) < 0 )  /* binaries */
                         done = 0 ;
                      else
                         done = 1 ;
                   }
                   else
                      done = 1 ;
               }

               if ( done == 1 )
               {
                  if ( len != originalLen && significantBytes > len )
                  {
                     #ifdef S4VFP_KEY
                        if ( skippedSpaces )
                        {
                           cLen = originalLen - ( len < significantBytes ? len : significantBytes ) ;
                           if ( u4tailCmp( b4->curPos + compareLen, searchValue + len, cLen ) != cLen )
                              return r4after ;
                        }
                        else
                     #endif
                        {
                           if ( significantBytes < originalLen )
                              return r4after ;
                           #ifdef E4ANALYZE
                              if ( len > originalLen )
                                 return error4( b4->tag->codeBase, e4info, E90438 ) ;
                           #endif
                           if ( (*b4->tag->cmp)( b4->curPos + compareLen, searchValue + len, (unsigned int)(originalLen - len) ) != (originalLen - len) )
                              return r4after ;
                         }
                  }
                  if ( allBlank != 1 )   /* not all blanks, so significants matter */
                     b4->curDupCnt += bytesSame ;
                  else
                     b4->curDupCnt = 0 ;
                  return 0 ;
               }
            }
            else   /* this with lines below added AS 10/29/97 for fix #110 in changes.60 */
            {
               /* it is possible in binary seeks that we are too far now this would happen if the next
                  byte to seek is < 32 and the search key has trailing blanks as next byte  */
               if ( ( (unsigned char)searchValue[b4->curDupCnt + bytesSame] < (unsigned char)b4->tag->pChar ) && ( (b4->curDupCnt + bytesSame) == significantBytes ) )
               {
                  /* then remaining bytes must be blank, we are too far */
                  return r4after ;
               }
               if ( ( searchValue[b4->curDupCnt + bytesSame] == b4->tag->pChar ) && ( (b4->curDupCnt + bytesSame) == significantBytes ) )
               {
                  /* we may have a match -- if the rest of the seek key is < = blanks (since significant bytes is matched, only blanks must be left in found key)
                     then remaining bytes must be blank, we are too far */
                  for ( loop = b4->curDupCnt + bytesSame ; loop < originalLen ; loop++ )
                  {
                     if ( (unsigned char)searchValue[loop] > (unsigned char)b4->tag->pChar )  /* not found */
                     {
                        loop = -1 ;
                        break ;
                     }
                  }
                  if ( loop != -1 )
                     return r4after ;
               }
            }
         }

         b4->curDupCnt += bytesSame ;
      }

      b4->keyOn++ ;
      if ( b4->keyOn >= b4numKeys( b4 ) )
         return r4after ;
      duplicates = x4dupCnt( b4, b4->keyOn ) ;
      #ifdef E4ANALYZE
         if ( keyLen - duplicates - x4trailCnt( b4, b4->keyOn ) < 0 )  /* index corrupt */
            return error4( b4->tag->codeBase, e4index, E90438 ) ;
      #endif
      b4->curPos -= keyLen - duplicates - x4trailCnt( b4, b4->keyOn ) ;
      if ( b4->curDupCnt > duplicates )
      {
         /* AS 03/11/97 there are some rare cases where this will happen ok, and that is
            if our search key contains blanks at the front and greater than blank at current
            seek position, but current key goes from binary pre-blank (i.e. < 0x20) on current
            key to blank, at which point trail-cnt takes over for duplicates so that we get
            one less duplicate, but in fact it is duplicate due to blank!!!
            i.e. what happened was that 'blanks' caused the duplicates to go down in
            favor of trails
         */
         if ( searchValue[b4->curDupCnt-1] == 0x20 ) /* is a blank, so possibly the case */
            if ( keyLen - x4trailCnt( b4, b4->keyOn ) < b4->curDupCnt )  /* it has happened! */
               if ( keyLen - x4dupCnt( b4, b4->keyOn ) - x4trailCnt( b4, b4->keyOn ) == 0 )
               {
                  /* what should we do? I'd say just continue */
                  /* we actually have to reset the curDupCnt because we need to re-scan some keys
                     just subtract the number of blanks we encountered
                  */
                  b4->curDupCnt = duplicates = x4dupCnt( b4, b4->keyOn ) ;
                  b4->curPos -= keyLen - duplicates - x4trailCnt( b4, b4->keyOn ) ;
               }
      }
      if ( b4->curDupCnt > duplicates )
      {
         if ( allBlank )  /* must reset the duplicates in this case only */
         {
            b4->curDupCnt = duplicates ;
            if ( x4trailCnt( b4, b4->keyOn ) == len )   /* blank key found */
               return 0 ;
         }
         return r4after ;
      }
   }
}

/* S4FOX */
#ifndef S4OFF_WRITE
int b4removeLeaf( B4BLOCK *b4 )
{
   int oldDup, leftDup, leftTrail, keyLen, removeLen, newDup, oldBytes, newBytes, movLen, leftBytes ;
   unsigned char buffer[6], iLen ;
   char *oldPos, *infoPos ;

   E4PARMLOW( b4, E90438 ) ;

   b4->builtOn = -1 ;

   if ( b4numKeys( b4 ) == 1 )  /* removing last key */
   {
      b4->nodeHdr.freeSpace = B4BLOCK_SIZE - sizeof( B4NODE_HEADER ) - sizeof( B4STD_HEADER ) ;
      memset( b4->data, 0, (unsigned int)b4->nodeHdr.freeSpace ) ;
      b4->header.nKeys = 0 ;
      b4->keyOn = 0 ;
      b4->curPos = ((char *)&b4->header) + B4BLOCK_SIZE ;
      b4->changed = 1 ;
      return 0 ;
   }

   keyLen = b4->tag->header.keyLen ;
   iLen = b4->nodeHdr.infoLen ;
   oldDup = x4dupCnt( b4, b4->keyOn) ;

   if ( b4->keyOn == b4numKeys( b4 ) - 1 )  /* at end */
   {
      removeLen = keyLen - oldDup - x4trailCnt( b4, b4->keyOn ) ;

      #ifdef E4ANALYZE
         if ( removeLen < 0 )
            return error4( b4->tag->codeBase, e4info, E80401 ) ;
      #endif

      memset( b4->curPos, 0, (unsigned int)removeLen ) ;
      memset( b4->data + b4->keyOn * iLen, 0, iLen ) ;
      b4->keyOn-- ;
   }
   else
   {
      oldBytes = keyLen - oldDup - x4trailCnt( b4, b4->keyOn ) ;
      oldPos = b4->curPos ;
      b4skip( b4, 1L ) ;
      leftDup = x4dupCnt( b4, b4->keyOn) ;
      leftTrail = x4trailCnt( b4, b4->keyOn ) ;
      newDup = oldDup < leftDup ? oldDup : leftDup ;
      leftBytes = keyLen - leftTrail - leftDup ;
      newBytes = keyLen - leftTrail - newDup ;

      memcpy( b4->builtKey->value, oldPos, (unsigned int)(leftDup - newDup) ) ;  /* copy prev dup bytes between the 2 keys only */
      memcpy( b4->builtKey->value + ( leftDup - newDup ), b4->curPos, (unsigned int)leftBytes ) ;
      memcpy( oldPos + oldBytes - newBytes, b4->builtKey->value, (unsigned int)newBytes ) ;

      removeLen = oldBytes + leftBytes - newBytes ;
      #ifdef E4ANALYZE
         if ( removeLen < 0 )
            return error4( b4->tag->codeBase, e4info, E80401 ) ;
      #endif

      movLen = B4BLOCK_SIZE - sizeof( B4STD_HEADER ) - iLen * b4numKeys( b4 )
                - sizeof( B4NODE_HEADER ) - b4->nodeHdr.freeSpace
                - ( ( (char *)&b4->header ) + B4BLOCK_SIZE - b4->curPos ) ;
      #ifdef E4ANALYZE
         if ( movLen < 0 )
            return error4( b4->tag->codeBase, e4info, E80401 ) ;
      #endif

      c4memmove( b4->curPos - movLen + removeLen, b4->curPos - movLen, (unsigned int)movLen ) ;

      memset( b4->curPos - movLen, 0, (unsigned int)removeLen ) ;
      b4->keyOn-- ;

      /* move and put info */
      infoPos = b4->data + ( b4->keyOn ) * iLen ;
      c4memmove( infoPos, infoPos + iLen, (unsigned int)(iLen * ( b4numKeys( b4 ) - b4->keyOn ) ) ) ;
      x4putInfo( &b4->nodeHdr, buffer, x4recNo( b4, b4->keyOn ), leftTrail, (unsigned int)newDup ) ;
      memcpy( infoPos, (void *)buffer, iLen ) ;
      memset( b4->data + (b4numKeys( b4 ) - 1 ) * iLen, 0, iLen ) ;
   }
   b4->changed = 1 ;
   b4->curPos += removeLen ;
   b4->header.nKeys-- ;
   b4->nodeHdr.freeSpace += (short)( (int)iLen + removeLen ) ;

   return 0 ;
}

/* S4FOX */
int b4remove( B4BLOCK *b4 )
{
   char *keyCur ;
   int len, iLen ;

   E4PARMLOW( b4, E90438 ) ;

   if( b4->header.nodeAttribute >= 2 ) /* leaf */
      b4removeLeaf( b4 ) ;
   else
   {
      iLen = b4->tag->header.keyLen + 2 * sizeof(S4LONG ) ;
      keyCur = ((char *)&b4->nodeHdr) + iLen * b4->keyOn ;
      len = (b4numKeys( b4 ) - b4->keyOn - 1) * iLen ;

      #ifdef E4ANALYZE
         if ( b4->keyOn < 0  || b4->keyOn > b4lastpos( b4 ) ||
              len < 0 || (unsigned)len > (B4BLOCK_SIZE - sizeof(B4STD_HEADER) - (unsigned)iLen ))
            return error4( b4->tag->codeBase, e4info, E90438 ) ;
      #endif

      if ( len > 0 )
         c4memmove( keyCur, keyCur + iLen, (unsigned int)len ) ;

      b4->header.nKeys-- ;
      memset( ((char *)&b4->nodeHdr) + b4numKeys( b4 ) * iLen, 0, (unsigned int)iLen ) ;
      b4->changed = 1 ;
   }

   return 0 ;
}
#endif /* S4OFF_WRITE */

/* S4FOX */
int b4skip( B4BLOCK *b4, const long num )
{
   int numSkipped, kLen, nKeys ;
   long n ;

   n = num ;

   E4PARMLOW( b4, E90438 ) ;

   nKeys = b4numKeys( b4 ) ;

   if ( b4->header.nodeAttribute < 2 )  /* branch */
   {
      if ( n > 0 )
         numSkipped = nKeys - b4->keyOn ;
      else
         numSkipped = -b4->keyOn ;

      if ( ( n <= 0L ) ? (numSkipped <= (int)n) : (numSkipped >= (int)n) )
      {
         b4->keyOn += (int)n ;
         return (int)n ;
      }
      else
      {
         b4->keyOn += numSkipped ;
         return numSkipped ;
      }
   }
   else  /* leaf */
   {
      if ( nKeys == 0 )
         return 0 ;
      kLen = b4->tag->header.keyLen ;
      if (n > 0 )
      {
         if ( b4->keyOn + n >= nKeys )
         {
            b4->curPos = ((char *)&b4->header ) + sizeof( B4STD_HEADER ) + sizeof( B4NODE_HEADER )
                          + b4->nodeHdr.infoLen * nKeys + b4->nodeHdr.freeSpace ;
            n = (long)nKeys - (long)b4->keyOn - ( b4->keyOn != nKeys ) ;
            b4->keyOn = nKeys ;
            return (int)n ;
         }
         for( numSkipped = (int)n ; numSkipped != 0 ; numSkipped-- )
         {
            b4->keyOn++ ;
            b4->curPos -= ( kLen - x4dupCnt( b4, b4->keyOn ) - x4trailCnt( b4, b4->keyOn ) ) ;
         }
      }
      else
      {
         if ( b4->keyOn + n < 0 )
         {
            n = -b4->keyOn ;
            b4->keyOn = 0 ;
            b4->curPos = ((char *)&b4->header ) + B4BLOCK_SIZE - kLen + x4trailCnt( b4, b4->keyOn ) ;
            return (int) n ;
         }
         for( numSkipped = (int)n ; numSkipped != 0 ; numSkipped++ )
         {
            b4->curPos += ( kLen - x4dupCnt( b4, b4->keyOn ) - x4trailCnt( b4, b4->keyOn ) ) ;
            b4->keyOn-- ;
         }
      }

      return (int) n ;
   }
}
#endif  /* S4FOX */

#ifdef S4CLIPPER

#ifndef S4OFF_WRITE
int b4flush( B4BLOCK *b4 )
{
   int rc ;
   #ifdef S4BYTE_SWAP
      char *swap, *swapPtr ;
      int i ;
      S4LONG longVal ;
      short shortVal ;
   #else
      FILE4LONG pos ;
   #endif

   E4PARMLOW( b4, E90438 ) ;

   if ( b4->changed )
   {
      #ifdef E4INDEX_VERIFY
         if ( b4verify( b4 ) == -1 )
            error4describe( b4->tag->codeBase, e4index, E90438, b4->tag->alias, 0, 0 ) ;
      #endif

      #ifndef S4OFF_OPTIMIZE
         b4->tag->readBlockTag = b4->tag ;
      #endif
      #ifdef S4BYTE_SWAP
         swap = (char *)u4allocEr( b4->tag->codeBase, B4BLOCK_SIZE ) ;
         if ( swap == 0 )
            return -1 ;

         memcpy( (void *)swap, (void *)&b4->nKeys, B4BLOCK_SIZE ) ;

         index4swapBlockClipper(swap, b4->tag->header.keysMax, b4->tag->header.groupLen) ;

         rc = file4writeInternal( &b4->tag->file, (long)b4->fileBlock*I4MULTIPLY, swap, B4BLOCK_SIZE ) ;
         u4free( swap ) ;
      #else
         file4longAssign( pos, b4->fileBlock, 0 ) ;
         file4longMultiply( pos, I4MULTIPLY ) ;
         rc = file4writeInternal( &b4->tag->file, pos, &b4->nKeys, B4BLOCK_SIZE ) ;
      #endif
      #ifndef S4OFF_OPTIMIZE
         b4->tag->readBlockTag = 0 ;
      #endif

      if ( rc < 0 )
         return rc ;
      b4->changed = 0 ;
   }
   return 0 ;
}

int b4append( B4BLOCK *b4, const long pointer )
{
   long adjPointer = pointer * 512 ;
   B4KEY_DATA *dataPtr ;
   #ifdef E4ANALYZE
      int leftLen ;
   #endif

   E4PARM_TEST( b4 == 0 || pointer < 1L, E90438 ) ;

   #ifdef E4ANALYZE
      if ( b4leaf( b4 ) )
         return error4( b4->tag->codeBase, e4info, E90438 ) ;
   #endif

   b4goEof( b4 ) ;
   dataPtr = b4key( b4, b4->keyOn ) ;

   #ifdef E4ANALYZE
      leftLen = B4BLOCK_SIZE - b4->keyOn * b4->tag->header.groupLen -
         sizeof(short) - sizeof(char[2]) - sizeof(pointer) ;
      if ( b4->keyOn < 0  ||  b4->keyOn != b4numKeys( b4 ) || leftLen < 0 )
          return error4( b4->tag->codeBase, e4info, E90438 ) ;
   #endif

   memcpy( &dataPtr->pointer, &adjPointer, sizeof(pointer) ) ;
   b4->changed = 1 ;
   #ifdef E4INDEX_VERIFY
      if ( b4verify( b4 ) == -1 )
         return error4describe( b4->tag->codeBase, e4index, E90438, b4->tag->alias, 0, 0 ) ;
   #endif

   return 0 ;
}
#endif /* S4OFF_WRITE */

#ifndef S4OFF_WRITE
int b4append2( B4BLOCK *b4, const void *k, const long r, const long pointer )
{
   unsigned short temp ;
   B4KEY_DATA *dataPtr ;
   long adjPointer ;

   E4PARM_TEST( b4 == 0 || k == 0 || r < 0L || pointer < 0L, E90438 ) ;

   b4goEof( b4 ) ;

   if ( b4numKeys( b4 ) > 0 )
   {
      if ( !b4leaf( b4 ) )   /* insert after branch */
         b4->keyOn++ ;
      else
         if ( b4->keyOn + !b4leaf( b4 ) < b4->tag->header.keysMax )
         {
            temp = b4->pointers[b4numKeys( b4 )+1] ;
            b4->pointers[b4numKeys( b4 )+1] = b4->pointers[b4numKeys( b4 )] ;
            b4->pointers[b4numKeys( b4 )] = temp ;
         }
   }

   dataPtr  = b4key( b4, b4->keyOn ) ;
   adjPointer = pointer * 512 ;
   b4->nKeys++ ;

   memcpy( dataPtr->value, k, b4->tag->header.groupLen-2*sizeof(S4LONG) ) ;
   memcpy( &dataPtr->num, &r, sizeof(r) ) ;
   memcpy( &dataPtr->pointer, &adjPointer, sizeof(pointer) ) ;
   b4->changed = 1 ;

   return 0 ;
}

/* clipper just puts at end and inserts into the index part insert means place before cur pos*/
int b4insert( B4BLOCK *b4, const void *k, const long r, const long pointer )
{
   short temp, insertPos ;

   E4PARM_TEST( b4 == 0 || k == 0 || r < 0L || pointer < 0L, E90438 ) ;

   insertPos = b4->keyOn ;

   /* put at block end: */
   b4append2( b4, k, r, pointer ) ;

   temp = b4->pointers[b4->keyOn] ;   /* save the placed position */
   c4memmove( &b4->pointers[insertPos+1], &b4->pointers[insertPos], sizeof(short) * ( b4lastpos( b4 ) - insertPos -
     (!b4leaf( b4 ) && b4numKeys( b4 ) < 2 ) ) ) ;
   b4->pointers[insertPos] = temp ;
   if ( b4leaf( b4 ) )
      if ( b4key( b4, b4numKeys( b4 ) )->pointer != 0 )
         b4key( b4, b4numKeys( b4 ) )->pointer = 0L ;

   return 0 ;
}
#endif /* S4OFF_WRITE */

#ifndef S4INLINE
/* goes to one past the end of the block */
void b4goEof( B4BLOCK *b4 )
{
   E4PARMLOW( b4, E90438 ) ;

   b4->keyOn = b4numKeys( b4 ) ;

   return 0 ;
}

B4KEY_DATA *b4key( const B4BLOCK *b4, const int iKey )
{
   E4PARM_TRET( b4 == 0 || i < 0, E90438, NULL ) ;

   #ifdef E4ANALYZE
      if ( iKey > 2 + b4->tag->header.keysMax )
      {
         error4( b4->tag->codeBase, e4parm, E90438 ) ;
         return 0 ;
      }
      if ( ( b4->pointers[iKey] < (short)((sizeof(short)) * ( b4->tag->header.keysMax + 1 ) ) ) ||
         ( b4->pointers[iKey] > B4BLOCK_SIZE - b4->tag->header.keyLen ) )
      {
         error4( b4->tag->codeBase, e4info, E90438 ) ;
         return 0 ;
      }
   #endif
   return  (B4KEY_DATA *)((char *)&b4->nKeys + b4->pointers[iKey] ) ;
}

unsigned char *b4keyKey( B4BLOCK *b4, const int iKey )
{
   E4PARM_TRET( b4 == 0 || iKey < 0, E90438, NULL ) ;

   return (unsigned char *) b4key( b4, iKey )->value ;
}

int b4lastpos( const B4BLOCK *b4 )
{
   E4PARMLOW( b4, E90438 ) ;

   return ( ( b4leaf( b4 ) ) ? ( b4numKeys( b4 ) - 1 ) : ( b4numKeys( b4 ) ) ) ;
}

int b4leaf( const B4BLOCK *b4 )
{
   E4PARMLOW( b4, E90438 ) ;

   return( b4key( b4, 0 )->pointer == 0L ) ;
}

long b4recNo( const B4BLOCK *b4, const int i )
{
   E4PARM_TEST( b4 == 0 || i < 0, E90438 );

   return b4key( b4, i )->num ;
}
#endif /* S4INLINE */

/* S4CLIPPER */
/* based on the actual stored-data (from file) only, determine whether or
   not a leaf.  Used by file optimization routines */
int b4dataLeaf( void *data, TAG4FILE *tag )
{
   B4KEY_DATA *keyData ;

   E4PARMLOW( data && tag, E90438 ) ;

   keyData = (B4KEY_DATA *)( (char *)data + *((short *)((char *)data + sizeof(short))) ) ;

   return( keyData->pointer == 0L ) ;
}

/* S4CLIPPER */
int b4remove( B4BLOCK *b4 )
{
   short temp ;

   E4PARMLOW( b4, E90438 ) ;

   /* just delete this entry */
   temp = b4->pointers[b4->keyOn] ;
   if ( b4leaf( b4 ) && ( b4->keyOn + !b4leaf( b4 ) < b4->tag->header.keysMax ) )
   {
      c4memmove( &b4->pointers[b4->keyOn], &b4->pointers[b4->keyOn+1],
               sizeof(short) * (b4lastpos( b4 ) - b4->keyOn + 1) ) ;
      b4->pointers[b4lastpos( b4 )+1] = temp ;
   }
   else
   {
      c4memmove( &b4->pointers[b4->keyOn], &b4->pointers[b4->keyOn+1],
               sizeof(short) * (b4lastpos( b4 ) - b4->keyOn) ) ;
      b4->pointers[b4lastpos( b4 )] = temp ;
   }
   b4->nKeys-- ;
   b4->changed = 1 ;

   if ( b4leaf( b4 ) )
      memset( b4key( b4, b4lastpos( b4 ) + 1 ), 0, b4->tag->header.keyLen + 2 * sizeof(S4LONG ) ) ;

   return 0 ;
}

int b4room( const B4BLOCK *b4 )
{
   E4PARMLOW( b4, E90438 ) ;


   if ( b4leaf( b4 ) )
      return ( b4numKeys( b4 ) < b4->tag->header.keysMax ) ;

   return( ( b4numKeys( b4 ) < b4->tag->header.keysMax ) && ( ( B4BLOCK_SIZE -
      b4numKeys( b4 ) * b4->tag->header.groupLen - sizeof(short) - 2*sizeof(char)) >= sizeof(S4LONG) ) ) ;
}

int b4seek( B4BLOCK *b4, const char *searchValue, const int len )
{
   int rc, saveRc, keyCur, keyLower, keyUpper ;
   S4CMP_FUNCTION *cmp = b4->tag->cmp ;

   E4PARM_TEST( b4 == 0 || searchValue == 0 || len < 0, E90438 );

   /* keyOn must be between  keyLower and  keyUpper */
   keyLower = -1 ;
   keyUpper = b4numKeys( b4 ) ;

   if ( keyUpper == 0 )
   {
      b4->keyOn = 0 ;
      return r4after ;
   }

   saveRc = 1 ;

   for(;;)  /* Repeat until the key is found */
   {
      keyCur  = (keyUpper + keyLower) / 2  ;
      rc = (*cmp)( b4keyKey(b4,keyCur), searchValue, len ) ;

      if ( rc >= 0 )
      {
         keyUpper = keyCur ;
         saveRc = rc ;
      }
      else
         keyLower = keyCur ;

      if ( keyLower >= (keyUpper-1) )  /* then there is no exact match */
      {
         b4->keyOn = keyUpper ;
         if ( saveRc )
            return r4after ;
         return 0 ;
      }
   }
}

int b4skip( B4BLOCK *b4, const long n )
{
   int numLeft ;

   E4PARMLOW( b4, E90438 ) ;


   if ( n > 0 )
   {
      numLeft = b4numKeys( b4 ) - b4->keyOn ;
      if ( b4leaf( b4 ) )
      if ( numLeft != 0 )
         numLeft -- ;
   }
   else
      numLeft = -b4->keyOn ;

   if ( ( n <= 0L ) ? ( (long)numLeft <= n ) : ( (long)numLeft >= n ) )
   {
      b4->keyOn = b4->keyOn + (int) n ;
      return (int) n ;
   }
   else
   {
      b4->keyOn = b4->keyOn + numLeft ;
      return numLeft ;
   }
}
#endif /* S4CLIPPER */

#ifdef E4INDEX_VERIFY
int b4verify( B4BLOCK *b4 )
{
   int i ;
   #ifdef S4CLIPPER
      int j ;
      long val ;
   #endif
   #ifdef S4FOX
      int holdDup, holdTrail ;
   #else
      int iType ;
   #endif

   E4PARMLOW( b4, E90438 ) ;


   #ifdef S4FOX
      holdDup = b4->curDupCnt ;
      holdTrail = b4->curTrailCnt ;
   #endif

   if ( b4->tag->codeBase->doIndexVerify == 0 )
      return 0 ;

   #ifdef S4CLIPPER
      /* block internal verification... */
      for ( i = 0 ; i < b4->tag->header.keysMax ; i++ )
         for ( j = i + 1 ; j <= b4->tag->header.keysMax ; j++ )
            if ( b4->pointers[i] == b4->pointers[j] )
               #ifdef E4ANALYZE_ALL
                  return error4describe( b4->tag->codeBase, e4index, E80403, b4->tag->alias, 0, 0 ) ;
               #else
                  return error4( b4->tag->codeBase, e4index, E80403 ) ;
               #endif

      /* validate key count */
      if ( b4numKeys( b4 ) < b4->tag->header.keysHalf && b4->tag->header.root / 512 != b4->fileBlock && b4->tag->header.keysHalf > 1 )   /* allowed if only max 2 keys per index block */
         return error4describe( b4->tag->codeBase, e4index, E85709, b4->tag->alias, 0, 0 ) ;
   #endif

   /* all keys in a given block should be of the same type... - branch = 1 */
   #ifndef S4FOX
      iType = b4leaf( b4 ) ? 0 : 1 ;
   #endif

   /* key order verification */
   #ifdef S4CLIPPER
      for ( i = 0 ; i <= b4numKeys( b4 ) ; i++ )
      {
         val = b4key( b4, i )->pointer ;
         if ( ( b4key( b4, i )->pointer ? 1 : 0 ) != iType )
            #ifdef E4ANALYZE_ALL
               return error4describe( b4->tag->codeBase, E80405, e4index, b4->tag->alias, 0, 0 ) ;
            #else
               return error4( b4->tag->codeBase, e4index, E80405 ) ;
            #endif
      }
   #endif

   for ( i = 0 ; i < b4numKeys( b4 ) - 1 ; i++ )
   {
      #ifdef S4FOX
         if ( u4keycmp( b4keyKey( b4, i), b4keyKey( b4, i + 1 ), (unsigned)b4->tag->header.keyLen, (unsigned)b4->tag->header.keyLen, 0, &b4->tag->vfpInfo ) > 0 )
      #else
         if ( (*b4->tag->cmp)( b4keyKey( b4, i ), b4keyKey( b4, i + 1 ), b4->tag->header.keyLen ) > 0 )
      #endif
         #ifdef E4ANALYZE_ALL
            return error4describe( b4->tag->codeBase, e4index, E80406, b4->tag->alias, 0, 0 ) ;
         #else
            return error4( b4->tag->codeBase, e4index, E80406 ) ;
         #endif
   }

   #ifdef S4FOX
      b4->curDupCnt = holdDup ;
      b4->curTrailCnt = holdTrail ;
   #endif

   return 0 ;
}
#endif /* E4INDEX_VERIFY */
#endif /* S4INDEX_OFF */
