/* i4add.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4OFF_INDEX
#ifndef S4OFF_WRITE

#ifndef S4CLIENT
int S4FUNCTION t4uniqueModify( TAG4 *tag, int newUnique )
{
   TAG4FILE *tagFile ;
   FILE4LONG pos ;

   #ifdef E4PARM_HIGH
      if ( tag == 0 )
         return error4( 0, e4parm_null, E91716 ) ;
      if ( newUnique != e4unique && newUnique != r4unique && newUnique !=
           r4uniqueContinue && newUnique != 0 )
         #ifdef S4FOX
            if ( newUnique != r4candidate && newUnique != e4candidate )
         #endif
            return error4( 0, e4parm, E91716 ) ;
   #endif

   tagFile = tag->tagFile ;
   tag->errUnique = newUnique ;

   #ifdef S4FOX
      tagFile->header.typeCode &= 0xFA ;  /* clear the unqiuye setting */
      if ( newUnique == r4candidate || newUnique == e4candidate )
         tagFile->header.typeCode |= 0x04 ;
      else
         if ( newUnique == r4unique || newUnique == e4unique || newUnique == r4uniqueContinue )
            tagFile->header.typeCode |= 0x01 ;
      file4longAssign( pos, tagFile->headerOffset + 3 * sizeof( S4LONG ) + sizeof( short ), 0 ) ;
      return file4writeInternal( &tagFile->indexFile->file, pos, &tagFile->header.typeCode, sizeof(tagFile->header.typeCode)) ;
   #endif
   #ifdef S4MDX
      tagFile->header.typeCode &= 0xDF ;
      if ( newUnique == r4unique || newUnique == e4unique || newUnique == r4uniqueContinue )
      {
         tagFile->header.typeCode |= 0x40 ;
         tagFile->header.unique = 0x4000 ;
      }
      file4longAssign( pos, tagFile->headerOffset + 8, 0 ) ;
      return file4writeInternal( &tagFile->indexFile->file, pos, &tagFile->header.typeCode, sizeof(tagFile->header.typeCode)) ;
   #endif
   #ifdef S4CLIPPER
      if ( newUnique == r4unique || newUnique == e4unique || newUnique == r4uniqueContinue )
         tagFile->header.unique = 1 ;
      else
         tagFile->header.unique = 0 ;
      file4longAssign( pos, I4MAX_EXPR_SIZE + 22, 0 ) ;
      return file4writeInternal( &tagFile->file, pos, &tagFile->header.unique, sizeof(tagFile->header.unique)) ;
   #endif
}
#endif /* S4CLIENT */

#ifndef S4CLIPPER
#ifdef S4CLIENT
int S4FUNCTION i4tagAdd( INDEX4 *i4, const TAG4INFO *tagData )
{
   CONNECTION4 *connection ;
   CONNECTION4TAG_ADD_INFO_IN *dataIn ;
   CONNECTION4TAG_ADD_INFO_OUT *dataOut ;
   CONNECTION4TAG_INFO *tinfo ;
   unsigned int len, len2, len3 ;
   DATA4 *d4 ;
   TAG4 *tag ;
   CODE4 *c4 ;
   int rc ;
   short i, j, offset ;

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E91717 ) )
         return -1 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( i4 == 0 || tagData == 0 )
         return error4( 0, e4parm_null, E91717 ) ;
   #endif

   d4 = i4->data ;
   c4 = i4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   #ifndef S4OFF_WRITE
      rc = d4updateRecord( d4, 0 ) ;
      if ( rc )
         return rc ;
   #endif

   if ( d4->readOnly == 1 )
      return error4describe( c4, e4write, E80606, d4alias( d4 ), 0, 0 ) ;
   error4set( c4, 0 ) ;  /* Make sure it is not 'r4unique' or 'r4noCreate'. */

   connection = d4->dataFile->connection ;
   if ( connection == 0 )
      return error4( c4, e4parm, E91717 ) ;
   connection4assign( connection, CON4ADD_TAG, data4clientId( d4 ), data4serverId( d4 ) ) ;
   for( i = 0 ; tagData[i].name != 0; i++ )
      ;
   connection4addData( connection, NULL, sizeof(CONNECTION4TAG_ADD_INFO_IN), (void **)&dataIn ) ;
   u4ncpy( dataIn->indexFileName, i4->indexFile->accessName, strlen( i4->indexFile->accessName ) + 1 ) ;
   dataIn->numTags = htons(i) ;
   len = 0 ;
   offset = sizeof( CONNECTION4TAG_ADD_INFO_IN ) ;
   for ( j = 0 ; j != i ; j++ )  /* Where "i" is numTags */
   {
      len = strlen( tagData[j].name ) + 1 ;
      offset += sizeof( CONNECTION4TAG_INFO ) ;
      connection4addData( connection, NULL, sizeof(CONNECTION4TAG_INFO), (void **)&tinfo ) ;
      tinfo->name.offset = htons(offset);
      len2 = strlen( tagData[j].expression ) + 1 ;
      offset += len ;
      tinfo->expression.offset = htons(offset) ;
      offset += len2 ;
      if ( tagData[j].filter == 0 )
      {
         len3 = 0 ;
         tinfo->filter.offset = 0 ;
      }
      else
      {
         len3 = strlen( tagData[j].filter ) + 1 ;
         tinfo->filter.offset = htons(offset) ;
      }
      offset += len3 ;
      tinfo->unique = htons(tagData[j].unique) ;
      tinfo->descending = htons(tagData[j].descending) ;
      connection4addData( connection, tagData[j].name, len, NULL ) ;
      connection4addData( connection, tagData[j].expression, len2, NULL ) ;
      if ( len3 != 0 )
         connection4addData( connection, tagData[j].filter, len3, NULL ) ;
   }
   connection4sendMessage( connection ) ;
   rc = connection4receiveMessage( connection ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E91717 ) ;
   if ( connection4type( connection ) != CON4ADD_TAG )
      return error4( c4, e4connection, E81705 ) ;

   rc = connection4status( connection ) ;
   if ( rc < 0 )
      return connection4error( connection, c4, rc, E91717 ) ;

   if ( rc == r4unique )
      return r4unique ;

   if ( connection4len( connection ) != sizeof( CONNECTION4TAG_ADD_INFO_OUT ) )
      return error4( c4, e4packetLen, E91717 ) ;

   dataOut = ( CONNECTION4TAG_ADD_INFO_OUT *)connection4data( connection ) ;

   if ( dataOut->lockedDatafile )
      d4->dataFile->fileLock = d4 ;

   for ( i = 0 ; tagData[i].name != 0 ; i++ )
   {
      tag = (TAG4 *)mem4alloc( c4->tagMemory ) ;
      if ( tag == 0 )
         return e4memory ;
      tag->tagFile = (TAG4FILE *)mem4alloc( c4->tagFileMemory ) ;
      if ( tag->tagFile == 0 )
         return e4memory ;
      tag->index = i4 ;
      tag->tagFile->codeBase = c4 ;
      tag->tagFile->indexFile = i4->indexFile ;

      u4ncpy( tag->tagFile->alias, tagData[i].name, sizeof(tag->tagFile->alias) - 1 ) ;
      c4upper( tag->tagFile->alias ) ;

      if ( tagData[i].expression == 0 )
         return error4describe( c4, e4tagInfo, E85303, tagData[i].name, tagData[i].expression, 0 ) ;

      if ( error4code( c4 ) < 0 )
         break ;
      l4add( &i4->indexFile->tags, tag->tagFile ) ;
      l4add( &i4->tags, tag ) ;
      t4uniqueSetLow( tag, tagData[i].unique, 0 ) ;
   }

   return 0 ;
}
#else

#include "r4reinde.h"

#ifdef S4MDX
#define GARBAGE_LEN 518

static int r4reindexTagHeadersWriteSp( R4REINDEX * ) ;

static int r4reindexTagHeadersWriteSp( R4REINDEX *r4 )
{
   /* First, calculate the T4DESC.leftChld, T4DESC.rightChld values, T4DESC.parent values */
   int higher[49], lower[49], parent[49] ;
   TAG4FILE *tagOn, *tagPtr ;
   INDEX4FILE *i4 ;
   DATA4 *d4 ;
   int nTag, iTag, jField, saveCode ;
   T4DESC tagInfo ;
   FILE4LONG pos ;
   #ifdef S4BYTE_SWAP
      I4HEADER swapHeader ;
   #endif  /* S4BYTE_SWAP */

   memset( (void *)higher, 0, sizeof(higher) ) ;
   memset( (void *)lower,  0, sizeof(lower) ) ;
   memset( (void *)parent, 0, sizeof(parent) ) ;

   i4 = r4->indexFile ;
   d4 = r4->data ;

   tagOn = (TAG4FILE *)l4first( &i4->tags ) ;
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
                  return error4( d4->codeBase, e4result, E92102 ) ;
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
   file4longAssign( pos, 0, 0 ) ;
   file4seqWriteInitLow( &r4->seqwrite, &i4->file, pos, r4->buffer, r4->bufferLen ) ;

   i4->header.eof = r4->lastblock + r4->lastblockInc + 2 ;
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

      file4seqWrite( &r4->seqwrite, &swapHeader, sizeof(I4HEADER) ) ;
   #else
      file4seqWrite( &r4->seqwrite, &i4->header, sizeof(I4HEADER) ) ;
   #endif  /* S4BYTE_SWAP */

   file4seqWriteRepeat( &r4->seqwrite, 512-sizeof(I4HEADER)+17, 0 ) ;
   /* There is a 0x01 on byte 17 of the first 32 bytes. */
   file4seqWrite( &r4->seqwrite, "\001", 1 ) ;
   file4seqWriteRepeat( &r4->seqwrite, 14, 0 ) ;

   tagOn = (TAG4FILE *)l4first( &i4->tags ) ;

   for ( iTag = 0; iTag < 47; iTag++ )
   {
      memset( (void *)&tagInfo, 0, sizeof(tagInfo) ) ;

      if ( iTag < r4->nTags )
      {
         if ( tagOn->headerOffset == 0 )
            tagOn->headerOffset = file4longGetLo( file4lenLow( &i4->file ) ) ;

         tagInfo.headerPos = tagOn->headerOffset / 512 ;

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
            saveCode = i4->codeBase->errFieldName ;
            i4->codeBase->errFieldName = 0 ;
            jField = d4fieldNumber( d4, tagOn->expr->source ) ;
            i4->codeBase->errFieldName = saveCode ;
            if ( jField > 0 )
            {
               file4longAssign( pos, ( jField + 1 ) * sizeof( FIELD4IMAGE ) - 1, 0 ) ;
               file4writeInternal( &d4->dataFile->file, pos, "\001", 1 ) ;
            }
         }
         tagOn = (TAG4FILE *)l4next( &i4->tags, tagOn ) ;
      }
      if ( file4seqWrite( &r4->seqwrite, &tagInfo, sizeof(T4DESC)) < 0 )
         return -1 ;
   }

   return 0 ;
}
#endif

static int i4addOneTag( INDEX4 *i4, const TAG4INFO *tagData )
{
   int rc ;
   TAG4 *tagPtr ;
   TAG4FILE *tagFile ;
   R4REINDEX reindex ;
   DATA4 *d4 ;
   CODE4 *c4 ;
   const char *ptr ;
   FILE4LONG pos ;
   #ifdef S4FOX
      int keysMax, exprType ;
      B4BLOCK *b4 ;
      long rNode, goTo ;
      int totLen, exprHdrLen ;
      #ifdef S4BYTE_SWAP
         T4HEADER swapTagHeader ;
         char *swapPtr ;
         S4LONG longVal ;
         short shortVal ;
         int i ;
      #endif
      #ifdef S4DATA_ALIGN
         unsigned int size, delta ;
      #endif
   #endif
   #ifdef S4MDX
      #ifdef S4BYTE_SWAP
         T4HEADER swapTagHeader ;
      #endif  /* S4BYTE_SWAP */
      int len ;
   #endif

   #ifdef E4PARM_LOW
      if ( i4 == 0 || tagData == 0 )
         return error4( 0, e4parm_null, E91716 ) ;
   #endif

   if ( error4code( i4->codeBase ) < 0 )
      return e4codeBase ;

   d4 = i4->data ;
   c4 = d4->codeBase ;

   rc = r4reindexInit( &reindex, i4, i4->indexFile ) ;
   if ( rc < 0 )
      return error4stack( c4, (short)rc, E91716 ) ;

   tagPtr = (TAG4 *)mem4alloc( c4->tagMemory ) ;
   if ( tagPtr == 0 )
      rc = error4stack( c4, e4memory, E91716 ) ;
   else
   {
      memset( (void *)tagPtr, 0, sizeof( TAG4 ) ) ;
      tagPtr->tagFile = (TAG4FILE *)mem4alloc( c4->tagFileMemory ) ;
      tagFile = tagPtr->tagFile ;
      tagPtr->index = i4 ;
      if ( tagFile == 0 )
         rc = error4stack( c4, e4memory, E91716 ) ;
      else
      {
         memset( (void *)tagFile, 0, sizeof( TAG4FILE ) ) ;
         tagFile->codeBase = c4 ;
         tagFile->indexFile = i4->indexFile ;
         u4ncpy( tagFile->alias, tagData[0].name, sizeof(tagFile->alias) ) ;
         c4upper( tagFile->alias ) ;
      }
   }

   #ifdef S4MDX
      for ( ; rc == 0 ; )
      {
         if( i4->indexFile->header.numTags == 47 )  /* already at max */
         {
            rc = error4( c4, e4tagInfo, E85306 ) ;
            break ;
         }

         i4->indexFile->header.numTags++ ;

         tagFile->header.typeCode = 0x10 ;
         if ( tagData[0].unique )
         {
            tagFile->header.typeCode += 0x40 ;
            tagFile->header.unique = 0x4000 ;
            tagPtr->errUnique = tagData[0].unique ;

            #ifdef E4MISC
               if ( tagData[0].unique != e4unique && tagData[0].unique != r4unique &&
                    tagData[0].unique != r4uniqueContinue )
               {
                  rc = error4( c4, e4info, E81711 ) ;
                  break ;
               }
            #endif
         }
         if ( tagData[0].descending)
         {
            tagFile->header.typeCode += 0x08 ;
            #ifdef E4MISC
               if ( tagData[0].descending != r4descending )
               {
                  rc = error4( c4, e4info, E81712 ) ;
                  break ;
               }
            #endif
         }

         #ifdef E4MISC
            if ( tagData[0].expression == 0 )
            {
               rc = error4( c4, e4parm, E81512 ) ;
               break ;
            }
         #endif

         tagFile->expr = expr4parseLow( d4, tagData[0].expression, tagFile ) ;
         if( tagFile->expr == 0 )
         {
            if ( error4code( c4 ) == 0 )
               rc = error4( c4, e4info, E91716 ) ;
            else
               rc = error4code( c4 ) ;
            break ;
         }
         if ( tagData[0].filter != 0 )
            if ( *(tagData[0].filter) != '\0' )
            {
               tagFile->filter = expr4parseLow( d4, tagData[0].filter, tagFile ) ;
               if ( tagFile->filter == 0 )
               {
                  if ( error4code( c4 ) == 0 )
                     rc = error4( c4, e4info, E91716 ) ;
                  else
                     rc = error4code( c4 ) ;
                  break ;
               }
            }

         if ( error4code( c4 ) < 0 )
         {
            rc = error4code( c4 ) ;
            break ;
         }

         l4add( &i4->tags, tagPtr ) ;
         l4add( &i4->indexFile->tags, tagFile ) ;

         tagFile->header.keyLen = expr4keyLen( tagFile->expr ) ;
         if ( tagFile->header.keyLen < 0 )
         {
            rc = tagFile->header.keyLen ;
            break ;
         }
         tagFile->header.type = (char)expr4type( tagFile->expr ) ;
         if ( tagFile->header.type == r4dateDoub )
            tagFile->header.type = r4date ;
         if ( tagFile->header.type == r4numDoub )
            tagFile->header.type = r4num ;

         tfile4initSeekConv( tagFile, tagFile->header.type ) ;
         tagFile->header.groupLen = tagFile->header.keyLen+ 2*sizeof(S4LONG)-1 ;
         tagFile->header.groupLen-= tagFile->header.groupLen % sizeof(S4LONG) ;

         tagFile->header.keysMax = (reindex.indexFile->header.blockRw - sizeof(short) - 6 - sizeof(S4LONG)) /
            tagFile->header.groupLen;
         if ( tagFile->header.keysMax < reindex.minKeysmax )
            reindex.minKeysmax = tagFile->header.keysMax ;
         tagFile->hasKeys = 0 ;
         tagFile->hadKeys = 1 ;
         rc = r4reindexBlocksAlloc( &reindex ) ;
         if ( rc < 0 )
            break ;

         reindex.nTags = i4->indexFile->header.numTags ;

         reindex.lastblockInc = reindex.indexFile->header.blockRw / 512 ;

         reindex.lastblock = file4longGetLo( file4lenLow( &reindex.indexFile->file ) ) / 512 - reindex.lastblockInc ;
         if ( reindex.lastblock < 0 )
         {
            rc = (int)reindex.lastblock ;
            break ;
         }

         reindex.tag = tagFile ;

         rc = r4reindexSupplyKeys( &reindex ) ;
         if ( rc < 0 )
            break ;

         rc = r4reindexWriteKeys( &reindex, t4unique( tagPtr ) ) ;
         if ( rc < 0 )
            break ;

         /* regenerate the tagHeaders special... */
         rc = r4reindexTagHeadersWriteSp( &reindex ) ;
         if ( rc < 0 )
            break ;

         rc = file4seqWriteFlush( &reindex.seqwrite ) ;
         if ( rc < 0 )
            break ;

         file4longAssign( pos, tagFile->headerOffset, 0 ) ;
         file4seqWriteInitLow( &reindex.seqwrite, &i4->indexFile->file, pos, reindex.buffer, reindex.bufferLen ) ;

         #ifdef S4BYTE_SWAP
            memcpy( (void *)&swapTagHeader, (void *)&tagFile->header, sizeof(T4HEADER) ) ;

            swapTagHeader.root = x4reverseLong( (void *)&swapTagHeader.root ) ;
            swapTagHeader.keyLen = x4reverseShort( (void *)&swapTagHeader.keyLen ) ;
            swapTagHeader.keysMax = x4reverseShort( (void *)&swapTagHeader.keysMax ) ;
            swapTagHeader.groupLen = x4reverseShort( (void *)&swapTagHeader.groupLen ) ;
            swapTagHeader.isDate = x4reverseShort( (void *)&swapTagHeader.isDate ) ;
            swapTagHeader.unique = x4reverseShort( (void *)&swapTagHeader.unique ) ;

            rc = file4seqWrite( &reindex.seqwrite, &swapTagHeader, sizeof(T4HEADER) ) ;
            if ( rc < 0 )
               break ;
         #else
            rc = file4seqWrite( &reindex.seqwrite, &tagFile->header, sizeof(T4HEADER) ) ;
            if ( rc < 0 )
               break ;
         #endif  /* S4BYTE_SWAP */

         ptr = tagFile->expr->source ;
         len = strlen( ptr ) ;

         rc = file4seqWrite( &reindex.seqwrite, ptr, len ) ;
         if ( rc < 0 )
            break ;

         rc = file4seqWriteRepeat( &reindex.seqwrite, 221-len, 0 ) ;
         if ( rc < 0 )
            break ;

         if( tagFile->filter != 0 )
         {
            rc = file4seqWriteRepeat( &reindex.seqwrite, 1, 1 ) ;
            if ( rc < 0 )
               break ;

            if ( tagFile->hasKeys )
               rc = file4seqWriteRepeat( &reindex.seqwrite, 1, 1 ) ;
            else
               rc = file4seqWriteRepeat( &reindex.seqwrite, 1, 0 ) ;
            if ( rc < 0 )
               break ;
         }
         else
         {
            rc = file4seqWriteRepeat( &reindex.seqwrite, 2, 0 ) ;
            if ( rc < 0 )
               break ;
         }

         /* write extra space up to filter write point */
         rc = file4seqWriteRepeat( &reindex.seqwrite, GARBAGE_LEN - 3, 0 ) ;
         if ( rc < 0 )
            break ;

         if ( tagFile->filter == 0 )
            len = 0 ;
         else
         {
            ptr = tagFile->filter->source ;
            len = strlen(ptr) ;
            rc = file4seqWrite( &reindex.seqwrite, ptr, len ) ;
            if ( rc < 0 )
               break ;
         }
         rc = file4seqWriteRepeat( &reindex.seqwrite,
            reindex.blocklen - GARBAGE_LEN - len - 220 - sizeof(tagFile->header), 0 );
         if ( rc < 0 )
            break ;
         rc = file4seqWriteFlush(&reindex.seqwrite ) ;
         if ( rc == 0 )
         {
            file4longAssign( pos, i4->indexFile->header.eof * 512, 0 ) ;
            rc = file4lenSetLow( &i4->indexFile->file, pos ) ;
         }
         break ;
      }

      if ( rc < 0 )
         file4seqWriteFlush(&reindex.seqwrite ) ;
   #endif
   #ifdef S4FOX
      for ( ; rc == 0 ; )
      {
         tagFile->header.typeCode  = 0x60 ;  /* compact */
         if ( tagData[0].unique )
         {
            tagFile->header.typeCode += 0x01 ;
            tagPtr->errUnique = tagData[0].unique ;

            #ifdef E4MISC
               if ( tagData[0].unique != e4unique && tagData[0].unique != r4unique &&
                       tagData[0].unique != r4candidate && tagData[0].unique != r4uniqueContinue )
               {
                  rc = error4( c4, e4info, E81711 ) ;
                  break ;
               }
            #endif
         }
         if ( tagData[0].descending)
         {
            tagFile->header.descending = 1 ;
            #ifdef E4MISC
               if ( tagData[0].descending != r4descending )
               {
                  rc = error4( c4, e4info, E81712 ) ;
                  break ;
               }
            #endif
         }

         /* set the tag's collating sequence */
         if ( tfile4setCollatingSeq( tagFile, c4->collatingSequence ) < 0 )
         {
            rc = error4( c4, e4index, E84907 ) ;
            break ;
         }
         if ( tfile4setCodePage( tagFile, i4->data->codePage ) < 0 )
         {
            rc = error4( c4, e4index, E91642 ) ;
            break ;
         }

         #ifdef E4MISC
            if ( tagData[0].expression == 0 )
            {
               rc = error4( c4, e4parm, E85303 ) ;
               break ;
            }
         #endif

         tagFile->expr = expr4parseLow( d4, tagData[0].expression, tagFile ) ;
         if ( tagFile->expr == 0 )
         {
            if ( error4code( c4 ) == 0 )
               rc = error4( c4, e4info, E91716 ) ;
            else
               rc = error4code( c4 ) ;
            break ;
         }

         tagFile->header.exprLen = strlen( tagFile->expr->source ) + 1 ;
         if ( tagData[0].filter != 0 )
            if ( *( tagData[0].filter ) != '\0' )
            {
               tagFile->header.typeCode += 0x08 ;
               tagFile->filter = expr4parseLow( d4, tagData[0].filter, tagFile ) ;
               if ( tagFile->filter == 0 )
               {
                  if ( error4code( c4 ) == 0 )
                     rc = error4( c4, e4info, E91716 ) ;
                  else
                     rc = error4code( c4 ) ;
                  break ;
               }
               tagFile->header.filterLen = strlen( tagFile->filter->source ) ;
            }

         tagFile->header.filterLen++ ;  /* minimum of 1, for the '\0' */
         tagFile->header.filterPos = tagFile->header.exprLen ;

         if ( error4code( c4 ) < 0 || error4code( c4 ) == r4unique )
         {
            rc = error4code( c4 ) ;
            break ;
         }

         reindex.tag = tagFile ;
         reindex.nBlocksUsed = 0 ;

         reindex.lastblock = file4longGetLo( file4lenLow( &i4->indexFile->file ) ) - B4BLOCK_SIZE ;

         tagFile->header.keyLen = expr4keyLen( tagFile->expr ) ;


         #ifdef S4DATA_ALIGN
            size = (unsigned int)sizeof(S4LONG) + tagFile->header.keyLen ;
            delta = sizeof(void *) - size % sizeof(void *);
            tagFile->builtKeyMemory = mem4create( c4, 3, size + delta, 2, 0 ) ;
         #else
            tagFile->builtKeyMemory = mem4create( c4, 3, (unsigned int)sizeof(S4LONG) + tagFile->header.keyLen + 1, 2, 0 ) ;
         #endif

         exprType = expr4type( tagFile->expr ) ;
         if ( exprType < 0 )
         {
            rc = exprType ;
            break ;
         }
         tfile4initSeekConv( tagFile, exprType ) ;
         if ( tagFile->header.keyLen < 0 )
         {
            rc = tagFile->header.keyLen ;
            break ;
         }

         keysMax = ( B4BLOCK_SIZE - sizeof(B4STD_HEADER) ) / ( tagFile->header.keyLen + 2*sizeof(S4LONG) ) ;

         if ( keysMax < reindex.minKeysmax )
            reindex.minKeysmax = keysMax ;

         rc = r4reindexBlocksAlloc( &reindex ) ;
         if ( rc < 0 )
            break ;

         rc = r4reindexSupplyKeys( &reindex ) ;
         if ( rc < 0 )
            break ;

         rc = r4reindexWriteKeys( &reindex, t4unique( tagPtr ) ) ;
         if ( rc < 0 )
            break ;

         #ifdef S4BYTE_SWAP
            memcpy( (void *)&swapTagHeader, (void *)&tagFile->header, sizeof(T4HEADER) ) ;

            swapTagHeader.root = x4reverseLong( (void *)&swapTagHeader.root ) ;
            swapTagHeader.freeList = x4reverseLong( (void *)&swapTagHeader.freeList ) ;
            /* version is stored in non-intel format */
            swapTagHeader.keyLen = x4reverseShort( (void *)&swapTagHeader.keyLen ) ;
            swapTagHeader.descending = x4reverseShort( (void *)&swapTagHeader.descending ) ;
            swapTagHeader.filterPos = x4reverseShort( (void *)&swapTagHeader.filterPos ) ;
            swapTagHeader.filterLen = x4reverseShort( (void *)&swapTagHeader.filterLen ) ;
            swapTagHeader.exprPos = x4reverseShort( (void *)&swapTagHeader.exprPos ) ;
            swapTagHeader.exprLen = x4reverseShort( (void *)&swapTagHeader.exprLen ) ;

            rc = file4seqWrite( &reindex.seqwrite, &swapTagHeader, LEN4HEADER_WR ) ;
         #else
            tagFile->header.version = x4reverseShort((void *)&tagFile->header.version) ;
            rc = file4seqWrite( &reindex.seqwrite, &tagFile->header, LEN4HEADER_WR ) ;
            tagFile->header.version = x4reverseShort((void *)&tagFile->header.version) ;
         #endif

         if ( rc < 0 )
            break ;

         rc = file4seqWriteRepeat( &reindex.seqwrite, 478L, 0 ) ;
         if ( rc < 0 )
            break ;
         rc = file4seqWrite( &reindex.seqwrite, &tagFile->header.sortSeq, 8 ) ;

         if ( rc < 0 )
            break ;

         exprHdrLen = 5*sizeof(short) ;

         #ifdef S4BYTE_SWAP
            rc = file4seqWrite( &reindex.seqwrite, &swapTagHeader.descending, exprHdrLen ) ;
         #else
            rc = file4seqWrite( &reindex.seqwrite, &tagFile->header.descending, (unsigned int)exprHdrLen ) ;
         #endif
         if ( rc < 0 )
            break ;

         ptr = tagFile->expr->source ;
         totLen = tagFile->header.exprLen ;
         rc = file4seqWrite( &reindex.seqwrite, ptr, (unsigned int)tagFile->header.exprLen ) ;
         if ( rc < 0 )
            break ;

         if ( tagFile->filter != 0 )
         {
            ptr = tagFile->filter->source ;
            rc = file4seqWrite( &reindex.seqwrite, ptr, (unsigned int)tagFile->header.filterLen ) ;
            if ( rc < 0 )
               break ;
            totLen += tagFile->header.filterLen ;
         }
         rc = file4seqWriteRepeat( &reindex.seqwrite, (long)B4BLOCK_SIZE - totLen, 0 );
         if ( rc < 0 )
            break ;

         rc = file4seqWriteFlush(&reindex.seqwrite ) ;
         if ( rc < 0 )
            break ;

         /* now must fix the right node branches for all blocks by moving leftwards */
         if ( rc == 0 )
         {
            for( tfile4rlBottom( tagFile ) ; tagFile->blocks.lastNode ; tfile4up( tagFile ) )
            {
               b4 = tfile4block( tagFile ) ;
               goTo = b4->header.leftNode ;

               while ( goTo != -1 && rc == 0 )
               {
                  rNode = b4->fileBlock ;
                  if ( b4->changed )
                  {
                     rc = b4flush( b4 ) ;
                     if ( rc < 0 )
                        break ;
                  }

                  file4longAssign( pos, I4MULTIPLY * goTo, 0 ) ;
                  rc = file4readAllInternal( &tagFile->indexFile->file, pos, &b4->header, B4BLOCK_SIZE ) ;
                  if ( rc < 0 )
                     break ;

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
                     else   /* if b4 is a branch */
                     {
                        shortVal = b4->tag->header.keyLen + sizeof(S4LONG) ;
                        /* position swapPtr to end of first key expression */
                        swapPtr = (char *)&b4->nodeHdr.freeSpace + b4->tag->header.keyLen ;

                        /* move through all B4KEY's to swap 'long's */
                        for ( i = 0 ; i < (int)b4numKeys( b4 ) ; i++ )
                        {
                           longVal = x4reverseLong((void *)swapPtr ) ;
                           memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
                           swapPtr += sizeof(S4LONG) ;
                           longVal = x4reverseLong((void *)swapPtr ) ;
                           memcpy( swapPtr, (void *) &longVal, sizeof(S4LONG) ) ;
                           swapPtr += shortVal ;
                        }
                     }
                  #endif

                  b4->fileBlock = goTo ;
                  if ( b4->header.rightNode != rNode )  /* if a bad value */
                  {
                     b4->header.rightNode = rNode ;
                     b4->changed = 1 ;
                  }
                  goTo = b4->header.leftNode ;
               }

               if ( rc < 0 )
                  break ;

               b4->builtOn = -1 ;
               b4top( b4 ) ;
            }
         }

         l4add( &i4->tags, tagPtr ) ;
         l4add( &i4->indexFile->tags, tagFile ) ;
         tagFile->headerOffset = reindex.lastblock + B4BLOCK_SIZE ;
         rc = tfile4add( i4->indexFile->tagIndex, (unsigned char *)tagFile->alias, tagFile->headerOffset, t4unique( tagPtr ) ) ;
         if ( rc == 0 )
         {
            i4->indexFile->eof = reindex.lastblock + 3 * B4BLOCK_SIZE ;
            file4longAssign( pos, i4->indexFile->eof, 0 ) ;
            rc = file4lenSetLow( &i4->indexFile->file, pos ) ;
         }
         break ;
      }

      if ( rc <= 0 )
         file4seqWriteFlush(&reindex.seqwrite ) ;
   #endif

   r4reindexFree( &reindex ) ;
   if ( rc != 0 || error4code( i4->codeBase ) < 0 )
   {
      if ( tagPtr != 0 )
      {
         if ( tagFile != 0 )
            mem4free( c4->tagFileMemory, tagFile ) ;
         mem4free( c4->tagMemory, tagPtr ) ;
      }
      return rc ;
   }

   /* ensure that the tagIndex addition gets flushed to disk, otherwise problems may ensue
      because we generally don't flush changes to that tag */
   rc = index4update( i4->indexFile ) ;
   if ( rc < 0 )
      return rc ;

   return 0 ;
}

/* adds TAG4 entries to all other i4 structures referring to the same index file */
static int i4tagAssociate( INDEX4 *i4 )
{
   INDEX4 *indexOn ;
   DATA4 *dataOn ;
   LIST4 *list ;
   CODE4 *c4 ;
   TAG4 *tagOn, *newTag, *oldTag ;
   #ifdef S4SERVER
      SERVER4CLIENT *client ;
   #endif

   #ifdef E4PARM_LOW
      if ( i4 == 0 )
         return error4( 0, e4parm_null, E91718 ) ;
   #endif

   c4 = i4->codeBase ;

   #ifdef S4SERVER
       list4mutexWait(&c4->server->clients) ;
       for( client = 0 ;; )
       {
          client = (SERVER4CLIENT *)l4next( &c4->server->clients.list, client ) ;
          if ( client == 0 )
             break ;
          list = tran4dataList( &client->trans ) ;
   #else
      list = tran4dataList( &c4->c4trans.trans ) ;
   #endif
      for( dataOn = 0 ;; )
      {
         dataOn = (DATA4 *)l4next( list, dataOn ) ;
         if ( dataOn == 0 )
            break ;
         for( indexOn = 0 ;; )
         {
            indexOn = (INDEX4 *)l4next( &dataOn->indexes, indexOn ) ;
            if ( indexOn == 0 || indexOn == i4 )
               break ;
            if ( indexOn->indexFile == i4->indexFile )
            {
               for ( tagOn = 0 ;; )
               {
                  tagOn = (TAG4 *)l4next( &i4->tags, tagOn ) ;
                  if ( tagOn == 0 )
                     break ;
                  for ( oldTag = 0 ;; )
                  {
                     oldTag = (TAG4 *)l4next( &indexOn->tags, oldTag ) ;
                     if ( oldTag == 0 )
                        break ;
                     if ( oldTag->tagFile == tagOn->tagFile )
                        break ;
                  }
                  if ( oldTag == 0 )
                  {
                     newTag = (TAG4 *)mem4alloc( c4->tagMemory ) ;
                     if ( newTag == 0 )
                     {
                     #ifdef S4SERVER
                        list4mutexRelease(&c4->server->clients) ;
                     #endif
                        return error4stack( c4, e4memory, E91718 ) ;
                     }
                     newTag->index = indexOn ;
                     newTag->tagFile = tagOn->tagFile ;
                     #ifdef S4SERVER
                        newTag->errUnique = t4unique( tagOn ) ;
                     #endif
                     l4add( &indexOn->tags, newTag ) ;
                  }
               }
            }
         }
      }
   #ifdef S4SERVER
      }
      list4mutexRelease(&c4->server->clients) ;
   #endif

   return 0 ;
}

/* takes an array of TAG4INFO and adds the input tags to the already existing
   index file i4 */
int S4FUNCTION i4tagAdd( INDEX4 *i4, const TAG4INFO *tagData )
{
   int i, saveRc, rc ;
   CODE4 *c4 ;
   DATA4 *d4 ;
   #ifdef S4LOW_MEMORY
      #ifndef S4OFF_OPTIMIZE
         int hasOpt ;
      #endif
   #endif
   #ifdef E4MISC
      int oldTagError ;
   #endif

   #ifdef E4PARM_HIGH
      if ( i4 == 0 || tagData == 0 )
         return error4( 0, e4parm_null, E91717 ) ;
   #endif

   c4 = i4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;
   d4 = i4->data ;

   #ifndef S4OFF_WRITE
      rc = d4updateRecord( d4, 0 ) ;
      if ( rc )
         return rc ;
   #endif

   if ( d4->readOnly == 1 )
      return error4describe( c4, e4write, E80606, d4alias( d4 ), 0, 0 ) ;

   #ifndef S4SINGLE
      rc = d4lockIndex( d4 ) ;
      if ( rc < 0 )
         return rc ;
   #endif  /* S4SINGLE */

   #ifndef S4OFF_OPTIMIZE
      #ifdef S4LOW_MEMORY
         hasOpt = c4->hasOpt ;
         rc = code4optSuspend( c4 ) ;
         if ( rc < 0 )
            return rc ;
      #endif
   #endif  /* not S4OFF_OPTIMIZE */

   #ifndef S4SINGLE
      #ifdef S4FOX
         if ( i4->indexFile->file.lowAccessMode != OPEN4DENY_RW )
            i4->indexFile->tagIndex->header.version =  i4->indexFile->versionOld + 1 ;
      #endif
   #endif

   saveRc = 0 ;
   for ( i = 0 ; tagData[i].name ; i++ )
   {
      #ifdef E4MISC
         oldTagError = c4->errTagName ;
         c4->errTagName = 0 ;
         if ( d4tag( d4, tagData[i].name ) != 0 )
         {
            saveRc = error4( c4, e4index, E81713 ) ;
            break ;
         }
         c4->errTagName = oldTagError ;
      #endif

      rc = i4addOneTag( i4, &tagData[i] ) ;
      if ( rc != 0 )
      {
         saveRc = rc ;
         break ;
      }
   }

   #ifndef S4OFF_OPTIMIZE
      #ifdef S4LOW_MEMORY
         if ( hasOpt )
            code4optRestart( c4 ) ;
      #endif
   #endif

   rc = i4tagAssociate( i4 ) ;
   if ( rc < 0 && saveRc == 0 )
      saveRc = rc ;
   return saveRc ;
}

#endif /* S4CLIENT */
#endif /* S4CLIPPER */

#ifndef S4CLIENT
/* remove all tags/indexes from data file */
#ifdef S4CLIPPER
int i4indexRemove( INDEX4 *index )
{
   char indexPathName[LEN4PATH] ;
   TAG4 *tag ;

   for ( tag = 0 ; ; )
   {
      tag = (TAG4 *)l4first( &index->tags ) ;
      if ( tag == 0 )
         break ;
      if ( tag->tagFile->userCount != 1
      #ifndef S4OFF_MULTI
         || tag->tagFile->file.lowAccessMode != OPEN4DENY_RW
      #endif
      )
         return error4( tag->index->codeBase, e4remove, E81306 ) ;

      u4nameCurrent( indexPathName, sizeof( indexPathName ), tag->tagFile->file.name ) ;

      t4close( tag ) ;

      u4remove( indexPathName ) ;
   }

   /* i4closeLow( index ) ;  not required because t4close removes index from list */

   return 0 ;
}
#else
int i4indexRemove( INDEX4 *index )
{
   INDEX4FILE *i4file ;
   char indexPathName[LEN4PATH] ;

   i4file = index->indexFile ;

   if ( i4file->userCount != 1
      #ifndef S4OFF_MULTI
         || i4file->file.lowAccessMode != OPEN4DENY_RW
      #endif
      )
      return error4( index->data->codeBase, e4remove, E81306 ) ;

   u4nameCurrent( indexPathName, sizeof( indexPathName ), i4file->file.name ) ;

   if ( i4file != 0 )  /* means still around, force closure */
   {
      if ( index4isProduction( i4file ) )
         #ifdef S4SERVER
            index->data->dataFile->hasMdxMemo = 0 ;
         #else
            index->data->dataFile->openMdx = 0 ;
         #endif
      i4closeLow( index ) ;
   }

   u4remove( indexPathName ) ;

   return 0 ;
}
#endif /* S4CLIPPER */

int S4FUNCTION d4indexesRemove( DATA4 *data )
{
   INDEX4 *index ;

   for ( index = 0 ;; )
   {
      index = (INDEX4 *)l4first( &data->indexes ) ;
      if ( index == 0 )
         break ;
      if ( i4indexRemove( index ) < 0 )
         return -1 ;
   }

   return 0 ;
}

#ifdef S4CLIPPER
int S4FUNCTION i4tagRemove( TAG4 *tag )
{
   char indexPathName[LEN4PATH] ;
   DATA4FILE *data ;
   CODE4 *c4 ;

   data = tag->index->data->dataFile ;
   c4 = data->c4 ;

   if ( tag->tagFile->userCount != 1
   #ifndef S4OFF_MULTI
      || tag->tagFile->file.lowAccessMode != OPEN4DENY_RW
   #endif
   )
      return error4( c4, e4remove, E81306 ) ;

   u4nameCurrent( indexPathName, sizeof( indexPathName ), tag->tagFile->file.name ) ;
   if ( l4numNodes( &tag->index->tags ) == 1 ) /* close the index */
   {
      i4closeLow( tag->index ) ;
   }
   else  /* close the tag only */
   {
      tfile4close( tag->tagFile, data ) ;
   }
   u4remove( indexPathName ) ;

   /* may need to remove the tag name from the group file... */
   /* see impnotes.txt, not implemented to handle group file */

   return 0 ;
}
#else
int S4FUNCTION i4tagRemove( TAG4 *tag )
{
   char indexPathName[LEN4PATH] ;
   DATA4FILE *data ;
   INDEX4FILE *i4file ;
   CODE4 *c4 ;

   data = tag->tagFile->indexFile->dataFile ;
   c4 = data->c4 ;

   if ( tag->tagFile->indexFile->userCount != 1
      #ifndef S4OFF_MULTI
         || tag->tagFile->indexFile->file.lowAccessMode != OPEN4DENY_RW
      #endif
      )
      return error4( c4, e4remove, E81306 ) ;

   if ( l4numNodes( &(tag->tagFile->indexFile->tags) ) == 1 )  /* last tag, so delete index file */
   {
      u4nameCurrent( indexPathName, sizeof( indexPathName ), tag->index->indexFile->file.name ) ;
      i4closeLow( tag->index ) ;

      for( ;; )
      {
         i4file = dfile4index( data, indexPathName ) ;
         if ( i4file != 0 )  /* means still around, force closure */
         {
            if ( index4isProduction( i4file ) )
            {
               #ifdef S4SERVER
                  #ifdef S4MDX
                     i4file->header.isProduction = 0 ;
                  #endif
                  data->hasMdxMemo = 0 ;
               #else
                  data->openMdx = 0 ;
               #endif
            }
            index4close( i4file ) ;
         }
         else  /* done */
            break ;
      }

      u4remove( indexPathName ) ;
   }
   else  /* just remove a tag from the file */
   {
      #ifdef S4FOX
         return error4( c4, e4struct, E81306 ) ;  /* not implemented yet */
      #endif
      #ifdef S4MDX
         return error4( c4, e4struct, E81306 ) ;  /* not implemented yet */
      #endif
   }

   return 0 ;
}
#endif /* S4CLIPPER */
#endif /* S4CLIENT */
#endif /* S4OFF_WRITE */
#endif /* S4OFF_INDEX */
