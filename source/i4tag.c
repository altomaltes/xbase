/* i4tag.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

/*#ifndef S4LANGUAGE */
/*#ifndef u4memcmp */
/*int S4CALL u4memcmp( S4CMP_PARM p1, S4CMP_PARM p2, size_t len ) */
/*{ */
/*   return memcmp( p1, p2, len ) ; */
/*} */
/*#endif */
/*#endif */

#ifndef S4OFF_INDEX

int tfile4unique( TAG4FILE *tag, const short int errUnique )
{
   #ifdef S4FOX
      if ( ( tag->header.typeCode & 0x01 ) || ( tag->header.typeCode & 0x04 ) )
   #else
      if ( tag->header.unique )
   #endif
         return errUnique ;
   return 0 ;
}

short int S4FUNCTION t4unique( const TAG4 *tag )
{
   #ifdef S4VBASIC
      #ifdef S4CB51
         if ( c4parm_check ( tag, 4, E40150 ) ) return 0 ;
      #else
         if ( c4parm_check ( tag, 4, E91639 ) ) return 0 ;
      #endif
   #endif

   #ifdef E4PARM_HIGH
      if ( tag == 0 )
         return error4( 0, e4parm_null, E91639 ) ;
      if ( tag->tagFile == 0 )
         return error4( 0, e4parm, E91639 ) ;
   #endif

      return tfile4unique( tag->tagFile, tag->errUnique ) ;
}

int S4FUNCTION t4uniqueSetLow( TAG4 *, const short, const char ) ;

int S4FUNCTION t4uniqueSet( TAG4 *t4, const short uniqueCode )
{
   #ifdef S4VBASIC
      if ( c4parm_check ( t4, 4, E91601 ) ) return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91601 ) ;
   #endif

   if ( uniqueCode == t4unique( t4 ) )  /* possibly both zero ok */
      return 0 ;

   /* if not zero matches, then neither should be zero */
   #ifdef E4PARM_HIGH
      #ifdef CLIENT_OR_FOX
         if ( t4unique( t4 ) == r4candidate )
         {
            if ( uniqueCode != e4candidate )
               return error4( t4->index->data->codeBase, e4parm, E91601 ) ;
            return t4uniqueSetLow( t4, uniqueCode, 1 ) ;
         }
         if ( t4unique( t4 ) == e4candidate )
         {
            if ( uniqueCode != r4candidate )
               return error4( t4->index->data->codeBase, e4parm, E91601 ) ;
            return t4uniqueSetLow( t4, uniqueCode, 1 ) ;
         }
      #endif
      if ( uniqueCode != e4unique && uniqueCode != r4unique && uniqueCode != r4uniqueContinue )
         return error4( t4->index->data->codeBase, e4parm, E91601 ) ;
   #endif

   return t4uniqueSetLow( t4, uniqueCode, 1 ) ;
}
#endif  /* S4OFF_INDEX */

#ifndef S4OFF_INDEX
int S4FUNCTION t4uniqueSetLow( TAG4 *t4, const short uniqueCode, const char doZeroCheck )
{
   #ifdef E4PARM_HIGH
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91601 ) ;
      #ifdef CLIENT_OR_FOX
         if ( t4unique( t4 ) == r4candidate )  /* can't change a candidate setting */
         {
            if ( uniqueCode != e4candidate )
               return error4( t4->index->data->codeBase, e4parm, E91601 ) ;
         }
         else
            if ( t4unique( t4 ) == e4candidate )  /* can't change a candidate setting */
            {
               if ( uniqueCode != r4candidate )
                  return error4( t4->index->data->codeBase, e4parm, E91601 ) ;
            }
            else
      #endif
      if ( uniqueCode != e4unique && uniqueCode != r4unique && uniqueCode != r4uniqueContinue && uniqueCode != 0 )
         return error4( t4->index->data->codeBase, e4parm, E91601 ) ;
   #endif

   /* verify that the tag is unqiue before setting */
   if ( doZeroCheck )
      if ( t4unique( t4 ) == 0 )
         return error4( t4->index->data->codeBase, e4parm, E81609 ) ;

   t4->errUnique = uniqueCode ;

   return 0 ;
}

long S4FUNCTION tfile4dskip( TAG4FILE *t4, long numSkip )
{
   #ifdef S4HAS_DESCENDING
      #ifdef E4PARM_LOW
         if ( t4 == 0 )
            return error4( 0, e4parm_null, E91642 ) ;
      #endif

      if ( t4->header.descending )
         return -tfile4skip( t4, -numSkip ) ;
      else
   #endif /* S4HAS_DESCENDING */
      return tfile4skip( t4, numSkip ) ;
}

int tfile4outOfDate( TAG4FILE *t4 )
{
   #ifndef S4SINGLE
      time_t oldTime ;
   #endif

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   #ifdef S4SINGLE
      return error4describe( t4->codeBase, e4index, E81605, tfile4alias( t4 ), 0, 0 ) ;
   #else
      /* first make sure we are at a potential read conflict situation (otherwise must be corrupt file) */
      #ifdef S4CLIPPER
         if ( tfile4lockTest( t4 ) )
      #else
         if ( index4lockTest( t4->indexFile ) )
      #endif
         return error4describe( t4->codeBase, e4index, E81606, tfile4alias( t4 ), 0, 0 ) ;

      /* wait a second and refresh */
      time( &oldTime) ;
      while ( time( (time_t *)0 ) <= oldTime) ;
      tfile4freeAll( t4 ) ;
      #ifdef S4CLIPPER
         return file4refresh( &t4->file ) ;
      #else
         return file4refresh( &t4->indexFile->file ) ;
      #endif
   #endif
}

/* uses parent to determine if an inconsistancy has arisen */
/* return <0 = error, 0 = success, 1 = inconsistant */
int i4readBlock( FILE4 *file, const long blockNo, B4BLOCK *parent, B4BLOCK *b4 )
{
   int rc ;
   TAG4FILE *tag ;
   CODE4 *c4 ;
   FILE4LONG pos ;
   #ifndef S4CLIPPER
      INDEX4FILE *i4file ;
   #endif
   #ifdef S4BYTE_SWAP
      char *swapPtr ;
      int i ;
      S4LONG longVal ;
      #ifdef S4FOX
         short shortVal ;
      #endif
   #endif
   #ifdef S4MDX
      int parOn, blkOn, eq ;
   #endif

   #ifdef E4PARM_LOW
      if ( file == 0 || blockNo < 0 || b4 == 0 )
         return error4( 0, e4parm, E91604 ) ;
   #endif

   tag = b4->tag ;
   #ifndef S4CLIPPER
      i4file = tag->indexFile ;
   #endif
   c4 = tag->codeBase ;

   #ifdef S4MDX
      /* i.e. if ndx or mdx */
      #ifndef S4OFF_OPTIMIZE
         i4file->readBlockTag = tag ;
      #endif
      file4longAssign( pos, I4MULTIPLY * blockNo, 0 ) ;
      rc = file4readAllInternal( file, pos, &b4->nKeys, i4file->header.blockRw ) ;
      #ifndef S4OFF_OPTIMIZE
         i4file->readBlockTag = 0 ;
      #endif
      if ( rc < 0 )
         return -1 ;
      #ifdef S4BYTE_SWAP
         /* swap the numKeys value */
         b4->nKeys = x4reverseShort( (void *)&b4->nKeys ) ;

         /* position swapPtr at beginning of B4KEY's */
         swapPtr = (char *)&b4->nKeys ;
         swapPtr += 6 + sizeof(short) ;

         /* move through all B4KEY's to swap 'long' */
         for ( i = 0 ; i < (int)b4numKeys( b4 ) ; i++ )
         {
            longVal = x4reverseLong( (void *)swapPtr ) ;
            memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
            swapPtr += tag->header.groupLen ;
         }

         /* mark lastPointer */
         if ( !b4leaf( b4 ) )
         {
            longVal = x4reverseLong( (void *)swapPtr ) ;
            memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
         }
      #endif  /* S4BYTE_SWAP */
   #endif

   rc = 0 ;

   #ifdef S4FOX
      #ifndef S4OFF_OPTIMIZE
         i4file->readBlockTag = tag ;
      #endif
      file4longAssign( pos, I4MULTIPLY * blockNo, 0 ) ;
      rc = file4readAllInternal( file, pos, &b4->header, B4BLOCK_SIZE ) ;
      #ifndef S4OFF_OPTIMIZE
         i4file->readBlockTag = 0 ;
      #endif
      if ( rc < 0 )
         return error4stack( c4, (short)rc, E91604 ) ;

         #ifdef S4BYTE_SWAP
            b4->header.nodeAttribute = x4reverseShort( (void *)&b4->header.nodeAttribute ) ;
            b4->header.nKeys = x4reverseShort( (void *)&b4->header.nKeys ) ;
            b4->header.leftNode = x4reverseLong( (void *)&b4->header.leftNode ) ;
            b4->header.rightNode = x4reverseLong( (void *)&b4->header.rightNode ) ;

            /* if b4 is a leaf */
            if (b4->header.nodeAttribute >= 2 )
            {
               b4->nodeHdr.freeSpace = x4reverseShort( (void *)&b4->nodeHdr.freeSpace ) ;
               longVal = x4reverseLong( (void *)&b4->nodeHdr.recNumMask[0] ) ;
               memcpy( (void *)&b4->nodeHdr.recNumMask[0], (void *)&longVal, sizeof(S4LONG) ) ;
            }
            else /* if b4 is a branch */
            {
               shortVal = tag->header.keyLen + sizeof(S4LONG) ;
               /* position swapPtr to end of first key expression */
               swapPtr = (char *) &b4->nodeHdr.freeSpace + tag->header.keyLen ;

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
         #endif
   #endif

   #ifdef S4CLIPPER
      #ifndef S4OFF_OPTIMIZE
         tag->readBlockTag = tag ;
      #endif
      file4longAssign( pos, blockNo, 0 ) ;
      rc = file4readAllInternal( file, pos, &b4->nKeys, B4BLOCK_SIZE ) ;
      #ifndef S4OFF_OPTIMIZE
         tag->readBlockTag = 0 ;
      #endif
      if ( rc < 0 )
         return error4stack( c4, rc, E91604 ) ;
      #ifdef S4BYTE_SWAP
         index4swapBlockClipper(&b4->nKeys, tag->header.keysMax, tag->header.groupLen ) ;
      #endif
      b4->fileBlock = blockNo/512 ;
   #endif

   if ( c4->doIndexVerify == 0 )
      return rc ;

   if ( b4numKeys( b4 ) == 0 )   /* added to free list... therefore bad */
      #ifdef S4CLIPPER
         if ( tag->header.keysMax > 2 )  /* remote possibility of 0 keys if max = 2 and in a reindex mode */
            if ( tag->header.root != b4->fileBlock * I4MULTIPLY )  /* b4 has no keys but is not the root, must be a problem... */
      #else
         if ( tag->header.root != blockNo )  /* b4 has no keys but is not the root, must be a problem... */
      #endif
      return 1 ;

   if ( parent != 0 )  /* check consistancy */
   {
      #ifdef S4MDX
         eq = 0 ;
         blkOn = b4numKeys( b4 ) - 1 ;
         if ( parent->keyOn >= b4numKeys( parent ) )
         {
            parOn = b4numKeys( parent ) - 1 ;
            eq = 1 ;
         }
         else
            parOn = parent->keyOn ;
         if ( !b4leaf( b4 ) )
         {
            blkOn = 0 ;
            if ( eq != 1 )
            {
               if ( parOn == 0 )
                  eq = 2 ;
               else
               {
                  eq = 1 ;
                  parOn-- ;
               }
            }
         }

         switch( eq )
         {
            case 0:
               if ( tag->cmp( b4keyKey( parent, parOn ), b4keyKey( b4, blkOn ), tag->header.keyLen  ) != 0 )
                  rc = 1 ;
               break ;
            case 1:
               if ( tag->cmp( b4keyKey( parent, parOn ), b4keyKey( b4, blkOn ), tag->header.keyLen  ) > 0 )
                  rc = 1 ;
               break ;
            case 2:
               if ( tag->cmp( b4keyKey( parent, parOn ), b4keyKey( b4, blkOn ), tag->header.keyLen  ) < 0 )
                  rc = 1 ;
               break ;
            #ifdef E4ANALYZE
               default:
                  return error4( c4, e4index, E81601 ) ;
            #endif
         }
      #endif

      #ifdef S4FOX
         if ( rc == 0 )
            if ( b4recNo( parent, parent->keyOn) != b4recNo( b4, b4numKeys( b4 ) - 1 ) )
               rc = 1 ;
      #endif
      if ( rc == 0 )
      {
         #ifdef E4INDEX_VERIFY
            if ( b4verify( b4 ) == -1 )
               return error4describe( c4, e4index, E91604, tag->alias, 0, 0 ) ;
            if ( b4verify( parent ) == -1 )
               return error4describe( c4, e4index, E91604, tag->alias, 0, 0 ) ;
         #endif
      }
      #ifdef S4CLIPPER
         if ( rc == 0 )
         {
            if ( parent->keyOn < b4numKeys( parent ) )
            {
               if ( tag->cmp( b4keyKey( parent, parent->keyOn ), b4keyKey( b4, b4numKeys( b4 ) - 1 ), tag->header.keyLen  ) < 0 )
                  rc = 1 ;
            }
            else
            {
               if ( b4numKeys( b4 ) != 0 && b4numKeys( parent ) != 0 )   /* again, special instance whereby no key to compare with, so assume correct */
                  if ( tag->cmp( b4keyKey( parent, parent->keyOn - 1 ), b4keyKey( b4, 0 ), tag->header.keyLen  ) > 0 )
                     rc = 1 ;
            }
         }
      #endif
   }

   if ( rc == 1 )
      #ifdef S4CLIPPER
         if ( tfile4lockTest( tag ) == 1 )  /* corrupt */
      #else
         if ( index4lockTest( i4file ) == 1 )  /* corrupt */
      #endif
         return error4describe( c4, e4index, E81607, tag->alias, 0, 0 ) ;

   return rc ;
}

int S4FUNCTION tfile4exprKey( TAG4FILE *tag, unsigned char **ptrPtr )
{
   int len ;
   #ifdef S4CLIPPER
      int oldDec ;
   #endif

   #ifdef E4PARM_LOW
      if ( tag == 0 || ptrPtr == 0)
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   #ifdef S4CLIPPER
      oldDec = tag->codeBase->decimals ;
      tag->codeBase->decimals = tag->header.keyDec ;
   #endif

   len = expr4key( tag->expr, (char **)ptrPtr, tag ) ;

   #ifdef S4CLIPPER
      tag->codeBase->decimals = oldDec ;
   #endif

   return len ;
}

int S4FUNCTION tfile4go( TAG4FILE *t4, const unsigned char *ptr, const long recNum, const int goAdd )
{
   int rc ;
   #ifdef S4HAS_DESCENDING
      int oldDesc ;
   #endif

   #ifdef E4PARM_LOW
      if ( t4 == 0 || ptr == 0 )
         return error4( 0, e4parm, E91642 ) ;
      if ( recNum <= 0L )
         return error4( 0, e4parm, E91642 ) ;
   #endif

   #ifdef S4HAS_DESCENDING
      oldDesc = t4->header.descending ;
      t4->header.descending = 0 ;
   #endif

   rc = tfile4go2( t4, ptr, recNum, goAdd ) ;

   #ifdef S4HAS_DESCENDING
      t4->header.descending = (short)oldDesc ;
   #endif

   return rc ;
}

int tfile4empty( TAG4FILE *tag )
{
   B4BLOCK *b4 ;

   #ifdef E4PARM_LOW
      if ( tag == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   b4 = tfile4block( tag ) ;
   if ( b4 == 0 )
      return 1 ;
   if ( b4numKeys( b4 ) == 0L )
      return 1 ;

   return 0 ;
}

#ifndef S4CLIPPER
#ifdef S4MDX
void S4FUNCTION tfile4descending( TAG4FILE *tag, const unsigned short int setting )
{
} /* A do nothing function to facilitate index independent OLEDB dll*/
#endif
#ifdef S4FOX
#ifdef S4HAS_DESCENDING
void S4FUNCTION tfile4descending( TAG4FILE *tag, const unsigned short int setting )
{
   #ifdef E4PARM_LOW
      if ( tag == 0 )
      {
         error4( 0, e4parm_null, E91642 ) ;
         return ;
      }
   #endif

   tag->header.descending = setting ;
}
#endif

#ifdef S4VFP_KEY
/* returns 1 (true) if tag uses double-length keys--otherwise 0 (false) is returned */
int tfile4vfpKey( TAG4FILE *t4 )
{
   switch( t4->vfpInfo.sortType )
   {
      case sort4machine:
         return 0 ;
      case sort4general:
         return 1 ;
      default:
         return error4( 0, e4parm, E91642 ) ;
   } ;
}
#endif

int tfile4setCollatingSeq( TAG4FILE *t4, const int type )
{
   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   switch ( type )
   {
      case sort4machine:
         memset( t4->header.sortSeq, 0, 8 ) ;
         t4->vfpInfo.tablePtr = 0 ;   /* the machine collating sequence isn't translated */
         t4->vfpInfo.compPtr = 0 ;
         break ;
   #ifdef S4GENERAL
      case sort4general:
         memcpy( t4->header.sortSeq, "GENERAL\0", 8 ) ;
         t4->vfpInfo.tablePtr = v4general ;
         t4->vfpInfo.compPtr = v4generalComp ;
         break ;
   #endif
      default:
         return error4( t4->codeBase, e4parm, E91642 ) ;
   } ;

   t4->vfpInfo.sortType = type ;
   return 0 ;
}

int tfile4setCodePage( TAG4FILE *t4, const int type )
{
   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   /* only if the tag is general should we verify the codepage */
   if ( t4->vfpInfo.sortType == sort4general )
   {
      switch ( type )
      {
         case cp0:
            t4->vfpInfo.cpPtr = CodePage_1252 ;   /* set codepage 1252 as the default codepage */
            break ;
         #ifdef S4CODEPAGE_1252
            case cp1252:
               t4->vfpInfo.cpPtr = CodePage_1252 ;
               break ;
         #endif
         #ifdef S4CODEPAGE_437
            case cp437:
               t4->vfpInfo.cpPtr = CodePage_437 ;
               break ;
         #endif
         default:
            return error4( t4->codeBase, e4parm, E81610 ) ;
      }
      t4->vfpInfo.codePage = type ;
   }
   else
   {
      t4->vfpInfo.cpPtr = 0 ;
      t4->vfpInfo.codePage = cp0 ;
   }

   return 0 ;
}

void t4strToFox( char *result, const char *inputPtr, const int inputPtrLen )
{
   t4dblToFox( result, c4atod( inputPtr, inputPtrLen ) ) ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
void t4dtstrToFox( char *result, const char *inputPtr, const int inputPtrLen )
{
   t4dblToFox( result, (double) date4long( inputPtr ) ) ;
}

int tfile4rlBottom( TAG4FILE *t4 )
{
   int rc, rc2 ;
   B4BLOCK *blockOn ;

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
         return error4stack( t4->codeBase, (short)rc, E91642 ) ;

      if ( rc != 2 )
      {
         if ( b4numKeys( tfile4block( t4 ) ) != 0 )
         {
            rc2 = b4go( tfile4block( t4 ), (long)b4numKeys( tfile4block( t4 ) ) - 1L ) ;
            if ( rc2 < 0 )
               return error4stack( t4->codeBase, rc2, E91642 ) ;
            do
            {
               rc = tfile4down( t4 ) ;
               if ( rc < 0 )
                  return error4stack( t4->codeBase, (short)rc, E91642 ) ;
               rc2 = b4go( tfile4block( t4 ), (long)b4numKeys( tfile4block( t4 ) ) - 1L ) ;
               if ( rc2 < 0 )
                  return error4stack( t4->codeBase, (short)rc2, E91642 ) ;
            } while ( rc == 0 ) ;
         }
      }

      if ( rc == 2 )   /* failed due to read while locked */
      {
         rc2 = tfile4outOfDate( t4 ) ;
         if ( rc2 < 0 )
            return error4stack( t4->codeBase, (short)rc2, E91642 ) ;
      }
   } while ( rc == 2 ) ;

   blockOn = tfile4block( t4 ) ;

   #ifdef E4ANALYZE
      if ( blockOn == 0 )
         return error4( t4->codeBase, e4result, E91642 ) ;
   #endif

   if ( blockOn->keyOn > 0 )
   {
      b4goEof( blockOn ) ;
      blockOn->keyOn-- ;  /* update keyOn after going to last spot */
   }

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

#else /*  if not S4FOX  */

int S4FUNCTION tfile4bottom( TAG4FILE *t4 )
{
   int rc, rc2 ;
   B4BLOCK *blockOn ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   rc = 2 ;

   while ( rc == 2 )
   {
      rc = tfile4upToRoot( t4 ) ;
      if ( rc < 0 )
         return error4stack( t4->codeBase, rc, E91642 ) ;

      if ( rc != 2 )
      {
         b4goEof( tfile4block(t4) ) ;
         do
         {
            rc = tfile4down( t4 ) ;
            if ( rc < 0 )
               return error4stack( t4->codeBase, rc, E91642 ) ;
            b4goEof( tfile4block( t4 ) ) ;
         } while ( rc == 0 ) ;
      }

      if ( rc == 2 )   /* failed due to read while locked */
      {
         rc2 = tfile4outOfDate( t4 ) ;
         if ( rc2 < 0 )
            return error4stack( t4->codeBase, rc2, E91642 ) ;
      }
   }

   blockOn = tfile4block( t4 ) ;
   #ifdef E4ANALYZE
      if ( blockOn == 0 )
         return error4( t4->codeBase, e4result, E91642 ) ;
   #endif

   if ( blockOn->keyOn > 0 )
      blockOn->keyOn = b4numKeys( blockOn ) - 1 ;

   return 0 ;
}

#endif  /* ifdef S4FOX */

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
         error4( t4->codeBase, e4index, E91642 ) ;
         return 0 ;
      }
   #endif
   return (B4BLOCK *)t4->blocks.lastNode ;
}

/* Returns  2 - cannot go down due to out of date blocks 1 - Cannot move down; 0 - Success; -1 Error */
int tfile4down( TAG4FILE *t4 )
{
   long blockDown ;
   B4BLOCK *blockOn, *popBlock, *newBlock, *parent ;
   INDEX4FILE *i4 ;
   int rc ;
   FILE4LONG pos ;
   #ifdef S4ADVANCE_READ
      long nextBlockDown ;
   #endif

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   i4 = t4->indexFile ;

   blockOn = (B4BLOCK *)t4->blocks.lastNode ;

   if ( blockOn == 0 )    /* Read the root block */
   {
      if ( t4->header.root <= 0L )
      {
         file4longAssign( pos, t4->headerOffset, 0 ) ;
         rc = file4readAllInternal( &i4->file, pos, &t4->header.root,sizeof(t4->header.root)) ;
         if ( rc < 0 )
            return error4stack( t4->codeBase, (short)rc, E91642 ) ;
         #ifdef S4BYTE_SWAP
            t4->header.root = x4reverseLong( (void *)&t4->header.root ) ;
         #endif
      }
      blockDown = t4->header.root ;
   }
   else
   {
      if ( b4leaf( blockOn ) )
         return 1 ;
      #ifdef S4FOX
         #ifdef S4DATA_ALIGN
            memcpy( (void *)&blockDown,(void *)(((unsigned char *)&blockOn->nodeHdr)
                    + (blockOn->keyOn+1)*(2*sizeof(S4LONG) + t4->header.keyLen) - sizeof(S4LONG)), sizeof(S4LONG) ) ;
            blockDown = x4reverseLong( (void *)&blockDown ) ;

         #else
            blockDown = x4reverseLong( (void *)( ((unsigned char *)&blockOn->nodeHdr)
                    + (blockOn->keyOn+1)*(2*sizeof(S4LONG) + t4->header.keyLen) - sizeof(S4LONG) ) ) ;
         #endif
         #ifdef S4ADVANCE_READ
            if ( b4numKeys( blockOn ) >= (blockOn->keyOn + 1) )
            {
               #ifdef S4DATA_ALIGN
                  memcpy( (void *)&nextBlockDown,(void *)(((unsigned char *)&blockOn->nodeHdr)
                          + (blockOn->keyOn+2)*(2*sizeof(S4LONG) + t4->header.keyLen) - sizeof(S4LONG)), sizeof(S4LONG) ) ;
                  nextBlockDown = x4reverseLong( (void *)&blockDown ) ;
               #else
                  nextBlockDown = x4reverseLong( (void *)( ((unsigned char *)&blockOn->nodeHdr)
                          + (blockOn->keyOn+2)*(2*sizeof(S4LONG) + t4->header.keyLen) - sizeof(S4LONG) ) ) ;
               #endif
            }
            else
               nextBlockDown = -1L ;
         #endif
      #else
         blockDown = b4key(blockOn,blockOn->keyOn)->num ;
         #ifdef S4ADVANCE_READ
            if ( b4numKeys( blockOn ) >= (blockOn->keyOn + 1) )
               nextBlockDown = b4key(blockOn,blockOn->keyOn+1)->num ;
            else
               nextBlockDown = -1L ;
         #endif
      #endif
      #ifdef E4ANALYZE
         if ( blockDown <= 0L )
            return error4( t4->codeBase, e4index, E81602 ) ;
      #endif
   }

   /* Get memory for the new block */
   popBlock = (B4BLOCK *)l4pop( &t4->saved ) ;
   if ( popBlock == 0 )
   {
      newBlock = b4alloc( t4, blockDown) ;
      if ( newBlock == 0 )
         return error4stack( t4->codeBase, e4memory, E91642 ) ;
   }
   else
      newBlock = popBlock ;

   parent = (B4BLOCK *)l4last( &t4->blocks ) ;
   l4add( &t4->blocks, newBlock ) ;

   if ( popBlock == 0 || newBlock->fileBlock != blockDown )
   {
      #ifndef S4OFF_WRITE
         rc = b4flush( newBlock ) ;
         if ( rc < 0 )
            return error4stack( t4->codeBase, (short)rc, E91642 ) ;
      #endif

      rc = i4readBlock( &i4->file, blockDown, parent, newBlock ) ;
      if ( rc < 0 )
         return error4stack( t4->codeBase, (short)rc, E91642 ) ;

      #ifndef S4OFF_OPTIMIZE
         #ifdef S4ADVANCE_READ
            if ( nextBlockDown != -1L )
               if ( i4->dataFile->hiPrio == -2 )  /* tag-skipping */
                  if ( b4leaf( newBlock ) )  /* pre-read the next one */
                     #ifdef S4FOX
                        opt4advanceReadBuf( &i4->file, I4MULTIPLY * nextBlockDown, B4BLOCK_SIZE ) ;
                     #else
                        opt4advanceReadBuf( &i4->file, I4MULTIPLY * nextBlockDown, i4->header.blockRw ) ;
                     #endif
         #endif
      #endif

      if ( rc == 1 )
      {
         l4remove( &t4->blocks, newBlock ) ;
         l4add( &t4->saved, newBlock ) ;
         return 2 ;
      }

      newBlock->fileBlock = blockDown ;

      #ifdef S4FOX
         newBlock->builtOn = -1 ;
      #endif

      /* flush blocks, don't delete */
      for( blockOn = 0 ;; )
      {
         blockOn = (B4BLOCK *)l4next(&t4->saved,blockOn) ;
         if ( blockOn == 0 )
            break ;
         #ifndef S4OFF_WRITE
            rc = b4flush( blockOn ) ;
            if ( rc < 0 )
               return error4stack( t4->codeBase, (short)rc, E91642 ) ;
         #endif
         blockOn->fileBlock = -1 ;   /* make it invalid */
      }
   }

   #ifdef S4FOX
      return b4top( newBlock ) ;
   #else
      newBlock->keyOn = 0 ;
      return 0 ;
   #endif
}

int S4FUNCTION tfile4eof( TAG4FILE *t4 )
{
   B4BLOCK *b4 ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   b4 = tfile4block( t4 ) ;
   if ( b4 == 0 )
      return error4stack( 0, e4result, E91642 ) ;

   #ifdef S4FOX
      return( (b4->keyOn >= b4numKeys( b4 ) ) || (b4numKeys( b4 ) == 0) ) ;
   #else
      return( b4->keyOn >= b4numKeys( b4 ) ) ;
   #endif
}

#ifndef S4OFF_WRITE
int tfile4update( TAG4FILE *t4 )
{
   B4BLOCK *blockOn ;
   int rc ;
   FILE4LONG pos ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   for( blockOn = 0 ;; )
   {
      blockOn = (B4BLOCK *)l4next(&t4->saved,blockOn) ;
      if ( blockOn == 0 )
         break ;
      rc = b4flush( blockOn ) ;
      if ( rc < 0 )
         return error4stack( t4->codeBase, (short)rc, E91642 ) ;
   }

   for( blockOn = 0 ;; )
   {
      blockOn = (B4BLOCK *)l4next(&t4->blocks,blockOn) ;
      if ( blockOn == 0 )
         break ;
      rc = b4flush( blockOn ) ;
      if ( rc < 0 )
         return error4stack( t4->codeBase, (short)rc, E91642 ) ;
   }

   if ( t4->rootWrite )
   {
      #ifdef S4BYTE_SWAP
         t4->header.root = x4reverseLong( (void *)&t4->header.root ) ;
      #endif

      file4longAssign( pos, t4->headerOffset, 0 ) ;
      rc = file4writeInternal( &t4->indexFile->file, pos, &t4->header.root, sizeof(t4->header.root)) ;
      if ( rc < 0 )
         return error4stack( t4->codeBase, (short)rc, E91642 ) ;

      #ifdef S4BYTE_SWAP
         t4->header.root = x4reverseLong( (void *)&t4->header.root ) ;
      #endif

      t4->rootWrite = 0 ;
   }

   return 0 ;
}
#endif  /* S4OFF_WRITE */

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
   int rc ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   #ifndef S4OFF_WRITE
      rc = tfile4update( t4 ) ;
      if ( rc < 0 )
         return error4stack( t4->codeBase, (short)rc, E91642 ) ;
   #endif

   for ( ;; )
   {
      blockOn = (B4BLOCK *)l4pop( &t4->saved ) ;
      if ( blockOn == 0 )
         return 0 ;
      #ifndef S4OFF_WRITE
         rc = b4flush( blockOn ) ;
         if ( rc < 0 )
            return error4stack( t4->codeBase, (short)rc, E91642 ) ;
      #endif
      rc = b4free( blockOn ) ;
      if ( rc < 0 )
         return error4stack( t4->codeBase, (short)rc, E91642 ) ;
   }
}

/* addGo true if performing for add purposes instead of seek purposes */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int tfile4go2( TAG4FILE *t4, const unsigned char *ptr, const long recNum, const int addGo )
{
   #ifdef S4FOX
      B4BLOCK *blockOn ;
      int rc, rc2, rc3, kLen, allBlank, i, blnkCount ;
      long rec ;
      char hasSkipped ;
   #else
      int rc, rc3 ;
      long rec, recSave, tagRec ;
      B4BLOCK *blockOn ;
   #endif

   #ifdef E4PARM_LOW
      if ( t4 == 0 || ptr == 0 )
         return error4( 0, e4parm, E91642 ) ;
      if ( recNum <= 0L )
         return error4( 0, e4parm, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   #ifdef S4FOX
      kLen = t4->header.keyLen ;
      rec = x4reverseLong( (void *)&recNum ) ;

      do
      {
         /* Do initial search, moving up only as far as necessary */
         rc2 = tfile4upToRoot( t4 ) ;
         if ( rc2 < 0 )
            return error4stack( t4->codeBase, (short)rc2, E91642 ) ;

         if ( rc2 != 2 )
         {
            for(;;) /* Repeat until found */
            {
               blockOn = (B4BLOCK *)t4->blocks.lastNode ;
               #ifdef E4ANALYZE
                  if ( blockOn == 0 )
                     return error4( t4->codeBase, e4index, E91642 ) ;
               #endif

               if ( b4leaf(blockOn) )
               {
                  rc = b4seek( blockOn, (char *)ptr, kLen ) ;

                  if ( rc )    /* leaf seek did not end in perfect find */
                     return rc ;

                  /* now do the seek for recno on the leaf block */
                  blockOn = (B4BLOCK *)t4->blocks.lastNode ;

/*                  if ( addGo == 0 )
                  {
                     rec = tfile4recNo( t4 ) ;
                     if ( rec < 0 )
                        return error4stack( t4->codeBase, (short)rec, E91642 ) ;
                     if ( rec == recNum )
                        return 0 ;
                  }   */

                  hasSkipped = 0 ;
                  if ( x4trailCnt( blockOn, blockOn->keyOn ) == t4->header.keyLen )
                     allBlank = 1 ;
                  else
                  {
                     if ( blockOn->keyOn == 0 )  /* could still be all blanks - 1st key */
                     {
                        allBlank = 1 ;
                        for ( i = 0 ; i < t4->header.keyLen ; i++ )
                        {
                           if ( ptr[i] != ' ' )
                           {
                              allBlank = 0 ;
                              break ;
                           }
                        }
                     }
                     else
                        allBlank = 0 ;
                  }
                  for(;;)
                  {
                     rec = tfile4recNo( t4 ) ;
                     if ( rec < 0 )
                        return error4stack( t4->codeBase, (short)rec, E91642 ) ;
                     if ( addGo == 0 )
                        if ( rec == recNum )
                           return 0 ;
                     if ( rec >= recNum )
                     {
                        if ( !hasSkipped )
                           blockOn->curDupCnt = x4dupCnt( blockOn, blockOn->keyOn ) ;
                        return r4found ;
                     }

                     hasSkipped = 1 ;

                     rc = (int)tfile4skip( t4, 1L ) ;
                     if ( rc == -1 )
                        return -1 ;
                     if ( rc == 0 )
                     {
                        b4goEof( tfile4block( t4 ) ) ;
                        return r4found ;
                     }

                     /* AS 05/14/98 - 1 instance where this won't work:
                           < blank key, suddenly goes to all-blanks, then
                            all blank is true, should stop but doesn't
                     */
                     blnkCount = x4trailCnt( blockOn, blockOn->keyOn ) ;
                     if ( x4dupCnt( blockOn, blockOn->keyOn ) + blnkCount != t4->header.keyLen )
                     {
                       /* case where key changed --> need to go back one to be on r4found value*/
                        return r4found ;
                     }
                     if ( blnkCount == t4->header.keyLen && !allBlank )
                     {
                        /* done because our search key was < blank, but we found blank */
                        return r4found ;
                     }
                  }
               }
               else
               {
                  rc = b4rBrseek( blockOn, (char *)ptr, kLen, rec ) ;
                  if ( rc == 0 && t4->header.typeCode & 0x01 )
                     if ( b4recNo( tfile4block(t4), tfile4block(t4)->keyOn ) != recNum )
                        return r4found ;
               }

               rc2 = tfile4down( t4 ) ;
               if ( rc2 < 0 )
                  return error4stack( t4->codeBase, (short)rc2, E91642 ) ;
               if ( rc2 == 2 )
               {
                  rc3 = tfile4outOfDate( t4 ) ;
                  if ( rc3 < 0 )
                     return error4stack( t4->codeBase, (short)rc3, E91642 ) ;
                  break ;
               }
            }
         }
      } while ( rc2 == 2 ) ;
      return 0 ;
   #endif

   #ifndef S4FOX
      rc = tfile4seek( t4, ptr, t4->header.keyLen ) ;
      if ( rc )
         return rc ;
      recSave = tfile4recNo( t4 ) ;
      if ( recSave == recNum )
         return 0 ;

      /* else find the far end, and then skip back to where now or find */
      tfile4upToRoot( t4 ) ;
      for( ;; )
      {
         blockOn = (B4BLOCK *)t4->blocks.lastNode ;
         #ifdef E4ANALYZE
            if ( blockOn == 0 )
               return error4( t4->codeBase, e4index, E91642 ) ;
         #endif
         rc = b4seek( blockOn, (const char *)ptr, t4->header.keyLen ) ;
         while( rc == 0 )  /* perfect find, check next */
         {
            if ( b4skip( blockOn, 1L ) == 0 )
               break ;
            if ( (*t4->cmp)( b4keyKey( blockOn, blockOn->keyOn ), ptr, t4->header.keyLen ) != 0 )
               rc = r4after ;  /* stop loop and avoid rare return if b4leaf below */
         }

         if ( b4leaf( blockOn ) )
         {
            tagRec = tfile4recNo( t4 ) ;
            if ( tagRec == recNum && rc == 0 )   /* found */
               return 0 ;
            if ( tagRec == recSave )   /* didn't move */
               return r4found ;
            break ;
         }
         rc3 = tfile4down( t4 ) ;
         if ( rc3 < 0 )
            return error4stack( t4->codeBase, rc3, E91642 ) ;
      }

      for(;;)
      {
         rc = (int)b4skip( tfile4block( t4 ), -1L ) ;
         if ( rc == 0 ) /* try previous tag */
         {
            if ( tfile4skip( t4, -1L ) == 0 )
               return r4found ;
         }
         rec = tfile4recNo( t4 ) ;
         if ( rec == recSave )   /* failed to find */
            return r4found ;
         if ( rec == recNum )
            return 0 ;
      }
   #endif
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
   #ifdef E4ANALYZE
      if ( b4 == 0 )
      {
         error4( t4->codeBase, e4index, E91642 ) ;
         return 0 ;
      }
      #ifdef S4FOX
         if ( b4->keyOn >= b4->header.nKeys )     /* invalid key */
      #else
         if ( b4->keyOn >= b4->nKeys )     /* invalid key */
      #endif
      {
         error4( t4->codeBase, e4index, E91642 ) ;
         return 0 ;
      }
   #endif
   return b4key( b4, b4->keyOn ) ;
}

/* returns 0 if tag is empty */
PUBLIC long S4FUNCTION tfile4recNo( TAG4FILE *t4 )
{
   B4BLOCK *blockOn ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   blockOn = (B4BLOCK *)t4->blocks.lastNode ;
   if ( blockOn == 0 )
      return -2L ;

   if ( b4numKeys( blockOn ) == 0 )
      return 0 ;
   #ifdef S4FOX
      if ( blockOn->keyOn >= b4numKeys( blockOn ) )
         return -1 ;
   #else
      if ( !b4leaf( blockOn ) )
         return -2L ;
   #endif

   return b4recNo( blockOn, blockOn->keyOn ) ;
}

int S4FUNCTION tfile4seek( TAG4FILE *t4, const void *ptr, const int lenIn )
{
   int rc, rc2, lenPtr  ;
   B4BLOCK *blockOn ;
   #ifdef S4FOX
      int incPos = 0 ;
      int dSet ;
      unsigned char *cPtr ;
      #ifdef S4VFP_KEY
         int tmpLenPtr ;
         int vfpKey = tfile4vfpKey( t4 ) ;
      #endif
   #endif

   #ifdef S4FOX
      dSet = 0 ;
      cPtr = (unsigned char *)ptr ;
   #endif
   lenPtr = lenIn ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 || ptr == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
      if ( lenPtr != t4->header.keyLen && tfile4type( t4 ) != r4str && tfile4type( t4 ) != r5wstr )
         return error4( t4->codeBase, e4parm, E91642 ) ;
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return e4codeBase ;

   if ( lenPtr > t4->header.keyLen )
      lenPtr = t4->header.keyLen ;
   #ifdef S4VFP_KEY
      tmpLenPtr = lenPtr ;
   #endif

   #ifdef S4FOX
      if ( t4->header.descending )   /* look for current item less one: */
      {
         #ifdef S4VFP_KEY
            if ( vfpKey && tfile4type( t4 ) == r4str )
            {
               for( incPos = lenPtr-1 ; cPtr[incPos] < 10 ; incPos-- ) ;
               if ( incPos >= 0 )
                  cPtr[incPos]++ ;
               tmpLenPtr = incPos + 1 ;
               incPos-- ;   /* allows cPtr to be reset correctly later on */
               dSet = 1 ;
            }
            else
         #endif
               for( incPos = lenPtr-1 ; dSet == 0 && incPos >=0 ; incPos-- )
                  if ( cPtr[incPos] != 0xFF )
                  {
                     cPtr[incPos]++ ;
                     dSet = 1 ;
                  }
                  else
                     cPtr[incPos] = 0 ;  /* make sure increment by 1 total only, not by a whole order of magnitude */
      }
   #endif

   rc = 3 ;
   for( ;; ) /* Repeat until found */
   {
      while ( rc >= 2 )
      {
         if ( rc == 2 )
         {
            rc2 = tfile4outOfDate( t4 ) ;
            if ( rc2 < 0 )
               return error4stack( t4->codeBase, (short)rc2, E91642 ) ;
         }
         rc = tfile4upToRoot( t4 ) ;
         if ( rc < 0 )
            return error4stack( t4->codeBase, (short)rc, E91642 ) ;
      }
      blockOn = (B4BLOCK *)t4->blocks.lastNode ;
      #ifdef E4ANALYZE
         if ( blockOn == 0 )
            return error4( t4->codeBase, e4index, E91642 ) ;
      #endif

      #ifdef S4VFP_KEY
         rc = b4seek( blockOn, (char *)ptr, tmpLenPtr ) ;
      #else
         rc = b4seek( blockOn, (char *)ptr, lenPtr ) ;
      #endif

      if ( rc < 0 )
         return error4stack( t4->codeBase, (short)rc, E91642 ) ;
      if ( b4leaf( blockOn ) )
         break ;

      rc = tfile4down( t4 ) ;
      if ( rc < 0 )
         return error4stack( t4->codeBase, (short)rc, E91642 ) ;
   }

   #ifdef S4FOX
      if ( t4->header.descending )   /* must go back one! */
      {
         cPtr[incPos+1]-- ; /* reset the searchPtr ; */
         if ( dSet )
         {
            if ( b4numKeys( blockOn ) == 0 )
               rc = 0 ;
            else
            {
               if ( blockOn->keyOn == 0 )   /* special case, must balance tree */
               {
                  rc = (int)tfile4skip( t4, -1L ) ;
                  if ( rc == 0 )
                     rc2 = tfile4top( t4 ) ;
                  else
                     rc2 = tfile4go( t4, tfile4keyData( t4 )->value, tfile4recNo( t4 ), 0 ) ;
                  if ( rc2 < 0 )
                     return error4stack( t4->codeBase, (short)rc2, E91642 ) ;
               }
               else
                  rc = (int)tfile4skip( t4, -1L ) ;
            }
            if ( rc == 0L )  /* bof = eof condition */
            {
               b4goEof( blockOn ) ;
               rc = r4eof ;
            }
            else
            {
               rc2 = b4go( tfile4block( t4 ), (long)tfile4block( t4 )->keyOn ) ;
               if ( rc2 < 0 )
                  return error4stack( t4->codeBase, (short)rc2, E91642 ) ;
               rc2 = u4keycmp( (void *)b4keyKey( tfile4block( t4 ), tfile4block( t4 )->keyOn ), ptr, (unsigned)lenPtr, (unsigned)t4->header.keyLen, 0, &t4->vfpInfo ) ;
               if ( rc2 )
                  rc = r4after ;
               else
                  rc = 0 ;  /* successful find */
            }
         }
         else
         {
            if ( rc == 0 )  /* the item was found, so go top, */
            {
               rc2 = tfile4top( t4 ) ;
               if ( rc2 < 0 )
                  return error4stack( t4->codeBase, (short)rc2, E91642 ) ;
            }
            else  /* otherwise want an eof type condition */
            {
               b4goEof( blockOn ) ;
               rc = r4eof ;
            }
         }
      }
   #endif
   return rc ;
}

long S4FUNCTION tfile4skip( TAG4FILE *t4, long numSkip )
{
   long numLeft ;
   B4BLOCK *blockOn ;
   int rc ;
   #ifdef S4FOX
      int saveDups ;
      long goTo ;
   #else
      int sign ;
   #endif

   #ifdef E4PARM_LOW
      if ( t4 == 0 )
      {
         error4( 0, e4parm_null, E91642 ) ;
         return -numSkip ;
      }
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return -numSkip ;

   numLeft = numSkip ;

   blockOn = (B4BLOCK *)t4->blocks.lastNode ;
   if ( blockOn == 0 )
   {
      if ( tfile4top( t4 ) < 0 )
         return -numSkip ;
      blockOn = (B4BLOCK *)t4->blocks.lastNode ;
   }

   #ifdef E4ANALYZE
      if ( ! b4leaf(blockOn) )
      {
         error4( t4->codeBase, e4index, E91642 ) ;
         return -numSkip ;
      }
   #endif

   #ifdef S4FOX
      for(;;)
      {
         numLeft -= b4skip( blockOn, numLeft ) ;
         if ( numLeft == 0 )  /* Success */
            return numSkip ;

         if ( numLeft > 0 )
            goTo = blockOn->header.rightNode ;
         else
            goTo = blockOn->header.leftNode ;

         if ( goTo == -1 )
         {
            if ( numSkip > 0 && t4->header.descending == 0 ||
                 numSkip <= 0 && t4->header.descending == 1 )
            {
               saveDups = tfile4block( t4 )->curDupCnt ;
               rc = tfile4bottom( t4 ) ;
               if ( rc < 0 )
                  return -numSkip ;
               tfile4block( t4 )->curDupCnt = saveDups ;
            }
            else
               if ( tfile4top(t4) < 0 )
                 return -numSkip ;
            return (numSkip - numLeft) ;
         }

         #ifndef S4OFF_WRITE
            if ( blockOn->changed )
               if ( b4flush( blockOn ) < 0 )
                  return -numSkip ;
         #endif

         /* save the current key in case skip fails -- for leaf blocks only */
         if ( b4leaf( blockOn ) )
            memcpy( t4->codeBase->savedKey, (void *)b4keyKey( blockOn, b4numKeys( blockOn ) - 1 ), (unsigned int)t4->header.keyLen ) ;

         rc = i4readBlock( &t4->indexFile->file, goTo, 0, blockOn ) ;
         if ( rc < 0 )
            return -numSkip ;

         if ( rc == 1 )   /* failed on i/o, seek current spot to make valid */
         {
            #ifndef S4OPTIMIZE_OFF
            #ifndef S4SINGLE
               file4refresh( &t4->indexFile->file ) ;
            #endif
            #endif
            rc = tfile4seek( t4, t4->codeBase->savedKey, t4->header.keyLen ) ;
            if ( rc < 0 )
               return -numSkip ;
            if ( rc == r4after )   /* means skipped 1 ahead */
               numLeft-- ;
         }

         #ifdef S4READ_ADVANCE
            #ifndef S4OFF_OPTIMIZE
               if ( t4->indexFile->file.doBuffer )
               {
                  /* advance-read the next block since skipping */
                  if ( numSkip > 0 )
                     goTo = blockOn->header.rightNode ;
                  else
                     goTo = blockOn->header.leftNode ;
                  if ( goTo != -1 )
                     opt4advanceReadBuf( &t4->indexFile->file, I4MULTIPLY * goTo, B4BLOCK_SIZE ) ;
               }
            #endif
         #endif

         blockOn->fileBlock = goTo ;
         blockOn->builtOn = -1 ;
         b4top( blockOn ) ;

         if ( numLeft < 0 )
            numLeft += b4numKeys( blockOn ) ;
         else
            numLeft -= 1 ;  /* moved to the next entry */
      }
   #endif

   #ifndef S4FOX
      if ( numSkip < 0)
         sign = -1 ;
      else
         sign = 1 ;

      for(;;)
      {
         /* save the current key in case skip fails -- for leaf blocks only */
         if ( b4leaf( blockOn ) )
            memcpy( t4->codeBase->savedKey, b4keyKey( blockOn, blockOn->keyOn ), t4->header.keyLen ) ;

         while ( ( rc = tfile4down( t4 ) ) == 0 )
            if ( sign < 0 )
            {
               blockOn = tfile4block( t4 ) ;
               b4goEof( blockOn ) ;
               if ( b4leaf( blockOn) )
               {
                  blockOn->keyOn-- ;
                  #ifdef E4ANALYZE
                     if ( blockOn->keyOn < 0 )
                     {
                        error4( t4->codeBase, e4index, E91642 ) ;
                        return -numSkip ;
                     }
                  #endif
               }
            }

         if ( rc < 0 )
            return -numSkip ;

         if ( rc == 2 )   /* failed on i/o, seek current spot to make valid */
         {
            tfile4outOfDate( t4 ) ;
            rc = tfile4seek( t4, t4->codeBase->savedKey, t4->header.keyLen ) ;
            if ( rc < 0 )
               return -numSkip ;
            if ( rc == r4after )   /* means skipped 1 ahead */
               numLeft-- ;
            continue ;
         }

         blockOn = tfile4block( t4 ) ;
         if ( rc < 0 || blockOn == 0 )
            return -numSkip ;

         numLeft -= b4skip( blockOn, numLeft ) ;
         if ( numLeft == 0 )      /* Success */
            return numSkip ;

         do  /* Skip 1 to the next leaf block  */
         {
            if ( (B4BLOCK *)blockOn->link.p == blockOn )
            {
               if ( numSkip > 0 )
               {
                  if ( tfile4bottom( t4 ) < 0 )
                     return -numSkip ;
               }
               else
                  if ( tfile4top( t4 ) < 0 )
                     return -numSkip ;

               return( numSkip - numLeft ) ;
            }
            else
               if ( tfile4up( t4 ) < 0 )
                  return -numSkip ;

            blockOn = (B4BLOCK *)t4->blocks.lastNode ;
         }  while ( b4skip( blockOn, (long) sign) != sign) ;

         numLeft -= sign ;
      }
   #endif
}

#ifndef S4OFF_WRITE
B4BLOCK *tfile4split( TAG4FILE *t4, B4BLOCK *oldBlock )
{
   long newFileBlock ;
   B4BLOCK *newBlock ;
   #ifdef S4FOX
      int rc ;
      FILE4LONG pos ;
   #else
      int totLen, newLen ;
   #endif

   #ifdef E4PARM_LOW
      if ( t4 == 0 || oldBlock == 0 )
      {
         error4( 0, e4parm_null, E91642 ) ;
         return 0 ;
      }
   #endif

   if ( error4code( t4->codeBase ) < 0 )
      return 0 ;

   #ifdef E4INDEX_VERIFY
      if ( b4verify( oldBlock ) == -1 )
      {
         error4describe( oldBlock->tag->codeBase, e4index, E91642, oldBlock->tag->alias, 0, 0 ) ;
         return 0 ;
      }
   #endif

   newFileBlock = index4extend( t4->indexFile ) ;

   newBlock = b4alloc( t4, newFileBlock ) ;
   if ( newBlock == 0 )  return 0 ;

   newBlock->changed = 1 ;
   oldBlock->changed = 1 ;

   #ifdef S4FOX
      if ( b4leaf( oldBlock ) )
         rc = tfile4leafSplit( t4, oldBlock, newBlock ) ;
      else
         rc = tfile4branchSplit( t4, oldBlock, newBlock ) ;

      if ( rc < 0 )
         return 0 ;

      newBlock->header.rightNode = oldBlock->header.rightNode ;
      newBlock->header.leftNode = oldBlock->fileBlock ;
      oldBlock->header.rightNode = newBlock->fileBlock ;

      if ( newBlock->header.rightNode != -1 )   /* must change left ptr for next block over */
      {
         #ifdef S4BYTE_SWAP
            newBlock->fileBlock = x4reverseLong( (void *)&newBlock->fileBlock ) ;
         #endif
         file4longAssign( pos, newBlock->header.rightNode + 2 * sizeof( short ), 0 ) ;
         rc = file4writeInternal( &t4->indexFile->file, pos, &newBlock->fileBlock, sizeof( newBlock->header.leftNode ) ) ;
         if ( rc < 0 )
            return 0 ;
         #ifdef S4BYTE_SWAP
            newBlock->fileBlock = x4reverseLong( (void *)&newBlock->fileBlock ) ;
         #endif
      }
   #else
      /* NNNNOOOO  N - New, O - Old */
      newBlock->nKeys  = (short)( ( b4numKeys( oldBlock ) + 1 ) / 2 ) ;
      oldBlock->nKeys -= b4numKeys( newBlock ) ;
      newBlock->keyOn  = oldBlock->keyOn ;

      totLen = t4->indexFile->header.blockRw - sizeof(oldBlock->nKeys) - sizeof(oldBlock->dummy) ;
      newLen = b4numKeys( newBlock ) * t4->header.groupLen ;

      memcpy( (void *)b4key( newBlock, 0 ), (void *)b4key( oldBlock, 0 ), newLen ) ;
      c4memmove( b4key(oldBlock,0), b4key( oldBlock, b4numKeys( newBlock )), totLen - newLen ) ;
      oldBlock->keyOn = oldBlock->keyOn - b4numKeys( newBlock ) ;
   #endif

   return newBlock ;
}

#ifdef S4FOX
int tfile4branchSplit( TAG4FILE *t4, B4BLOCK *oldBlock, B4BLOCK *newBlock )
{
   int newLen, nNewKeys ;
   int gLen = t4->header.keyLen + 2*sizeof(S4LONG) ;
   char *oPos ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 || oldBlock == 0 || newBlock == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   /* NNNNOOOO  N - New, O - Old */
   nNewKeys = ( b4numKeys( oldBlock ) + 1 ) / 2 ;
   if ( oldBlock->keyOn > nNewKeys )
      nNewKeys-- ;
   newBlock->header.nKeys = (short)nNewKeys ;
   oldBlock->header.nKeys -= (short)nNewKeys ;

   newLen = b4numKeys( newBlock ) * gLen ;

   oPos = ((char *)&oldBlock->nodeHdr) + gLen * b4numKeys( oldBlock ) ;
   memcpy( (void *)&newBlock->nodeHdr, oPos, (unsigned int)newLen ) ;
   newBlock->header.nodeAttribute = 0 ;
   oldBlock->header.nodeAttribute = 0 ;
   newBlock->keyOn = oldBlock->keyOn - b4numKeys( oldBlock ) ;

   /* clear the old data */
   memset( oPos, 0, (unsigned int)newLen ) ;

   return 0 ;
}

int tfile4leafSplit( TAG4FILE *t4, B4BLOCK *oldBlock, B4BLOCK *newBlock )
{
   char *obdPos, *obiPos ;
   unsigned char buffer[6] ;
   int len, nKeys, rc, kLen, iLen, bLen, oldDup ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 || oldBlock == 0 || newBlock == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   kLen = t4->header.keyLen ;
   iLen = oldBlock->nodeHdr.infoLen ;
   bLen = B4BLOCK_SIZE - (sizeof(oldBlock->header)) - (sizeof(oldBlock->nodeHdr))
              - b4numKeys( oldBlock ) * iLen - oldBlock->nodeHdr.freeSpace ;
   oldDup = oldBlock->curDupCnt ;

   b4top( oldBlock ) ;
   nKeys = b4numKeys( oldBlock ) / 2 ;
   for ( len = 0 ; len < b4numKeys( oldBlock ) - nKeys ; len++ )
   {
      rc = b4skip( oldBlock, 1L ) ;
      if ( rc < 0 )
         return error4stack( t4->codeBase, (short)rc, E91642 ) ;
   }

   /* build the key 1st key of the new block from one past new end of old block */
   b4key( oldBlock, oldBlock->keyOn ) ;

   /* copy the general information */
   memcpy( (void *)&newBlock->header, (void *)&oldBlock->header,
           (sizeof( oldBlock->header)) + (sizeof(oldBlock->nodeHdr)) ) ;

   /* AS 06/19/98 - changes.60 fix #134, 2.6 version should not include blanks... */
   /* put 1st key of new block */
   if ( t4->indexFile->dataFile->version != 0x30 && t4->codeBase->compatibility == 26 && t4->filter != 0 )
      newBlock->curTrailCnt = 0 ;
   else
      newBlock->curTrailCnt = b4calcBlanks( oldBlock->builtKey->value, kLen, t4->pChar ) ;
   len = kLen - newBlock->curTrailCnt ;
   newBlock->curPos = ((char *)&newBlock->header) + B4BLOCK_SIZE - len ;
   memcpy( newBlock->curPos, oldBlock->builtKey->value, (unsigned int)len ) ;

   /* copy remaining key data */
   obdPos = ((char *)&oldBlock->header) + B4BLOCK_SIZE - bLen ;
   len = oldBlock->curPos - obdPos ;
   newBlock->curPos -= len ;
   memcpy( newBlock->curPos, obdPos, (unsigned int)len ) ;

   /* copy the info data */
   obiPos = oldBlock->data + oldBlock->keyOn * iLen ;
   memcpy( newBlock->data, obiPos, (unsigned int)nKeys * iLen ) ;

   /* clear the old data */
   rc = b4skip( oldBlock, -1L ) ;  /* go to new last entry */
   if ( rc == 0 )
      if ( error4code( t4->codeBase ) < 0 )
         return error4stack( t4->codeBase, (short)error4code( t4->codeBase ), E91642 ) ;
   #ifdef E4ANALYZE
      if ( obdPos < obiPos )
         return error4( t4->codeBase, e4info, E81603 ) ;
   #endif
   memset( obiPos, 0, (unsigned int)(oldBlock->curPos - obiPos) ) ;

   /* now reset the place new info data for the first key */
   memset( newBlock->data, 0, (unsigned int)iLen ) ;
   x4putInfo( &newBlock->nodeHdr, buffer, oldBlock->builtKey->num, newBlock->curTrailCnt, 0 ) ;
   memcpy( newBlock->data, (void *)buffer, (unsigned int)iLen ) ;

   newBlock->header.nKeys = (short)nKeys ;
   oldBlock->header.nKeys -= (short)nKeys ;
   newBlock->header.nodeAttribute = 2 ;
   oldBlock->header.nodeAttribute = 2 ;
   newBlock->nodeHdr.freeSpace = (short) (newBlock->curPos - newBlock->data
                                    - b4numKeys( newBlock ) * iLen) ;
   oldBlock->nodeHdr.freeSpace = (short) (oldBlock->curPos - oldBlock->data
                                    - b4numKeys( oldBlock ) * iLen) ;
   oldBlock->builtOn = -1 ;
   newBlock->builtOn = -1 ;

   rc = b4top( oldBlock ) ;
   if ( rc < 0 )
      return error4stack( t4->codeBase, (short)rc, E91642 ) ;
   rc = b4top( newBlock ) ;
   if ( rc < 0 )
      return error4stack( t4->codeBase, (short)rc, E91642 ) ;

   /* make sure dupCnt is updated on blocks for insert */
   newBlock->curDupCnt = oldDup ;
   oldBlock->curDupCnt = oldDup ;

   return 0 ;
}
#endif
#endif  /* S4OFF_WRITE */

#ifdef S4FOX
int tfile4rlTop( TAG4FILE *t4 )
{
   int rc, rc2 ;

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
         return error4stack( t4->codeBase, (short)rc, E91642 ) ;

      if ( rc != 2 )
      {
         do
         {
            b4top( (B4BLOCK *)t4->blocks.lastNode ) ;
            if ( (rc = tfile4down(t4)) < 0 )
               return error4stack( t4->codeBase, (short)rc, E91642 ) ;
         } while ( rc == 0 ) ;
      }

      if ( rc == 2 )   /* failed due to read while locked */
      {
         rc2 = tfile4outOfDate( t4 ) ;
         return error4stack( t4->codeBase, (short)rc2, E91642 ) ;
      }
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
#else
int S4FUNCTION tfile4top( TAG4FILE *t4 )
{
   int rc, rc2 ;

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
         return error4stack( t4->codeBase, rc, E91642 ) ;

      if ( rc != 2 )
      {
         ((B4BLOCK *)t4->blocks.lastNode)->keyOn = 0 ;
         do
         {
            if ( (rc = tfile4down(t4)) < 0 )
               return error4stack( t4->codeBase, rc, E91642 ) ;
            ((B4BLOCK *)t4->blocks.lastNode)->keyOn = 0 ;
         } while ( rc == 0 ) ;
      }

      if ( rc == 2 )   /* failed due to read while locked */
      {
         rc2 = tfile4outOfDate( t4 ) ;
         return error4stack( t4->codeBase, rc2, E91642 ) ;
      }
   } while ( rc == 2 ) ;

   return 0 ;
}
#endif

int tfile4up( TAG4FILE *t4 )
{
   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parm_null, E91642 ) ;
   #endif

   if ( t4->blocks.lastNode == 0 )
      return 1 ;

   l4add( &t4->saved, l4pop( &t4->blocks ) ) ;
   return 0 ;
}

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
         return tfile4down( t4 ) ;
      l4add( &t4->saved, linkOn ) ;
   }
}

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

   #ifdef S4FOX
      if ( b4numKeys( tfile4block(t4) ) != 0 )
         tfile4block(t4)->keyOn = b4numKeys( tfile4block(t4) ) ;
   #else
      tfile4block(t4)->keyOn++ ;
   #endif

   return 0 ;
}

#endif  /* S4CLIPPER */
#endif  /* S4OFF_INDEX */

#ifdef S4VB_DOS

int tfile4go_v( TAG4FILE *t4, char *key, long recNo )
{
   return tfile4go( t4, c4str(key), recNo, 0 ) ;
}

int tfile4seekV( TAG4FILE *t4, char *seekVal, int keyLen )
{
   return tfile4seek( t4, v4str(seekVal), keyLen) ;
}

#endif
